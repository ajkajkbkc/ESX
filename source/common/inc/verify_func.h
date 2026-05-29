/**
  ******************************************************************************
  * @file    verify.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   常用校验算法
  ******************************************************************************
  */

#ifndef __COM_VERIFY_FUNC_H
#define __COM_VERIFY_FUNC_H
//#include "stm32f4xx.h"
#include "FreeRTOS.h"

extern unsigned char gcv_CRCHi[];
extern unsigned char gcv_CRCLo[];
extern unsigned short gsv_CcittCrcTbl[256];

unsigned short calc_ccitt(unsigned char *ptr, unsigned long len);
uint32_t calc_crc32(unsigned char *pu8In, uint32_t len);
unsigned short calc_crc16(unsigned char * lcp_Buff, unsigned short luv_Len);
#endif /*__COM_VERIFY_FUNC_H*/

