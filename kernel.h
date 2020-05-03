// kernel.h
/* Heavily based on TinyRealTime- Dan Henriksson, Anton Cervin */
#ifndef KERNEL_H
#define KERNEL_H

#define MAX_NUMBER_OF_TASKS 5 

void kernel_init(uint16_t stackReserve);

char kernel_taskCreate(void (*func)(void*), uint16_t stacksize, void* args);

void panic();

#endif /* KERNEL_H */
