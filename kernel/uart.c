// driver for ARM PrimeCell UART (PL011)
#include "types.h"
#include "memlayout.h"
#include "defs.h"
#include "arm.h"
#include "virt.h"

static volatile uint* uart_base = (uint*)UART_BASE;

// the address of uart register r.
#define R(r)            ((volatile uint32 *)(uart_base + (r)))

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
#define UART_RXI	    (1 << 4)	    // receive interrupt
#define UART_TXI	    (1 << 5)	    // transmit interrupt
#define UART_MIS	    16	        // masked interrupt status register
#define	UART_ICR	    17	        // interrupt clear register

#define UART_BITRATE    19200
#define UART_CLK        24000000 

void uartinit(void)
{
    uart_base = (uint*)(P2V(UART_BASE));

    // set the bit rate: integer/fractional baud rate registers
    *R(UART_IBRD) = UART_CLK / (16 * UART_BITRATE);

    uint left = UART_CLK % (16 * UART_BITRATE);
    *R(UART_FBRD) = (left * 4 + UART_BITRATE / 2) / UART_BITRATE;

    // enable trasmit and receive
    *R(UART_CR) |= (UART_CR_EN | UART_CR_RXE | UART_CR_TXE);

    // enable FIFO
    *R(UART_LCRH) |= UART_LCRH_FEN;

    cprintf("uartinit done!\n");
}

void uartputc(char c)
{
    *R(UART_DR) = (uint)c;
}

void uartputs(const char* s)
{
    while (*s != '\0')
    {
        *R(UART_DR) = (uint)(*s);
        s++;
    }
}

// enable uart interrupt
void uartsti(void)
{
    *R(UART_IMSC) = UART_RXI;
    intrset(SPI2ID(IRQ_UART), uartintr);

    cprintf("uartintr enabled!\n");
}

// uart interrupt handler
void uartintr(struct trapframe* f, int id, uint32 el)
{
    if (*R(UART_MIS) & UART_RXI)
        uartputc('!');

    *R(UART_ICR) = UART_RXI | UART_TXI;
}