/**
  ******************************************************************************
  * @file    kalyke_ping_task.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2022-10-10
  * @brief   
  ******************************************************************************
  */
  
#ifndef _KALYKE_PING_TASK_H
#define _KALYKE_PING_TASK_H

#if 0
#include "FreeRTOS.h"
#include "task.h"
#include "kalyke_opts.h"
#include "plc_netcfg.h"
#include "fsl_phy.h"
#endif

extern void kalyke_start_ping(uint8_t *pc, ip4_addr_t pingTarget, uint16_t wanOrLan);
#endif /* _KALYKE_PING_TASK_H */

