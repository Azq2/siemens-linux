#include <linux/module.h>

#include <asm/irq.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <linux/platform_device.h>
#include <asm/mach-types.h>
#include <asm/setup.h>
#include <asm/tlbflush.h>

/*
 *
    [SIEMENS:EL71:45]
    Name = Siemens EL71 fw45
    BootROMaddress = 0x400000;
    BootROMsize = 0x100000;
    FlashAddress = 0xa0000000;
    FlashSize = 0x04000000;
    IntRAM1address = 0x0;
    IntRAM1size = 0x4000;
    IntRAM2address = 0x80000;
    IntRAM2size = 0x18000;
    ExtRAMaddress = 0xa8000000;
    ExtRAMsize = 0x01000000;
    VMalloc1address=0xaA000000
    VMalloc1size=0x01000000
    VMalloc2address=0xaB000000
    VMalloc2size=0x01000000
    VMalloc3address=0xaC000000
    VMalloc3size=0x01000000
    VMalloc4address=0xaD000000
    VMalloc4size=0x01000000
    MiscSpaceaddress=0xc0000000
    MiscSpacesize=0x00100000
    IOaddress = 0xf0000000;
    IOsize =    0x10000000;
    UseRAM = 0x89000;
    MallocAddress = 0xA0092F51;
    MallocPages = 300;

*/


/* the base address of IO of PMB8876 */
#define PMB8876_IO_BASE 0xf0000000

/* for present time this value is enough */
#define PMB8876_IO_SIZE (0xf5000000 - 0xf0000000)

/* the second watchdog in external place. TODO sleep it */
#define SIEMENS_EL71_EXT_WATCHDOG    0xf4300118



/* this variable is used in head.S (saving r12 register from chaos-boot) */
unsigned long last_watchdog_time = 0;


static void pmb8876_switch_watchdog(void)
{
    unsigned int r2 = *(unsigned int *)(SIEMENS_EL71_EXT_WATCHDOG);
    unsigned int r0 = r2 << 22;
    r0 = r0 >> 31;
    r0 = ~r0;
    r0 = r0 & 1;
    r0 = r0 << 9;

    r2 = r2 & ~0x200;

    *(unsigned int *)(SIEMENS_EL71_EXT_WATCHDOG) = r0 | r2;
}


int pmb8876_serve_watchdog(void)
{
    unsigned int now = *(unsigned int *)(0xf4b00020);

    if (now - last_watchdog_time < 0x200)
        return 0;

    pmb8876_switch_watchdog();

    last_watchdog_time = now;
    return 1;
}



static struct map_desc pmb8876_io_desc[] __initdata __maybe_unused = {
    {
        .virtual	= PMB8876_IO_BASE,
        .pfn		= __phys_to_pfn(PMB8876_IO_BASE),
        .length		= PMB8876_IO_SIZE,
        .type		= MT_DEVICE
    }
};

static void __init pmb8876_map_io(void)
{
    iotable_init(pmb8876_io_desc, ARRAY_SIZE(pmb8876_io_desc));
}


static void pmb8876_board_init(void)
{
    printk("%s(): HELLO BLJAD (.)(.)\n", __func__);
}


static void pmb8876_init_irq(void)
{

}


MACHINE_START(PMB8876, "PMB8876")
    .map_io         = pmb8876_map_io,
    .init_machine   = pmb8876_board_init,
    .init_irq		= pmb8876_init_irq,
    .nr_irqs		= 0
MACHINE_END