/* linker.ld - Linker command/script file */

/*
 * Copyright (c) 2025 STMicroelectronics
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include <zephyr/arch/arm/cortex_a_r/scripts/linker.ld>

SECTIONS
    {

#include <zephyr/linker/rel-sections.ld>

#ifdef CONFIG_LLEXT
#include <zephyr/linker/llext-sections.ld>
#endif

}