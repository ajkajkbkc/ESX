/**
  ******************************************************************************
  * @file    plc_modbustcpins.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   MODBUS 相关指令实现
  ******************************************************************************
  */
#ifndef __PLC_MODBUSTCP_INS_H
#define __PLC_MODBUSTCP_INS_H

#include "plc_variable.h"

enum MBCCONNECT_ERROR
{
  MBCCONNECT_NO_ERROR                      = 0,
  MBCCONNECT_NO_LINK                       = 1,
  MBCCONNECT_IDENTIFICATION_CODE_ERROR     = 2,
  MBCCONNECT_NO_MORE_TCP_BUFFER            = 3,

  PING_SUCCESS                             = 4,
  PING_SEND_ERR                            = 5,
  PING_DROP                                = 6,
  PING_TIME_OUT                            = 7,
  PING_ERR_UNKNOW                          = 8
};

enum MBCLINK_ERROR
{
  MBCLINK_NO_ERROR            = 0,
  MBCLINK_ERROR_CLIENT_ID     = 1,
  MBCLINK_ERROR_SHEET_NUM     = 2,
  MBCLINK_ERROR_TABLE_TYPE    = 3,
  MBCLINK_ERROR_INS_NULL      = 4,
  MBCLINK_ERROR_INS_INVALID   = 5,
  MBCLINK_ERROR_BUFFER_OVER   = 6,
  MBCLINK_ERROR_BUFFER_OVER2  = 7,
  MBCLINK_ERROR_BUFFER_OVER3  = 8,
  MBCLINK_ERROR_BUFFER_OVER4  = 9,
  MBCLINK_ERROR_TIME_OUT      = 10
};

typedef struct _MBC_LINK_SEND
{
    uint8_t *pc; // 指向ucode的pc指针
    uint8_t flag;// 和不同modbus的功能码相对应
    uint8_t *resultData; //收到的寄存器、线圈等数据保存在这里
    uint8_t ifHaveData;// 1 = Some data had received.
}mbc_link_send_st;

typedef struct _MBC_CONN_ST
{
    uint8_t client_id;
    uint8_t *pc;
}mbc_conn_st;

typedef void (*pMbcconnCB)(uint8_t connState);

unsigned char run_ci_mbcconn_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_mbclink_ins(plc_run_power_flow_st *ltp_RunEnv);

extern void mbclink_save_Error_D2(uint8_t *pc, uint16_t val);
extern void mbclink_save_Done_D1(uint8_t *pc, uint8_t val);
#endif /*__PLC_MODBUSTCP_INS_H*/

