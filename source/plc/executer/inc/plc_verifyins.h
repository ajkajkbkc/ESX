/**
  ******************************************************************************
  * @file    plc_verifyins.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   数据校验指令
  ******************************************************************************
  */

#ifndef __PLC_VERIFY_INS_H
#define __PLC_VERIFY_INS_H
unsigned char run_ci_ccitt_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_crc16_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_lrc_ins(plc_run_power_flow_st *ltp_RunEnv);

#endif /*__PLC_VERIFY_INS_H*/

