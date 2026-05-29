/**
  ******************************************************************************
  * @file    bsp_led.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   LED Çý¶¯
  ******************************************************************************
  */
#ifndef __BSP_LED_H
#define __BSP_LED_H
#include "FreeRTOS.h"
#include "board.h"
#include "fsl_gpio.h"
#include "kalyke_opts.h"

#if 1
#define LED_1 GPIO1 // MCU_LED_RUN
#define LED_1_PIN (15U)
#define LED_1_PIN_MASK (0x00008000)
#define LED_RUN  LED_1
#define LED_RUN_PIN_MASK LED_1_PIN_MASK


//#define LED_2 GPIO1 // MCU_LED_NET1
//#define LED_2_PIN (21U)
//#define LED_2_PIN_MASK (0x00200000)
//#define LED_NET1  LED_2
//#define LED_NET1_PIN_MASK LED_2_PIN_MASK

//#define LED_3 GPIO1 // MCU_LED_NET2
//#define LED_3_PIN (25U)
//#define LED_3_PIN_MASK (0x02000000)
//#define LED_NET2  LED_3
//#define LED_NET2_PIN_MASK LED_3_PIN_MASK

#define LED_4 GPIO1 // MCU_LED_ERR
#define LED_4_PIN (14U)
#define LED_4_PIN_MASK (0x00004000)
#define LED_ERR  LED_4
#define LED_ERR_PIN_MASK LED_4_PIN_MASK

#define LED_5 GPIO2 // MCU_LED_UNDEF
#define LED_5_PIN (28U)
#define LED_5_PIN_MASK (0x10000000)
#define LED_UNDEF  LED_5
#define LED_UNDEF_PIN_MASK LED_5_PIN_MASK
#else
#define LED_1 GPIO1 // MCU_LED_RUN
#define LED_1_PIN (28U)
#define LED_1_PIN_MASK (0x10000000)
#define LED_RUN  LED_1
#define LED_RUN_PIN_MASK LED_1_PIN_MASK


#define LED_2 GPIO1 // MCU_LED_NET1
#define LED_2_PIN (21U)
#define LED_2_PIN_MASK (0x00200000)
#define LED_NET1  LED_2
#define LED_NET1_PIN_MASK LED_2_PIN_MASK

#define LED_3 GPIO1 // MCU_LED_NET2
#define LED_3_PIN (25U)
#define LED_3_PIN_MASK (0x02000000)
#define LED_NET2  LED_3
#define LED_NET2_PIN_MASK LED_3_PIN_MASK

#define LED_4 GPIO1 // MCU_LED_ERR
#define LED_4_PIN (20U)
#define LED_4_PIN_MASK (0x00100000)
#define LED_ERR  LED_4
#define LED_ERR_PIN_MASK LED_4_PIN_MASK

#define LED_5 GPIO1 // MCU_LED_UNDEF
#define LED_5_PIN (24U)
#define LED_5_PIN_MASK (0x01000000)
#define LED_UNDEF  LED_5
#define LED_UNDEF_PIN_MASK LED_5_PIN_MASK
#endif


#define LED_ALL_PIN_MASK (LED_1_PIN_MASK|LED_2_PIN_MASK|LED_3_PIN_MASK|LED_4_PIN_MASK|LED_5_PIN_MASK)


#define UART0_A_LED_VALUE   0x00000001
#define UART0_A_LED_MASK    0x00000001

#define UART0_B_LED_VALUE   0x00000002
#define UART0_B_LED_MASK    0x00000002

#define UART1_A_LED_VALUE   0x00000008
#define UART1_A_LED_MASK    0x00000008

#define UART1_B_LED_VALUE   0x00000010
#define UART1_B_LED_MASK    0x00000010

static inline void bsp_toggle_led_RUN(void)
{
    GPIO_PortToggle(LED_RUN, LED_RUN_PIN_MASK);
}

static inline void bsp_toggle_led_ERR(void)
{
    GPIO_PortToggle(LED_ERR, LED_ERR_PIN_MASK);
}

//static inline void bsp_toggle_led_net2(void)
//{
//    GPIO_PortToggle(LED_NET2, LED_NET2_PIN_MASK);
//}
//static inline void bsp_open_led_net2(void)
//{
//    LED_NET2->DR_CLEAR = LED_NET2_PIN_MASK;
//}
//static inline void bsp_close_led_net2(void)
//{
//    LED_NET2->DR_SET = LED_NET2_PIN_MASK;
//}

//open: green on
//close: red on
static inline void bsp_close_LED_RUN_ERR(void)
{
    LED_RUN->DR_CLEAR = LED_RUN_PIN_MASK;
    LED_ERR->DR_CLEAR = LED_ERR_PIN_MASK;
}

static inline void bsp_open_run_led(void)
{
    LED_RUN->DR_SET = LED_RUN_PIN_MASK;
}
static inline void bsp_close_run_led(void)
{
    LED_RUN->DR_CLEAR = LED_RUN_PIN_MASK;
}

//static inline void bsp_open_net_led(void)
//{
//    LED_NET1->DR_CLEAR = LED_NET1_PIN_MASK;
//}
//static inline void bsp_close_net_led(void)
//{
//    LED_NET1->DR_SET = LED_NET1_PIN_MASK;
//}

static inline void bsp_open_err_led(void)
{
    LED_ERR->DR_SET = LED_ERR_PIN_MASK;
}
static inline void bsp_close_err_led(void)
{
    LED_ERR->DR_CLEAR = LED_ERR_PIN_MASK;
}

static inline void bsp_open_led_mqtt(void)
{
    LED_UNDEF->DR_SET = LED_UNDEF_PIN_MASK;
}
static inline void bsp_close_led_mqtt(void)
{
    LED_UNDEF->DR_CLEAR = LED_UNDEF_PIN_MASK;
}
static inline void bsp_toggle_led_mqtt(void)
{
    GPIO_PortToggle(LED_UNDEF, LED_UNDEF_PIN_MASK);
}


//static inline void bsp_toggle_net_module_led(void)
//{
//    GPIO_PortToggle(LED_NET1, LED_NET1_PIN_MASK);
//}

//static inline void bsp_toggle_net_ethernet_led(void)
//{
//    GPIO_PortToggle(LED_NET2, LED_NET2_PIN_MASK);
//}

extern void bsp_led_init(void);
extern void bsp_refresh_io_port_led(unsigned short lsv_InputValue, unsigned short lsv_OutputValue);
extern void bsp_refresh_communication_led(unsigned long llv_Vaule, unsigned long llv_mask);
extern void bsp_open_all_led(void);
extern void bsp_toggle_all_led(void);
extern void bsp_close_all_led(void);
extern void bsp_close_err_led(void);
#endif /*__BSP_LED_H*/

