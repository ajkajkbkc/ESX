/**
  ******************************************************************************
  * @file    plc_spd.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2020-01-11
  * @brief   
  ******************************************************************************
  */
#ifndef __PLC_SPD_H
#define __PLC_SPD_H
#include "FreeRTOS.h"
#include "task.h"

#define SPD_X_NUMBER    4 //X0 ~ X3

typedef struct _hs_spd
{
    volatile bool started; // true = had started
    uint8_t elemType;//保存脉冲数据个数的软元件类型：ADDR_D, ADDR_V, ADDR_R
    uint16_t address;//软元件地址
    uint32_t timeBegin;//本次开始时间
    uint32_t intCount; // 本次测试的中断数
} hs_spd_t;

extern hs_spd_t gSPD[SPD_X_NUMBER];

extern void spd_init(void);
extern void spd_deinit(void);
extern void spd_X0_start(uint16_t ms);
extern void spd_X1_start(uint16_t ms);
extern void spd_X2_start(uint16_t ms);
extern void spd_X3_start(uint16_t ms);
extern void spd_X0_stop(void);
extern void spd_X1_stop(void);
extern void spd_X2_stop(void);
extern void spd_X3_stop(void);

#endif /*__PLC_SPD_H*/

