/**
  ******************************************************************************
  * @file    kalyke_tool.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-04-11
  * @brief   Some tools
  ******************************************************************************
  */
#include "bsp_uart.h"
#include "plc_task.h"
#include "pid_task.h"
#include "kalyke_monitor_task.h"
#include "kalyke_internet_task.h"
#include "plc_interrupt.h"
#include "kalyke_sd_card_task.h"
#include "kalyke_ad_task.h"
#include "fsl_debug_console.h"
#include "plsd_task.h"
#include "daisy_task.h"
#include "daisy_uart_task.h"

uint32_t gCpuTick0;
uint32_t gFreeRTOSTick0;
bool gSuspendFlag = false;

void hexdump(const void *p, size_t size)
{
#if (SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_SDK) || (SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_TOOLCHAIN)
    const uint8_t *c = p;

    PRINTF("Dumping %u bytes from %p:\r\n", size, p);

    while (size > 0)
    {
        unsigned i;

        for (i = 0; i < 16; i++)
        {
            if (i < size)
                PRINTF("%02X ", c[i]);
            else
                PRINTF("   ");
        }
#if 0
        for (i = 0; i < 16; i++)
        {
            if (i < size)
                PRINTF("%c", c[i] >= 32 && c[i] < 127 ? c[i] : '.');
            else
                PRINTF(" ");
        }
#endif
        PRINTF("\r\n");

        c += 16;

        if (size <= 16)
            break;

        size -= 16;
    }
    PRINTF("\r\n");
#endif
}

void hexdump1(const void *p, size_t size)
{
#if (SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_SDK) || (SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_TOOLCHAIN)
    const uint8_t *c = p;
    PRINTF("Dumping %u bytes from %p:\r\n", size, p);
    while (size > 0)
    {
        unsigned i;

        for (i = 0; i < 16; i++)
        {
            if (i < size)
                PRINTF("0x%02X, ", c[i]);
            else
                PRINTF("   ");
        }
        PRINTF("\r\n");

        c += 16;

        if (size <= 16)
            break;

        size -= 16;
    }
    PRINTF("\r\n");
#endif
}



//RAMFUNCTION_SECTION_CODE(void suspend_task_when_download_ucode(void))
//void RAMFUNCTION_SECTION_CODE(suspend_task_when_download_ucode) (void)
void suspend_task_when_download_ucode(void)
{
    //taskENTER_CRITICAL();
    LOGD("tool", "Enter %s(), gSuspendFlag=%u", __func__, gSuspendFlag);
    if (gSuspendFlag == false)
    {
        gSuspendFlag = true;
        LOGV("tool", "gtv_PlsdTaskHandler = 0x%08X", gtv_PlsdTaskHandler);
        if (gtv_PlsdTaskHandler)
        {
            vTaskSuspend(gtv_PlsdTaskHandler);
        }
//        suspend_uart1_task();
    #if 0
        if (gKalykeADTaskHandle)
        {
            vTaskSuspend(gKalykeADTaskHandle);
        }
    #endif
        LOGD("tool", "gtv_PlcTaskHandler = 0x%08X", gtv_PlcTaskHandler);
        if (gtv_PlcTaskHandler)
        {
            vTaskSuspend(gtv_PlcTaskHandler);
        }
        LOGD("tool", "gPIDTaskHandler = 0x%08X", gPIDTaskHandler);
        if (gPIDTaskHandler)
        {
            vTaskSuspend(gPIDTaskHandler);
        }
        LOGI("tool", "gKalykeSecondTaskHandle = 0x%08X", gKalykeSecondTaskHandle);
        if (gKalykeSecondTaskHandle)
        {
            vTaskSuspend(gKalykeSecondTaskHandle);
        }
        LOGW("tool", "gKalykeInternetTaskHandle = 0x%08X", gKalykeInternetTaskHandle);
        if (gKalykeInternetTaskHandle)
        {
            vTaskSuspend(gKalykeInternetTaskHandle);
        }
        kalyke_ethernet_suspend();
        LOGV("tool", "gtv_InterruptTaskHandler = 0x%08X", gtv_InterruptTaskHandler);
        if (gtv_InterruptTaskHandler)
        {
            vTaskSuspend(gtv_InterruptTaskHandler);
        }

        LOGD("tool", "gKalykeSDCardTaskHandle = 0x%08X", gKalykeSDCardTaskHandle);
        if (gKalykeSDCardTaskHandle)
        {
            vTaskSuspend(gKalykeSDCardTaskHandle);
        }

        LOGI("tool", "gKalykeMonitorTaskHandle = 0x%08X", gKalykeMonitorTaskHandle);
        if (gKalykeMonitorTaskHandle)
        {
            vTaskSuspend(gKalykeMonitorTaskHandle);
        }
    #if 0
        LOGW("tool", "gDaisyTaskHandle = 0x%08X", gDaisyTaskHandle);
        if (gDaisyTaskHandle)
        {
            vTaskSuspend(gDaisyTaskHandle);
        }

        LOGW("tool", "gDaisyUartLoopTaskHandler = 0x%08X", gDaisyUartLoopTaskHandler);
        if (gDaisyUartLoopTaskHandler)
        {
            vTaskSuspend(gDaisyUartLoopTaskHandler);
        }
    #endif
    }
    //taskEXIT_CRITICAL();
}

void resume_task_after_download_ucode(void)
{
    LOGD("tool", "Enter %s, gSuspendFlag=%u", __func__, gSuspendFlag);
    //taskENTER_CRITICAL();
    if (gSuspendFlag == true)
    {
        gSuspendFlag = false;
        if (gKalykeMonitorTaskHandle)
        {
            vTaskResume(gKalykeMonitorTaskHandle);
        }
        if (gKalykeSDCardTaskHandle)
        {
            vTaskResume(gKalykeSDCardTaskHandle);
        }
        if (gtv_InterruptTaskHandler)
        {
            vTaskResume(gtv_InterruptTaskHandler);
        }
        
        kalyke_ethernet_resume();
        if (gKalykeInternetTaskHandle)
        {
            vTaskResume(gKalykeInternetTaskHandle);
        }
        if (gtv_PlcTaskHandler)
        {
            vTaskResume(gtv_PlcTaskHandler);
        }
        if (gPIDTaskHandler)
        {
            vTaskResume(gPIDTaskHandler);
        }
    #if 0
        if (gKalykeADTaskHandle)
        {
            vTaskResume(gKalykeADTaskHandle);
        }
    #endif
        if (gKalykeSecondTaskHandle)
        {
            vTaskResume(gKalykeSecondTaskHandle);
        }
        if (gtv_PlsdTaskHandler)
        {
            vTaskResume(gtv_PlsdTaskHandler);
        }
//        resume_uart1_task();
    #if 0
        if (gDaisyTaskHandle)
        {
            vTaskResume(gDaisyTaskHandle);
        }
        if (gDaisyUartLoopTaskHandler)
        {
            vTaskResume(gDaisyUartLoopTaskHandler);
        }
    #endif
    }
    //taskEXIT_CRITICAL();
}

/**
 * @param count while循环的次数.
 *
 * 根据观察，count从4开始就会出现规律：
 * count每增加1，tick增加2，时间增加3.3333333ns
 *
 * count = 0 -> 17 tick, 28.3333333ns
 * count = 1 -> 22 tick, 36.6666667ns
 * count = 2 -> 27 tick, 45ns
 * count = 3 -> 24 tick, 40ns
 * count = 4 -> 36 tick, 60ns
 *
 * 延时时间计算：
 * delay_time = ((count - 4) * 2 + 36) * 5 / 3 (每个时钟周期是5/3ns，即1.6666667ns)
 */
void kalyke_delay_count(uint32_t count)
{
    while (count--);
}
/**
 * @param ns 要延时的纳秒数 >= 70ns
 */
void kalyke_delay_ns(uint32_t ns)
{
    uint32_t count = (ns * 3 / 5 - 36) / 2 + 4 - 7;// + 1 - 8 = -7
    while (count--)
    {
        __asm("NOP");
    }
}

/**
 * @param count while循环的次数.
 *
 * 根据观察，count从6开始就会出现规律：
 * count每增加1，tick增加1，时间增加1.6666667ns
 *
 * count = 0 -> 15 tick, 25ns
 * count = 1 -> 19 tick, 31.6666667ns
 * count = 2 -> 23 tick, 38.3333333ns
 * count = 3 -> 24 tick, 40ns
 * count = 4 -> 25 tick, 41.6666667ns
 * count = 5 -> 21 tick, 35ns
 * count = 6 -> 32 tick, 53.3333333ns
 *
 * 延时时间计算：
 * delay_time = ((count - 6) * 1 + 32) * 5 / 3 (每个时钟周期是5/3ns，即1.6666667ns)
 */
void kalyke_delay_count2(uint32_t count)
{
    while (count--);
}
/**
 * @param ns 要延时的纳秒数 > 58ns
 */
void kalyke_delay_ns2(uint32_t ns)
{
    //uint32_t count = (ns * 3 / 5 - 32) + 6;
    //uint32_t count = ns * 3 / 5 - 26;
    uint32_t count = ns * 3 / 5 - 35;
    while (count--);
}

/* Delay 1.6666667ns, 1 tick */
void kalyke_delay_1_6_ns(void)
{
    __asm("NOP");
}

void kalyke_delay_5_ns(void)
{
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
}

void kalyke_delay_6_6_ns(void)
{
    __asm("NOP");//2 ticks, 3.3ns
    __asm("NOP");//3 ticks
    __asm("NOP");//3 ticks, 5ns
    __asm("NOP");//4 ticks
    __asm("NOP");//4 ticks, 6.6ns
}

void kalyke_delay_8_3_ns(void)
{
    __asm("NOP");//2 ticks, 3.3ns
    __asm("NOP");//3 ticks
    __asm("NOP");//3 ticks, 5ns
    __asm("NOP");//4 ticks
    __asm("NOP");//4 ticks, 6.6ns
    __asm("NOP");//5 ticks
    __asm("NOP");//5 ticks, 8.3ns
}



void kalyke_delay_11_6_ns(void)
{
    __asm("NOP");//2 ticks, 3.3ns
    __asm("NOP");//3 ticks
    __asm("NOP");//3 ticks, 5ns
    __asm("NOP");//4 ticks
    __asm("NOP");//4 ticks, 6.6ns
    __asm("NOP");//5 ticks
    __asm("NOP");//5 ticks, 8.3ns
    __asm("NOP");//6 ticks
    __asm("NOP");//6 ticks, 10ns
    __asm("NOP");//7 ticks
    __asm("NOP");//7 ticks, 11.6ns
}

void kalyke_delay_13_3_ns(void)
{
    __asm("NOP");//2 ticks, 3.3ns
    __asm("NOP");//3 ticks
    __asm("NOP");//3 ticks, 5ns
    __asm("NOP");//4 ticks
    __asm("NOP");//4 ticks, 6.6ns
    __asm("NOP");//5 ticks
    __asm("NOP");//5 ticks, 8.3ns
    __asm("NOP");//6 ticks
    __asm("NOP");//6 ticks, 10ns
    __asm("NOP");//7 ticks
    __asm("NOP");//7 ticks, 11.6ns
    __asm("NOP");//8 ticks
    __asm("NOP");//8 ticks, 13.3ns
}

