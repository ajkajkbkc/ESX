/**
  ******************************************************************************
  * @file    plc_compiler.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   PLC由STOP切换到RUN状态时,编译UCODE代码
  ******************************************************************************
  */

#ifndef __PLC__COMPILER_H
#define __PLC__COMPILER_H

/*UCODE编译时所处理指令标志*/
enum __PLC_COMPILER_INS_FLAG{
    COMPILER_NORMAL_INS     = 0,
    COMPILER_STL_INS,
    COMPILER_MC_MCR_INS,
    COMPILER_SUB_INS
};

extern unsigned char plc_compiler_ucode(unsigned char *lcv_UCodePtr);
extern void plc_free_ins(void);
#endif /*__PLC__COMPILER_H*/

