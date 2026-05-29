/**
  ******************************************************************************
  * @file    bsp_iwdg.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   看门狗驱动
  ******************************************************************************
  */
#include "FreeRTOS.h"

#include "bsp_iwdg.h"
#include "fsl_debug_console.h"
#include "bsp_led.h"
#include "bsp_flash.h"
#include "plsd_task.h"
#include "kalyke_monitor_task.h"

/**
  * @brief  看门狗初始化
  * @param  None
  * @retval None
  */
void bsp_iwdg_init(void)
{

}

/**
  * @brief  看门狗使能
  * @param  None
  * @retval None
  */
void bsp_watch_dog_enable(void)
{
    wdog_config_t config;
    WDOG_GetDefaultConfig(&config);
    config.timeoutValue       = 0xFU; /* Timeout value is 8 sec [(timeoutValue+1)*0.5] */
    config.enableInterrupt    = true;
    config.interruptTimeValue = 0x4U; /* Interrupt occurred 2 sec before WDOG timeout. */
    WDOG_Init(KALYKE_WDOG_BASE, &config);
}

void WDOG1_IRQHandler(void)
{
    WDOG_ClearInterruptStatus(KALYKE_WDOG_BASE, kWDOG_InterruptFlag);
    //bsp_open_all_led();
    plsd_save_data();
    bsp_save_KalykeSecondTick(gKalykeSecondTick + 6);
}

