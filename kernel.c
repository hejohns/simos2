// kernel.c
/* Heavily based on TinyRealTime- Dan Henriksson, Anton Cervin */
#include <stdio.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <AVR-UART-lib/usart.h>
#include "kernel.h"

#define lo8(X) (((uint16_t)X)&0x00FF)
#define hi8(X) ((((uint16_t)X)&0xFF00)>>8)

#define resetCounter1() do{ TCNT1 = 0x0000;} while(false) // reset counter 1 (register of timer 1)

static uint8_t* STACK_MINIMUM_ADDRESS = (uint8_t*)400;

/* Refer to godbolt.org for graphical asm output and
 * https://www.avrfreaks.net/forum/need-save-and-restore-stack-pointer-table
 * (clawson includes a snippet from FreeRTOS w/ their context push/pop)
 */
#define contextPush() \
    asm volatile(\
        "push __tmp_reg__\n\t"\
        "push __zero_reg__\n\t"\
        "push r2\n\t"\
        "push r3\n\t"\
        "push r4\n\t"\
        "push r5\n\t"\
        "push r6\n\t"\
        "push r7\n\t"\
        "push r8\n\t"\
        "push r9\n\t"\
        "push r10\n\t"\
        "push r11\n\t"\
        "push r12\n\t"\
        "push r13\n\t"\
        "push r14\n\t"\
        "push r15\n\t"\
        "push r16\n\t"\
        "push r17\n\t"\
        "push r18\n\t"\
        "push r19\n\t"\
        "push r20\n\t"\
        "push r21\n\t"\
        "push r22\n\t"\
        "push r23\n\t"\
        "push r24\n\t"\
        "push r25\n\t"\
        "push r26\n\t"\
        "push r27\n\t"\
        "push r28\n\t"\
        "push r29\n\t"\
        "push r30\n\t"\
        "push r31\n\t"\
        "in __tmp_reg__,  __SREG__\n\t"\
        "push __tmp_reg__\n\t"\
        "in r28,__SP_L__\n\t"\
        "in r29,__SP_H__\n\t"\
        "clr __zero_reg__")

#define contextPop() \
    asm volatile(\
        "pop __tmp_reg__\n\t"\
        "out __SREG__, __tmp_reg__\n\t"\
        "pop r31\n\t"\
        "pop r30\n\t"\
        "pop r29\n\t"\
        "pop r28\n\t"\
        "pop r27\n\t"\
        "pop r26\n\t"\
        "pop r25\n\t"\
        "pop r24\n\t"\
        "pop r23\n\t"\
        "pop r22\n\t"\
        "pop r21\n\t"\
        "pop r20\n\t"\
        "pop r19\n\t"\
        "pop r18\n\t"\
        "pop r17\n\t"\
        "pop r16\n\t"\
        "pop r15\n\t"\
        "pop r14\n\t"\
        "pop r13\n\t"\
        "pop r12\n\t"\
        "pop r11\n\t"\
        "pop r10\n\t"\
        "pop r9\n\t"\
        "pop r8\n\t"\
        "pop r7\n\t"\
        "pop r6\n\t"\
        "pop r5\n\t"\
        "pop r4\n\t"\
        "pop r3\n\t"\
        "pop r2\n\t"\
        "pop __zero_reg__\n\t"\
        "pop __tmp_reg__")

typedef enum __attribute__ ((__packed__)) state
{
    terminated = 0x0,
    raw = 0x1,
    ready = 0x2,
    running = 0x3,
    stopped = 0x4
} state;

typedef struct process
{
    uint8_t* stackTop; // what to restore kernel.memptr to after tasks completes
    uint16_t sp;
    state state;
} process;

static struct
{
    process pid[MAX_NUMBER_OF_TASKS];
    uint8_t* stackBottom; // pointer to bottom of stack
    uint16_t sp_tmp;
    uint8_t nbrOfTasks;
    uint8_t running;
    uint8_t scratch[8];
} kernel;

void kernel_init(uint16_t stackReserve)
{
    // set up timer 1
    TCCR1A = 0b00000000; // normal operation- disconnect OC1, disable pwm
    TCCR1B = 0b00001100; // reset counter on overflow, prescaler=256
    TIMSK1 = 0b00000010; // output compare (OCIE1A) bit set
    OCR1AH = 0x34; // 100ms=6250 counts at 256 scale
    OCR1AL = 0xBC; // set to 200ms slices
    kernel.stackBottom = (uint8_t*)(RAMEND-stackReserve);
    kernel.nbrOfTasks = 0;
    kernel.running = 0;
    TCNT1 = 0x0000; // reset counter 1 (register of timer 1)
}

char kernel_taskCreate(void (*func)(void*), uint16_t stackSize , void* args){
    uint8_t* sp;
    if(kernel.stackBottom-stackSize < STACK_MINIMUM_ADDRESS){
        goto ENOMEM;
    }
    if(kernel.nbrOfTasks >= MAX_NUMBER_OF_TASKS){
        goto ENOBUFS;
    }
    if(0){
        // what qualifies as an invalid arg?
        goto EINVAL;
    }
    sp = kernel.stackBottom;
    kernel.pid[kernel.nbrOfTasks].stackTop = kernel.stackBottom;
    kernel.pid[kernel.nbrOfTasks].state = raw;
    for(unsigned char i=0; i<24; i++){
        *(sp--) = 0x00;
    }
    // load args in r24:25
    *(sp--) = lo8(args);
    *(sp--) = hi8(args);
    *(sp--) = 0x00; // clear r26
    *(sp--) = 0x00; // clear r27
    *(sp--) = lo8(kernel.stackBottom); // r28- aka lo8(Y)
    *(sp--) = hi8(kernel.stackBottom); // r29- aka hi8(Y)
    // load Z reg for ijmp
    *(sp--) = lo8(func); // r30- aka lo8(Z)
    *(sp--) = hi8(func); // r31- aka hi8(Z)
    *(sp--) = 0x0; // __SREG__
    kernel.pid[kernel.nbrOfTasks].sp = (uint16_t)sp;
    kernel.nbrOfTasks++;
    kernel.stackBottom -= stackSize;
    return 0;
ENOBUFS:
    return 1;
EINVAL:
    return -1;
ENOMEM:
    return 2;
}

static inline void kernel_scheduler(){
    if(kernel.pid[kernel.running].state == running){
        kernel.pid[kernel.running].state = ready;
        kernel.pid[kernel.running].sp = kernel.sp_tmp;
    }
    // simple round robin
    kernel.running = (kernel.running+1) % kernel.nbrOfTasks;
}

ISR(TIMER1_COMPA_vect, ISR_NAKED)
{
    // called by timer interrupt. The "meat" of the kernel.
    // prologue
    cli();
    /* interrupt pushes 3 byte pc to stack */
    contextPush();
    kernel.sp_tmp = SP;
    kernel_scheduler();
    // epilogue
    if(kernel.pid[kernel.running].state == raw){
        goto raw2running;
    }
    SP = kernel.pid[kernel.running].sp;
    contextPop();
    resetCounter1();
    reti();
raw2running:
    kernel.pid[kernel.running].state = running;
    SP = kernel.pid[kernel.running].sp;
    contextPop();
    resetCounter1();
    sei();
    asm volatile("ijmp");
}

ISR(BADISR_vect)
{
    // if called, something very bad has happened...
    cli();
    printf("Catch-all interrupt triggered\n");
    panic();
}

void panic()
{
    cli();
    printf("Kernel panic!\nJumping to bootloader...\n");
    // goto address 0 == reboot
    SP = RAMEND;
    goto *0x0000; //some day, will change to watchdog reset to properly reset everything
}

/* Useful info: */

/* SREG bits: 
     * 7- I: Global Interrupt Enable
     * 6- T: Copy Storage
     * 5- H: Half Carry Flag
     * 4- S: Sign Flag
     * 3- V: Two's Compliment Overflow Flag
     * 2- N: Negative Flag
     * 1- Z: Zero Flag
     * 0- C: Carry Flag
 */

/* Unnecessary, but leaving this here to remember
 * about such instructions
    asm volatile(
            "sbrc __zero_reg__,0\n\t"
            "reti\n\t"
            "sbrc __zero_reg__,1\n\t"
            "reti\n\t"
            "sbrc __zero_reg__,2\n\t"
            "reti\n\t"
            "sbrc __zero_reg__,3\n\t"
            "reti\n\t"
            "sbrc __zero_reg__,4\n\t"
            "reti\n\t"
            "sbrc __zero_reg__,5\n\t"
            "reti\n\t"
            "sbrc __zero_reg__,6\n\t"
            "reti\n\t"
            "sbrc __zero_reg__,7\n\t"
            "reti");
*/
