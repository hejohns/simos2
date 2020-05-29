#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>

#include <AVR-UART-lib/usart.h>

#include "kernel.h"
#include "sh.h"
#include "mymalloc.h"

size_t volatile omg = 0;
size_t volatile omg2 = 0;
size_t volatile omg3 = 0;

void sh_init(void* arg){
    kernel_taskCreate(&test2, 128, (void*)0);
    kernel_taskCreate(&test3, 128, (void*)0);
    while(true){
        omg++;
    }
}

void test2(void* arg){
    while(true){
        omg2++;
    }
}

void test3(void* arg){
    char* ptr = malloc(64);
    ptr[1] = 'A';
    ptr[2] = ptr[1];
    while(true){
        omg3++;
    }
}
