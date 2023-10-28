/* Generic Interrupt Controller */
// https://zhuanlan.zhihu.com/p/520133301
#include "types.h"
#include "memlayout.h"
#include "defs.h"
#include "arm.h"
#include "virt.h"

static volatile uint* gic_base;

#define GIC_INT_MAX     64

#define GICC_REG(x)     (*(uint64*)((uint64)gic_base + 0x10000 + x))

// CPU Interface Control Register
// Controls the CPU interface, including enabling of interrupt groups, 
// interrupt signal bypass, binary point registers used, 
// and separation of priority drop and interrupt deactivation.
#define GICC_CTLR       0x000
// CPU Interface Priority Mask Register
// This register provides an interrupt priority filter. 
// Only interrupts with a higher priority than the value in this register are signaled to the PE.
#define GICC_PMR        0x004
// CPU Interface Interrupt Acknowledge Register
// Provides the INTID of the signaled interrupt. 
// A read of this register by the PE acts as an acknowledge for the interrupt.
#define GICC_IAR        0x00c
// CPU Interface End Of Interrupt Register
// A write to this register performs priority drop for the specified interrupt.
#define GICC_EOIR       0x010

#define GICD_REG(x, o)  (*(uint64*)((uint64)gic_base + x + 4 * o))

// GICD_CTLR, Distributor Control Register
// Enables interrupts and affinity routing.
#define GICD_CTLR       0x000
// GICD_ISENABLER<n>, Interrupt Set-Enable Registers, n = 0 - 31
// Enables forwarding of the corresponding interrupt to the CPU interfaces.
#define GICD_ISENABLER  0x100
// Interrupt Clear-Enable Registers, n = 0 - 31
// Disables forwarding of the corresponding interrupt to the CPU interfaces.
#define GICD_ICENABLER  0x180
// GICD_ISPENDR<n>, Interrupt Set-Pending Registers, n = 0 - 31
// Adds the pending state to the corresponding interrupt.
#define GICD_ISPENDR    0x200
// Interrupt Clear-Pending Registers, n = 0 - 31
// Removes the pending state from the corresponding interrupt.
#define GICD_ICPENDR    0x280
// Interrupt Processor Targets Registers, n = 0 - 254
// When affinity routing is not enabled, holds the list of target PEs for the interrupt. 
#define GICD_ITARGETSR  0x800
// GICD_ICFGR<n>, Interrupt Configuration Registers, n = 0 - 63
// Determines whether the corresponding interrupt is edge-triggered or level-sensitive.
#define GICD_ICFGR      0xc00


#define IRQ_MAX_COUNT   64
static intrhandler      intrs[IRQ_MAX_COUNT];

// register ih to irq handler list.
void intrset(int id, intrhandler ih)
{
    //printf("intrset! id: %d\n", id);
    if (id < IRQ_MAX_COUNT)
        intrs[id] = ih;
}

// default irq handler
static void defintr(struct trapframe* f, int id)
{
    printf("unhandled interrupt!\n");
}

static void intrinit()
{
    for (int i = 0; i < IRQ_MAX_COUNT; i++)
    {
        intrs[i] = defintr;
    }
}

static void cfgset(int id, int isedge)
{
    int offset = id / 16;
    int bitpos = (id % 16) * 2;
    uint val = GICD_REG(GICD_ICFGR, offset);
    uint mask = 0x03;
    val &= ~(mask << bitpos);
    if (isedge)
        val |= 0x02 << bitpos;
    GICD_REG(GICD_ICFGR, offset) = val;
}

static void enbset(int id)
{
    //GICD_REG(GICD_ISENABLER, id / 32) |= (1 << (id % 32));
    int offset = id / 32;
    int bitpos = id % 32;
    uint val = GICD_REG(GICD_ISENABLER, offset);
    val |= 1 << bitpos;
    GICD_REG(GICD_ISENABLER, offset) = val;
}

// set target processor
static void cpu0set(int id)
{
    int offset = id / 4;
    int bitpos = (id % 4) * 8;
    uint val = GICD_REG(GICD_ITARGETSR, offset);
    uchar tcpu = 0x01;      // cpu0
    val |= tcpu << bitpos;
    GICD_REG(GICD_ITARGETSR, offset) = val;
}

static void spiset(int spi, int isedge)
{
    int id = SPI2ID(spi);
    cfgset(id, isedge);
    enbset(id);
    cpu0set(id);
}

static void ppiset(int ppi, int isedge)
{
    int id = PPI2ID(ppi);
    cfgset(id, isedge);
    enbset(id);
}

// enable group 0
static void group0enb()
{
    GICD_REG(GICD_CTLR, 0) |= 1;
    GICC_REG(GICC_CTLR) |= 1;
}

/*static void group0dsb()
{
    GICD_REG(GICD_CTLR, 0) &= ~(uint)1;
    GICC_REG(GICC_CTLR) &= ~(uint)1;
}*/

static void cpuinit()
{
    GICC_REG(GICC_PMR) = 0x0f;  /* 16 priority levels */
}

void gicinit(void)
{
    gic_base = (uint*)(P2V(GIC_BASE));

    cpuinit();
    intrinit();

    ppiset(IRQ_TIMER0, 0);
    spiset(IRQ_UART, 1);
    spiset(IRQ_MMIO, 1);

    group0enb();

    //printf("gicinit done!\n");
}

void irqhandle(struct trapframe* f, uint32 el)
{
    int id = GICC_REG(GICC_IAR);
    // ! put this before intrs, or yield might not reach it
    GICC_REG(GICC_EOIR) = id;
    intrs[id](f, id, el);
}