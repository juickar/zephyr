# Copyright (C) 2025 Savoir-faire Linux, Inc.
#
# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(.)

if(CONFIG_CPU_CORTEX_M33)
  add_subdirectory(m33)
elseif(CONFIG_CPU_CORTEX_A35)
  add_subdirectory(a35)
elseif(CONFIG_CPU_CORTEX_A35_SMP)
  add_subdirectory(a35)
endif()
