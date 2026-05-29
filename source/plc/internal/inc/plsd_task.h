/**
  ******************************************************************************
  * @file    plsd_task.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   掉电保持任务
  ******************************************************************************
  */
#ifndef __PLC_PLSD_TASK_H
#define __PLC_PLSD_TASK_H

/*------------------------------------------------------------------------------
*   plsd task相关宏, 变量, 函数定义
*-----------------------------------------------------------------------------*/
#define PLSD_TASK_STACK_SIZE    256
#define PLSD_TASK_PRIO          5

extern TaskHandle_t gtv_PlsdTaskHandler;

extern void plsd_task(void *p_arg);
extern void plsd_restore_data(void);
extern void plsd_save_data(void);
extern bool kalyke_is_system_low_power(void);
extern void plsd_erase_all(void);
#endif /*__PLC_PLSD_TASK_H*/  
