/**
  ******************************************************************************
  * @file    bsp_tim.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   定时器相关驱动
  ******************************************************************************
  */

#include "FreeRTOS.h"
#include "bsp_tim.h"
#include "FreeRTOS.h"
#include "task.h"
#include "plc_element.h"
#include "bsp_led.h"
#include "fsl_pit.h"
#include "fsl_debug_console.h"
#include "plc_internalmanage.h"
#include "fsl_pwm.h"

#include "pin_mux.h"
#include "fsl_xbara.h"
#include "fsl_iomuxc.h"
#include "kalyke_monitor_task.h"
#include "bsp_gpio.h"
#include "fsl_gpt.h"
#include "fsl_qtmr.h"

/* The PWM base address */
#define BOARD_PWM_BASEADDR PWM1
#define PWM_SRC_CLK_FREQ CLOCK_GetFreq(kCLOCK_IpgClk)


#define PIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_PerClk)
#define PIT_IRQ_ID PIT_IRQn
#define PIT_KALYKE_HANDLER PIT_IRQHandler



bsp_pwm_channel_info_st gPwmInfo[MAX_PWM_OUTPUT_CHANNEL_NUM];
bsp_hsp_ins_channel_info_st gHspChannelInfo[MAX_DRVI_OUTPUT_CHANNEL_NUM] = {0};

/*------------------------------------------------------------------------------
*  外部函数定义
*-----------------------------------------------------------------------------*/
extern void plc_refresh_cycle_clock(void);

/**
  * @brief  时钟振荡时钟源使能
  * @param  None
  * @retval None
  */
void bsp_cycle_clock_enable(unsigned char lcv_En)
{
    if (lcv_En)
    {
        PIT_StartTimer(PIT, kPIT_Chnl_0);
        PIT_StartTimer(PIT, kPIT_Chnl_1);
        PIT_StartTimer(PIT, kPIT_Chnl_2);
        PIT_StartTimer(PIT, kPIT_Chnl_3);
    }
    else
    {
        PIT_StopTimer(PIT, kPIT_Chnl_0);
        PIT_StopTimer(PIT, kPIT_Chnl_1);
        PIT_StopTimer(PIT, kPIT_Chnl_2);
        PIT_StopTimer(PIT, kPIT_Chnl_3);
    }
}

/**
  * @brief  TIM7 中断函数
  * @param  None
  * @retval None
  */
#if 0
void TIM7_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM7, TIM_IT_Update) == SET)
    {
        plc_refresh_cycle_clock();
    }

    TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
}
#else
void PIT_KALYKE_HANDLER(void)
{
    if (PIT_GetStatusFlags(PIT, kPIT_Chnl_0) & kPIT_TimerFlag) // 5ms timer
    {
        /*10ms反转*/
        if(gtv_ClockCycleRecord.mcv_Status & 0x01)
        {
            //LOGI("bsp_tim", "10ms timer happen......");
            plc_set_bit_element_value(SM_ELEMENT, 10, 0);
            gtv_ClockCycleRecord.mcv_Status &= ~0x01;
        }
        else
        {
            plc_set_bit_element_value(SM_ELEMENT, 10, 1);
            gtv_ClockCycleRecord.mcv_Status |= 0x01;
        }
        /* Clear interrupt flag.*/
        PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
    }
    else if (PIT_GetStatusFlags(PIT, kPIT_Chnl_1) & kPIT_TimerFlag)//50ms timer
    {
        /*100ms反转*/
        if(gtv_ClockCycleRecord.mcv_Status & 0x02)
        {
            //LOGI("bsp_tim", "100ms timer happen......");
            plc_set_bit_element_value(SM_ELEMENT, 11, 0);
            gtv_ClockCycleRecord.mcv_Status &= ~0x02;
        }
        else
        {
            plc_set_bit_element_value(SM_ELEMENT, 11, 1);
            gtv_ClockCycleRecord.mcv_Status |= 0x02;
        }
        /* Clear interrupt flag.*/
        PIT_ClearStatusFlags(PIT, kPIT_Chnl_1, kPIT_TimerFlag);
    }
    else if (PIT_GetStatusFlags(PIT, kPIT_Chnl_2) & kPIT_TimerFlag) // 500ms timer
    {
        /*1秒反转*/
        if(gtv_ClockCycleRecord.mcv_Status & 0x04)
        {
            //LOGI("bsp_tim", "1s timer happen......");
            plc_set_bit_element_value(SM_ELEMENT, 12, 0);
            gtv_ClockCycleRecord.mcv_Status &= ~0x04;
        }
        else
        {
            plc_set_bit_element_value(SM_ELEMENT, 12, 1);
            gtv_ClockCycleRecord.mcv_Status |= 0x04;
        }
        /* Clear interrupt flag.*/
        PIT_ClearStatusFlags(PIT, kPIT_Chnl_2, kPIT_TimerFlag);
    }
    else if (PIT_GetStatusFlags(PIT, kPIT_Chnl_3) & kPIT_TimerFlag) // 30s timer
    {
        gtv_ClockCycleRecord.mlv_Hour += 30000;

        /*1分钟反转*/
        if(gtv_ClockCycleRecord.mcv_Status & 0x08)
        {
            plc_set_bit_element_value(SM_ELEMENT, 13, 0);
            gtv_ClockCycleRecord.mcv_Status &= ~0x08;
        }
        else
        {
            plc_set_bit_element_value(SM_ELEMENT, 13, 1);
            gtv_ClockCycleRecord.mcv_Status |= 0x08;
        }

        /*1小时反转*/
        if(gtv_ClockCycleRecord.mlv_Hour >= 30 * 60 * 1000)
        {
            if(gtv_ClockCycleRecord.mcv_Status & 0x10)
            {
                plc_set_bit_element_value(SM_ELEMENT, 14, 0);
                gtv_ClockCycleRecord.mcv_Status &= ~0x10;
            }
            else
            {
                plc_set_bit_element_value(SM_ELEMENT, 14, 1);
                gtv_ClockCycleRecord.mcv_Status |= 0x10;
            }
            gtv_ClockCycleRecord.mlv_Hour = 0;
        }
        /* Clear interrupt flag.*/
        PIT_ClearStatusFlags(PIT, kPIT_Chnl_3, kPIT_TimerFlag);
    }
    SDK_ISR_EXIT_BARRIER;
}
#endif

/**
  * @brief  初始化振荡时钟中断源,
            kPIT_Chnl_0: 5ms触发一次中断
            kPIT_Chnl_1: 50ms触发一次中断
            kPIT_Chnl_2: 500ms触发一次中断
            kPIT_Chnl_3: 30s 触发一次中断
  * @param  None
  * @retval None
  */
void bsp_init_cycle_clock_tim(void)
{
    //CLOCK_EnableClock(kCLOCK_Gpio1);
#if 1
    CLOCK_SetMux(kCLOCK_PerclkMux, 1U);
    //CLOCK_SetDiv(kCLOCK_PerclkDiv, 63U);// 375,000 Hz
    //CLOCK_SetDiv(kCLOCK_PerclkDiv, 31U);// 750,000 Hz
    //CLOCK_SetDiv(kCLOCK_PerclkDiv, 15U);// 1,500,000 Hz
    CLOCK_SetDiv(kCLOCK_PerclkDiv, 23U);// 1,000,000 Hz
#endif
    pit_config_t pitConfig;
    /*
     * pitConfig.enableRunInDebug = false;
     */
    PIT_GetDefaultConfig(&pitConfig);

    /* Init pit module */
    PIT_Init(PIT, &pitConfig);
    PRINTF("CHANNEL[1].TCTRL = 0x%08X\r\n", PIT->CHANNEL[1].TCTRL);

    /* Set timer period for channel 0 */
    PRINTF("PIT_SOURCE_CLOCK = %u\r\n", PIT_SOURCE_CLOCK);
    PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, MSEC_TO_COUNT(5U, PIT_SOURCE_CLOCK)); // 5ms
    PIT_SetTimerPeriod(PIT, kPIT_Chnl_1, MSEC_TO_COUNT(50U, PIT_SOURCE_CLOCK));// 50ms
    PIT_SetTimerPeriod(PIT, kPIT_Chnl_2, MSEC_TO_COUNT(500U, PIT_SOURCE_CLOCK));// 500ms
    PIT_SetTimerPeriod(PIT, kPIT_Chnl_3, MSEC_TO_COUNT(30000U, PIT_SOURCE_CLOCK));// 30s

    PIT_SetTimerChainMode(PIT, kPIT_Chnl_0, false);
    PIT_SetTimerChainMode(PIT, kPIT_Chnl_1, false);
    PIT_SetTimerChainMode(PIT, kPIT_Chnl_2, false);
    PIT_SetTimerChainMode(PIT, kPIT_Chnl_3, false);

    /* Enable timer interrupts */
    PIT_EnableInterrupts(PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);
    PIT_EnableInterrupts(PIT, kPIT_Chnl_1, kPIT_TimerInterruptEnable);
    PIT_EnableInterrupts(PIT, kPIT_Chnl_2, kPIT_TimerInterruptEnable);
    PIT_EnableInterrupts(PIT, kPIT_Chnl_3, kPIT_TimerInterruptEnable);

    PRINTF("CHANNEL[1].TCTRL = 0x%08X\r\n", PIT->CHANNEL[1].TCTRL);

    NVIC_SetPriority(PIT_IRQ_ID, 1);
    /* Enable at the NVIC */
    EnableIRQ(PIT_IRQ_ID);

}

/*------------------------------------------------------------------------------
*  以下代码用以实现PLSY指令
*-----------------------------------------------------------------------------*/
bsp_plsy_ins_channel_info_st gPlsyChannelInfo[MAX_PLSY_OUTPUT_CHANNEL_NUM];

/**
  * @brief  TIM3 初始化，用作PLSY指令周期产生
  * @param  None
  * @retval None
  */
#if  0// 先不弄这块，因为 CI_PLSY是运动控制方面的
void bsp_tim3_init()
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseSt;
    NVIC_InitTypeDef NVIC_InitSt;
    unsigned short lsv_PrescalerValue;
    unsigned char i;

    TIM_DeInit(TIM3);

    RCC_APB1PeriphClockCmd(RCC_APB1ENR_TIM3EN, ENABLE);

    TIM_TimeBaseStructInit(&TIM_TimeBaseSt);
    TIM_TimeBaseSt.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseSt.TIM_Period = 65535;
    TIM_TimeBaseSt.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseSt.TIM_Prescaler = 0;

    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseSt);

    /*设置TIM3的计数频率为 10MHz*/
    lsv_PrescalerValue = (unsigned short)((SystemCoreClock / 2) / TIM3_COUNTER_CLOCK) - 1;
    TIM_PrescalerConfig(TIM3, lsv_PrescalerValue, TIM_PSCReloadMode_Immediate);

    /*TIM3 中断配置*/
    NVIC_InitSt.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitSt.NVIC_IRQChannelPreemptionPriority = 6;
    NVIC_InitSt.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitSt.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitSt);

    /*分配内存*/
    if(gtp_BspPlsyChannelInfo == NULL)
    {
        gtp_BspPlsyChannelInfo = (bsp_plsy_ins_channel_info_st *)pvPortMalloc(sizeof(bsp_plsy_ins_channel_info_st) * MAX_PLSY_OUTPUT_CHANNEL_NUM);
        configASSERT(gtp_BspPlsyChannelInfo != NULL);
    }

    if(gtp_BspPlsyChannelInfo)
    {
        for(i = 0; i < MAX_PLSY_OUTPUT_CHANNEL_NUM; i++)
        {
            gtp_BspPlsyChannelInfo[i].mcv_Status = 0;
            gtp_BspPlsyChannelInfo[i].mcv_OutputLevel = 0;
            gtp_BspPlsyChannelInfo[i].msv_ccrValue = 0;
            gtp_BspPlsyChannelInfo[i].mlv_Freq = 0;
            gtp_BspPlsyChannelInfo[i].mlv_DestPulseNum = 0;
            gtp_BspPlsyChannelInfo[i].mlv_OutPulseCnt = 0;
        }
    }

}
#else
void bsp_tim3_init(void)
{
    LOGV("bsp_tim", "Enter %s()", __func__);
    //memset(gPlsyChannelInfo, 0, sizeof(gPlsyChannelInfo));
}
#endif

/**
  * @brief  TIM3四个通道输出比较模式配置
  * @param  None
  * @retval None
  */
#if 0 // TODO:
void bsp_set_tim3_output_compare_mode(unsigned char lcv_Channel, unsigned short lsv_ccrValue)
{
    TIM_OCInitTypeDef TIM_OCInitSt;

    TIM_OCInitSt.TIM_OCMode = TIM_OCMode_Timing;
    TIM_OCInitSt.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitSt.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitSt.TIM_Pulse = lsv_ccrValue;

    switch(lcv_Channel)
    {
    case 0:
        TIM_OC1Init(TIM3, &TIM_OCInitSt);
        TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Disable);
        break;

    case 1:
        TIM_OC2Init(TIM3, &TIM_OCInitSt);
        TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Disable);
        break;

    case 2:
        TIM_OC3Init(TIM3, &TIM_OCInitSt);
        TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Disable);
        break;

    case 3:
        TIM_OC4Init(TIM3, &TIM_OCInitSt);
        TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Disable);
        break;
    }
}
#else
void bsp_set_tim3_output_compare_mode(unsigned char lcv_Channel, unsigned short lsv_ccrValue)
{
    LOGV("bsp_tim", "Enter %s(), lcv_Channel = %u, lsv_ccrValue = %u", __func__, lcv_Channel, lsv_ccrValue);
}
#endif

#ifdef PWM_EXAMPLE
static void PWM_DRV_Init3PhPwm(void)
{
    uint16_t deadTimeVal;
    pwm_signal_param_t pwmSignal[2];
    uint32_t pwmSourceClockInHz;
    uint32_t pwmFrequencyInHz = 1000;

    pwmSourceClockInHz = PWM_SRC_CLK_FREQ;

    /* Set deadtime count, we set this to about 650ns */
    deadTimeVal = ((uint64_t)pwmSourceClockInHz * 650) / 1000000000;

    pwmSignal[0].pwmChannel       = kPWM_PwmA;
    pwmSignal[0].level            = kPWM_HighTrue;
    pwmSignal[0].dutyCyclePercent = 50; /* 1 percent dutycycle */
    pwmSignal[0].deadtimeValue    = deadTimeVal;
    pwmSignal[0].faultState       = kPWM_PwmFaultState0;

    pwmSignal[1].pwmChannel = kPWM_PwmB;
    pwmSignal[1].level      = kPWM_HighTrue;
    /* Dutycycle field of PWM B does not matter as we are running in PWM A complementary mode */
    pwmSignal[1].dutyCyclePercent = 50;
    pwmSignal[1].deadtimeValue    = deadTimeVal;
    pwmSignal[1].faultState       = kPWM_PwmFaultState0;

    /*********** PWMA_SM0 - phase A, configuration, setup 2 channel as an example ************/
    PWM_SetupPwm(BOARD_PWM_BASEADDR, kPWM_Module_0, pwmSignal, 2, kPWM_SignedCenterAligned, pwmFrequencyInHz,
                 pwmSourceClockInHz);

    /*********** PWMA_SM1 - phase B configuration, setup PWM A channel only ************/
    PWM_SetupPwm(BOARD_PWM_BASEADDR, kPWM_Module_1, pwmSignal, 1, kPWM_SignedCenterAligned, pwmFrequencyInHz,
                 pwmSourceClockInHz);

    /*********** PWMA_SM2 - phase C configuration, setup PWM A channel only ************/
    PWM_SetupPwm(BOARD_PWM_BASEADDR, kPWM_Module_2, pwmSignal, 1, kPWM_SignedCenterAligned, pwmFrequencyInHz,
                 pwmSourceClockInHz);
}
#else
static void PWM_DRV_Init3PhPwm(uint8_t channel, uint32_t freq, uint8_t dutyCyclePercent)
{
    uint16_t deadTimeVal;
    pwm_signal_param_t pwmSignal[1];
    uint32_t pwmSourceClockInHz;
    uint32_t pwmFrequencyInHz = freq;

    pwmSourceClockInHz = PWM_SRC_CLK_FREQ;
    LOGI("bsp_tim", "Enter %s(), pwmSourceClockInHz = %u, channel = %u", __func__, pwmSourceClockInHz, channel);

    /* Set deadtime count, we set this to about 650ns */
    deadTimeVal = ((uint64_t)pwmSourceClockInHz * 650) / 1000000000;
    LOGV("bsp_tim", "deadTimeVal = %u", deadTimeVal);

    pwm_submodule_t subm;
    pwm_channels_t pwmChannel;
    switch (channel)
    {
        case 0: // Y0
            subm = kPWM_Module_1; // IOMUXC_GPIO_SD_B0_02_FLEXPWM1_PWMA01
            pwmChannel = kPWM_PwmA;
            break;
        case 1: // Y1
            subm = kPWM_Module_1; // IOMUXC_GPIO_SD_B0_03_FLEXPWM1_PWMB01
            pwmChannel = kPWM_PwmB;
            break;
        default:
            subm = kPWM_Module_1;
            pwmChannel = kPWM_PwmA;
            break;
    }
    pwmSignal[0].pwmChannel       = pwmChannel;
    pwmSignal[0].level            = kPWM_HighTrue;
    pwmSignal[0].dutyCyclePercent = dutyCyclePercent; /* 1 percent dutycycle */
    pwmSignal[0].deadtimeValue    = deadTimeVal;
    pwmSignal[0].faultState       = kPWM_PwmFaultState0;

    LOGD("bsp_tim", "subm = %u, pwmChannel = %u", subm, pwmChannel);
    /*********** PWMA_SM1 - phase B configuration, setup PWM A channel only ************/
    PWM_SetupPwm(BOARD_PWM_BASEADDR, subm, pwmSignal, 1, kPWM_SignedCenterAligned, pwmFrequencyInHz, pwmSourceClockInHz);
}
#endif

static void bsp_tim_init_pwm_gpio(uint8_t channel)
{
    LOGV("bsp_tim", "Enter %s(), channel = %u", __func__, channel);
    switch (channel)
    {
        case 0:
            IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_02_FLEXPWM1_PWMA01, 0U);
            IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_02_FLEXPWM1_PWMA01, 0x10B0u);
            break;
        case 1:
            IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_03_FLEXPWM1_PWMB01, 0U);
            IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_03_FLEXPWM1_PWMB01, 0x10B0u);
            break;
    }
}

static void bsp_tim_uninit_pwm_gpio(uint8_t channel)
{
    LOGV("bsp_tim", "Enter %s(), channel = %u", __func__, channel);
    switch (channel)
    {
        case 0:
            IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_02_GPIO3_IO14, 0U);
            IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_02_GPIO3_IO14, 0x1008u);
            break;
        case 1:
            IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_03_GPIO3_IO15, 0U);
            IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_03_GPIO3_IO15, 0x1008u);
            break;
    }
}

/*
bool bsp_is_highspeed_running(uint8_t channel)
{
    if (gPlsyChannelInfo[channel].plsyStatus != 0)
    {
        return true;
    }
    if (gPwmInfo[channel].pwmStatus != 0)
    {
        return true;
    }
    if (gHspChannelInfo[channel].hspStatus != 0)
    {
        return true;
    }
  
    return false;
}
*/


bool bsp_is_plsy_running(uint8_t channel)
{
    if (gPlsyChannelInfo[channel].plsyStatus != 0)
    {
        return true;
    }
    return false;
}

void bsp_start_plsy(uint8_t plsyChannel, uint32_t plsyFreq, uint32_t plsyDestPulseNum)
{
    LOGV("bsp_tim", "Enter %s(), plsyChannel = %u, plsyFreq = %u, plsyDestPulseNum = %u", __func__, plsyChannel, plsyFreq, plsyDestPulseNum);
    LOGD("bsp_tim", "gPlsyChannelInfo[%u].curFreq = %u", plsyChannel, gPlsyChannelInfo[plsyChannel].curFreq);
    //如果频率相同则直接返回
    if (plsyFreq == gPlsyChannelInfo[plsyChannel].curFreq)
    {
        return;
    }

    IRQn_Type gptIRQ;
    GPT_Type *gpt;
    if (plsyChannel == 0) // Y0
    {
        gpt = GPT1;
        gptIRQ = GPT1_IRQn;
    }
    else if (plsyChannel == 1) // Y1
    {
        gpt = GPT2;
        gptIRQ = GPT2_IRQn;
    }
    if (gPlsyChannelInfo[plsyChannel].curFreq == 0)//初始化
    {
        gPlsyChannelInfo[plsyChannel].curFreq = plsyFreq;
        gPlsyChannelInfo[plsyChannel].destPulseNum = plsyDestPulseNum * 2;
        gPlsyChannelInfo[plsyChannel].outPulseCnt = 0;
        gPlsyChannelInfo[plsyChannel].plsyStatus = 1;
        gPlsyChannelInfo[plsyChannel].mcv_OutputLevel = 0;

        gpt_config_t gptConfig;
        GPT_GetDefaultConfig(&gptConfig);
        GPT_Init(gpt, &gptConfig);

        uint32_t gptFreq = CLOCK_GetFreq(kCLOCK_PerClk); //1,000,000 Hz
        LOGI("bsp_tim", "gptFreq = %uHz", gptFreq);
        /* 因为GPT的时钟频率为1,000,000Hz，所以1ms的counter值为1,000 */
        uint32_t cValue = gptFreq / (2 * plsyFreq);
        LOGI("bsp_tim", "cValue = %u", cValue);
        GPT_SetOutputCompareValue(gpt, kGPT_OutputCompare_Channel1, cValue);

        GPT_EnableInterrupts(gpt, kGPT_OutputCompare1InterruptEnable);
        EnableIRQ(gptIRQ);
        NVIC_SetPriority(gptIRQ, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
        GPT_StartTimer(gpt);
    }
    else// 频率改变了
    {
        gPlsyChannelInfo[plsyChannel].curFreq = plsyFreq;
        GPT_SetOutputCompareValue(gpt, kGPT_OutputCompare_Channel1, CLOCK_GetFreq(kCLOCK_PerClk) / (2 * plsyFreq));
    }
}

void bsp_stop_plsy(uint8_t channel)
{
    LOGV("bsp_tim", "Enter %s(), channel = %u", __func__, channel);
    IRQn_Type gptIRQ;
    GPT_Type *gpt;
    if (channel == 0) // Y0
    {
        gpt = GPT1;
        gptIRQ = GPT1_IRQn;
    }
    else if (channel == 1) // Y1
    {
        gpt = GPT2;
        gptIRQ = GPT2_IRQn;
    }
    GPT_DisableInterrupts(gpt, kGPT_OutputCompare2InterruptEnable);
    //GPT_StopTimer(gpt);
    GPT_Deinit(gpt);
    DisableIRQ(gptIRQ);
    memset(&gPlsyChannelInfo[channel], 0, sizeof(bsp_plsy_ins_channel_info_st));
}

void _bsp_start_plsy_channel(uint8_t lcv_Channel, uint32_t llv_Freq, uint32_t llv_DestPulseNum, bool pwmFlag, uint8_t dutyCyclePercent)
{
    LOGV("bsp_tim", "Enter %s(), lcv_Channel = %u, llv_Freq = %u, llv_DestPulseNum = %u", __func__, lcv_Channel, llv_Freq, llv_DestPulseNum);
    LOGD("bsp_tim", "pwmFlag = %u, dutyCyclePercent = %u", pwmFlag, dutyCyclePercent);

    //CLOCK_SetDiv(kCLOCK_AhbDiv, 0x2); /* Set AHB PODF to 2, divide by 3 */
    CLOCK_SetDiv(kCLOCK_IpgDiv, 0x3); /* Set IPG PODF to 3, divede by 4 */

    bsp_tim_init_pwm_gpio(lcv_Channel);
    
    /* Structure of initialize PWM */
    pwm_config_t pwmConfig;
    pwm_fault_param_t faultConfig;
    static uint16_t delay;
    uint32_t pwmVal = 4;
    uint16_t i;
#if 1
    /* Set the PWM Fault inputs to a low value */
    XBARA_Init(XBARA1);
    XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputLogicHigh, kXBARA1_OutputFlexpwm1Fault0);
    XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputLogicHigh, kXBARA1_OutputFlexpwm1Fault1);
    XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputLogicHigh, kXBARA1_OutputFlexpwm1234Fault2);
    XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputLogicHigh, kXBARA1_OutputFlexpwm1234Fault3);
#endif
    /*!
     * brief  Fill in the PWM config struct with the default settings
     *
     * The default values are:
     * code
     *   config->enableDebugMode = false;
     *   config->enableWait = false;
     *   config->reloadSelect = kPWM_LocalReload;
     *   config->clockSource = kPWM_BusClock;
     *   config->prescale = kPWM_Prescale_Divide_1;
     *   config->initializationControl = kPWM_Initialize_LocalSync;
     *   config->forceTrigger = kPWM_Force_Local;
     *   config->reloadFrequency = kPWM_LoadEveryOportunity;
     *   config->reloadLogic = kPWM_ReloadImmediate;
     *   config->pairOperation = kPWM_Independent;
     * endcode
     * param config Pointer to user's PWM config structure.
     */
    PWM_GetDefaultConfig(&pwmConfig);

    /* Use full cycle reload */
    pwmConfig.reloadLogic = kPWM_ReloadPwmFullCycle;
    pwmConfig.pairOperation   = kPWM_Independent;
    pwmConfig.prescale = kPWM_Prescale_Divide_128;
    //pwmConfig.enableDebugMode = true;

    pwm_submodule_t subm;
    switch (lcv_Channel)
    {
        case 0: // Y0
            subm = kPWM_Module_1; // IOMUXC_GPIO_SD_B0_02_FLEXPWM1_PWMA01
            break;
        case 1: // Y1
            subm = kPWM_Module_1; // IOMUXC_GPIO_SD_B0_03_FLEXPWM1_PWMB01
            break;
        default:
            subm = kPWM_Module_1;
            break;
    }
    /* Initialize submodule */
    if (PWM_Init(BOARD_PWM_BASEADDR, subm, &pwmConfig) == kStatus_Fail)
    {
        LOGE("bsp_tim", "PWM initialization failed");
        return ;
    }
#if 0
    /*
     *   config->faultClearingMode = kPWM_Automatic;
     *   config->faultLevel = false;
     *   config->enableCombinationalPath = true;
     *   config->recoverMode = kPWM_NoRecovery;
     */
    PWM_FaultDefaultConfig(&faultConfig);

#if (defined(FSL_FEATURE_PWM_FAULT_LEVEL_HIGH) && FSL_FEATURE_PWM_FAULT_LEVEL_HIGH)
    faultConfig.faultLevel = true;
asdf
#endif

    /* Sets up the PWM fault protection */
    PWM_SetupFaults(BOARD_PWM_BASEADDR, kPWM_Fault_0, &faultConfig);
    PWM_SetupFaults(BOARD_PWM_BASEADDR, kPWM_Fault_1, &faultConfig);
    PWM_SetupFaults(BOARD_PWM_BASEADDR, kPWM_Fault_2, &faultConfig);
    PWM_SetupFaults(BOARD_PWM_BASEADDR, kPWM_Fault_3, &faultConfig);

    /* Set PWM fault disable mapping for submodule 0/1/2 */
    PWM_SetupFaultDisableMap(BOARD_PWM_BASEADDR, kPWM_Module_0, kPWM_PwmA, kPWM_faultchannel_0,
                             kPWM_FaultDisable_0 | kPWM_FaultDisable_1 | kPWM_FaultDisable_2 | kPWM_FaultDisable_3);
    PWM_SetupFaultDisableMap(BOARD_PWM_BASEADDR, kPWM_Module_1, kPWM_PwmA, kPWM_faultchannel_0,
                             kPWM_FaultDisable_0 | kPWM_FaultDisable_1 | kPWM_FaultDisable_2 | kPWM_FaultDisable_3);
    PWM_SetupFaultDisableMap(BOARD_PWM_BASEADDR, kPWM_Module_2, kPWM_PwmA, kPWM_faultchannel_0,
                             kPWM_FaultDisable_0 | kPWM_FaultDisable_1 | kPWM_FaultDisable_2 | kPWM_FaultDisable_3);
#endif
    /* Call the init function with demo configuration */
    if (pwmFlag == true)
    {
        PWM_DRV_Init3PhPwm(lcv_Channel, llv_Freq, dutyCyclePercent);
    }
    else
    {
        PWM_DRV_Init3PhPwm(lcv_Channel, llv_Freq, 50);
    }
    uint8_t subModulesToUpdate;
    switch (lcv_Channel)
    {
        case 0: // Y0
            subModulesToUpdate = kPWM_Control_Module_1; // IOMUXC_GPIO_SD_B0_02_FLEXPWM1_PWMA01
            break;
        case 1: // Y1
            subModulesToUpdate = kPWM_Control_Module_1; // IOMUXC_GPIO_SD_B0_03_FLEXPWM1_PWMB01
            break;
        default:
            subModulesToUpdate = kPWM_Control_Module_1;
            break;
    }
    /* Set the load okay bit for all submodules to load registers from their buffer */
    PWM_SetPwmLdok(BOARD_PWM_BASEADDR, subModulesToUpdate, true);
    /* Start the PWM generation from Submodules 0, 1 and 2 */
    PWM_StartTimer(BOARD_PWM_BASEADDR, subModulesToUpdate);
#if 0
    delay = 0x0fffU;

    while (1U)
    {
        for (i = 0U; i < delay; i++)
        {
            __ASM volatile("nop");
        }
        pwmVal = pwmVal + 4;

        /* Reset the duty cycle percentage */
        if (pwmVal > 100)
        {
            pwmVal = 4;
        }

        /* Update duty cycles for all 3 PWM signals */
        PWM_UpdatePwmDutycycle(BOARD_PWM_BASEADDR, kPWM_Module_0, kPWM_PwmA, kPWM_SignedCenterAligned, pwmVal);
        PWM_UpdatePwmDutycycle(BOARD_PWM_BASEADDR, kPWM_Module_1, kPWM_PwmA, kPWM_SignedCenterAligned, (pwmVal >> 1));
        PWM_UpdatePwmDutycycle(BOARD_PWM_BASEADDR, kPWM_Module_2, kPWM_PwmA, kPWM_SignedCenterAligned, (pwmVal >> 2));

        /* Set the load okay bit for all submodules to load registers from their buffer */
        PWM_SetPwmLdok(BOARD_PWM_BASEADDR, kPWM_Control_Module_0 | kPWM_Control_Module_1 | kPWM_Control_Module_2, true);
    }
#endif
    Kalyke_PrintRunFrequency(1);
}

void bsp_start_plsy_channel(uint8_t lcv_Channel, uint32_t llv_Freq, uint32_t llv_DestPulseNum)
{
    _bsp_start_plsy_channel(lcv_Channel, llv_Freq, llv_DestPulseNum, false, 0);
}

void bsp_stop_plsy_channel(uint8_t channel)
{
    uint8_t module = 1;
    switch (channel)
    {
        case 0: // Y0
            module = kPWM_Control_Module_1; // IOMUXC_GPIO_SD_B0_02_FLEXPWM1_PWMA01
            break;
        case 1: // Y1
            module = kPWM_Control_Module_1; // IOMUXC_GPIO_SD_B0_03_FLEXPWM1_PWMB01
            break;
        default:
            module = kPWM_Control_Module_1;
            break;
    }
    PWM_StopTimer(BOARD_PWM_BASEADDR, module);

    bsp_tim_uninit_pwm_gpio(channel);
    gpio_pin_config_t sw_config_Y =
    {
        kGPIO_DigitalOutput, 0,
        kGPIO_NoIntmode,
    };
    GPIO_PinInit(Y0_GPIO, Y0_GPIO_PIN, &sw_config_Y);
    GPIO_PinInit(Y1_GPIO, Y1_GPIO_PIN, &sw_config_Y);
}


#if 0
void bsp_HSP_refreshDirection(uint8_t Channel, uint8_t Direction)
{
    if (gHspChannelInfo[Channel].destDirection== 0)     //输出方向
    {
        plc_set_bit_element_value(Y_ELEMENT, Direction, 0);
    }
    else
    {
        plc_set_bit_element_value(Y_ELEMENT, Direction, 1);
    }
}

#endif

void bsp_HSP_refreshDirection(uint8_t Channel, uint8_t Direction)
{
    if (gHspChannelInfo[Channel].destDirection== 0)     //输出方向
    {
        switch (Direction)
        {
            case 0:
//                GPIO_PortClear(Y0_GPIO, Y0_PIN_MASK);
                break;
            case 1:
//                GPIO_PortClear(Y1_GPIO, Y1_PIN_MASK);
                break;
            case 2:
//                GPIO_PortClear(Y2_GPIO, Y2_PIN_MASK);
                break;
            case 3:
//                GPIO_PortClear(Y3_GPIO, Y3_PIN_MASK);
                break;
            case 4:
                GPIO_PortClear(Y4_GPIO, Y4_PIN_MASK);
                break;
            case 5:
                GPIO_PortClear(Y5_GPIO, Y5_PIN_MASK);
                break;
            case 6:
                GPIO_PortClear(Y6_GPIO, Y6_PIN_MASK);
                break;
            case 7:
                GPIO_PortClear(Y7_GPIO, Y7_PIN_MASK);
                break;
            default:
                break;
        }
    }
    else
    {
        switch (Direction)
        {
            case 0:
//                GPIO_PortSet(Y0_GPIO, Y0_PIN_MASK);
                break;
            case 1:
//                GPIO_PortSet(Y1_GPIO, Y1_PIN_MASK);
                break;
            case 2:
//                GPIO_PortSet(Y2_GPIO, Y2_PIN_MASK);
                break;
            case 3:
//                GPIO_PortSet(Y3_GPIO, Y3_PIN_MASK);
                break;
            case 4:
                GPIO_PortSet(Y4_GPIO, Y4_PIN_MASK);
                break;
            case 5:
                GPIO_PortSet(Y5_GPIO, Y5_PIN_MASK);
                break;
            case 6:
                GPIO_PortSet(Y6_GPIO, Y6_PIN_MASK);
                break;
            case 7:
                GPIO_PortSet(Y7_GPIO, Y7_PIN_MASK);
                break;
            default:
                break;
        }
    }
}

void bsp_HSP_start(uint8_t Channel, uint32_t Frequency)
{
    IRQn_Type tmrIRQ;
    TMR_Type *tmr;

    switch (Channel)
    {
        case 0:
            tmr = TMR1;
            tmrIRQ = TMR1_IRQn;
            break;

        case 1:
            tmr = TMR2;
            tmrIRQ = TMR2_IRQn;
            break;

        case 2:
            tmr = TMR3;
            tmrIRQ = TMR3_IRQn;
            break;

        case 3:
            tmr = TMR4;
            tmrIRQ = TMR4_IRQn;
            break;

        default:
            break;
    }
    		
    gHspChannelInfo[Channel].curFreq = Frequency;
    gHspChannelInfo[Channel].outPulseCnt = 0;
    gHspChannelInfo[Channel].mcv_OutputLevel = 0;

    qtmr_config_t tmrConfig;
    QTMR_GetDefaultConfig(&tmrConfig);
    tmrConfig.primarySource = kQTMR_ClockDivide_128;
    QTMR_Init(tmr, kQTMR_Channel_0, &tmrConfig);

    uint32_t tmrFreq = CLOCK_GetFreq(kCLOCK_IpgClk); 
    LOGI("bsp_tim", "tmrFreq = %uHz", tmrFreq);
    uint32_t cValue = tmrFreq / (128 * 2 * Frequency);
    LOGI("bsp_tim", "cValue = %u", cValue);
    QTMR_SetTimerPeriod(tmr, kQTMR_Channel_0, cValue);

    QTMR_EnableInterrupts(tmr, kQTMR_Channel_0, kQTMR_CompareInterruptEnable);
    EnableIRQ(tmrIRQ);
    NVIC_SetPriority(tmrIRQ, 0);
    QTMR_StartTimer(tmr, kQTMR_Channel_0, kQTMR_PriSrcRiseEdge);
}

void bsp_HSP_changeFreq(uint8_t Channel,uint32_t Frequency)
{
    IRQn_Type tmrIRQ;
    TMR_Type *tmr;
    
    switch (Channel)
    {
        case 0:
            tmr = TMR1;
            tmrIRQ = TMR1_IRQn;
            break;

        case 1:
            tmr = TMR2;
            tmrIRQ = TMR2_IRQn;
            break;

        case 2:
            tmr = TMR3;
            tmrIRQ = TMR3_IRQn;
            break;

        case 3:
            tmr = TMR4;
            tmrIRQ = TMR4_IRQn;
            break;

        default:
            break;
    }

    uint32_t tmrFreq = CLOCK_GetFreq(kCLOCK_IpgClk); 
//    LOGI("bsp_tim", "tmrFreq = %uHz", tmrFreq);
    uint32_t cValue = tmrFreq / (128 * 2 * Frequency);
//    LOGI("bsp_tim", "cValue = %u", cValue);
    QTMR_SetTimerPeriod(tmr, kQTMR_Channel_0, cValue);
	
}

void bsp_HSP_stop(uint8_t Channel)
{
    IRQn_Type tmrIRQ;
    TMR_Type *tmr;
    switch (Channel)
    {
        case 0:
            tmr = TMR1;
            tmrIRQ = TMR1_IRQn;
            GPIO_PortClear(Y0_GPIO, Y0_PIN_MASK);
            break;

        case 1:
            tmr = TMR2;
            tmrIRQ = TMR2_IRQn;
            GPIO_PortClear(Y1_GPIO, Y1_PIN_MASK);
            break;

        case 2:
            tmr = TMR3;
            tmrIRQ = TMR3_IRQn;
            GPIO_PortClear(Y2_GPIO, Y2_PIN_MASK);
            break;

        case 3:
            tmr = TMR4;
            tmrIRQ = TMR4_IRQn;
            GPIO_PortClear(Y3_GPIO, Y3_PIN_MASK);
            break;

        default:
            break;
    }
    
    QTMR_DisableInterrupts(tmr, kQTMR_Channel_0, kQTMR_CompareInterruptEnable );
    QTMR_Deinit(tmr, kQTMR_Channel_0);
    DisableIRQ(tmrIRQ);
    
}

void Save_current_position(uint8_t Channel)
{
    gHspChannelInfo[Channel].location = (GET_SD_ELEMENT_VALUE(SD90+2*Channel+1)<<16)|(GET_SD_ELEMENT_VALUE(SD90+2*Channel));
}


void Refresh_current_position(uint8_t Channel)
{
    int32_t position;

//    if (gHspChannelInfo[Channel].hspStatus)
    {
        if (gHspChannelInfo[Channel].destDirection)
        {
            position = gHspChannelInfo[Channel].location + gHspChannelInfo[Channel].outPulseCnt/2;
        }
        else
        {
            position = gHspChannelInfo[Channel].location - gHspChannelInfo[Channel].outPulseCnt/2;
        }
        SET_SD_ELEMENT_VALUE((SD90+2*Channel),(position & 0x0000FFff));
        SET_SD_ELEMENT_VALUE((SD90+2*Channel+1),(position>>16));
        
    }
}


void bsp_start_pwm_channel(uint16_t channel, uint16_t pwmVal, uint16_t pwmCycle)
{
    LOGV("bsp_tim", "Enter %s(), pwmVal = %u, pwmCycle = %u", __func__, pwmVal, pwmCycle);
    uint8_t percent;
    uint32_t freq;

    double dFreq = 1.0f / ((double)pwmCycle / 1000.0f);
    freq = dFreq;

    percent = pwmVal * 100 / pwmCycle;
    _bsp_start_plsy_channel(channel, freq, 0, true, percent);
}

void bsp_stop_pwm_channel(uint8_t channel)
{
    //bsp_stop_plsy_channel(channel);
}

/**
  * @brief  TIM3中断服务函数
  * @param  None
  * @retval None
  */
#if 0
void TIM3_IRQHandler()
{
    unsigned short msv_Capture;

    if (TIM_GetITStatus(TIM3, TIM_IT_CC1) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);

        if(gtp_BspPlsyChannelInfo[0].mcv_OutputLevel)
        {
            GPIOJ->BSRRL = GPIO_Pin_8;
            gtp_BspPlsyChannelInfo[0].mcv_OutputLevel = 0;
        }
        else
        {
            GPIOJ->BSRRH = GPIO_Pin_8;
            gtp_BspPlsyChannelInfo[0].mcv_OutputLevel = 1;
        }

        gtp_BspPlsyChannelInfo[0].mlv_OutPulseCnt ++;
        SET_SD_ELEMENT_VALUE(80, (gtp_BspPlsyChannelInfo[1].mlv_OutPulseCnt / 2));

        if((gtp_BspPlsyChannelInfo[0].mlv_OutPulseCnt > gtp_BspPlsyChannelInfo[0].mlv_DestPulseNum * 2) ||
                !plc_get_bit_element_value(SM_ELEMENT, 80))
        {
            TIM_ITConfig(TIM3, TIM_IT_CC1, DISABLE);
            gtp_BspPlsyChannelInfo[0].mcv_Status = 0;
            /*清除脉冲输出标志*/
            plc_set_bit_element_value(SM_ELEMENT, 84, 0);
        }
        else
        {
            msv_Capture = TIM_GetCapture1(TIM3);
            TIM_SetCompare1(TIM3, msv_Capture + gtp_BspPlsyChannelInfo[0].msv_ccrValue);
        }
    }
    else if (TIM_GetITStatus(TIM3, TIM_IT_CC2) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);

        if(gtp_BspPlsyChannelInfo[1].mcv_OutputLevel)
        {
            GPIOJ->BSRRL = GPIO_Pin_9;
            gtp_BspPlsyChannelInfo[1].mcv_OutputLevel = 0;
        }
        else
        {
            GPIOJ->BSRRH = GPIO_Pin_9;
            gtp_BspPlsyChannelInfo[1].mcv_OutputLevel = 1;
        }

        gtp_BspPlsyChannelInfo[1].mlv_OutPulseCnt ++;
        SET_SD_ELEMENT_VALUE(81, (gtp_BspPlsyChannelInfo[1].mlv_OutPulseCnt / 2));

        if((gtp_BspPlsyChannelInfo[1].mlv_OutPulseCnt >= gtp_BspPlsyChannelInfo[1].mlv_DestPulseNum * 2) ||
                !plc_get_bit_element_value(SM_ELEMENT, 81))
        {
            TIM_ITConfig(TIM3, TIM_IT_CC2, DISABLE);
            gtp_BspPlsyChannelInfo[1].mcv_Status = 0;
            /*清除脉冲输出标志*/
            plc_set_bit_element_value(SM_ELEMENT, 85, 0);
        }
        else
        {
            msv_Capture = TIM_GetCapture2(TIM3);
            TIM_SetCompare2(TIM3, msv_Capture + gtp_BspPlsyChannelInfo[1].msv_ccrValue);
        }
    }
    else if (TIM_GetITStatus(TIM3, TIM_IT_CC3) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_CC3);

        if(gtp_BspPlsyChannelInfo[2].mcv_OutputLevel)
        {
            GPIOJ->BSRRL = GPIO_Pin_10;
            gtp_BspPlsyChannelInfo[2].mcv_OutputLevel = 0;
        }
        else
        {
            GPIOJ->BSRRH = GPIO_Pin_10;
            gtp_BspPlsyChannelInfo[2].mcv_OutputLevel = 1;
        }

        gtp_BspPlsyChannelInfo[2].mlv_OutPulseCnt ++;
        SET_SD_ELEMENT_VALUE(82, (gtp_BspPlsyChannelInfo[1].mlv_OutPulseCnt / 2));

        if((gtp_BspPlsyChannelInfo[2].mlv_OutPulseCnt >= gtp_BspPlsyChannelInfo[2].mlv_DestPulseNum * 2) ||
                !plc_get_bit_element_value(SM_ELEMENT, 82))
        {
            TIM_ITConfig(TIM3, TIM_IT_CC3, DISABLE);
            gtp_BspPlsyChannelInfo[2].mcv_Status = 0;
            /*清除脉冲输出标志*/
            plc_set_bit_element_value(SM_ELEMENT, 86, 0);
        }
        else
        {
            msv_Capture = TIM_GetCapture3(TIM3);
            TIM_SetCompare3(TIM3, msv_Capture + gtp_BspPlsyChannelInfo[2].msv_ccrValue);
        }
    }
    else
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_CC4);

        if(gtp_BspPlsyChannelInfo[3].mcv_OutputLevel)
        {
            GPIOJ->BSRRL = GPIO_Pin_11;
            gtp_BspPlsyChannelInfo[3].mcv_OutputLevel = 0;
        }
        else
        {
            GPIOJ->BSRRH = GPIO_Pin_11;
            gtp_BspPlsyChannelInfo[3].mcv_OutputLevel = 1;
        }

        gtp_BspPlsyChannelInfo[3].mlv_OutPulseCnt ++;
        SET_SD_ELEMENT_VALUE(83, (gtp_BspPlsyChannelInfo[1].mlv_OutPulseCnt / 2));

        if((gtp_BspPlsyChannelInfo[3].mlv_OutPulseCnt >= gtp_BspPlsyChannelInfo[3].mlv_DestPulseNum * 2) ||
                !plc_get_bit_element_value(SM_ELEMENT, 83))
        {
            TIM_ITConfig(TIM3, TIM_IT_CC4, DISABLE);
            gtp_BspPlsyChannelInfo[3].mcv_Status = 0;
            /*清除脉冲输出标志*/
            plc_set_bit_element_value(SM_ELEMENT, 87, 0);
        }
        else
        {
            msv_Capture = TIM_GetCapture4(TIM3);
            TIM_SetCompare4(TIM3, msv_Capture + gtp_BspPlsyChannelInfo[3].msv_ccrValue);
        }
    }

}
#endif

bool bsp_is_pwm_running(uint8_t channel)
{
    if (gPwmInfo[channel].pwmStatus != 0)
    {
        return true;
    }
    return false;
}

bool bsp_is_hsp_running(uint8_t channel)
{
    if (gHspChannelInfo[channel].hspStatus!= 0)
    {
        return true;
    }
    return false;
}



bool is_pwm_ms(void)
{
    if (!plc_get_bit_element_value(SM_ELEMENT, SM84))
    {
        return true;
    }
    return false;
}

void bsp_start_pwm(uint16_t channel, uint16_t pwmVal, uint16_t pwmCycle)
{
    LOGV("bsp_pwm", "Enter %s(), channel = %u, pwmVal = %u, pwmCycle = %u", __func__, channel, pwmVal, pwmCycle);
    LOGD("bsp_pwm", "curPwmVal = %u, pwmCycle = %u", gPwmInfo[channel].curPwmVal, gPwmInfo[channel].pwmCycle);

    IRQn_Type gptIRQ;
    GPT_Type *gpt;
    GPIO_Type *theGPIO;
    uint32_t theMask;
    if (channel == 0) // Y0
    {
        gpt = GPT1;
        gptIRQ = GPT1_IRQn;
        theGPIO = Y0_GPIO;
        theMask = Y0_PIN_MASK;
    }
    else if (channel == 1) // Y1
    {
        gpt = GPT2;
        gptIRQ = GPT2_IRQn;
        theGPIO = Y1_GPIO;
        theMask = Y1_PIN_MASK;
    }

    uint32_t gptFreq = CLOCK_GetFreq(kCLOCK_PerClk); //1,000,000 Hz
    LOGI("bsp_pwm", "gptFreq = %uHz", gptFreq);
    uint32_t cValueCycle;
    uint32_t cValueWidth;
    if (is_pwm_ms())
    {
        cValueCycle = MSEC_TO_COUNT(pwmCycle, gptFreq);
        cValueWidth = MSEC_TO_COUNT(pwmVal, gptFreq);
    }
    else
    {
        cValueCycle = USEC_TO_COUNT(pwmCycle, gptFreq);
        cValueWidth = USEC_TO_COUNT(pwmVal, gptFreq);
    }
    LOGI("bsp_pwm", "cValueCycle = %u, cValueWidth = %u", cValueCycle, cValueWidth);
    if (gPwmInfo[channel].pwmStatus == 0)//初始化
    {
        gPwmInfo[channel].pwmStatus = 1;
        gPwmInfo[channel].pwmCycle = pwmCycle;
        gPwmInfo[channel].curPwmVal = pwmVal;

        LOGI("bsp_pwm", "gpt = 0x%08X", gpt);
        gpt_config_t gptConfig;
        GPT_GetDefaultConfig(&gptConfig);
        GPT_Init(gpt, &gptConfig);

        /* 因为GPT的时钟频率为1,000,000Hz，所以1ms的counter值为1,000 */
        GPT_SetOutputCompareValue(gpt, kGPT_OutputCompare_Channel1, cValueCycle);
        GPT_SetOutputCompareValue(gpt, kGPT_OutputCompare_Channel2, cValueWidth);

        GPT_EnableInterrupts(gpt, kGPT_OutputCompare1InterruptEnable);
        GPT_EnableInterrupts(gpt, kGPT_OutputCompare2InterruptEnable);

        EnableIRQ(gptIRQ);
        NVIC_SetPriority(gptIRQ, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
        GPT_StartTimer(gpt);
        GPIO_PortSet(theGPIO, theMask); //先输出高电平
    }
    else
    {
        if ((pwmCycle != gPwmInfo[channel].pwmCycle) &&
            (pwmVal != gPwmInfo[channel].curPwmVal))
        {
            gPwmInfo[channel].pwmCycle = pwmCycle;
            gPwmInfo[channel].curPwmVal = pwmVal;

            LOGI("bsp_pwm", "cValueWidth1 = %u", cValueWidth);
            LOGD("bsp_pwm", "cValueCycle1 = %u", cValueCycle);
            GPT_SetOutputCompareValue(gpt, kGPT_OutputCompare_Channel2, cValueWidth);            
            GPT_SetOutputCompareValue(gpt, kGPT_OutputCompare_Channel1, cValueCycle);//只要往Channel1里面写数据，GPT这个timer就会重启
            GPIO_PortSet(theGPIO, theMask); //先输出高电平
        }
        else if (pwmVal != gPwmInfo[channel].curPwmVal)
        {
            gPwmInfo[channel].curPwmVal = pwmVal;

            LOGI("bsp_pwm", "cValueWidth2 = %u", cValueWidth);
            GPT_SetOutputCompareValue(gpt, kGPT_OutputCompare_Channel2, cValueWidth);
        }
        else if (pwmCycle != gPwmInfo[channel].pwmCycle)
        {
            gPwmInfo[channel].pwmCycle = pwmCycle;

            LOGD("bsp_pwm", "cValueCycle2 = %u", cValueCycle);
            GPT_SetOutputCompareValue(gpt, kGPT_OutputCompare_Channel1, cValueCycle);//只要往Channel1里面写数据，GPT这个timer就会重启
            GPIO_PortSet(theGPIO, theMask); //先输出高电平
        }
    }
}

void bsp_stop_pwm(uint8_t channel)
{
    LOGV("bsp_pwm", "Enter %s(), channel = %u", __func__, channel);
    IRQn_Type gptIRQ;
    GPT_Type *gpt;
    if (channel == 0) // Y0
    {
        gpt = GPT1;
        gptIRQ = GPT1_IRQn;
    }
    else if (channel == 1) // Y1
    {
        gpt = GPT2;
        gptIRQ = GPT2_IRQn;
    }
    GPT_StopTimer(gpt);
    GPT_DisableInterrupts(gpt, kGPT_OutputCompare1InterruptEnable);
    GPT_DisableInterrupts(gpt, kGPT_OutputCompare2InterruptEnable);
    DisableIRQ(gptIRQ);
    GPT_Deinit(gpt);

    memset(&gPwmInfo[channel], 0, sizeof(bsp_pwm_channel_info_st));
}

