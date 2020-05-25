// kernel.h
/* Heavily based on TinyRealTime- Dan Henriksson, Anton Cervin */
#ifndef KERNEL_H
#define KERNEL_H

#define MAX_NUMBER_OF_TASKS 5 
#define TOTAL_TASK_RAM_SIZE 4092

void kernel_init(uint8_t* task_ram_start);

char kernel_taskCreate(void (*func)(void*), uint16_t stacksize, void* args);

void panic();

#endif /* KERNEL_H */
