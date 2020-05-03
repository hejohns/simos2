#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
//#include <avr/pgmspace.h>
//#include <string.h>

#include <AVR-UART-lib/usart.h>
#include "kernel.h"

int main()
{
    //uart_set_FrameFormat(USART_8BIT_DATA|USART_1STOP_BIT|USART_NO_PARITY|USART_ASYNC_MODE); // default settings
    uart_init(BAUD_CALC(115200)); // 8n1 transmission is set as default
    stdout = &uart0_io; // attach uart stream to stdout & stdin
    stdin = &uart0_io; // uart0_in and uart0_out are only available if NO_USART_RX or NO_USART_TX is defined
    
    kernel_init(RAMEND-SP+8);
    sei(); // enable interrupts, library wouldn't work without this
    printf("hello from printf\n");
    while(1){
        // wait for interrupts eternally
    }
    return 0;
}
