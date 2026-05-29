/**
  ******************************************************************************
  * @file    pid_task.c
  * @author  zhanghuajie
  * @version V0.0.1
  * @date    2022-03-23
  * @brief
  ******************************************************************************
  */

#include "plc_element.h"
#include "plc_variable.h"
#include "Bsp_flash.h"

#include "pid_task.h"

#include "stdlib.h"

/*------------------------------------------------------------------------------
*   pid_task相关全局变量定义
*-----------------------------------------------------------------------------*/

TaskHandle_t gPIDTaskHandler;

pid_channel_set_st gpid_channel_set[PID_CHANNEL_MAX] = {0};


pid_err_st gpid_err[PID_CHANNEL_MAX] = {0};

time_moitor_st gout_time[PID_CHANNEL_MAX][2] = {0};

time_moitor_st gperiod_time[PID_CHANNEL_MAX] = {0};


/*------------------------------------------------------------------------------
*   pid_main_task相关任务和函数
*-----------------------------------------------------------------------------*/


void pid_task(void *p_arg)
{
    uint8_t channel,pid_on=0;

    vTaskDelay(100);
   
    while (1)
    {
        if (gtv_PlcElement.msp_DElement[PID_D_CONTROL] & CONTROL_READ_FLASH_MASK)
        {
            pid_read_parameter_1();
            pid_read_parameter_2();
            gtv_PlcElement.msp_DElement[PID_D_CONTROL] &= (~CONTROL_READ_FLASH_MASK);
        }

        if (gtv_PlcElement.msp_DElement[PID_D_CONTROL] & CONTROL_WRITE_FLASH_MASK)
        {
            pid_write_parameter_2();      //固化PID参数
            gtv_PlcElement.msp_DElement[PID_D_CONTROL] &= (~CONTROL_WRITE_FLASH_MASK);
        }
        pid_digital_range_adjust();     //调整输出最大值
        pid_run();      //PID控制
        pid_smart_run();        //智能调整PID参数
        pid_io_refresh();       //刷新IO输出

        vTaskDelay(1);
    }
}


unsigned char pid_open_check(void)
{
    uint8_t channel;
    if (pid_read_parameter_1()==pdFALSE)       //PID参数1从Flash读到 RAM
    {
        return pdFALSE;
    }
    
    for (channel=1;channel<PID_CHANNEL_MAX;channel++)
    {
        if(gpid_channel_set[channel].pid_control.PID_type)
           return pdTRUE;
    }
    return pdFALSE;
}


unsigned char  pid_read_parameter_1(void)
{
    uint8_t *pS,*pD;
    uint16_t size,i;
    flash_part_info_t *ltp_UCodeInfo = bsp_get_flash_info(PID1_START_PAGE);
    pS = (uint8_t *)(ltp_UCodeInfo->startAddr);
    if (0xAAAA != *(uint16_t *)pS)
    {
        LOGE("pid_read_parameter_1", "pS != 0xAAAA");
        return pdFALSE;
    }
    pS = (uint8_t *)(ltp_UCodeInfo->startAddr+26);
    pD = (uint8_t *)&gpid_channel_set;
    size = sizeof(pid_channel_set_st)*PID_CHANNEL_MAX;
    for (i = 0; i < size; i++)
    {
        *pD = *pS;
        pD++;
        pS++;
    }
    LOGE("pid_read_parameter_1", "GOOD!");
    return pdTRUE;
}

void pid_read_parameter_2(void)
{
    uint16_t *pS;
    uint8_t channel,i;
    flash_part_info_t *PID2Info = bsp_get_flash_info(PID2_START_PAGE);
    pS = (uint16_t *)(PID2Info->startAddr+26);
    for (channel = 0; channel < PID_CHANNEL_MAX; channel++)
    {
        pS++;        //跳过通道号
        plc_set_bit_element_value(M_ELEMENT,gpid_channel_set[channel].pid_control.run_control_addr,(uint8_t)*pS);       //启停控制
        pS++;
        gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_control.period_addr] = (uint16_t)*pS;     //控制周期
        pS++;
        gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_control.kp_addr] = (uint16_t)*pS;     //KP
        pS++;
        gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_control.ki_addr] = (uint16_t)*pS;     //KI
        pS++;
        gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_control.kd_addr] = (uint16_t)*pS;     //KD
        pS++;
        gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_smart.smart_type_addr] = (uint16_t)*pS;       //智能模式
        pS++;
        gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_smart.err_big_addr] = (uint16_t)*pS;      //大偏差值
        pS++;
        gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_smart.err_small_addr] = (uint16_t)*pS;        //小偏差值
        pS++;
        gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_in.set_addr] = (int16_t)*pS;      //设定值
        pS += 3;        //预留2个字
    }
    LOGE("pid_read_parameter_2", "GOOD");
}

void pid_write_parameter_1(void)
{

}

void pid_write_parameter_2(void)
{
    flash_part_info_t *PID2Info = bsp_get_flash_info(PID2_START_PAGE);
    mem_part_info_t MemInfo;
    MemInfo.startAddr = PID2Info->startAddr;
    MemInfo.partSize = (PID2Info->endAddr - PID2Info->startAddr + 1);
    uint16_t *pS = (uint16_t *)(PID2Info->startAddr);
    uint8_t channel,i,number=210;
    uint16_t pid2[number];
    
    for (i=0;i<number;i++)
    {
        pid2[i] = *pS;
        pS++;
    }
    i=13;
    for (channel=0;channel<PID_CHANNEL_MAX;channel++)
    {
        i++;        //跳过通道号
        pid2[i] = plc_get_bit_element_value(M_ELEMENT,gpid_channel_set[channel].pid_control.run_control_addr);      //启停控制
        i++;
        pid2[i] = gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_control.period_addr];       //控制周期
        i++;
        pid2[i] = gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_control.kp_addr];       //KP
        i++;
        pid2[i] = gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_control.ki_addr];       //KI
        i++;
        pid2[i] = gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_control.kd_addr];       //KD
        i++;
        pid2[i] = gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_smart.smart_type_addr];     //智能模式
        i++;
        pid2[i] = gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_smart.err_big_addr];        //大偏差值
        i++;
        pid2[i] = gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_smart.err_small_addr];      //小偏差值
        i++;
        pid2[i] = gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_in.set_addr];       //设定值
        i += 3;        //预留2个字
    }
    
    bsp_flash_erase_partition(&MemInfo);
    bsp_flash_write_buffer(MemInfo.startAddr, (uint8_t *)&pid2, number*2);
}


void pid_digital_range_adjust(void)
{
    uint8_t channel;
    for (channel = 0; channel < PID_CHANNEL_MAX; channel++)
    {
        if (gpid_channel_set[channel].pid_out.out[0].channel_type == DIGITAL_OUT)
            gpid_channel_set[channel].pid_out.out[0].channel_max = gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_control.period_addr];
        if (gpid_channel_set[channel].pid_out.out[1].channel_type == DIGITAL_OUT)
            gpid_channel_set[channel].pid_out.out[1].channel_max = gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_control.period_addr];
    }
}


void pid_io_refresh(void)
{
    uint8_t channel;
    uint8_t pid_on_flag;
    uint32_t time;

    for (channel = 0; channel < PID_CHANNEL_MAX; channel++)
    {
        pid_on_flag=plc_get_bit_element_value(M_ELEMENT, gpid_channel_set[channel].pid_control.run_control_addr);
            
        if (gpid_channel_set[channel].pid_control.PID_type)       //PID通道已打开并开启了控制
        {
            if (gpid_channel_set[channel].pid_out.out[0].channel_type == DIGITAL_OUT)        //第0通道数字量输出
            {
                if (gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_out.out[0].channel_digital_addr])        //输出时间不为0
                {
                    plc_set_bit_element_value(Y_ELEMENT,gpid_channel_set[channel].pid_out.out[0].channel_bit_addr,1);        //置位对应输出
                    if (!(gout_time[channel][0].start_flag))         //输出开始时间未标记
                    {
                        gout_time[channel][0].time_start = GET_1MS_TICKS_COUNT();
                        gout_time[channel][0].start_flag = 1;
                    }
                    time = GET_1MS_TICKS_COUNT();       //输出时间监控
                    if (time >= gout_time[channel][0].time_start)        //时间计数溢出
                        time -= gout_time[channel][0].time_start;
                    else
                        time += 0xffff-gout_time[channel][0].time_start;
                    if ((uint16_t)time > gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_out.out[0].channel_digital_addr])       //输出时间到
                    {
                        plc_set_bit_element_value(Y_ELEMENT,gpid_channel_set[channel].pid_out.out[0].channel_bit_addr,0);        //复位对应输出
                        gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_out.out[0].channel_digital_addr] = 0;        //清除字寄存器
                        gout_time[channel][0].start_flag = 0;
                    }
                }
                if (gpid_channel_set[channel].pid_out.out_type == DOUBLE_OUT)        //第1通道数字量输出
                {
                    if (gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_out.out[1].channel_digital_addr])        //输出时间不为0
                    {
                        plc_set_bit_element_value(Y_ELEMENT,gpid_channel_set[channel].pid_out.out[1].channel_bit_addr,1);        //置位对应输出
                        if (!(gout_time[channel][1].start_flag))     //输出开始时间未标记
                        {
                            gout_time[channel][1].time_start = GET_1MS_TICKS_COUNT();
                            gout_time[channel][1].start_flag = 1;
                        }
                        time = GET_1MS_TICKS_COUNT() - gout_time[channel][1].time_start;     //输出时间监控
                        if (time < 0)       //时间计数溢出
                            time += 0xffff;
                        if (time > gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_out.out[1].channel_digital_addr])     //输出时间到
                        {
                            plc_set_bit_element_value(Y_ELEMENT,gpid_channel_set[channel].pid_out.out[1].channel_bit_addr,0);        //复位对应输出
                            gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_out.out[1].channel_digital_addr] = 0;        //清除字寄存器
                            gout_time[channel][1].start_flag = 0;
                        }
                    }
                }
            }
        }
    }
}


void pid_run(void)
{
    uint8_t channel;
    uint8_t pid_on_flag;
    uint32_t time;

    for (channel = 0; channel < PID_CHANNEL_MAX; channel++)
    {
        pid_on_flag=plc_get_bit_element_value(M_ELEMENT, gpid_channel_set[channel].pid_control.run_control_addr);
        if (gpid_channel_set[channel].pid_control.PID_type && pid_on_flag)       //PID通道已打开并开启了控制
        {
            if (0)      //自整定
            {
                pid_auto_tune(channel);
            }
            else
            {
                if (!(gperiod_time[channel].start_flag))     //输出开始时间未标记
                {
                    gperiod_time[channel].time_start = GET_1MS_TICKS_COUNT();
                    gperiod_time[channel].start_flag = 1;
                }
                time = GET_1MS_TICKS_COUNT();       //输出时间监控
                if (time >= gperiod_time[channel].time_start)        //时间计数溢出
                    time -= gperiod_time[channel].time_start;
                else
                    time += 0xffff-gperiod_time[channel].time_start;
                if ((uint16_t)time > gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_control.period_addr])       //输出时间到
                {
                    pid_calculate(channel);     //进行PID运算
                    gperiod_time[channel].start_flag = 0;        //更新周期开始标志位
                }
            }
        }
    }
}


void pid_auto_tune(uint8_t channel)
{

}


void pid_calculate(uint8_t channel)
{
    int32_t out_digital;
    int16_t err_temp;

    gpid_err[channel].err_last = gpid_err[channel].err_next;      //保存e(k-2)
    gpid_err[channel].err_next = gpid_err[channel].err;       //保存e(k-1)
    gpid_err[channel].err = gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_in.set_addr] - gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_in.back_addr];      //保存e(k)

    switch (gpid_channel_set[channel].pid_control.PID_type)
    {
    case PID_INC:       //增量式PID
        out_digital = gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_control.kp_addr] * (gpid_err[channel].err - gpid_err[channel].err_next);     //计算比例部分
        out_digital += gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_control.ki_addr] * gpid_err[channel].err;      //增加积分部分
        out_digital += gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_control.kd_addr] * (gpid_err[channel].err - 2 * gpid_err[channel].err_next + gpid_err[channel].err_last);        //增加微分部分
        if (out_digital < gpid_channel_set[channel].pid_out.out[0].channel_min)
            out_digital = gpid_channel_set[channel].pid_out.out[0].channel_min;
        if (out_digital > gpid_channel_set[channel].pid_out.out[0].channel_max)
            out_digital = gpid_channel_set[channel].pid_out.out[0].channel_max;
        gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_out.out[0].channel_digital_addr] = (int16_t)out_digital;
        break;

    case PID_ABS:       //位置式PID
        if (abs(gpid_err[channel].err) < gpid_channel_set[channel].pid_out.out[0].channel_max/4)      //积分分离
            {
            gpid_err[channel].err_cumsum += gpid_err[channel].err;        //计算e(k)累积
            err_temp = gpid_channel_set[channel].pid_out.out[0].channel_min/gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_control.ki_addr];
            if (gpid_err[channel].err_cumsum < err_temp)
                gpid_err[channel].err_cumsum = err_temp;
            err_temp = gpid_channel_set[channel].pid_out.out[0].channel_max/gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_control.ki_addr];
            if (gpid_err[channel].err_cumsum > err_temp)
                gpid_err[channel].err_cumsum = err_temp;
            }
        out_digital = gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_control.kp_addr] * gpid_err[channel].err;     //计算比例部分
        out_digital += gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_control.ki_addr] * gpid_err[channel].err_cumsum;       //增加积分部分
        out_digital += gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_control.kd_addr] * (gpid_err[channel].err - gpid_err[channel].err_next);        //增加微分部分
        if (out_digital < gpid_channel_set[channel].pid_out.out[0].channel_min)
            out_digital = gpid_channel_set[channel].pid_out.out[0].channel_min;
        if (out_digital > gpid_channel_set[channel].pid_out.out[0].channel_max)
            out_digital = gpid_channel_set[channel].pid_out.out[0].channel_max;
        gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_out.out[0].channel_digital_addr] = (int16_t)out_digital;
        break;

    default:

        break;
    }
}


void pid_smart_run(void)
{
    uint8_t channel;
    uint8_t pid_on_flag;
    uint16_t time;

    //备份PID参数

    for (channel = 0; channel < PID_CHANNEL_MAX; channel++)
    {
        pid_on_flag=plc_get_bit_element_value(M_ELEMENT, gpid_channel_set[channel].pid_control.run_control_addr);
        if (gpid_channel_set[channel].pid_control.PID_type && pid_on_flag)       //PID通道已打开并开启了控制
        {
            if (1)       //非自整定
            {
                if (gtv_PlcElement.msp_DElement[gpid_channel_set[channel].pid_smart.smart_type_addr] == SMART_TYPE_FUZZY)
                    pid_smart_fuzzy(channel);
            }
        }
    }
}


void pid_smart_fuzzy(uint8_t channel)
{

}



    


