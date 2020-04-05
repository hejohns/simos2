//kernel.c
/* Heavily based on TinyRealTime- Dan Henriksson, Anton Cervin */
#include <stdio.h>
#include <avr/io.h>
#include "kernel.h"

#define lo8(X) (((uint16_t)X)&0x00FF)
#define hi8(X) ((((uint16_t)X)&0xFF00)>>8)

#define contextPush() \
    asm(\
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
        "push __tmp_reg__")
			
#define contextPop() \
    asm(\
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

//const char taskHeader[] = "task header";
//const char taskFooter[] = "task footer";
//const char kernelHeader[] = "kernel header";
//const char kernelFooter[] = "kernel footer";

typedef enum
{
    terminated = 0,
    raw = 1,
    ready = 2,
    running = 3,
    stopped = 4
} state_T;

typedef struct task
{
    //char header[strlen(taskHeader)+1];
    void* sTop;
    void* sp; //stack pointer
    state_T state;
    //char footer[strlen(taskFooter)+1];
} task_T;

static struct
{
    //char header[strlen(kernelHeader)+1];
    void* memptr; //pointer to bottom of stack
    uint8_t nbrOfTasks;
    uint8_t running;
    task_T tasks[MAX_NUMBER_OF_TASKS];
    //char footer[strlen(kernelFooter)+1];
} kernel;

void kernel_init(uint16_t stackReserve)
{
    //set up timer 1
    TCCR1A = 0b00000000; //normal operation- disconnect OC1, disable pwm
    TCCR1B = 0b00001100; //reset counter on overflow, prescaler=256
    TIMSK1 = 0b00000010; //output compare (OCIE1A) bit set
    OCR1AH = 0x34; //100ms=6250 counts at 256 scale
    OCR1AL = 0xBC; //set to 200ms slices
    //strcpy(kernel.header, kernelHeader);
    //strcpy(kernel.footer, kernelFooter);
    kernel.memptr = (void*)(RAMEND-stackReserve);
    kernel.nbrOfTasks = 0;
    kernel.running = 0;
    //kernel.taskRunning2Ready = &taskRunning2Ready_;
    //kernel.taskReady2Running = &taskReady2Running_;
    //kernel.taskRaw2Running = &taskRaw2Running_;
    //kernel.taskCreate = &taskCreate_;
    //kernel.taskTerminate = &taskTerminate_;
    //kernel.taskStop = &taskStop_;
    //kernel.taskResume = &taskResume_;
    //kernel.taskDestroy = &taskDestroy_;
    //kernel.shInit = &shInit_;
    TCNT1 = 0x0000; //reset counter 1 (register of timer 1)
}

ISR(TIMER1_COMPA_vect, ISR_NAKED)
{
    //called by timer interrupt. The "meat" of the kernel.
    //prologue
    cli();
    contextPush();
    kernel.tasks[kernel.running].sp = SP;
    uint8_t runNext;
    //pop terminated tasks off kernel task stack
    for(uint8_t i=kernel.nbrOfTasks-1; i>=0; i--){
        task_T* taskN = &(kernel.tasks[i]);
        if(taskN->state == terminated){
            kernel.taskDestroy(i);
            continue;
        }
        else{
            break;
        }
    }
    //Round Robin scheduler
    if(kernel.nbrOfTasks <= 0){
        panic();
    }
    else if(kernel.nbrOfTasks == 1){
            runNext = 0;
    }
    else{
        uint8_t i = (kernel.running+1)%kernel.nbrOfTasks;
        do{
            //process based on state
            task* taskN = &kernel.tasks[i];
            if(taskN->state == stopped){
                i = (i+1)%kernel.nbrOfTasks;
                continue;
            }
            else if(taskN->state == terminated){
                i = (i+1)%kernel.nbrOfTasks;
                continue;
            }
            else if(taskN->state == ready){
                kernel.taskRunning2Ready(kernel.running);
                kernel.taskReady2Running(i);
                kernel.running = i;
                runNext = i;
                break;
            }
            else if(taskN->state == raw){
                kernel.taskRunning2Ready(kernel.running);
                kernel.taskRaw2Running(i);
            }
            else
            {
                printf(\
                "Scheduler Failure\ni: %d\ntaskN->state: %d\n", i, taskN->state);
                panic();
            }
        } while(true);
    }
    //epilogue
    SP = kernel.tasks[runNext].sp;
    contextPop();
    TCNT1 = 0x0000;
    reti();
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
    goto *0x0000;
}
