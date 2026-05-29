/**
  ******************************************************************************
  * @file    plc_tcpins.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2022-07-24
  * @brief   TCP_CONN, TCP_XMT, TCP_RCV 相关指令实现
  ******************************************************************************
  */
#ifndef __PLC_TCP_INS_H
#define __PLC_TCP_INS_H

#include "plc_variable.h"

/*
enum MBCCONNECT_ERROR
{
  MBCCONNECT_NO_ERROR                      = 0,
  MBCCONNECT_NO_LINK                       = 1,
  MBCCONNECT_IDENTIFICATION_CODE_ERROR     = 2,
  MBCCONNECT_NO_MORE_TCP_BUFFER            = 3,
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
*/

typedef struct _TCP_XMT
{
    uint8_t *pPc; // 指向ucode的pc指针
    uint8_t flags;// 和不同modbus的功能码相对应
    uint16_t *result; //收到的寄存器、线圈等数据保存在这里
} tcp_xmt_st;

typedef struct _TCP_CONN_ST
{
    uint8_t conn_id;
    uint8_t *p_PC;
} tcp_conn_st;

//typedef void (*pMbcconnCB)(uint8_t connState);

unsigned char run_ci_tcp_conn_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_tcp_xmt_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_tcp_rcv_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_tcp_ping_ins(plc_run_power_flow_st *ltp_RunEnv);

#endif /* __PLC_TCP_INS_H */

