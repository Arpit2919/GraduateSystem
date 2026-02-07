#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <sys/uio.h>

// GLOBALS
int g_message_size = 1024;
volatile int g_running = 1;
int g_server_socket;

long long g_total_bytes_sent = 0;
pthread_mutex_t g_stats_mutex = PTHREAD_MUTEX_INITIALIZER;


//MESSAGE

typedef struct {
    char *field[8];
} Message;


//MESSAGE HELPERS

Message* create_message(int total)
{
    Message *m = malloc(sizeof(Message));
    if(!m) return NULL;

    int fs = total/8;

    for(int i=0;i<8;i++){
        m->field[i] = malloc(fs);
        if(!m->field[i]){
            for(int j=0;j<i;j++) free(m->field[j]);
            free(m);
            return NULL;
        }
        memset(m->field[i],'A'+i,fs);
    }
    return m;
}

void free_message(Message *m)
{
    if(!m) return;
    for(int i=0;i<8;i++) free(m->field[i]);
    free(m);
}


//SIGNAL

void signal_handler(int sig)
{
    (void)sig;
    g_running = 0;
    close(g_server_socket);  
}


//THREADS

void* client_handler(void *arg)
{
    int sock = *(int*)arg;
    free(arg);

    printf("[Thread %lu] client connected\n",(unsigned long)pthread_self());

    int enable=1;
    setsockopt(sock,SOL_SOCKET,SO_ZEROCOPY,&enable,sizeof(enable));

    Message *msg = create_message(g_message_size);
    if(!msg){
        close(sock);
        return NULL;
    }

    int fs = g_message_size/8;

    struct iovec iov[8];
    for(int i=0;i<8;i++){
        iov[i].iov_base = msg->field[i];
        iov[i].iov_len  = fs;
    }

    long long bytes=0;

    while(g_running)
    {
        struct msghdr m={0};
        m.msg_iov=iov;
        m.msg_iovlen=8;

        ssize_t s = sendmsg(sock,&m,MSG_ZEROCOPY|MSG_NOSIGNAL);

        if(s < 0){
            if(errno==EINTR) continue;
            if(errno==ENOBUFS){ usleep(1000); continue; }
            break;
        }

        bytes += s;
    }

    pthread_mutex_lock(&g_stats_mutex);
    g_total_bytes_sent += bytes;
    pthread_mutex_unlock(&g_stats_mutex);

    printf("[Thread %lu] bytes sent = %lld\n",
           (unsigned long)pthread_self(), bytes);

    free_message(msg);
    close(sock);
    return NULL;
}


//MAIN

int main(int argc,char *argv[])
{
    if(argc!=3){
        printf("Usage: %s <port> <size>\n",argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    g_message_size = atoi(argv[2]);

    signal(SIGINT,signal_handler);
    signal(SIGTERM,signal_handler);
    signal(SIGPIPE,SIG_IGN);

    g_server_socket = socket(AF_INET,SOCK_STREAM,0);

    int opt=1;
    setsockopt(g_server_socket,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    setsockopt(g_server_socket,IPPROTO_TCP,TCP_NODELAY,&opt,sizeof(opt));

    struct sockaddr_in addr={0};
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    addr.sin_addr.s_addr=INADDR_ANY;

    bind(g_server_socket,(struct sockaddr*)&addr,sizeof(addr));
    listen(g_server_socket,50);

    printf("=== Zero-Copy Server (A3) ===\n");

    while(g_running)
    {
        int *cs = malloc(sizeof(int));

        *cs = accept(g_server_socket,NULL,NULL);

        if(*cs < 0){
            free(cs);
            if(errno==EINTR) continue;
            if(!g_running) break;
            continue;
        }

        pthread_t t;
        pthread_create(&t,NULL,client_handler,cs);
        pthread_detach(t);
    }

    printf("\nTotal bytes sent: %lld\n",g_total_bytes_sent);
    printf("Total MB: %.2f\n",g_total_bytes_sent/(1024.0*1024.0));

    close(g_server_socket);
    pthread_mutex_destroy(&g_stats_mutex);
    return 0;
}
