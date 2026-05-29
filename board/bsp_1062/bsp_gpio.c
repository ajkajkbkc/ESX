/**
  ******************************************************************************
  * @file    bsp_gpio.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   I/O 驱动
  ******************************************************************************
  */

#include "FreeRTOS.h"
#include "plc_interrupt.h"
#include "bsp_gpio.h"
#include "kalyke_version.h"
#include "fsl_debug_console.h"
#include "kalyke_tool.h"
#include "plc_element.h"
#include "highspeed_int.h"
#include "plc_io_interrupt.h"
#include "bsp_tim.h"



//pFuncRefreshIO gGPIORefreshIO = NULL; //2022.3.3
uint32_t gX0IntCount = 0;

/*
 * X000: GPIO1_16
*/
void GPIO1_Combined_16_31_IRQHandler(void)
{
    if (X0_PIN_MASK & GPIO_PortGetInterruptFlags(X0_GPIO))
    {
        GPIO_PortClearInterruptFlags(X0_GPIO, X0_PIN_MASK);
        gX0IntCount++;
        X000_Interrupt();
    }
    SDK_ISR_EXIT_BARRIER;
}

/*
 * X001: GPIO2_02
 * 
*/
void GPIO2_Combined_0_15_IRQHandler(void)
{
    if (X1_PIN_MASK & GPIO_PortGetInterruptFlags(X1_GPIO))
    {
        GPIO_PortClearInterruptFlags(X1_GPIO, X1_PIN_MASK);
        X001_Interrupt();
    }
    SDK_ISR_EXIT_BARRIER;
}

/*
 * X002: GPIO3_25
 * 
*/
void GPIO3_Combined_16_31_IRQHandler(void)
{
    if (X2_PIN_MASK & GPIO_PortGetInterruptFlags(X2_GPIO))
    {
        GPIO_PortClearInterruptFlags(X2_GPIO, X2_PIN_MASK);
        X002_Interrupt();
    }
    SDK_ISR_EXIT_BARRIER;
}

/*
 * X003: GPIO1_02
 * X006: GPIO1_00
 * 
*/
void GPIO1_Combined_0_15_IRQHandler(void)
{
    if (X3_PIN_MASK & GPIO_PortGetInterruptFlags(X3_GPIO))
    {
        GPIO_PortClearInterruptFlags(X3_GPIO, X3_PIN_MASK);
        X003_Interrupt();
    }
    else if (X6_PIN_MASK & GPIO_PortGetInterruptFlags(X6_GPIO))
    {
        GPIO_PortClearInterruptFlags(X6_GPIO, X6_PIN_MASK);
    }
    SDK_ISR_EXIT_BARRIER;
}

/*
 * X004: GPIO3_03
 * 
*/
void GPIO3_Combined_0_15_IRQHandler(void)
{
    if (X4_PIN_MASK & GPIO_PortGetInterruptFlags(X4_GPIO))
    {
        GPIO_PortClearInterruptFlags(X4_GPIO, X4_PIN_MASK);
        X004_Interrupt();
    }
    SDK_ISR_EXIT_BARRIER;
}

/*
 * X005: GPIO2_31
 * X007: GPIO2_28
*/
void GPIO2_Combined_16_31_IRQHandler(void)
{
    if (X5_PIN_MASK & GPIO_PortGetInterruptFlags(X5_GPIO))
    {
        GPIO_PortClearInterruptFlags(X5_GPIO, X5_PIN_MASK);
        X005_Interrupt();
    }
    else if (X7_PIN_MASK & GPIO_PortGetInterruptFlags(X7_GPIO))
    {
        GPIO_PortClearInterruptFlags(X7_GPIO, X7_PIN_MASK);
    }
    SDK_ISR_EXIT_BARRIER;
}


static void bsp_gpio_other_init(void)
{
    gpio_pin_config_t sw_config_input =
    {
        kGPIO_DigitalInput, 0,
        kGPIO_IntRisingEdge,
    };
    gpio_pin_config_t sw_config_output =
    {
        kGPIO_DigitalOutput, 0,
        kGPIO_NoIntmode,
    };
    
    GPIO_PinInit(MCU_UART3_DE_GPIO, MCU_UART3_DE_PIN, &sw_config_output);
    //GPIO_PinInit(MCU_CARD_UNDEF1_GPIO, MCU_CARD_UNDEF1_PIN, &sw_config_output);
    //GPIO_PinInit(MCU_CFG6_GPIO, MCU_CFG6_PIN, &sw_config_input);
    /* Init input switch GPIO. */
    //EnableIRQ(MCU_CFG6_IRQ);
    
    /* Enable GPIO pin interrupt */
    //GPIO_PortEnableInterrupts(MCU_CFG6_GPIO, MCU_CFG6_MASK);

    /* Power detect */
    sw_config_input.interruptMode = kGPIO_NoIntmode;
    GPIO_PinInit(LOW_POWER_DETECT_GPIO, LOW_POWER_DETECT_PIN, &sw_config_input);

#if 0
    /* SN74AHC595 */
    GPIO_PinInit(SRCLK_GPIO, SRCLK_PIN, &sw_config_output);
    GPIO_PinInit(RCLK_GPIO, RCLK_PIN, &sw_config_output);
    GPIO_PinInit(SER_GPIO, SER_PIN, &sw_config_output);
    sw_config_output.outputLogic = 1;
    GPIO_PinInit(SRCLR_GPIO, SRCLR_PIN, &sw_config_output);
    GPIO_PinInit(OE_GPIO, OE_PIN, &sw_config_output);    

    /* SN74HC165 */
    GPIO_PinInit(QH_GPIO, QH_PIN, &sw_config_input);
    sw_config_output.outputLogic = 1;
    GPIO_PinInit(LD_GPIO, LD_PIN, &sw_config_output);
#endif
    NVIC_SetPriority(X0_IRQ, 0);
    NVIC_SetPriority(X1_IRQ, 0);
    NVIC_SetPriority(X2_IRQ, 0);
    NVIC_SetPriority(X3_IRQ, 0);
    NVIC_SetPriority(X4_IRQ, 0);
    NVIC_SetPriority(X5_IRQ, 0);
    EnableIRQ(X0_IRQ);
    EnableIRQ(X1_IRQ);
    EnableIRQ(X2_IRQ);
    EnableIRQ(X3_IRQ);// X6_IRQ
    EnableIRQ(X4_IRQ);
    EnableIRQ(X5_IRQ);// X7_IRQ
}

/**
  * @brief  系统IO初始化
  * @param  None
  * @retval None
  */
void bsp_gpio_init(void)
{
    /* Define the init structure for the input switch pin */
    gpio_pin_config_t sw_config_X =
    {
        kGPIO_DigitalInput, 0,
        kGPIO_NoIntmode,
    };
    gpio_pin_config_t sw_config_Y =
    {
        kGPIO_DigitalOutput, 0,
        kGPIO_NoIntmode,
    };

    /* Init input switch GPIO. */
    GPIO_PinInit(X0_GPIO, X0_GPIO_PIN, &sw_config_X);
    GPIO_PinInit(X1_GPIO, X1_GPIO_PIN, &sw_config_X);
    GPIO_PinInit(X2_GPIO, X2_GPIO_PIN, &sw_config_X);
    GPIO_PinInit(X3_GPIO, X3_GPIO_PIN, &sw_config_X);
    GPIO_PinInit(X4_GPIO, X4_GPIO_PIN, &sw_config_X);
    GPIO_PinInit(X5_GPIO, X5_GPIO_PIN, &sw_config_X);
    GPIO_PinInit(X6_GPIO, X6_GPIO_PIN, &sw_config_X);
    GPIO_PinInit(X7_GPIO, X7_GPIO_PIN, &sw_config_X);

    /* Init output switch GPIO. */
    GPIO_PinInit(Y0_GPIO, Y0_GPIO_PIN, &sw_config_Y);
    GPIO_PinInit(Y1_GPIO, Y1_GPIO_PIN, &sw_config_Y);
    GPIO_PinInit(Y2_GPIO, Y2_GPIO_PIN, &sw_config_Y);
    GPIO_PinInit(Y3_GPIO, Y3_GPIO_PIN, &sw_config_Y);
    GPIO_PinInit(Y4_GPIO, Y4_GPIO_PIN, &sw_config_Y);
    GPIO_PinInit(Y5_GPIO, Y5_GPIO_PIN, &sw_config_Y);
    GPIO_PinInit(Y6_GPIO, Y6_GPIO_PIN, &sw_config_Y);
    GPIO_PinInit(Y7_GPIO, Y7_GPIO_PIN, &sw_config_Y);

    bsp_gpio_other_init();
}

void bsp_kalyke_enable_X_interrupt(uint8_t Xx, gpio_interrupt_mode_t interruptMode)
{
    if (interruptMode == kGPIO_IntRisingEdge)
    {
        //interruptMode = kGPIO_IntFallingEdge;
    }
    GPIO_Type *pGpioType;
    uint32_t mask, pin;
    switch(Xx)
    {
    case 0:
        pGpioType = X0_GPIO;
        mask = X0_PIN_MASK;
        pin = X0_GPIO_PIN;
        break;
    case 1:
        pGpioType = X1_GPIO;
        mask = X1_PIN_MASK;
        pin = X1_GPIO_PIN;
        break;
    case 2:
        pGpioType = X2_GPIO;
        mask = X2_PIN_MASK;
        pin = X2_GPIO_PIN;
        break;
    case 3:
        pGpioType = X3_GPIO;
        mask = X3_PIN_MASK;
        pin = X3_GPIO_PIN;
        break;
    case 4:
        pGpioType = X4_GPIO;
        mask = X4_PIN_MASK;
        pin = X4_GPIO_PIN;
        break;
    case 5:
        pGpioType = X5_GPIO;
        mask = X5_PIN_MASK;
        pin = X5_GPIO_PIN;
        break;
    case 6:
        pGpioType = X6_GPIO;
        mask = X6_PIN_MASK;
        pin = X6_GPIO_PIN;
        break;
    case 7:
        pGpioType = X7_GPIO;
        mask = X7_PIN_MASK;
        pin = X7_GPIO_PIN;
        break;
    default:
        LOGE("bsp_gpio", "Input %u does not exist!", Xx);
        break;
    }
    GPIO_PinSetInterruptConfig(pGpioType, pin, interruptMode);
    /* Enable GPIO pin interrupt */
    GPIO_PortEnableInterrupts(pGpioType, mask);
}

void bsp_kalyke_disable_X_interrupt(uint8_t Xx)
{
    GPIO_Type *pGpioType;
    uint32_t mask;
    switch(Xx)
    {
    case 0:
        pGpioType = X0_GPIO;
        mask = X0_PIN_MASK;
        break;
    case 1:
        pGpioType = X1_GPIO;
        mask = X1_PIN_MASK;
        break;
    case 2:
        pGpioType = X2_GPIO;
        mask = X2_PIN_MASK;
        break;
    case 3:
        pGpioType = X3_GPIO;
        mask = X3_PIN_MASK;
        break;
    case 4:
        pGpioType = X4_GPIO;
        mask = X4_PIN_MASK;
        break;
    case 5:
        pGpioType = X5_GPIO;
        mask = X5_PIN_MASK;
        break;
    case 6:
        pGpioType = X6_GPIO;
        mask = X6_PIN_MASK;
        break;
    case 7:
        pGpioType = X7_GPIO;
        mask = X7_PIN_MASK;
        break;
    default:
        LOGE("bsp_gpio", "Input %u does not exist!!!", Xx);
        break;
    }
    /* Disable GPIO pin interrupt */
    GPIO_PortDisableInterrupts(pGpioType, mask);
}



/**
  * @brief  刷新X元件输入值
  * @param  None
  * @retval None
  */
unsigned char bsp_refresh_input_port(unsigned short *lsp_StartElement, unsigned short lsv_InputNum)
{
#if SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_SDK
    //static uint32_t ticknow = 0;
#endif
    unsigned char lsv_Data = 0;
    unsigned char i;
    unsigned char lsv_CurValue;

    if(lsv_InputNum == 0)
    {
        return pdPASS;
    }

    lsv_CurValue = lsp_StartElement[0] & 0x00FF;

    lsv_Data = GPIO_PinReadPadStatus(X0_GPIO, X0_GPIO_PIN);
    lsv_Data |= GPIO_PinReadPadStatus(X1_GPIO, X1_GPIO_PIN) << 1;
    lsv_Data |= GPIO_PinReadPadStatus(X2_GPIO, X2_GPIO_PIN) << 2;
    lsv_Data |= GPIO_PinReadPadStatus(X3_GPIO, X3_GPIO_PIN) << 3;
    lsv_Data |= GPIO_PinReadPadStatus(X4_GPIO, X4_GPIO_PIN) << 4;
    lsv_Data |= GPIO_PinReadPadStatus(X5_GPIO, X5_GPIO_PIN) << 5;
    lsv_Data |= GPIO_PinReadPadStatus(X6_GPIO, X6_GPIO_PIN) << 6;
    lsv_Data |= GPIO_PinReadPadStatus(X7_GPIO, X7_GPIO_PIN) << 7;
    /*与具体硬件设计有关*/
    //lsv_Data = ~lsv_Data;
    //lsv_Data &= 0x0F;
    lsp_StartElement[0] &= 0xFF00;
    lsp_StartElement[0] |= lsv_Data;
#if 0
#if SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_SDK
    if (xTaskGetTickCount() - ticknow > 5000)
    {
        LOGD("bsp_gpio", "lsv_Data = 0x%X, lsv_CurValue = 0x%X, IS_INTERRUPT_ENABLE = %d", lsv_Data, lsv_CurValue, IS_INTERRUPT_ENABLE());
        ticknow = xTaskGetTickCount();
    }
#endif
#endif
    /*用户中断处理*/
    if(IS_INTERRUPT_ENABLE())
    {
        for(i = 0; i < 4; i++)
        {
            if((lsv_CurValue & (0x01 << i)) ^ (lsv_Data & (0x01 << i)))
            {
                if(lsv_CurValue & (0x01 << i))
                {
                    plc_add_int_to_interrupt_queue(X0_DOWN_EDGE_INT + i, 0);
                }
                else
                {
                    plc_add_int_to_interrupt_queue(X0_UP_EDGE_INT + i, 0);
                }
            }
        }
    }
    return pdPASS;
}

static void gpio_delay_20_ns(void)
{
    __asm("NOP");//1
    __asm("NOP");//2
    __asm("NOP");//3
#if 1
    __asm("NOP");//4
    __asm("NOP");//5
    __asm("NOP");//6
    __asm("NOP");//7
    __asm("NOP");//8
    __asm("NOP");//9

    __asm("NOP");//10
    __asm("NOP");//11
    __asm("NOP");//12
    __asm("NOP");//13
    __asm("NOP");//14
    __asm("NOP");//15
    __asm("NOP");//16
    __asm("NOP");//17
    __asm("NOP");//18

    __asm("NOP");//19
    __asm("NOP");//20
    __asm("NOP");//21
    __asm("NOP");//22
    __asm("NOP");//23
    __asm("NOP");//24
#endif
}


static void dm_end(void)
{
    
}
static void dm_start(void)
{
}

/* 单位均为纳秒 */
#define LD_HOLD_TIME          5
#define LD_PULSE_DURATION     120
#define CLK_PULSE_DURATION    120
#if 0
/* From H to A(X_7 to X_0) */
static uint8_t dm_read_X(void)
{
    uint8_t u8X = 0;

    GPIO_PortClear(SRCLR_GPIO, SRCLR_PIN_MASK);//先使595的clk无效
    //GPIO_PortClear(CLK_SRCLK_GPIO, CLK_SRCLK_PIN_MASK);
    //kalyke_delay_ns2(120);

    //GPIO_PortClear(LD_GPIO, LD_PIN_MASK);
    //kalyke_delay_ns2(LD_PULSE_DURATION);
    GPIO_PortSet(LD_GPIO, LD_PIN_MASK);
    kalyke_delay_ns2(500);
    u8X |= GPIO_PinReadPadStatus(QH_GPIO, QH_PIN) << 7;
    for (int i = 6; i >= 0; i--)
    {
        GPIO_PortSet(CLK_SRCLK_GPIO, CLK_SRCLK_PIN_MASK);
        kalyke_delay_ns2(225);
        u8X |= GPIO_PinReadPadStatus(QH_GPIO, QH_PIN) << i;
        GPIO_PortClear(CLK_SRCLK_GPIO, CLK_SRCLK_PIN_MASK);
        kalyke_delay_ns2(CLK_PULSE_DURATION);
    }
    return u8X;
}

/* 单位均为纳秒 */
#define Y_CLK_PULSE_DURATION    6
static void dm_write_Y(unsigned short yElement)
{
    GPIO_PortSet(SRCLR_GPIO, SRCLR_PIN_MASK);//先使595的clk有效
    GPIO_PortClear(LD_GPIO, LD_PIN_MASK);//先使165的clk无效
    uint8_t serVal = yElement >> 8;

    for (int i = 7; i >= 0; i--)
    {
        if (serVal & (1 << i))
        {
            GPIO_PortSet(SER_GPIO, SER_PIN_MASK);
        }
        else
        {
            GPIO_PortClear(SER_GPIO, SER_PIN_MASK);
        }
        gpio_delay_20_ns();
        GPIO_PortSet(CLK_SRCLK_GPIO, CLK_SRCLK_PIN_MASK);
        gpio_delay_20_ns();
        GPIO_PortClear(CLK_SRCLK_GPIO, CLK_SRCLK_PIN_MASK);
        gpio_delay_20_ns();
    }
    GPIO_PortSet(RCLK_GPIO, RCLK_PIN_MASK);
    gpio_delay_20_ns();
    GPIO_PortClear(OE_GPIO, OE_PIN_MASK);
    GPIO_PortClear(RCLK_GPIO, RCLK_PIN_MASK);
}
#endif

static uint8_t dm_read_X_write_Y(unsigned short yElement)
{
    uint8_t serVal = yElement >> 8;
    uint8_t u8X = 0;

    //GPIO_PortClear(CLK_SRCLK_GPIO, CLK_SRCLK_PIN_MASK);
    //kalyke_delay_ns2(120);

    /* 载入SN74HC165的A~H */
    GPIO_PortClear(LD_GPIO, LD_PIN_MASK);
    kalyke_delay_ns2(LD_PULSE_DURATION);
    GPIO_PortSet(LD_GPIO, LD_PIN_MASK);
    kalyke_delay_ns2(225);
    u8X |= GPIO_PinReadPadStatus(QH_GPIO, QH_PIN) << 7;
    /* 同时进行：对X(SN74HC165)的读取以及对Y(SN74AHC595)的输出 */
    for (int i = 7; i >= 0; i--)
    {
        if (serVal & (1 << i))
        {
            GPIO_PortSet(SER_GPIO, SER_PIN_MASK);
        }
        else
        {
            GPIO_PortClear(SER_GPIO, SER_PIN_MASK);
        }
        GPIO_PortSet(CLK_SRCLK_GPIO, CLK_SRCLK_PIN_MASK);
        kalyke_delay_ns2(99);//225ns
        if (i != 0)
        {
            u8X |= GPIO_PinReadPadStatus(QH_GPIO, QH_PIN) << (i - 1);
        }
        kalyke_delay_ns2(126);
        GPIO_PortClear(CLK_SRCLK_GPIO, CLK_SRCLK_PIN_MASK);
        kalyke_delay_ns2(CLK_PULSE_DURATION);
    }
    /* SN74AHC595最终输出 */
    GPIO_PortSet(RCLK_GPIO, RCLK_PIN_MASK);
    gpio_delay_20_ns();
    GPIO_PortClear(OE_GPIO, OE_PIN_MASK);
    GPIO_PortClear(RCLK_GPIO, RCLK_PIN_MASK);
    //u8X >>= 1;
    return u8X;
}

#if 0
/* 刷新I板的SN74HC165
*/
unsigned char bsp_refresh_input_port_dm(unsigned short *lsp_StartElement, unsigned short lsv_InputNum)
{
    if(lsv_InputNum == 0)
    {
        return pdPASS;
    }
    dm_start();
    uint16_t lsv_Data;
    uint16_t lsv_CurValue = lsp_StartElement[0];

    lsv_Data  = GPIO_PinReadPadStatus(X0_GPIO, X0_GPIO_PIN);
    lsv_Data |= GPIO_PinReadPadStatus(X1_GPIO, X1_GPIO_PIN) << 1;
    lsv_Data |= GPIO_PinReadPadStatus(X2_GPIO, X2_GPIO_PIN) << 2;
    lsv_Data |= GPIO_PinReadPadStatus(X3_GPIO, X3_GPIO_PIN) << 3;
    lsv_Data |= GPIO_PinReadPadStatus(X4_GPIO, X4_GPIO_PIN) << 4;
    lsv_Data |= GPIO_PinReadPadStatus(X5_GPIO, X5_GPIO_PIN) << 5;
    lsv_Data |= GPIO_PinReadPadStatus(X6_GPIO, X6_GPIO_PIN) << 6;
    lsv_Data |= GPIO_PinReadPadStatus(X7_GPIO, X7_GPIO_PIN) << 7;
    lsv_Data |= dm_read_X() << 8;
    lsp_StartElement[0] = lsv_Data;
    /*用户中断处理*/
    if(IS_INTERRUPT_ENABLE())
    {
        for(uint8_t i = 0; i < 4; i++)
        {
            if((lsv_CurValue & (0x01 << i)) ^ (lsv_Data & (0x01 << i)))
            {
                if(lsv_CurValue & (0x01 << i))
                {
                    plc_add_int_to_interrupt_queue(X0_DOWN_EDGE_INT + i, 0);
                }
                else
                {
                    plc_add_int_to_interrupt_queue(X0_UP_EDGE_INT + i, 0);
                }
            }
        }
    }
    return pdPASS;
}

/**
  * @brief  刷新Y元件输出值
  * @param  None
  * @retval None
  */
unsigned char bsp_refresh_output_port(unsigned short *lsp_StartElement, unsigned short lsv_OutputNum)
{
#ifdef PROJECT_KALYKE
    if(lsv_OutputNum == 0) {
        return pdPASS;
    }

    if(lsp_StartElement[0] & 0x01)
    {
        GPIO_PortSet(Y0_GPIO, Y0_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y0_GPIO, Y0_PIN_MASK);
    }

    if(lsp_StartElement[0] & 0x02)
    {
        GPIO_PortSet(Y1_GPIO, Y1_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y1_GPIO, Y1_PIN_MASK);
    }

    if(lsp_StartElement[0] & 0x04)
    {
        GPIO_PortSet(Y2_GPIO, Y2_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y2_GPIO, Y2_PIN_MASK);
    }

    if(lsp_StartElement[0] & 0x08)
    {
        GPIO_PortSet(Y3_GPIO, Y3_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y3_GPIO, Y3_PIN_MASK);
    }

    if(lsp_StartElement[0] & 0x10)
    {
        GPIO_PortSet(Y4_GPIO, Y4_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y4_GPIO, Y4_PIN_MASK);
    }

    if(lsp_StartElement[0] & 0x20)
    {
        GPIO_PortSet(Y5_GPIO, Y5_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y5_GPIO, Y5_PIN_MASK);
    }
#endif
    return pdPASS;
}

/* 刷新I板的SN74AHC595 */
unsigned char bsp_refresh_output_port_dm(unsigned short *lsp_StartElement, unsigned short lsv_OutputNum)
{
    if(lsv_OutputNum == 0)
    {
        dm_end();
        return pdPASS;
    }

    if(lsp_StartElement[0] & 0x01)
    {
        GPIO_PortSet(Y0_GPIO, Y0_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y0_GPIO, Y0_PIN_MASK);
    }

    if(lsp_StartElement[0] & 0x02)
    {
        GPIO_PortSet(Y1_GPIO, Y1_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y1_GPIO, Y1_PIN_MASK);
    }

    if(lsp_StartElement[0] & 0x04)
    {
        GPIO_PortSet(Y2_GPIO, Y2_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y2_GPIO, Y2_PIN_MASK);
    }

    if(lsp_StartElement[0] & 0x08)
    {
        GPIO_PortSet(Y3_GPIO, Y3_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y3_GPIO, Y3_PIN_MASK);
    }

    if(lsp_StartElement[0] & 0x10)
    {
        GPIO_PortSet(Y4_GPIO, Y4_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y4_GPIO, Y4_PIN_MASK);
    }

    if(lsp_StartElement[0] & 0x20)
    {
        GPIO_PortSet(Y5_GPIO, Y5_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y5_GPIO, Y5_PIN_MASK);
    }



    dm_end();
    return pdPASS;
}
#endif
void kalyke_Y_output(unsigned char yValue, unsigned char on_off)
{
    LOGI("bsp_gpio", "yValue = %u, on_off = %u", yValue, on_off);
    switch (yValue)
    {
        case 0:
            if (on_off)
            {
                GPIO_PortSet(Y0_GPIO, Y0_PIN_MASK);
            }
            else
            {
                GPIO_PortClear(Y0_GPIO, Y0_PIN_MASK);
            }
            break;
        case 1:
            if (on_off)
            {
                GPIO_PortSet(Y1_GPIO, Y1_PIN_MASK);
            }
            else
            {
                GPIO_PortClear(Y1_GPIO, Y1_PIN_MASK);
            }
            break;
        case 2:
            if (on_off)
            {
                GPIO_PortSet(Y2_GPIO, Y2_PIN_MASK);
            }
            else
            {
                GPIO_PortClear(Y2_GPIO, Y2_PIN_MASK);
            }
            break;
        case 3:
            if (on_off)
            {
                GPIO_PortSet(Y3_GPIO, Y3_PIN_MASK);
            }
            else
            {
                GPIO_PortClear(Y3_GPIO, Y3_PIN_MASK);
            }
            break;
        case 4:
            if (on_off)
            {
                GPIO_PortSet(Y4_GPIO, Y4_PIN_MASK);
            }
            else
            {
                GPIO_PortClear(Y4_GPIO, Y4_PIN_MASK);
            }
            break;
        case 5:
            if (on_off)
            {
                GPIO_PortSet(Y5_GPIO, Y5_PIN_MASK);
            }
            else
            {
                GPIO_PortClear(Y5_GPIO, Y5_PIN_MASK);
            }
            break;
        default:
            LOGE("bsp_gpio", "%s : ERROR!", __func__);
            break;
    }
}

void hccmd_output(unsigned char yValue, unsigned char on_off)
{
    LOGI("bsp_gpio", "yValue = %u, on_off = %u", yValue, on_off);
    switch (yValue)
    {
        case 0:
            if (on_off)
            {
                GPIO_PortSet(Y0_GPIO, Y0_PIN_MASK);
            }
            else
            {
                GPIO_PortClear(Y0_GPIO, Y0_PIN_MASK);
            }
            break;
        case 1:
            if (on_off)
            {
                GPIO_PortSet(Y1_GPIO, Y1_PIN_MASK);
            }
            else
            {
                GPIO_PortClear(Y1_GPIO, Y1_PIN_MASK);
            }
            break;
        case 2:
            if (on_off)
            {
                GPIO_PortSet(Y2_GPIO, Y2_PIN_MASK);
            }
            else
            {
                GPIO_PortClear(Y2_GPIO, Y2_PIN_MASK);
            }
            break;
        case 3:
            if (on_off)
            {
                GPIO_PortSet(Y3_GPIO, Y3_PIN_MASK);
            }
            else
            {
                GPIO_PortClear(Y3_GPIO, Y3_PIN_MASK);
            }
            break;
        case 4:
            if (on_off)
            {
                GPIO_PortSet(Y4_GPIO, Y4_PIN_MASK);
            }
            else
            {
                GPIO_PortClear(Y4_GPIO, Y4_PIN_MASK);
            }
            break;
        case 5:
            if (on_off)
            {
                GPIO_PortSet(Y5_GPIO, Y5_PIN_MASK);
            }
            else
            {
                GPIO_PortClear(Y5_GPIO, Y5_PIN_MASK);
            }
            break;
        default:
            LOGE("bsp_gpio", "%s : ERROR!", __func__);
            break;
    }
}

void bsp_refresh_input_output_port(unsigned short inputNum, unsigned short outputNum)
{

#if SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_SDK
    //static uint32_t ticknow = 0;
#endif
    unsigned short *lsp_StartElement;
    if(inputNum == 0)
    {
        goto REFRESH_OUT;
    }
    unsigned char lsv_Data;
    unsigned char i;
    unsigned char lsv_CurValue;
    lsp_StartElement = X_ELEMENT;

    lsv_CurValue = lsp_StartElement[0] & 0x00FF;

    lsv_Data = GPIO_PinReadPadStatus(X0_GPIO, X0_GPIO_PIN);
    lsv_Data |= GPIO_PinReadPadStatus(X1_GPIO, X1_GPIO_PIN) << 1;
    lsv_Data |= GPIO_PinReadPadStatus(X2_GPIO, X2_GPIO_PIN) << 2;
    lsv_Data |= GPIO_PinReadPadStatus(X3_GPIO, X3_GPIO_PIN) << 3;
    lsv_Data |= GPIO_PinReadPadStatus(X4_GPIO, X4_GPIO_PIN) << 4;
    lsv_Data |= GPIO_PinReadPadStatus(X5_GPIO, X5_GPIO_PIN) << 5;
    lsv_Data |= GPIO_PinReadPadStatus(X6_GPIO, X6_GPIO_PIN) << 6;
    lsv_Data |= GPIO_PinReadPadStatus(X7_GPIO, X7_GPIO_PIN) << 7;
    /*与具体硬件设计有关*/
    //lsv_Data = ~lsv_Data;
    //lsv_Data &= 0x0F;
    lsv_Data = 0xFF ^ lsv_Data;
    lsp_StartElement[0] &= 0xFF00;
    lsp_StartElement[0] |= lsv_Data;
#if 0
#if SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_SDK
    if (xTaskGetTickCount() - ticknow > 5000)
    {
        LOGD("bsp_gpio", "lsv_Data = 0x%X, lsv_CurValue = 0x%X, IS_INTERRUPT_ENABLE = %d", lsv_Data, lsv_CurValue, IS_INTERRUPT_ENABLE());
        ticknow = xTaskGetTickCount();
    }
#endif
#endif
    /*用户中断处理*/
    if(IS_INTERRUPT_ENABLE())
    {
        for(i = 0; i < 4; i++)
        {
            if((lsv_CurValue & (0x01 << i)) ^ (lsv_Data & (0x01 << i)))
            {
                if(lsv_CurValue & (0x01 << i))
                {
                    plc_add_int_to_interrupt_queue(X0_DOWN_EDGE_INT + i, 0);
                }
                else
                {
                    plc_add_int_to_interrupt_queue(X0_UP_EDGE_INT + i, 0);
                }
            }
        }
    }

REFRESH_OUT:
    if (outputNum == 0)
    {
        return;
    }

    lsp_StartElement = Y_ELEMENT;
    if(lsp_StartElement[0] & 0x01)
    {
        GPIO_PortSet(Y0_GPIO, Y0_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y0_GPIO, Y0_PIN_MASK);
    }

    if(lsp_StartElement[0] & 0x02)
    {
        GPIO_PortSet(Y1_GPIO, Y1_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y1_GPIO, Y1_PIN_MASK);
    }

    if(lsp_StartElement[0] & 0x04)
    {
        GPIO_PortSet(Y2_GPIO, Y2_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y2_GPIO, Y2_PIN_MASK);
    }
    if(lsp_StartElement[0] & 0x08)
    {
        GPIO_PortSet(Y3_GPIO, Y3_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y3_GPIO, Y3_PIN_MASK);
    }
    if(lsp_StartElement[0] & 0x10)
    {
        GPIO_PortSet(Y4_GPIO, Y4_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y4_GPIO, Y4_PIN_MASK);
    }

    if(lsp_StartElement[0] & 0x20)
    {
        GPIO_PortSet(Y5_GPIO, Y5_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y5_GPIO, Y5_PIN_MASK);
    }

    if(lsp_StartElement[0] & 0x40)
    {
        GPIO_PortSet(Y6_GPIO, Y6_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y6_GPIO, Y6_PIN_MASK);
    }

    if(lsp_StartElement[0] & 0x80)
    {
        GPIO_PortSet(Y7_GPIO, Y7_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y7_GPIO, Y7_PIN_MASK);
    }
}

void bsp_refresh_input_output_port_dm(unsigned short inputNum, unsigned short outputNum)
{
    static uint32_t mTick = 0;
    static uint32_t curTick = 0;

    static bool ifPrint = false;
#if 0
    curTick = xTaskGetTickCount();
    if (curTick - mTick > 3000)
    {
        ifPrint = true;
        mTick = curTick;
    }
    else
    {
        ifPrint = false;
    }
#endif
    unsigned short *lsp_StartElementX = X_ELEMENT;
    unsigned short *lsp_StartElementY = Y_ELEMENT;

    //dm_write_Y(lsp_StartElementY[0]);
    
    uint16_t lsv_CurValueX = lsp_StartElementX[0];
    if (ifPrint) LOGD("bsp_gpio", "lsv_CurValueX = 0x%04X", lsv_CurValueX);
    dm_start();
    uint16_t lsv_Data;

    lsv_Data  = GPIO_PinReadPadStatus(X0_GPIO, X0_GPIO_PIN);
    lsv_Data |= GPIO_PinReadPadStatus(X1_GPIO, X1_GPIO_PIN) << 1;
    lsv_Data |= GPIO_PinReadPadStatus(X2_GPIO, X2_GPIO_PIN) << 2;
    lsv_Data |= GPIO_PinReadPadStatus(X3_GPIO, X3_GPIO_PIN) << 3;
    lsv_Data |= GPIO_PinReadPadStatus(X4_GPIO, X4_GPIO_PIN) << 4;
    lsv_Data |= GPIO_PinReadPadStatus(X5_GPIO, X5_GPIO_PIN) << 5;
    lsv_Data |= GPIO_PinReadPadStatus(X6_GPIO, X6_GPIO_PIN) << 6;
    lsv_Data |= GPIO_PinReadPadStatus(X7_GPIO, X7_GPIO_PIN) << 7;
    //lsv_Data |= dm_read_X() << 8;
    lsv_Data |= dm_read_X_write_Y(lsp_StartElementY[0]) << 8;
    if (ifPrint) LOGD("bsp_gpio", "lsv_Data = 0x%04X", lsv_Data);
    lsp_StartElementX[0] = lsv_Data;
    /*用户中断处理*/
    if(IS_INTERRUPT_ENABLE())
    {
        for(uint8_t i = 0; i < 4; i++)
        {
            if((lsv_CurValueX & (0x01 << i)) ^ (lsv_Data & (0x01 << i)))
            {
                if(lsv_CurValueX & (0x01 << i))
                {
                    plc_add_int_to_interrupt_queue(X0_DOWN_EDGE_INT + i, 0);
                }
                else
                {
                    plc_add_int_to_interrupt_queue(X0_UP_EDGE_INT + i, 0);
                }
            }
        }
    }

    if(lsp_StartElementY[0] & 0x01)
    {
        GPIO_PortSet(Y0_GPIO, Y0_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y0_GPIO, Y0_PIN_MASK);
    }

    if(lsp_StartElementY[0] & 0x02)
    {
        GPIO_PortSet(Y1_GPIO, Y1_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y1_GPIO, Y1_PIN_MASK);
    }

    if(lsp_StartElementY[0] & 0x04)
    {
        GPIO_PortSet(Y2_GPIO, Y2_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y2_GPIO, Y2_PIN_MASK);
    }

    if(lsp_StartElementY[0] & 0x08)
    {
        GPIO_PortSet(Y3_GPIO, Y3_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y3_GPIO, Y3_PIN_MASK);
    }

    if(lsp_StartElementY[0] & 0x10)
    {
        GPIO_PortSet(Y4_GPIO, Y4_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y4_GPIO, Y4_PIN_MASK);
    }

    if(lsp_StartElementY[0] & 0x20)
    {
        GPIO_PortSet(Y5_GPIO, Y5_PIN_MASK);
    }
    else
    {
        GPIO_PortClear(Y5_GPIO, Y5_PIN_MASK);
    }

    //dm_write_Y(lsp_StartElement[0]);
    dm_end();
}

