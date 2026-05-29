/**
  ******************************************************************************
  * @file    mb_ctrl.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   Modbus slave 控制子功能
  ******************************************************************************
  */
#include "main.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_snvs_hp.h"
#include "fsl_snvs_lp.h"
#include "mb.h"
#include "mb_ctrl.h"
#include "bsp_dct.h"
#include "bsp_flash.h"
#include "bsp_iwdg.h"
#include "mb_download.h"
#include "plc_commonfunc.h"
#include "FreeRTOS.h"
#include "plc_variable.h"
#include "mb_maptable.h"
#include "plc_password.h"
#include "plc_sysinit.h"
#include "plc_element.h"
#include "plc_executer.h"
#include "rc6.h"
#include "verify_func.h"
#include "kalyke_tool.h"
#include "kalyke_ota.h"
#include "kalyke_monitor_task.h"
#include "bsp_led.h"
#include "kalyke_version.h"
#include "plsd_task.h"
#include "kalyke_opts.h"
#include "kalyke_internet_task.h"
#include "daisy_task.h"
#include "ether_cat_task.h"
#include "kalyke_DS1302.h"


static const char *TAG = "mb_ctrl";

#if (KALYKE_DS1302_FEATURE == 1)
static void mb_slave_ctrl_read_realtime(md_slave_msg_pack *pMsg)
{
    LOGV(TAG, "Enter %s()", __func__);
    unsigned char i;
    uint8_t buf[8] = {0};
    DS1302_ReadTimeBurst(buf);
    LOGD(TAG, "DS1302: %04u-%02u-%02u  %02u:%02u:%02u, week:%u.", buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);

    for (i = 0; i < 3; i++)
    {
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];
    }
    uint16_t year = 2000 + buf[1];
    pMsg->mcp_RespBuff[3] = year >> 8;
    pMsg->mcp_RespBuff[4] = year & 0xFF;

    pMsg->mcp_RespBuff[5] = 0;
    pMsg->mcp_RespBuff[6] = buf[2];
    pMsg->mcp_RespBuff[7] = 0;
    pMsg->mcp_RespBuff[8] = buf[3];
    
    pMsg->mcp_RespBuff[9] = 0;
    pMsg->mcp_RespBuff[10] = buf[4];
    
    pMsg->mcp_RespBuff[11] = 0;
    pMsg->mcp_RespBuff[12] = buf[5];
    
    pMsg->mcp_RespBuff[13] = 0;
    pMsg->mcp_RespBuff[14] = buf[6];

    // 星期几
    pMsg->mcp_RespBuff[15] = 0;
    pMsg->mcp_RespBuff[16] = buf[7];
    
    pMsg->msv_RespLen = 17;
    mb_slave_verify_resp_msg(pMsg);    
}
#else
static void mb_slave_ctrl_read_realtime(md_slave_msg_pack *pMsg)
{
    PRINTF("Enter %s\r\n", __func__);
    unsigned char i;
    snvs_hp_rtc_datetime_t rtcDate;

    SNVS_HP_RTC_GetDatetime(SNVS, &rtcDate);

    PRINTF("\r\n RTC: %04u-%02u-%02u  %02u:%02u:%02u\r\n", rtcDate.year, rtcDate.month, rtcDate.day, rtcDate.hour, rtcDate.minute, rtcDate.second);
    for (i = 0; i < 3; i++)
    {
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];
    }
    
    pMsg->mcp_RespBuff[3] = rtcDate.year >> 8;
    pMsg->mcp_RespBuff[4] = rtcDate.year & 0xFF;

    pMsg->mcp_RespBuff[5] = 0;
    pMsg->mcp_RespBuff[6] = rtcDate.month;
    pMsg->mcp_RespBuff[7] = 0;
    pMsg->mcp_RespBuff[8] = rtcDate.day;
    
    pMsg->mcp_RespBuff[9] = 0;
    pMsg->mcp_RespBuff[10] = rtcDate.hour;
    
    pMsg->mcp_RespBuff[11] = 0;
    pMsg->mcp_RespBuff[12] = rtcDate.minute;
    
    pMsg->mcp_RespBuff[13] = 0;
    pMsg->mcp_RespBuff[14] = rtcDate.second;

    // 星期几
    pMsg->mcp_RespBuff[15] = 0;
    pMsg->mcp_RespBuff[16] = 2;
    
    pMsg->msv_RespLen = 17;
    mb_slave_verify_resp_msg(pMsg);    
}
#endif

#if (KALYKE_LP_RTC == 1)
#if (KALYKE_DS1302_FEATURE == 1)
static void mb_slave_ctrl_set_realtime(md_slave_msg_pack *pMsg)
{
    snvs_lp_srtc_datetime_t srtcDate;
    srtcDate.year = (pMsg->mcp_ReceiveBuff[3]<<8) | pMsg->mcp_ReceiveBuff[4];
    srtcDate.month = pMsg->mcp_ReceiveBuff[6];
    srtcDate.day = pMsg->mcp_ReceiveBuff[8];
    srtcDate.hour = pMsg->mcp_ReceiveBuff[10];
    srtcDate.minute = pMsg->mcp_ReceiveBuff[12];
    srtcDate.second = pMsg->mcp_ReceiveBuff[14];
    LOGI(TAG, "Set RTC: %04u-%02u-%02u  %02u:%02u:%02u\r\n", srtcDate.year, srtcDate.month, srtcDate.day, srtcDate.hour, srtcDate.minute, srtcDate.second);

    uint8_t buf[8] = {0};
    buf[1] = srtcDate.year - 2000;
    buf[2] = srtcDate.month;
    buf[3] = srtcDate.day;
    buf[4] = srtcDate.hour;
    buf[5] = srtcDate.minute;
    buf[6] = srtcDate.second;
    buf[7] = get_WeekDayNum(srtcDate.year, srtcDate.month, srtcDate.day);
    LOGW(TAG, "Set DS1302: %04u-%02u-%02u  %02u:%02u:%02u, week:%u\r\n", buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
    DS1302_WriteTimeBurst(buf);
    if (SNVS_LP_SRTC_SetDatetime(SNVS, &srtcDate) == kStatus_Success)
    {
        SNVS_HP_RTC_TimeSynchronize(SNVS);
        /*组响应帧，特殊的响应帧在对应函数完成*/
        for(uint8_t i=0; i<3; i++)
        {
            pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];
        }
        pMsg->msv_RespLen = 3;
        mb_slave_verify_resp_msg(pMsg);
        LOGD(TAG, "Set RTC Success!");
    }
    else
    {
        pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
        mb_slave_error_resp(pMsg);
        LOGE(TAG, "Set RTC Failed!");
    }
}
#else
static void mb_slave_ctrl_set_realtime(md_slave_msg_pack *pMsg)
{
    snvs_lp_srtc_datetime_t srtcDate;
    srtcDate.year = (pMsg->mcp_ReceiveBuff[3]<<8) | pMsg->mcp_ReceiveBuff[4];
    srtcDate.month = pMsg->mcp_ReceiveBuff[6];
    srtcDate.day = pMsg->mcp_ReceiveBuff[8];
    srtcDate.hour = pMsg->mcp_ReceiveBuff[10];
    srtcDate.minute = pMsg->mcp_ReceiveBuff[12];
    srtcDate.second = pMsg->mcp_ReceiveBuff[14];

    if (SNVS_LP_SRTC_SetDatetime(SNVS, &srtcDate) == kStatus_Success)
    {
        SNVS_HP_RTC_TimeSynchronize(SNVS);
        /*组响应帧，特殊的响应帧在对应函数完成*/
        for(uint8_t i=0; i<3; i++)
        {
            pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];
        }
        pMsg->msv_RespLen = 3;
        mb_slave_verify_resp_msg(pMsg);
        LOGD(TAG, "Set RTC Success!");
    }
    else
    {
        pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
        mb_slave_error_resp(pMsg);
        LOGE(TAG, "Set RTC Failed!");
    }
}
#endif
#else
void mb_slave_ctrl_set_realtime(md_slave_msg_pack *pMsg)
{
    snvs_hp_rtc_datetime_t rtcDate;
    rtcDate.year = (pMsg->mcp_ReceiveBuff[3]<<8) | pMsg->mcp_ReceiveBuff[4];
    rtcDate.month = pMsg->mcp_ReceiveBuff[6];
    rtcDate.day = pMsg->mcp_ReceiveBuff[8];
    rtcDate.hour = pMsg->mcp_ReceiveBuff[10];
    rtcDate.minute = pMsg->mcp_ReceiveBuff[12];
    rtcDate.second = pMsg->mcp_ReceiveBuff[14];
#if 0
    if (rtcDate.second + 3 >= 60)
    {
        rtcDate.second -= 60;
        rtcDate.minute++;
        if (rtcDate.minute >= 60)
        {
            rtcDate.minute -= 60;
            rtcDate.hour++;
            if (rtcDate.hour >= 24)
            {
                rtcDate.hour -= 24;
                
            }
        }
    }
#endif
    if (SNVS_HP_RTC_SetDatetime(SNVS, &rtcDate) == kStatus_Success)
    {
        /*组响应帧，特殊的响应帧在对应函数完成*/
        for(uint8_t i=0; i<3; i++)
            pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];
        pMsg->msv_RespLen = 3;
        mb_slave_verify_resp_msg(pMsg);
        LOGD("mb_ctrl", "Set RTC Success!\r\n");
    }
    else
    {
        pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
        mb_slave_error_resp(pMsg);
        LOGE("mb_ctrl", "Set RTC Failed!\r\n");
    }
}
#endif

/**
  * @brief  PLC运行指令
  * @param  None
  * @retval None
  */
void mb_slave_ctrl_run_cmd(md_slave_msg_pack *pMsg)
{
    /*置位CMD运行标志*/
    gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.cmd_run = 1;
    /*清除停止运行标志*/
    gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.cmd_stop = 0;
    gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.error_status_stop = 0;
    gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.cmd_reboot = 0;

    bsp_close_LED_RUN_ERR();
}

/**
  * @brief  PLC运行停止指令
  * @param  None
  * @retval None
  */
void mb_slave_ctrl_stop_cmd(md_slave_msg_pack *pMsg)
{
    /*清除上电自动运行标志*/
    gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.poweron_auto_run = 0;
    gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.cmd_run = 0;
    /*置位停止运行标志*/
    gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.cmd_stop = 1;
}

/**
  * @brief  PLC重启指令
  * @param  None
  * @retval None
  */
static void mb_slave_ctrl_system_reboot(md_slave_msg_pack *pMsg)
{
    unsigned char i;

    /*组响应帧，特殊的响应帧在对应函数完成*/
    for(i=0; i<3; i++)
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];

    pMsg->msv_RespLen = 3;
    mb_slave_verify_resp_msg(pMsg);
#if 0
    当Daisy配置错误时，阻塞在plc_status_stop_to_run函数 ( xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_PLC_TASK_WAIT_DAISY, pdTRUE, pdFALSE, portMAX_DELAY) )
    /*清除上电自动运行标志*/
    gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.poweron_auto_run = 0;
    /*置位停止运行标志*/
    gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.cmd_stop = 1;
    /*置重启标志*/
    gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.cmd_reboot = 1;
#else
    plc_reboot();
#endif
}

/**
  * @brief  PLC强制位元件
  * @param  None
  * @retval None
  */
void mb_slave_ctrl_force_bits(md_slave_msg_pack *pMsg)
{
    struct list_head *ltp_Head;
    struct list_head *ltp_ForNext;
    struct list_head *ltp_ForCur;
    unsigned short lsv_ForceElemetCnt;
    mb_force_element_t *ltp_ForceData;
    unsigned char lcv_Ret;
    unsigned short i;
    unsigned char *lcp_Temp;
    unsigned short lsv_ModbusAddr;
    unsigned char lcv_ElementValue;

    lsv_ForceElemetCnt = pMsg->mcp_ReceiveBuff[3];

    ltp_Head = &gtv_ForceBits.head;

    lcp_Temp = &pMsg->mcp_ReceiveBuff[4];

    if(gtv_ForceBits.lsv_ListLen > 0) {
        for(i=0; i<lsv_ForceElemetCnt; i++) {
            lsv_ModbusAddr = GET_BIGPU16_DATA(lcp_Temp + i*3);
            lcv_ElementValue = GET_PU8_DATA(lcp_Temp + i*3 +2);

            /*查找链表是否有相同元素，更新元件值*/
            list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
                ltp_ForceData = list_entry(ltp_ForCur, mb_force_element_t, list);
                if(ltp_ForceData->msv_ModbusAddr == lsv_ModbusAddr) {
                    ltp_ForceData->msv_ElementValue = lcv_ElementValue;
                    goto NEXT_BIT_ELEMENT;
                }
            }

            /*申请链表元素内存*/
            ltp_ForceData = (mb_force_element_t *)pvPortMalloc(sizeof(mb_force_element_t));
            configASSERT(ltp_ForceData != NULL);

            ltp_ForceData->msv_ModbusAddr = lsv_ModbusAddr;

            lcv_Ret = mb_slave_convert_element_info(MB_BIT_ELEMENT, ltp_ForceData->msv_ModbusAddr, &ltp_ForceData->mcv_ElementType, &ltp_ForceData->msv_ElementAddr);
            if(lcv_Ret != pdPASS) {
                vPortFree(ltp_ForceData);
                pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
                mb_slave_error_resp(pMsg);
                return;
            }

            ltp_ForceData->msv_ElementValue = lcv_ElementValue;

            list_add_tail(&ltp_ForceData->list, ltp_Head);
            gtv_ForceBits.lsv_ListLen ++;

        NEXT_BIT_ELEMENT:
            __NOP();
        }

    } else {
        /*链表为空，直接插入链表即可*/
        for(i=0; i<lsv_ForceElemetCnt; i++) {
            /*申请链表元素内存*/
            ltp_ForceData = (mb_force_element_t *)pvPortMalloc(sizeof(mb_force_element_t));
            configASSERT(ltp_ForceData != NULL);

            ltp_ForceData->msv_ModbusAddr = GET_BIGPU16_DATA(lcp_Temp + i*3);

            lcv_Ret = mb_slave_convert_element_info(MB_BIT_ELEMENT, ltp_ForceData->msv_ModbusAddr, &ltp_ForceData->mcv_ElementType, &ltp_ForceData->msv_ElementAddr);
            if(lcv_Ret != pdPASS) {
                vPortFree(ltp_ForceData);
                pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
                mb_slave_error_resp(pMsg);
                return;
            }

            ltp_ForceData->msv_ElementValue = GET_PU8_DATA(lcp_Temp + i*3 +2);

            list_add_tail(&ltp_ForceData->list, ltp_Head);
            gtv_ForceBits.lsv_ListLen ++;
        }
    }
}

/**
  * @brief  PLC强制字元件
  * @param  None
  * @retval None
  */
void mb_slave_ctrl_force_words(md_slave_msg_pack *pMsg)
{
    struct list_head *ltp_Head;
    struct list_head *ltp_ForNext;
    struct list_head *ltp_ForCur;
    unsigned short lsv_ForceElemetCnt;
    mb_force_element_t *ltp_ForceData;
    unsigned char lcv_Ret;
    unsigned short i;
    unsigned char *lcp_Temp;
    unsigned short lsv_ModbusAddr;
    unsigned short lsv_ElementValue;

    lsv_ForceElemetCnt = pMsg->mcp_ReceiveBuff[3];

    ltp_Head = &gtv_ForceWords.head;

    lcp_Temp = &pMsg->mcp_ReceiveBuff[4];

    if(gtv_ForceWords.lsv_ListLen > 0) {
        for(i=0; i<lsv_ForceElemetCnt; i++) {
            lsv_ModbusAddr = GET_BIGPU16_DATA(lcp_Temp + i*4);
            lsv_ElementValue = GET_BIGPU16_DATA(lcp_Temp + i*4 +2);

            /*查找链表是否有相同元素，更新元件值*/
            list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
                ltp_ForceData = list_entry(ltp_ForCur, mb_force_element_t, list);
                if(ltp_ForceData->msv_ModbusAddr == lsv_ModbusAddr) {
                    ltp_ForceData->msv_ElementValue = lsv_ElementValue;
                    goto NEXT_WORD_ELEMENT;
                }
            }

            /*申请链表元素内存*/
            ltp_ForceData = (mb_force_element_t *)pvPortMalloc(sizeof(mb_force_element_t));
            configASSERT(ltp_ForceData != NULL);

            ltp_ForceData->msv_ModbusAddr = lsv_ModbusAddr;

            lcv_Ret = mb_slave_convert_element_info(MB_WORD_ELEMENT, ltp_ForceData->msv_ModbusAddr, &ltp_ForceData->mcv_ElementType, &ltp_ForceData->msv_ElementAddr);
            if(lcv_Ret != pdPASS) {
                vPortFree(ltp_ForceData);
                pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
                mb_slave_error_resp(pMsg);
                return;
            }

            ltp_ForceData->msv_ElementValue = lsv_ElementValue;

            list_add_tail(&ltp_ForceData->list, ltp_Head);
            gtv_ForceWords.lsv_ListLen ++;

        NEXT_WORD_ELEMENT:
            __NOP();
        }

    } else {
        /*链表为空，直接插入链表即可*/
        for(i=0; i<lsv_ForceElemetCnt; i++) {
            /*申请链表元素内存*/
            ltp_ForceData = (mb_force_element_t *)pvPortMalloc(sizeof(mb_force_element_t));
            configASSERT(ltp_ForceData != NULL);

            ltp_ForceData->msv_ModbusAddr = GET_BIGPU16_DATA(lcp_Temp + i*4);

            lcv_Ret = mb_slave_convert_element_info(MB_WORD_ELEMENT, ltp_ForceData->msv_ModbusAddr, &ltp_ForceData->mcv_ElementType, &ltp_ForceData->msv_ElementAddr);
            if(lcv_Ret != pdPASS) {
                vPortFree(ltp_ForceData);
                pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
                mb_slave_error_resp(pMsg);
                return;
            }

            ltp_ForceData->msv_ElementValue = GET_BIGPU16_DATA(lcp_Temp + i*4 +2);

            list_add_tail(&ltp_ForceData->list, ltp_Head);
            gtv_ForceWords.lsv_ListLen ++;
        }
    }
}

/**
  * @brief  取消位元件强制
  * @param  None
  * @retval None
  */
void mb_slave_ctrl_unforce_bits(md_slave_msg_pack *pMsg)
{
    struct list_head *ltp_Head;
    struct list_head *ltp_ForNext;
    struct list_head *ltp_ForCur;
    unsigned short lsv_ForceElemetCnt;
    mb_force_element_t *ltp_ForceData;
    unsigned short i;
    unsigned char *lcp_Temp;
    unsigned short lsv_ModbusAddr;

    lsv_ForceElemetCnt = pMsg->mcp_ReceiveBuff[3];

    ltp_Head = &gtv_ForceBits.head;

    lcp_Temp = &pMsg->mcp_ReceiveBuff[4];

    if(gtv_ForceBits.lsv_ListLen > 0) {
        for(i=0; i<lsv_ForceElemetCnt; i++) {
            lsv_ModbusAddr = GET_BIGPU16_DATA(lcp_Temp + i*2);

            /*查找链表是否有相同元素，并删除*/
            list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
                ltp_ForceData = list_entry(ltp_ForCur, mb_force_element_t, list);
                if(ltp_ForceData->msv_ModbusAddr == lsv_ModbusAddr) {
                    list_del(&ltp_ForceData->list);
                    vPortFree(ltp_ForceData);
                    gtv_ForceBits.lsv_ListLen --;
                    break;
                }
            }
        }
    }

    if(gtv_ForceBits.lsv_ListLen == 0)
        INIT_LIST_HEAD(&gtv_ForceBits.head);
}

/**
  * @brief  取消字元件强制
  * @param  None
  * @retval None
  */
void mb_slave_ctrl_unforce_words(md_slave_msg_pack *pMsg)
{
    struct list_head *ltp_Head;
    struct list_head *ltp_ForNext;
    struct list_head *ltp_ForCur;
    unsigned short lsv_ForceElemetCnt;
    mb_force_element_t *ltp_ForceData;
    unsigned short i;
    unsigned char *lcp_Temp;
    unsigned short lsv_ModbusAddr;

    lsv_ForceElemetCnt = pMsg->mcp_ReceiveBuff[3];

    ltp_Head = &gtv_ForceWords.head;

    lcp_Temp = &pMsg->mcp_ReceiveBuff[4];

    if(gtv_ForceWords.lsv_ListLen > 0) {
        for(i=0; i<lsv_ForceElemetCnt; i++) {
            lsv_ModbusAddr = GET_BIGPU16_DATA(lcp_Temp + i*2);

            /*查找链表是否有相同元素，并删除*/
            list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
                ltp_ForceData = list_entry(ltp_ForCur, mb_force_element_t, list);
                if(ltp_ForceData->msv_ModbusAddr == lsv_ModbusAddr) {
                    list_del(&ltp_ForceData->list);
                    vPortFree(ltp_ForceData);
                    gtv_ForceWords.lsv_ListLen --;
                    break;
                }
            }
        }
    }

    if(gtv_ForceWords.lsv_ListLen == 0)
        INIT_LIST_HEAD(&gtv_ForceWords.head);
}

/**
  * @brief  取消全部元件强制
  * @param  None
  * @retval None
  */
void mb_slave_ctrl_unforce_all(md_slave_msg_pack *pMsg)
{
    struct list_head *ltp_Head;
    struct list_head *ltp_ForNext;
    struct list_head *ltp_ForCur;
    mb_force_element_t *ltp_ForceData;

    ltp_Head = &gtv_ForceBits.head;

    if(gtv_ForceBits.lsv_ListLen > 0) {
        list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
            ltp_ForceData = list_entry(ltp_ForCur, mb_force_element_t, list);
            list_del(&ltp_ForceData->list);
            vPortFree(ltp_ForceData);
        }
    }

    gtv_ForceBits.lsv_ListLen = 0;
    INIT_LIST_HEAD(&gtv_ForceBits.head);


    ltp_Head = &gtv_ForceWords.head;

    if(gtv_ForceWords.lsv_ListLen > 0) {
        list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
            ltp_ForceData = list_entry(ltp_ForCur, mb_force_element_t, list);
            list_del(&ltp_ForceData->list);
            vPortFree(ltp_ForceData);
        }
    }

    gtv_ForceWords.lsv_ListLen = 0;
    INIT_LIST_HEAD(&gtv_ForceWords.head);
}

/**
  * @brief  CBIN文件UCODE内容解密
  * @param  None
  * @retval None
  */
void mb_slave_ctrl_descrypt_cbin_file(unsigned char *lcp_Ucode, int llv_Length)
{
    LOGD(TAG, "Enter %s(), lcp_Ucode = 0x%08X, llv_Length = %d", __func__, lcp_Ucode, llv_Length);
    unsigned char lcv_Index =0;
    unsigned char lcav_PswCode[]  = {0x37,0x9e,0x88,0x12,0x4f,0xa3,0x77};
    unsigned char lcv_PswCodeSize = 7;
    int i;

    for(i=0; i < llv_Length; i++) {
        lcv_Index++;
        if(lcv_Index==lcv_PswCodeSize) {
            lcv_Index=0;
        }

        if(((*lcp_Ucode) < lcav_PswCode[lcv_Index])) {
            *lcp_Ucode =(unsigned char)((int)((*lcp_Ucode) - lcav_PswCode[lcv_Index]) + 256);
        } else {
            *lcp_Ucode = (unsigned char)((*lcp_Ucode) - lcav_PswCode[lcv_Index]);
        }

        lcp_Ucode++;
    }
    LOGD(TAG, "Leave %s()", __func__);
}
  
/*
ADDR        INFO                 USIZE   SIZE
----------------------------------------------
602FF000    PID2                 4k       4k
602FE000    PID1                 4k       4k
60260000    cbin                 128k     632k
60240000    CLange(no use)       128k     128k
60220000    UCODE分区            128k     128k
60210000    数据块分区           64k      64k
6020C000    GVT                  16k      16k
60208000    POU INFO             16k      16k
601C0000    MB_DL_NETCFG         128k     128k
601A0000    系统块分区           64k      128k
60120000    AppCode              128k     512k
6010C000    设备信息             4k       80k
60100000    bootloader           16k      48k
60080000    系统升级bin文件2     500k     512k
60000000    系统升级bin文件1     500k     512k

具体可查找 flash_page_info
*/
/**
  * @brief  写缓存区程序至Flash特定区域
  * @param  None
  * @retval None
  */
void mb_slave_ctrl_programme_cmd(md_slave_msg_pack *pMsg)
{
    unsigned char i;
    unsigned long llv_FileLen;
    unsigned char *lcp_Buffer;
    unsigned long lVersion;
    unsigned short nProductType;
    LOGV(TAG, "Enter %s()", __func__);

    mem_part_info_t erasePart;
    /*定义各文件存储FLASH分区开始地址,及分区大小*/
    mem_part_info_t ltv_FlashPartInfo[] = {
        /*UCODE分区 0 */
        {/*  17  128K ucode   */
            0x60220000,
            UCODE_FILE_MAX_SIZE,
        },
        /*系统块分区 1*/
        { /*  13  64K sys      */
            0x601A0000,
            SYS_BLOCK_MAX_SIZE,
        },
        /*数据块分区 2*/
        {  /*  16  64K db       */
            0x60210000,
            DATA_BLOCK_MAX_SIZE,
        },
        /*POU INFO 3*/
        {  /*  14  16K pou      */
            0x60208000,
            POU_FILE_MAX_SIZE,
        },
        /*GVT 4*/
        {  /*  15  16K gvt      */
            0x6020C000,
            GVT_FILE_MAX_SIZE,
        },
        /*MB_DL_NETCFG 5*/
        { /* 10  */
            0x601C0000,
            DEV_NETWORK_INFO_SIZE,
        },
        /*系统升级bin文件 6*/
        {  /* 两个bin，一个放到0x60080000，一个放到0x60000000，对于RT1061只能如此 */
            0x60080000,
            SYSTEM_UPGRADE_MAX_SIZE,
        },
        /* cbin  7 */
        {  /*  19 CBIN_UPGRADE_START_PAGE */
            0x60260000,
            CBIN_MAX_SIZE,
        },
        /* PID  8 */
        {  /*  19 CBIN_UPGRADE_START_PAGE */
            0x602FE000,
            PID1_MAX_SIZE,
        },
        /* PID  9 */
        {  /*  19 CBIN_UPGRADE_START_PAGE */
            0x602FF000,
            PID2_MAX_SIZE,
        },
    };

    for(i=0; i<MB_DL_MAX; i++) {
        LOGD(TAG, "i = %d, flag = %d", i, gtv_ModbusFileTrans[i]->mcv_Flag);
        switch(i) {
            case MB_DL_UCODE:
            case MB_DL_SYS_BLOCK:
            case MB_DL_DATA_BLOCK:
            case MB_DL_POU_INFO:
            case MB_DL_GVT:
            case MB_DL_NETCFG:
            case MB_DL_PID1:
            case MB_DL_PID2:
                /*文件传输完成,写入Flash*/
                if((NULL != gtv_ModbusFileTrans[i]) && (gtv_ModbusFileTrans[i]->mcv_Flag & 0x02))
                {
                    llv_FileLen = plc_get_file_length(gtv_ModbusFileTrans[i]->mcp_FileHandler + FILE_LEN_INFO_START_INDEX, 4);
                    LOGD(TAG, "file length = %d", llv_FileLen);
                    if(llv_FileLen > ltv_FlashPartInfo[i].partSize) {
                        /*释放动态内存*/
                        LOGE(TAG, "Oh MyGod! file length too long  = %d exceed%u", llv_FileLen,ltv_FlashPartInfo[i].partSize);
                        vPortFree(gtv_ModbusFileTrans[i]->mcp_FileHandler);
                        gtv_ModbusFileTrans[i]->mcp_FileHandler = NULL;
                        vPortFree(gtv_ModbusFileTrans[i]);
                        gtv_ModbusFileTrans[i] = NULL;

                        pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
                        mb_slave_error_resp(pMsg);
                        return;
                    }
                    /*喂狗*/
                    LOGD(TAG, "bsp_feed_watch_dog");
                    bsp_feed_watch_dog();

                    erasePart.startAddr = ltv_FlashPartInfo[i].startAddr;
                    uint32_t chuShu = llv_FileLen / SECTOR_SIZE;
                    LOGD(TAG, "chuShu = %u", chuShu);
                    chuShu++;
                    erasePart.partSize = SECTOR_SIZE * chuShu;

                    /*格式化目标分区*/
                    //bsp_flash_erase_partition(&ltv_FlashPartInfo[i]);
                    bsp_flash_erase_partition(&erasePart);
                    /*喂狗*/
                    bsp_feed_watch_dog();

                    /*写文件到目标分区*/
                    bsp_flash_write_buffer(ltv_FlashPartInfo[i].startAddr, 
                                            gtv_ModbusFileTrans[i]->mcp_FileHandler, 
                                            gtv_ModbusFileTrans[i]->mlv_FileLen);
                    LOGD(TAG, "mcp_FileHandler = 0x%08X, xPortGetFreeHeapSize=%u\r\n", gtv_ModbusFileTrans[i]->mcp_FileHandler, xPortGetFreeHeapSize());
                    /*释放动态内存*/
                    vPortFree(gtv_ModbusFileTrans[i]->mcp_FileHandler);
                    gtv_ModbusFileTrans[i]->mcp_FileHandler = NULL;
                    vPortFree(gtv_ModbusFileTrans[i]);
                    gtv_ModbusFileTrans[i] = NULL;
                }
                break;

            case MB_DL_SYS_UPGRADE:
                /*文件传输完成,写入Flash*/
                if((NULL != gtv_ModbusFileTrans[i]) && (gtv_ModbusFileTrans[i]->mcv_Flag & 0x02))
                {
                    /*文件长度大于目标区域长度，返回错误*/
                    if(gtv_ModbusFileTrans[i]->mlv_FileLen > ltv_FlashPartInfo[i].partSize)
                    {
                        LOGE(TAG, "gtv_ModbusFileTrans[i]->mlv_FileLen(%u) > ltv_FlashPartInfo[i].partSize(%u)",gtv_ModbusFileTrans[i]->mlv_FileLen,ltv_FlashPartInfo[i].partSize );
                        /*释放动态内存*/
                        vPortFree(gtv_ModbusFileTrans[i]->mcp_FileHandler);
                        gtv_ModbusFileTrans[i]->mcp_FileHandler = NULL;
                        vPortFree(gtv_ModbusFileTrans[i]);
                        gtv_ModbusFileTrans[i] = NULL;

                        pMsg->mcv_ErrorCode = MB_ERROR_FRAME;
                        mb_slave_error_resp(pMsg);
                        return;
                    }
                #if (DAISY_MASTER_FEATURE == 1)
                    LOGD(TAG, "gSlaveIDUpgrade = %u", gSlaveIDUpgrade);
                    if (gSlaveIDUpgrade != 0)
                    {
                        gSlaveIDUpgrade = 0;
                        /*释放动态内存*/
                        vPortFree(gtv_ModbusFileTrans[i]->mcp_FileHandler);
                        gtv_ModbusFileTrans[i]->mcp_FileHandler = NULL;
                        vPortFree(gtv_ModbusFileTrans[i]);
                        gtv_ModbusFileTrans[i] = NULL;
                        break;
                    }
                #endif
                    uint32_t idx = ota_get_image_idx();
                    LOGD(TAG, "ota_get_image_idx() = %u", idx);
                    if (idx == 0)//We should same image to idx 1
                    {
                        /*喂狗*/

                        bsp_feed_watch_dog();
                        /*擦除目标文件区域*/
                        LOGD(TAG, "begin bsp_flash_erase_partition()");
                        bsp_flash_erase_partition(&ltv_FlashPartInfo[i]);
                        /*喂狗*/
                        bsp_feed_watch_dog();

                        LOGD(TAG, "begin bsp_write_m_flash_config()");
                        bsp_write_m_flash_config(ltv_FlashPartInfo[i].startAddr);

                        LOGD(TAG, "begin bsp_flash_write_buffer() to idx %u", idx);
                        /*写数据到目标文件区*/
                        bsp_flash_write_buffer(ltv_FlashPartInfo[i].startAddr + 0x1000, 
                                        gtv_ModbusFileTrans[i]->mcp_FileHandler, 
                                        gtv_ModbusFileTrans[i]->mlv_FileLen);
                        ota_program_once_MISC_CONF1();
                        LOGD(TAG, "ota_save_image_idx() to idx %u", idx);
                        ota_save_image_idx(1);
                    #if (OTA_LOGIC == 0)
                        bsp_imageID_plus_one();
                    #else
                        bsp_save_Image1IfReset(0);
                    #endif
                    }
                    else//We should same image to idx 0
                    {
                        /*喂狗*/
                        bsp_feed_watch_dog();
                        /*擦除目标文件区域*/
                        ltv_FlashPartInfo[i].startAddr = 0x60000000;

                        LOGD(TAG, "begin bsp_flash_erase_partition()");
                        bsp_flash_erase_partition(&ltv_FlashPartInfo[i]);

                        /*喂狗*/
                        bsp_feed_watch_dog();
                        LOGD(TAG, "begin bsp_write_m_flash_config()");
                        bsp_write_m_flash_config(ltv_FlashPartInfo[i].startAddr);

                        /*写数据到目标文件区*/
                        LOGD(TAG, "begin bsp_flash_write_buffer() to idx %u", idx);
                        bsp_flash_write_buffer(ltv_FlashPartInfo[i].startAddr + 0x1000, 
                                        gtv_ModbusFileTrans[i]->mcp_FileHandler, 
                                        gtv_ModbusFileTrans[i]->mlv_FileLen);

                        LOGD(TAG, "ota_save_image_idx() to idx %u", idx);
                        ota_save_image_idx(0);
                    #if (OTA_LOGIC == 0)
                        bsp_imageID_plus_one();
                    #endif
                    }
                    /*释放动态内存*/
                    vPortFree(gtv_ModbusFileTrans[i]->mcp_FileHandler);
                    gtv_ModbusFileTrans[i]->mcp_FileHandler = NULL;
                    vPortFree(gtv_ModbusFileTrans[i]);
                    gtv_ModbusFileTrans[i] = NULL;
                }
                break;

            case MB_DL_PLC_CBIN:
                if((NULL != gtv_ModbusFileTrans[i]) && (gtv_ModbusFileTrans[i]->mcv_Flag & 0x02))
                {
                    lcp_Buffer = gtv_ModbusFileTrans[i]->mcp_FileHandler;

                    //毛继科:20120416更改了CBIN的文件结构 
                    //最新的文件结构：头标记（四字节CSRF）+ 文件长度（4字节）+ 版本号（4字节）+ 产品型号（2字节）+ 保留12字节									

                    /*跳过文件头CSRF*/
                    lcp_Buffer += 4;
                    /*跳过文件长度*/
                    lcp_Buffer += 4;
                    //版本号
                    lVersion = GET_PU32_DATA(lcp_Buffer);
                    lcp_Buffer += 4;
                    /*型号*/
                    nProductType = GET_PU16_DATA(lcp_Buffer);
                    lcp_Buffer += 2;
                    //跳过12字节保留
                    lcp_Buffer += 12;
                    bsp_feed_watch_dog();
                    /*固化UCODE*/
                    llv_FileLen = plc_get_file_length(lcp_Buffer, 4);
                    LOGV(TAG, "ProductType is %u,UCODE length is %u, partSize = %u", nProductType, llv_FileLen, ltv_FlashPartInfo[0].partSize);
                    if(gtv_DeviceConfigTable.mtv_DevInfo.mlv_DeviceTypeId != nProductType || 
                       llv_FileLen > ltv_FlashPartInfo[0].partSize) 
                    {
                        /*释放动态内存*/
                        vPortFree(gtv_ModbusFileTrans[i]->mcp_FileHandler);
                        gtv_ModbusFileTrans[i]->mcp_FileHandler = NULL;
                        vPortFree(gtv_ModbusFileTrans[i]);
                        gtv_ModbusFileTrans[i] = NULL;

                        pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
                        mb_slave_error_resp(pMsg);
                        return;
                    }

                    lcp_Buffer += 4;

                    /*ucode解密*/
                    mb_slave_ctrl_descrypt_cbin_file(lcp_Buffer, llv_FileLen);

                    /*喂狗*/
                    bsp_feed_watch_dog();

                    /*格式化目标分区*/
                    bsp_flash_erase_partition(&ltv_FlashPartInfo[0]);

                    /*喂狗*/
                    bsp_feed_watch_dog();
                    LOGI(TAG, "xPortGetFreeHeapSize = %u", xPortGetFreeHeapSize());

                    /*写文件到目标分区*/
                    bsp_flash_write_buffer(ltv_FlashPartInfo[0].startAddr, lcp_Buffer, llv_FileLen);

                    lcp_Buffer += llv_FileLen;

                    /*固化POU*/
                    llv_FileLen = plc_get_file_length(lcp_Buffer, 4);
                    LOGV(TAG, "POU length is %u", llv_FileLen);
                    if(llv_FileLen > ltv_FlashPartInfo[3].partSize) {
                        /*释放动态内存*/
                        vPortFree(gtv_ModbusFileTrans[i]->mcp_FileHandler);
                        gtv_ModbusFileTrans[i]->mcp_FileHandler = NULL;
                        vPortFree(gtv_ModbusFileTrans[i]);
                        gtv_ModbusFileTrans[i] = NULL;

                        pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
                        mb_slave_error_resp(pMsg);
                        return;
                    }

                    lcp_Buffer += 4;

                    /*喂狗*/
                    bsp_feed_watch_dog();

                    /*格式化目标分区*/
                    bsp_flash_erase_partition(&ltv_FlashPartInfo[3]);

                    /*喂狗*/
                    bsp_feed_watch_dog();

                    /*写文件到目标分区*/
                    bsp_flash_write_buffer(ltv_FlashPartInfo[3].startAddr, lcp_Buffer, llv_FileLen);

                    lcp_Buffer += llv_FileLen;

                    /*固化GVT*/
                    llv_FileLen = plc_get_file_length(lcp_Buffer, 4);
                    LOGV(TAG, "GVT length is %u", llv_FileLen);
                    if(llv_FileLen > ltv_FlashPartInfo[4].partSize) {
                        /*释放动态内存*/
                        vPortFree(gtv_ModbusFileTrans[i]->mcp_FileHandler);
                        gtv_ModbusFileTrans[i]->mcp_FileHandler = NULL;
                        vPortFree(gtv_ModbusFileTrans[i]);
                        gtv_ModbusFileTrans[i] = NULL;

                        pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
                        mb_slave_error_resp(pMsg);
                        return;
                    }

                    lcp_Buffer += 4;

                    /*喂狗*/
                    bsp_feed_watch_dog();

                    /*格式化目标分区*/
                    bsp_flash_erase_partition(&ltv_FlashPartInfo[4]);

                    /*喂狗*/
                    bsp_feed_watch_dog();

                    /*写文件到目标分区*/
                    bsp_flash_write_buffer(ltv_FlashPartInfo[4].startAddr, lcp_Buffer, llv_FileLen);

                    lcp_Buffer += llv_FileLen;

                    /*固化系统块*/
                    llv_FileLen = plc_get_file_length(lcp_Buffer, 4);
                    LOGV(TAG, "System block length is %u", llv_FileLen);
                    if(llv_FileLen > ltv_FlashPartInfo[1].partSize) {
                        /*释放动态内存*/
                        vPortFree(gtv_ModbusFileTrans[i]->mcp_FileHandler);
                        gtv_ModbusFileTrans[i]->mcp_FileHandler = NULL;
                        vPortFree(gtv_ModbusFileTrans[i]);
                        gtv_ModbusFileTrans[i] = NULL;

                        pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
                        mb_slave_error_resp(pMsg);
                        return;
                    }

                    lcp_Buffer += 4;

                    /*喂狗*/
                    bsp_feed_watch_dog();

                    /*格式化目标分区*/
                    bsp_flash_erase_partition(&ltv_FlashPartInfo[1]);

                    /*喂狗*/
                    bsp_feed_watch_dog();

                    /*写文件到目标分区*/
                    bsp_flash_write_buffer(ltv_FlashPartInfo[1].startAddr, lcp_Buffer, llv_FileLen);

                    lcp_Buffer += llv_FileLen;

                    /*固化数据块*/
                    llv_FileLen = plc_get_file_length(lcp_Buffer, 4);
                    LOGV(TAG, "Data block length is %u", llv_FileLen);
                    if(llv_FileLen > ltv_FlashPartInfo[2].partSize) {
                        /*释放动态内存*/
                        vPortFree(gtv_ModbusFileTrans[i]->mcp_FileHandler);
                        gtv_ModbusFileTrans[i]->mcp_FileHandler = NULL;
                        vPortFree(gtv_ModbusFileTrans[i]);
                        gtv_ModbusFileTrans[i] = NULL;

                        pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
                        mb_slave_error_resp(pMsg);
                        return;
                    }

                    lcp_Buffer += 4;

                    /*数据块解密*/
                    mb_slave_ctrl_descrypt_cbin_file(lcp_Buffer, llv_FileLen);

                    /*喂狗*/
                    bsp_feed_watch_dog();

                    /*格式化目标分区*/
                    bsp_flash_erase_partition(&ltv_FlashPartInfo[2]);

                    /*喂狗*/
                    bsp_feed_watch_dog();

                    /*写文件到目标分区*/
                    bsp_flash_write_buffer(ltv_FlashPartInfo[2].startAddr, lcp_Buffer, llv_FileLen);

                    lcp_Buffer += llv_FileLen;

                    /*固化网络参数表*/
                    llv_FileLen = plc_get_file_length(lcp_Buffer, 4);
                    LOGV(TAG, "GVT length is %u", llv_FileLen);
                    if(llv_FileLen > ltv_FlashPartInfo[5].partSize) {
                        /*释放动态内存*/
                        vPortFree(gtv_ModbusFileTrans[i]->mcp_FileHandler);
                        gtv_ModbusFileTrans[i]->mcp_FileHandler = NULL;
                        vPortFree(gtv_ModbusFileTrans[i]);
                        gtv_ModbusFileTrans[i] = NULL;

                        pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
                        mb_slave_error_resp(pMsg);
                        return;
                    }

                    lcp_Buffer += 4;
                    /*固化网络参数表解密*/
                    mb_slave_ctrl_descrypt_cbin_file(lcp_Buffer, llv_FileLen);

                    /*喂狗*/
                    bsp_feed_watch_dog();

                    /*格式化目标分区*/
                    bsp_flash_erase_partition(&ltv_FlashPartInfo[5]);

                    /*喂狗*/
                    bsp_feed_watch_dog();

                    /*写文件到目标分区*/
                    bsp_flash_write_buffer(ltv_FlashPartInfo[5].startAddr, lcp_Buffer, llv_FileLen);

                    lcp_Buffer += llv_FileLen;

                    /*固化PID参数1*/
                    llv_FileLen = plc_get_file_length(lcp_Buffer, 4);
                    LOGV(TAG, "GVT length is %u", llv_FileLen);
                    if(llv_FileLen > ltv_FlashPartInfo[8].partSize) {
                        /*释放动态内存*/
                        vPortFree(gtv_ModbusFileTrans[i]->mcp_FileHandler);
                        gtv_ModbusFileTrans[i]->mcp_FileHandler = NULL;
                        vPortFree(gtv_ModbusFileTrans[i]);
                        gtv_ModbusFileTrans[i] = NULL;

                        pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
                        mb_slave_error_resp(pMsg);
                        return;
                    }

                    lcp_Buffer += 4;

                    /*喂狗*/
                    bsp_feed_watch_dog();

                    /*格式化目标分区*/
                    bsp_flash_erase_partition(&ltv_FlashPartInfo[8]);

                    /*喂狗*/
                    bsp_feed_watch_dog();

                    /*写文件到目标分区*/
                    bsp_flash_write_buffer(ltv_FlashPartInfo[8].startAddr, lcp_Buffer, llv_FileLen);

                    lcp_Buffer += llv_FileLen;

                    /*固化PID参数2*/
                    llv_FileLen = plc_get_file_length(lcp_Buffer, 4);
                    LOGV(TAG, "GVT length is %u", llv_FileLen);
                    if(llv_FileLen > ltv_FlashPartInfo[9].partSize) {
                        /*释放动态内存*/
                        vPortFree(gtv_ModbusFileTrans[i]->mcp_FileHandler);
                        gtv_ModbusFileTrans[i]->mcp_FileHandler = NULL;
                        vPortFree(gtv_ModbusFileTrans[i]);
                        gtv_ModbusFileTrans[i] = NULL;

                        pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
                        mb_slave_error_resp(pMsg);
                        return;
                    }

                    lcp_Buffer += 4;

                    /*喂狗*/
                    bsp_feed_watch_dog();

                    /*格式化目标分区*/
                    bsp_flash_erase_partition(&ltv_FlashPartInfo[9]);

                    /*喂狗*/
                    bsp_feed_watch_dog();

                    /*写文件到目标分区*/
                    bsp_flash_write_buffer(ltv_FlashPartInfo[9].startAddr, lcp_Buffer, llv_FileLen);

                    lcp_Buffer += llv_FileLen;

                    vPortFree(gtv_ModbusFileTrans[i]->mcp_FileHandler);
                    gtv_ModbusFileTrans[i]->mcp_FileHandler = NULL;
                    vPortFree(gtv_ModbusFileTrans[i]);
                    gtv_ModbusFileTrans[i] = NULL;
                }
                break;
        }
    }

    /*组响应帧，特殊的响应帧在对应函数完成*/
    for(i=0; i<3; i++)
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];

    pMsg->msv_RespLen = 3;
    mb_slave_verify_resp_msg(pMsg);
}

/**
  * @brief  清除PLC用户程序
  * @param  None
  * @retval None
  */
void mb_slave_ctrl_clean_user_code(md_slave_msg_pack *pMsg)
{
    LOGD(TAG, "Enter %s()", __func__);
    flash_part_info_t *ltp_FlashPtr;
    mem_part_info_t ltv_MemInfo;

    /*清除POU存储区域*/
    gtv_UserFilePtrSt.PouInfoPtr = (unsigned char *)pouinfo_default;
    ltp_FlashPtr = bsp_get_flash_info(POU_FILE_START_PAGE);
    if(ltp_FlashPtr == (flash_part_info_t *)0) {
        pMsg->mcv_ErrorCode = MB_ERROR_SLAVE_OP;
        mb_slave_error_resp(pMsg);
        return;
    }

    ltv_MemInfo.startAddr = ltp_FlashPtr->startAddr;
    ltv_MemInfo.partSize = (ltp_FlashPtr->endAddr - ltp_FlashPtr->startAddr + 1);
    bsp_flash_erase_partition(&ltv_MemInfo);

    /*清除GVT存储区域*/
    gtv_UserFilePtrSt.GvtPtr = (unsigned char *)gvt_default;
    ltp_FlashPtr = bsp_get_flash_info(GVT_FILE_START_PAGE);
    if(ltp_FlashPtr == (flash_part_info_t *)0) {
        pMsg->mcv_ErrorCode = MB_ERROR_SLAVE_OP;
        mb_slave_error_resp(pMsg);
        return;
    }

    ltv_MemInfo.startAddr = ltp_FlashPtr->startAddr;
    ltv_MemInfo.partSize = (ltp_FlashPtr->endAddr - ltp_FlashPtr->startAddr + 1);
    bsp_flash_erase_partition(&ltv_MemInfo);

    /*清除UCODE存储区域*/
    gtv_UserFilePtrSt.UCodePtr = (unsigned char *)ucode_default;
    ltp_FlashPtr = bsp_get_flash_info(UCODE_FILE_START_PAGE);
    if(ltp_FlashPtr == (flash_part_info_t *)0) {
        pMsg->mcv_ErrorCode = MB_ERROR_SLAVE_OP;
        mb_slave_error_resp(pMsg);
        return;
    }

    ltv_MemInfo.startAddr = ltp_FlashPtr->startAddr;
    ltv_MemInfo.partSize = (ltp_FlashPtr->endAddr - ltp_FlashPtr->startAddr + 1);
    bsp_flash_erase_partition(&ltv_MemInfo);
}

/**
  * @brief  清除PLC系统块
  * @param  None
  * @retval None
  */

void mb_slave_ctrl_clean_system_block(md_slave_msg_pack *pMsg)
{
    flash_part_info_t *ltp_FlashPtr;
    mem_part_info_t ltv_MemInfo;

    /*清除系统块*/
    gtv_UserFilePtrSt.SysBlockPtr = (unsigned char *)sysblock_default;
    ltp_FlashPtr = bsp_get_flash_info(SYS_BLOCK_START_PAGE);
    if(ltp_FlashPtr == (flash_part_info_t *)0) {
        pMsg->mcv_ErrorCode = MB_ERROR_SLAVE_OP;
        mb_slave_error_resp(pMsg);
        return;
    }

    ltv_MemInfo.startAddr = ltp_FlashPtr->startAddr;
    ltv_MemInfo.partSize = (ltp_FlashPtr->endAddr - ltp_FlashPtr->startAddr + 1);
    bsp_flash_erase_partition(&ltv_MemInfo);
}

/**
  * @brief  清除PLC数据块
  * @param  None
  * @retval None
  */
void mb_slave_ctrl_clean_data_block(md_slave_msg_pack *pMsg)
{
    flash_part_info_t *ltp_FlashPtr;
    mem_part_info_t ltv_MemInfo;

    /*清除数据块*/
    gtv_UserFilePtrSt.DataBlockPtr = (unsigned char *)datablock_default;
    ltp_FlashPtr = bsp_get_flash_info(DATA_BLOCK_START_PAGE);
    if(ltp_FlashPtr == (flash_part_info_t *)0) {
        pMsg->mcv_ErrorCode = MB_ERROR_SLAVE_OP;
        mb_slave_error_resp(pMsg);
        return;
    }

    ltv_MemInfo.startAddr = ltp_FlashPtr->startAddr;
    ltv_MemInfo.partSize = (ltp_FlashPtr->endAddr - ltp_FlashPtr->startAddr + 1);
    bsp_flash_erase_partition(&ltv_MemInfo);
}

/**
  * @brief  清除PLC网络配置
  * @param  None
  * @retval None
  */
void mb_slave_ctrl_clean_net_cfg(md_slave_msg_pack *pMsg)
{

    flash_part_info_t *ltp_FlashPtr;
    mem_part_info_t ltv_MemInfo;

    /*清除网络配置*/
    gtv_UserFilePtrSt.NetcfgBlockPtr = (unsigned char *)netcfg_default;
    ltp_FlashPtr = bsp_get_flash_info(DEV_NETWORK_INFO_START_FLASH_PAGE);
    if(ltp_FlashPtr == (flash_part_info_t *)0) {
        pMsg->mcv_ErrorCode = MB_ERROR_SLAVE_OP;
        mb_slave_error_resp(pMsg);
        return;
    }

    ltv_MemInfo.startAddr = ltp_FlashPtr->startAddr;
    ltv_MemInfo.partSize = (ltp_FlashPtr->endAddr - ltp_FlashPtr->startAddr + 1);
    bsp_flash_erase_partition(&ltv_MemInfo);
}

/**
  * @brief  清除PLC,格式化
  * @param  None
  * @retval None
  */
void mb_slave_ctrl_clean_all_user_data(md_slave_msg_pack *pMsg)
{
    LOGD(TAG, "Enter %s()", __func__);
    mb_slave_ctrl_clean_user_code(pMsg);
    mb_slave_ctrl_clean_system_block(pMsg);
    mb_slave_ctrl_clean_data_block(pMsg);
    mb_slave_ctrl_clean_net_cfg(pMsg);	
    plc_password_erase();
    
    plc_element_value_init();
    plc_eu_ed_init();
    plc_HSP_init();
    plc_sm_sd_init();
    mb_slave_ctrl_unforce_all(pMsg);
    plc_clean_user_run_error_flag();
}

/**
  * @brief  读取PLC信息
  * @param  None
  * @retval None
  */
void mb_slave_ctrl_read_plc_info(md_slave_msg_pack *pMsg)
{
    LOGD(TAG, "Enter %s()", __func__);
    unsigned char i;

    for(i=0; i<3; i++)
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];

    /*设备类型*/
    pMsg->mcp_RespBuff[3] = (unsigned char)(gtv_DeviceConfigTable.mtv_DevInfo.mlv_DeviceTypeId >> 8);
    pMsg->mcp_RespBuff[4] = (unsigned char)(gtv_DeviceConfigTable.mtv_DevInfo.mlv_DeviceTypeId & 0xFF);

    /*设备ID*/
    for(i=0; i<12; i++) {
        pMsg->mcp_RespBuff[5+i] = gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[i];
    }

    LOGV(TAG, "Version:%u, Rongliang:%u\r\n", GET_SD_ELEMENT_VALUE(1), GET_SD_ELEMENT_VALUE(2));
    /*版本号*/
    uint16_t ver = FIRMWARE_IMAGE_ID;
    pMsg->mcp_RespBuff[17] = (unsigned char)(ver >> 0x08);
    pMsg->mcp_RespBuff[18] = (unsigned char)(ver & 0xFF);

    /*程序容量*/
    ver = PROGRAM_CAPACITY;
    pMsg->mcp_RespBuff[19] = (unsigned char)(ver >> 0x08);
    pMsg->mcp_RespBuff[20] = (unsigned char)(ver & 0xFF);
    /*系统错误*/
    pMsg->mcp_RespBuff[21] = (unsigned char)(GET_SD_ELEMENT_VALUE(3)>>0x08);
    pMsg->mcp_RespBuff[22] = (unsigned char)(GET_SD_ELEMENT_VALUE(3)&0xFF);
    /*运行错误*/
    pMsg->mcp_RespBuff[23] = (unsigned char)(GET_SD_ELEMENT_VALUE(20)>>0x08);
    pMsg->mcp_RespBuff[24] = (unsigned char)(GET_SD_ELEMENT_VALUE(20)&0xFF);
    /*电池电压值*/
    pMsg->mcp_RespBuff[25] = 0;
    pMsg->mcp_RespBuff[26] = 33;
    /*PLC运行状态*/
    pMsg->mcp_RespBuff[27] = 0;
    pMsg->mcp_RespBuff[28] = plc_get_bit_element_value(SM_ELEMENT, 0);
    /*当前扫描速率*/
    pMsg->mcp_RespBuff[29] = (unsigned char)(GET_SD_ELEMENT_VALUE(30)>>0x08);
    pMsg->mcp_RespBuff[30] = (unsigned char)(GET_SD_ELEMENT_VALUE(30)&0xFF);
    /*最小扫描速率*/
    pMsg->mcp_RespBuff[31] = (unsigned char)(GET_SD_ELEMENT_VALUE(31)>>0x08);
    pMsg->mcp_RespBuff[32] = (unsigned char)(GET_SD_ELEMENT_VALUE(31)&0xFF);
    /*最大扫描速率*/
    pMsg->mcp_RespBuff[33] = (unsigned char)(GET_SD_ELEMENT_VALUE(32)>>0x08);
    pMsg->mcp_RespBuff[34] = (unsigned char)(GET_SD_ELEMENT_VALUE(32)&0xFF);

    /*IP地址*/
    MEMCPY (&(pMsg->mcp_RespBuff[35]),(unsigned char *)&(g_plc_netcfg.wan.ip.addr),4);
    /*子网掩码*/
    MEMCPY (&(pMsg->mcp_RespBuff[39]),(unsigned char *)&(g_plc_netcfg.wan.mask.addr),4);
    /*网关*/
    MEMCPY (&(pMsg->mcp_RespBuff[43]),(unsigned char *)&(g_plc_netcfg.wan.gate.addr),4);
    /*DNS*/
    MEMCPY (&(pMsg->mcp_RespBuff[47]) , (unsigned char *)&(g_plc_netcfg.wan.dns.addr),4);

    /*当前连接*/
    pMsg->mcp_RespBuff[51] =  (unsigned char)(GET_SD_ELEMENT_VALUE(SD223)>>0x08);
    pMsg->mcp_RespBuff[52] =  (unsigned char)(GET_SD_ELEMENT_VALUE(SD223)&0xFF);
    /*4G信号强度*/
    pMsg->mcp_RespBuff[53] = (unsigned char)(GET_SD_ELEMENT_VALUE(SD226)>>0x08);
    pMsg->mcp_RespBuff[54] = (unsigned char)(GET_SD_ELEMENT_VALUE(SD226)&0xFF);

    /*串口0*/
    unsigned short us_Pro = GET_UART_SD_VALUE(0, UART_SD_MODE_CONFIG);
    pMsg->mcp_RespBuff[55] = 0xFF & ( us_Pro >> 8 );
    pMsg->mcp_RespBuff[56] = 0xFF & us_Pro;
    /*串口1*/
    us_Pro = GET_UART_SD_VALUE(1, UART_SD_MODE_CONFIG);
    pMsg->mcp_RespBuff[57] = 0xFF & ( us_Pro >> 8 );
    pMsg->mcp_RespBuff[58] = 0xFF & us_Pro;
    /*COM2*/
    us_Pro = GET_UART_SD_VALUE(2, UART_SD_MODE_CONFIG);
    pMsg->mcp_RespBuff[59] = 0xFF & ( us_Pro >> 8 );
    pMsg->mcp_RespBuff[60] = 0xFF & us_Pro;
    /*预留COM3 ~ COM5*/
    pMsg->mcp_RespBuff[61] = 0;
    pMsg->mcp_RespBuff[62] = 0;

    pMsg->mcp_RespBuff[63] = 0;
    pMsg->mcp_RespBuff[64] = 0;

    pMsg->mcp_RespBuff[65] = 0;
    pMsg->mcp_RespBuff[66] = 0;

    /*系统运行时间*/
    uint32_t mlv_Systime = gKalykeSecondTickCurrent;
    LOGD(TAG, "mlv_Systime = %u second", mlv_Systime);
    pMsg->mcp_RespBuff[67] = (unsigned char)(mlv_Systime >> 24);
    pMsg->mcp_RespBuff[68] = (unsigned char)(mlv_Systime >> 16);
    pMsg->mcp_RespBuff[69] = (unsigned char)(mlv_Systime >> 8);
    pMsg->mcp_RespBuff[70] = (unsigned char)(mlv_Systime);

    //版本号
    pMsg->mcp_RespBuff[71] = (unsigned char)(GET_SD_ELEMENT_VALUE(209)>>0x08);
    pMsg->mcp_RespBuff[72] = (unsigned char)(GET_SD_ELEMENT_VALUE(209)&0xFF);

    pMsg->mcp_RespBuff[73] = (unsigned char)(GET_SD_ELEMENT_VALUE(210)>>0x08);
    pMsg->mcp_RespBuff[74] = (unsigned char)(GET_SD_ELEMENT_VALUE(210)&0xFF);

    pMsg->mcp_RespBuff[75] = (unsigned char)(GET_SD_ELEMENT_VALUE(211)>>0x08);
    pMsg->mcp_RespBuff[76] = (unsigned char)(GET_SD_ELEMENT_VALUE(211)&0xFF);

    pMsg->mcp_RespBuff[77] = (unsigned char)(GET_SD_ELEMENT_VALUE(212)>>0x08);
    pMsg->mcp_RespBuff[78] = (unsigned char)(GET_SD_ELEMENT_VALUE(212)&0xFF);

    // LAN口FEXLINK扩展协议
    pMsg->mcp_RespBuff[79] = g_plc_netcfg.lan.ioExp;
    /*IP地址*/
    MEMCPY (&(pMsg->mcp_RespBuff[80]),(unsigned char *)&(g_plc_netcfg.lan.ip.addr),4);
    /*子网掩码*/
    MEMCPY (&(pMsg->mcp_RespBuff[84]),(unsigned char *)&(g_plc_netcfg.lan.mask.addr),4);
    /*网关*/
    MEMCPY (&(pMsg->mcp_RespBuff[88]),(unsigned char *)&(g_plc_netcfg.lan.gate.addr),4);

    // WAN口DHCP
    pMsg->mcp_RespBuff[92] = g_plc_netcfg.wan.ioExp;


    /*保留空间填充0xBB*/
    for (i = 93; i < 100; i++)
    {
        pMsg->mcp_RespBuff[i] = 0xBB;
    }


    pMsg->msv_RespLen = 100;
    mb_slave_verify_resp_msg(pMsg);
    
    //ota_reset();
    //showMem();
}


/**
  * @brief  读取PLC状态
  * @param  None
  * @retval None
  */
void mb_slave_ctrl_read_plc_status(md_slave_msg_pack *pMsg)
{
    unsigned char i;

    for(i=0; i<3; i++)
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];

    pMsg->mcp_RespBuff[3] = plc_get_bit_element_value(SM_ELEMENT, 0);
	
    pMsg->mcp_RespBuff[4] = (unsigned char)(GET_SD_ELEMENT_VALUE(30)>>0x08);
    pMsg->mcp_RespBuff[5] = (unsigned char)(GET_SD_ELEMENT_VALUE(30)&0xFF);	

	
    pMsg->msv_RespLen = 6;
    mb_slave_verify_resp_msg(pMsg);
}

/**
  * @brief  读取PLC信息
  * @param  None
  * @retval None
  */
void mb_slave_ctrl_verify_ucode(md_slave_msg_pack *pMsg)
{
    LOGD(TAG, "Enter %s()", __func__);
    unsigned short i;
    unsigned long llv_FileLen;
    flash_part_info_t *ltp_UCodeInfo = bsp_get_flash_info(UCODE_FILE_START_PAGE);
    unsigned char *mcp_UcodePtr = (unsigned char *)ltp_UCodeInfo->startAddr;

    for(i=0; i<3; i++) {
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];
    }

    /*拷贝文件名*/
    for(i=0; i<16; i++) {
        pMsg->mcp_RespBuff[3+i] = GET_PU8_DATA(mcp_UcodePtr + 6 + i);
    }

    /*获取文件长度*/
    llv_FileLen = plc_get_file_length(mcp_UcodePtr + FILE_LEN_INFO_START_INDEX, 4);
    LOGD(TAG, "llv_FileLen = %d, llv_FileLen = 0x%x", llv_FileLen, llv_FileLen);

    for(i=0; i<2; i++) {
        pMsg->mcp_RespBuff[19+i] = GET_PU8_DATA(mcp_UcodePtr + llv_FileLen - 4 + i);
    }

    pMsg->msv_RespLen = 21;
    mb_slave_verify_resp_msg(pMsg);
}


void mb_slave_ctrl_verify_pid1(md_slave_msg_pack *pMsg)
{
    LOGD(TAG, "Enter %s()", __func__);
    unsigned short i;
    unsigned long llv_FileLen;
    flash_part_info_t *ltp_UCodeInfo = bsp_get_flash_info(PID1_START_PAGE);
    unsigned char *mcp_UcodePtr = (unsigned char *)ltp_UCodeInfo->startAddr;

    for(i=0; i<3; i++) {
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];
    }

    /*拷贝文件名*/
    for(i=0; i<16; i++) {
        pMsg->mcp_RespBuff[3+i] = GET_PU8_DATA(mcp_UcodePtr + 6 + i);
    }

    /*获取文件长度*/
    llv_FileLen = plc_get_file_length(mcp_UcodePtr + FILE_LEN_INFO_START_INDEX, 4);
    LOGD(TAG, "llv_FileLen = %d, llv_FileLen = 0x%x", llv_FileLen, llv_FileLen);

    for(i=0; i<2; i++) {
        pMsg->mcp_RespBuff[19+i] = GET_PU8_DATA(mcp_UcodePtr + llv_FileLen - 4 + i);
    }

    pMsg->msv_RespLen = 21;
    mb_slave_verify_resp_msg(pMsg);
}



/**
  * @brief  清除错误信息
  * @param  None
  * @retval None
  */
static void mb_slave_ctrl_error_info(md_slave_msg_pack *pMsg)
{
    LOGD(TAG, "Enter %s()", __func__);
    plc_clean_user_run_error_flag();
}

/**
  * @brief  清除错误信息
  * @param  None
  * @retval None
  */
static void mb_slave_ctrl_clean_element(md_slave_msg_pack *pMsg)
{
    plc_element_value_init();
    plc_sm_sd_init();
    plsd_erase_all();
}

/**
  * @brief  设置禁止上载标志
  * @param  None
  * @retval None
  */
static void mb_slave_ctrl_set_upload_forbid(md_slave_msg_pack *pMsg)
{
    plc_set_upload_forbid_flag(1);
}

/**
  * @brief  清除禁止上载标志
  * @param  None
  * @retval None
  */
static void mb_slave_clean_upload_forbid(md_slave_msg_pack *pMsg)
{
    plc_set_upload_forbid_flag(0);
}

/**
  * @brief  设置用户下载,上载,监控密码
  * @param  mcv_PwdType 密码类型, 0:下载, 1:上载,  2:监控
  * @retval None
  */
static void mb_slave_ctrl_set_user_password(md_slave_msg_pack *pMsg, unsigned char lcv_PwdType)
{
    unsigned char lcv_OldPwLen, lcv_NewPwLen;
    unsigned char lcv_Ret;

    lcv_OldPwLen = pMsg->mcp_ReceiveBuff[3];
    lcv_NewPwLen = pMsg->msv_ReceiveLen - 6 - lcv_OldPwLen;

    if(lcv_OldPwLen > MAX_PASSWORD_LEN || lcv_NewPwLen > MAX_PASSWORD_LEN) {
        pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
        mb_slave_error_resp(pMsg);
        return;
    }

    lcv_Ret = plc_check_password_info(lcv_PwdType, &pMsg->mcp_ReceiveBuff[4], lcv_OldPwLen);
    if(lcv_Ret != pdPASS) {
        pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
        mb_slave_error_resp(pMsg);
        return;
    }

    plc_update_password_info(lcv_PwdType, &pMsg->mcp_ReceiveBuff[4+lcv_OldPwLen], lcv_NewPwLen);
}

/**
  * @brief  用户下载,上载,监控密码验证
  * @param  mcv_PwdType 密码类型, 0:下载, 1:上载,  2:监控
  * @retval None
  */
static void mb_slave_ctrl_check_user_password(md_slave_msg_pack *pMsg, unsigned char lcv_PwdType)
{
    unsigned char lcv_PwLen;
    unsigned char lcv_Ret;

    lcv_PwLen = pMsg->msv_ReceiveLen - 5;
    lcv_Ret = plc_check_password_info(lcv_PwdType, &pMsg->mcp_ReceiveBuff[3], lcv_PwLen);
    if(lcv_Ret != pdPASS) {
        pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
        mb_slave_error_resp(pMsg);
        return;
    }
}

/**
  * @brief  进入在线编程模式
  * @param  None
  * @retval None
  */
static void mb_slave_ctrl_entry_online_program(md_slave_msg_pack *pMsg)
{
    unsigned char i;

    gtv_PlcRunStatus.mcv_IsOnlineProgram = 1;

    /*组响应帧*/
    for(i=0; i<pMsg->msv_ReceiveLen-2; i++)
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];

    pMsg->msv_RespLen = pMsg->msv_ReceiveLen-2;
    mb_slave_verify_resp_msg(pMsg);
}

/**
  * @brief  退出在线编程模式
  * @param  None
  * @retval None
  */
static void mb_slave_ctrl_exit_online_program(md_slave_msg_pack *pMsg)
{
    unsigned char i;

    gtv_PlcRunStatus.mcv_IsOnlineProgram = 0;

    /*组响应帧*/
    for(i=0; i<pMsg->msv_ReceiveLen-2; i++)
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];

    pMsg->msv_RespLen = pMsg->msv_ReceiveLen-2;
    mb_slave_verify_resp_msg(pMsg);
}

/**
  * @brief  KS读取错误信息
  * @param  None
  * @retval None
  */
static void mb_slave_read_error_info(md_slave_msg_pack *pMsg)
{
    unsigned char i;
    unsigned char *lcp_MsgBuff;
    unsigned long llv_Temp;

    lcp_MsgBuff = &pMsg->mcp_RespBuff[3];

    /*清除发送缓冲区*/
    for(i=0; i<30; i++) {
        lcp_MsgBuff[i] = 0;
    }

    switch(gtv_PlcExecErrorRecord.mcv_ErrorCnt) {
        case 5:
            lcp_MsgBuff[24] = gtv_PlcExecErrorRecord.msv_ErrorMsg[4] >> 8;
            lcp_MsgBuff[25] = gtv_PlcExecErrorRecord.msv_ErrorMsg[4]&0xFF;
            llv_Temp = gtv_PlcExecErrorRecord.msv_ErrorAddr[4]/2;
            lcp_MsgBuff[26] = (llv_Temp >> 24);
            lcp_MsgBuff[27] = (llv_Temp >> 16);
            lcp_MsgBuff[28] = (llv_Temp >> 8);
            lcp_MsgBuff[29] = (llv_Temp&0xFF);

        case 4:
            lcp_MsgBuff[18] = gtv_PlcExecErrorRecord.msv_ErrorMsg[3] >> 8;
            lcp_MsgBuff[19] = gtv_PlcExecErrorRecord.msv_ErrorMsg[3]&0xFF;
            llv_Temp = gtv_PlcExecErrorRecord.msv_ErrorAddr[3]/2;
            lcp_MsgBuff[20] = (llv_Temp >> 24);
            lcp_MsgBuff[21] = (llv_Temp >> 16);
            lcp_MsgBuff[22] = (llv_Temp >> 8);
            lcp_MsgBuff[23] = (llv_Temp&0xFF);

        case 3:
            lcp_MsgBuff[12] = gtv_PlcExecErrorRecord.msv_ErrorMsg[2] >> 8;
            lcp_MsgBuff[13] = gtv_PlcExecErrorRecord.msv_ErrorMsg[2]&0xFF;
            llv_Temp = gtv_PlcExecErrorRecord.msv_ErrorAddr[2]/2;
            lcp_MsgBuff[14] = (llv_Temp >> 24);
            lcp_MsgBuff[15] = (llv_Temp >> 16);
            lcp_MsgBuff[16] = (llv_Temp >> 8);
            lcp_MsgBuff[17] = (llv_Temp&0xFF);

        case 2:
            lcp_MsgBuff[6] = gtv_PlcExecErrorRecord.msv_ErrorMsg[1] >> 8;
            lcp_MsgBuff[7] = gtv_PlcExecErrorRecord.msv_ErrorMsg[1]&0xFF;
            llv_Temp = gtv_PlcExecErrorRecord.msv_ErrorAddr[1]/2;
            lcp_MsgBuff[8] = (llv_Temp >> 24);
            lcp_MsgBuff[9] = (llv_Temp >> 16);
            lcp_MsgBuff[10] = (llv_Temp >> 8);
            lcp_MsgBuff[11] = (llv_Temp&0xFF);

        case 1:
            lcp_MsgBuff[0] = gtv_PlcExecErrorRecord.msv_ErrorMsg[0] >> 8;
            lcp_MsgBuff[1] = gtv_PlcExecErrorRecord.msv_ErrorMsg[0]&0xFF;
            llv_Temp = gtv_PlcExecErrorRecord.msv_ErrorAddr[0]/2;
            lcp_MsgBuff[2] = (llv_Temp >> 24);
            lcp_MsgBuff[3] = (llv_Temp >> 16);
            lcp_MsgBuff[4] = (llv_Temp >> 8);
            lcp_MsgBuff[5] = (llv_Temp&0xFF);
            break;
    }

    /*组响应帧*/
    for(i=0; i<3; i++)
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];

    pMsg->msv_RespLen = 33;
    mb_slave_verify_resp_msg(pMsg);
}

/**
  * @brief  写设备ID MAC地址等信息
  * @param  None
  * @retval None
  */
static void mb_slave_ctrl_write_device_info(md_slave_msg_pack *pMsg)
{
    unsigned char i;
    unsigned char *lcp_InBuff = &pMsg->mcp_ReceiveBuff[3];

    gtv_DeviceConfigTable.mtv_DevInfo.mlv_Head = 0x44455649;

    for(i=0; i<6; i++) {
        gtv_DeviceConfigTable.mtv_DevInfo.mcv_EthernetMac[i] = *lcp_InBuff++;
    }

    for(i=0; i<6; i++) {
        gtv_DeviceConfigTable.mtv_DevInfo.mcv_WifiMac[i] = *lcp_InBuff++;
    }

    for(i=0; i<12; i++) {
        gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[i] = *lcp_InBuff++;
    }

    for(i=0; i<12; i++) {
        gtv_DeviceConfigTable.mtv_DevInfo.mcv_DevicePasswd[i] = *lcp_InBuff++;
    }

    gtv_DeviceConfigTable.mtv_DevInfo.mlv_DeviceTypeId = *((unsigned short *)lcp_InBuff);

    gtv_DeviceConfigTable.mtv_DevInfo.mlv_Crc32 = calc_crc32((unsigned char *)&gtv_DeviceConfigTable.mtv_DevInfo, (sizeof(bsp_device_info_st)-4));

    /*写入flash*/
    bsp_write_device_info_to_flash();
}

static TaskHandle_t gLedNetTaskHandle = NULL;
static void led_net_blink_task(void *p_arg)
{
    //uint8_t bak = GPIO_PinReadPadStatus(LED_2, LED_2_PIN);
    //LOGV("mb_ctrl", "bak = %u", bak);
    uint8_t bak1 = GPIO_PinRead(LED_5, LED_5_PIN);
    //LOGD("mb_ctrl", "bak1 = %u", bak1);
    uint32_t tickBegin = xTaskGetTickCount();
    while(1)
    {
        bsp_open_led_mqtt();
        vTaskDelay(100);
        bsp_close_led_mqtt();
        vTaskDelay(100);
        if (xTaskGetTickCount() - tickBegin > 6000)
        {
            break;
        }
    }
    if (bak1)
    {
        bsp_close_led_mqtt();
    }
    else
    {
        bsp_open_led_mqtt();
    }
    gLedNetTaskHandle = NULL;
    vTaskDelete(NULL);
}

static void mb_slave_blink_net_led(md_slave_msg_pack *pMsg)
{
    if (gLedNetTaskHandle == NULL)
    {
        xTaskCreate((TaskFunction_t)led_net_blink_task,
                    (const char *)"net_led_blink",
                    512,
                    (void *)NULL,
                    (UBaseType_t )14,
                    &gLedNetTaskHandle);
    }
}

#if (KALYKE_TOUCHUAN_WAN_LAN == 1)
//只能透传至LAN
static void mb_slave_set_touchuan(uint8_t tou)
{
    LOGV(TAG, "Enter %s(), tou = %u", __func__, tou);
#if 0
    gTouChuan = 4;
    SET_SD_ELEMENT_VALUE(SD231, tou);

    re_init_ENET2_LAN();
#else
    bsp_save_TouChuan(4);
#endif
}
#endif

/**
  * @brief  CTRL 功能码处理函数
  * @param  None
  * @retval None
  */
void mb_slave_ctrl_manage(md_slave_msg_pack *pMsg)
{
    unsigned char i;
    LOGI(TAG, "Enter %s(), pMsg->mcp_ReceiveBuff[2] = 0x%X", __func__, pMsg->mcp_ReceiveBuff[2]);
    bsp_feed_watch_dog();
    switch(pMsg->mcp_ReceiveBuff[2]) {
        /*读取时钟信息*/
        case MB_CTRL_READ_RTC:
            mb_slave_ctrl_read_realtime(pMsg);
            return;
        /*写入时钟信息*/
        case MB_CTRL_WRITE_RTC:
            mb_slave_ctrl_set_realtime(pMsg);
            return;
        /*运行PLC程序*/
        case MB_CTRL_RUN_CMD:
            mb_slave_ctrl_run_cmd(pMsg);
            break;
        /*停止运行PLC*/
        case MB_CTRL_STOP_CMD:
            mb_slave_ctrl_stop_cmd(pMsg);
            break;
        /*批量强制位元件*/
        case MB_CTRL_FORCE_BITS:
            mb_slave_ctrl_force_bits(pMsg);
            break;
        /*批量字元件强制*/
        case MB_CTRL_FORCE_WORDS:
            mb_slave_ctrl_force_words(pMsg);
            break;
        /*取消位元件强制*/
        case MB_CTRL_UNFORCE_BITS:
            mb_slave_ctrl_unforce_bits(pMsg);
            break;
        /*取消字元件强制*/
        case MB_CTRL_UNFORCE_WORDS:
            mb_slave_ctrl_unforce_words(pMsg);
            break;
        /*取消全部强制*/
        case MB_CTRL_UNFORCE_ALL:
            mb_slave_ctrl_unforce_all(pMsg);
            break;
        /*读取PLC信息*/
        case MB_CTRL_READ_PLC_INFO:
            mb_slave_ctrl_read_plc_info(pMsg);
            return;
        /*读PLC状态*/
        case MB_CTRL_READ_PLC_STATUS:
            mb_slave_ctrl_read_plc_status(pMsg);
            return;
        /*固化指令*/
        case MB_CTRL_PROGRAMME_CMD:
            /*在线模式开启时，固化需要暂停PLC运行*/
            LOGD(TAG, "Begin GuHua!!!\r\n");
            unsigned char IsOnlineProgram = gtv_PlcRunStatus.mcv_IsOnlineProgram;
            if(IsOnlineProgram)
            {
                LOGE(TAG, "In online mode  we need SSSSStop plc before GuHua!!!\r\n");
                mb_slave_ctrl_stop_cmd(pMsg);
            }
            suspend_task_when_download_ucode();
            gGUHUAing = 1;
            mb_slave_ctrl_programme_cmd(pMsg);
            gGUHUAing = 0;
            resume_task_after_download_ucode();
            if(IsOnlineProgram)
            { 
                vTaskDelay(20);
                LOGE(TAG, "In online mode  we need RRRRRRun plc after GuHua!!!\r\n");
                mb_slave_ctrl_run_cmd(pMsg);
            }
            LOGD(TAG, "GuHua Over\r\n");
            return;
        /*清除用户程序*/
        case MB_CTRL_CLEAN_USER_CODE:
            if (guv_PlcSysBlkAdSetting.bit.forbidden_format == 0)
            {
                mb_slave_ctrl_clean_user_code(pMsg);
            }
            else
            {
                pMsg->mcv_ErrorCode = MB_ERROR_SLAVE_OP;
                mb_slave_error_resp(pMsg);
                return;
            }
            break;
        /*清除系统块*/
        case MB_CTRL_CLEAN_SYS_BLOCK:
            if (guv_PlcSysBlkAdSetting.bit.forbidden_format == 0)
            {
                mb_slave_ctrl_clean_system_block(pMsg);
            }
            else
            {
                pMsg->mcv_ErrorCode = MB_ERROR_SLAVE_OP;
                mb_slave_error_resp(pMsg);
                return;
            }
            break;
        /*清除数据块*/
        case MB_CTRL_CLEAN_DATA_BLOCK:
            if (guv_PlcSysBlkAdSetting.bit.forbidden_format == 0)
            {
                mb_slave_ctrl_clean_data_block(pMsg);
            }
            else
            {
                pMsg->mcv_ErrorCode = MB_ERROR_SLAVE_OP;
                mb_slave_error_resp(pMsg);
                return;
            }
            break;
        /*清除网络配置*/
        case MB_CTRL_CLEAN_NETCFG:
            if (guv_PlcSysBlkAdSetting.bit.forbidden_format == 0)
            {
                mb_slave_ctrl_clean_net_cfg(pMsg);
            }
            else
            {
                pMsg->mcv_ErrorCode = MB_ERROR_SLAVE_OP;
                mb_slave_error_resp(pMsg);
                return;
            }
            break;

        /*格式化*/
        case MB_CTRL_CLEAN_ALL:
            if (guv_PlcSysBlkAdSetting.bit.forbidden_format == 0)
            {
                mb_slave_ctrl_clean_all_user_data(pMsg);
            }
            else
            {
                pMsg->mcv_ErrorCode = MB_ERROR_SLAVE_OP;
                mb_slave_error_resp(pMsg);
                return;
            }
            break;
        /*UCODE校验*/
        case MB_CTRL_VERIFY_UCODE:
            mb_slave_ctrl_verify_ucode(pMsg);
            return;
        /*PID参数1校验*/
        case MB_CTRL_VERIFY_PID1:
            mb_slave_ctrl_verify_pid1(pMsg);
            return;
        /*复位*/
        case MB_CTRL_REBOOT:
            mb_slave_ctrl_system_reboot(pMsg);
            return;
        /*清除错误信息*/
        case MB_CTRL_CLEAN_ERROR_INFO:
            mb_slave_ctrl_error_info(pMsg);
            break;
        /*清除元件值*/
        case MB_CTRL_CLEAN_ELEMENT:
            mb_slave_ctrl_clean_element(pMsg);
            break;
        /*禁止上载*/
        case MB_CTRL_SET_UPLOAD_FORBID:
            mb_slave_ctrl_set_upload_forbid(pMsg);
            break;
        /*解除禁止上载*/
        case MB_CTRL_CLEAN_UPLOAD_FORBID:
            mb_slave_clean_upload_forbid(pMsg);
            break;
        /*读取错误信息*/
        case MB_CTRL_READ_ERROR_INFO:
            mb_slave_read_error_info(pMsg);
            return;
        /*设置上载密码*/
        case MB_CTRL_SET_UPLOAD_PWD:
            mb_slave_ctrl_set_user_password(pMsg, UPLOAD_PASSWORD);
            break;
        /*验证上载密码*/
        case MB_CTRL_CHECK_UPLOAD_PWD:
            mb_slave_ctrl_check_user_password(pMsg, UPLOAD_PASSWORD);
            break;
        /*设置下载密码*/
        case MB_CTRL_SET_DOWNLOAD_PWD:
            mb_slave_ctrl_set_user_password(pMsg, DOWNLOAD_PASSWORD);
            break;
        /*验证下载密码*/
        case MB_CTRL_CHECK_DOWNLOAD_PWD:
            mb_slave_ctrl_check_user_password(pMsg, DOWNLOAD_PASSWORD);
            break;
        /*设置监控密码*/
        case MB_CTRL_SET_MONITOR_PWD:
            mb_slave_ctrl_set_user_password(pMsg, MONITOR_PASSWORD);
            break;
        /*验证监控密码*/
        case MB_CTRL_CHECK_MONITOR_PWD:
            mb_slave_ctrl_check_user_password(pMsg, MONITOR_PASSWORD);
            break;
        /*设置时钟密码*/
        case MB_CTRL_SET_TIMER_PWD:
            mb_slave_ctrl_set_user_password(pMsg, TIMER_PASSWORD);
            break;
        /*验证时钟密码*/
        case MB_CTRL_CHECK_TIMER_PWD:
            mb_slave_ctrl_check_user_password(pMsg, TIMER_PASSWORD);
            break;		
        /*进入在线编程模式*/
        case MB_CTRL_ENTERY_ONLINE_PROGRAM:
            mb_slave_ctrl_entry_online_program(pMsg);
            return;
        /*退出在线编程模式*/
        case MB_CTRL_EXIT_ONLINE_PROGRAM:
            mb_slave_ctrl_exit_online_program(pMsg);
            return;
        /*写设备ID MAC地址等信息*/
        case MB_CTRL_WRITE_DEVICE_INFO:
            mb_slave_ctrl_write_device_info(pMsg);
            break;
        case MB_CTRL_BLINK_NET_LED:
            mb_slave_blink_net_led(pMsg);
            break;

        case MB_CTRL_TOU_CHUAN_COM0:
            gTouChuan = 0;
            SET_SD_ELEMENT_VALUE(SD231, 0);
            LOGV(TAG, "gTouChuan =  %u", gTouChuan);
            break;
        
        case MB_CTRL_TOU_CHUAN_COM1:
            gTouChuan = 1;
            SET_SD_ELEMENT_VALUE(SD231, 1);
            LOGV(TAG, "gTouChuan =  %u", gTouChuan);
            break;
            
        case MB_CTRL_TOU_CHUAN_COM2:
            gTouChuan = 2;
            SET_SD_ELEMENT_VALUE(SD231, 2);
            LOGV(TAG, "gTouChuan =  %u", gTouChuan);
            break;

#if (KALYKE_TOUCHUAN_WAN_LAN == 1)
        case MB_CTRL_TOU_CHUAN_WAN:
            mb_slave_set_touchuan(3);
            LOGV(TAG, "gTouChuan =  %u", gTouChuan);
            break;
        case MB_CTRL_TOU_CHUAN_LAN:
            mb_slave_set_touchuan(4);
            LOGV(TAG, "gTouChuan =  %u", gTouChuan);
            break;
#endif

        case MB_CTRL_TOU_CHUAN_CLOSE:
            gTouChuan = 0xFF;
            SET_SD_ELEMENT_VALUE(SD231, 0xFF);
            break;

        case MB_CTRL_SEARCH_SLAVE:
            daisy_get_info(pMsg);
            return;
    #if (ETHERCAT_SOEM == 1)
        case MB_CTRL_SEARCH_ETH_SLAVE:
            kalyke_slave_scan(pMsg);
            break;
    #endif

        /* 01 6A 90 01(SlaveID) */
        case MB_CTRL_SLAVE_ID:
        #if (DAISY_MASTER_FEATURE == 1)
            gSlaveIDUpgrade = pMsg->mcp_ReceiveBuff[3];
            LOGV(TAG, "gSlaveIDUpgrade = %u", gSlaveIDUpgrade);
        #endif
            break;

        default:
            gtp_ModbusSlaveDiagInfo[pMsg->mcv_Sender].msv_SlaveErrCnt ++;
            pMsg->mcv_ErrorCode = MB_ILIEGAL_CODE;
            mb_slave_error_resp(pMsg);
            return;
    }

    /*组响应帧，特殊的响应帧在对应函数完成*/
    for(i=0; i<3; i++)
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];

    pMsg->msv_RespLen = 3;
    mb_slave_verify_resp_msg(pMsg);
}

