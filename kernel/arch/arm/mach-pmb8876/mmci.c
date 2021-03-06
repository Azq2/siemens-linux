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
#include <linux/amba/mmci.h>
#include <linux/mmc/host.h>
#include <linux/clk-provider.h>
#include <linux/clkdev.h>

static int mmc_handle_ios(struct device *dev, struct mmc_ios *ios) {
	gpio_set_value(GPIO_MMC_VCC_EN, ios->power_mode == MMC_POWER_OFF ? 0 : 1);
	return 0;
}
static struct mmci_platform_data mmci_data = {
	.ocr_mask		= MMC_VDD_29_30, 
	.ios_handler    = mmc_handle_ios, 
	.gpio_cd		= -1, 
	.gpio_wp		= -1
};

static AMBA_AHB_DEVICE(mmc0, "pmb8876:mmc0", 0x00041180, 0xF7301000, {PMB8876_MMCI_IRQ}, &mmci_data);


static int __init pmb8876_mmci_init(void) {
	struct clk *clk_mmci;
	
	clk_mmci = clk_register_fixed_rate(NULL, "clk_mmci", NULL, 0, 6500000);
	clk_register_clkdev(clk_mmci, NULL, "pmb8876:mmc0");
	
	gpio_request(GPIO_MMC_CD, "MMCI_CD");
	gpio_request(GPIO_MMC_CLK, "MMCI_CLK");
	gpio_request(GPIO_MMC_DAT, "MMCI_DAT");
	gpio_request(GPIO_MMC_CMD, "MMCI_CMD");
	gpio_request(GPIO_MMC_VCC_EN, "MMCI_VCC_EN");
	
	writel(0x400, (void *)0xF7300000);
	pmb8876_set_irq_priority(PMB8876_MMCI_IRQ, 1);
	
	amba_device_register(&mmc0_device, &iomem_resource);
	return 0;
}
arch_initcall(pmb8876_mmci_init);
