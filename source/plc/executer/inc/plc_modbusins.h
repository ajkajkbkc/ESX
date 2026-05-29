/**
  ******************************************************************************
  * @file    plc_modbusins.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   MODBUS 相关指令实现
  ******************************************************************************
  */
#ifndef __PLC_MODBUS_INS_H
#define __PLC_MODBUS_INS_H

unsigned char run_ci_modbus_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_modlink_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_modrw_ins(plc_run_power_flow_st *ltp_RunEnv);
#endif /*__PLC_MODBUS_INS_H*/

