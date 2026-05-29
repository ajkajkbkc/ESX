/**
  ******************************************************************************
  * @file    plc_simpleins.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   简单指令解释执行函数定义
  ******************************************************************************
  */

#ifndef __PLC_SIMPLE_INS_H
#define __PLC_SIMPLE_INS_H

unsigned char plc_exec_simple_instruction(plc_run_power_flow_st *ltp_RunEnv);
void run_eu_ins(plc_run_power_flow_st *ltp_RunEnv);
void run_ed_ins(plc_run_power_flow_st *ltp_RunEnv);
void run_eu_p_ins(plc_run_power_flow_st *ltp_RunEnv,unsigned char lsv_charValue);
void run_ed_f_ins(plc_run_power_flow_st *ltp_RunEnv,unsigned char lsv_charValue);

unsigned char get_char_in_simpleIns(unsigned char *lcp_UcodeAddr, unsigned char *lcp_Value);
unsigned char save_char_in_simpleIns(unsigned char *lcp_UcodeAddr, unsigned char lcp_Value);

#endif /*__PLC_SIMPLE_INS_H*/
