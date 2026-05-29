/**
  ******************************************************************************
  * @file    daisy_uart_task.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2021-08-19
  * @brief   背板总线串口任务
  ******************************************************************************
  */

#ifndef __DAISY_UART_TASK_H
#define __DAISY_UART_TASK_H
#include "FreeRTOS.h"
#include "timers.h"

extern uint8_t gGUHUAing;
extern TaskHandle_t gDaisyUartLoopTaskHandler;

extern void start_daisy_uart_task(void);
extern void uart3_callback_func(TimerHandle_t ltv_TimeHandle);


#endif /* __DAISY_UART_TASK_H */
