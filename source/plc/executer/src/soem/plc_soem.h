/**
  ******************************************************************************
  * @file    plc_soem.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2020-09-11
  * @brief   SOEM 相关指令实现
  ******************************************************************************
  */
#ifndef __PLC_SOEM_INS_H
#define __PLC_SOEM_INS_H

#include "plc_variable.h"


extern unsigned char run_ci_ecatsdord_ins(plc_run_power_flow_st *ltp_RunEnv);
extern unsigned char run_ci_ecatsdowr_ins(plc_run_power_flow_st *ltp_RunEnv);

#endif /*__PLC_SOEM_INS_H*/

