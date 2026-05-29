/**
  ******************************************************************************
  * @file    kalyke_event.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-04-20
  * @brief   FreeRTOS event group for Kalyke project.
  ******************************************************************************
  */

#ifndef _KALYKE_EVENT_H_
#define _KALYKE_EVENT_H_

#include "FreeRTOS.h"
#include "event_groups.h"
#if 0 // Used for RTOS
#define BIT31   0x80000000
#define BIT30   0x40000000
#define BIT29   0x20000000
#define BIT28   0x10000000
#define BIT27   0x08000000
#define BIT26   0x04000000
#define BIT25   0x02000000
#define BIT24   0x01000000
#endif
#define BIT23   0x00800000UL
#define BIT22   0x00400000UL
#define BIT21   0x00200000UL
#define BIT20   0x00100000UL
#define BIT19   0x00080000UL
#define BIT18   0x00040000UL
#define BIT17   0x00020000UL
#define BIT16   0x00010000UL
#define BIT15   0x00008000UL
#define BIT14   0x00004000UL
#define BIT13   0x00002000UL
#define BIT12   0x00001000UL
#define BIT11   0x00000800UL
#define BIT10   0x00000400UL
#define BIT9    0x00000200UL
#define BIT8    0x00000100UL
#define BIT7    0x00000080UL
#define BIT6    0x00000040UL
#define BIT5    0x00000020UL
#define BIT4    0x00000010UL
#define BIT3    0x00000008UL
#define BIT2    0x00000004UL
#define BIT1    0x00000002UL
#define BIT0    0x00000001UL

/* The event group allows multiple bits for each event */
#define KALYKE_EVENT_GOT_IP_SNTP              BIT0
#define KALYKE_EVENT_GOT_IP_MQTT              BIT1
#define KALYKE_EVENT_GOT_IP_TCP_SERVER        BIT2
#define KALYKE_EVENT_GOT_IP_TCP_SERVER2       BIT3
#define KALYKE_EVENT_GOT_IP_TCP_SERVER3       BIT23


#define KALYKE_EVENT_MQTT_CONNECTED           BIT4
#define KALYKE_EVENT_MQTT_DISCONNECTED        BIT5

#define KALYKE_EVENT_OTA_START                BIT6

#define KALYKE_EVENT_AT_RESPONSE_OCCUR        BIT7

#define KALYKE_EVENT_ENET_INIT_DONE_LED       BIT8
#define KALYKE_EVENT_ENET_INIT_DONE_PLC       BIT9

#define KALYKE_EVENT_DAISY_WAIT_ID            BIT10
#define KALYKE_EVENT_UPGRADE_SLAVE            BIT11
#define KALYKE_EVENT_PLC_TASK_WAIT_DAISY      BIT12

#define KALYKE_EVENT_WAIT_MICROLINK_CONNECTED BIT13
#define KALYKE_EVENT_TCP_WAIT_4G_MQTT         BIT14

#define KALYKE_EVENT_MQTT_PUBLISH_NOW         BIT15

#define KALYKE_EVENT_WKC_ERROR                BIT16

#define KALYKE_EVENT_SOEM_PLC                 BIT17

#define DAISY_EVENT_WAIT_EXT_RECEIVE          BIT18
#define DAISY_EVENT_WAIT_SLAVE_RECEIVE        BIT19

//UART收到了从站的响应(bit20~23: com0~com3)实际上只用到COM0，COMx根据COM0偏移
#define KALYKE_EVENT_UART_COM0_HAD_RESP       BIT20
#define KALYKE_EVENT_UART_COM1_HAD_RESP       BIT21
#define KALYKE_EVENT_UART_COM2_HAD_RESP       BIT22
#define KALYKE_EVENT_UART_COM3_HAD_RESP       BIT23



extern EventGroupHandle_t g_kalyke_event_group;

#endif /* _KALYKE_EVENT_H_ */
