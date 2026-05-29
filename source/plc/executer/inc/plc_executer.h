/**
  ******************************************************************************
  * @file    plc_executer.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   PLC运行中相关函数定义
  ******************************************************************************
  */
#ifndef __PLC_EXECUTER_H
#define __PLC_EXECUTER_H

extern unsigned char *gcp_BackupUserPc;

extern void plc_sys_status_switch(void);
extern void plc_status_stop_to_run(void);
extern void plc_clean_user_run_error_flag(void);
extern void plc_run_deamon(void);
extern unsigned char plc_run_user_program(unsigned char *lcp_UcodeAddr, uint8_t flag);
extern void plc_status_run_to_stop(void);
extern void plc_status_stop(void);
extern void plc_reboot(void);
extern void plc_judge_run_over_time(void);
extern void plc_restore_data_block(void);
extern void plc_run_do(void);
#endif /*__PLC_EXECUTER_H*/
