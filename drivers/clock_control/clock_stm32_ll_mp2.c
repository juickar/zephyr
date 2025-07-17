/*
 * Copyright (C) 2025 Savoir-faire Linux, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <soc.h>
#include <stm32_ll_bus.h>
#include <stm32_ll_rcc.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/drivers/clock_control/stm32_clock_control.h>
#include <zephyr/sys/util.h>

static int stm32_clock_control_on(const struct device *dev, clock_control_subsys_t sub_system)
{
	struct stm32_pclken *pclken = (struct stm32_pclken *) sub_system;

	ARG_UNUSED(dev);

	if (!IN_RANGE(pclken->bus, STM32_CLOCK_PERIPH_MIN, STM32_CLOCK_PERIPH_MAX)) {
		/* Attempt to change a wrong periph clock bit */
		return -ENOTSUP;
	}

	sys_set_bits(DT_REG_ADDR(DT_NODELABEL(rcc)) + pclken->bus, pclken->enr);

	return 0;
}

static int stm32_clock_control_off(const struct device *dev, clock_control_subsys_t sub_system)
{
	struct stm32_pclken *pclken = (struct stm32_pclken *) sub_system;

	ARG_UNUSED(dev);

	if (!IN_RANGE(pclken->bus, STM32_CLOCK_PERIPH_MIN, STM32_CLOCK_PERIPH_MAX)) {
		/* Attempt to toggle a wrong periph clock bit */
		return -ENOTSUP;
	}

	sys_clear_bits(DT_REG_ADDR(DT_NODELABEL(rcc)) + pclken->bus, pclken->enr);

	return 0;
}

static int stm32_clock_control_get_subsys_rate(const struct device *dev,
					       clock_control_subsys_t sub_system, uint32_t *rate)
{
	struct stm32_pclken *pclken = (struct stm32_pclken *)(sub_system);

	ARG_UNUSED(dev);

	switch (pclken->bus) {
	case STM32_CLOCK_PERIPH_USART1:
		*rate = LL_RCC_GetUARTClockFreq(LL_RCC_USART1_CLKSOURCE);
		break;
	case STM32_CLOCK_PERIPH_USART2:
	case STM32_CLOCK_PERIPH_UART4:
		*rate = LL_RCC_GetUARTClockFreq(LL_RCC_UART24_CLKSOURCE);
		break;
	case STM32_CLOCK_PERIPH_USART3:
	case STM32_CLOCK_PERIPH_UART5:
		*rate = LL_RCC_GetUARTClockFreq(LL_RCC_USART35_CLKSOURCE);
		break;
	case STM32_CLOCK_PERIPH_USART6:
		*rate = LL_RCC_GetUARTClockFreq(LL_RCC_USART6_CLKSOURCE);
		break;
	case STM32_CLOCK_PERIPH_UART7:
	case STM32_CLOCK_PERIPH_UART8:
		*rate = LL_RCC_GetUARTClockFreq(LL_RCC_UART78_CLKSOURCE);
		break;
	case STM32_CLOCK_PERIPH_UART9:
		*rate = LL_RCC_GetUARTClockFreq(LL_RCC_UART9_CLKSOURCE);
		break;
	default:
		return -ENOTSUP;
	}

	return 0;
}

static DEVICE_API(clock_control, stm32_clock_control_api) = {
	.on = stm32_clock_control_on,
	.off = stm32_clock_control_off,
	.get_rate = stm32_clock_control_get_subsys_rate,
};

static void set_up_fixed_clock_sources(void)
{
	if (IS_ENABLED(STM32_HSE_ENABLED)) {
		/* Enable HSE */
		LL_RCC_HSE_Enable();
		while (LL_RCC_HSE_IsReady() != 1) {
			/* Wait for HSE ready */
		}
	}

	if (IS_ENABLED(STM32_HSI_ENABLED)) {
		/* Enable HSI */
		LL_RCC_HSI_Enable();
		while (LL_RCC_HSI_IsReady() != 1) {
		/* Wait for HSI ready */
		}
	}
}

static void set_up_plls(void)
{
	uint32_t frefdiv = DT_PROP(DT_NODELABEL(pll1), frefdiv);
	uint32_t fbdiv = DT_PROP(DT_NODELABEL(pll1), fbdiv);
	uint32_t postdiv_1 = DT_PROP(DT_NODELABEL(pll1), postdiv_1);
	uint32_t postdiv_2 = DT_PROP(DT_NODELABEL(pll1), postdiv_2);

#if STM32_PLL_SRC_HSI

	BUILD_ASSERT(IS_ENABLED(STM32_HSI_ENABLED),
		     "STM32MP2 PLL1 requires HSI to be enabled!");

	LL_RCC_PLL1_SetSource(LL_RCC_PLL1SOURCE_HSI);

#elif STM32_PLL_SRC_HSE

	BUILD_ASSERT(IS_ENABLED(STM32_HSE_ENABLED),
		     "STM32MP2 PLL1 requires HSE to be enabled!");

	LL_RCC_PLL1_SetSource(LL_RCC_PLL1SOURCE_HSE);

#endif

	LL_CA35SS_PLL1_Reset_output();

	LL_CA35SS_PLL1_SetFREFDIV(frefdiv);
	LL_CA35SS_PLL1_SetFBDIV(fbdiv);
	LL_CA35SS_PLL1_SetPostDiv1(postdiv_1);
	LL_CA35SS_PLL1_SetPostDiv2(postdiv_2);

	LL_CA35SS_PLL1_Enable();

	while (!LL_CA35SS_PLL1_IsReady()) {
	};

	LL_CA35SS_PLL1_Set_output();
}

static void set_up_stgen(void)
{
#if STM32_STGEN_SRC_HSI

	BUILD_ASSERT(IS_ENABLED(STM32_HSI_ENABLED),
		     "STM32MP2 STGEN requires HSI to be enabled!");

	LL_RCC_EnableSTGEN_KerClk();

	LL_RCC_SetSTGENClockSource(LL_RCC_XBAR_CLKSRC_HSI);

#elif STM32_STGEN_SRC_HSE

	BUILD_ASSERT(IS_ENABLED(STM32_HSE_ENABLED),
		     "STM32MP2 STGEN requires HSE to be enabled!");

	LL_RCC_EnableSTGEN_KerClk();

	LL_RCC_SetSTGENClockSource(LL_RCC_XBAR_CLKSRC_HSE);
#endif
}

static int stm32_clock_control_init(const struct device *dev)
{
	ARG_UNUSED(dev);

#if IS_ENABLED(CONFIG_SOC_STM32MP2X_A35) || IS_ENABLED(CONFIG_SOC_STM32MP2X_A35_SMP)

	LL_CA35SS_SetCA35SSClockSourceExt();

	set_up_fixed_clock_sources();

	set_up_plls();

#if STM32_SYSCLK_SRC_PLL

	LL_CA35SS_SetCA35SSClockSourcePLL1();

#endif
	set_up_stgen();
#endif
	return 0;
}

/**
 * @brief RCC device, note that priority is intentionally set to 1 so
 * that the device init runs just after SOC init
 */
DEVICE_DT_DEFINE(DT_NODELABEL(rcc), stm32_clock_control_init, NULL, NULL, NULL, PRE_KERNEL_1,
		 CONFIG_CLOCK_CONTROL_INIT_PRIORITY, &stm32_clock_control_api);
