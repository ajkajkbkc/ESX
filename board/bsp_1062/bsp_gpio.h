/**
  ******************************************************************************
  * @file    bsp_gpio.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   I/O 驱动
  ******************************************************************************
  */
#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

#include "fsl_gpio.h"
#include "fsl_common.h"
#include "kalyke_opts.h"

//typedef void (*pFuncRefreshIO)(unsigned short inputNum, unsigned short outputNum); //2022.3.3


// GPIO_B0_10, D9
#define MCU_CFG6_GPIO GPIO2
#define MCU_CFG6_PIN  (10U)
#define MCU_CFG6_MASK (0x00000400U)
#define MCU_CFG6_IRQ  GPIO2_Combined_0_15_IRQn

#define MCU_UART3_DE_GPIO GPIO1
#define MCU_UART3_DE_PIN (17U)
#define MCU_UART3_DE_MASK (0x00020000U)


#define X0_GPIO     GPIO1
#define X0_GPIO_PIN (16U)
#define X0_PIN_MASK (0x00010000U)
#define X0_IRQ      GPIO1_Combined_16_31_IRQn

#define X1_GPIO     GPIO2
#define X1_GPIO_PIN (2U)
#define X1_PIN_MASK (0x00000004U)
#define X1_IRQ      GPIO2_Combined_0_15_IRQn

#define X2_GPIO     GPIO3
#define X2_GPIO_PIN (25U)
#define X2_PIN_MASK (0x02000000U)
#define X2_IRQ      GPIO3_Combined_16_31_IRQn

#define X3_GPIO     GPIO1
#define X3_GPIO_PIN (2U)
#define X3_PIN_MASK (0x00000004U)
#define X3_IRQ      GPIO1_Combined_0_15_IRQn

#define X4_GPIO     GPIO3
#define X4_GPIO_PIN (3U)
#define X4_PIN_MASK (0x00000008U)
#define X4_IRQ      GPIO3_Combined_0_15_IRQn

#define X5_GPIO     GPIO2
#define X5_GPIO_PIN (31U)
#define X5_PIN_MASK (0x80000000U)
#define X5_IRQ      GPIO2_Combined_16_31_IRQn

#define X6_GPIO     GPIO1
#define X6_GPIO_PIN (0U)
#define X6_PIN_MASK (0x00000001U)
#define X6_IRQ      GPIO1_Combined_0_15_IRQn

#define X7_GPIO     GPIO2
#define X7_GPIO_PIN (28U)
#define X7_PIN_MASK (0x10000000U)
#define X7_IRQ      GPIO2_Combined_16_31_IRQn

/////////////////////////////////////////////////////////////
#define Y0_GPIO     GPIO3
#define Y0_GPIO_PIN (14U)
#define Y0_PIN_MASK (0x00004000U)

#define Y1_GPIO     GPIO3
#define Y1_GPIO_PIN (15U)
#define Y1_PIN_MASK (0x00008000U)

#define Y2_GPIO     GPIO3
#define Y2_GPIO_PIN (16U)
#define Y2_PIN_MASK (0x00010000U)

#define Y3_GPIO     GPIO3
#define Y3_GPIO_PIN (17U)
#define Y3_PIN_MASK (0x00020000U)

#define Y4_GPIO     GPIO1
#define Y4_GPIO_PIN (10U)
#define Y4_PIN_MASK (0x00000400U)

#define Y5_GPIO     GPIO1
#define Y5_GPIO_PIN (11U)
#define Y5_PIN_MASK (0x00000800U)

#define Y6_GPIO     GPIO1
#define Y6_GPIO_PIN (9U)
#define Y6_PIN_MASK (0x00000200U)

#define Y7_GPIO     GPIO3
#define Y7_GPIO_PIN (2U)
#define Y7_PIN_MASK (0x00000004U)



/* GPIO_SD_B1_03, M4 */
#define MCU_CARD_UNDEF1_GPIO   GPIO3
#define MCU_CARD_UNDEF1_PIN    (3U)


#define ENET1_RST_GPIO    GPIO2
#define ENET1_RST_PIN     (10U)

#define ENET2_RST_GPIO    GPIO2
#define ENET2_RST_PIN     (11U)


/* For SN74AHC595 */
#define SRCLR_GPIO      GPIO1
#define SRCLR_PIN       (26U)
#define SRCLR_PIN_MASK  (0x04000000U)

#define SRCLK_GPIO      GPIO1
#define SRCLK_PIN       (31U)
#define SRCLK_PIN_MASK  (0x80000000U)

#define RCLK_GPIO       GPIO1
#define RCLK_PIN        (28U)
#define RCLK_PIN_MASK   (0x10000000U)

#define OE_GPIO         GPIO1
#define OE_PIN          (20U)
#define OE_PIN_MASK     (0x00100000U)

#define SER_GPIO        GPIO1
#define SER_PIN         (29U)
#define SER_PIN_MASK    (0x20000000U)

/* For SN74HC165 */
#define CLK_GPIO        SRCLK_GPIO
#define CLK_PIN         SRCLK_PIN
#define CLK_PIN_MASK    SRCLK_PIN_MASK

#define QH_GPIO         GPIO1
#define QH_PIN          (30U)

#define LD_GPIO         GPIO1
#define LD_PIN          (3U)
#define LD_PIN_MASK     (0x00000008U)

/* SN74AHC595 & SN74HC165 共用一个CLK */
#define CLK_SRCLK_GPIO      SRCLK_GPIO
#define CLK_SRCLK_PIN       SRCLK_PIN
#define CLK_SRCLK_PIN_MASK  SRCLK_PIN_MASK

// Low power detect pin
#define LOW_POWER_DETECT_GPIO  GPIO2
#define LOW_POWER_DETECT_PIN   (30U)



extern void bsp_gpio_init(void);
extern void bsp_refresh_input_output_port(unsigned short inputNum, unsigned short outputNum);
extern void bsp_refresh_input_output_port_dm(unsigned short inputNum, unsigned short outputNum);
extern void bsp_kalyke_enable_X_interrupt(uint8_t Xx, gpio_interrupt_mode_t interruptMode);
extern void bsp_kalyke_disable_X_interrupt(uint8_t Xx);
extern void kalyke_Y_output(unsigned char yValue, unsigned char on_off);


//extern pFuncRefreshIO gGPIORefreshIO; //2022.3.3
extern uint32_t gX0IntCount;
#endif /*__BSP_GPIO_H*/

