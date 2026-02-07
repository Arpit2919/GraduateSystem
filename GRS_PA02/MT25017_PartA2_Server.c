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

int g_message_size = 1024;
volatile int g_running = 1;

int g_server_socket;

long long g_total_bytes_sent = 0;
pthread_mutex_t g_stats_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    char *field1,*field2,*field3,*field4;
    char *field5,*field6,*field7,*field8;
} Message;

Message* create_message(int total_size)
{
    Message *msg = malloc(sizeof(Message));
    int fs = total_size / 8;

    msg->field1 = malloc(fs); msg->field2 = malloc(fs);
    msg->field3 = malloc(fs); msg->field4 = malloc(fs);
    msg->field5 = malloc(fs); msg->field6 = malloc(fs);
    msg->field7 = malloc(fs); msg->field8 = malloc(fs);

    memset(msg->field1,'A',fs);
    memset(msg->field2,'B',fs);
    memset(msg->field3,'C',fs);
    memset(msg->field4,'D',fs);
    memset(msg->field5,'E',fs);
    memset(msg->field6,'F',fs);
    memset(msg->field7,'G',fs);
    memset(msg->field8,'H',fs);

    return msg;
}

void free_message(Message *m)
{
    free(m->field1); free(m->field2); free(m->field3); free(m->field4);
    free(m->field5); free(m->field6); free(m->field7); free(m->field8);
    free(m);
}

ssize_t sendmsg_full(int sock, struct msghdr *msg, size_t total_len)
{
    size_t sent_total = 0;

    while(sent_total < total_len)
    {
        ssize_t n = sendmsg(sock, msg, MSG_NOSIGNAL);

        if(n < 0){
            if(errno == EINTR) continue;
            return -1;
        }

        if(n == 0) return 0;

        sent_total += n;
    }

    return sent_total;
}

void* client_handler(void *arg)
{
    int sock = *(int*)arg;   //
    free(arg);

    Message *msg = create_message(g_message_size);

    int fs = g_message_size / 8;

    struct iovec iov[8] = {
        {msg->field1, fs},{msg->field2, fs},{msg->field3, fs},{msg->field4, fs},
        {msg->field5, fs},{msg->field6, fs},{msg->field7, fs},{msg->field8, fs}
    };

    struct msghdr message = {0};
    message.msg_iov = iov;
    message.msg_iovlen = 8;

    long long bytes_sent = 0;

    while(g_running)
    {
        ssize_t s = sendmsg_full(sock, &message, g_message_size);
        if(s <= 0) break;
        bytes_sent += s;
    }

    pthread_mutex_lock(&g_stats_mutex);
    g_total_bytes_sent += bytes_sent;
    pthread_mutex_unlock(&g_stats_mutex);

    free_message(msg);
    close(sock);
    return NULL;
}

void signal_handler(int signum)
{
    (void)signum;
    g_running = 0;
    close(g_server_socket);  
}

int main(int argc, char *argv[])
{
    if(argc != 3){
        printf("Usage: %s <port> <message_size>\n", argv[0]);
        return 1;
    }

    signal(SIGPIPE, SIG_IGN);  
    signal(SIGINT, signal_handler);

    int port = atoi(argv[1]);
    g_message_size = atoi(argv[2]);

    g_server_socket = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(g_server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr={0};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(g_server_socket,(struct sockaddr*)&addr,sizeof(addr));
    listen(g_server_socket,50);

    while(g_running)
    {
        int *cs = malloc(sizeof(int));
        *cs = accept(g_server_socket,NULL,NULL);

        if(*cs < 0){
            free(cs);
            continue;
        }

        pthread_t t;
        pthread_create(&t,NULL,client_handler,cs);
        pthread_detach(t);
    }

    printf("Total bytes sent: %lld\n", g_total_bytes_sent);
    return 0;
}
