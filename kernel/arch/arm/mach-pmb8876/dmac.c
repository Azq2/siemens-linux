#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/syscore_ops.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/of_device.h>
#include <linux/gpio.h>
#include <linux/bitops.h>
#include <mach/pmb8876-gpio.h>
#include <mach/irqs.h>
#include <linux/amba/bus.h>
#include <linux/amba/pl08x.h>
#include <linux/amba/pl080.h>
#include <linux/clk-provider.h>
#include <linux/clkdev.h>

static spinlock_t lock = __SPIN_LOCK_UNLOCKED(x);

static int pl08x_get_xfer_signal(const struct pl08x_channel_data *cd) {
	unsigned int signal = cd->min_signal, val;
	unsigned long flags;
	
	spin_lock_irqsave(&lock, flags);
	val = readl((void *) 0xF4400084);
	val |= 1 << signal;
	writel(val, (void *) 0xF4400084);
	spin_unlock_irqrestore(&lock, flags);
	
	return signal;
}

static void pl08x_put_xfer_signal(const struct pl08x_channel_data *cd, int signal) {
	unsigned int val;
	unsigned long flags;
	
	spin_lock_irqsave(&lock, flags);
	val = readl((void *) 0xF4400084);
	val &= ~(1 << signal);
	writel(val, (void *) 0xF4400084);
	spin_unlock_irqrestore(&lock, flags);
}

static struct pl08x_channel_data pmb8876_dma0_info[] = {
	{
		.bus_id = "mmci0_tx",
		.min_signal = 13,
		.max_signal = 13,
		.periph_buses = PL08X_AHB2,
	}, 
	{
		.bus_id = "mmci0_rx",
		.min_signal = 6,
		.max_signal = 6,
		.periph_buses = PL08X_AHB2
	}
};

static const struct dma_slave_map pmb8876_dma0_slave_map[] = {
	{ "pmb8876:mmc0", "tx", &pmb8876_dma0_info[0] },
	{ "pmb8876:mmc0", "rx", &pmb8876_dma0_info[1] },
};

struct pl08x_platform_data dma0_plat_data = {
	.lli_buses = PL08X_AHB2,
	.mem_buses = PL08X_AHB2,
	.slave_channels = pmb8876_dma0_info,
	.num_slave_channels = ARRAY_SIZE(pmb8876_dma0_info),
	.slave_map = pmb8876_dma0_slave_map,
	.slave_map_len = ARRAY_SIZE(pmb8876_dma0_slave_map),
	.get_xfer_signal = pl08x_get_xfer_signal,
	.put_xfer_signal = pl08x_put_xfer_signal,
};

#define ___BRACES(...) {__VA_ARGS__}

static AMBA_APB_DEVICE(dma0, "pmb8876:dma0", 0x00041080, 0xF3000000, ___BRACES(0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B), &dma0_plat_data);

static int __init pmb8876_dma_init(void) {
	int i;
	
	pr_info("pmb8876_dma_init - start\n");
	
	writel(1 << 6, (void *)0xF4400084);
	for (i = 0x23; i <= 0x2B; ++i)
		pmb8876_set_irq_priority(i, 1);
	
	amba_device_register(&dma0_device, &iomem_resource);
	
	pr_info("pmb8876_dma_init - OK\n");
	return 0;
}
arch_initcall(pmb8876_dma_init);
