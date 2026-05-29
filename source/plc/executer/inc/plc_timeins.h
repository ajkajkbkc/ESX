/**
  ******************************************************************************
  * @file    plc_timeins.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   时间相关指令函数
  ******************************************************************************
  */

#ifndef __PLC_TIME_INS_H
#define __PLC_TIME_INS_H

enum __PLC_TIMER_TYPE_E{
    /*接通延时计时*/
    TIME_TYPE_TON = 0x00,
    /*记忆型接通延时计时*/
    TIME_TYPE_TONR,
    /*断开接通延时*/
    TIME_TYPE_TOF,
    /*不重触发单稳计时*/
    TIME_TYPE_TMON,
};

enum __PLC_TIMER_ACCURACY_E{
    TIME_ACCURACY_1MS = 0x00,
    TIME_ACCURACY_10MS,
    TIME_ACCURACY_100MS,
};

void plc_stop_all_timer(void);
void plc_reset_one_timer(unsigned short lsv_TimeNum);
unsigned char run_ci_ton_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_tof_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_tonr_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_tmon_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_rst_t_ins(plc_run_power_flow_st *ltp_RunEnv);
void plc_refresh_10ms_1ms_timer(void);
void plc_reset_signal_alarm_element(void);
unsigned char run_ci_ans_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_anr_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_trd_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_twr_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_tadd_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_tsub_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_hour_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dcmp_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_tcmp_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_htos_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_stoh_ins(plc_run_power_flow_st *ltp_RunEnv);
void plc_hour_ins_data_init(void);
#endif /*__PLC_TIME_INS_H*/
