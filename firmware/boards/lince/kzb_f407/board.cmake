# SPDX-License-Identifier: Apache-2.0

board_runner_args(jlink "--device=STM32F407ZE" "--iface=swd" "--speed=1000" "--reset")
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)
