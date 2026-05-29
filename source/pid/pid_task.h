/*
  ******************************************************************************
  * @file    pid_task.h
  * @author  zhanghuajie
  * @version V0.0.1
  * @date    2022-03-23
  * @brief
  ******************************************************************************
  */

#ifndef __PID_TASK_H
#define __PID_TASK_H

#include "FreeRTOS.h"
#include "task.h"

#include "plc_sysblock.h"

/*------------------------------------------------------------------------------
*   pid_task 相关宏, 变量, 函数定义
*-----------------------------------------------------------------------------*/
extern TaskHandle_t gPIDTaskHandler;

#define PID_CHANNEL_MAX             16          //PID最大通道数

#define PID_D_CONTROL               7999        //PID全局控制寄存器地址
#define CONTROL_READ_FLASH_MASK     0x0001      //读参数(从FLASH读到RAM)
#define CONTROL_WRITE_FLASH_MASK    0x0002      //写参数(从RAM写到FLASH)


//保存在D元件里的PID相对寄存器
enum __PID_D_REGISTER
{
    //基本控制参数寄存器
    PID_CONTROL_REG = 0,            //控制寄存器
    PID_PERIOD_REG,                 //PID周期
    PID_KP_REG,                     //比例
    PID_KI_REG,                     //积分
    PID_KD_REG,                     //微分

    //智能控制参数寄存器
    PID_SMART_REG = 6,              //智能模式
    PID_ERR_LARGE_REG,              //大偏差值
    PID_ERR_SMALL_REG,              //小偏差值
};


//定义PID控制参数数据结构体
typedef struct __PID_CONTROL_ST
{
    union
    {
        uint16_t PID_type;            //通道类型
        enum
        {
            PID_OFF = 0,            //关闭
            PID_INC,                //增量型
            PID_ABS,                //位置型
        };
    };
    uint16_t run_control_addr;      //启停控制元件地址,M元件地址
    uint16_t period_addr;          //控制周期地址
    uint16_t kp_addr;               //比例地址
    uint16_t ki_addr;               //积分地址
    uint16_t kd_addr;               //微分地址
} pid_control_st;

#define CONTROL_SELF_TUNING_MASK      0x0001      //自整定


//定义智能参数数据结构体
typedef struct __PID_SMART_ST
{
    uint16_t smart_type_addr;      //智能模式
    uint16_t err_big_addr;          //大偏差值
    uint16_t err_small_addr;        //小偏差值
} pid_smart_st;

#define SMART_TYPE_CLOSE      0     //智能模式关闭
#define SMART_TYPE_FUZZY      1     //模糊控制


//定义PID输入参数数据结构体
typedef struct __PID_IN_ST
{
    int16_t set_addr;              //设定值地址
    int16_t back_addr;             //反馈值地址
} pid_in_st;



//定义PID输出通道结构体
typedef struct __PID_OUT_CHANNEL_ST
{
    union
    {
        uint16_t channel_type;            //通道类型
        enum
        {
            DIGITAL_OUT= 0,             //DO
            ANALOG_OUT,                 //AO
        };
    };
    uint16_t channel_bit_addr;          //通道位地址
    uint16_t channel_digital_addr;      //通道字地址
    int16_t  channel_min;               //通道最小值
    int16_t  channel_max;               //通道最大值
} pid_out_channel_st;



//定义PID输出参数数据结构体
typedef struct __PID_OUT_ST
{
    union
    {
        uint16_t out_type;               //输出方式
        enum
        {
            SINGLE_OUT = 0,             //单路输出
            DOUBLE_OUT,                 //双路输出
        };
    };
    pid_out_channel_st out[2];          //输出通道参数
} pid_out_st;


//定义PID通道设置参数结构体
typedef struct __PID_CHANNEL_SET_ST
{
    uint16_t channel_number;
    pid_control_st pid_control;
    pid_smart_st pid_smart;
    pid_in_st pid_in;
    pid_out_st pid_out;
    uint16_t reserved1;
    uint16_t reserved2;
    uint16_t reserved3;
    uint16_t reserved4;
}pid_channel_set_st;


//定义时间监控结构体
typedef struct __TIME_MONITOR_ST
{
    uint32_t time_start;
    bool start_flag;
} time_moitor_st;



//定义ERR数据结构体
typedef struct __PID_ERR_ST
{
    int16_t err;
    int16_t err_next;
    int16_t err_last;
    int16_t err_cumsum;
} pid_err_st;

#if 0
extern pid_control_st pid_control[];
extern pid_smart_st pid_smart[];
extern pid_in_st pid_in[];
extern pid_out_st pid_out[];
extern pid_err_st pid_err[];
#endif


void pid_task(void *p_arg);
unsigned char pid_open_check(void);
unsigned char pid_read_parameter_1(void);
void pid_read_parameter_2(void);
void pid_write_parameter_1(void);
void pid_write_parameter_2(void);
void pid_digital_range_adjust(void);
void pid_parameter_init(uint8_t channel);
void pid_io_refresh(void);
void pid_run(void);
void pid_auto_tune(uint8_t channel);
void pid_calculate(uint8_t channel);
void pid_smart_run(void);
void pid_smart_fuzzy(uint8_t channel);


#endif

