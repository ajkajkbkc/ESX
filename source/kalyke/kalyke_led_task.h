/**
  ******************************************************************************
  * @file    kalyke_led_task.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   系统状态LED维护任务
  ******************************************************************************
  */
#ifndef __KALYKE_LED_TASK_H
#define __KALYKE_LED_TASK_H
#include "FreeRTOS.h"
#include "task.h"

/*------------------------------------------------------------------------------
*   LED task相关宏, 变量, 函数定义
*-----------------------------------------------------------------------------*/

extern TaskHandle_t gtv_LedTaskHandler;
extern void led_task(void *p_arg);
#endif /*__KALYKE_LED_TASK_H*/

