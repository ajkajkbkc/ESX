/**
  ******************************************************************************
  * @file    mb_download.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   Modbus slave 下载子功能
  ******************************************************************************
  */

#include "fsl_debug_console.h"
#include "mb.h"
#include "mb_download.h"

#include "FreeRTOS.h"
#include "bsp_flash.h"
#include "bsp_dct.h"

#include "task.h"
#include "plc_variable.h"
#include "mb_maptable.h"
#include "list_func.h"
#include "plc_commonfunc.h"
#include "plc_password.h"
#include "mb_ctrl.h"
#include "plc_compiler.h"
#include "plc_errormsg.h"
#include "kalyke_tool.h"
#include "daisy_task.h"
#include "kalyke_event.h"
#include "bsp_iwdg.h"
#include "kalyke_monitor_task.h"

/*------------------------------------------------------------------------------
*   变量定义
*-----------------------------------------------------------------------------*/
static const char *TAG = "mb_download";
/*文件传输数组*/
mb_file_trans_st *gtv_ModbusFileTrans[MB_DL_MAX] = { NULL, };

/**
  * @brief  下载监控位元件
  * @param  None
  * @retval None
  */
void mb_slave_download_monitor_bits(md_slave_msg_pack *pMsg)
{
    struct list_head *ltp_Head;
    struct list_head *ltp_ForNext;
    struct list_head *ltp_ForCur;
    unsigned char lcv_FrameNum;
    unsigned char lcv_ElementNum;
    mb_monitor_element_t *ltp_MonitorData;
    unsigned short i;
    unsigned char *lcp_Temp;
    unsigned short lsv_ModbusAddr;
    unsigned char lcv_Ret;

    lcv_FrameNum = pMsg->mcp_ReceiveBuff[3];

    ltp_Head = &gtv_MbMonitorBits.head;

    if(lcv_FrameNum == 0x01 || lcv_FrameNum == 0xFE) {
        /*清除当前链表*/
        list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
            ltp_MonitorData = list_entry(ltp_ForCur, mb_monitor_element_t, list);
            list_del(ltp_ForCur);
            vPortFree(ltp_MonitorData);
        }

        /*需要监控元件数量*/
        lcv_ElementNum = pMsg->mcp_ReceiveBuff[6];

        gtv_MbMonitorBits.lsv_ListLen = 0;
        INIT_LIST_HEAD(&gtv_MbMonitorBits.head);

        lcp_Temp = &pMsg->mcp_ReceiveBuff[7];

        for(i=0; i<lcv_ElementNum; i++) {

            /*申请链表元素内存*/
            ltp_MonitorData = (mb_monitor_element_t *)pvPortMalloc(sizeof(mb_monitor_element_t));
            configASSERT(ltp_MonitorData != NULL);

            lsv_ModbusAddr = GET_BIGPU16_DATA(lcp_Temp + i*2);

            lcv_Ret = mb_slave_convert_element_info(MB_BIT_ELEMENT, lsv_ModbusAddr, &ltp_MonitorData->mcv_ElementType, &ltp_MonitorData->msv_ElementAddr);
            if(lcv_Ret != pdPASS) {
                vPortFree(ltp_MonitorData);
                pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
                mb_slave_error_resp(pMsg);
                return;
            }

            ltp_MonitorData->msv_ModbusAddr = lsv_ModbusAddr;

            list_add_tail(&ltp_MonitorData->list, ltp_Head);
            gtv_MbMonitorBits.lsv_ListLen ++;
        }
    } else {
        /*20170804:多帧数据，待后续增加...*/
    }

    for(i=0; i<4; i++) {
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];
    }
    pMsg->msv_RespLen = 4;
    mb_slave_verify_resp_msg(pMsg);

}

/**
  * @brief  下载监控字元件
  * @param  None
  * @retval None
  */
void mb_slave_download_monitor_words(md_slave_msg_pack *pMsg)
{
    struct list_head *ltp_Head;
    struct list_head *ltp_ForNext;
    struct list_head *ltp_ForCur;
    unsigned char lcv_FrameNum;
    unsigned char lcv_ElementNum;
    mb_monitor_element_t *ltp_MonitorData;
    unsigned short i;
    unsigned char *lcp_Temp;
    unsigned short lsv_ModbusAddr;
    unsigned char lcv_Ret;

    lcv_FrameNum = pMsg->mcp_ReceiveBuff[3];

    ltp_Head = &gtv_MbMonitorWords.head;

    if(lcv_FrameNum == 0x01 || lcv_FrameNum == 0xFE) {
        /*清除当前链表*/
        list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
            ltp_MonitorData = list_entry(ltp_ForCur, mb_monitor_element_t, list);
            list_del(ltp_ForCur);
            vPortFree(ltp_MonitorData);
        }

        /*需要监控元件数量*/
        lcv_ElementNum = pMsg->mcp_ReceiveBuff[6];

        gtv_MbMonitorWords.lsv_ListLen = 0;
        INIT_LIST_HEAD(&gtv_MbMonitorWords.head);

        lcp_Temp = &pMsg->mcp_ReceiveBuff[7];

        for(i=0; i<lcv_ElementNum; i++) {

            /*申请链表元素内存*/
            ltp_MonitorData = (mb_monitor_element_t *)pvPortMalloc(sizeof(mb_monitor_element_t));
            configASSERT(ltp_MonitorData != NULL);

            lsv_ModbusAddr = GET_BIGPU16_DATA(lcp_Temp + i*2);

            lcv_Ret = mb_slave_convert_element_info(MB_WORD_ELEMENT, lsv_ModbusAddr, &ltp_MonitorData->mcv_ElementType, &ltp_MonitorData->msv_ElementAddr);
            if(lcv_Ret != pdPASS) {
                vPortFree(ltp_MonitorData);
                pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
                mb_slave_error_resp(pMsg);
                return;
            }

            ltp_MonitorData->msv_ModbusAddr = lsv_ModbusAddr;

            list_add_tail(&ltp_MonitorData->list, ltp_Head);
            gtv_MbMonitorWords.lsv_ListLen ++;
        }
    } else {
        /*20170804:多帧数据，待后续增加...*/
    }

    for(i=0; i<4; i++) {
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];
    }
    pMsg->msv_RespLen = 4;
    mb_slave_verify_resp_msg(pMsg);

}

/**
  * @brief  下载监控字位元件
  * @param  None
  * @retval None
  */
void mb_slave_download_monitor_bits_words(md_slave_msg_pack *pMsg)
{
    struct list_head *ltp_Head;
    struct list_head *ltp_ForNext;
    struct list_head *ltp_ForCur;
    unsigned char lcv_FrameNum;
    unsigned char lcv_BitsNum;
    unsigned char lcv_WordsNum;
    mb_monitor_element_t *ltp_MonitorData;
    unsigned short i;
    unsigned char *lcp_Temp;
    unsigned short lsv_ModbusAddr;
    unsigned char lcv_Ret;

    lcv_FrameNum = pMsg->mcp_ReceiveBuff[3];

    ltp_Head = &gtv_MbMonitorBitsWords.head;

    if(lcv_FrameNum == 0x01 || lcv_FrameNum == 0xFE) {
        /*清除当前链表*/
        list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
            ltp_MonitorData = list_entry(ltp_ForCur, mb_monitor_element_t, list);
            list_del(ltp_ForCur);
            vPortFree(ltp_MonitorData);
        }

        /*需要监控元件数量*/
        lcv_BitsNum = pMsg->mcp_ReceiveBuff[4];
        lcv_WordsNum = pMsg->mcp_ReceiveBuff[5];

        gtv_MbMonitorBitsWords.lsv_ListLen = 0;
        INIT_LIST_HEAD(&gtv_MbMonitorBitsWords.head);

        lcp_Temp = &pMsg->mcp_ReceiveBuff[8];

        for(i=0; i<lcv_BitsNum; i++) {

            /*申请链表元素内存*/
            ltp_MonitorData = (mb_monitor_element_t *)pvPortMalloc(sizeof(mb_monitor_element_t));
            configASSERT(ltp_MonitorData != NULL);

            lsv_ModbusAddr = GET_BIGPU16_DATA(lcp_Temp);

            lcv_Ret = mb_slave_convert_element_info(MB_BIT_ELEMENT, lsv_ModbusAddr, &ltp_MonitorData->mcv_ElementType, &ltp_MonitorData->msv_ElementAddr);
            if(lcv_Ret != pdPASS) {
                vPortFree(ltp_MonitorData);
                pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
                mb_slave_error_resp(pMsg);
                return;
            }

            ltp_MonitorData->msv_ModbusAddr = lsv_ModbusAddr;

            list_add_tail(&ltp_MonitorData->list, ltp_Head);
            gtv_MbMonitorBitsWords.lsv_ListLen ++;

            lcp_Temp += 2;
        }

        for(i=0; i<lcv_WordsNum; i++) {
            /*申请链表元素内存*/
            ltp_MonitorData = (mb_monitor_element_t *)pvPortMalloc(sizeof(mb_monitor_element_t));
            configASSERT(ltp_MonitorData != NULL);

            lsv_ModbusAddr = GET_BIGPU16_DATA(lcp_Temp);

            lcv_Ret = mb_slave_convert_element_info(MB_WORD_ELEMENT, lsv_ModbusAddr, &ltp_MonitorData->mcv_ElementType, &ltp_MonitorData->msv_ElementAddr);
            if(lcv_Ret != pdPASS) {
                vPortFree(ltp_MonitorData);
                pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
                mb_slave_error_resp(pMsg);
                return;
            }

            ltp_MonitorData->msv_ModbusAddr = lsv_ModbusAddr;

            list_add_tail(&ltp_MonitorData->list, ltp_Head);
            gtv_MbMonitorBitsWords.lsv_ListLen ++;

            lcp_Temp += 2;
        }
    } else {
        /*20170804:多帧数据，待后续增加...*/
    }

    for(i=0; i<4; i++) {
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];
    }
    pMsg->msv_RespLen = 4;
    mb_slave_verify_resp_msg(pMsg);

}


/**
  * @brief  下载文件结构体内存分配
  * @param  None
  * @retval None
  */
void mb_slave_init_file_info(mb_file_trans_st *gvt_pFile)
{
    if(gvt_pFile != NULL) {
        gvt_pFile->mcv_FrameCnt = 0;
        gvt_pFile->mcv_PreFrame = 0;
        gvt_pFile->mcv_Flag = 0;
        gvt_pFile->mlv_FileLen = 0;
    }
}

/**
  * @brief  带SDRAM项目文件下载函数
  * @param  None
  * @retval None
  */
unsigned char mb_slave_download_file(md_slave_msg_pack *pMsg)
{
    mb_file_trans_st * ltv_pFile;
    unsigned char lcv_FrameNum;
    unsigned short lsv_Cnt;
    unsigned char * lcp_SrcBuff;
    unsigned char * lcp_DestBuff;
    unsigned short i;
    unsigned char temp;
    unsigned char lcv_Ret;
    unsigned long ulongFileLength;

    switch(pMsg->mcp_ReceiveBuff[2]) {
        /*下载UCODE*/
        case MB_DOWNLOAD_UCODE:
            hexdump(gtv_ModbusFileTrans, sizeof(gtv_ModbusFileTrans));
            /*之前有接收完成，未固化文件。在线模式出现*/
            if((gtv_ModbusFileTrans[MB_DL_UCODE] != NULL) && (gtv_ModbusFileTrans[MB_DL_UCODE]->mcv_Flag & 0x2))
            {
                PRINTF("mcv_Flag = 0x%x\r\n", gtv_ModbusFileTrans[MB_DL_UCODE]->mcv_Flag);
                LOGD(TAG, "MB_DOWNLOAD_UCODE: ...........000, mcp_FileHandler = 0x%08X", gtv_ModbusFileTrans[MB_DL_UCODE]->mcp_FileHandler);
                /*释放动态内存*/
                vPortFree(gtv_ModbusFileTrans[MB_DL_UCODE]->mcp_FileHandler);
                gtv_ModbusFileTrans[MB_DL_UCODE]->mcp_FileHandler = NULL;
                vPortFree(gtv_ModbusFileTrans[MB_DL_UCODE]);
                gtv_ModbusFileTrans[MB_DL_UCODE] = NULL;
            }
            LOGD(TAG, "%s: ...........001", __func__);
            if(NULL == gtv_ModbusFileTrans[MB_DL_UCODE])
            {
                LOGI(TAG, "%s: ...........002", __func__);
                /*在线模式开启时，接收阶段暂停PLC运行*/
                if(gtv_PlcRunStatus.mcv_IsOnlineProgram)
                {
                    LOGD(TAG, "%s: online mode  need stop plc..003", __func__);
                    mb_slave_ctrl_stop_cmd(pMsg);
                }
                LOGD(TAG, "%s: ...........004", __func__);
                gtv_ModbusFileTrans[MB_DL_UCODE] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
                configASSERT(gtv_ModbusFileTrans[MB_DL_UCODE] != NULL);

                gtv_ModbusFileTrans[MB_DL_UCODE]->mcp_FileHandler = (unsigned char *)pvPortMalloc(UCODE_FILE_MAX_SIZE);
                configASSERT(gtv_ModbusFileTrans[MB_DL_UCODE]->mcp_FileHandler != NULL);

                mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_UCODE]);
            }

            ltv_pFile = gtv_ModbusFileTrans[MB_DL_UCODE];
            break;

        /*下载系统块*/
        case MB_DOWNLOAD_SYS_BLOCK:
            if(NULL == gtv_ModbusFileTrans[MB_DL_SYS_BLOCK])
            {
                PRINTF("mcv_Flag = 0x%x\r\n", gtv_ModbusFileTrans[MB_DL_SYS_BLOCK]->mcv_Flag);
                LOGD(TAG, "MB_DOWNLOAD_SYS_BLOCK, mcp_FileHandler = 0x%08X", gtv_ModbusFileTrans[MB_DL_SYS_BLOCK]->mcp_FileHandler);
                gtv_ModbusFileTrans[MB_DL_SYS_BLOCK] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
                configASSERT(gtv_ModbusFileTrans[MB_DL_SYS_BLOCK] != NULL);

                gtv_ModbusFileTrans[MB_DL_SYS_BLOCK]->mcp_FileHandler = (unsigned char *)pvPortMalloc(SYS_BLOCK_MAX_SIZE);
                configASSERT(gtv_ModbusFileTrans[MB_DL_SYS_BLOCK]->mcp_FileHandler != NULL);

                mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_SYS_BLOCK]);
            }

            ltv_pFile = gtv_ModbusFileTrans[MB_DL_SYS_BLOCK];
            break;

        /*下载数据块*/
        case MB_DOWNLOAD_DATA_BLOCK:
            
            if((NULL != gtv_ModbusFileTrans[MB_DL_DATA_BLOCK]) && (gtv_ModbusFileTrans[MB_DL_DATA_BLOCK]->mcv_Flag & 0x2))
            {
                LOGI(TAG, "MB_DOWNLOAD_DATA_BLOCK, mcp_FileHandler = 0x%08X", gtv_ModbusFileTrans[MB_DL_DATA_BLOCK]->mcp_FileHandler);
                PRINTF("mcv_Flag = 0x%x\r\n", gtv_ModbusFileTrans[MB_DL_DATA_BLOCK]->mcv_Flag);            
                /*释放动态内存*/
                vPortFree(gtv_ModbusFileTrans[MB_DL_DATA_BLOCK]->mcp_FileHandler);
                gtv_ModbusFileTrans[MB_DL_DATA_BLOCK]->mcp_FileHandler = NULL;
                vPortFree(gtv_ModbusFileTrans[MB_DL_DATA_BLOCK]);
                gtv_ModbusFileTrans[MB_DL_DATA_BLOCK] = NULL;
            }
            if(NULL == gtv_ModbusFileTrans[MB_DL_DATA_BLOCK])
            {
                gtv_ModbusFileTrans[MB_DL_DATA_BLOCK] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
                configASSERT(gtv_ModbusFileTrans[MB_DL_DATA_BLOCK] != NULL);
                LOGD(TAG, "MB_DOWNLOAD_DATA_BLOCK;Free heap = %u(bytes)", xPortGetFreeHeapSize());
                gtv_ModbusFileTrans[MB_DL_DATA_BLOCK]->mcp_FileHandler = (unsigned char *)pvPortMalloc(DATA_BLOCK_MAX_SIZE);
                configASSERT(gtv_ModbusFileTrans[MB_DL_DATA_BLOCK]->mcp_FileHandler != NULL);

                mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_DATA_BLOCK]);
            }
            ltv_pFile = gtv_ModbusFileTrans[MB_DL_DATA_BLOCK];
            break;

        /*下载POU INFO*/
        case MB_DOWNLOAD_POU_INFO:
            /*之前有接收完成，未固化文件。在线模式出现*/
            if((NULL != gtv_ModbusFileTrans[MB_DL_POU_INFO]) && (gtv_ModbusFileTrans[MB_DL_POU_INFO]->mcv_Flag & 0x2))
            {
                PRINTF("mcv_Flag = 0x%x\r\n", gtv_ModbusFileTrans[MB_DL_POU_INFO]->mcv_Flag);
                LOGI(TAG, "MB_DOWNLOAD_POU_INFO, mcp_FileHandler = 0x%08X", gtv_ModbusFileTrans[MB_DL_POU_INFO]->mcp_FileHandler);
                /*释放动态内存*/
                vPortFree(gtv_ModbusFileTrans[MB_DL_POU_INFO]->mcp_FileHandler);
                gtv_ModbusFileTrans[MB_DL_POU_INFO]->mcp_FileHandler = NULL;
                vPortFree(gtv_ModbusFileTrans[MB_DL_POU_INFO]);
                gtv_ModbusFileTrans[MB_DL_POU_INFO] = NULL;
            }

            if(NULL == gtv_ModbusFileTrans[MB_DL_POU_INFO])
            {
                gtv_ModbusFileTrans[MB_DL_POU_INFO] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
                configASSERT(gtv_ModbusFileTrans[MB_DL_POU_INFO] != NULL);

                gtv_ModbusFileTrans[MB_DL_POU_INFO]->mcp_FileHandler = (unsigned char *)pvPortMalloc(POU_FILE_MAX_SIZE);
                configASSERT(gtv_ModbusFileTrans[MB_DL_POU_INFO]->mcp_FileHandler != NULL);

                mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_POU_INFO]);
            }
            ltv_pFile = gtv_ModbusFileTrans[MB_DL_POU_INFO];
            break;

        /*下载全局变量表*/
        case MB_DOWNLOAD_GVT:
            if(NULL == gtv_ModbusFileTrans[MB_DL_GVT])
            {
                PRINTF("mcv_Flag = 0x%x\r\n", gtv_ModbusFileTrans[MB_DL_GVT]->mcv_Flag);
                gtv_ModbusFileTrans[MB_DL_GVT] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
                configASSERT(gtv_ModbusFileTrans[MB_DL_GVT] != NULL);
                LOGD(TAG, "MB_DOWNLOAD_GVT;Free heap = %u(bytes)", xPortGetFreeHeapSize());
                gtv_ModbusFileTrans[MB_DL_GVT]->mcp_FileHandler = (unsigned char *)pvPortMalloc(GVT_FILE_MAX_SIZE);
                configASSERT(gtv_ModbusFileTrans[MB_DL_GVT]->mcp_FileHandler != NULL);

                mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_GVT]);
            }
            ltv_pFile = gtv_ModbusFileTrans[MB_DL_GVT];
            break;

        /*下载网络参数*/
        case MB_DOWNLOAD_NETCFG:
            if(NULL == gtv_ModbusFileTrans[MB_DL_NETCFG])
            {
                PRINTF("mcv_Flag = 0x%x\r\n", gtv_ModbusFileTrans[MB_DL_NETCFG]->mcv_Flag);
                gtv_ModbusFileTrans[MB_DL_NETCFG] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
                configASSERT(gtv_ModbusFileTrans[MB_DL_NETCFG] != NULL);
                LOGD(TAG, "MB_DOWNLOAD_NETCFG,Free heap = %u(bytes)", xPortGetFreeHeapSize());
                gtv_ModbusFileTrans[MB_DL_NETCFG]->mcp_FileHandler = (unsigned char *)pvPortMalloc(DEV_NETWORK_INFO_SIZE);
                configASSERT(gtv_ModbusFileTrans[MB_DL_NETCFG]->mcp_FileHandler != NULL);

                mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_NETCFG]);
            }
            ltv_pFile = gtv_ModbusFileTrans[MB_DL_NETCFG];
            break;

        /*下载PID参数1*/
        case MB_DOWNLOAD_PID1:
            if(NULL == gtv_ModbusFileTrans[MB_DL_PID1])
            {
                PRINTF("mcv_Flag = 0x%x\r\n", gtv_ModbusFileTrans[MB_DL_PID1]->mcv_Flag);
                gtv_ModbusFileTrans[MB_DL_PID1] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
                configASSERT(gtv_ModbusFileTrans[MB_DL_PID1] != NULL);
                LOGD(TAG, "MB_DOWNLOAD_PID1,Free heap = %u(bytes)", xPortGetFreeHeapSize());
                gtv_ModbusFileTrans[MB_DL_PID1]->mcp_FileHandler = (unsigned char *)pvPortMalloc(PID1_MAX_SIZE);
                configASSERT(gtv_ModbusFileTrans[MB_DL_PID1]->mcp_FileHandler != NULL);

                mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_PID1]);
            }
            ltv_pFile = gtv_ModbusFileTrans[MB_DL_PID1];
            break;

        /*下载PID参数2*/
        case MB_DOWNLOAD_PID2:
            if(NULL == gtv_ModbusFileTrans[MB_DL_PID2])
            {
                PRINTF("mcv_Flag = 0x%x\r\n", gtv_ModbusFileTrans[MB_DL_PID2]->mcv_Flag);
                gtv_ModbusFileTrans[MB_DL_PID2] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
                configASSERT(gtv_ModbusFileTrans[MB_DL_PID2] != NULL);
                LOGD(TAG, "MB_DOWNLOAD_PID2,Free heap = %u(bytes)", xPortGetFreeHeapSize());
                gtv_ModbusFileTrans[MB_DL_PID2]->mcp_FileHandler = (unsigned char *)pvPortMalloc(PID2_MAX_SIZE);
                configASSERT(gtv_ModbusFileTrans[MB_DL_PID2]->mcp_FileHandler != NULL);

                mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_PID2]);
            }
            ltv_pFile = gtv_ModbusFileTrans[MB_DL_PID2];
            break;
            
        /*系统升级*/
        case MB_DOWNLOAD_SYS_UPGRADE:
            if (NULL == gtv_ModbusFileTrans[MB_DL_SYS_UPGRADE])
            {
                PRINTF("mcv_Flag = 0x%x\r\n", gtv_ModbusFileTrans[MB_DL_SYS_UPGRADE]->mcv_Flag);
                gtv_ModbusFileTrans[MB_DL_SYS_UPGRADE] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
                configASSERT(gtv_ModbusFileTrans[MB_DL_SYS_UPGRADE] != NULL);
                LOGD(TAG, "MB_DOWNLOAD_SYS_UPGRADE,Free heap = %u(bytes)", xPortGetFreeHeapSize());
            #if  (0)
                gtv_ModbusFileTrans[MB_DL_SYS_UPGRADE]->mcp_FileHandler = (unsigned char *)pvPortMalloc(SYSTEM_UPGRADE_MAX_SIZE);
            #else
                uint32_t nHeapSize = xPortGetFreeHeapSize();
                gtv_ModbusFileTrans[MB_DL_SYS_UPGRADE]->mcp_FileHandler = (unsigned char *)pvPortMalloc(nHeapSize - 40*1024);
            #endif
                configASSERT(gtv_ModbusFileTrans[MB_DL_SYS_UPGRADE]->mcp_FileHandler != NULL);
                mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_SYS_UPGRADE]);
            }
            ltv_pFile = gtv_ModbusFileTrans[MB_DL_SYS_UPGRADE];

            break;

        /*PLC应用程序CBIN文件升级*/
        case MB_DOWNLOAD_PLC_PROGRAME_CBIN:
            
            if (NULL == gtv_ModbusFileTrans[MB_DL_PLC_CBIN])
            {
                PRINTF("mcv_Flag = 0x%x\r\n", gtv_ModbusFileTrans[MB_DL_PLC_CBIN]->mcv_Flag);
                hexdump(pMsg->mcp_ReceiveBuff,64); //92 1D 00 00是长度
                //01 68 6A 01 43 53 52 46 92 1D 00 00 00 00 00 00 
                //00 30 00 00 00 00 00 00 00 00 00 00 00 00 28 00 
                //00 00 48 32 12 4F A3 79 37 9E 88 12 4F A3 77 37 
                //9E 88 12 4F A3 77 37 CE B0 12 4F A3 77 37 9E 9C 

                gtv_ModbusFileTrans[MB_DL_PLC_CBIN] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
                configASSERT(gtv_ModbusFileTrans[MB_DL_PLC_CBIN] != NULL);
                //毛继科:20120416更改了CBIN的文件结构 
                //最新的文件结构：头标记（四字节CSRF）+ 文件长度（4字节）+ 版本号（4字节）+ 产品型号（2字节）+ 保留12字节
                ulongFileLength = GET_PU32_DATA(pMsg->mcp_ReceiveBuff+8);
                PRINTF("cbinFilelength = %u\r\n", ulongFileLength);
                gtv_ModbusFileTrans[MB_DL_PLC_CBIN]->mcp_FileHandler = (unsigned char *)pvPortMalloc(ulongFileLength);
                configASSERT(gtv_ModbusFileTrans[MB_DL_PLC_CBIN]->mcp_FileHandler != NULL);

                mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_PLC_CBIN]);
            }
            ltv_pFile = gtv_ModbusFileTrans[MB_DL_PLC_CBIN];
            
            break;
    }
    PRINTF("Enter %s(), [2] = 0x%x, [3] = 0x%x, ", __func__, pMsg->mcp_ReceiveBuff[2], pMsg->mcp_ReceiveBuff[3]);
    lcv_FrameNum = pMsg->mcp_ReceiveBuff[3];
    PRINTF("mcv_FrameCnt = %d, mcv_PreFrame = %d, mcv_Flag = 0x%x\r\n", ltv_pFile->mcv_FrameCnt, ltv_pFile->mcv_PreFrame, ltv_pFile->mcv_Flag);
    if ((ltv_pFile->mcv_FrameCnt % 50) == 0)
    {
         PRINTF("ltv_pFile->mcv_FrameCnt % 50 == 0,bsp_feed_watch_dog!\r\n");
         bsp_feed_watch_dog();
         LOGV(TAG, "Free heap size is %d bytes", xPortGetFreeHeapSize());
         //print_vTaskList(__func__, __LINE__);
    }
    lsv_Cnt = pMsg->msv_ReceiveLen - 6;

    if(ltv_pFile->mcv_Flag & 0x01) {
        if(lcv_FrameNum == ltv_pFile->mcv_PreFrame)
            return pdPASS;

        if(lcv_FrameNum == 0xFE || (lcv_FrameNum == (ltv_pFile->mcv_PreFrame + 1))) {
            ltv_pFile->mcv_PreFrame = lcv_FrameNum;
            ltv_pFile->mcv_FrameCnt ++;

            lcp_SrcBuff = &pMsg->mcp_ReceiveBuff[4];
            lcp_DestBuff = (unsigned char *)(ltv_pFile->mcp_FileHandler + ltv_pFile->mlv_FileLen);

            for(i=0; i<lsv_Cnt; i++) {
                *lcp_DestBuff++ = *lcp_SrcBuff++;
            }

            ltv_pFile->mlv_FileLen += lsv_Cnt;

            if(lcv_FrameNum == 0xFE)
                ltv_pFile->mcv_Flag |= 0x02;

        } else if(lcv_FrameNum == 0x01 && ltv_pFile->mcv_PreFrame == 0xFD) {
            ltv_pFile->mcv_PreFrame = lcv_FrameNum;
            ltv_pFile->mcv_FrameCnt ++;

            /*记录帧号反转次数*/
            temp = ((((ltv_pFile->mcv_Flag & 0x1A) >> 2) +1) << 2);
            ltv_pFile->mcv_Flag &=0xE3;
            ltv_pFile->mcv_Flag |= temp;

            lcp_SrcBuff = &pMsg->mcp_ReceiveBuff[4];
            lcp_DestBuff = (unsigned char *)(ltv_pFile->mcp_FileHandler + ltv_pFile->mlv_FileLen);

            for(i=0; i<lsv_Cnt; i++) {
                *lcp_DestBuff++ = *lcp_SrcBuff++;
            }

            ltv_pFile->mlv_FileLen += lsv_Cnt;

            if(lcv_FrameNum == 0xFE)
                ltv_pFile->mcv_Flag |= 0x02;

        } else {
            return pdFAIL;
        }

    } else {
        /*第一帧数据*/
        if(lcv_FrameNum == 0x01 || lcv_FrameNum == 0xFE) {
            ltv_pFile->mcv_Flag |= 0x01;
            ltv_pFile->mcv_PreFrame = lcv_FrameNum;
            ltv_pFile->mcv_FrameCnt = 1;

            lcp_SrcBuff = &pMsg->mcp_ReceiveBuff[4];
            lcp_DestBuff = (unsigned char *)(ltv_pFile->mcp_FileHandler + ltv_pFile->mlv_FileLen);

            for(i=0; i<lsv_Cnt; i++)
            {
                *lcp_DestBuff++ = *lcp_SrcBuff++;
            }
            ltv_pFile->mlv_FileLen += lsv_Cnt;

            if(lcv_FrameNum == 0xFE)
            {
                ltv_pFile->mcv_Flag |= 0x02;
            }
        }
    }

    /*在线模式处理*/
    if(gtv_PlcRunStatus.mcv_IsOnlineProgram && (ltv_pFile->mcv_Flag & 0x02))
    {
        switch(pMsg->mcp_ReceiveBuff[2])
        {
            case MB_DOWNLOAD_UCODE:
                lcv_Ret = plc_compiler_ucode(ltv_pFile->mcp_FileHandler);
                if(lcv_Ret != pdPASS)
                {
                    gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.error_status_stop = 1;
                    /*错误信息处理*/
                    plc_refresh_error_msg(ERR_COMPILER);
                    return pdFAIL;
                }
                gtv_UserFilePtrSt.UCodePtr = ltv_pFile->mcp_FileHandler;
                break;

            case MB_DOWNLOAD_POU_INFO:
                gtv_UserFilePtrSt.PouInfoPtr = ltv_pFile->mcp_FileHandler;
                mb_slave_ctrl_run_cmd(pMsg);
                break;
        }
    }

    return pdPASS;
}

/**
  * @brief  无SDRAM项目文件下载函数
  * @param  None
  * @retval None
  */
unsigned char mb_slave_download_file_without_sdram(md_slave_msg_pack *pMsg)
{
    return pdPASS;
}

/**
  * @brief  download 功能码处理函数
  * @param  None
  * @retval None
  */
void mb_slave_download_manage(md_slave_msg_pack *pMsg)
{
    unsigned char ret;
    unsigned char i;

    /*判断帧号*/
    if(pMsg->mcp_ReceiveBuff[3] == 0x00 || pMsg->mcp_ReceiveBuff[3] >0xFE) {
        pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
        mb_slave_error_resp(pMsg);
    }

    switch(pMsg->mcp_ReceiveBuff[2]) {
        /*监控位元件*/
        case MB_DOWNLOAD_MONITOR_BITS:
            if(plc_get_password_check_result(MONITOR_PASSWORD) != pdPASS) {
                pMsg->mcv_ErrorCode = MB_PASSWORD_CHECK_FAIL;
                mb_slave_error_resp(pMsg);
                break;
            }
            mb_slave_download_monitor_bits(pMsg);
            break;
        /*监控字元件*/
        case MB_DOWNLOAD_MONITOR_WORDS:
            if(plc_get_password_check_result(MONITOR_PASSWORD) != pdPASS) {
                pMsg->mcv_ErrorCode = MB_PASSWORD_CHECK_FAIL;
                mb_slave_error_resp(pMsg);
                break;
            }
            mb_slave_download_monitor_words(pMsg);
            break;
        /*监控位、字元件*/
        case MB_DOWNLOAD_MONITOR_BITS_WORDS:
            if(plc_get_password_check_result(MONITOR_PASSWORD) != pdPASS) {
                pMsg->mcv_ErrorCode = MB_PASSWORD_CHECK_FAIL;
                mb_slave_error_resp(pMsg);
                break;
            }

            mb_slave_download_monitor_bits_words(pMsg);
            break;

        /*下载UCODE*/
        case MB_DOWNLOAD_UCODE:
        /*下载系统块*/
        case MB_DOWNLOAD_SYS_BLOCK:
        /*下载数据块*/
        case MB_DOWNLOAD_DATA_BLOCK:
        /*下载POU INFO*/
        case MB_DOWNLOAD_POU_INFO:
        /*下载全局变量表*/
        case MB_DOWNLOAD_GVT:
        /*下载网络参数  */
        case MB_DOWNLOAD_NETCFG:
        /*下载PID参数1  */
        case MB_DOWNLOAD_PID1:
        /*下载PID参数2  */
        case MB_DOWNLOAD_PID2:
            if(plc_get_password_check_result(DOWNLOAD_PASSWORD) != pdPASS) {
                PRINTF("check password ERROR!\r\n");
                pMsg->mcv_ErrorCode = MB_PASSWORD_CHECK_FAIL;
                mb_slave_error_resp(pMsg);
                break;
            }

            //suspend_task_when_download_ucode();
            if(gtv_DeviceConfigTable.isSupportSdram) {
                ret = mb_slave_download_file(pMsg);
            } else {
                //ret = mb_slave_download_file_without_sdram(pMsg);
                ret = mb_slave_download_file(pMsg);
            }

            if(ret) {
                for(i=0; i<4; i++)
                    pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];

                pMsg->msv_RespLen = 4;
                mb_slave_verify_resp_msg(pMsg);

            } else {
                pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
                mb_slave_error_resp(pMsg);
            }
            break;
        /*系统升级*/
        case MB_DOWNLOAD_SYS_UPGRADE:
        // 升级从站 01 68 69 
        case MB_DOWNLOAD_SLAVE:
        /*PLC应用程序CBIN文件升级*/
        case MB_DOWNLOAD_PLC_PROGRAME_CBIN:
            if(gtv_PlcRunStatus.mcv_IsOnlineProgram) {
                pMsg->mcv_ErrorCode = MB_ERROR_SLAVE_BUSY;
                mb_slave_error_resp(pMsg);
                break;
            }

            suspend_task_when_download_ucode();
        #if (DAISY_MASTER_FEATURE == 1)
            if (gSlaveIDUpgrade != 0)
            {
                ret = pdPASS;
                daisy_LAN_send_bin(pMsg->mcp_ReceiveBuff, pMsg->msv_ReceiveLen);
                EventBits_t uxBits = xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_UPGRADE_SLAVE, pdTRUE, pdFALSE, 2000);
                if (( uxBits & KALYKE_EVENT_UPGRADE_SLAVE ) != 0)
                {
                    
                    // 01 68 69 03 EE 55 00 00 00 00 00 00 00 00
                    hexdump(gDaisyLANRecvBuffer, 16);
                    if (gDaisyLANRecvBuffer[1] == 0x68)
                    {
                        ret = pdPASS;
                    }
                    else
                    {
                        ret = pdFAIL;
                    }
                }
                else
                {
                    ret = pdFAIL;
                    resume_task_after_download_ucode();
                }
            }
            else
        #endif
            {
                if(gtv_DeviceConfigTable.isSupportSdram)
                {
                    ret = mb_slave_download_file(pMsg);
                }
                else
                {
                    //ret = mb_slave_download_file_without_sdram(pMsg);
                    ret = mb_slave_download_file(pMsg);
                }
            }

            if (ret == pdPASS)
            {
                for(i=0; i<4; i++)
                {
                    pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];
                }
                pMsg->msv_RespLen = 4;
                mb_slave_verify_resp_msg(pMsg);

            }
            else
            {
                pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
                mb_slave_error_resp(pMsg);
            }
            break;

        default:
            gtp_ModbusSlaveDiagInfo[pMsg->mcv_Sender].msv_SlaveErrCnt++;
            pMsg->mcv_ErrorCode = MB_ILIEGAL_CODE;
            mb_slave_error_resp(pMsg);
    }
}

