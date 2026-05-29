/**
  ******************************************************************************
  * @file    kalyke_ad_task.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-06-13
  * @brief   AD task
  ******************************************************************************
  */
#ifndef __KALYKE_AD_TASK_H
#define __KALYKE_AD_TASK_H
#include "FreeRTOS.h"
#include "task.h"

#define ADS8668_CHANNEL_NUMBERS  8 // ADS8668 has 8 channels.

extern void kalyke_start_AD(void);
extern void kalyke_stop_AD(void);
extern void kalyke_ad_task(void *p_arg);

extern TaskHandle_t gKalykeADTaskHandle;

#endif /* __KALYKE_AD_TASK_H */

