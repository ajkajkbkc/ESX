/**
  ******************************************************************************
  * @file    plc_interrupt.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   用户中断处理模块
  ******************************************************************************
  */

#ifndef __PLC_INTERRUPT_H
#define __PLC_INTERRUPT_H
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/*中断优先级定义*/
enum __PLC_INTERRUPT_PRIORITY_E {
    /*低优先级*/
    INT_PRI_LOW = 0,
    /*高优先级*/
    INT_PRI_HIGH,
};

/*中断源定义*/
enum __PLC_INTERRUPT_SOURCE_E {
    /*X元件上升沿中断*/
    X0_UP_EDGE_INT  = 0x0,
    X1_UP_EDGE_INT,
    X2_UP_EDGE_INT,
    X3_UP_EDGE_INT,
    X4_UP_EDGE_INT,
    X5_UP_EDGE_INT,
    X6_UP_EDGE_INT,
    X7_UP_EDGE_INT,
    /*X元件下降沿中断*/
    X0_DOWN_EDGE_INT    = 8,
    X1_DOWN_EDGE_INT,
    X2_DOWN_EDGE_INT,
    X3_DOWN_EDGE_INT,
    X4_DOWN_EDGE_INT,
    X5_DOWN_EDGE_INT,
    X6_DOWN_EDGE_INT,
    X7_DOWN_EDGE_INT,
    /*定时中断0*/
    TIME_0_INT          = 16,
    TIME_1_INT,
    TIME_2_INT,	
    /*电源失电中断*/
    SYSTEM_POWER_LOSE_INT  = 19,
    /*COM0相关中断*/
    COM0_FRAME_SEND_INT    = 20,
    COM0_FRAME_RECV_INT,
    /*COM1相关中断*/
    COM1_FRAME_SEND_INT,
    COM1_FRAME_RECV_INT,
    /*COM1相关中断*/
    COM2_FRAME_SEND_INT,
    COM2_FRAME_RECV_INT,

    /*高速输出完成中断*/
    Y0_OUTPUT_FINISH = 26,
    Y1_OUTPUT_FINISH,
    Y2_OUTPUT_FINISH,
    Y3_OUTPUT_FINISH,
    Y4_OUTPUT_FINISH,
    Y5_OUTPUT_FINISH,
    Y6_OUTPUT_FINISH,
    Y7_OUTPUT_FINISH,

    /* 高速计数器中断 */
    HIGH_SPEED_0_INT = 34,
    HIGH_SPEED_1_INT,
    HIGH_SPEED_2_INT,
    HIGH_SPEED_3_INT,
    HIGH_SPEED_4_INT,
    HIGH_SPEED_5_INT,
    HIGH_SPEED_6_INT,
    HIGH_SPEED_7_INT,

    /*高速输出完成中断*/
    Y0_OUTPUT_POSITION = 42,
    Y1_OUTPUT_POSITION,
    Y2_OUTPUT_POSITION,
    Y3_OUTPUT_POSITION,
    Y4_OUTPUT_POSITION,
    Y5_OUTPUT_POSITION,
    Y6_OUTPUT_POSITION,
    Y7_OUTPUT_POSITION,

    TEMP_POSITION = 59,
    MAX_SYS_INT_NUM
};

/*中断源信息*/
typedef struct __PLC_INTERRUPT_SOURCE_ST{
    /*中断优先级*/
    unsigned char mcv_Priority;
    /*中断标志：bit0:是否允许多个未处理中断在队列中, bit1:中断使能*/
    unsigned char mcv_IntFlag;
    /*未处理中断次数*/
    unsigned char mcv_Count;
    /*中断程序参数*/
    unsigned char *mcp_IntPara;
    /*中断程序*/
    unsigned char (*pIntFunc)(unsigned char *mcp_Data, uint8_t flag);
}plc_interrupt_source_st;


/*系统中断信息*/
typedef struct __PLC_INTERRUPT_INFO_ST{
    /*中断使能标志*/
    unsigned char mcv_IntEnable;
    /*中断源状态*/
    plc_interrupt_source_st mtv_IntSource[MAX_SYS_INT_NUM];
}plc_interrupt_info_st;

extern plc_interrupt_info_st *gtp_InterruptInfo;

/*中断消息结构体*/
typedef struct __PLC_INTERRUPT_MSG_ST{
    /*中断号*/
    unsigned char mcv_IntNum;
    /*数据，保留待用*/
    void *mvp_Data;
}int_msg_st;

/*------------------------------------------------------------------------------
*   interrupt task相关宏, 变量, 函数定义
*-----------------------------------------------------------------------------*/
extern TaskHandle_t gtv_InterruptTaskHandler;

#define IS_INTERRUPT_ENABLE()   (gtp_InterruptInfo->mcv_IntEnable)

extern void plc_user_interrupt_init(void);
void plc_user_interrupt_enable(unsigned char lcv_Enable);
void plc_add_int_to_interrupt_queue(unsigned char lcv_IntNum, unsigned char lcv_IsFromISR);
void interrupt_task(void *p_arg);

#endif /*__PLC_INTERRUPT_H*/
