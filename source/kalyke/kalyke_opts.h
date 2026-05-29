/**
  ******************************************************************************
  * @file    kalyke_opts.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-05-23
  * @brief   All Kalyke defined options put here.
  ******************************************************************************
  */
#ifndef __KALYKE_OPTS_H
#define __KALYKE_OPTS_H


//#define SLAVE_NUMBER        0x1A00 // FR-08R06
//#define SLAVE_NUMBER        0x1A01 // FR-16R14
//#define SLAVE_NUMBER        0x1A02 // FR-08R06AI08
//#define SLAVE_NUMBER        0x1A06 // FR-Adapter
#define SLAVE_NUMBER        0x1A10 // smart device



#define ETHERCAT_SOEM                      0
#define DAISY_MASTER_FEATURE               0

#define KALYKE_FEATURE_PLC_INTERRUPT_TASK  1
#define KALYKE_FEATURE_UART_TASK           1
#define KALYKE_FEATURE_USB                 0
#define KALYKE_FEATURE_PLC_TASK            1
#define KALYKE_FEATURE_COLLECT_TASK        1

#define KALYKE_FEATURE_OTA                 1
#define OTA_LOGIC                          1

#define KALYKE_FEATURE_SD_CARD_TASK        0
#define KALYKE_FEATURE_LED_TASK            1
#define KALYKE_FEATURE_MONITOR_TASK        1
#define KALYKE_FEATURE_AD_TASK             0

#define KALYKE_FEATURE_INTERNET_TASK       0
#define KALYKE_FEATURE_SNTP_TASK           0
#define USE_FIRST_ENET                     1
#define USE_SECOND_ENET                    1

#define KALYKE_FEATURE_4G_TASK             1
#define KALYKE_FEATURE_4G_TCP_TASK         1

#define KALYKE_FEATURE_LOW_POWER_TASK      0

#define INLINE_BIT_ELEMENT                 0
#define PLC_SCAN_TIME_LOGIC_2              1
#define WAN_MQTT_PUBLISH_IN_TCP_TASK       1
#define WAN_4G_SWITCH_AUTO                 0
#define KALYKE_HIGH_SPEED_IO               1
#define KALYKE_LP_RTC                      1
#define KALYKE_MODBUS_TCP_SHEET            1
#define KALYKE_TOUCHUAN_WAN_LAN            1

#define KALYKE_DS1302_FEATURE              0
#define ONLY_READ_TIME_FROM_DS1302         0

#define KALYKE_PING_FEATURE                0

#define KALYKE_FEATURE_OLED_KEY_TASK       1


#ifndef KALYKE_FEATURE_OTA
#define KALYKE_FEATURE_OTA                 0
#endif

#ifndef KALYKE_FEATURE_PLC_INTERRUPT_TASK
#define KALYKE_FEATURE_PLC_INTERRUPT_TASK  1
#endif

#ifndef KALYKE_FEATURE_UART_TASK
#define KALYKE_FEATURE_UART_TASK           1
#endif

#ifndef KALYKE_FEATURE_USB
#define KALYKE_FEATURE_USB                 0
#endif

#ifndef KALYKE_FEATURE_PLC_TASK
#define KALYKE_FEATURE_PLC_TASK            1
#endif

#ifndef KALYKE_FEATURE_COLLECT_TASK
#define KALYKE_FEATURE_COLLECT_TASK        1
#endif

#ifndef KALYKE_FEATURE_PID_TASK
#define KALYKE_FEATURE_PID_TASK            0
#endif

#ifndef KALYKE_FEATURE_SD_CARD_TASK
#define KALYKE_FEATURE_SD_CARD_TASK        0
#endif

#ifndef KALYKE_FEATURE_LED_TASK
#define KALYKE_FEATURE_LED_TASK            1
#endif

#ifndef KALYKE_FEATURE_MONITOR_TASK
#define KALYKE_FEATURE_MONITOR_TASK        0
#endif

#ifndef KALYKE_FEATURE_INTERNET_TASK
#define KALYKE_FEATURE_INTERNET_TASK       0
#endif

#ifndef KALYKE_FEATURE_SNTP_TASK
#define KALYKE_FEATURE_SNTP_TASK           0
#endif

#ifndef KALYKE_FEATURE_AD_TASK
#define KALYKE_FEATURE_AD_TASK             0
#endif

#ifndef KALYKE_FEATURE_4G_TASK
#define KALYKE_FEATURE_4G_TASK             0
#endif

#ifndef KALYKE_FEATURE_4G_TCP_TASK
#define KALYKE_FEATURE_4G_TCP_TASK         0
#endif

#ifndef KALYKE_FEATURE_LOW_POWER_TASK
#define KALYKE_FEATURE_LOW_POWER_TASK      0
#endif

#ifndef KALYKE_FEATURE_OLED_KEY_TASK
#define KALYKE_FEATURE_OLED_KEY_TASK       0
#endif

#ifndef LOG_OPEN
#define LOG_OPEN                           0
#endif
#ifndef LOG_OPEN_CONSOLE
#define LOG_OPEN_CONSOLE                   1 // 0 = DEBUGCONSOLE_REDIRECT_TO_TOOLCHAIN (Only printf work), 1 = DEBUGCONSOLE_REDIRECT_TO_SDK
#endif
#ifndef LOG_USE_LPUART3
#define LOG_USE_LPUART3                    0 // MCU_UART3_RS485_COM1
#endif
#ifndef LOG_USE_LPUART4
#define LOG_USE_LPUART4                    1 // MCU_UART4_RS485_COM0
#endif
#ifndef LOG_USE_LPUART8
#define LOG_USE_LPUART8                    0 // MCU_UART8
#endif


#ifndef USE_FIRST_ENET
#define USE_FIRST_ENET                     1
#endif

#ifndef USE_SECOND_ENET
#define USE_SECOND_ENET                    0
#endif

#ifndef INLINE_BIT_ELEMENT
#define INLINE_BIT_ELEMENT                 0
#endif

#ifndef PLC_SCAN_TIME_LOGIC_2
#define PLC_SCAN_TIME_LOGIC_2              1
#endif

#ifndef WAN_MQTT_PUBLISH_IN_TCP_TASK
#define WAN_MQTT_PUBLISH_IN_TCP_TASK       1
#endif

#ifndef WAN_4G_SWITCH_AUTO
#define WAN_4G_SWITCH_AUTO                 0
#endif

#ifndef OTA_LOGIC
#define OTA_LOGIC                          0
#endif

#ifndef KALYKE_HIGH_SPEED_IO
#define KALYKE_HIGH_SPEED_IO               0
#endif

#ifndef KALYKE_LP_RTC
#define KALYKE_LP_RTC                      0
#endif

#ifndef DAISY_MASTER_FEATURE
#define DAISY_MASTER_FEATURE               0
#endif

#ifndef DAISY_CONFIG_WHEN_LOOP
#define DAISY_CONFIG_WHEN_LOOP             0
#endif

#ifndef PLC_RUN_WAIT_DAISY
#define PLC_RUN_WAIT_DAISY                 1
#endif

#ifndef RTOS_DEBUG_QUEUE
#define RTOS_DEBUG_QUEUE                   0
#endif

#ifndef KALYKE_CJSON
#define KALYKE_CJSON                       1
#endif

#ifndef DEBUG_PORTENTERCRITICAL
#define DEBUG_PORTENTERCRITICAL            0
#endif

#ifndef DEBUG_SEMAPHORETAKE
#define DEBUG_SEMAPHORETAKE                0
#endif

#ifndef ETHERCAT_SOEM
#define ETHERCAT_SOEM                      1
#endif

#ifndef KALYKE_MODBUS_TCP_SHEET
#define KALYKE_MODBUS_TCP_SHEET            0
#endif

#ifndef KALYKE_TOUCHUAN_WAN_LAN
#define KALYKE_TOUCHUAN_WAN_LAN            0
#endif

#ifndef KALYKE_DS1302_FEATURE
#define KALYKE_DS1302_FEATURE              0
#endif
#ifndef ONLY_READ_TIME_FROM_DS1302
#define ONLY_READ_TIME_FROM_DS1302         0
#endif

#ifndef DAISY_UART3_WORKER_LOGIC
#define DAISY_UART3_WORKER_LOGIC           0
#endif

#ifndef DAISY_UART1_SEND_LOGIC
#define DAISY_UART1_SEND_LOGIC             1
#endif

#ifndef DAISY_UART3_RECV_LOGIC
#define DAISY_UART3_RECV_LOGIC             0
#endif

#ifndef KALYKE_PHY_TIMEOUT_COUNT
#define KALYKE_PHY_TIMEOUT_COUNT           0
#endif

#ifndef DAISY_BUS_BAUDRATE
#define DAISY_BUS_BAUDRATE                 4000000//921600//3686400//3686400//115200//230400//460800
#define DAISY_BUS_BAUDRATE_485             921600//1843200//3686400
#endif

#ifndef RT1061_FREQUENCY_528M
#define RT1061_FREQUENCY_528M              0  //0表示600M主频 1表示528M
#endif

#ifndef KALYKE_PING_FEATURE
#define KALYKE_PING_FEATURE                0
#endif

#endif /* __KALYKE_OPTS_H */

