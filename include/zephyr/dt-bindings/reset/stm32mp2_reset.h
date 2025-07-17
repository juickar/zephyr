/*
 * Copyright (C) 2025 Savoir-faire Linux, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_DT_BINDINGS_RESET_STM32MP2_RESET_H_
#define ZEPHYR_INCLUDE_DT_BINDINGS_RESET_STM32MP2_RESET_H_

/**
 * Pack RCC register offset and bit in one 32-bit value.
 *
 * 5 LSBs are used to keep bit number in 32-bit RCC register.
 * Next 12 bits are used to keep RCC register offset.
 * Remaining bits are unused.
 *
 * @param per STM32 peripheral name
 * @param bit Reset bit
 */
#define STM32_RESET(per, bit) (((STM32_RESET_PERIPH_##per##) << 5U) | (1 << bit))

/* Reset reg */
#define STM32_RST	0U

/** GPIO Peripheral */

#define STM32_RESET_PERIPH_GPIOA	0x52C
#define STM32_RESET_PERIPH_GPIOB	0x530
#define STM32_RESET_PERIPH_GPIOC	0x534
#define STM32_RESET_PERIPH_GPIOD	0x538
#define STM32_RESET_PERIPH_GPIOE	0x53C
#define STM32_RESET_PERIPH_GPIOF	0x540
#define STM32_RESET_PERIPH_GPIOG	0x544
#define STM32_RESET_PERIPH_GPIOH	0x548
#define STM32_RESET_PERIPH_GPIOI	0x54C
#define STM32_RESET_PERIPH_GPIOJ	0x550
#define STM32_RESET_PERIPH_GPIOK	0x554
#define STM32_RESET_PERIPH_GPIOZ	0x558

/* USART/UART Peripheral */
#define STM32_RESET_PERIPH_USART1	0x77C
#define STM32_RESET_PERIPH_USART2	0x780
#define STM32_RESET_PERIPH_USART3	0x784
#define STM32_RESET_PERIPH_UART4	0x788
#define STM32_RESET_PERIPH_UART5	0x78C
#define STM32_RESET_PERIPH_USART6	0x790
#define STM32_RESET_PERIPH_UART7	0x794
#define STM32_RESET_PERIPH_UART8	0x798
#define STM32_RESET_PERIPH_UART9	0x79C

#endif /* ZEPHYR_INCLUDE_DT_BINDINGS_RESET_STM32MP2_RESET_H_ */
