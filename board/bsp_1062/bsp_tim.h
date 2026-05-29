/**
  ******************************************************************************
  * @file    bsp_tim.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   定时器相关驱动
  ******************************************************************************
  */

#ifndef __BSP_TIM_H
#define __BSP_TIM_H

#include "fsl_common.h"

void bsp_cycle_clock_enable(unsigned char lcv_En);
void bsp_init_cycle_clock_tim(void);


#define MAX_PLSY_OUTPUT_CHANNEL_NUM     2
#define MAX_DRVI_OUTPUT_CHANNEL_NUM     4
//#define TIM3_COUNTER_CLOCK  42000000

/*plsy指令各通道信息结构体*/
typedef struct __BSP_PLSY_INS_CHANNEL_INFO_ST
{
    /*通道当前信息，0: 关闭, 1: 正在输出方波*/
    unsigned char plsyStatus;
    /*当前输出高低电平*/
    unsigned char mcv_OutputLevel;
    /*通道当前频率值*/
    unsigned long curFreq;
    /*目标脉冲数量*/
    unsigned long destPulseNum;
    /*已输出数量*/
    unsigned long outPulseCnt;
} bsp_plsy_ins_channel_info_st;

/*drvi指令各通道信息结构体*/
typedef struct __BSP_HSP_INS_CHANNEL_INFO_ST
{
    /*指令ID*/
    unsigned short orderID;
    union
    {
        /*通道状态信息*/
        unsigned char hspStatus;
        enum
        {
            HSP_CLOSE = 0,          //
            HSP_START,              //
            HSP_ON,                 //
            HSP_LOW_SPEED,

            HSP_STOP = 100,          //
            HSP_END,          //
        };
    };
    /*当前输出高低电平*/
    unsigned char mcv_OutputLevel;
    /*目标脉冲方向，0：负方向，1：正方向*/
    unsigned char destDirection;
    /*当前频率变化方向，0：负方向，1：正方向*/
    unsigned char freqDirection;
    /*当前频率值*/
    unsigned long curFreq;
    /*目标频率值*/
    unsigned long destFreq;
    /*步进频率值*/
    unsigned long stepFreq;
    /*基底频率值*/
    unsigned long baseFreq;    
    /*原点回归爬行频率值*/
    unsigned long slowFreq;
    /*目标脉冲数量*/
    unsigned long destPulseNum;
    /*已输出数量*/
    unsigned long outPulseCnt;
    /*加到目标频率的脉冲数量*/
    unsigned long frepPulseCnt;
    /*变频时间*/
    TickType_t freqTick;
    /*沿ID*/
    unsigned short edgeID;
    /*启动定位前的当前位置*/
    signed long location;
} bsp_hsp_ins_channel_info_st;


#define MAX_PWM_OUTPUT_CHANNEL_NUM     2

/* PWM指令各通道信息结构体 */
typedef struct __BSP_PWM_CHANNEL_INFO_ST
{
    /* 通道当前信息，0: 关闭, 1: 正在输出方波 */
    unsigned char pwmStatus;
    /* 脉冲宽度 */
    unsigned short curPwmVal;
    /* 脉冲周期 */
    unsigned short pwmCycle;

} bsp_pwm_channel_info_st;


extern bsp_pwm_channel_info_st gPwmInfo[MAX_PWM_OUTPUT_CHANNEL_NUM];
extern bsp_plsy_ins_channel_info_st gPlsyChannelInfo[MAX_PLSY_OUTPUT_CHANNEL_NUM];
extern bsp_hsp_ins_channel_info_st gHspChannelInfo[MAX_DRVI_OUTPUT_CHANNEL_NUM];


extern void bsp_tim3_init(void);
extern void bsp_set_tim3_output_compare_mode(unsigned char lcv_Channel, unsigned short lsv_ccrValue);
extern void bsp_start_plsy_channel(uint8_t lcv_Channel, uint32_t llv_Freq, uint32_t llv_DestPulseNum);
extern void bsp_stop_plsy_channel(uint8_t channel);
extern void bsp_start_pwm_channel(uint16_t channel, uint16_t pwmVal, uint16_t pwmCycle);
extern void bsp_stop_pwm_channel(uint8_t channel);

extern void bsp_start_plsy(uint8_t lcv_Channel, uint32_t llv_Freq, uint32_t llv_DestPulseNum);
extern void bsp_HSP_refreshDirection(uint8_t Channel, uint8_t Direction);
extern void bsp_HSP_start(uint8_t Channel, uint32_t Frequency);
extern void bsp_HSP_changeFreq(uint8_t Channel,uint32_t Frequency);

extern void bsp_stop_plsy(uint8_t channel);
extern void bsp_HSP_stop(uint8_t channel);
extern void Save_current_position(uint8_t Channel);
extern void Refresh_current_position(uint8_t Channel);
extern bool bsp_is_plsy_running(uint8_t channel);
//extern bool bsp_is_highspeed_running(uint8_t channel);
extern bool bsp_is_hsp_running(uint8_t channel);

extern void bsp_start_pwm(uint16_t channel, uint16_t pwmVal, uint16_t pwmCycle);
extern void bsp_stop_pwm(uint8_t channel);
extern bool bsp_is_pwm_running(uint8_t channel);
#endif /* __BSP_TIM_H */

