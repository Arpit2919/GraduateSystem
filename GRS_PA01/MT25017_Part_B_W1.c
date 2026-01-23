#include "MT25017_Part_B_W2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

#define COUNT 10000

void* cpu(void* arg) {
    (void)arg;
    
    volatile unsigned long long sum0 = 0, sum1 = 0;
    volatile double fsum = 0.0;
    for (int outer = 0; outer < COUNT; outer++) {
        for (volatile int i = 0; i < 10000; i++) {
            unsigned long long x = (unsigned long long)i * 48271u + 12345u;
            sum0 += x;
            sum1 ^= x;
            sum0 = (sum0 << 13) ^ (sum0 >> 19); 
        }
        
        for (volatile int j = 0; j < 10000; j++) {
            double a = (double)j * 3.14159 / 100000.0;
            fsum += sin(a) * sin(a) + cos(a) * cos(a); 
        }
    }
    
    (void)sum0; (void)sum1; (void)fsum;
    return NULL;
}
void* mem(void* arg) {
    sleep(1);
    (void)arg;
    
    const size_t size = 4* 1024 * 1024;  
    int *data = malloc(size / sizeof(int));
    if (!data) return NULL;
    
    for (int loop = 0; loop < COUNT; loop++) {
        for (size_t i = 0; i < size / sizeof(int); i++) {
            data[i] = (int)(i + loop); 
        }
        volatile int dummy = 0;
        for (size_t i = 0; i < size / sizeof(int); i += 1024) {
            dummy += data[i]; 
        }
        (void)dummy;
    }
    
    free(data);
    return NULL;
}

void* io(void* arg) {
    (void)arg;
    
    char *buffer = malloc(256* 1024);
    if (!buffer) return NULL;
    memset(buffer, 0xAA, 256* 1024);
    for (int outer = 0; outer < COUNT; outer++) {
        FILE *fp = fopen("/tmp/io_stress.tmp", "wb");
        if (fp) {
            size_t written = fwrite(buffer, 1, 256* 1024, fp);
            (void)written; 
            fsync(fileno(fp)); 
            fclose(fp);
        }
        fp = fopen("/tmp/io_stress.tmp", "rb");
        if (fp) {
            size_t total = fread(buffer, 1, 256*1024, fp);
            (void)total;
            fclose(fp);
        }
        
        unlink("/tmp/io_stress.tmp");
    }
    
    free(buffer);
    return NULL;
}

