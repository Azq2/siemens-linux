#ifndef __ASM_ARCH_MEMORY_H
#define __ASM_ARCH_MEMORY_H

#ifndef CONFIG_MMU
#define PLAT_PHYS_OFFSET		UL(0xA8600000)
#define MEM_SIZE	(3*1024*1024)
#endif

#endif
