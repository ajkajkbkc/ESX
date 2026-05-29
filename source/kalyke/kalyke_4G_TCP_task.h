/**
  ******************************************************************************
  * @file    kalyke_sntp_task.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-05-11
  * @brief   SNTP
  ******************************************************************************
  */
#ifndef __KALYKE_4G_TCP_TASK_H
#define __KALYKE_4G_TCP_TASK_H

#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

extern void kalyke_4G_tcp_task(void *p_arg);
extern void reset_4G_heart_timer(void);
extern void at_QIDEACT(void);
extern bool check_QIACT(void);
extern void tcp_4G_start_busy_timer(void);


extern QueueHandle_t g4GTCPQueue;
extern volatile bool g4GTCPBusy;
#endif /* __KALYKE_4G_TCP_TASK_H */

