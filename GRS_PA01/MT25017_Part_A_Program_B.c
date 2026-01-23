#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include "MT25017_Part_B_W2.h"
#include<string.h>

void* run(void* arg) {
    if (arg == NULL) return NULL;
    char* worker = (char*)arg;
    if(strcmp(worker, "cpu") == 0) cpu(NULL);
    else if(strcmp(worker, "mem") == 0) mem(NULL);
    else io(NULL);
    return NULL;
}

int main(int argc, char* argv[]) {
    int N;
    char* worker;
    
    // Handle BOTH Part C (1 arg) and Part D (2 args)
    if (argc == 2) {
        // Part C: ./program cpu|mem|io (default 2 threads)
        N = 2;
        worker = argv[1];
    } 
    else if (argc == 3) {
        // Part D: ./program <N> <cpu|mem|io>
        N = atoi(argv[1]);
        worker = argv[2];
    } 
    else {
        printf("Usage:\n");
        printf("  %s <cpu|mem|io>          (Part C, 2 threads)\n", argv[0]);
        printf("  %s <N> <cpu|mem|io>      (Part D, N threads)\n", argv[0]);
        return 1;
    }
    
    pthread_t threads[8];  // Max 8 threads (Part D)
    
    // Create N threads
    for(int i = 0; i < N; i++) {
        pthread_create(&threads[i], NULL, run, (void*)worker);
    }
    
    // Join all N threads
    for(int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Main thread: All %d worker threads finished.\n", N);
    return 0;
}
