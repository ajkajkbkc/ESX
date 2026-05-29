/**
  ******************************************************************************
  * @file    kalyke_tcp_task.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   
  ******************************************************************************
  */
  
#ifndef _KALYKE_TCP_TASK_H
#define _KALYKE_TCP_TASK_H

#include "FreeRTOS.h"
#include "task.h"
#include "kalyke_opts.h"
#include "plc_netcfg.h"
#include "fsl_phy.h"


extern void joanna_tcp_send(uint8_t *pBuff, uint16_t len, uint8_t clientID, uint8_t *ucode_pc, uint8_t flag, uint8_t *recvBuf);
extern uint8_t tcp_client_get_received_bit(uint8_t clientID);
extern void tcp_client_set_received_bit(uint8_t clientID, uint8_t value);
extern void tcp_client_set_sending_bit(uint8_t clientID, uint8_t value);
extern uint8_t tcp_client_get_sending_bit(uint8_t clientID);

#endif /* _KALYKE_TCP_TASK_H */

