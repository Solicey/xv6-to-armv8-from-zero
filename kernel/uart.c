// driver for ARM PrimeCell UART (PL011)
#include "memlayout.h"

static volatile unsigned int* uart_base = (unsigned int*)UART0;

// https://github.com/umanovskis/baremetal-arm/blob/master/doc/06_uart.md
#define UART_DR		    0	        // data register
#define UART_RSR	    1	        // receive status register/error clear register
#define UART_FR		    6	        // flag register
#define UART_FR_TXFF	(1 << 5)	// tramit FIFO full
#define UART_FR_RXFE	(1 << 4)	// receive FIFO empty
#define	UART_IBRD	    9	        // integer baud rate register: 波特率整数部分
#define UART_FBRD	    10	        // Fractional baud rate register
#define UART_LCRH	    11	        // line control register high
#define UART_LCRH_FEN	(1 << 4)	// enable FIFO
#define UART_CR		    12	        // control register: 用于启用和禁用UART，以及配置其他选项，例如发送和接收使能、奇偶校验、流控制等。
#define	UART_CR_RXE	    (1 << 9)	// enable receive
#define UART_CR_TXE	    (1 << 8)	// enable transmit
#define	UART_CR_EN	    (1 << 0)	// enable UART
#define UART_IMSC	    14	        // interrupt mask set/clear register
#define UART_IMSC_RXI	(1 << 4)	// receive interrupt
#define UART_IMSC_TXI	(1 << 5)	// transmit interrupt
#define UART_MIS	    16	        // masked interrupt status register
#define	UART_ICR	    17	        // interrupt clear register

#define UART_BITRATE    19200
#define UART_CLK        24000000 

void uartinit(void)
{
    uart_base = (unsigned int*)(UART0 + KERN_BASE);

    // set the bit rate: integer/fractional baud rate registers
    uart_base[UART_IBRD] = UART_CLK / (16 * UART_BITRATE);

    unsigned int left = UART_CLK % (16 * UART_BITRATE);
    uart_base[UART_FBRD] = (left * 4 + UART_BITRATE / 2) / UART_BITRATE;

    // enable trasmit and receive
    uart_base[UART_CR] |= (UART_CR_EN | UART_CR_RXE | UART_CR_TXE);

    // enable FIFO
    uart_base[UART_LCRH] |= UART_LCRH_FEN;
}

void uartputc(char c)
{
    uart_base[UART_DR] = (unsigned int)c;
}

void uartputs(const char* s)
{
    while (*s != '\0')
    {
        uart_base[UART_DR] = (unsigned int)(*s);
        s++;
    }
}