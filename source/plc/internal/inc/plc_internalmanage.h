/**
  ******************************************************************************
  * @file    plc_internalmanage.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   内务处理相关函数
  ******************************************************************************
  */

#ifndef __PLC_INTERNAL_MANAGE_H
#define __PLC_INTERNAL_MANAGE_H

typedef struct __PLC_CLOCK_CYCLE_RECORD_ST{
    /*1小时振荡源时间记录, 时间基数 500ms*/
    unsigned long mlv_Hour;
    /*1分钟振荡源时间记录*/
    unsigned long mlv_Minute;
    /*1秒振荡源时间记录*/
    unsigned long mlv_Second;
    /*100ms*/
    unsigned long mlv_HundredMs;
    /*10ms
    unsigned long mlv_TenMs; 
    */
    /*当前状态,bit0 ~ bit4 分别代表10ms, 100ms, 1秒, 1分钟, 1小时振荡时钟当前状态*/
    unsigned char mcv_Status;
}plc_clock_cycle_record_st;


extern plc_clock_cycle_record_st gtv_ClockCycleRecord;

void plc_clock_cycle_init(void);
void plc_refresh_cycle_clock(void);
unsigned char plc_refresh_sys_scan_time(void);
void plc_refresh_io_port(void);
void plc_refresh_force_element_value(void);
void plc_set_stop_status_output_port(void);
void plc_refresh_scan_time(uint32_t lastTick);

#endif /*__PLC_INTERNAL_MANAGE_H*/

