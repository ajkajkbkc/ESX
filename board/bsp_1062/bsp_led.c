/**
  ******************************************************************************
  * @file    bsp_led.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   LED 驱动
  ******************************************************************************
  */
#include "bsp_led.h"
#include "fsl_debug_console.h"

volatile static unsigned long slv_IndicationLedData;
/**
  * @brief  LED初始化
  * @param  None
  * @retval None
  */
void bsp_led_init(void)
{
    gpio_pin_config_t led_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
    /* Init output LED GPIO. */
    GPIO_PinInit(LED_1, LED_1_PIN, &led_config);
//    GPIO_PinInit(LED_2, LED_2_PIN, &led_config);
//    GPIO_PinInit(LED_3, LED_3_PIN, &led_config);
    GPIO_PinInit(LED_4, LED_4_PIN, &led_config);
    GPIO_PinInit(LED_5, LED_5_PIN, &led_config);
}

void bsp_open_all_led(void)
{
#if 0
    LED_1->DR_CLEAR = LED_1_PIN_MASK;
    LED_2->DR_CLEAR = LED_2_PIN_MASK;
    LED_3->DR_CLEAR = LED_3_PIN_MASK;
    LED_4->DR_CLEAR = LED_4_PIN_MASK;
    LED_5->DR_CLEAR = LED_5_PIN_MASK;
#elif 1
    LED_1->DR_CLEAR = LED_1_PIN_MASK;
    LED_4->DR_CLEAR = LED_4_PIN_MASK;
    LED_5->DR_CLEAR = LED_5_PIN_MASK;
#else
    LED_1->DR_CLEAR = LED_ALL_PIN_MASK;
#endif
}

void bsp_close_all_led(void)
{
    LOGW("bsp_led", "Enter %s()", __func__);
#if 0
    LED_1->DR_SET = LED_1_PIN_MASK;
    LED_2->DR_SET = LED_2_PIN_MASK;
    LED_3->DR_SET = LED_3_PIN_MASK;
    LED_4->DR_SET = LED_4_PIN_MASK;
    LED_5->DR_SET = LED_5_PIN_MASK;
#elif 1
    LED_1->DR_SET = LED_1_PIN_MASK;
    LED_4->DR_SET = LED_4_PIN_MASK;
    LED_5->DR_SET = LED_5_PIN_MASK;
#else
    LED_1->DR_SET = LED_ALL_PIN_MASK;
#endif
}

void bsp_toggle_all_led(void)
{
    GPIO_PortToggle(LED_1, LED_1_PIN_MASK);
    GPIO_PortToggle(LED_4, LED_4_PIN_MASK);
    GPIO_PortToggle(LED_5, LED_5_PIN_MASK);
}

#if 0
/**
  * @brief  驱动端口LED指示灯
  * @param  None
  * @retval None
  */
void bsp_output_indication_led_ic595(unsigned long llv_LedData)
{
    for(int8_t i = 23; i >= 0; i--)
    {
        //LED_2->DR_CLEAR = LED_2_PIN_MASK; // Led On.
        if((llv_LedData >> i) & 0x01)
        {
            LED_3->DR_CLEAR = LED_3_PIN_MASK; // Led On.
        }
        else
        {
            LED_3->DR_SET = LED_3_PIN_MASK; // Led Off.
        }
        //LED_2->DR_SET = LED_2_PIN_MASK; // Led Off.
    }
}

void bsp_toggle_led_1(void)
{
#if (defined(FSL_FEATURE_IGPIO_HAS_DR_TOGGLE) && (FSL_FEATURE_IGPIO_HAS_DR_TOGGLE == 1))
    GPIO_PortToggle(LED_1, LED_1_PIN_MASK);
#else
    if (g_pinSet)
    {
        GPIO_PinWrite(LED_1, LED_1_PIN, 0U);
        g_pinSet = false;
    }
    else
    {
        GPIO_PinWrite(LED_1, LED_1_PIN, 1U);
        g_pinSet = true;
    }
#endif /* FSL_FEATURE_IGPIO_HAS_DR_TOGGLE */
}

void bsp_toggle_led_2(void)
{
#if (defined(FSL_FEATURE_IGPIO_HAS_DR_TOGGLE) && (FSL_FEATURE_IGPIO_HAS_DR_TOGGLE == 1))
    GPIO_PortToggle(LED_2, 1u << LED_2_PIN);
#else
    if (g_pinSet)
    {
        GPIO_PinWrite(LED_2, LED_2_PIN, 0U);
        g_pinSet = false;
    }
    else
    {
        GPIO_PinWrite(LED_2, LED_2_PIN, 1U);
        g_pinSet = true;
    }
#endif /* FSL_FEATURE_IGPIO_HAS_DR_TOGGLE */
}
void bsp_toggle_led_3(void)
{
#if (defined(FSL_FEATURE_IGPIO_HAS_DR_TOGGLE) && (FSL_FEATURE_IGPIO_HAS_DR_TOGGLE == 1))
    GPIO_PortToggle(LED_3, 1u << LED_3_PIN);
#else
    if (g_pinSet)
    {
        GPIO_PinWrite(LED_3, LED_3_PIN, 0U);
        g_pinSet = false;
    }
    else
    {
        GPIO_PinWrite(LED_3, LED_3_PIN, 1U);
        g_pinSet = true;
    }
#endif /* FSL_FEATURE_IGPIO_HAS_DR_TOGGLE */
}
#endif

#if 0
/**
  * @brief  刷新IO端口LED
  * @param  None
  * @retval None
  */
void bsp_refresh_io_port_led(unsigned short lsv_InputValue, unsigned short lsv_OutputValue)
{
    /*Demo code for kt*/
    slv_IndicationLedData &= 0xFFF087FF;
    slv_IndicationLedData |= (lsv_InputValue & 0x0F) << 11;

    slv_IndicationLedData |= (lsv_OutputValue & 0x0F) << 16;

    bsp_output_indication_led_ic595(slv_IndicationLedData);
}
#endif

/**
  * @brief  刷新通信端口LED
  * @param  None
  * @retval None
  */
void bsp_refresh_communication_led(unsigned long llv_Vaule, unsigned long llv_mask)
{
#if 0
    slv_IndicationLedData &= ~llv_mask;
    slv_IndicationLedData |= llv_Vaule;

    bsp_output_indication_led_ic595(slv_IndicationLedData);
#endif
}

