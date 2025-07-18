/*
 * Copyright (c) 2025 STMicroelectronics
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief System/hardware module for STM32MP25 processor
 */

#include <zephyr/arch/arm/mmu/arm_mmu.h>
#include <zephyr/drivers/interrupt_controller/gic.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/linker/linker-defs.h>

#include <stm32_ll_bus.h>
#include <cmsis_core.h>

#define VECTOR_ADDRESS ((uintptr_t)_vector_start)

#define SMC_CPU_ON_AARCH32	0x84000003
#define SMC_CPU_1_MPDIR		0x84000001
#define GICC_CTLR_FIQ_EN	0x3

void relocate_vector_table(void)
{
	write_sctlr(read_sctlr() & ~HIVECS);
	write_vbar(VECTOR_ADDRESS & VBAR_MASK);
	barrier_isync_fence_full();
}

/**
 * @brief Perform basic hardware initialization at boot.
 *
 * This needs to be run from the very beginning.
 * So the init priority has to be 0 (zero).
 *
 * @return 0
 */

int soc_per_core_init_hook(void)
{
	/* set the PPI's to group 0 */
	sys_write32(0, GICD_IGROUPRn);

	int val = sys_read32(GICC_CTLR);

	/* disable the FIQ */
	val &= ~(1 << GICC_CTLR_FIQ_EN);
	sys_write32(val, GICC_CTLR);

	return 0;
}

int soc_early_init_hook(void)
{

#ifdef CONFIG_SMP

	extern void z_arm_reset(void);
	uintptr_t cpu1_entry = (uintptr_t)z_arm_reset;

	__asm__ volatile (
		"mov r0, %[smc_cpu_on_aarch32]	\n"
		"mov r1, %[smc_cpu_1_mpdir]	\n"
		"mov r2, %[cpu1_entry]		\n"
		"mov r3, #0			\n"
		"smc #0				\n"
		:
		: [smc_cpu_on_aarch32] "r" (SMC_CPU_ON_AARCH32),
		[smc_cpu_1_mpdir] "r" (SMC_CPU_1_MPDIR),
		[cpu1_entry] "r" (cpu1_entry)
		: "r0", "r1", "r2", "r3", "memory"
	);

#endif
	relocate_vector_table();
	write_sctlr(read_sctlr() & ~SCTLR_TE_Msk);
	barrier_isync_fence_full();

	return 0;
}

static const struct arm_mmu_region mmu_regions[] = {
	MMU_REGION_FLAT_ENTRY("APB1", 0x40000000, 0x1D0400, MPERM_R | MPERM_W | MT_DEVICE),
	MMU_REGION_FLAT_ENTRY("APB2", 0x40200000, 0x180400, MPERM_R | MPERM_W | MT_DEVICE),
	MMU_REGION_FLAT_ENTRY("AHB2", 0x40400000, 0x100400, MPERM_R | MPERM_W | MT_DEVICE),
	MMU_REGION_FLAT_ENTRY("AHB3", 0x42000000, 0x141000, MPERM_R | MPERM_W | MT_DEVICE),
	MMU_REGION_FLAT_ENTRY("APB3", 0x44000000, 0x90400, MPERM_R | MPERM_W | MT_DEVICE),
	MMU_REGION_FLAT_ENTRY("AHB4", 0x44200000, 0xE0400, MPERM_R | MPERM_W | MT_DEVICE),
	MMU_REGION_FLAT_ENTRY("SmartRun_APB", 0x46000000, 0xA0400, MPERM_R | MPERM_W | MT_DEVICE),
	MMU_REGION_FLAT_ENTRY("SmartRun_AHB", 0x46200000, 0x50400, MPERM_R | MPERM_W | MT_DEVICE),
	MMU_REGION_FLAT_ENTRY("APB4", 0x48000000, 0xE0800, MPERM_R | MPERM_W | MT_DEVICE),
	MMU_REGION_FLAT_ENTRY("AHB5", 0x48200000, 0xD00000, MPERM_R | MPERM_W | MT_DEVICE),
	MMU_REGION_FLAT_ENTRY("APB_DBG", 0x4A000000, 0x350000, MPERM_R | MPERM_W | MT_DEVICE),
	MMU_REGION_FLAT_ENTRY("AXIM", 0x4A800000, 0x462000, MPERM_R | MPERM_W | MT_DEVICE),
	MMU_REGION_FLAT_ENTRY("AHB6_1", 0x4B000000, 0xC0000, MPERM_R | MPERM_W | MT_DEVICE),
	MMU_REGION_FLAT_ENTRY("AHB6_2", 0x4C000000, 0x1000000, MPERM_R | MPERM_W | MT_DEVICE),
	MMU_REGION_FLAT_ENTRY("vectors", 0x82000000, 0x1000, MPERM_R | MPERM_X | MT_NORMAL),
};

const struct arm_mmu_config mmu_config = {
	.num_regions = ARRAY_SIZE(mmu_regions),
	.mmu_regions = mmu_regions,
};
