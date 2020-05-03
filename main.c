#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
//#include <avr/pgmspace.h>
//#include <string.h>

#include <AVR-UART-lib/usart.h>
#include "kernel.h"

void testFunc(void* tmp){
    printf("testFunc\n");
    while(1){
        asm("nop");
    }
}

int main()
{
    //uart_set_FrameFormat(USART_8BIT_DATA|USART_1STOP_BIT|USART_NO_PARITY|USART_ASYNC_MODE); // default settings
    uart_init(BAUD_CALC(115200)); // 8n1 transmission is set as default
    stdout = &uart0_io; // attach uart stream to stdout & stdin
    stdin = &uart0_io; // uart0_in and uart0_out are only available if NO_USART_RX or NO_USART_TX is defined
    
    kernel_init(RAMEND-SP+128);
    sei(); // enable interrupts, library wouldn't work without this
    printf("hello from printf\n");
    kernel_taskCreate(&testFunc, 128, (void*)0);
    while(1){
        asm("nop");
        // wait for interrupts eternally
    }
    return 0;
}
