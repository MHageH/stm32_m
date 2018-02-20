/****************************************************************************
 * configs/stm32_stroboscopic/include/board.h
 * include/arch/board/board.h
 *
 *   Copyright (C) 2017 Gregory Nutt. All rights reserved.
 *   Author: Mateusz Szafoni <raiden00@railab.me>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#ifndef __CONFIG_STM32_STROBOSCOPIC_INCLUDE_BOARD_H
#define __CONFIG_STM32_STROBOSCOPIC_INCLUDE_BOARD_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#ifndef __ASSEMBLY__
#  include <stdint.h>
#  include <stdbool.h>
#endif

#ifdef __KERNEL__
#  include "stm32.h"
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Clocking *****************************************************************/

/* HSI - Internal 8 MHz RC Oscillator
 * LSI - 32 KHz RC
 * HSE - 8 MHz from MCO output of ST-LINK
 * LSE - 32.768 kHz
 */

#define STM32_BOARD_XTAL        8000000ul

#define STM32_HSI_FREQUENCY     8000000ul
#define STM32_LSI_FREQUENCY     32000            /* Between 30kHz and 60kHz */
#define STM32_HSE_FREQUENCY     STM32_BOARD_XTAL
#define STM32_LSE_FREQUENCY     32768            /* X2 on board */

/* PLL source is HSE/1, PLL multipler is 9: PLL frequency is 8MHz (XTAL) x 9 = 72MHz */

#define STM32_CFGR_PLLSRC       RCC_CFGR_PLLSRC
#define STM32_CFGR_PLLXTPRE     0
#define STM32_CFGR_PLLMUL       RCC_CFGR_PLLMUL_CLKx9
#define STM32_PLL_FREQUENCY     (9*STM32_BOARD_XTAL)

/* Use the PLL and set the SYSCLK source to be the PLL */

#define STM32_SYSCLK_SW         RCC_CFGR_SW_PLL
#define STM32_SYSCLK_SWS        RCC_CFGR_SWS_PLL
#define STM32_SYSCLK_FREQUENCY  STM32_PLL_FREQUENCY

/* AHB clock (HCLK) is SYSCLK (72MHz) */

#define STM32_RCC_CFGR_HPRE     RCC_CFGR_HPRE_SYSCLK
#define STM32_HCLK_FREQUENCY    STM32_PLL_FREQUENCY
#define STM32_BOARD_HCLK        STM32_HCLK_FREQUENCY

/* APB2 clock (PCLK2) is HCLK (72MHz) */

#define STM32_RCC_CFGR_PPRE2    RCC_CFGR_PPRE2_HCLK
#define STM32_PCLK2_FREQUENCY   STM32_HCLK_FREQUENCY
#define STM32_APB2_CLKIN        (STM32_PCLK2_FREQUENCY)

/* APB2 timers 1, 8, 15-17 and HRTIM1 will receive PCLK2. */

/* Timers driven from APB2 will be PCLK2 */

#define STM32_APB2_TIM1_CLKIN     (STM32_PCLK2_FREQUENCY)
#define STM32_APB2_TIM8_CLKIN     (STM32_PCLK2_FREQUENCY)
#define STM32_APB1_TIM15_CLKIN    (STM32_PCLK2_FREQUENCY)
#define STM32_APB1_TIM16_CLKIN    (STM32_PCLK2_FREQUENCY)
#define STM32_APB1_TIM17_CLKIN    (STM32_PCLK2_FREQUENCY)
#define STM32_APB1_THRTIM1_CLKIN  (STM32_PCLK2_FREQUENCY)

/* APB1 clock (PCLK1) is HCLK/2 (36MHz) */

#define STM32_RCC_CFGR_PPRE1    RCC_CFGR_PPRE1_HCLKd2
#define STM32_PCLK1_FREQUENCY   (STM32_HCLK_FREQUENCY/2)

/* APB1 timers 2-7 will be twice PCLK1 */

#define STM32_APB1_TIM2_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM3_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM6_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM7_CLKIN   (2*STM32_PCLK1_FREQUENCY)

/* Timer Frequencies, if APBx is set to 1, frequency is same to APBx
 * otherwise frequency is 2xAPBx.
 * Note: TIM1,8 are on APB2, others on APB1
 */

#define BOARD_TIM1_FREQUENCY    STM32_HCLK_FREQUENCY
#define BOARD_TIM15_FREQUENCY   STM32_HCLK_FREQUENCY
#define BOARD_TIM16_FREQUENCY   STM32_HCLK_FREQUENCY
#define BOARD_TIM17_FREQUENCY   STM32_HCLK_FREQUENCY
#define BOARD_TIM2_FREQUENCY    (STM32_HCLK_FREQUENCY / 2)
#define BOARD_TIM3_FREQUENCY    (STM32_HCLK_FREQUENCY / 2)
#define BOARD_TIM5_FREQUENCY    (STM32_HCLK_FREQUENCY / 2)
#define BOARD_TIM6_FREQUENCY    (STM32_HCLK_FREQUENCY / 2)
#define BOARD_TIM7_FREQUENCY    (STM32_HCLK_FREQUENCY / 2)
#define BOARD_HRTIM1_FREQUENCY  STM32_HCLK_FREQUENCY

/* LED definitions **********************************************************/

/* LED index values for use with board_userled() */

#define BOARD_LED1       0
#define BOARD_LED2       1
#define BOARD_LED3       2
#define BOARD_LED4       3
#define BOARD_NLEDS      4

/* LED bits for use with board_userled_all() */

#define BOARD_LED1_BIT   (1 << BOARD_LED1)
#define BOARD_LED2_BIT   (1 << BOARD_LED2)
#define BOARD_LED3_BIT   (1 << BOARD_LED3)
#define BOARD_LED4_BIT   (1 << BOARD_LED4)

/* If CONFIG_ARCH_LEDs is defined, then NuttX will control the 4 LEDs on board the
 * stm32f334-disco.  The following definitions describe how NuttX controls the LEDs:
 */

#define LED_STARTED       0  /* LED1 */
#define LED_HEAPALLOCATE  1  /* LED2 */
#define LED_IRQSENABLED   2  /* LED1 + LED2 */
#define LED_STACKCREATED  3  /* LED3 */
#define LED_INIRQ         4  /* LED1 + LED3 */
#define LED_SIGNAL        5  /* LED2 + LED3 */
#define LED_ASSERTION     6  /* LED1 + LED2 + LED3 */
#define LED_PANIC         7  /* N/C  + N/C  + N/C + LED4 */

/* Button definitions *******************************************************/
/* The STM32F334-DISCO supports two buttons; only one button is controllable
 * by software:
 *
 *   B1 USER:  user button connected to the I/O PA0 of the STM32F334R8.
 *   B2 RESET: push button connected to NRST is used to RESET the
 *             STM32F334R8.
 */

#define BUTTON_USER      0
#define NUM_BUTTONS      1

#define BUTTON_USER_BIT  (1 << BUTTON_USER)

/* Alternate function pin selections ****************************************/
/* CAN */

#define GPIO_CAN1_RX GPIO_CAN_RX_2
#define GPIO_CAN1_TX GPIO_CAN_TX_2

/* I2C */

#define GPIO_I2C1_SCL GPIO_I2C1_SCL_3
#define GPIO_I2C1_SDA GPIO_I2C1_SDA_3

/* SPI */

#define GPIO_SPI1_MISO GPIO_SPI1_MISO_1
#define GPIO_SPI1_MOSI GPIO_SPI1_MOSI_1
#define GPIO_SPI1_SCK GPIO_SPI1_SCK_1

/* TIM */

#define GPIO_TIM2_CH2OUT GPIO_TIM2_CH2OUT_2
#define GPIO_TIM2_CH3OUT GPIO_TIM2_CH3OUT_3

#define GPIO_TIM3_CH1OUT GPIO_TIM3_CH1OUT_2
#define GPIO_TIM3_CH2OUT GPIO_TIM3_CH2OUT_4

#define GPIO_TIM4_CH1OUT GPIO_TIM4_CH1OUT_2

/* USART */

#define GPIO_USART2_RX GPIO_USART2_RX_3 /* PB4 */
#define GPIO_USART2_TX GPIO_USART2_TX_3 /* PB3 */

// Mohamed :: Mod_

#define GPIO_USART1_RX GPIO_USART1_RX_1 // PA10
#define GPIO_USART1_TX GPIO_USART1_TX_1 // PA9

/* Board configuration for powerled example:
 *   - Set HRTIM TIMC output 1 (PB12) on PERIOD.
 *   - Reset HRTIM TIMC output 1 on HRTIM EEV2.
 *   - HRTIM EEV2 is connected to COMP4 output which works as current limit.
 *   - COMP4 inverting input is connected to DAC1CH1 output.
 *   - COMP4 non-inverting input (PB1) is connceted to current sense
 *     resitor (1 Ohm).
 *   - DAC1CH1 DMA transfer is triggered by HRTIM TIMC events, which is used
 *     to provide slope compensation.
 */

#if defined(CONFIG_EXAMPLES_POWERLED)

/* Comparators configuration ************************************************/

#define COMP4_INM  COMP_INMSEL_DAC1CH1

/* HRTIM configuration ******************************************************/

#define HRTIM_TIMC_PRESCALER      HRTIM_PRESCALER_1
#define HRTIM_TIMC_MODE           HRTIM_MODE_CONT
#define HRTIM_TIMC_DMA            (HRTIM_DMA_REP|HRTIM_DMA_CMP1|HRTIM_DMA_CMP2| \
                                  HRTIM_DMA_CMP3|HRTIM_DMA_CMP4)
#define HRTIM_TIMC_CH1_SET        HRTIM_OUT_SET_PER
#define HRTIM_TIMC_CH1_RST        HRTIM_OUT_RST_EXTEVNT2
#define HRTIM_TIMC_CH1_IDLE_STATE HRTIM_IDLE_INACTIVE

#define HRTIM_EEV_SAMPLING        HRTIM_EEV_SAMPLING_d1
#define HRTIM_EEV2_SRC            HRTIM_EEV_SRC_ANALOG
#define HRTIM_EEV2_FILTER         HRTIM_EEV_DISABLE
#define HRTIM_EEV2_POL            HRTIM_EEV_POL_HIGH
#define HRTIM_EEV2_SEN            HRTIM_EEV_SEN_LEVEL
#define HRTIM_EEV2_MODE           HRTIM_EEV_MODE_FAST

#define HRTIM_BURST_CLOCK         HRTIM_BURST_CLOCK_HRTIM
#define HRTIM_BURST_PRESCALER     HRTIM_BURST_PRESCALER_1
#define HRTIM_BURST_TRIGGERS      0

/* DMA channels *************************************************************/

/* DAC */

#define DAC1CH1_DMA_CHAN     DMACHAN_HRTIM1_C

#endif  /* CONFIG_EXAMPLES_POWERLED */

/* Board configuration for SMPS example:
 *   PA8  - HRTIM_CHA1
 *   PA9  - HRTIM_CHA2
 *   PA10 - HRTIM_CHB1
 *   PA11 - HRTIM_CHB2
 *   VIN  - ADC Channel 2 (PA1)
 *   VOUT - ADC Channel 4 (PA3)
 */

#if defined(CONFIG_EXAMPLES_SMPS)

/* ADC configuration ******************************************************/

#define ADC1_INJECTED_CHAN 2
#define ADC1_EXTSEL_VALUE 0
#define ADC1_SMP2 ADC_SMPR_61p5
#define ADC1_SMP4 ADC_SMPR_61p5

/* HRTIM configuration ******************************************************/

/* Timer A configuration - Buck operations */

#define HRTIM_TIMA_PRESCALER HRTIM_PRESCALER_1
#define HRTIM_TIMA_MODE      HRTIM_MODE_CONT

#define HRTIM_TIMA_CH1_SET   HRTIM_OUT_SET_NONE
#define HRTIM_TIMA_CH1_RST   HRTIM_OUT_RST_NONE
#define HRTIM_TIMA_CH2_SET   HRTIM_OUT_SET_NONE
#define HRTIM_TIMA_CH2_RST   HRTIM_OUT_RST_NONE

#define HRTIM_TIMA_DT_FSLOCK HRTIM_DT_LOCK
#define HRTIM_TIMA_DT_RSLOCK HRTIM_DT_LOCK
#define HRTIM_TIMA_DT_FVLOCK HRTIM_DT_RW
#define HRTIM_TIMA_DT_RVLOCK HRTIM_DT_RW
#define HRTIM_TIMA_DT_FSIGN  HRTIM_DT_SIGN_POSITIVE
#define HRTIM_TIMA_DT_RSIGN  HRTIM_DT_SIGN_POSITIVE
#define HRTIM_TIMA_DT_PRESCALER HRTIM_DEADTIME_PRESCALER_1

/* Timer B configuration - Boost operations */

#define HRTIM_TIMB_PRESCALER HRTIM_PRESCALER_1
#define HRTIM_TIMB_MODE      HRTIM_MODE_CONT

#define HRTIM_TIMB_CH1_SET   HRTIM_OUT_SET_NONE
#define HRTIM_TIMB_CH1_RST   HRTIM_OUT_RST_NONE
#define HRTIM_TIMB_CH2_SET   HRTIM_OUT_SET_NONE
#define HRTIM_TIMB_CH2_RST   HRTIM_OUT_RST_NONE

#define HRTIM_TIMB_DT_FSLOCK HRTIM_DT_LOCK
#define HRTIM_TIMB_DT_RSLOCK HRTIM_DT_LOCK
#define HRTIM_TIMB_DT_FVLOCK HRTIM_DT_RW
#define HRTIM_TIMB_DT_RVLOCK HRTIM_DT_RW
#define HRTIM_TIMB_DT_FSIGN  HRTIM_DT_SIGN_POSITIVE
#define HRTIM_TIMB_DT_RSIGN  HRTIM_DT_SIGN_POSITIVE
#define HRTIM_TIMB_DT_PRESCALER HRTIM_DEADTIME_PRESCALER_1

#define HRTIM_ADC_TRG2       HRTIM_ADCTRG24_AC4

/* DMA channels *************************************************************/

#endif  /* CONFIG_EXAMPLES_SMPS */

#endif /* __CONFIG_STM32_STROBOSCOPIC_INCLUDE_BOARD_H */