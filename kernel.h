//kernel.h
/* Heavily based on TinyRealTime- Dan Henriksson, Anton Cervin */
#ifndef KERNEL_H
#define KERNEL_H
#include <avr/interrupt.h>

void kernel_init(uint16_t stackReserve);
void panic();

#endif /* KERNEL_H */
