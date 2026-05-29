/**
  ******************************************************************************
  * @file    kalyke_sntp_task.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-05-11
  * @brief   SNTP
  ******************************************************************************
  */
#ifndef __KALYKE_4G_TASK_H
#define __KALYKE_4G_TASK_H

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


#define AT_SEND_MAX_BYTE  1440
#define AT_RECV_MAX_BYTE  (AT_SEND_MAX_BYTE + 128)


typedef struct _GPS_ST
{
    int gpsState;// 1=fixed
    int satelliteCount;
    uint32_t utc;
    double latitude;
    double longitude;
    double altitude;
}gps_st;


extern void kalyke_4G_task(void *p_arg);
extern void uart8_send_buffer(char *lcp_SendBuff, unsigned short lsv_Length);
extern void ec20_mqtt_publish(char *topic, char *payload, uint8_t qos);
extern void mqtt_4G_QMTDISC(void);
extern void mqtt_4G_rst(void);
extern void sendAT(char *pAT);
extern void init_UART8_4G(void);
extern int waitResponse(uint32_t timeout_ms,
                 char *r1, char *r2,
                 char *r3, char *r4, char *r5);
extern bool isEC20ModuleOK(void);
extern bool isSIMCardReady(void);
extern bool isNETRegister(void);
extern void at_QIACT_mqtt(void);
extern void init_uart8_receive_buffer(void);
extern void init_uart8_receive_buffer_small(void);
extern int waitResponsePublish(uint32_t timeout_ms,
                 char *r1, char *r2,
                 char *r3, char *r4, char *r5);
extern void closeATEchoMode(void);
extern void kalyke_stop_4G_for_PLC(void);

extern TaskHandle_t gKalyke4GTaskHandle;
extern volatile bool gIs4GMqttConnected;
extern gps_st gGpsValue;
extern SemaphoreHandle_t g4GMutex;
extern uint8_t gUart8RecvBuffer[AT_RECV_MAX_BYTE];
extern volatile uint8_t gUartStatus;
#endif /* __KALYKE_4G_TASK_H */

