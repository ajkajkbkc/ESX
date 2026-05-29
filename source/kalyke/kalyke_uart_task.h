/**
  ******************************************************************************
  * @file    kalyke_uart_task.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   串口任务
  ******************************************************************************
  */

#ifndef __KALYKE_UART_TASK_H
#define __KALYKE_UART_TASK_H
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/*uart任务消息队列结构体*/
typedef struct __UART_MSG_ST{
    /*设备UART端口号*/
    unsigned char mcv_UartPort;
    /*数据长度*/
    unsigned short msv_MsgLength;
    /*数据缓存区指针*/
    unsigned char *mcp_DataBuff;
}uart_msg_st;

extern QueueHandle_t gtv_UartTaskMsgQueueHandle;
/*------------------------------------------------------------------------------
*   UART task相关宏, 变量, 函数定义
*-----------------------------------------------------------------------------*/

extern TaskHandle_t gtv_UartTaskHandler;

extern void uart_task(void *p_arg);

#endif /* __KALYKE_UART_TASK_H */
