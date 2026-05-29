/**
  ******************************************************************************
  * @file    plc_spd.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2020-01-11
  * @brief   
  ******************************************************************************
  */
#include "fsl_debug_console.h"
#include "plc_spd.h"
#include "plc_variable.h"
#include "plc_parseaddr.h"

#include "fsl_gpt.h"
#include "fsl_qtmr.h"
#include "bsp_gpio.h"
#include "bsp_tim.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/


/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Variables
 ******************************************************************************/
static const char *TAG = "SPD";
hs_spd_t gSPD[SPD_X_NUMBER];


/*******************************************************************************
 * Code
 ******************************************************************************/
static inline void handleIrq(uint8_t xNum)
{
    switch (gSPD[xNum].elemType)
    {
        case ADDR_D:
            //LOGW(TAG, "gSPD[0].intCount = %u", gSPD[0].intCount);
            SET_D_ELEMENT_VALUE(gSPD[xNum].address, gSPD[xNum].intCount);
            break;
    }
    gSPD[xNum].intCount = 0;
    gSPD[xNum].timeBegin = xTaskGetTickCount();
}

/********************************* X0 ******************************/
void GPT1_IRQHandler(void)
{
    //LOGV(TAG, "Enter %s()", __func__);
    if (gPwmInfo[0].pwmStatus != 0)
    {
        if (GPT_GetStatusFlags(GPT1, kGPT_OutputCompare2Flag))
        {
            GPT_ClearStatusFlags(GPT1, kGPT_OutputCompare2Flag);
            GPIO_PortClear(Y0_GPIO, Y0_PIN_MASK);
        }
        else if (GPT_GetStatusFlags(GPT1, kGPT_OutputCompare1Flag))
        {
            GPT_ClearStatusFlags(GPT1, kGPT_OutputCompare1Flag);
            GPIO_PortSet(Y0_GPIO, Y0_PIN_MASK);
        }
        SDK_ISR_EXIT_BARRIER;
        return;
    }

    if (gPlsyChannelInfo[0].plsyStatus != 0)
    {
        if (GPT_GetStatusFlags(GPT1, kGPT_OutputCompare1Flag))
        {
            GPT_ClearStatusFlags(GPT1, kGPT_OutputCompare1Flag);

            if (gPlsyChannelInfo[0].destPulseNum != 0)
            {
                if (gPlsyChannelInfo[0].outPulseCnt < gPlsyChannelInfo[0].destPulseNum)
                {
                    GPIO_PortToggle(Y0_GPIO, Y0_PIN_MASK);
                    gPlsyChannelInfo[0].outPulseCnt++;
                }
                else if (gPlsyChannelInfo[0].outPulseCnt == gPlsyChannelInfo[0].destPulseNum)
                {
                    GPT_StopTimer(GPT1);
                    gPlsyChannelInfo[0].outPulseCnt++;
                    plc_set_bit_element_value(SM_ELEMENT, (SM80 + 0), 0);
                    GPIO_PortClear(Y0_GPIO, Y0_PIN_MASK);
                }
            }
            else
            {
                GPIO_PortToggle(Y0_GPIO, Y0_PIN_MASK);
            }
        }
        SDK_ISR_EXIT_BARRIER;
        return;
    }

    /* Clear interrupt flag.*/
    GPT_ClearStatusFlags(GPT1, kGPT_OutputCompare1Flag);

    handleIrq(0);

    SDK_ISR_EXIT_BARRIER;
}

void spd_X0_start(uint16_t ms)
{
    LOGV(TAG, "Enter %s()", __func__);
    gpt_config_t gptConfig;
    GPT_GetDefaultConfig(&gptConfig);
    /* Initialize GPT module */
    GPT_Init(GPT1, &gptConfig);
    uint32_t gptFreq = CLOCK_GetFreq(kCLOCK_PerClk); //1,000,000 Hz
    /* 因为GPT的时钟频率为1,000,000Hz，所以1ms的counter值为1,000 */
    GPT_SetOutputCompareValue(GPT1, kGPT_OutputCompare_Channel1, MSEC_TO_COUNT(ms, gptFreq));
    /* Enable GPT Output Compare1 interrupt */
    GPT_EnableInterrupts(GPT1, kGPT_OutputCompare1InterruptEnable);
    /* Enable at the Interrupt */
    EnableIRQ(GPT1_IRQn);

    GPT_StartTimer(GPT1);
    bsp_kalyke_enable_X_interrupt(0, kGPIO_IntRisingEdge);
    LOGV(TAG, "Leave %s()", __func__);
}
void spd_X0_stop(void)
{
    LOGV(TAG, "Enter %s()", __func__);
    if (gSPD[0].started == false)
    {
        return;
    }
    bsp_kalyke_disable_X_interrupt(0);
    GPT_DisableInterrupts(GPT1, kGPT_OutputCompare1InterruptEnable);
    //GPT_StopTimer(GPT1);
    GPT_Deinit(GPT1);
    DisableIRQ(GPT1_IRQn);
    
    LOGV(TAG, "Leave %s()", __func__);
}

/********************************* X1 ******************************/
void GPT2_IRQHandler(void)
{
    //LOGV(TAG, "Enter %s()", __func__);
    if (gPwmInfo[1].pwmStatus != 0)
    {
        if (GPT_GetStatusFlags(GPT2, kGPT_OutputCompare2Flag))
        {
            GPT_ClearStatusFlags(GPT2, kGPT_OutputCompare2Flag);
            GPIO_PortClear(Y1_GPIO, Y1_PIN_MASK);
        }
        else if (GPT_GetStatusFlags(GPT2, kGPT_OutputCompare1Flag))
        {
            GPT_ClearStatusFlags(GPT2, kGPT_OutputCompare1Flag);
            GPIO_PortSet(Y1_GPIO, Y1_PIN_MASK);
        }
        SDK_ISR_EXIT_BARRIER;
        return;
    }
    if (gPlsyChannelInfo[1].plsyStatus != 0)
    {
        if (GPT_GetStatusFlags(GPT2, kGPT_OutputCompare1Flag))
        {
            GPT_ClearStatusFlags(GPT2, kGPT_OutputCompare1Flag);

            if (gPlsyChannelInfo[1].destPulseNum != 0)
            {
                if (gPlsyChannelInfo[1].outPulseCnt < gPlsyChannelInfo[1].destPulseNum)
                {
                    GPIO_PortToggle(Y1_GPIO, Y1_PIN_MASK);
                    gPlsyChannelInfo[1].outPulseCnt++;
                }
                else if (gPlsyChannelInfo[1].outPulseCnt == gPlsyChannelInfo[1].destPulseNum)
                {
                    GPT_StopTimer(GPT2);
                    gPlsyChannelInfo[1].outPulseCnt++;
                    plc_set_bit_element_value(SM_ELEMENT, (SM80 + 1), 0);
                    GPIO_PortClear(Y1_GPIO, Y1_PIN_MASK);
                }
            }
            else
            {
                GPIO_PortToggle(Y1_GPIO, Y1_PIN_MASK);
            }
        }
        SDK_ISR_EXIT_BARRIER;
        return;
    }

    /* Clear interrupt flag.*/
    GPT_ClearStatusFlags(GPT2, kGPT_OutputCompare1Flag);

    handleIrq(1);

    SDK_ISR_EXIT_BARRIER;
}

void spd_X1_start(uint16_t ms)
{
    LOGV(TAG, "Enter %s()", __func__);
    gpt_config_t gptConfig;
    GPT_GetDefaultConfig(&gptConfig);
    /* Initialize GPT module */
    GPT_Init(GPT2, &gptConfig);
    uint32_t gptFreq = CLOCK_GetFreq(kCLOCK_PerClk); //1,000,000 Hz
    /* 因为GPT的时钟频率为1,000,000Hz，所以1ms的counter值为1,000 */
    GPT_SetOutputCompareValue(GPT2, kGPT_OutputCompare_Channel1, MSEC_TO_COUNT(ms, gptFreq));
    /* Enable GPT Output Compare1 interrupt */
    GPT_EnableInterrupts(GPT2, kGPT_OutputCompare1InterruptEnable);
    /* Enable at the Interrupt */
    EnableIRQ(GPT2_IRQn);

    GPT_StartTimer(GPT2);
    bsp_kalyke_enable_X_interrupt(1, kGPIO_IntRisingEdge);
    LOGV(TAG, "Leave %s()", __func__);
}
void spd_X1_stop(void)
{
    LOGV(TAG, "Enter %s()", __func__);
    if (gSPD[1].started == false)
    {
        return;
    }
    bsp_kalyke_disable_X_interrupt(1);
    GPT_DisableInterrupts(GPT2, kGPT_OutputCompare1InterruptEnable);
    //GPT_StopTimer(GPT2);
    GPT_Deinit(GPT2);
    DisableIRQ(GPT2_IRQn);
    LOGV(TAG, "Leave %s()", __func__);
}

void TMR1_IRQHandler(void)
{
    if (QTMR_GetStatus(TMR1, kQTMR_Channel_0)|kQTMR_CompareFlag)
    {
    	IOMUXC_GPR->GPR2 |= IOMUXC_GPR_GPR2_QTIMER1_TMR_CNTS_FREEZE_MASK;
        QTMR_ClearStatusFlags(TMR1, kQTMR_Channel_0, kQTMR_CompareFlag);
//        bsp_HSP_changeFreq(0,gHspChannelInfo[0].curFreq);
		QTMR_SetTimerPeriod(TMR1, kQTMR_Channel_0, 500000/gHspChannelInfo[0].curFreq);
        if (gHspChannelInfo[0].destPulseNum != 0)
        {
            if (gHspChannelInfo[0].outPulseCnt < gHspChannelInfo[0].destPulseNum)
            {
                GPIO_PortToggle(Y0_GPIO, Y0_PIN_MASK);
                gHspChannelInfo[0].outPulseCnt++;
            }
            else
            {
                GPIO_PortClear(Y0_GPIO, Y0_PIN_MASK);
                QTMR_DisableInterrupts(TMR1, kQTMR_Channel_0, kQTMR_CompareInterruptEnable );
                gHspChannelInfo[0].outPulseCnt++;
                plc_set_bit_element_value(SM_ELEMENT, (SM80 + 0), 0);
                gHspChannelInfo[0].hspStatus = HSP_STOP;
            }
        }
        else
        {
            GPIO_PortToggle(Y0_GPIO, Y0_PIN_MASK);
            gHspChannelInfo[0].outPulseCnt++;
        }
		IOMUXC_GPR->GPR2 &= ~IOMUXC_GPR_GPR2_QTIMER1_TMR_CNTS_FREEZE_MASK;
    }
    SDK_ISR_EXIT_BARRIER;
}

void TMR2_IRQHandler(void)
{
    if (QTMR_GetStatus(TMR2, kQTMR_Channel_0)|kQTMR_CompareFlag)
    {
    	IOMUXC_GPR->GPR2 |= IOMUXC_GPR_GPR2_QTIMER2_TMR_CNTS_FREEZE_MASK;
        QTMR_ClearStatusFlags(TMR2, kQTMR_Channel_0, kQTMR_CompareFlag);
//        bsp_HSP_changeFreq(1,gHspChannelInfo[1].curFreq);
		QTMR_SetTimerPeriod(TMR2, kQTMR_Channel_0, 500000/gHspChannelInfo[1].curFreq);
        if (gHspChannelInfo[1].destPulseNum != 0)
        {
            if (gHspChannelInfo[1].outPulseCnt < gHspChannelInfo[1].destPulseNum)
            {
                GPIO_PortToggle(Y1_GPIO, Y1_PIN_MASK);
                gHspChannelInfo[1].outPulseCnt++;
            }
            else
            {
                GPIO_PortClear(Y1_GPIO, Y1_PIN_MASK);
                QTMR_DisableInterrupts(TMR2, kQTMR_Channel_0, kQTMR_CompareInterruptEnable );
                gHspChannelInfo[1].outPulseCnt++;
                plc_set_bit_element_value(SM_ELEMENT, (SM80 + 1), 0);
                gHspChannelInfo[1].hspStatus = HSP_STOP;
            }
        }
        else
        {
            GPIO_PortToggle(Y1_GPIO, Y1_PIN_MASK);
            gHspChannelInfo[1].outPulseCnt++;
        }
		IOMUXC_GPR->GPR2 &= ~IOMUXC_GPR_GPR2_QTIMER2_TMR_CNTS_FREEZE_MASK;
    }
    SDK_ISR_EXIT_BARRIER;
}


/********************************* X2 ******************************/
#define X2_QTMR_BASEADDR     TMR3
/* Get source clock for QTMR driver */
#define QTMR_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_IpgClk) //150000000 Hz

void TMR3_IRQHandler(void)
{
    if (QTMR_GetStatus(TMR3, kQTMR_Channel_0)|kQTMR_CompareFlag)
    {
    	IOMUXC_GPR->GPR2 |= IOMUXC_GPR_GPR2_QTIMER3_TMR_CNTS_FREEZE_MASK;
        QTMR_ClearStatusFlags(TMR3, kQTMR_Channel_0, kQTMR_CompareFlag);
 //       bsp_HSP_changeFreq(2,gHspChannelInfo[2].curFreq);
		QTMR_SetTimerPeriod(TMR3, kQTMR_Channel_0, 500000/gHspChannelInfo[2].curFreq);
        if (gHspChannelInfo[2].destPulseNum != 0)
        {
            if (gHspChannelInfo[2].outPulseCnt < gHspChannelInfo[2].destPulseNum)
            {
                GPIO_PortToggle(Y2_GPIO, Y2_PIN_MASK);
                gHspChannelInfo[2].outPulseCnt++;
            }
            else
            {
                GPIO_PortClear(Y2_GPIO, Y2_PIN_MASK);
                QTMR_DisableInterrupts(TMR3, kQTMR_Channel_0, kQTMR_CompareInterruptEnable );
                gHspChannelInfo[2].outPulseCnt++;
                plc_set_bit_element_value(SM_ELEMENT, (SM80 + 2), 0);
                gHspChannelInfo[2].hspStatus = HSP_STOP;
            }
        }
        else
        {
            GPIO_PortToggle(Y2_GPIO, Y2_PIN_MASK);
            gHspChannelInfo[2].outPulseCnt++;
        }
    	IOMUXC_GPR->GPR2 &= ~IOMUXC_GPR_GPR2_QTIMER3_TMR_CNTS_FREEZE_MASK;
    }
    SDK_ISR_EXIT_BARRIER;
}

void spd_X2_start(uint16_t ms)
{
    LOGV(TAG, "Enter %s()", __func__);
    qtmr_config_t qtmrConfig;
    QTMR_GetDefaultConfig(&qtmrConfig);
    /* Init the first channel to use the IP Bus clock div by 128 */
    qtmrConfig.primarySource = kQTMR_ClockDivide_128; // 150000000 / 128 = 1171875 Hz
    QTMR_Init(X2_QTMR_BASEADDR, kQTMR_Channel_0, &qtmrConfig);

    /* Init the second channel to use output of the first channel as we are chaining the first channel and the second
     * channel */
    qtmrConfig.primarySource = kQTMR_ClockCounter0Output;
    QTMR_Init(X2_QTMR_BASEADDR, kQTMR_Channel_1, &qtmrConfig);

    /* Set the first channel period to be 1 millisecond */
    uint16_t ticks = MSEC_TO_COUNT(1U, (QTMR_SOURCE_CLOCK / 128));
    LOGW(TAG, "ticks of 1ms = %u", ticks);
    QTMR_SetTimerPeriod(X2_QTMR_BASEADDR, kQTMR_Channel_0, ticks);

    /* Set the second channel count which increases every millisecond, set compare event for ms millisecond. */
    QTMR_SetTimerPeriod(X2_QTMR_BASEADDR, kQTMR_Channel_1, ms);

    /* Enable at the NVIC */
    EnableIRQ(TMR3_IRQn);

    /* Enable the second channel compare interrupt */
    QTMR_EnableInterrupts(X2_QTMR_BASEADDR, kQTMR_Channel_1, kQTMR_CompareInterruptEnable);

    /* Start the second channel in cascase mode, chained to the first channel as set earlier via the primary source
     * selection */
    QTMR_StartTimer(X2_QTMR_BASEADDR, kQTMR_Channel_1, kQTMR_CascadeCount);

    /* Start the first channel to count on rising edge of the primary source clock */
    LOGV(TAG, "Before QTMR_StartTimer(X2_QTMR_BASEADDR, kQTMR_Channel_0, kQTMR_PriSrcRiseEdge)");
    QTMR_StartTimer(X2_QTMR_BASEADDR, kQTMR_Channel_0, kQTMR_PriSrcRiseEdge);

    bsp_kalyke_enable_X_interrupt(2, kGPIO_IntRisingEdge);
    LOGV(TAG, "Leave %s()", __func__);
}
void spd_X2_stop(void)
{
    LOGV(TAG, "Enter %s()", __func__);
    if (gSPD[2].started == false)
    {
        return;
    }
    bsp_kalyke_disable_X_interrupt(2);
    //QTMR_StopTimer(X2_QTMR_BASEADDR, kQTMR_Channel_0);
    //QTMR_StopTimer(X2_QTMR_BASEADDR, kQTMR_Channel_1);
    QTMR_Deinit(X2_QTMR_BASEADDR, kQTMR_Channel_0);
    QTMR_Deinit(X2_QTMR_BASEADDR, kQTMR_Channel_1);
    DisableIRQ(TMR3_IRQn);
    
    LOGV(TAG, "Leave %s()", __func__);
}

/********************************* X3 ******************************/
#define X3_QTMR_BASEADDR     TMR4

void TMR4_IRQHandler(void)
{
    if (QTMR_GetStatus(TMR4, kQTMR_Channel_0)|kQTMR_CompareFlag)
    {
    	IOMUXC_GPR->GPR2 |= IOMUXC_GPR_GPR2_QTIMER4_TMR_CNTS_FREEZE_MASK;
        QTMR_ClearStatusFlags(TMR4, kQTMR_Channel_0, kQTMR_CompareFlag);
//        bsp_HSP_changeFreq(3,gHspChannelInfo[3].curFreq);
		QTMR_SetTimerPeriod(TMR4, kQTMR_Channel_0, 500000/gHspChannelInfo[3].curFreq);
        if (gHspChannelInfo[3].destPulseNum != 0)
        {
            if (gHspChannelInfo[3].outPulseCnt < gHspChannelInfo[3].destPulseNum)
            {
                GPIO_PortToggle(Y3_GPIO, Y3_PIN_MASK);
                gHspChannelInfo[3].outPulseCnt++;
            }
            else
            {
                GPIO_PortClear(Y3_GPIO, Y3_PIN_MASK);
                QTMR_DisableInterrupts(TMR4, kQTMR_Channel_0, kQTMR_CompareInterruptEnable );
                gHspChannelInfo[3].outPulseCnt++;
                plc_set_bit_element_value(SM_ELEMENT, (SM80 + 3), 0);
                gHspChannelInfo[3].hspStatus = HSP_STOP;
            }
        }
        else
        {
            GPIO_PortToggle(Y3_GPIO, Y3_PIN_MASK);
            gHspChannelInfo[3].outPulseCnt++;
        }
		IOMUXC_GPR->GPR2 &= ~IOMUXC_GPR_GPR2_QTIMER4_TMR_CNTS_FREEZE_MASK;
    }
    SDK_ISR_EXIT_BARRIER;
}

void spd_X3_start(uint16_t ms)
{
    LOGV(TAG, "Enter %s()", __func__);
    qtmr_config_t qtmrConfig;
    QTMR_GetDefaultConfig(&qtmrConfig);
    /* Init the first channel to use the IP Bus clock div by 128 */
    qtmrConfig.primarySource = kQTMR_ClockDivide_128; // 150000000 / 128 = 1171875 Hz
    QTMR_Init(X3_QTMR_BASEADDR, kQTMR_Channel_0, &qtmrConfig);

    /* Init the second channel to use output of the first channel as we are chaining the first channel and the second
     * channel */
    qtmrConfig.primarySource = kQTMR_ClockCounter0Output;
    QTMR_Init(X3_QTMR_BASEADDR, kQTMR_Channel_1, &qtmrConfig);

    /* Set the first channel period to be 1 millisecond */
    uint16_t ticks = MSEC_TO_COUNT(1U, (QTMR_SOURCE_CLOCK / 128));
    LOGW(TAG, "ticks of 1ms = %u", ticks);
    QTMR_SetTimerPeriod(X3_QTMR_BASEADDR, kQTMR_Channel_0, ticks);

    /* Set the second channel count which increases every millisecond, set compare event for ms millisecond. */
    QTMR_SetTimerPeriod(X3_QTMR_BASEADDR, kQTMR_Channel_1, ms);

    /* Enable at the NVIC */
    EnableIRQ(TMR4_IRQn);

    /* Enable the second channel compare interrupt */
    QTMR_EnableInterrupts(X3_QTMR_BASEADDR, kQTMR_Channel_1, kQTMR_CompareInterruptEnable);

    /* Start the second channel in cascase mode, chained to the first channel as set earlier via the primary source
     * selection */
    QTMR_StartTimer(X3_QTMR_BASEADDR, kQTMR_Channel_1, kQTMR_CascadeCount);

    /* Start the first channel to count on rising edge of the primary source clock */
    LOGV(TAG, "Before QTMR_StartTimer(X3_QTMR_BASEADDR, kQTMR_Channel_0, kQTMR_PriSrcRiseEdge)");
    QTMR_StartTimer(X3_QTMR_BASEADDR, kQTMR_Channel_0, kQTMR_PriSrcRiseEdge);

    bsp_kalyke_enable_X_interrupt(3, kGPIO_IntRisingEdge);
    LOGV(TAG, "Leave %s()", __func__);
}

void spd_X3_stop(void)
{
    LOGV(TAG, "Enter %s()", __func__);
    if (gSPD[3].started == false)
    {
        return;
    }
    bsp_kalyke_disable_X_interrupt(3);
    //QTMR_StopTimer(X2_QTMR_BASEADDR, kQTMR_Channel_0);
    //QTMR_StopTimer(X2_QTMR_BASEADDR, kQTMR_Channel_1);
    QTMR_Deinit(X3_QTMR_BASEADDR, kQTMR_Channel_0);
    QTMR_Deinit(X3_QTMR_BASEADDR, kQTMR_Channel_1);
    DisableIRQ(TMR4_IRQn);
    
    LOGV(TAG, "Leave %s()", __func__);
}

void spd_init(void)
{
    LOGV(TAG, "Enter %s(), sizeof(gSPD) = %u", __func__, sizeof(gSPD));
    memset(gSPD, 0, sizeof(gSPD));
}

void spd_deinit(void)
{
    LOGD(TAG, "Enter %s()", __func__);
    for (int i = 0; i < SPD_X_NUMBER; i++)
    {
        switch (i)
        {
            case 0:
                spd_X0_stop();
                break;
            
            case 1:
                spd_X1_stop();
                break;
                
            case 2:
                spd_X2_stop();
                break;
                
            case 3:
                spd_X3_stop();
                break;
        }
    }
}
