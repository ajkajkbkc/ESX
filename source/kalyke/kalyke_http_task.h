/**
  ******************************************************************************
  * @file    kalyke_http_task.c
  * @author  pj
  * @version V0.0.1
  * @date    2024-04-01
  * @brief   http发送
  ******************************************************************************
  */
#ifndef __KALYKE_HTTP_TASK_H
#define __KALYKE_HTTP_TASK_H
#include "FreeRTOS.h"
#include "task.h"



extern void HTTP_POST(char *payload);

//typedef struct
//{
//u8 *pPackDataBuff; //数据包缓冲区
//u16 PackDataBuffSize; //数据包缓冲区大小
//u8 *pHttpHeadDataBuff; //Http头部信息缓冲区
//PROTOCOL_HANDWARE_INTERFACE *pHandwareInterface; //硬件接口
//RTU_CenTelAttr SendDataTelAttr; //多遥测站发送前需要进行设置
//u16 RxTimeOutMs; //接收超时时间，单位ms
//u8 SendRetry; //发送重试次数
//u8 ExtDataBuff[8]; //扩展区域存储区（用于缓存扩展的命令数据）
//}POST_HANDLE;


#endif /* __KALYKE_HTTP_TASK_H */

