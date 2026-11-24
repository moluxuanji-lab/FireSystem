/**
 * @file kzb_early_state.h
 * @brief KZB-F407 板级早期初始化状态快照定义
 *
 * 保存在内核启动最早期（board_early_init_hook）锁存的芯片标识、
 * 复位原因及安全 IO 钳位回读状态，供 MCUboot 钩子、主应用自检及
 * 试启动健康评估模块查询。
 */

#ifndef KZB_EARLY_STATE_H
#define KZB_EARLY_STATE_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief 板级早期初始化阶段采集的硬件状态快照
 */
struct kzb_early_board_state {
    uint32_t rcc_csr;         /**< 复位后早期锁存的 STM32 RCC->CSR 复位原因寄存器值 */
    uint32_t dbgmcu_idcode;   /**< DBGMCU->IDCODE 芯片型号与版本标识码 */
    uint16_t flash_size_kib;  /**< 片上只读系统区记录的内部 Flash 容量（KiB） */
    uint32_t uid_words[3];    /**< STM32F407 96 位唯一设备标识（3 个 32 位字） */
    uint8_t dr1_early;        /**< 继电器输出 DR1（PE5）早期钳位后的回读电平 */
    uint8_t dr2_early;        /**< 继电器输出 DR2（PE6）早期钳位后的回读电平 */
    uint8_t rs485a_de_early;  /**< RS-485 A 路方向脚 DE（PC8）早期钳位后的回读电平 */
    uint8_t rs485b_de_early;  /**< RS-485 B 路方向脚 DE（PF11）早期钳位后的回读电平 */
    uint8_t led_idle_early;   /**< LED 串行发送脚（PG8）早期钳位后的空闲回读电平 */
    bool early_clamp_ok;      /**< 所有安全关键引脚是否全部满足预期安全电平 */
};

/** @brief 全局板级早期状态实例，由 board_early_init_hook() 填充 */
extern struct kzb_early_board_state kzb_early_state;

#endif /* KZB_EARLY_STATE_H */
