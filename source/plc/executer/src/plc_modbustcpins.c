/**
  ******************************************************************************
  * @file    plc_modbustcpins.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   MODBUS 相关指令实现
  ******************************************************************************
  */
#include "plc_modbustcpins.h"
//#include "plc_variable.h"
#include "plc_commonfunc.h"
#include "plc_element.h"
#include "plc_parseaddr.h"
#include "plc_errormsg.h"
#include "plc_instruction.h"

#include "bsp_uart.h"
#include "bsp_dct.h"
#include "plc_sysblock.h"
#include "kalyke_internet_task.h"

#include "verify_func.h"
#include "fsl_debug_console.h"
#include "kalyke_tool.h"

//uint16_t gWanOrLan = 0;//0：WAN， 1：LAN
uint16_t gWanOrLan[MAX_TCP_CONFIG_ITEM];

#if 0
static uint32_t gCounts = 0;
#define MODLINK_DEBUGF(message) do { \
                                if (gCounts++ % 2000 == 0) { \
                                    do {PRINTF message;} while(0); \
                                } \
                             } while(0)
static bool gDebug = true;
#else
static bool gDebug = false;
#define MODLINK_DEBUGF(message)
#endif
static uint8_t gTcpSendBuf[128];
#if 0
static void tcpConnCB(uint8_t connState)
{
}
#endif

#if (KALYKE_MODBUS_TCP_SHEET == 0)
/* CI_MBCCONN, 0xF19D
  9D F1 
  00 FF 01 00 
  00 FF 01 00 
  00 05 64 00 
  00 05 65 00 
  00 05 66 00 
  00 11 E8 03 
  00 00
*/
unsigned char run_ci_mbcconn_ins(plc_run_power_flow_st *ltp_RunEnv)
{
#if 0
    static uint32_t mTick = 0;
    uint32_t curTick = xTaskGetTickCount();
    if (curTick - mTick > 3000)
    {
        gDebug = true;
        mTick = curTick;
    }
    else
    {
        gDebug = false;
    }
#endif
    if (gDebug) LOGI("mbcconn", "Enter %s(), ltp_RunEnv = 0x%08X", __func__, ltp_RunEnv);
    uint16_t mS1;// 连接模式，0：断开连接，  1：进行连接
    uint16_t mS2;// 网络连接标识码 clientID
    //uint16_t mS3;// 使用哪个网口连接，0：WAN，1：LAN
    uint8_t mD1; // 1: TCP正在连接中
    uint8_t mD2; // 1: TCP连接已成功建立,  0: 尚未连接 
    //uint8_t mD3; // 1: 建立或断开连接时发生了错误
    int16_t mD3; // 错误码
    uint8_t lcv_Ret;

    //网络连接标识码
    lcv_Ret = get_word_default(ltp_RunEnv->mcp_PC + 6, &mS2);
    if(lcv_Ret != pdPASS)
    {
        if (gDebug) LOGE("mbcconn", "Leave %s()...001\r\n", __func__);
        return lcv_Ret;
    }
    if (mS2 > MAX_TCP_CONFIG_ITEM || mS2 == 0)
    {
        mD3 = MBCCONNECT_IDENTIFICATION_CODE_ERROR;
        save_word_default(ltp_RunEnv->mcp_PC + 22, (uint16_t*)&mD3);
        if (gDebug) LOGE("mbcconn", "Leave %s()...005\r\n", __func__);
        return ERR_MDTCP_EXCEEDMAXCONN;
    }
    mS2--;

    // 使用哪个网口连接，0：wan，1：LAN
    lcv_Ret = get_word_default(ltp_RunEnv->mcp_PC + 10, &gWanOrLan[mS2]);
    if(lcv_Ret != pdPASS)
    {
        if (gDebug) LOGE("mbcconn", "Leave %s()...002\r\n", __func__);
        return lcv_Ret;
    }   

    if (gWanOrLan[mS2] == 0)
    {
        if (!plc_get_bit_element_value(SM_ELEMENT, SM269))
        {
            mD3 = MBCCONNECT_NO_LINK;
            save_word_default(ltp_RunEnv->mcp_PC + 22, (uint16_t*)&mD3);
            if (gDebug) LOGE("mbcconn", "Leave %s(), because WAN not ready now...", __func__);
            return pdPASS;
        }
        else
        {
            mD3 = MBCCONNECT_NO_ERROR;
        }
    }
    else
    {
        if (!plc_get_bit_element_value(SM_ELEMENT, SM270))
        {
            mD3 = MBCCONNECT_NO_LINK;
            save_word_default(ltp_RunEnv->mcp_PC + 22, (uint16_t*)&mD3);
            if (gDebug) LOGE("mbcconn", "Leave %s(), because LAN not ready now...", __func__);
            return pdPASS;
        }
        else
        {
            mD3 = MBCCONNECT_NO_ERROR;
        }
    }
    
    //连接模式，0：断开连接，  1：进行连接
    lcv_Ret = get_word_default(ltp_RunEnv->mcp_PC + 2, &mS1);
    if(lcv_Ret != pdPASS)
    {
        if (gDebug) LOGE("mbcconn", "Leave %s()...000\r\n", __func__);
        return lcv_Ret;
    }
	
    //正在连接
    lcv_Ret = get_char_default(ltp_RunEnv->mcp_PC + 14, &mD1);
    if(lcv_Ret != pdPASS)
    {
        if (gDebug) LOGE("mbcconn", "Leave %s()...003\r\n", __func__);
        return lcv_Ret;
    }	

    //TCP连接已成功建立
    lcv_Ret = get_char_default(ltp_RunEnv->mcp_PC + 18, &mD2);
    if(lcv_Ret != pdPASS)
    {
        if (gDebug) LOGE("mbcconn", "Leave %s()...004\r\n", __func__);
        return lcv_Ret;
    }

    
    if (gDebug) LOGV("mbcconn", "mS1 = %u, mS2 = %u, gWanOrLan = %u, mD1 = %u, mD2 = %u, mD3 = %u,", mS1, mS2, gWanOrLan, mD1, mD2, mD3);

    if(!GET_POWER_FLOW(ltp_RunEnv)) // Power flow not effective, just return.
    {
        if (gDebug) LOGE("mbcconn", "Power flow not effective, just return.");
        return pdPASS;
    }
    else
    {
        if (gDebug) LOGI("mbcconn", "Power flow effective!!!");
        save_word_default(ltp_RunEnv->mcp_PC + 22, (uint16_t*)&mD3);
    }

    if (mS1 == 0) // 断开连接
    {
        if (!tcp_client_get_connected_bit(mS2))//如果已是断开状态，直接返回
        {
            if (gDebug) LOGE("mbcconn", "Leave %s()...006\r\n", __func__);
            return pdPASS;
        }

        if (tcp_client_get_connectting_bit(mS2)) // 如果正在断开，直接返回
        {
            if (gDebug) LOGE("mbcconn", "Leave %s()...007\r\n", __func__);
            return pdPASS;
        }
        
        stop_tcp_client(mS2);
    }
    else //进行连接
    {
        if (tcp_client_get_connected_bit(mS2)) // Already connected, just return.
        {
            if (gDebug) LOGE("mbcconn", "Leave %s()...008\r\n", __func__);
            return pdPASS;
        }
        if (tcp_client_get_connectting_bit(mS2)) // Connecting , just return.
        {
            if (gDebug) LOGE("mbcconn", "Leave %s()...009\r\n", __func__);
            return pdPASS;
        }
        start_tcp_client(mS2, ltp_RunEnv->mcp_PC);
    }
    return pdPASS;
}

/* 网络连接标识码 */
static uint16_t mbclink_get_S1(uint8_t *pc)
{
    uint16_t clientID;
    get_word(pc + 2, &clientID, 0, 1);
    return clientID;
}

/* 便捷指令编号，从0开始 */
static uint16_t mbclink_get_S2(uint8_t *pc)
{
    uint16_t val;
    get_word(pc + 6, &val, 0, 1);
    return val;
}

#if 0
/* 执行模式 */
static uint16_t mbclink_get_S3(uint8_t *pc)
{
    uint16_t val;
    get_word(pc + 10, &val, 0, 1);
    return val;
}
#endif

/* 停止运行控制位 */
static uint8_t mbclink_get_S4(uint8_t *pc)
{
    uint8_t val;
    get_char(pc + 14, &val, 0, 1);
    return val;
}
#if 0
static void mbclink_save_S4(uint8_t *pc, uint8_t val)
{
    save_char(pc + 14, &val, 0, 1);
}
#endif

/* Done完成位 */
static uint8_t mbclink_get_Done_D1(uint8_t *pc)
{
    uint8_t val;
    get_char(pc + 18, &val, 0, 1);
    return val;
}

/*
 val=0 : sending data
*/
void mbclink_save_Done_D1(uint8_t *pc, uint8_t val)
{
    save_char(pc + 18, &val, 0, 1);
}


uint16_t mbclink_get_Error_D2(uint8_t *pc)
{
    uint16_t val;
    get_word(pc + 22, &val, 0, 1);
    return val;
}

/* Error:错误代码 */
void mbclink_save_Error_D2(uint8_t *pc, uint16_t val)
{
    save_word(pc + 22, &val, 0, 1);
}

/**
   9E F1 
   00 FF 02 00 // 网络连接标识码
   00 FF 00 00 // 便捷指令编号
   00 FF 01 00 // 执行模式只能为0或者1， 0：只读或者只写，1为：可读可写
   00 05 C8 00 // (BOOL)停止运行的控制位（on表示暂停运行）
   00 05 C9 00 // (BOOL)Done完成位
   00 11 F4 01 // Error：错误代码
   01 00
  */
unsigned char run_ci_mbclink_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    bool ifPrint = false;
#if 0
    static uint32_t mTick = 0;
    uint32_t curTick = xTaskGetTickCount();
    if (curTick - mTick > 3000)
    {
        ifPrint = true;
        mTick = curTick;
    }
    else
    {
        ifPrint = false;
    }
#endif
    if (ifPrint) LOGV("MBC_LINK", "Enter %s(), ltp_RunEnv = 0x%08X", __func__, ltp_RunEnv);
    unsigned short clientID;
    unsigned short lsv_SheetNum;
    unsigned short lsv_EdgeId;
    plc_modlink_head_st *ltp_ModlinkHead;
    plc_modlink_ins_item_st *ltp_ModlinkItem;
    unsigned char lcv_SuspendFlag;
    unsigned short i, j;
    unsigned short lsv_TxLength, lsv_RxLength;
    unsigned char *lcp_UartTxBuff = NULL;
    unsigned short lsv_TxCrc;
    uint8_t flag = 1;

    /* Get the clientID */
    clientID = mbclink_get_S1(ltp_RunEnv->mcp_PC);
    if (ifPrint) LOGV("MBC_LINK", "clientID = %u", clientID);
    if (clientID > MAX_TCP_CONFIG_ITEM || clientID == 0)
    {
        mbclink_save_Error_D2(ltp_RunEnv->mcp_PC, MBCLINK_ERROR_CLIENT_ID);
        if (ifPrint) LOGE("MBC_LINK", "Leave %s()...001", __func__);
        return ERR_MDTCP_EXCEEDMAXCONN;
    }
    clientID--;

    if (gWanOrLan[clientID] == 0)
    {
        if (!plc_get_bit_element_value(SM_ELEMENT, SM269))
        {
            if (gDebug) LOGE("MBC_LINK", "Leave %s(), because WAN not ready now...", __func__);
            return pdPASS;
        }
    }
    else
    {
        if (!plc_get_bit_element_value(SM_ELEMENT, SM270))
        {
            if (gDebug) LOGE("MBC_LINK", "Leave %s(), because LAN not ready now...", __func__);
            return pdPASS;
        }
    }
    
#if 0
    if(!GET_POWER_FLOW(ltp_RunEnv)) // Power flow not effective, just return.
    {
        if (gDebug) LOGE("MBC_LINK", "Power flow not effective, just return.");
        return pdPASS;
    }
    else
    {
        if (gDebug) LOGI("MBC_LINK", "Power flow effective!!!");
    }
#endif

    if (!tcp_client_get_connected_bit(clientID)) // TCP not connected, just return.
    {
        if (ifPrint) LOGW("MBC_LINK", "%s(), TCP not connected, just return.", __func__);
        return pdPASS;
    }
    if (tcp_client_get_connectting_bit(clientID)) //如果正在连接或正在断开
    {
        if (ifPrint) LOGW("MBC_LINK", "%s(), TCP is connectting or disconnectting, so just return.", __func__);
        return pdPASS;
    }

    /*取便携通信指令表序号*/
    lsv_SheetNum = mbclink_get_S2(ltp_RunEnv->mcp_PC);
    if (ifPrint) LOGV("MBC_LINK", "lsv_SheetNum = %u", lsv_SheetNum);
    if(lsv_SheetNum >= MAX_MODLINK_SHEET_NUM)
    {
        mbclink_save_Error_D2(ltp_RunEnv->mcp_PC, MBCLINK_ERROR_SHEET_NUM);
        if (ifPrint) LOGE("MBC_LINK", "Leave %s()...002", __func__);
        return ERR_OPERANDS;
    }

    ltp_ModlinkHead = &gtv_ModlinkSheetHead[lsv_SheetNum];
    if (ifPrint) LOGV("MBC_LINK", "mcv_TableType = %u, mcv_InsListNum = %u", ltp_ModlinkHead->mcv_TableType, ltp_ModlinkHead->mcv_InsListNum);
    if(ltp_ModlinkHead->mcv_TableType != MDL_TYPE_MBCLINK)
    {
        mbclink_save_Error_D2(ltp_RunEnv->mcp_PC, MBCLINK_ERROR_TABLE_TYPE);
        if (ifPrint) LOGE("MBC_LINK", "Leave %s()...003", __func__);
        return ERR_OPERANDS;
    }

    /*取沿号并初始化*/
    lsv_EdgeId = GET_PU16_DATA(ltp_RunEnv->mcp_PC + 26);
    if (ifPrint) LOGV("MBC_LINK", "lsv_EdgeId = %u", lsv_EdgeId);
    if(GET_EDGE_INIT_FLAG(lsv_EdgeId))
    {
        if (ifPrint) LOGV("MBC_LINK", "GET_EDGE_INIT_FLAG done.");
        RST_EDGE_INIT_FLAG(lsv_EdgeId);
        RST_EDGE_VALUE(lsv_EdgeId);
    }

    if(GET_POWER_FLOW(ltp_RunEnv))
    {
        if (ifPrint) LOGD("MBC_LINK", "Power flow effective!!!");

        /*取暂停运行标志*/
        lcv_SuspendFlag = mbclink_get_S4(ltp_RunEnv->mcp_PC);
        /*暂停运行标志置位，直接退出*/
        if(lcv_SuspendFlag)
        {
            if (ifPrint) LOGE("MBC_LINK", "Leave %s()...005", __func__);
            return pdPASS;
        }
#if 0
        if (!mbclink_get_Done_D1(ltp_RunEnv->mcp_PC))
        {
            if (ifPrint) LOGE("MBC_LINK", "%s(), Still sending data, just return.", __func__);
            return pdPASS;
        }
#endif
        /*便携指令表为空，直接退出*/
        if(ltp_ModlinkHead->mcv_InsListNum <= 0)
        {
            mbclink_save_Error_D2(ltp_RunEnv->mcp_PC, MBCLINK_ERROR_INS_NULL);
            if (ifPrint) LOGE("MBC_LINK", "Leave %s()...006", __func__);
            return pdPASS;
        }

        if(ltp_ModlinkHead->msv_ListIndex >= ltp_ModlinkHead->mcv_InsListNum)
        {
            ltp_ModlinkHead->msv_ListIndex = 0;
        }

#if 0
        /*能流由低至高时，从第一条开始执行*/
        if(GET_EDGE_VALUE(lsv_EdgeId) == 0)
        {
            if (ifPrint) LOGD("MBC_LINK", "Power flow from low to high.");
            ltp_ModlinkHead->msv_ListIndex = 0;
            SET_EDGE_VALUE(lsv_EdgeId);
        }
#endif
        /*查找待发送指令*/
        for(i = ltp_ModlinkHead->msv_ListIndex; i < ltp_ModlinkHead->mcv_InsListNum; i++)
        {
            ltp_ModlinkItem = &ltp_ModlinkHead->mtp_InsListPtr[i];
            if(!plc_get_bit_element_value(M_ELEMENT, ltp_ModlinkItem->msv_StopFlagIndex))
            {
                break;
            }
        }

        /*当前待发送指令后无有效发送指令，从头开始查找*/
        if(i >= ltp_ModlinkHead->mcv_InsListNum)
        {
            for(i = 0; i < ltp_ModlinkHead->msv_ListIndex; i++)
            {
                ltp_ModlinkItem = &ltp_ModlinkHead->mtp_InsListPtr[i];
                if(!plc_get_bit_element_value(M_ELEMENT, ltp_ModlinkItem->msv_StopFlagIndex))
                {
                    break;
                }
            }

            /*表格中无有效指令，直接退出*/
            if(i >= ltp_ModlinkHead->msv_ListIndex)
            {
                mbclink_save_Error_D2(ltp_RunEnv->mcp_PC, MBCLINK_ERROR_INS_INVALID);
                if (ifPrint) LOGE("MBC_LINK", "Leave %s()...007", __func__);
                return pdPASS;
            }
        }

        ltp_ModlinkHead->msv_ListIndex = i;

        ltp_ModlinkItem = &ltp_ModlinkHead->mtp_InsListPtr[ltp_ModlinkHead->msv_ListIndex];

        if (ifPrint) LOGI("MBC_LINK", "ltp_ModlinkItem = 0x%08X, mcv_IsExec = %u", ltp_ModlinkItem, ltp_ModlinkItem->mcv_IsExec);
        /*指令未执行*/
        if(!ltp_ModlinkItem->mcv_IsExec)
        {
        #if 0
            /*获取串口发送缓冲区*/
            lcp_UartTxBuff = ring_buffer_get_write_mem(&gtp_UartPort[lsv_PortNum].mtv_SendBuff);
            configASSERT(lcp_UartTxBuff != NULL);

            /*切换发送缓冲区，为下次消息准备*/
            ring_buffer_switch_write_mem(&gtp_UartPort[lsv_PortNum].mtv_SendBuff);
        #endif
            lcp_UartTxBuff = gTcpSendBuf;
            switch(ltp_ModlinkItem->mcv_MbFunc)
            {
            case MB_READ_COILS_STATUS:
            case MB_READ_DESCRETE_INPUT_STATUS:
                lcp_UartTxBuff[0] = ltp_ModlinkItem->mcv_SlaveAddr;
                lcp_UartTxBuff[1] = ltp_ModlinkItem->mcv_MbFunc;
                lcp_UartTxBuff[2] = (unsigned char)(ltp_ModlinkItem->msv_StartAddr >> 8);
                lcp_UartTxBuff[3] = (unsigned char)ltp_ModlinkItem->msv_StartAddr;
                lcp_UartTxBuff[4] = (unsigned char)(ltp_ModlinkItem->msv_ElementCnt >> 8);
                lcp_UartTxBuff[5] = (unsigned char)ltp_ModlinkItem->msv_ElementCnt;
                lsv_TxLength = 6;

                if(ltp_ModlinkItem->msv_ElementCnt % 8)
                {
                    lsv_RxLength = (ltp_ModlinkItem->msv_ElementCnt >> 3) + 1 + 5;
                }
                else
                {
                    lsv_RxLength = (ltp_ModlinkItem->msv_ElementCnt >> 3) + 5;
                }

                /*接收缓冲区越界检查*/
                if(ltp_ModlinkItem->msp_ResultData + lsv_RxLength > ltp_ModlinkItem->msp_MaxResultData)
                {
                    *(ltp_ModlinkItem->msp_ExecResult) = MODLINK_PARA_ERR;
                    if (ifPrint) LOGE("MBC_LINK", "Leave %s()...008", __func__);
                    mbclink_save_Error_D2(ltp_RunEnv->mcp_PC, MBCLINK_ERROR_BUFFER_OVER);
                    return ERR_OVER_ELEMENT_RANG;
                }

                /*设置接收参数*/
                //bsp_set_free_port_receive_para(lsv_PortNum, ltp_ModlinkItem->msp_ResultData, lsv_RxLength, 1);
                flag = 1;
                break;

            case MB_READ_HOLDING_REGISTER:
            case MB_READ_MULTIPLE_INPUT_REGISTER:
                lcp_UartTxBuff[0] = ltp_ModlinkItem->mcv_SlaveAddr;
                lcp_UartTxBuff[1] = ltp_ModlinkItem->mcv_MbFunc;
                lcp_UartTxBuff[2] = (unsigned char)(ltp_ModlinkItem->msv_StartAddr >> 8);
                lcp_UartTxBuff[3] = (unsigned char)ltp_ModlinkItem->msv_StartAddr;
                lcp_UartTxBuff[4] = (unsigned char)(ltp_ModlinkItem->msv_ElementCnt >> 8);
                lcp_UartTxBuff[5] = (unsigned char)ltp_ModlinkItem->msv_ElementCnt;
                lsv_TxLength = 6;

                lsv_RxLength = (ltp_ModlinkItem->msv_ElementCnt << 1) + 5;

                /*接收缓冲区越界检查*/
                if(ltp_ModlinkItem->msp_ResultData + lsv_RxLength > ltp_ModlinkItem->msp_MaxResultData)
                {
                    *(ltp_ModlinkItem->msp_ExecResult) = MODLINK_PARA_ERR;
                    if (ifPrint) LOGE("MBC_LINK", "Leave %s()...009", __func__);
                    mbclink_save_Error_D2(ltp_RunEnv->mcp_PC, MBCLINK_ERROR_BUFFER_OVER2);
                    return ERR_OVER_ELEMENT_RANG;
                }

                /*设置接收参数*/
                //bsp_set_free_port_receive_para(lsv_PortNum, ltp_ModlinkItem->msp_ResultData, lsv_RxLength, 1);
                flag = 1;
                break;

            case MB_WRITE_SINGLE_COIL:
                //MODLINK_DEBUGF(("msv_ElementCnt = %u\r\n", ltp_ModlinkItem->msv_ElementCnt));
                //MODLINK_DEBUGF(("msp_ResultData :\r\n"));
                //hexdump(ltp_ModlinkItem->msp_ResultData, 32);

                lcp_UartTxBuff[0] = ltp_ModlinkItem->mcv_SlaveAddr;
                lcp_UartTxBuff[1] = ltp_ModlinkItem->mcv_MbFunc;
                lcp_UartTxBuff[2] = (unsigned char)(ltp_ModlinkItem->msv_StartAddr >> 8);
                lcp_UartTxBuff[3] = (unsigned char)ltp_ModlinkItem->msv_StartAddr;
                lcp_UartTxBuff[4] = 0x00;
                lcp_UartTxBuff[5] = 0x00;
                if(ltp_ModlinkItem->msv_ElementCnt > 0)
                {
                    if (ltp_ModlinkItem->msp_ResultData[0] != 0)
                    {
                        lcp_UartTxBuff[4] = 0xFF;
                        lcp_UartTxBuff[5] = 0x00;
                    }
                }
                lsv_TxLength = 6;
                lsv_RxLength = 8;
                /*设置接收参数*/
                //bsp_set_free_port_receive_para(lsv_PortNum, ltp_ModlinkHead->msp_RecvBuff, lsv_RxLength, 0);
                flag = 0;
                break;

            case MB_WRITE_REGISTER:
                lcp_UartTxBuff[0] = ltp_ModlinkItem->mcv_SlaveAddr;
                lcp_UartTxBuff[1] = ltp_ModlinkItem->mcv_MbFunc;
                lcp_UartTxBuff[2] = (unsigned char)(ltp_ModlinkItem->msv_StartAddr >> 8);
                lcp_UartTxBuff[3] = (unsigned char)ltp_ModlinkItem->msv_StartAddr;
                lcp_UartTxBuff[4] = (unsigned char)(ltp_ModlinkItem->msv_ElementCnt >> 8);
                lcp_UartTxBuff[5] = (unsigned char)ltp_ModlinkItem->msv_ElementCnt;
                lsv_TxLength = 6;
                lsv_RxLength = 8;
                /*设置接收参数*/
                //bsp_set_free_port_receive_para(lsv_PortNum, ltp_ModlinkHead->msp_RecvBuff, lsv_RxLength, 0);
                flag = 0;
                break;

            case MB_WRITE_MULTIPLE_COILS: // 02 0F 07 D0 00 03 01 00 0F 27
                lcp_UartTxBuff[0] = ltp_ModlinkItem->mcv_SlaveAddr;
                lcp_UartTxBuff[1] = ltp_ModlinkItem->mcv_MbFunc;
                lcp_UartTxBuff[2] = (unsigned char)(ltp_ModlinkItem->msv_StartAddr >> 8);
                lcp_UartTxBuff[3] = (unsigned char)ltp_ModlinkItem->msv_StartAddr;
                lcp_UartTxBuff[4] = (unsigned char)(ltp_ModlinkItem->msv_ElementCnt >> 8);
                lcp_UartTxBuff[5] = (unsigned char)ltp_ModlinkItem->msv_ElementCnt;

                if(ltp_ModlinkItem->msv_ElementCnt % 8)
                {
                    lsv_TxLength = (ltp_ModlinkItem->msv_ElementCnt >> 3) + 1;
                }
                else
                {
                    lsv_TxLength = (ltp_ModlinkItem->msv_ElementCnt >> 3);
                }

                lcp_UartTxBuff[6] = lsv_TxLength;

                /*发送数据缓冲区越界检查*/
                if(ltp_ModlinkItem->msp_ResultData + lsv_TxLength > ltp_ModlinkItem->msp_MaxResultData)
                {
                    *(ltp_ModlinkItem->msp_ExecResult) = MODLINK_PARA_ERR;
                    if (ifPrint) LOGE("MBC_LINK", "Leave %s()...010", __func__);
                    mbclink_save_Error_D2(ltp_RunEnv->mcp_PC, MBCLINK_ERROR_BUFFER_OVER3);
                    return ERR_OVER_ELEMENT_RANG;
                }

                lsv_TxLength += 7;
                j = 0;

                for(i = 7; i < lsv_TxLength; i += 2)
                {
                    lcp_UartTxBuff[i] = (unsigned char)ltp_ModlinkItem->msp_ResultData[j];
                    lcp_UartTxBuff[i + 1] = (unsigned char)(ltp_ModlinkItem->msp_ResultData[j] >> 8);
                    j++;
                }

                lsv_RxLength = 8;

                /*设置接收参数*/
                //bsp_set_free_port_receive_para(lsv_PortNum, ltp_ModlinkHead->msp_RecvBuff, lsv_RxLength, 0);
                flag = 0;
                break;

            case MB_WRITE_MULTIPLE_REGISTERS:
                lcp_UartTxBuff[0] = ltp_ModlinkItem->mcv_SlaveAddr;
                lcp_UartTxBuff[1] = ltp_ModlinkItem->mcv_MbFunc;
                lcp_UartTxBuff[2] = (unsigned char)(ltp_ModlinkItem->msv_StartAddr >> 8);
                lcp_UartTxBuff[3] = (unsigned char)ltp_ModlinkItem->msv_StartAddr;
                lcp_UartTxBuff[4] = (unsigned char)(ltp_ModlinkItem->msv_ElementCnt >> 8);
                lcp_UartTxBuff[5] = (unsigned char)ltp_ModlinkItem->msv_ElementCnt;

                lsv_TxLength = ltp_ModlinkItem->msv_ElementCnt << 1;

                lcp_UartTxBuff[6] = lsv_TxLength;

                /*发送数据缓冲区越界检查*/
                if(ltp_ModlinkItem->msp_ResultData + lsv_TxLength > ltp_ModlinkItem->msp_MaxResultData)
                {
                    *(ltp_ModlinkItem->msp_ExecResult) = MODLINK_PARA_ERR;
                    if (ifPrint) LOGE("MBC_LINK", "Leave %s()...011", __func__);
                    mbclink_save_Error_D2(ltp_RunEnv->mcp_PC, MBCLINK_ERROR_BUFFER_OVER4);
                    return ERR_OVER_ELEMENT_RANG;
                }

                lsv_TxLength += 7;
                j = 0;
                for(i = 7; i < lsv_TxLength; i += 2)
                {
                    lcp_UartTxBuff[i] = (unsigned char)(ltp_ModlinkItem->msp_ResultData[j] >> 8);
                    lcp_UartTxBuff[i + 1] = (unsigned char)ltp_ModlinkItem->msp_ResultData[j];
                    j++;
                }

                lsv_RxLength = 8;

                /*设置接收参数*/
                //bsp_set_free_port_receive_para(lsv_PortNum, ltp_ModlinkHead->msp_RecvBuff, lsv_RxLength, 0);
                flag = 0;
                break;

            default:
                *(ltp_ModlinkItem->msp_ExecResult) = MODLINK_PARA_ERR;
                if (ifPrint) LOGE("MBC_LINK", "Leave %s()...012", __func__);
                return pdFAIL;
            }

            lsv_TxCrc = calc_crc16(lcp_UartTxBuff, lsv_TxLength);
            lcp_UartTxBuff[lsv_TxLength + 1] = (unsigned char)(lsv_TxCrc >> 8);
            lcp_UartTxBuff[lsv_TxLength] = (unsigned char)lsv_TxCrc;

        #if 0
            /*清除相关标志位*/
            RST_UART_SM_FLAG(lsv_PortNum, UART_SM_TX_FINISH);
            RST_UART_SM_FLAG(lsv_PortNum, UART_SM_RX_FINISH);
            RST_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH);
            RST_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR);
            SET_UART_SD_VALUE(lsv_PortNum, UART_SD_MASTER_ERROR_CODE, 0);
            ltp_ModlinkItem->mlv_TimeOut = GET_1MS_TICKS_COUNT() + gtp_UartPort[lsv_PortNum].mtv_ModeInfo.MasterPort.msv_TxTimeOut + 1;
            ltp_ModlinkItem->mcv_IsExec = 1;
            *(ltp_ModlinkItem->msp_ExecResult) = MODLINK_RUNNING;
            /*发送数据*/
            if(gtp_UartPort[lsv_PortNum].pSendFunc)
                gtp_UartPort[lsv_PortNum].pSendFunc(lcp_UartTxBuff, lsv_TxLength + 2);
        #else
            mbclink_save_Done_D1(ltp_RunEnv->mcp_PC, 0); // Sending data...
            mbclink_save_Error_D2(ltp_RunEnv->mcp_PC, MBCLINK_NO_ERROR);
            ltp_ModlinkItem->mlv_TimeOut = GET_1MS_TICKS_COUNT() + 5000;
            ltp_ModlinkItem->mcv_IsExec = 1;
            *(ltp_ModlinkItem->msp_ExecResult) = MODLINK_RUNNING;
            /* 发送数据 */
            mbc_link_send(lcp_UartTxBuff, lsv_TxLength + 2, clientID, ltp_RunEnv->mcp_PC, flag, ltp_ModlinkItem->msp_ResultData);
        #endif
        }
        else
        {
            if (mbclink_get_Done_D1(ltp_RunEnv->mcp_PC)) // Send and receive finished.
            {
                if (mbclink_get_Error_D2(ltp_RunEnv->mcp_PC) == MBCLINK_ERROR_TIME_OUT)
                {
                    *(ltp_ModlinkItem->msp_ExecResult) = MODLINK_RUN_ERR;
                }
                else
                {
                    *(ltp_ModlinkItem->msp_ExecResult) = MODLINK_PASS;
                }
                ltp_ModlinkItem->mcv_IsExec = 0;
                ltp_ModlinkHead->msv_ListIndex += 1;
            }
            #if 0
            if(GET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH) &&
                    GET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR))
            {
                *(ltp_ModlinkItem->msp_ExecResult) = MODLINK_RUN_ERR;
                ltp_ModlinkItem->mcv_IsExec = 0;
                ltp_ModlinkHead->msv_ListIndex += 1;
            }
            else if(GET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH))
            {
                *(ltp_ModlinkItem->msp_ExecResult) = MODLINK_PASS;
                ltp_ModlinkItem->mcv_IsExec = 0;
                ltp_ModlinkHead->msv_ListIndex += 1;
            }
            else if(GET_1MS_TICKS_COUNT() - ltp_ModlinkItem->mlv_TimeOut > 0)
            {
                /*帧传输未超时，等待...*/
                if (ifPrint) LOGE("MBC_LINK", "Leave %s()...013", __func__);
                return pdPASS;
            }
            #endif
        }
    }
    else
    {
        if (ifPrint) LOGW("MBC_LINK", "Power flow not effective!");
        /*能流无效*/
        #if 0
        RST_EDGE_VALUE(lsv_EdgeId);
        uint8_t edge = GET_EDGE_VALUE(lsv_EdgeId);
        if (ifPrint) LOGW("MBC_LINK", "edge = %u", edge);
        #endif
    }

    if (ifPrint) LOGV("MBC_LINK", "Leave %s()...014", __func__);
    return pdPASS;
}
#endif

