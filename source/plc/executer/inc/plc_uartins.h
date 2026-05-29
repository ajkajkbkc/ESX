/**
  ******************************************************************************
  * @file    plc_uartins.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   UART 相关指令实现
  ******************************************************************************
  */

#ifndef __PLC_UART_INS_H
#define __PLC_UART_INS_H

unsigned char run_ci_xmt_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_rcv_ins(plc_run_power_flow_st *ltp_RunEnv);
#endif /*__PLC_UART_INS_H*/
