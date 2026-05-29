/**
  ******************************************************************************
  * @file    plc_strins.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   字符串相关指令函数
  ******************************************************************************
  */
#ifndef __PLC_STR_INS_H
#define __PLC_STR_INS_H
unsigned char run_ci_stradd_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_strlen_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_strright_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_strleft_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_strmidr_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_strmidw_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_strinstr_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_strmov_ins(plc_run_power_flow_st *ltp_RunEnv);
#endif /*__PLC_STR_INS_H*/

