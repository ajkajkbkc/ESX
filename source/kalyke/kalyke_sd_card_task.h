/**
  ******************************************************************************
  * @file    kalyke_sd_card_task.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-05-02
  * @brief   SD card
  ******************************************************************************
  */
#ifndef __KALYKE_SD_CARD_TASK_H
#define __KALYKE_SD_CARD_TASK_H
#include "FreeRTOS.h"
#include "task.h"

extern void kalyke_sd_task(void *p_arg);

extern TaskHandle_t gKalykeSDCardTaskHandle;
#endif /* __SD_CARD_TASK_H */

