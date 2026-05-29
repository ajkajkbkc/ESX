/**
  ******************************************************************************
  * @file    plc_io_interrupt.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2020-01-13
  * @brief   
  ******************************************************************************
  */
#ifndef __PLC_IO_INTERRUPT_H
#define __PLC_IO_INTERRUPT_H
#include "FreeRTOS.h"
#include "task.h"

extern void kalyke_highspped_init(void);
extern void X000_Interrupt(void);
extern void X001_Interrupt(void);
extern void X002_Interrupt(void);
extern void X003_Interrupt(void);
extern void X005_Interrupt(void);

#endif /*__PLC_IO_INTERRUPT_H*/

