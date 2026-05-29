/**
  ******************************************************************************
  * @file    plc_flowctrlins.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   程序流控制指令
  ******************************************************************************
  */

#ifndef __PLC_FLOW_CTRL_INS_H
#define __PLC_FLOW_CTRL_INS_H
/*CALL 指令参数类型定义*/
enum __PLC_CALL_INS_PARA_TYPE_E {
    PARA_BOOL   = 0x01,
    PARA_WORD   = 0x02,
    PARA_INT    = 0x04,
    PARA_DWORD  = 0x08,
    PARA_DINT   = 0x10,
    PARA_REAL   = 0x20
};

unsigned char run_ci_for_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_next_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_continue_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_break_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_lbl_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_cj_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_fend_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_cfend_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_wdt_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_ei_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_di_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_iret_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_ciret_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_sret_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_stop_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned short plc_call_instruction_length(unsigned char *lcp_CallInsPtr);
unsigned char plc_get_call_ins_id(unsigned char  *lcp_Ucode, unsigned short *lsp_CallId);
unsigned char plc_call_ins_in_parameter_parse(unsigned char *lcp_BitCnt, unsigned char *lcp_WordCnt, unsigned char *lcp_CallInsPtr);
unsigned char plc_call_ins_out_parameter_parse(unsigned char lcv_BitCnt, unsigned char lcv_WordCnt, unsigned char *lcp_CallInsPtr);
unsigned char run_ci_call_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_csret_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_sret_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_sbr_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_mc_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_mcr_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_stl_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_set_s_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_out_s_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_rst_s_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_ret_ins(plc_run_power_flow_st *ltp_RunEnv);
#endif /*__PLC_FLOW_CTRL_INS_H*/
