/*
 * Copyright (c) 2022-2023, Fexlink Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-23     JPeng        first implementation
 */

#ifndef __KALYKE_COLLECT_TASK_H
#define __KALYKE_COLLECT_TASK_H

/* Includes ------------------------------------------------------------------*/
#include "task.h"
#include "fsl_gpio.h"

extern TaskHandle_t gtv_CollectTaskHandler;
extern volatile uint8_t collect_num, cur_addr, collect_complete;
extern volatile uint8_t gGetAtLeastOnce;
/* Private defines -----------------------------------------------------------*/
#define  MB_RTU_ADDR_MAX    0x20
#define  MAX_RTU_NUM        30   //最多连接终端个数

/* Production type -------------------------------------------- */
#define PARAM_DEP4              0
#define PARAM_ESM0              1
#define PARAM_ESM3              2
#define PARAM_E1E2              3         
#define PARAM_E2E2              4          
#define PARAM_E3E2              5          
#define PARAM_E4E2              6          
#define PARAM_ESF1              7          
#define PARAM_E2C1              8          
#define PARAM_E3C1              9          
#define PARAM_E2C3              10          
#define PARAM_E3C3              11          
#define PARAM_E1T2              12          
#define PARAM_E2T9              13          
#define PARAM_E1I2              14          
#define PARAM_E1B2              15          
#define PARAM_E2B2              16         
#define PARAM_E3B2              17         
#define PARAM_E4B2              18          
#define PARAM_E1A2              19          
#define PARAM_E2A2              20          
#define PARAM_E3A2              21          
#define PARAM_E4A2              22          
#define PARAM_E1P1              23  

#define  MAX_PRODEUCT_TYPE      24    //产品类型数量


/* Exported types ------------------------------------------------------------*/

typedef struct _RTU_INFO
{
    uint8_t RtuIdNum;      //终端站号
    uint8_t RtuIdInfo[22]; //终端ID号
    uint8_t RtuType;       //终端类型
} rtu_info_t;  //RTU终端信息

typedef struct _RTU_INFO_PACKET
{
    uint32_t RtuNum;
    rtu_info_t Rtu_Info[MAX_RTU_NUM];
} rtu_info_packet_t;  //RTU信息包

extern  uint8_t  gRtu_Num;
/* Private functions ---------------------------------------------------------*/
extern void GetAllRtuID(void);
extern void SETRtuID(void);
extern void ADDRtuID(void);
extern void SETRtuIDED(void);
extern void Handle_FindFlkRtu_RecvData(uint8_t *pBuf);
extern void collect_task(void);
#endif 