/**
  ******************************************************************************
  * @file    ether_cat_task.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2020-07-30
  * @brief
  ******************************************************************************
  */
#ifndef _ETHER_CAT_TASK_H
#define _ETHER_CAT_TASK_H
#include "FreeRTOS.h"
#include "task.h"
#include "mb.h"


typedef struct __SOEM_MSG_ST
{
    uint16_t wanOrLan; //0:WAN,  1:LAN
    uint16_t slaveID;  //设备地址（从站ID）
    uint16_t sdoIndex; //SDO索引
    uint16_t sdoSubIndex;
    uint16_t bufSize;  //读取长度
    uint32_t dataVal;  //数据存放地址
    //uint8_t ins_state; //指令执行状态，M寄存器
    int ret;   //指令执行错误码，D寄存器
}soem_msg_st;


//KOD - Keyword Object Dictionary
#define KOD_INDEX_1C12        0x1C12  //Sync Manager 2 PDO Assignment
#define KOD_INDEX_1C12_SUB_0  0       //Number of assigned RxPDO, UINT8
#define KOD_INDEX_1C12_SUB_1  1       //SubIndex 1, UINT16

#define KOD_INDEX_1C13        0x1C13  //Sync Manager 3 PDO Assignment
#define KOD_INDEX_1C13_SUB_0  0       //Number of assigned TxPDOs, UINT8
#define KOD_INDEX_1C13_SUB_1  1       //SubIndex 1, UINT16

#define KOD_INDEX_1600        0x1600  //Receive PDO Mapping Parameter 1
#define KOD_INDEX_1600_SUB_0  0       //Number of Entries, UINT8
#define KOD_INDEX_1600_SUB_1  1       //Mapping Entry 1, UINT32
#define KOD_INDEX_1600_SUB_2  2       //Mapping Entry 2, UINT32
#define KOD_INDEX_1600_SUB_3  3       //Mapping Entry 3, UINT32
#define KOD_INDEX_1600_SUB_4  4       //Mapping Entry 4, UINT32
#define KOD_INDEX_1600_SUB_5  5       //Mapping Entry 5, UINT32
#define KOD_INDEX_1600_SUB_6  6       //Mapping Entry 6, UINT32
#define KOD_INDEX_1600_SUB_7  7       //Mapping Entry 7, UINT32
#define KOD_INDEX_1600_SUB_8  8       //Mapping Entry 8, UINT32
#define KOD_INDEX_1600_SUB_9  9       //Mapping Entry 9, UINT32
#define KOD_INDEX_1600_SUB_10 10      //Mapping Entry 10, UINT32

#define KOD_INDEX_1601        0x1601  //Receive PDO Mapping Parameter 2
#define KOD_INDEX_1602        0x1602  //Receive PDO Mapping Parameter 3
#define KOD_INDEX_1603        0x1603  //Receive PDO Mapping Parameter 4

#define KOD_INDEX_1A00        0x1A00  //Transmit PDO Mapping Parameter 1
#define KOD_INDEX_1A00_SUB_0  0       //Number of Entries, UINT8
#define KOD_INDEX_1A00_SUB_1  1       //Mapping Entry 1, UINT32
#define KOD_INDEX_1A00_SUB_2  2       //Mapping Entry 2, UINT32
#define KOD_INDEX_1A00_SUB_3  3       //Mapping Entry 3, UINT32
#define KOD_INDEX_1A00_SUB_4  4       //Mapping Entry 4, UINT32
#define KOD_INDEX_1A00_SUB_5  5       //Mapping Entry 5, UINT32
#define KOD_INDEX_1A00_SUB_6  6       //Mapping Entry 6, UINT32
#define KOD_INDEX_1A00_SUB_7  7       //Mapping Entry 7, UINT32
#define KOD_INDEX_1A00_SUB_8  8       //Mapping Entry 8, UINT32
#define KOD_INDEX_1A00_SUB_9  9       //Mapping Entry 9, UINT32
#define KOD_INDEX_1A00_SUB_10 10      //Mapping Entry 10, UINT32

#define KOD_INDEX_1A01        0x1A01  //Transmit PDO Mapping Parameter 2
#define KOD_INDEX_1A02        0x1A02  //Transmit PDO Mapping Parameter 3
#define KOD_INDEX_1A03        0x1A03  //Transmit PDO Mapping Parameter 4

/* RxPDO */
#define KOD_INDEX_6040        0x6040  //Controlword, UINT16
#define KOD_INDEX_60FF        0x60FF  //Target Velocity,PUU/s, INT32
#define KOD_INDEX_607A        0x607A  //Target Position,PUU, INT32
#define KOD_INDEX_60FE        0x60FE  //Digital Outputs, ARRAY
#define KOD_INDEX_60FE_SUB_1  1       //Physical Outputs, UINT32
#define KOD_INDEX_6060        0x6060  //Modes of Operation, INT8

/* TxPDO */
#define KOD_INDEX_6041        0x6041  //Statusword, UINT16
#define KOD_INDEX_6064        0x6064  //Position Actual Value,PUU, INT32
#define KOD_INDEX_606C        0x606C  //Velocity Actual Value,PUU/s, INT32
#define KOD_INDEX_603F        0x603F  //Error Code, UINT16
#define KOD_INDEX_60FD        0x60FD  //Digital Inputs, UINT32



//extern void kalyke_ethercat_task(void *p_arg);
extern void kalyke_start_EtherCAT(void);
extern void soem_LAN_send(uint8_t *pBuf, uint16_t len);
extern void kalyke_slave_scan(md_slave_msg_pack *pMsg);
extern void soem_plc_sdo_read(soem_msg_st *p_msgSet);
extern void soem_plc_sdo_write(soem_msg_st *p_msgSet);

#endif /* _ETHER_CAT_TASK_H */

