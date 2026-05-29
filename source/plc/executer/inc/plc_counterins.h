/**
  ******************************************************************************
  * @file    plc_counterins.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   计数器相关指令函数
  ******************************************************************************
  */
#ifndef __PLC_COUNTER_INS_H
#define __PLC_COUNTER_INS_H
void plc_reset_one_counter(unsigned short Element);
unsigned char run_ci_ctu_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_ctr_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dcnt_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_rst_c_ins(plc_run_power_flow_st *ltp_RunEnv);
#endif /*__PLC_COUNTER_INS_H*/
