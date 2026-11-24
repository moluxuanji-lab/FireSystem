/**
 * @file board.c
 * @brief KZB-F407 板级早期安全初始化钩子实现
 *
 * 在 Zephyr 驱动框架与主程序运行之前，直接操作 STM32F407 寄存器完成
 * 消防继电器与总线方向引脚的无毛刺安全钳位，并锁存复位原因及芯片只读信息。
 */

#include "kzb_early_state.h"

#include <soc.h>
#include <zephyr/sys/barrier.h>

/** @brief 全局板级早期初始化状态实例 */
struct kzb_early_board_state kzb_early_state;

/*
 * 无毛刺地将指定 GPIO 引脚配置为推挽输出并钳位到目标安全电平。
 *
 * 严格遵循硬件时序：先写 BSRR 预设安全输出电平，再配置推挽、低速与
 * 下拉属性，最后切换 MODER 为通用输出模式，防止切换瞬间产生错误脉冲。
 */
static void clamp_pin_output(GPIO_TypeDef *gpio, uint32_t pin, bool high, bool pull_down)
{
    uint32_t mod_shift = pin * 2u;

    /* 必须在切换 MODER 为输出模式之前写入 BSRR，确保输出端零毛刺 */
    gpio->BSRR = high ? (1u << pin) : (1u << (pin + 16u));
    gpio->OTYPER &= ~(1u << pin);
    gpio->OSPEEDR &= ~(3u << mod_shift);
    gpio->PUPDR &= ~(3u << mod_shift);
    if (pull_down) {
        gpio->PUPDR |= (2u << mod_shift);
    }
    gpio->MODER = (gpio->MODER & ~(3u << mod_shift)) | (1u << mod_shift);
}

/**
 * @brief Zephyr 板级早期初始化钩子
 *
 * 由内核启动流程在外设驱动初始化前调用，完成关键执行机构安全钳位、
 * 复位原因锁存以及芯片唯一标识与 Flash 容量读取。
 */
void board_early_init_hook(void)
{
#ifdef CONFIG_KZB_EARLY_SAFE_IO
    /* 使能 GPIOC/E/F/G 时钟并插入数据同步屏障，确保寄存器访问前时钟已稳定 */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIOEEN |
                    RCC_AHB1ENR_GPIOFEN | RCC_AHB1ENR_GPIOGEN;
    __DSB();

    /*
     * 钳位安全关键引脚：
     * - 继电器线圈输出 DR1（PE5）、DR2（PE6）置低并使能下拉，防止上电误动作
     * - RS-485 A/B 发送使能 DE（PC8、PF11）置低并下拉，强制进入静默接收态
     * - LED 串行发送脚（PG8）置高，保持空闲态电平
     */
    clamp_pin_output(GPIOE, 5u, false, true);
    clamp_pin_output(GPIOE, 6u, false, true);
    clamp_pin_output(GPIOC, 8u, false, true);
    clamp_pin_output(GPIOF, 11u, false, true);
    clamp_pin_output(GPIOG, 8u, true, false);
    __DSB();

    /* 立即回读 IDR 寄存器校验物理引脚电平是否符合安全预期 */
    kzb_early_state.dr1_early = (uint8_t)((GPIOE->IDR >> 5u) & 1u);
    kzb_early_state.dr2_early = (uint8_t)((GPIOE->IDR >> 6u) & 1u);
    kzb_early_state.rs485a_de_early = (uint8_t)((GPIOC->IDR >> 8u) & 1u);
    kzb_early_state.rs485b_de_early = (uint8_t)((GPIOF->IDR >> 11u) & 1u);
    kzb_early_state.led_idle_early = (uint8_t)((GPIOG->IDR >> 8u) & 1u);
    kzb_early_state.early_clamp_ok =
        (kzb_early_state.dr1_early == 0u &&
         kzb_early_state.dr2_early == 0u &&
         kzb_early_state.rs485a_de_early == 0u &&
         kzb_early_state.rs485b_de_early == 0u &&
         kzb_early_state.led_idle_early == 1u);
#endif

    /*
     * 早期锁存复位原因标志；仅在应用固件中清除 RMVF，
     * 使 MCUboot 阶段保留硬件复位标志供跳转后的应用固件完整读取。
     */
    kzb_early_state.rcc_csr = RCC->CSR;
#ifndef CONFIG_MCUBOOT
    RCC->CSR |= RCC_CSR_RMVF;
#endif

    /* 读取 STM32F407 调试 ID 与系统只读区（Flash 容量及 96 位 UID） */
    kzb_early_state.dbgmcu_idcode = DBGMCU->IDCODE;
    kzb_early_state.flash_size_kib = *(const volatile uint16_t *)0x1FFF7A22U;
    kzb_early_state.uid_words[0] = *(const volatile uint32_t *)0x1FFF7A10U;
    kzb_early_state.uid_words[1] = *(const volatile uint32_t *)0x1FFF7A14U;
    kzb_early_state.uid_words[2] = *(const volatile uint32_t *)0x1FFF7A18U;
}
