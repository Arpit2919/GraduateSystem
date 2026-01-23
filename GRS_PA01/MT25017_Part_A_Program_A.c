#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<string.h>
#include "MT25017_Part_B_W2.h"

int main(int argc, char* argv[]) {
    int N;
    char* worker;
    
    // Handle BOTH Part C (1 arg) and Part D (2 args)
    if (argc == 2) {
        // Part C: ./program cpu|mem|io (default 2 child processes)
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
        printf("  %s <cpu|mem|io>          (Part C, 2 processes)\n", argv[0]);
        printf("  %s <N> <cpu|mem|io>      (Part D, N processes)\n", argv[0]);
        return 1;
    }
    
    pid_t pids[5];  // Max 5 processes
    
    // Create N child processes
    for(int i = 0; i < N; i++) {
        pids[i] = fork();
        if(pids[i] == 0) {  // Child
            if(strcmp(worker, "cpu") == 0) cpu(NULL);
            else if(strcmp(worker, "mem") == 0) mem(NULL);
            else if(strcmp(worker, "io") == 0) io(NULL);
            exit(0);
        }
    }
    
    // Wait for all N children
    for(int i = 0; i < N; i++) {
        wait(NULL);
    }
    
    printf("Main Process: All %d child processes finished.\n", N);
    return 0;
}
