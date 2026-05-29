/**
  ******************************************************************************
  * @file    kalyke_tool.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-04-11
  * @brief   Some tools
  ******************************************************************************
  */
#ifndef __KALYKE_TOOL_H
#define __KALYKE_TOOL_H
#include "FreeRTOS.h"
#include "task.h"
#include "fsl_common.h"
#include "kalyke_opts.h"

/*取指针地址值*/
#define GET_POINT_ADDR(x)           ((unsigned long)(x))

#define GET_PU8_DATA(x)             (*((unsigned char *)(x)))
#define GET_PS8_DATA(x)             (*((char *)(x)))

#define GET_PU16_DATA(x)             (*((unsigned short *)(x)))
#define GET_PS16_DATA(x)             (*((short *)(x)))

#define GET_PU32_DATA(x)             (*((unsigned long *)(x)))
#define GET_PS32_DATA(x)             (*((long *)(x)))

/* 按照小端序取值*/
#define GET_SMLPU16_DATA(x)          (unsigned short)((*(x + 1)<<8) + (*(x)))
#define GET_SMLPU32_DATA(x)          (unsigned long)((*(x+3)<<24) + (*(x+2)<<16) + (*(x+1)<<8) + (*(x)))

/*按照大端序取值*/
#define GET_BIGPU16_DATA(x)          (unsigned short)((*(x)<<8) + (*(x+1)))
#define GET_BIGPU32_DATA(x)          (unsigned long)((*(x)<<24) + (*(x+1)<<16) + (*(x+2)<<8) + (*(x+3)))


#define SD_TASK_STACK_SIZE              512
#define SD_TASK_PRIO                    17

#define INTERRUPT_TASK_STACK_SIZE       1024
#define INTERRUPT_TASK_PRIO             16

#define UART_WORKER_4G_TASK_STACK_SIZE  512
#define UART_WORKER_4G_TASK_PRIORITY    13

#define UART_WORKER_TASK_STACK_SIZE     1024
#define UART_WORKER_TASK_PRIORITY       12

#define PLC_TASK_STACK_SIZE             512
#define PLC_TASK_PRIO                   11

#define COLLECT_TASK_STACK_SIZE         512
#define COLLECT_TASK_PRIO               12

#define PID_TASK_STACK_SIZE             512
#define PID_TASK_PRIO                   11

#define UART_TASK_STACK_SIZE            1024
#define UART_TASK_PRIO                  9

#define MODBUS_COM_TASK_STACK_SIZE      512
#define MODBUS_COM_TASK_PRIO            3

#define USB_TASK_STACK_SIZE             512
#define USB_TASK_PRIORITY               5
#define USB_DEVICE_TASK_STACK_SIZE      512
#define USB_DEVICE_TASK_PRIORITY        5

#define TASK_4G_TCP_STACK_SIZE          1024
#define TASK_4G_TCP_PRIO                7

#define KALYKE_TCPIP_THREAD_STACKSIZE   1024
#define KALYKE_TCPIP_THREAD_PRIO        5

#define TCPSERVER_TASK_STACK_SIZE       512
#define TCPSERVER_TASK_PRIO             4

#define MODBUSTCPTBL_TASK_STACK_SIZE    1024
#define MODBUSTCPTBL_TASK_PRIO          4

#define INTERNET_TASK_STACK_SIZE        512
#define INTERNET_TASK_PRIO              4

#define MQTT_TASK_STACK_SIZE            1024
#define MQTT_TASK_PRIO                  4

#define TASK_4G_STACK_SIZE              1024
#define TASK_4G_PRIO                    7

#define AD_TASK_STACK_SIZE              1024
#define AD_TASK_PRIO                    3

#define LED_TASK_STACK_SIZE             256
#define LED_TASK_PRIO                   2

#define MONITOR_TASK_STACK_SIZE         512
#define MONITOR_TASK_PRIO               3

#define DAISY_TASK_STACK_SIZE           1024
#define DAISY_TASK_PRIO                 14

#define DAISY_UART_TASK_STACK_SIZE      1024
#define DAISY_UART_TASK_PRIO            13

#define DAISY_UART3_TASK_STACK_SIZE     1024
#define DAISY_UART3_TASK_PRIO           5

#define SOEM_TASK_STACK_SIZE            3072
#define SOEM_TASK_PRIO                  14
#define SOEM_DATA_TASK_STACK_SIZE       512
#define SOEM_DATA_TASK_PRIO             15

#define SECOND_TASK_STACK_SIZE          512
#define SECOND_TASK_PRIO                6

#define PING_TASK_STACK_SIZE            1024
#define PING_TASK_PRIO                  3

#define OLED_KEY_TASK_STACK_SIZE        1024
#define OLED_KEY_TASK_PRIO              3


extern void hexdump(const void* p, size_t size);
extern void hexdump1(const void *p, size_t size);

extern uint32_t gFreeRTOSTick0;
extern uint32_t gCpuTick0;

/** 对于小于1ms的时间间隔的计算
 */
static inline void cpu_tick_start_count(void)
{
    gCpuTick0 = SysTick->VAL;
}

/** 返回纳秒
 * 对于主频是600,000,000Hz的MCU，一个时钟周期既是1/600,000,000秒
 * 约等于1.66667纳秒
 */
#if (RT1061_FREQUENCY_528M == 1)
static inline float cpu_tick_stop_count(void)
{
    return (gCpuTick0 - SysTick->VAL) * 1.893939f;
}
#else
static inline float cpu_tick_stop_count(void)
{
    return (gCpuTick0 - SysTick->VAL) * 1.666667f;
}
#endif

//对于大于1ms的时间间隔的计算
static inline void freertos_tick_start(void)
{
    gFreeRTOSTick0 = xTaskGetTickCount();
}
//返回毫秒
static inline uint32_t freertos_tick_stop(void)
{
    return (xTaskGetTickCount() - gFreeRTOSTick0);
}

static inline void kalyke_delay_20_ns(void)
{
    __asm("NOP");//2 ticks, 3.3ns
    __asm("NOP");//3 ticks
    __asm("NOP");//3 ticks, 5ns
    __asm("NOP");//4 ticks
    __asm("NOP");//4 ticks, 6.6ns
    __asm("NOP");//5 ticks
    __asm("NOP");//5 ticks, 8.3ns
    __asm("NOP");//6 ticks
    __asm("NOP");//6 ticks, 10ns

    __asm("NOP");//2 ticks, 3.3ns
    __asm("NOP");//3 ticks
    __asm("NOP");//3 ticks, 5ns
    __asm("NOP");//4 ticks
    __asm("NOP");//4 ticks, 6.6ns
    __asm("NOP");//5 ticks
    __asm("NOP");//5 ticks, 8.3ns
    __asm("NOP");//6 ticks
    __asm("NOP");//6 ticks, 10ns

    __asm("NOP");//2 ticks, 3.3ns
    __asm("NOP");//3 ticks
    __asm("NOP");//3 ticks, 5ns
    __asm("NOP");//4 ticks
    __asm("NOP");//4 ticks, 6.6ns
}

extern void suspend_task_when_download_ucode(void);
extern void resume_task_after_download_ucode(void);
extern void kalyke_delay_count(uint32_t count);
extern void kalyke_delay_count2(uint32_t count);
extern void kalyke_delay_ns(uint32_t ns);
extern void kalyke_delay_ns2(uint32_t ns);
extern void kalyke_delay_6_6_ns(void);
extern void kalyke_delay_1_6_ns(void);
extern void kalyke_delay_5_ns(void);
extern bool gSuspendFlag;

#endif /*__KALYKE_TOOL_H */

