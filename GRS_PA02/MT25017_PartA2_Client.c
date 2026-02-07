/* Part A2:One-Copy Client Implementation
 * Description: Baseline TCP client using recv() - demonstrates two-copy mechanism
 */

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
#include <sys/time.h>
#include <signal.h>

// Global configuration
#define DURATION_SECONDS 10  // Fixed duration as per assignment

// Global variables
int g_message_size = 1024;
int g_server_socket;
volatile int g_running = 1;
long long g_total_bytes_received = 0;
long long g_total_messages = 0;
double g_total_latency_us = 0;
pthread_mutex_t g_stats_mutex = PTHREAD_MUTEX_INITIALIZER;

// Thread arguments structure
typedef struct {
    char *server_ip;
    int server_port;
    int thread_id;
} ThreadArgs;

// Get current time in microseconds
long long get_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000000LL + tv.tv_usec;
}

// Receive exact number of bytes (handles partial receives)
int recv_full(int socket, char *buffer, size_t length) {
    size_t total_received = 0;
    while (total_received < length) {
        ssize_t received = recv(socket, buffer + total_received,
                               length - total_received, 0);
        if (received < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (received == 0) return 0;  // Connection closed
        total_received += received;
    }
    return total_received;
}

// Client thread function
void* client_thread(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
   
    printf("[Client %d] Starting connection...\n", args->thread_id);
   
    // Create TCP socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        free(args);
        return NULL;
    }
   
    // Disable Nagle's algorithm
    int opt = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
   
    // Increase receive buffer size
    int rcvbuf = 256 * 1024;
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));
   
    // Setup server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(args->server_port);
   
    if (inet_pton(AF_INET, args->server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sock);
        free(args);
        return NULL;
    }
   
    // Connect to server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        free(args);
        return NULL;
    }
   
    printf("[Client %d] Connected to server successfully\n", args->thread_id);
   
    // Allocate receive buffer
    char *recv_buffer = (char *)malloc(g_message_size);
    if (!recv_buffer) {
        perror("Failed to allocate receive buffer");
        close(sock);
        free(args);
        return NULL;
    }
   
    // Statistics for this thread
    long long bytes_received = 0;
    long long messages_received = 0;
    double total_latency = 0;
   
    // Calculate end time (current time + duration)
    long long start_time = get_time_us();
    long long end_time = start_time + (DURATION_SECONDS * 1000000LL);
   
    // Receive messages for the specified duration
    while (g_running && get_time_us() < end_time) {
        long long msg_start = get_time_us();
       
        // Receive complete message
        // Two-copy mechanism: kernel buffer -> user buffer (via recv())
        int received = recv_full(sock, recv_buffer, g_message_size);
       
        if (received < 0) {
            perror("Receive failed");
            break;
        } else if (received == 0) {
            printf("[Client %d] Server disconnected\n", args->thread_id);
            break;
        }
       
        long long msg_end = get_time_us();
       
        // Update statistics
        bytes_received += received;
        messages_received++;
        total_latency += (msg_end - msg_start);
    }
   
    // Update global statistics (thread-safe)
    pthread_mutex_lock(&g_stats_mutex);
    g_total_bytes_received += bytes_received;
    g_total_messages += messages_received;
    g_total_latency_us += total_latency;
    pthread_mutex_unlock(&g_stats_mutex);
   
    printf("[Client %d] Finished. Bytes: %lld, Messages: %lld\n",
           args->thread_id, bytes_received, messages_received);
   
    // Cleanup
    free(recv_buffer);
    close(sock);
    free(args);
   
    return NULL;
}

// Signal handler for graceful shutdown
void signal_handler(int signum) {
    printf("\n[Signal] Received signal %d, stopping client...\n", signum);
    (void)signum;
    g_running = 0;
    close(g_server_socket);
    
}

int main(int argc, char *argv[]) {
    // Validate command-line arguments
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <server_ip> <port> <message_size> <num_threads>\n", argv[0]);
        fprintf(stderr, "Example: %s 127.0.0.1 8080 1024 2\n", argv[0]);
        exit(EXIT_FAILURE);
    }
   
    char *server_ip = argv[1];
    int port = atoi(argv[2]);
    g_message_size = atoi(argv[3]);
    int num_threads = atoi(argv[4]);
   
    // Validate parameters
    if (g_message_size % 8 != 0) {
        fprintf(stderr, "Error: Message size must be divisible by 8\n");
        exit(EXIT_FAILURE);
    }
   
    if (num_threads < 1 || num_threads > 100) {
        fprintf(stderr, "Error: Number of threads must be between 1 and 100\n");
        exit(EXIT_FAILURE);
    }
   
    printf("=== Two-Copy Client (Part A1) ===\n");
    printf("Server: %s:%d\n", server_ip, port);
    printf("Message Size: %d bytes\n", g_message_size);
    printf("Number of Threads: %d\n", num_threads);
    printf("Duration: %d seconds\n", DURATION_SECONDS);
    printf("Implementation: recv() - Two-copy mechanism\n\n");
   
    // Setup signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
   
    // Allocate thread array
    pthread_t *threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    if (!threads) {
        perror("Failed to allocate thread array");
        exit(EXIT_FAILURE);
    }
   
    // Record experiment start time
    long long experiment_start = get_time_us();
   
    // Create client threads
    printf("Starting %d client threads...\n\n", num_threads);
    for (int i = 0; i < num_threads; i++) {
        ThreadArgs *args = (ThreadArgs *)malloc(sizeof(ThreadArgs));
        if (!args) {
            perror("Failed to allocate thread arguments");
            continue;
        }
       
        args->server_ip = server_ip;
        args->server_port = port;
        args->thread_id = i;
       
        if (pthread_create(&threads[i], NULL, client_thread, args) != 0) {
            perror("Thread creation failed");
            free(args);
        }
    }
   
    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
   
    // Record experiment end time
    long long experiment_end = get_time_us();
    double total_time_sec = (experiment_end - experiment_start) / 1000000.0;
   
    // Calculate final statistics
    double throughput_gbps = (g_total_bytes_received * 8.0) / (total_time_sec * 1e9);
    double avg_latency_us = g_total_messages > 0 ?
                            g_total_latency_us / g_total_messages : 0;
   
    // Print results
    printf("\n=== Client Results ===\n");
    printf("Total Bytes Received: %lld\n", g_total_bytes_received);
    printf("Total MB Received: %.2f\n", g_total_bytes_received / (1024.0 * 1024.0));
    printf("Total Messages: %lld\n", g_total_messages);
    printf("Total Time: %.2f seconds\n", total_time_sec);
    printf("Throughput: %.4f Gbps\n", throughput_gbps);
    printf("Average Latency: %.2f microseconds\n", avg_latency_us);
    printf("Messages per second: %.2f\n", g_total_messages / total_time_sec);
   
    // Cleanup
    free(threads);
    pthread_mutex_destroy(&g_stats_mutex);
   
    return 0;
}
