/**
  ******************************************************************************
  * @file    mb_upload.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   Modbus slave 上载功能
  ******************************************************************************
  */
#include "mb.h"
#include "bsp_flash.h"
#include "plc_commonfunc.h"
#include "FreeRTOS.h"
#include "plc_variable.h"
#include "mb_upload.h"
#include "mb_maptable.h"
#include "plc_element.h"
#include "plc_password.h"
#include "mb_download.h"
#include "fsl_debug_console.h"
#include "kalyke_tool.h"


/**
  * @brief  刷新被监控元件值
  * @param  None
  * @retval None
  */
static void mb_slave_refresh_monitor_bits(md_slave_msg_pack *pMsg)
{
    struct list_head *ltp_Head;
    struct list_head *ltp_ForNext;
    struct list_head *ltp_ForCur;
    mb_monitor_element_t *ltp_MonitorData;

    ltp_Head = &gtv_MbMonitorBits.head;

    list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head)
    {
        ltp_MonitorData = list_entry(ltp_ForCur, mb_monitor_element_t, list);

        switch(ltp_MonitorData->mcv_ElementType)
        {
        case MB_BIT_Y:
            ltp_MonitorData->msv_ElementValue = plc_get_bit_element_value(Y_ELEMENT, ltp_MonitorData->msv_ElementAddr);
            break;
        case MB_BIT_X:
            ltp_MonitorData->msv_ElementValue = plc_get_bit_element_value(X_ELEMENT, ltp_MonitorData->msv_ElementAddr);
            break;
        case MB_BIT_M:
            ltp_MonitorData->msv_ElementValue = plc_get_bit_element_value(M_ELEMENT, ltp_MonitorData->msv_ElementAddr);
            break;
        case MB_BIT_SM:
            ltp_MonitorData->msv_ElementValue = plc_get_bit_element_value(SM_ELEMENT, ltp_MonitorData->msv_ElementAddr);
            break;
        case MB_BIT_S:
            ltp_MonitorData->msv_ElementValue = plc_get_bit_element_value(S_ELEMENT, ltp_MonitorData->msv_ElementAddr);
            break;
        case MB_BIT_T:
            ltp_MonitorData->msv_ElementValue = plc_get_bit_element_value(T_ELEMENT, ltp_MonitorData->msv_ElementAddr);
            break;
        case MB_BIT_C:
            ltp_MonitorData->msv_ElementValue = plc_get_bit_element_value(C_ELEMENT, ltp_MonitorData->msv_ElementAddr);
            break;
        }
    }
}

/**
  * @brief  刷新被监控元件值
  * @param  None
  * @retval None
  */
static void mb_slave_refresh_monitor_words(md_slave_msg_pack *pMsg)
{
    struct list_head *ltp_Head;
    struct list_head *ltp_ForNext;
    struct list_head *ltp_ForCur;
    mb_monitor_element_t *ltp_MonitorData;
    long llv_Temp;
    unsigned short lsv_C32RealAddr;

    ltp_Head = &gtv_MbMonitorWords.head;

    list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head)
    {
        ltp_MonitorData = list_entry(ltp_ForCur, mb_monitor_element_t, list);

        switch(ltp_MonitorData->mcv_ElementType)
        {
        case MB_WORD_D:
            ltp_MonitorData->msv_ElementValue = GET_D_ELEMENT_VALUE(ltp_MonitorData->msv_ElementAddr);
            break;
        case MB_WORD_SD:
            ltp_MonitorData->msv_ElementValue = GET_SD_ELEMENT_VALUE(ltp_MonitorData->msv_ElementAddr);
            break;
        case MB_WORD_Z:
            ltp_MonitorData->msv_ElementValue = GET_Z_ELEMENT_VALUE(ltp_MonitorData->msv_ElementAddr);
            break;
        case MB_WORD_T:
            ltp_MonitorData->msv_ElementValue = GET_T_CURRENT_VALUE(ltp_MonitorData->msv_ElementAddr);
            break;
        case MB_WORD_C:
            if(ltp_MonitorData->msv_ElementAddr < C16_RANG)
            {
                ltp_MonitorData->msv_ElementValue = GET_C16_CURRENT_VALUE(ltp_MonitorData->msv_ElementAddr);
            }
            else
            {
                /*32Bit C元件转化成两个16bit元件来处理，需要地址转换，例如：C200 -> C200, C201*/
                lsv_C32RealAddr = C16_RANG + (ltp_MonitorData->msv_ElementAddr - C16_RANG) / 2;
                llv_Temp = GET_C32_CURRENT_VALUE(lsv_C32RealAddr);

                if((ltp_MonitorData->msv_ElementAddr - C16_RANG) % 2)
                {
                    ltp_MonitorData->msv_ElementValue = (unsigned short)(llv_Temp >> 16);
                }
                else
                {
                    ltp_MonitorData->msv_ElementValue = (unsigned short)(llv_Temp);
                }
            }
            break;
        case MB_WORD_R:
            ltp_MonitorData->msv_ElementValue = GET_R_ELEMENT_VALUE(ltp_MonitorData->msv_ElementAddr);
            break;
        }
    }
}

/**
  * @brief  刷新被监控元件值
  * @param  None
  * @retval None
  */
static void mb_slave_refresh_monitor_bits_words(md_slave_msg_pack *pMsg)
{
    struct list_head *ltp_Head;
    struct list_head *ltp_ForNext;
    struct list_head *ltp_ForCur;
    mb_monitor_element_t *ltp_MonitorData;
    long llv_Temp;
    unsigned short lsv_C32RealAddr;

    ltp_Head = &gtv_MbMonitorBitsWords.head;

    list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head)
    {
        ltp_MonitorData = list_entry(ltp_ForCur, mb_monitor_element_t, list);

        switch(ltp_MonitorData->mcv_ElementType)
        {
        case MB_BIT_Y:
            ltp_MonitorData->msv_ElementValue = plc_get_bit_element_value(Y_ELEMENT, ltp_MonitorData->msv_ElementAddr);
            break;
        case MB_BIT_X:
            ltp_MonitorData->msv_ElementValue = plc_get_bit_element_value(X_ELEMENT, ltp_MonitorData->msv_ElementAddr);
            break;
        case MB_BIT_M:
            ltp_MonitorData->msv_ElementValue = plc_get_bit_element_value(M_ELEMENT, ltp_MonitorData->msv_ElementAddr);
            break;
        case MB_BIT_SM:
            ltp_MonitorData->msv_ElementValue = plc_get_bit_element_value(SM_ELEMENT, ltp_MonitorData->msv_ElementAddr);
            break;
        case MB_BIT_S:
            ltp_MonitorData->msv_ElementValue = plc_get_bit_element_value(S_ELEMENT, ltp_MonitorData->msv_ElementAddr);
            break;
        case MB_BIT_T:
            ltp_MonitorData->msv_ElementValue = plc_get_bit_element_value(T_ELEMENT, ltp_MonitorData->msv_ElementAddr);
            break;
        case MB_BIT_C:
            ltp_MonitorData->msv_ElementValue = plc_get_bit_element_value(C_ELEMENT, ltp_MonitorData->msv_ElementAddr);
            break;
        case MB_WORD_D:
            ltp_MonitorData->msv_ElementValue = GET_D_ELEMENT_VALUE(ltp_MonitorData->msv_ElementAddr);
            break;
        case MB_WORD_SD:
            ltp_MonitorData->msv_ElementValue = GET_SD_ELEMENT_VALUE(ltp_MonitorData->msv_ElementAddr);
            break;
        case MB_WORD_Z:
            ltp_MonitorData->msv_ElementValue = GET_Z_ELEMENT_VALUE(ltp_MonitorData->msv_ElementAddr);
            break;
        case MB_WORD_T:
            ltp_MonitorData->msv_ElementValue = GET_T_CURRENT_VALUE(ltp_MonitorData->msv_ElementAddr);
            break;
        case MB_WORD_C:
            if(ltp_MonitorData->msv_ElementAddr < C16_RANG)
            {
                ltp_MonitorData->msv_ElementValue = GET_C16_CURRENT_VALUE(ltp_MonitorData->msv_ElementAddr);
            }
            else
            {
                /*32Bit C元件转化成两个16bit元件来处理，需要地址转换，例如：C200 -> C200, C201*/
                lsv_C32RealAddr = C16_RANG + (ltp_MonitorData->msv_ElementAddr - C16_RANG) / 2;
                llv_Temp = GET_C32_CURRENT_VALUE(lsv_C32RealAddr);

                if((ltp_MonitorData->msv_ElementAddr - C16_RANG) % 2)
                {
                    ltp_MonitorData->msv_ElementValue = (unsigned short)(llv_Temp >> 16);
                }
                else
                {
                    ltp_MonitorData->msv_ElementValue = (unsigned short)(llv_Temp);
                }
            }
            break;
        case MB_WORD_R:
            ltp_MonitorData->msv_ElementValue = GET_R_ELEMENT_VALUE(ltp_MonitorData->msv_ElementAddr);
            break;
        }
    }
}

/**
  * @brief  监控位元件
  * @param  None
  * @retval None
  */
void mb_slave_upload_monitor_bits(md_slave_msg_pack *pMsg)
{
    struct list_head *ltp_Head;
    struct list_head *ltp_ForNext;
    struct list_head *ltp_ForCur;
    unsigned char lcv_FrameNum;
    unsigned short i, j;
    unsigned short lsv_ElementCnt;

    mb_monitor_element_t *ltp_MonitorData;

    /*刷新位元件值*/
    mb_slave_refresh_monitor_bits(pMsg);

    lcv_FrameNum = pMsg->mcp_ReceiveBuff[3];

    ltp_Head = &gtv_MbMonitorBits.head;

    if(lcv_FrameNum == 0x01)
    {
        i = 0;
        lsv_ElementCnt = 0;
        /*第一帧*/
        list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head)
        {
            ltp_MonitorData = list_entry(ltp_ForCur, mb_monitor_element_t, list);

            pMsg->mcp_RespBuff[5 + i] = (unsigned char)(ltp_MonitorData->msv_ModbusAddr >> 8);
            pMsg->mcp_RespBuff[6 + i] = (unsigned char)(ltp_MonitorData->msv_ModbusAddr);
            pMsg->mcp_RespBuff[7 + i] = (unsigned char)(ltp_MonitorData->msv_ElementValue);
            i += 3;
            lsv_ElementCnt ++;

            /*单帧数据最多监控250个位元件，帧长度位 250*3+7 = 757*/
            if(lsv_ElementCnt >= 250)
            {
                break;
            }
        }

    }
    else
    {
        /*20170808：监控超过1帧数据待后续补充，当前只监控一帧数据 ...*/

    }

    for(j = 0; j < 3; j++)
    {
        pMsg->mcp_RespBuff[j] = pMsg->mcp_ReceiveBuff[j];
    }
    /*填充帧号，默认单帧数据*/
    pMsg->mcp_RespBuff[3] = 0xFE;
    /*数据长度*/
    pMsg->mcp_RespBuff[4] = lsv_ElementCnt;

    pMsg->msv_RespLen = 5 + lsv_ElementCnt * 3;
    mb_slave_verify_resp_msg(pMsg);
}

/**
  * @brief  监控字元件
  * @param  None
  * @retval None
  */
void mb_slave_upload_monitor_words(md_slave_msg_pack *pMsg)
{
    struct list_head *ltp_Head;
    struct list_head *ltp_ForNext;
    struct list_head *ltp_ForCur;
    unsigned char lcv_FrameNum;
    unsigned long i;
    unsigned short lsv_ElementCnt, lsv_FrameLen;

    mb_monitor_element_t *ltp_MonitorData;

    /*刷新字元件值*/
    mb_slave_refresh_monitor_words(pMsg);

    lcv_FrameNum = pMsg->mcp_ReceiveBuff[3];

    ltp_Head = &gtv_MbMonitorWords.head;

    if(lcv_FrameNum == 0x01)
    {
        lsv_ElementCnt = 0;
        lsv_FrameLen = 5;
        /*第一帧*/
        list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head)
        {
            ltp_MonitorData = list_entry(ltp_ForCur, mb_monitor_element_t, list);

            pMsg->mcp_RespBuff[lsv_FrameLen + 0] = (unsigned char)(ltp_MonitorData->msv_ModbusAddr >> 8);
            pMsg->mcp_RespBuff[lsv_FrameLen + 1] = (unsigned char)(ltp_MonitorData->msv_ModbusAddr);
            pMsg->mcp_RespBuff[lsv_FrameLen + 2] = (unsigned char)(ltp_MonitorData->msv_ElementValue >> 8);
            pMsg->mcp_RespBuff[lsv_FrameLen + 3] = (unsigned char)(ltp_MonitorData->msv_ElementValue);
            lsv_FrameLen += 4;
            lsv_ElementCnt ++;

            /*最大帧长度 250*4 + 5 +2 = 1007*/
            if(lsv_ElementCnt >= 250)
            {
                break;
            }
        }

    }
    else
    {
        /*20170808：监控超过1帧数据待后续补充，当前只监控一帧数据 ...*/

    }

    for(i = 0; i < 3; i++)
    {
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];
    }
    /*填充帧号*/
    pMsg->mcp_RespBuff[3] = 0xFE;
    /*元件个数*/
    pMsg->mcp_RespBuff[4] = lsv_ElementCnt;

    pMsg->msv_RespLen = lsv_FrameLen;
    mb_slave_verify_resp_msg(pMsg);
}

/**
  * @brief  监控字位元件
  * @param  None
  * @retval None
  */
void mb_slave_upload_monitor_bits_words(md_slave_msg_pack *pMsg)
{
    struct list_head *ltp_Head;
    struct list_head *ltp_ForNext;
    struct list_head *ltp_ForCur;
    unsigned char lcv_FrameNum;
    unsigned long i;
    unsigned short lsv_FrameLen;
    unsigned char lcv_BitsCnt, lcv_WordsCnt;


    mb_monitor_element_t *ltp_MonitorData;

    /*刷新字、位元件值*/
    mb_slave_refresh_monitor_bits_words(pMsg);

    lcv_FrameNum = pMsg->mcp_ReceiveBuff[3];

    ltp_Head = &gtv_MbMonitorBitsWords.head;

    if(lcv_FrameNum == 0x01)
    {
        lcv_BitsCnt = 0;
        lcv_WordsCnt = 0;

        lsv_FrameLen = 6;
        /*第一帧*/
        list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head)
        {
            ltp_MonitorData = list_entry(ltp_ForCur, mb_monitor_element_t, list);

            if(ltp_MonitorData->mcv_ElementType <= MB_BIT_C)
            {
                pMsg->mcp_RespBuff[lsv_FrameLen + 0] = (unsigned char)(ltp_MonitorData->msv_ModbusAddr >> 8);
                pMsg->mcp_RespBuff[lsv_FrameLen + 1] = (unsigned char)(ltp_MonitorData->msv_ModbusAddr);
                pMsg->mcp_RespBuff[lsv_FrameLen + 2] = (unsigned char)(ltp_MonitorData->msv_ElementValue);
                lsv_FrameLen += 3;
                lcv_BitsCnt ++;
            }
            else
            {
                pMsg->mcp_RespBuff[lsv_FrameLen + 0] = (unsigned char)(ltp_MonitorData->msv_ModbusAddr >> 8);
                pMsg->mcp_RespBuff[lsv_FrameLen + 1] = (unsigned char)(ltp_MonitorData->msv_ModbusAddr);
                pMsg->mcp_RespBuff[lsv_FrameLen + 2] = (unsigned char)(ltp_MonitorData->msv_ElementValue >> 8);
                pMsg->mcp_RespBuff[lsv_FrameLen + 3] = (unsigned char)(ltp_MonitorData->msv_ElementValue);
                lsv_FrameLen += 4;
                lcv_WordsCnt ++;
            }

            /*最大帧长度 250*4 + 5 +2 = 1007*/
            if(lsv_FrameLen >= 1000)
            {
                break;
            }
        }

    }
    else
    {
        /*20170808：监控超过1帧数据待后续补充，当前只监控一帧数据 ...*/

    }

    for(i = 0; i < 3; i++)
    {
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];
    }
    /*填充帧号*/
    pMsg->mcp_RespBuff[3] = 0xFE;
    /*位元件个数*/
    pMsg->mcp_RespBuff[4] = lcv_BitsCnt;
    /*字元件个数*/
    pMsg->mcp_RespBuff[5] = lcv_WordsCnt;

    pMsg->msv_RespLen = lsv_FrameLen;
    mb_slave_verify_resp_msg(pMsg);
}


/**
  * @brief  上载强制位元件
  * @param  None
  * @retval None
  */
void mb_slave_upload_force_bits(md_slave_msg_pack *pMsg)
{
    unsigned short i;
    struct list_head *ltp_Head;
    struct list_head *ltp_ForNext;
    struct list_head *ltp_ForCur;
    unsigned char lcv_ElementCnt;
    unsigned short lsv_FrameLen;
    mb_force_element_t *ltp_ForceData;

    lsv_FrameLen = 5;
    lcv_ElementCnt = 0;

    ltp_Head = &gtv_ForceBits.head;
    i = 0;

    if(gtv_ForceBits.lsv_ListLen > 0)
    {
        list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head)
        {
            ltp_ForceData = list_entry(ltp_ForCur, mb_force_element_t, list);
            pMsg->mcp_RespBuff[5 + i] = (unsigned char)(ltp_ForceData->msv_ModbusAddr >> 8);
            pMsg->mcp_RespBuff[6 + i] = (unsigned char)(ltp_ForceData->msv_ModbusAddr);
            pMsg->mcp_RespBuff[7 + i] = (unsigned char)(ltp_ForceData->msv_ElementValue);
            i += 3;
            lcv_ElementCnt ++;

            if(lcv_ElementCnt >= 250)
            {
                break;
            }
        }
    }

    lsv_FrameLen += i;

    for(i = 0; i < 3; i++)
    {
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];
    }

    pMsg->mcp_RespBuff[3] = 0xFE;
    pMsg->mcp_RespBuff[4] = lcv_ElementCnt;

    pMsg->msv_RespLen = lsv_FrameLen;
    mb_slave_verify_resp_msg(pMsg);
}

/**
  * @brief  上载强制字元件
  * @param  None
  * @retval None
  */
void mb_slave_upload_force_words(md_slave_msg_pack *pMsg)
{
    unsigned short i;
    struct list_head *ltp_Head;
    struct list_head *ltp_ForNext;
    struct list_head *ltp_ForCur;
    unsigned char lcv_ElementCnt;
    unsigned short lsv_FrameLen;
    mb_force_element_t *ltp_ForceData;

    lsv_FrameLen = 5;
    lcv_ElementCnt = 0;

    ltp_Head = &gtv_ForceWords.head;
    i = 0;

    if(gtv_ForceWords.lsv_ListLen > 0)
    {
        list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head)
        {
            ltp_ForceData = list_entry(ltp_ForCur, mb_force_element_t, list);
            pMsg->mcp_RespBuff[5 + i] = (unsigned char)(ltp_ForceData->msv_ModbusAddr >> 8);
            pMsg->mcp_RespBuff[6 + i] = (unsigned char)(ltp_ForceData->msv_ModbusAddr);
            pMsg->mcp_RespBuff[7 + i] = (unsigned char)(ltp_ForceData->msv_ElementValue >> 8);
            pMsg->mcp_RespBuff[8 + i] = (unsigned char)(ltp_ForceData->msv_ElementValue);
            i += 4;
            lcv_ElementCnt ++;

            if(lcv_ElementCnt >= 250)
            {
                break;
            }
        }
    }

    lsv_FrameLen += i;

    for(i = 0; i < 3; i++)
    {
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];
    }

    pMsg->mcp_RespBuff[3] = 0xFE;
    pMsg->mcp_RespBuff[4] = lcv_ElementCnt;

    pMsg->msv_RespLen = lsv_FrameLen;
    mb_slave_verify_resp_msg(pMsg);
}

/**
  * @brief  从内存生成数据块
  * @param  None
  * @retval None
  */
void mb_slave_upload_gen_data_block_from_ram(md_slave_msg_pack *pMsg)
{
    unsigned char lcv_FrameNum;
    static unsigned short lsv_StartElement, lsv_ReadCnt;
    unsigned short i;
    unsigned short lsv_PreSendCnt, lsv_RemainCnt;

    lcv_FrameNum = pMsg->mcp_ReceiveBuff[3];

    lsv_ReadCnt = GET_BIGPU16_DATA(&pMsg->mcp_ReceiveBuff[4]);
    lsv_StartElement = GET_BIGPU16_DATA(&pMsg->mcp_ReceiveBuff[6]);

    if(lcv_FrameNum == 1)
    {
        /*第一帧*/
        if(lsv_StartElement + lsv_ReadCnt > D_RANG)
        {
            pMsg->mcv_ErrorCode = MB_ILIEGAL_ADDR;
            mb_slave_error_resp(pMsg);
            return;
        }

        if(lsv_ReadCnt < MB_MAX_R_WORD_NUM)
        {
            /*单帧可以读完*/
            for(i = 0; i < 3; i++)
                pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];

            pMsg->mcp_RespBuff[3] = 0xFE;
            pMsg->mcp_RespBuff[4] = lsv_ReadCnt >> 8;
            pMsg->mcp_RespBuff[5] = lsv_ReadCnt;


            for(i = 0; i < lsv_ReadCnt; i++)
            {
                pMsg->mcp_RespBuff[6 + 2 * i] = (unsigned char)(GET_D_ELEMENT_VALUE(lsv_StartElement + i) >> 8);
                pMsg->mcp_RespBuff[7 + 2 * i] = (unsigned char)(GET_D_ELEMENT_VALUE(lsv_StartElement + i));
            }

            pMsg->msv_RespLen = 6 + lsv_ReadCnt * 2;
        }
        else
        {
            /*发送第一帧数据*/
            for(i = 0; i < 3; i++)
                pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];

            pMsg->mcp_RespBuff[3] = 0x01;
            pMsg->mcp_RespBuff[4] = 0;
            pMsg->mcp_RespBuff[5] = MB_MAX_R_WORD_NUM;


            for(i = 0; i < MB_MAX_R_WORD_NUM; i++)
            {
                pMsg->mcp_RespBuff[6 + 2 * i] = (unsigned char)(GET_D_ELEMENT_VALUE(lsv_StartElement + i) >> 8);
                pMsg->mcp_RespBuff[7 + 2 * i] = (unsigned char)(GET_D_ELEMENT_VALUE(lsv_StartElement + i));
            }

            pMsg->msv_RespLen = 6 + MB_MAX_R_WORD_NUM * 2;
        }
    }
    else
    {
        /*计算已发送数据*/
        lsv_PreSendCnt = (lcv_FrameNum - 1) * MB_MAX_W_WORD_NUM;

        lsv_RemainCnt = lsv_ReadCnt - lsv_PreSendCnt;

        /*重新计算开始位置*/
        lsv_StartElement += lsv_PreSendCnt - 1;

        if(lsv_RemainCnt < MB_MAX_W_WORD_NUM)
        {
            /*最后一帧数据*/
            for(i = 0; i < 3; i++)
                pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];

            pMsg->mcp_RespBuff[3] = 0xFE;
            pMsg->mcp_RespBuff[4] = lsv_RemainCnt >> 8;
            pMsg->mcp_RespBuff[5] = lsv_RemainCnt;


            for(i = 0; i < lsv_RemainCnt; i++)
            {
                pMsg->mcp_RespBuff[6 + 2 * i] = (unsigned char)(GET_D_ELEMENT_VALUE(lsv_StartElement + i) >> 8);
                pMsg->mcp_RespBuff[7 + 2 * i] = (unsigned char)(GET_D_ELEMENT_VALUE(lsv_StartElement + i));
            }

            pMsg->msv_RespLen = 6 + lsv_RemainCnt * 2;

        }
        else
        {
            /*最后一帧数据*/
            for(i = 0; i < 4; i++)
                pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];

            pMsg->mcp_RespBuff[4] = 0;
            pMsg->mcp_RespBuff[5] = MB_MAX_R_WORD_NUM;


            for(i = 0; i < MB_MAX_R_WORD_NUM; i++)
            {
                pMsg->mcp_RespBuff[6 + 2 * i] = (unsigned char)(GET_D_ELEMENT_VALUE(lsv_StartElement + i) >> 8);
                pMsg->mcp_RespBuff[7 + 2 * i] = (unsigned char)(GET_D_ELEMENT_VALUE(lsv_StartElement + i));
            }

            pMsg->msv_RespLen = 6 + MB_MAX_R_WORD_NUM * 2;
        }
    }

    mb_slave_verify_resp_msg(pMsg);
}

/**
  * @brief  释放文件句柄
  * @param  None
  * @retval None
  */
void mb_slave_free_upload_file_handle(unsigned char mcv_Type)
{
    LOGV("mb_upload", "Enter %s(), mcv_Type = 0x%x", __func__, mcv_Type);
    switch(mcv_Type)
    {
    case MB_UPLOAD_UCODE:
        if(gtv_ModbusFileTrans[MB_DL_UCODE])
        {
            vPortFree(gtv_ModbusFileTrans[MB_DL_UCODE]);
            gtv_ModbusFileTrans[MB_DL_UCODE] = NULL;
        }
        //resume_task_after_download_ucode();
        break;

    case MB_UPLOAD_SYS_BLOCK:
        if(gtv_ModbusFileTrans[MB_DL_SYS_BLOCK])
        {
            vPortFree(gtv_ModbusFileTrans[MB_DL_SYS_BLOCK]);
            gtv_ModbusFileTrans[MB_DL_SYS_BLOCK] = NULL;
        }
        //resume_task_after_download_ucode();
        break;
    case MB_UPLOAD_POU_INFO:
        if(gtv_ModbusFileTrans[MB_DL_POU_INFO])
        {
            vPortFree(gtv_ModbusFileTrans[MB_DL_POU_INFO]);
            gtv_ModbusFileTrans[MB_DL_POU_INFO] = NULL;
        }
        break;
    case MB_UPLOAD_GVT:
        if(gtv_ModbusFileTrans[MB_DL_GVT])
        {
            vPortFree(gtv_ModbusFileTrans[MB_DL_GVT]);
            gtv_ModbusFileTrans[MB_DL_GVT] = NULL;
        }
        //resume_task_after_download_ucode();
        break;
    case MB_UPLOAD_DATA_BLOCK:
        if(gtv_ModbusFileTrans[MB_DL_DATA_BLOCK])
        {
            vPortFree(gtv_ModbusFileTrans[MB_DL_DATA_BLOCK]);
            gtv_ModbusFileTrans[MB_DL_DATA_BLOCK] = NULL;
        }
        //resume_task_after_download_ucode();
        break;
    case MB_UPLOAD_NETCFG:
        if(gtv_ModbusFileTrans[MB_DL_NETCFG])
        {
            vPortFree(gtv_ModbusFileTrans[MB_DL_NETCFG]);
            gtv_ModbusFileTrans[MB_DL_NETCFG] = NULL;
        }
        //resume_task_after_download_ucode();
        break;
    case MB_UPLOAD_PID1:
        if(gtv_ModbusFileTrans[MB_DL_PID1])
        {
            vPortFree(gtv_ModbusFileTrans[MB_DL_PID1]);
            gtv_ModbusFileTrans[MB_DL_PID1] = NULL;
        }
        //resume_task_after_download_ucode();
        break;
    case MB_UPLOAD_PID2:
        if(gtv_ModbusFileTrans[MB_DL_PID2])
        {
            vPortFree(gtv_ModbusFileTrans[MB_DL_PID2]);
            gtv_ModbusFileTrans[MB_DL_PID2] = NULL;
        }
        //resume_task_after_download_ucode();
        break;

    }
}

/**
  * @brief  上载文件
  * @param  None
  * @retval None
  */
static void mb_slave_upload_file(md_slave_msg_pack *pMsg)
{
    mb_file_trans_st *ltv_pFile;
    unsigned char lcv_FrameNum;
    unsigned char *lcp_SrcBuff;
    unsigned char *lcp_DestBuff;
    unsigned short i;
    unsigned char temp;
    unsigned short lsv_Cnt;

    switch(pMsg->mcp_ReceiveBuff[2])
    {
    /*上载UCODE*/
    case MB_UPLOAD_UCODE:
        if(!gtv_ModbusFileTrans[MB_DL_UCODE])
        {
            gtv_ModbusFileTrans[MB_DL_UCODE] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
            configASSERT(gtv_ModbusFileTrans[MB_DL_UCODE] != NULL);

            flash_part_info_t *ltp_UCodeInfo = bsp_get_flash_info(UCODE_FILE_START_PAGE);
            gtv_UserFilePtrSt.UCodePtr = (unsigned char *)ltp_UCodeInfo->startAddr;
            gtv_ModbusFileTrans[MB_DL_UCODE]->mcp_FileHandler = gtv_UserFilePtrSt.UCodePtr;
            mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_UCODE]);
            gtv_ModbusFileTrans[MB_DL_UCODE]->mlv_FileLen = plc_get_file_length(gtv_UserFilePtrSt.UCodePtr + FILE_LEN_INFO_START_INDEX, 4);
            PRINTF("gtv_UserFilePtrSt.UCodePtr = 0x%x, mlv_FileLen = %d\r\n", gtv_UserFilePtrSt.UCodePtr, gtv_ModbusFileTrans[MB_DL_UCODE]->mlv_FileLen);
        }
        ltv_pFile = gtv_ModbusFileTrans[MB_DL_UCODE];
        break;

    /*上载系统块*/
    case MB_UPLOAD_SYS_BLOCK:
        if(!gtv_ModbusFileTrans[MB_DL_SYS_BLOCK])
        {
            gtv_ModbusFileTrans[MB_DL_SYS_BLOCK] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
            //configASSERT(gtv_ModbusFileTrans[MB_DL_SYS_BLOCK] != NULL);
            LOGW("mb_upload", "gtv_UserFilePtrSt.SysBlockPtr = 0x%08X", gtv_UserFilePtrSt.SysBlockPtr);
            gtv_ModbusFileTrans[MB_DL_SYS_BLOCK]->mcp_FileHandler = gtv_UserFilePtrSt.SysBlockPtr;
            mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_SYS_BLOCK]);
            gtv_ModbusFileTrans[MB_DL_SYS_BLOCK]->mlv_FileLen = plc_get_file_length(gtv_UserFilePtrSt.SysBlockPtr + FILE_LEN_INFO_START_INDEX, 4);
            PRINTF("SysBlockPtr = 0x%x, mlv_FileLen = %d\r\n", gtv_UserFilePtrSt.SysBlockPtr, gtv_ModbusFileTrans[MB_DL_SYS_BLOCK]->mlv_FileLen);
        }
        ltv_pFile = gtv_ModbusFileTrans[MB_DL_SYS_BLOCK];
        break;

    /*上载数据块*/
    case MB_UPLOAD_DATA_BLOCK:
        if(!gtv_ModbusFileTrans[MB_DL_DATA_BLOCK])
        {
            gtv_ModbusFileTrans[MB_DL_DATA_BLOCK] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
            configASSERT(gtv_ModbusFileTrans[MB_DL_DATA_BLOCK] != NULL);
            gtv_ModbusFileTrans[MB_DL_DATA_BLOCK]->mcp_FileHandler = gtv_UserFilePtrSt.DataBlockPtr;
            mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_DATA_BLOCK]);
            gtv_ModbusFileTrans[MB_DL_DATA_BLOCK]->mlv_FileLen = plc_get_file_length(gtv_UserFilePtrSt.DataBlockPtr + FILE_LEN_INFO_START_INDEX, 4);
            PRINTF("DataBlockPtr = 0x%x, mlv_FileLen = %d\r\n", gtv_UserFilePtrSt.DataBlockPtr, gtv_ModbusFileTrans[MB_DL_DATA_BLOCK]->mlv_FileLen);
        }
        ltv_pFile = gtv_ModbusFileTrans[MB_DL_DATA_BLOCK];
        break;

    /*上载POU INFO*/
    case MB_UPLOAD_POU_INFO:
        if(!gtv_ModbusFileTrans[MB_DL_POU_INFO])
        {
            gtv_ModbusFileTrans[MB_DL_POU_INFO] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
            configASSERT(gtv_ModbusFileTrans[MB_DL_POU_INFO] != NULL);
            gtv_ModbusFileTrans[MB_DL_POU_INFO]->mcp_FileHandler = gtv_UserFilePtrSt.PouInfoPtr;
            mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_POU_INFO]);
            gtv_ModbusFileTrans[MB_DL_POU_INFO]->mlv_FileLen = plc_get_file_length(gtv_UserFilePtrSt.PouInfoPtr + FILE_LEN_INFO_START_INDEX, 4);
            PRINTF("PouInfoPtr = 0x%x, mlv_FileLen = %d\r\n", gtv_UserFilePtrSt.PouInfoPtr, gtv_ModbusFileTrans[MB_DL_POU_INFO]->mlv_FileLen);
        }
        ltv_pFile = gtv_ModbusFileTrans[MB_DL_POU_INFO];
        break;

    /*上载全局变量表*/
    case MB_UPLOAD_GVT:
        if(!gtv_ModbusFileTrans[MB_DL_GVT])
        {
            gtv_ModbusFileTrans[MB_DL_GVT] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
            configASSERT(gtv_ModbusFileTrans[MB_DL_GVT] != NULL);
            gtv_ModbusFileTrans[MB_DL_GVT]->mcp_FileHandler = gtv_UserFilePtrSt.GvtPtr;
            mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_GVT]);
            gtv_ModbusFileTrans[MB_DL_GVT]->mlv_FileLen = plc_get_file_length(gtv_UserFilePtrSt.GvtPtr + FILE_LEN_INFO_START_INDEX, 4);
            PRINTF("GvtPtr = 0x%x, mlv_FileLen = %d\r\n", gtv_UserFilePtrSt.GvtPtr, gtv_ModbusFileTrans[MB_DL_GVT]->mlv_FileLen);
        }
        ltv_pFile = gtv_ModbusFileTrans[MB_DL_GVT];
        break;
    /*上载网络参数配置*/
    case MB_UPLOAD_NETCFG:
        if(!gtv_ModbusFileTrans[MB_DL_NETCFG])
        {
            gtv_ModbusFileTrans[MB_DL_NETCFG] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
            configASSERT(gtv_ModbusFileTrans[MB_DL_NETCFG] != NULL);
            gtv_ModbusFileTrans[MB_DL_NETCFG]->mcp_FileHandler = gtv_UserFilePtrSt.NetcfgBlockPtr;
            mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_NETCFG]);
            gtv_ModbusFileTrans[MB_DL_NETCFG]->mlv_FileLen = plc_get_file_length(gtv_UserFilePtrSt.NetcfgBlockPtr + FILE_LEN_INFO_START_INDEX, 4);
            PRINTF("NetcfgBlockPtr = 0x%x, mlv_FileLen = %d\r\n", gtv_UserFilePtrSt.NetcfgBlockPtr, gtv_ModbusFileTrans[MB_DL_NETCFG]->mlv_FileLen);
        }
        ltv_pFile = gtv_ModbusFileTrans[MB_DL_NETCFG];
        break;
    /*上载PID参数1配置*/
    case MB_UPLOAD_PID1:
        if(!gtv_ModbusFileTrans[MB_DL_PID1])
        {
            gtv_ModbusFileTrans[MB_DL_PID1] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
            configASSERT(gtv_ModbusFileTrans[MB_DL_PID1] != NULL);

            flash_part_info_t *ltp_UCodeInfo = bsp_get_flash_info(PID1_START_PAGE);
            gtv_UserFilePtrSt.PID1Ptr= (unsigned char *)ltp_UCodeInfo->startAddr;
            gtv_ModbusFileTrans[MB_DL_PID1]->mcp_FileHandler = gtv_UserFilePtrSt.PID1Ptr;
            mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_PID1]);
            gtv_ModbusFileTrans[MB_DL_PID1]->mlv_FileLen = plc_get_file_length(gtv_UserFilePtrSt.PID1Ptr + FILE_LEN_INFO_START_INDEX, 4);
            PRINTF("PID1Ptr = 0x%x, mlv_FileLen = %d\r\n", gtv_UserFilePtrSt.PID1Ptr, gtv_ModbusFileTrans[MB_DL_PID1]->mlv_FileLen);
        }
        ltv_pFile = gtv_ModbusFileTrans[MB_DL_PID1];
        break;

    /*上载PID参数2配置*/
    case MB_UPLOAD_PID2:
        if(!gtv_ModbusFileTrans[MB_DL_PID2])
        {
            gtv_ModbusFileTrans[MB_DL_PID2] = (mb_file_trans_st *)pvPortMalloc(sizeof(mb_file_trans_st));
            configASSERT(gtv_ModbusFileTrans[MB_DL_PID2] != NULL);

            flash_part_info_t *ltp_UCodeInfo = bsp_get_flash_info(PID2_START_PAGE);
            gtv_UserFilePtrSt.PID2Ptr= (unsigned char *)ltp_UCodeInfo->startAddr;
            gtv_ModbusFileTrans[MB_DL_PID2]->mcp_FileHandler = gtv_UserFilePtrSt.PID2Ptr;
            mb_slave_init_file_info(gtv_ModbusFileTrans[MB_DL_PID2]);
            gtv_ModbusFileTrans[MB_DL_PID2]->mlv_FileLen = plc_get_file_length(gtv_UserFilePtrSt.PID2Ptr + FILE_LEN_INFO_START_INDEX, 4);
            PRINTF("PID2Ptr = 0x%x, mlv_FileLen = %d\r\n", gtv_UserFilePtrSt.PID2Ptr, gtv_ModbusFileTrans[MB_DL_PID2]->mlv_FileLen);
        }
        ltv_pFile = gtv_ModbusFileTrans[MB_DL_PID2];
        break;


    }
    PRINTF("Enter %s(), [2] = 0x%x, [3] = 0x%x, ", __func__, pMsg->mcp_ReceiveBuff[2], pMsg->mcp_ReceiveBuff[3]);
    lcv_FrameNum = pMsg->mcp_ReceiveBuff[3];
    PRINTF("mcv_FrameCnt = %d, mcv_PreFrame = %d, mcv_Flag = 0x%x\r\n", ltv_pFile->mcv_FrameCnt, ltv_pFile->mcv_PreFrame, ltv_pFile->mcv_Flag);
    if(ltv_pFile->mcv_Flag & 0x01)
    {
        if((lcv_FrameNum == (ltv_pFile->mcv_PreFrame + 1)) || lcv_FrameNum == 0xFE)
        {
            lcp_DestBuff = &pMsg->mcp_RespBuff[4];
            lcp_SrcBuff = (unsigned char *)(ltv_pFile->mcp_FileHandler + ltv_pFile->mcv_FrameCnt * MAX_UPLOAD_FILE_LENGTH);

            if(ltv_pFile->mlv_FileLen - ltv_pFile->mcv_FrameCnt * MAX_UPLOAD_FILE_LENGTH > MAX_UPLOAD_FILE_LENGTH)
            {
                lsv_Cnt = MAX_UPLOAD_FILE_LENGTH;
            }
            else
            {
                lsv_Cnt = ltv_pFile->mlv_FileLen - ltv_pFile->mcv_FrameCnt * MAX_UPLOAD_FILE_LENGTH;
                pMsg->mcp_ReceiveBuff[3] = 0xFE;
            }

            for(i = 0; i < lsv_Cnt; i++)
            {
                *lcp_DestBuff++ = *lcp_SrcBuff++;
            }

            ltv_pFile->mcv_PreFrame = lcv_FrameNum;
            ltv_pFile->mcv_FrameCnt++;

            if(pMsg->mcp_ReceiveBuff[3] == 0xFE)
            {
                //ltv_pFile->mcv_Flag |= 0x02;
                mb_slave_free_upload_file_handle(pMsg->mcp_ReceiveBuff[2]);
            }
        }
        else if (lcv_FrameNum == ltv_pFile->mcv_PreFrame)
        {
            unsigned char offset = lcv_FrameNum - 1;
            lcp_DestBuff = &pMsg->mcp_RespBuff[4];
            lcp_SrcBuff = (unsigned char *)(ltv_pFile->mcp_FileHandler + offset * MAX_UPLOAD_FILE_LENGTH);

            if(ltv_pFile->mlv_FileLen - offset * MAX_UPLOAD_FILE_LENGTH > MAX_UPLOAD_FILE_LENGTH)
            {
                lsv_Cnt = MAX_UPLOAD_FILE_LENGTH;
            }
            else
            {
                lsv_Cnt = ltv_pFile->mlv_FileLen - offset * MAX_UPLOAD_FILE_LENGTH;
                pMsg->mcp_ReceiveBuff[3] = 0xFE;
            }

            for(i = 0; i < lsv_Cnt; i++)
            {
                *lcp_DestBuff++ = *lcp_SrcBuff++;
            }

            ltv_pFile->mcv_PreFrame = lcv_FrameNum;

            if(pMsg->mcp_ReceiveBuff[3] == 0xFE)
            {
                mb_slave_free_upload_file_handle(pMsg->mcp_ReceiveBuff[2]);
            }
        }
        else if(lcv_FrameNum == 0x01 && ltv_pFile->mcv_PreFrame == 0xFD)
        {

            lcp_DestBuff = &pMsg->mcp_RespBuff[4];
            lcp_SrcBuff = (unsigned char *)(ltv_pFile->mcp_FileHandler + ltv_pFile->mcv_FrameCnt * MAX_UPLOAD_FILE_LENGTH);

            if(ltv_pFile->mlv_FileLen - ltv_pFile->mcv_FrameCnt * MAX_UPLOAD_FILE_LENGTH > MAX_UPLOAD_FILE_LENGTH)
            {
                lsv_Cnt = MAX_UPLOAD_FILE_LENGTH;
            }
            else
            {
                lsv_Cnt = ltv_pFile->mlv_FileLen - ltv_pFile->mcv_FrameCnt * MAX_UPLOAD_FILE_LENGTH;
                pMsg->mcp_ReceiveBuff[3] = 0xFE;
            }

            for(i = 0; i < lsv_Cnt; i++)
            {
                *lcp_DestBuff++ = *lcp_SrcBuff++;
            }

            ltv_pFile->mcv_PreFrame = lcv_FrameNum;
            ltv_pFile->mcv_FrameCnt++;

            /*记录帧号反转次数*/
            temp = ((((ltv_pFile->mcv_Flag & 0x1A) >> 2) + 1) << 2);
            ltv_pFile->mcv_Flag &= 0xE3;
            ltv_pFile->mcv_Flag |= temp;

            if(pMsg->mcp_ReceiveBuff[3] == 0xFE)
            {
                //ltv_pFile->mcv_Flag |= 0x02;
                mb_slave_free_upload_file_handle(pMsg->mcp_ReceiveBuff[2]);
            }

        }
        else
        {
            pMsg->mcv_ErrorCode = MB_ILIEGAL_DATA;
            mb_slave_error_resp(pMsg);
            return;
        }

    }
    else
    {
        /*第一帧*/
        if(lcv_FrameNum == 0x01 || lcv_FrameNum == 0xFE)
        {
            ltv_pFile->mcv_Flag |= 0x01;
            ltv_pFile->mcv_PreFrame = lcv_FrameNum;
            ltv_pFile->mcv_FrameCnt = 1;

            lcp_DestBuff = &pMsg->mcp_RespBuff[4];
            lcp_SrcBuff = (unsigned char *)ltv_pFile->mcp_FileHandler;

            if(ltv_pFile->mlv_FileLen > MAX_UPLOAD_FILE_LENGTH)
            {
                lsv_Cnt = MAX_UPLOAD_FILE_LENGTH;
            }
            else
            {
                lsv_Cnt = ltv_pFile->mlv_FileLen;
                pMsg->mcp_ReceiveBuff[3] = 0xFE;
            }

            for(i = 0; i < lsv_Cnt; i++)
            {
                *lcp_DestBuff++ = *lcp_SrcBuff++;
            }

            if(pMsg->mcp_ReceiveBuff[3] == 0xFE)
            {
                //ltv_pFile->mcv_Flag |= 0x02;
                mb_slave_free_upload_file_handle(pMsg->mcp_ReceiveBuff[2]);
            }
        }
    }

    /*响应帧*/
    for(i = 0; i < 4; i++)
    {
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];
    }

    pMsg->msv_RespLen = lsv_Cnt + 4;
    mb_slave_verify_resp_msg(pMsg);
}

/**
  * @brief  UPLOAD 功能码处理函数
  * @param  None
  * @retval None
  */
void mb_slave_upload_manage(md_slave_msg_pack *pMsg)
{
    //PRINTF("Enter %s(), mcp_ReceiveBuff[2] = 0x%x\r\n", __func__, pMsg->mcp_ReceiveBuff[2]);
    switch(pMsg->mcp_ReceiveBuff[2])
    {
    /*监控位元件*/
    case MB_UPLOAD_MONITOR_BITS:
        if(plc_get_password_check_result(MONITOR_PASSWORD) != pdPASS)
        {
            pMsg->mcv_ErrorCode = MB_PASSWORD_CHECK_FAIL;
            mb_slave_error_resp(pMsg);
            break;
        }
        mb_slave_upload_monitor_bits(pMsg);
        break;
    /*监控字元件*/
    case MB_UPLOAD_MONITOR_WORDS:
        if(plc_get_password_check_result(MONITOR_PASSWORD) != pdPASS)
        {
            pMsg->mcv_ErrorCode = MB_PASSWORD_CHECK_FAIL;
            mb_slave_error_resp(pMsg);
            break;
        }
        mb_slave_upload_monitor_words(pMsg);
        break;
    /*强制位元件*/
    case MB_UPLOAD_FORCE_BITS:
        mb_slave_upload_force_bits(pMsg);
        break;
    /*强制字元件*/
    case MB_UPLOAD_FORCE_WORDS:
        mb_slave_upload_force_words(pMsg);
        break;
    /*监控位、字元件*/
    case MB_UPLOAD_MONITOR_BITS_WORDS:
        if(plc_get_password_check_result(MONITOR_PASSWORD) != pdPASS)
        {
            pMsg->mcv_ErrorCode = MB_PASSWORD_CHECK_FAIL;
            mb_slave_error_resp(pMsg);
            break;
        }
        mb_slave_upload_monitor_bits_words(pMsg);
        break;
    /*从内存中生成数据块*/
    case MB_UPLOAD_GEN_DATA_BLOCK:
        mb_slave_upload_gen_data_block_from_ram(pMsg);
        break;
    /*上载UCODE*/
    case MB_UPLOAD_UCODE:
    /*上载系统块*/
    case MB_UPLOAD_SYS_BLOCK:
    /*上载数据块*/
    case MB_UPLOAD_DATA_BLOCK:
    /*上载POU INFO*/
    case MB_UPLOAD_POU_INFO:
    /*上载全局变量表*/
    case MB_UPLOAD_GVT:
    /*上载网络参数配置*/
    case MB_UPLOAD_NETCFG:
    /*上载PID参数1配置*/
    case MB_UPLOAD_PID1:
    /*上载PID参数2配置*/
    case MB_UPLOAD_PID2:
        if(plc_get_upload_forbid_flag())
        {
            PRINTF("CAN NOT UPLOAD NOW!!!\r\n");
            pMsg->mcv_ErrorCode = MB_UPLOAD_FORBID;
            mb_slave_error_resp(pMsg);
            break;
        }

        if(plc_get_password_check_result(UPLOAD_PASSWORD) != pdPASS)
        {
            PRINTF("PASSWORD NOT CORRECT!");
            pMsg->mcv_ErrorCode = MB_PASSWORD_CHECK_FAIL;
            mb_slave_error_resp(pMsg);
            break;
        }

        //suspend_task_when_download_ucode();
        mb_slave_upload_file(pMsg);
        return;
    default:
        gtp_ModbusSlaveDiagInfo[pMsg->mcv_Sender].msv_SlaveErrCnt++;
        pMsg->mcv_ErrorCode = MB_ILIEGAL_CODE;
        mb_slave_error_resp(pMsg);
    }
}

