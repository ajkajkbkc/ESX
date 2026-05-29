/**
  ******************************************************************************
  * @file    plc_sysinit.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   PLC 系统初始化
  ******************************************************************************
  */
#ifndef __PLC_SYS_INIT_H
#define __PLC_SYS_INIT_H

void plc_eu_ed_init(void);
void plc_HSP_init(void);
void plc_sys_init(void);
void plc_system_global_var_init(void);
void plc_sm_sd_init(void);
void plc_ucode_verify(unsigned char isSetUcodeErrFlag);
void plc_data_block_verify(unsigned char flag);
void plc_pou_info_verify(unsigned char flag);
void plc_system_block_verify(unsigned char flag);
void plc_netcfg_block_verify(unsigned char flag);
void plc_gvt_verify(unsigned char flag);
void plc_duty_ins_init(void);


extern unsigned long gUcodeLen;
#endif /*__PLC_SYS_INIT_H*/

