// driver for ARM PrimeCell UART (PL011)
#include "types.h"
#include "memlayout.h"
#include "defs.h"
#include "arm.h"
#include "virt.h"
#include "spinlock.h"

static volatile uint* uart_base = (uint*)UART_BASE;

extern volatile int panicked;

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
#define UART_RXI	    (1 << 4)	// receive interrupt
#define UART_TXI	    (1 << 5)	// transmit interrupt
#define UART_MIS	    16	        // masked interrupt status register
#define	UART_ICR	    17	        // interrupt clear register

#define UART_BITRATE    19200
#define UART_CLK        24000000 

// the transmit output buffer.
struct spinlock uart_tx_lock;
#define UART_TX_BUF_SIZE            32
char uart_tx_buf[UART_TX_BUF_SIZE];
uint64 uart_tx_w; // write next to uart_tx_buf[uart_tx_w % UART_TX_BUF_SIZE]
uint64 uart_tx_r; // read next from uart_tx_buf[uart_tx_r % UART_TX_BUF_SIZE]

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

    initlock(&uart_tx_lock, "uart");

    printf("uartinit done!\n");
}

// add a character to the output buffer and tell the
// UART to start sending if it isn't already.
// blocks if the output buffer is full.
// because it may block, it can't be called
// from interrupts; it's only suitable for use
// by write().
void uartputc(int c)
{
    // *R(UART_DR) = (uint)c;

    acquire(&uart_tx_lock);

    if (panicked >= 0 && panicked != cpuid())
    {
        for (;;);
    }

    while (uart_tx_w == uart_tx_r + UART_TX_BUF_SIZE)
    {
        // buffer is full.
        // wait for uartstart() to open up space in the buffer.
        sleep(&uart_tx_r, &uart_tx_lock);
    }
    uart_tx_buf[uart_tx_w % UART_TX_BUF_SIZE] = c;
    uart_tx_w += 1;
    uartstart();
    release(&uart_tx_lock);
}

// alternate version of uartputc() that doesn't 
// use interrupts, for use by kernel printf() and
// to echo characters. it spins waiting for the uart's
// output register to be empty.
void uartputc_sync(int c)
{
    push_off();

    if (panicked >= 0 && panicked != cpuid())
    {
        for (;;);
    }

    // wait for Transmit Holding Empty to be set in LSR.
    while (*R(UART_FR) & UART_FR_TXFF);

    *R(UART_DR) = c;

    pop_off();
}

// if the UART is idle, and a character is waiting
// in the transmit buffer, send it.
// caller must hold uart_tx_lock.
// ? called from both the top- and bottom-half.
void uartstart(void)
{
    while (1)
    {
        if (uart_tx_w == uart_tx_r)
        {
            // transmit buffer is empty.
            return;
        }

        if (*R(UART_FR) & UART_FR_TXFF)
        {
            // the UART transmit holding register is full,
            // so we cannot give it another byte.
            // it will interrupt when it's ready for a new byte.
            return;
        }

        int c = uart_tx_buf[uart_tx_r % UART_TX_BUF_SIZE];
        uart_tx_r += 1;

        // maybe uartputc() is waiting for space in the buffer.
        wakeup(&uart_tx_r);

        // WriteReg(THR, c);
        *R(UART_DR) = c;
    }
}

// read one input character from the UART.
// return -1 if none is waiting.
int uartgetc(void)
{
    if (*R(UART_FR) & UART_FR_RXFE)
        return -1;

    return *R(UART_DR);
}

void _uartputs(const char* s)
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

    printf("uartintr enabled!\n");
}

// uart interrupt handler
/*void uartintr(struct trapframe* f, int id, uint32 el)
{
    if (*R(UART_MIS) & UART_RXI)
        uartputc('!');

    *R(UART_ICR) = UART_RXI | UART_TXI;
}*/

// handle a uart interrupt, raised because input has
// arrived, or the uart is ready for more output, or
// both. called from irqintr().
void uartintr(struct trapframe* f, int id, uint32 el)
{
    // read and process incoming characters.
    while (*R(UART_MIS) & UART_RXI)
    {
        int c = uartgetc();
        if (c == -1)
            break;
        consoleintr(c);
    }

    // send buffered characters.
    acquire(&uart_tx_lock);
    uartstart();
    release(&uart_tx_lock);

    *R(UART_ICR) = UART_RXI | UART_TXI;
}
