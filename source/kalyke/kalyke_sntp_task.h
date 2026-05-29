/**
  ******************************************************************************
  * @file    kalyke_sntp_task.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-05-11
  * @brief   SNTP
  ******************************************************************************
  */
#ifndef __KALYKE_SNTP_TASK_H
#define __KALYKE_SNTP_TASK_H
#include "FreeRTOS.h"
#include "task.h"

extern void kalyke_sntp_task(void *p_arg);
extern void kalyke_ds1302_set_time(snvs_hp_rtc_datetime_t *g_rtcDate);
extern void kalyke_ds1302_init(void);
extern void kalyke_Synch_ds1302_time(void);
extern void kalyke_get_ds1302_times(snvs_hp_rtc_datetime_t *times);

extern TaskHandle_t gKalykeSNTPTaskHandle;

#endif /* __KALYKE_SNTP_TASK_H */

