/**
  ******************************************************************************
  * @file    kalyke_monitor_task.h
  * @author  lixianyu
  * @version V0.0.2
  * @date    2019-05-02
  * @brief   通过调试串口打印出RTOS的剩余HEAP的字节数
             一些监控信息，比如温度等
  ******************************************************************************
  */
#ifndef __KALYKE_MONITOR_TASK_H
#define __KALYKE_MONITOR_TASK_H
#include "FreeRTOS.h"
#include "task.h"

extern volatile uint32_t gKalykeSecondTick;
extern volatile uint32_t gKalykeSecondTickCurrent;
extern volatile uint32_t gWorldSecondTick;
extern void kalyke_monitor_task(void *p_arg);

extern TaskHandle_t gKalykeMonitorTaskHandle;
extern TaskHandle_t gKalykeSecondTaskHandle;
extern void Kalyke_PrintRunFrequency(int32_t run_freq_only);
extern void init_SD_SW_version(void);
extern void kalyke_monitor_init(void);
extern void monitor_publish_now(void);
extern void print_vTaskList(const char *, int);
extern uint8_t get_WeekDayNum(uint32_t nYear, uint8_t nMonth, uint8_t nDay);
extern void Kalyke_PrintRunFrequency(int32_t run_freq_only);
extern void monitor_publish_now_JIONTECH_reply(char *id);
#endif /* __KALYKE_MONITOR_TASK_H */

