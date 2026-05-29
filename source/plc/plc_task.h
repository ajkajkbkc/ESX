/**
  ******************************************************************************
  * @file    plc_task.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   创建系统运行中各任务
  ******************************************************************************
  */

#ifndef __PLC_TASK_H
#define __PLC_TASK_H

#include "FreeRTOS.h"
#include "task.h"

/*------------------------------------------------------------------------------
*   PLC task相关宏, 变量, 函数定义
*-----------------------------------------------------------------------------*/
extern TaskHandle_t gtv_PlcTaskHandler;
extern void plc_task(void *p_arg);
extern void plc_run(void);
extern void plc_re_run(void);

#endif /*__PLC_TASK_H*/

