/**
  ******************************************************************************
  * @file    plc_modbusins.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   MODBUS 相关指令实现
  ******************************************************************************
  */
#include "plc_variable.h"
#include "plc_commonfunc.h"
#include "plc_element.h"
#include "plc_parseaddr.h"
#include "plc_errormsg.h"
#include "plc_instruction.h"
#include "bsp_uart.h"
#include "bsp_dct.h"

#include "verify_func.h"
#include "fsl_debug_console.h"
#include "kalyke_tool.h"


//#define MODLINK_DEBUG
#ifdef MODLINK_DEBUG
static const char *TAG = "MODLINK";
#define LOGE_MODLINK    LOGE
#define LOGW_MODLINK    LOGW
#define LOGI_MODLINK    LOGI
#define LOGD_MODLINK    LOGD
#define LOGV_MODLINK    LOGV
#else
#define LOGE_MODLINK(...)
#define LOGW_MODLINK(...)
#define LOGI_MODLINK(...)
#define LOGD_MODLINK(...)
#define LOGV_MODLINK(...)
#endif

//#define MODBUS_DEBUG
#ifdef MODBUS_DEBUG
static const char *TAG = "MODBUS";

#define LOGE_MODBUS    LOGE
#define LOGW_MODBUS    LOGW
#define LOGI_MODBUS    LOGI
#define LOGD_MODBUS    LOGD
#define LOGV_MODBUS    LOGV
#else
#define LOGE_MODBUS(...)
#define LOGW_MODBUS(...)
#define LOGI_MODBUS(...)
#define LOGD_MODBUS(...)
#define LOGV_MODBUS(...)
#endif

/**
  * @brief  MODBUS 主站指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_modbus_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    LOGV_MODBUS(TAG, "Enter %s(), ltp_RunEnv = 0x%08X", __func__, ltp_RunEnv);
    unsigned char lcv_Ret;
    unsigned short lsv_PortNum;
    unsigned short lsv_EdgeNum, lsv_RxElementRang, lsv_TxElementRang, lsv_TxCrc;
    uart_port_info_st *ltp_UartPort;
    unsigned short *lsp_RxBuff, *lsp_TxBuff;
    short lsv_RxStartElement, lsv_TxStartElement;
    unsigned char lcv_ZValue, lcv_SlaveId;
    unsigned char lcv_TxLength;
    unsigned char *lcp_UartTxBuff = NULL;
    unsigned char i;

    /*获取串口号*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC + 2, &lsv_PortNum, 0, 1);
    if(lcv_Ret != pdPASS)
    {
        LOGW_MODBUS(TAG, "Leave %s()...000", __func__);
        return lcv_Ret;
    }
    if(lsv_PortNum >= gtv_DeviceConfigTable.mtv_UartPort.mcv_SupportUartNum)
    {
        LOGW_MODBUS(TAG, "Leave %s()...001", __func__);
        return ERR_OPERANDS;
    }

    if(!GET_UART_SM_FLAG(lsv_PortNum, UART_SM_IDLE))
    {
        LOGW_MODBUS(TAG, "Leave %s()...002", __func__);
        return pdPASS;
    }

    ltp_UartPort = &gtp_UartPort[lsv_PortNum];

    if(ltp_UartPort->mcv_Mode != UART_TYPE_MB_MASTER)
    {
        LOGW_MODBUS(TAG, "Leave %s()...003", __func__);
        SET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH);
        /*设置主从模式错误标志*/
        SET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR);
        SET_UART_SD_VALUE(lsv_PortNum, UART_SD_MASTER_ERROR_CODE, MB_SERVER_SLAVE_MODE_ERR);
        /*返回网络参数配置错误*/
        return ERR_NET_CONFIG;
    }

    /*取指令沿ID,初始化*/
    lsv_EdgeNum = GET_PU16_DATA(ltp_RunEnv->mcp_PC + 14);
    if(GET_EDGE_INIT_FLAG(lsv_EdgeNum))
    {
        RST_EDGE_INIT_FLAG(lsv_EdgeNum);
        RST_EDGE_VALUE(lsv_EdgeNum);
    }

    /*能流上升沿，串口空闲*/
    #if 1
    if(GET_POWER_FLOW(ltp_RunEnv)
            && (GET_EDGE_VALUE(lsv_EdgeNum) == 0)
            && (bsp_get_uart_port_status(lsv_PortNum) == UART_IDLE))
    #else
    if (GET_POWER_FLOW(ltp_RunEnv) && 
      (bsp_get_uart_port_status(lsv_PortNum) == UART_IDLE))
    #endif
    {
        LOGI_MODBUS(TAG, "POWER FLOW ON & Edge up");
        /*发送数据开始地址*/
        switch(GET_PU8_DATA(ltp_RunEnv->mcp_PC + 7))
        {
        case ADDR_D:
            lsv_TxStartElement = GET_PU16_DATA(ltp_RunEnv->mcp_PC + 8);
            lsv_TxElementRang = D_RANG;
            if(lsv_TxStartElement >= D_RANG)
            {
                SET_EDGE_VALUE(lsv_EdgeNum);
                LOGW_MODBUS(TAG, "Leave %s()...004", __func__);
                return ERR_OVER_ELEMENT_RANG;
            }
            lsp_TxBuff = &GET_D_ELEMENT_VALUE(lsv_TxStartElement);
            break;

        case ADDR_DZ:
            lcv_ZValue = GET_PU8_DATA(ltp_RunEnv->mcp_PC + 6);
            if(lcv_ZValue > Z_RANG)
            {
                LOGW_MODBUS(TAG, "Leave %s()...005", __func__);
                return ERR_OVER_ELEMENT_RANG;
            }

            lsv_TxStartElement = GET_Z_ELEMENT_VALUE(lcv_ZValue) + GET_PU16_DATA(ltp_RunEnv->mcp_PC + 8);
            lsv_TxElementRang = D_RANG;
            if(lsv_TxStartElement >= D_RANG)
            {
                SET_EDGE_VALUE(lsv_EdgeNum);
                LOGW_MODBUS(TAG, "Leave %s()...006", __func__);
                return ERR_OVER_ELEMENT_RANG;
            }
            lsp_TxBuff = &GET_D_ELEMENT_VALUE(lsv_TxStartElement);
            break;

        case ADDR_V:
            lsv_TxStartElement = GET_PU16_DATA(ltp_RunEnv->mcp_PC + 8);
            lsv_TxElementRang = V_RANG;
            if(lsv_TxStartElement >= V_RANG)
            {
                SET_EDGE_VALUE(lsv_EdgeNum);
                LOGW_MODBUS(TAG, "Leave %s()...007", __func__);
                return ERR_OVER_ELEMENT_RANG;
            }
            lsp_TxBuff = GET_V_ELEMENT_ADDR(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_TxStartElement);
            break;

        case ADDR_VZ:
            lcv_ZValue = GET_PU8_DATA(ltp_RunEnv->mcp_PC + 6);
            if(lcv_ZValue > Z_RANG)
            {
                LOGW_MODBUS(TAG, "Leave %s()...008", __func__);
                return ERR_OVER_ELEMENT_RANG;
            }

            lsv_TxStartElement = GET_Z_ELEMENT_VALUE(lcv_ZValue) + GET_PU16_DATA(ltp_RunEnv->mcp_PC + 8);
            lsv_TxElementRang = V_RANG;
            if(lsv_TxStartElement >= V_RANG)
            {
                SET_EDGE_VALUE(lsv_EdgeNum);
                LOGW_MODBUS(TAG, "Leave %s()...009", __func__);
                return ERR_OVER_ELEMENT_RANG;
            }
            lsp_TxBuff = GET_V_ELEMENT_ADDR(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_TxStartElement);
            break;

        case ADDR_R:
            lsv_TxStartElement = GET_PU16_DATA(ltp_RunEnv->mcp_PC + 8);
            lsv_TxElementRang = R_RANG;
            if(lsv_TxStartElement >= R_RANG)
            {
                SET_EDGE_VALUE(lsv_EdgeNum);
                LOGW_MODBUS(TAG, "Leave %s()...010", __func__);
                return ERR_OVER_ELEMENT_RANG;
            }
            lsp_TxBuff = &GET_R_ELEMENT_VALUE(lsv_TxStartElement);
            break;

        case ADDR_RZ:
            lcv_ZValue = GET_PU8_DATA(ltp_RunEnv->mcp_PC + 6);
            if(lcv_ZValue > Z_RANG)
            {
                LOGW_MODBUS(TAG, "Leave %s()...011", __func__);
                return ERR_OVER_ELEMENT_RANG;
            }

            lsv_TxStartElement = GET_Z_ELEMENT_VALUE(lcv_ZValue) + GET_PU16_DATA(ltp_RunEnv->mcp_PC + 8);
            lsv_TxElementRang = R_RANG;
            if(lsv_TxStartElement >= R_RANG)
            {
                SET_EDGE_VALUE(lsv_EdgeNum);
                LOGW_MODBUS(TAG, "Leave %s()...012", __func__);
                return ERR_OVER_ELEMENT_RANG;
            }
            lsp_TxBuff = &GET_R_ELEMENT_VALUE(lsv_TxStartElement);
            break;

        default:
            SET_EDGE_VALUE(lsv_EdgeNum);
            LOGW_MODBUS(TAG, "Leave %s()...013", __func__);
            return ERR_ELEMENT_TYPE;
        }

        /*接收数据开始地址*/
        switch(GET_PU8_DATA(ltp_RunEnv->mcp_PC + 11))
        {
        case ADDR_D:
            lsv_RxStartElement = GET_PU16_DATA(ltp_RunEnv->mcp_PC + 12);
            lsv_RxElementRang = D_RANG;
            if(lsv_RxStartElement >= D_RANG)
            {
                SET_EDGE_VALUE(lsv_EdgeNum);
                LOGW_MODBUS(TAG, "Leave %s()...014", __func__);
                return ERR_OVER_ELEMENT_RANG;
            }
            lsp_RxBuff = &GET_D_ELEMENT_VALUE(lsv_RxStartElement);
            break;

        case ADDR_DZ:
            lcv_ZValue = GET_PU8_DATA(ltp_RunEnv->mcp_PC + 10);
            if(lcv_ZValue > Z_RANG)
            {
                LOGW_MODBUS(TAG, "Leave %s()...015", __func__);
                return ERR_OVER_ELEMENT_RANG;
            }

            lsv_RxStartElement = GET_Z_ELEMENT_VALUE(lcv_ZValue) + GET_PU16_DATA(ltp_RunEnv->mcp_PC + 12);
            lsv_RxElementRang = D_RANG;
            if(lsv_RxStartElement >= D_RANG)
            {
                SET_EDGE_VALUE(lsv_EdgeNum);
                LOGW_MODBUS(TAG, "Leave %s()...016", __func__);
                return ERR_OVER_ELEMENT_RANG;
            }
            lsp_RxBuff = &GET_D_ELEMENT_VALUE(lsv_RxStartElement);
            break;

        case ADDR_V:
            lsv_RxStartElement = GET_PU16_DATA(ltp_RunEnv->mcp_PC + 12);
            lsv_RxElementRang = V_RANG;
            if(lsv_RxStartElement >= V_RANG)
            {
                SET_EDGE_VALUE(lsv_EdgeNum);
                LOGW_MODBUS(TAG, "Leave %s()...017", __func__);
                return ERR_OVER_ELEMENT_RANG;
            }
            lsp_RxBuff = GET_V_ELEMENT_ADDR(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_RxStartElement);
            break;

        case ADDR_VZ:
            lcv_ZValue = GET_PU8_DATA(ltp_RunEnv->mcp_PC + 10);
            if(lcv_ZValue > Z_RANG)
            {
                LOGW_MODBUS(TAG, "Leave %s()...018", __func__);
                return ERR_OVER_ELEMENT_RANG;
            }

            lsv_RxStartElement = GET_Z_ELEMENT_VALUE(lcv_ZValue) + GET_PU16_DATA(ltp_RunEnv->mcp_PC + 12);
            lsv_RxElementRang = V_RANG;
            if(lsv_RxStartElement >= V_RANG)
            {
                SET_EDGE_VALUE(lsv_EdgeNum);
                LOGW_MODBUS(TAG, "Leave %s()...019", __func__);
                return ERR_OVER_ELEMENT_RANG;
            }
            lsp_RxBuff = GET_V_ELEMENT_ADDR(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_RxStartElement);
            break;

        case ADDR_R:
            lsv_RxStartElement = GET_PU16_DATA(ltp_RunEnv->mcp_PC + 12);
            lsv_RxElementRang = R_RANG;
            if(lsv_RxStartElement >= R_RANG)
            {
                SET_EDGE_VALUE(lsv_EdgeNum);
                LOGW_MODBUS(TAG, "Leave %s()...020", __func__);
                return ERR_OVER_ELEMENT_RANG;
            }
            lsp_RxBuff = &GET_R_ELEMENT_VALUE(lsv_RxStartElement);
            break;

        case ADDR_RZ:
            lcv_ZValue = GET_PU8_DATA(ltp_RunEnv->mcp_PC + 10);
            if(lcv_ZValue > Z_RANG)
            {
                LOGW_MODBUS(TAG, "Leave %s()...021", __func__);
                return ERR_OVER_ELEMENT_RANG;
            }

            lsv_RxStartElement = GET_Z_ELEMENT_VALUE(lcv_ZValue) + GET_PU16_DATA(ltp_RunEnv->mcp_PC + 12);
            lsv_RxElementRang = R_RANG;
            if(lsv_RxStartElement >= R_RANG)
            {
                SET_EDGE_VALUE(lsv_EdgeNum);
                LOGW_MODBUS(TAG, "Leave %s()...022", __func__);
                return ERR_OVER_ELEMENT_RANG;
            }
            lsp_RxBuff = &GET_R_ELEMENT_VALUE(lsv_RxStartElement);
            break;

        default:
            SET_EDGE_VALUE(lsv_EdgeNum);
            LOGW_MODBUS(TAG, "Leave %s()...023", __func__);
            return ERR_ELEMENT_TYPE;
        }

        /*取指令操作从站ID*/
        lcv_SlaveId = lsp_TxBuff[0];
        /*设备ID同本站站号，错误返回*/
        if(lcv_SlaveId == GET_UART_SD_VALUE(lsv_PortNum, UART_SD_MODBUS_ID))
        {
            LOGW_MODBUS(TAG, "Leave %s()...024", __func__);
            SET_EDGE_VALUE(lsv_EdgeNum);
            SET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH);
            /*设置主从模式错误标志*/
            SET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR);
            SET_UART_SD_VALUE(lsv_PortNum, UART_SD_MASTER_ERROR_CODE, MB_SAME_SLAVE_ID_ERR);
            /*返回系统块配置错误*/
            return ERR_OPERANDS;
        }


        /*根据MODBUS功能码计算发送数据长度*/
        switch(lsp_TxBuff[1] & 0xFF)
        {
        case MB_READ_COILS_STATUS:
        case MB_READ_DESCRETE_INPUT_STATUS:
        case MB_READ_HOLDING_REGISTER:
        case MB_READ_MULTIPLE_INPUT_REGISTER:
        case MB_WRITE_SINGLE_COIL:
        case MB_WRITE_REGISTER:
        case MB_DIAG_DIAGNOSTIC:
            lcv_TxLength = 6;
            break;

        case MB_WRITE_MULTIPLE_COILS:
        case MB_WRITE_MULTIPLE_REGISTERS:
            lcv_TxLength = 7 + lsp_TxBuff[6] & 0xFF;
            break;

        default:
            LOGW_MODBUS(TAG, "Leave %s()...025", __func__);
            SET_EDGE_VALUE(lsv_EdgeNum);
            SET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH);
            /*设置模式错误标志*/
            SET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR);
            SET_UART_SD_VALUE(lsv_PortNum, UART_SD_MASTER_ERROR_CODE, MB_ILIEGAL_CODE);
            /*返回系统块配置错误*/
            return ERR_OPERANDS;
        }

        /*发送数据地址越界检查*/
        if(lsv_TxStartElement + lcv_TxLength >= lsv_TxElementRang)
        {
            LOGW_MODBUS(TAG, "Leave %s()...026", __func__);
            SET_EDGE_VALUE(lsv_EdgeNum);
            SET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH);
            /*设置模式错误标志*/
            SET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR);
            SET_UART_SD_VALUE(lsv_PortNum, UART_SD_MASTER_ERROR_CODE, MB_ILIEGAL_ADDR);
            /*返回系统块配置错误*/
            return ERR_OVER_ELEMENT_RANG;
        }

        /*获取串口发送缓冲区*/
        lcp_UartTxBuff = ring_buffer_get_write_mem(&gtp_UartPort[lsv_PortNum].mtv_SendBuff);
        configASSERT(lcp_UartTxBuff != NULL);

        /*切换发送缓冲区，为下次消息准备*/
        ring_buffer_switch_write_mem(&gtp_UartPort[lsv_PortNum].mtv_SendBuff);

        /*拷贝发送内容*/
        for(i = 0; i < lcv_TxLength; i++)
        {
            lcp_UartTxBuff[i] = lsp_TxBuff[i] & 0xFF;
        }

        /*计算CRC*/
        lsv_TxCrc = calc_crc16(lcp_UartTxBuff, lcv_TxLength);
        lcp_UartTxBuff[lcv_TxLength + 1] = (unsigned char)(lsv_TxCrc >> 8);
        lcp_UartTxBuff[lcv_TxLength] = (unsigned char)(lsv_TxCrc);

        /*设置接收参数*/
        bsp_set_free_port_receive_para(lsv_PortNum, lsp_RxBuff, (lsv_RxElementRang - lsv_RxStartElement), 0);

        /*清除相关标志位*/
        RST_UART_SM_FLAG(lsv_PortNum, UART_SM_TX_FINISH);
        RST_UART_SM_FLAG(lsv_PortNum, UART_SM_RX_FINISH);
        RST_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH);
        RST_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR);
        SET_UART_SD_VALUE(lsv_PortNum, UART_SD_MASTER_ERROR_CODE, 0);

        /*发送数据*/
        #if 0
        if(gtp_UartPort[lsv_PortNum].pSendFunc)
        {
            gtp_UartPort[lsv_PortNum].pSendFunc(lcp_UartTxBuff, lcv_TxLength + 2);
        }
        #else
        bsp_uart_modbus_send(lsv_PortNum, lcp_UartTxBuff, lcv_TxLength + 2);
        #endif
    }
    else if(GET_POWER_FLOW(ltp_RunEnv))
    {
        LOGI_MODBUS(TAG, "POWER FLOW ON");
        /*能流有效，保存当前能流*/
        SET_EDGE_VALUE(lsv_EdgeNum);
    }
    else
    {
        LOGI_MODBUS(TAG, "POWER FLOW OFF");
        RST_EDGE_VALUE(lsv_EdgeNum);
    }

    LOGW_MODBUS(TAG, "Leave %s()...027", __func__);
    return pdPASS;
}

/**
  * @brief  modlink指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_modlink_ins(plc_run_power_flow_st *ltp_RunEnv)
{    
#if 0
    static uint32_t mTick = 0;
    uint32_t curTick = xTaskGetTickCount();
    if (curTick - mTick > 3000)
    {
        gModLinkDebug = true;
        mTick = curTick;
    }
    else
    {
        gModLinkDebug = true;
    }
#endif
    LOGV_MODLINK(TAG, "Enter %s(), ltp_RunEnv = 0x%08X\r\n", __func__, ltp_RunEnv);
    unsigned char lcv_Ret;
    unsigned short lsv_PortNum;
    unsigned short lsv_SheetNum;
    unsigned short lsv_EdgeId;
    plc_modlink_head_st *ltp_ModlinkHead;
    plc_modlink_ins_item_st *ltp_ModlinkItem;
    uart_port_info_st *ltp_UartPort;
    unsigned char lcv_SuspendFlag;
    unsigned short i, j;
    unsigned short lsv_TxLength, lsv_RxLength;
    unsigned char *lcp_UartTxBuff = NULL;
    unsigned short lsv_TxCrc;

    /*取通信串口号*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC + 2, &lsv_PortNum, 0, 1);
    if(lcv_Ret != pdPASS)
    {
        LOGW_MODLINK(TAG, "Leave %s()...000\r\n", __func__);
        return lcv_Ret;
    }
    LOGE_MODLINK(TAG, "lsv_PortNum = %u\r\n", lsv_PortNum);
    if(lsv_PortNum >= gtv_DeviceConfigTable.mtv_UartPort.mcv_SupportUartNum)
    {
        LOGW_MODLINK(TAG, "Leave %s()...001\r\n", __func__);
        return ERR_OPERANDS;
    }
    if(!GET_UART_SM_FLAG(lsv_PortNum, UART_SM_IDLE))
    {
        LOGW_MODLINK(TAG, "Leave %s()...002\r\n", __func__);
        return pdPASS;
    }

    ltp_UartPort = &gtp_UartPort[lsv_PortNum];
    LOGV_MODLINK(TAG, "ltp_UartPort->mcv_Mode = %u", ltp_UartPort->mcv_Mode);
    if(ltp_UartPort->mcv_Mode != UART_TYPE_MB_MASTER)
    {
        SET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH);
        /*设置主从模式错误标志*/
        SET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR);
        SET_UART_SD_VALUE(lsv_PortNum, UART_SD_MASTER_ERROR_CODE, MB_SERVER_SLAVE_MODE_ERR);
        LOGW_MODLINK(TAG, "Leave %s()...003\r\n", __func__);
        /*返回网络参数配置错误*/
        return ERR_NET_CONFIG;
    }

    /*取便携通信指令表序号*/
    lsv_SheetNum = GET_PU16_DATA(ltp_RunEnv->mcp_PC + 8);
    if(lsv_SheetNum >= MAX_MODLINK_SHEET_NUM)
    {
        LOGW_MODLINK(TAG, "Leave %s()...004\r\n", __func__);
        return ERR_OPERANDS;
    }
    LOGD_MODLINK(TAG, "lsv_SheetNum = %u\r\n", lsv_SheetNum);
    ltp_ModlinkHead = &gtv_ModlinkSheetHead[lsv_SheetNum];
    if(ltp_ModlinkHead->mcv_TableType != MDL_TYPE_MODLINK)
    {
        LOGW_MODLINK(TAG, "Leave %s()...005\r\n", __func__);
        return ERR_OPERANDS;
    }
    LOGV_MODLINK(TAG, "ltp_ModlinkHead = 0x%08X\r\n", ltp_ModlinkHead);

    /*
      9C F1 00 FF 00 00 00 FF 00 00 00 11 00 00 00 05 00 00 00 00 
      9C F1 00 FF 01 00 00 FF 01 00 00 11 00 00 00 05 00 00 01 00 
    */
    /*取沿号并初始化*/
    lsv_EdgeId = GET_PU16_DATA(ltp_RunEnv->mcp_PC + 18);
    LOGI_MODLINK(TAG, "lsv_EdgeId = %u\r\n", lsv_EdgeId);

    LOGD_MODLINK(TAG, "msv_IsInit = 0x%04X, msv_Value = 0x%04X\r\n", gtp_EuEd[0].msv_IsInit, gtp_EuEd[0].msv_Value);
    if(GET_EDGE_INIT_FLAG(lsv_EdgeId))
    {
        RST_EDGE_INIT_FLAG(lsv_EdgeId);
        RST_EDGE_VALUE(lsv_EdgeId);
    }
    LOGD_MODLINK(TAG, "msv_IsInit = 0x%04X, msv_Value = 0x%04X\r\n", gtp_EuEd[0].msv_IsInit, gtp_EuEd[0].msv_Value);

    if(GET_POWER_FLOW(ltp_RunEnv))
    {// && (GET_EDGE_VALUE(lsv_EdgeId) == 0)
        /*能流有效*/
        LOGV_MODLINK(TAG, "POWER_FLOW ON\r\n");
        /*取暂停运行标志*/
        lcv_Ret = get_char(ltp_RunEnv->mcp_PC + 14, &lcv_SuspendFlag, 0, 1);
        if(lcv_Ret != pdPASS)
        {
            LOGW_MODLINK(TAG, "Leave %s()...006\r\n", __func__);
            return lcv_Ret;
        }

        /*暂停运行标志置位，直接退出*/
        if(lcv_SuspendFlag)
        {
            LOGW_MODLINK(TAG, "Leave %s()...007\r\n", __func__);
            return pdPASS;
        }

        /*便携指令表为空，直接退出*/
        if(ltp_ModlinkHead->mcv_InsListNum <= 0)
        {
            LOGW_MODLINK(TAG, "Leave %s()...008\r\n", __func__);
            return pdPASS;
        }

        LOGV_MODLINK(TAG, "ltp_ModlinkHead->msv_ListIndex = %u, ltp_ModlinkHead->mcv_InsListNum = %u\r\n", ltp_ModlinkHead->msv_ListIndex, ltp_ModlinkHead->mcv_InsListNum);
        if(ltp_ModlinkHead->msv_ListIndex >= ltp_ModlinkHead->mcv_InsListNum)
        {
            ltp_ModlinkHead->msv_ListIndex = 0;
        }

#if 0
        /*能流由低至高时，从第一条开始执行*/
        if(GET_EDGE_VALUE(lsv_EdgeId) == 0)
        {
            ltp_ModlinkHead->msv_ListIndex = 0;
            SET_EDGE_VALUE(lsv_EdgeId);
        }
#endif
        LOGD_MODLINK(TAG, "msv_ListIndex = %u\r\n", ltp_ModlinkHead->msv_ListIndex);
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
                LOGW_MODLINK(TAG, "Leave %s()...009\r\n", __func__);
                return pdPASS;
            }
        }

        ltp_ModlinkHead->msv_ListIndex = i;

        ltp_ModlinkItem = &ltp_ModlinkHead->mtp_InsListPtr[ltp_ModlinkHead->msv_ListIndex];

        /*指令未执行*/
        if(!ltp_ModlinkItem->mcv_IsExec)
        {

            /*获取串口发送缓冲区*/
            lcp_UartTxBuff = ring_buffer_get_write_mem(&gtp_UartPort[lsv_PortNum].mtv_SendBuff);
            configASSERT(lcp_UartTxBuff != NULL);

            /*切换发送缓冲区，为下次消息准备*/
            ring_buffer_switch_write_mem(&gtp_UartPort[lsv_PortNum].mtv_SendBuff);
            LOGI_MODLINK(TAG, "ltp_ModlinkItem->mcv_MbFunc = 0x%X\r\n", ltp_ModlinkItem->mcv_MbFunc);
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
                    LOGW_MODLINK(TAG, "Leave %s()...010\r\n", __func__);
                    return ERR_OVER_ELEMENT_RANG;
                }

                /*设置接收参数*/
                bsp_set_free_port_receive_para(lsv_PortNum, ltp_ModlinkItem->msp_ResultData, lsv_RxLength, 1);

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
                    LOGW_MODLINK(TAG, "Leave %s()...011\r\n", __func__);
                    return ERR_OVER_ELEMENT_RANG;
                }

                /*设置接收参数*/
                bsp_set_free_port_receive_para(lsv_PortNum, ltp_ModlinkItem->msp_ResultData, lsv_RxLength, 1);

                break;

            case MB_WRITE_SINGLE_COIL:
                LOGV_MODLINK(TAG, "msv_ElementCnt = %u\r\n", ltp_ModlinkItem->msv_ElementCnt);
                //LOGE_MODLINK(TAG, "msp_ResultData :\r\n");
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
                bsp_set_free_port_receive_para(lsv_PortNum, ltp_ModlinkHead->msp_RecvBuff, lsv_RxLength, 0);

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
                bsp_set_free_port_receive_para(lsv_PortNum, ltp_ModlinkHead->msp_RecvBuff, lsv_RxLength, 0);
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
                    LOGW_MODLINK(TAG, "Leave %s()...012\r\n", __func__);
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
                bsp_set_free_port_receive_para(lsv_PortNum, ltp_ModlinkHead->msp_RecvBuff, lsv_RxLength, 0);

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
                    LOGW_MODLINK(TAG, "Leave %s()...013\r\n", __func__);
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
                bsp_set_free_port_receive_para(lsv_PortNum, ltp_ModlinkHead->msp_RecvBuff, lsv_RxLength, 0);
                break;

            default:
                *(ltp_ModlinkItem->msp_ExecResult) = MODLINK_PARA_ERR;
                LOGW_MODLINK(TAG, "Leave %s()...014\r\n", __func__);
                return pdFAIL;
            }

            lsv_TxCrc = calc_crc16(lcp_UartTxBuff, lsv_TxLength);
            lcp_UartTxBuff[lsv_TxLength + 1] = (unsigned char)(lsv_TxCrc >> 8);
            lcp_UartTxBuff[lsv_TxLength] = (unsigned char)lsv_TxCrc;

            /*清除相关标志位*/
            RST_UART_SM_FLAG(lsv_PortNum, UART_SM_TX_FINISH);
            RST_UART_SM_FLAG(lsv_PortNum, UART_SM_RX_FINISH);
            RST_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH);
            RST_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR);
            SET_UART_SD_VALUE(lsv_PortNum, UART_SD_MASTER_ERROR_CODE, 0);

            ltp_ModlinkItem->mlv_TimeOut = GET_1MS_TICKS_COUNT() + gtp_UartPort[lsv_PortNum].mtv_ModeInfo.MasterPort.msv_TxTimeOut + 1;
            LOGW_MODLINK(TAG, "msv_TxTimeOut = %u, ltp_ModlinkItem->mlv_TimeOut = %u\r\n", gtp_UartPort[lsv_PortNum].mtv_ModeInfo.MasterPort.msv_TxTimeOut, ltp_ModlinkItem->mlv_TimeOut);
            ltp_ModlinkItem->mcv_IsExec = 1;

            *(ltp_ModlinkItem->msp_ExecResult) = MODLINK_RUNNING;

            /*发送数据*/
            if(gtp_UartPort[lsv_PortNum].pSendFunc)
                gtp_UartPort[lsv_PortNum].pSendFunc(lcp_UartTxBuff, lsv_TxLength + 2);

        }
        else
        {
            uint32_t curTick = GET_1MS_TICKS_COUNT();
            int32_t interVal = curTick - ltp_ModlinkItem->mlv_TimeOut;
            LOGI_MODLINK(TAG, "ltp_ModlinkItem->mlv_TimeOut = %u, curTick = %u, interVal = %d\r\n", ltp_ModlinkItem->mlv_TimeOut, curTick, interVal);
            if(GET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH) &&
                    GET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR))
            {
                *(ltp_ModlinkItem->msp_ExecResult) = MODLINK_RUN_ERR;
                ltp_ModlinkItem->mcv_IsExec = 0;
                ltp_ModlinkHead->msv_ListIndex += 1;
                LOGW_MODLINK(TAG, "Leave %s()...015\r\n", __func__);
                return pdPASS;
            }
            else if(GET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH))
            {
                *(ltp_ModlinkItem->msp_ExecResult) = MODLINK_PASS;
                ltp_ModlinkItem->mcv_IsExec = 0;
                ltp_ModlinkHead->msv_ListIndex += 1;
                LOGW_MODLINK(TAG, "Leave %s()...016\r\n", __func__);
                return pdPASS;
            }
            else if(interVal < 0)
            {
                /*帧传输未超时，等待...*/
                LOGW_MODLINK(TAG, "Leave %s()...017, Not time out, waitting...\r\n", __func__);
                return pdPASS;
            }
            else
            {
                // 超时了
                SET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH);
                SET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR);
                LOGW_MODLINK(TAG, "Leave %s()...018, Time out\r\n", __func__);
                return pdPASS;
            }
        }
    }
    else
    {
        /*能流无效*/
        LOGV_MODLINK(TAG, "POWER_FLOW OFF\r\n");
        #if 1
        LOGD_MODLINK(TAG, "msv_IsInit = 0x%04X, msv_Value = 0x%04X\r\n", gtp_EuEd[0].msv_IsInit, gtp_EuEd[0].msv_Value);
        RST_EDGE_VALUE(lsv_EdgeId);
        LOGD_MODLINK(TAG, "msv_IsInit = 0x%04X, msv_Value = 0x%04X\r\n", gtp_EuEd[0].msv_IsInit, gtp_EuEd[0].msv_Value);
        #endif
    }

    LOGW_MODLINK(TAG, "Leave %s()...019\r\n", __func__);
    return pdPASS;
}




/**
  * @brief  modrw指令
  * @param  None
  * @retval None
  */
// modrw 0 3  D3 100 D0 D200
// 60 F1 , 00 ff 00 00 , 00 ff 03 00 , 00 11 03 00 ,00 ff 64 00 , 00 11 00 00 , 00 11 C8 00, 00 00

unsigned char run_ci_modrw_ins(plc_run_power_flow_st *ltp_RunEnv)
{    
    LOGV_MODBUS(TAG, "Enter %s(), ltp_RunEnv = 0x%08X", __func__, ltp_RunEnv);
    unsigned char lcv_Ret;
    unsigned short lsv_PortNum;
    unsigned short lsv_EdgeNum, lsv_RxElementRang, lsv_TxCrc;
    uart_port_info_st *ltp_UartPort;
    unsigned short *lsp_RxBuff, *lsp_TxBuff;
    short lsv_RxStartElement, lsv_TxStartElement;
    unsigned char lcv_ZValue, lcv_SlaveId;
    unsigned char lcv_TxLength;
    unsigned char *lcp_UartTxBuff = NULL;
    unsigned char i,k;

    unsigned char *pu8Tmp;
    unsigned short tmpu16;

    unsigned short s2,s3,s4,s5;

    unsigned short TxByteCnt;           // 发送数据长度（不包括CRC校验）
    unsigned short RxByteCnt;           // 接收元件长度 (纯数据)   
    

    /*获取串口号*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC + 2, &lsv_PortNum, 0, 1);
    if(lcv_Ret != pdPASS)
    {
        LOGW_MODBUS(TAG, "Leave %s()...000", __func__);
        return lcv_Ret;
    }
    if(lsv_PortNum >= gtv_DeviceConfigTable.mtv_UartPort.mcv_SupportUartNum)
    {
        LOGW_MODBUS(TAG, "Leave %s()...001", __func__);
        return ERR_OPERANDS;
    }

    if(!GET_UART_SM_FLAG(lsv_PortNum, UART_SM_IDLE))
    {
        LOGW_MODBUS(TAG, "Leave %s()...002", __func__);
        return pdPASS;
    }

    ltp_UartPort = &gtp_UartPort[lsv_PortNum];

    if(ltp_UartPort->mcv_Mode != UART_TYPE_MB_MASTER)
    {
        LOGW_MODBUS(TAG, "Leave %s()...003", __func__);
        SET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH);
        /*设置主从模式错误标志*/
        SET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR);
        SET_UART_SD_VALUE(lsv_PortNum, UART_SD_MASTER_ERROR_CODE, MB_SERVER_SLAVE_MODE_ERR);
        /*返回网络参数配置错误*/
        return ERR_NET_CONFIG;
    }

    /*取指令沿ID,初始化*/
    lsv_EdgeNum = GET_PU16_DATA(ltp_RunEnv->mcp_PC + 26);
    if(GET_EDGE_INIT_FLAG(lsv_EdgeNum))
    {
        RST_EDGE_INIT_FLAG(lsv_EdgeNum);
        RST_EDGE_VALUE(lsv_EdgeNum);
    }

    // 取从站号
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC + 6, &s2, 0, 1);   
    if(lcv_Ret != pdPASS)
    {
        LOGW_MODBUS(TAG, "Leave %s()...S2", __func__);
        return lcv_Ret;
    }

     /*设备ID同本站站号，错误返回*/
    if(s2 == GET_UART_SD_VALUE(lsv_PortNum, UART_SD_MODBUS_ID))
    {
            LOGW_MODBUS(TAG, "Leave %s()...024", __func__);
            SET_EDGE_VALUE(lsv_EdgeNum);
            SET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH);
            /*设置主从模式错误标志*/
            SET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR);
            SET_UART_SD_VALUE(lsv_PortNum, UART_SD_MASTER_ERROR_CODE, MB_SAME_SLAVE_ID_ERR);
			
            /*返回系统块配置错误*/
            return ERR_OPERANDS;
    }

    /*能流上升沿，串口空闲*/
    #if 1
    if(GET_POWER_FLOW(ltp_RunEnv)
            && (GET_EDGE_VALUE(lsv_EdgeNum) == 0)
            && (bsp_get_uart_port_status(lsv_PortNum) == UART_IDLE))
    #else
    if (GET_POWER_FLOW(ltp_RunEnv) && 
      (bsp_get_uart_port_status(lsv_PortNum) == UART_IDLE))
    #endif
    {

         // 取从功能码
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC +10, &s3, 0, 1);     
        if(lcv_Ret != pdPASS)
        {
            LOGW_MODBUS(TAG, "S3 = %d lcv_ret = %d", s3,lcv_Ret);
            LOGW_MODBUS(TAG, "Leave %s()...S3", __func__);
            return lcv_Ret;
        }
 
        //功能码
        switch(s3) 
        {
           case MB_READ_COILS_STATUS:		             // 0x01 读位元件（线圈）
           case MB_READ_DESCRETE_INPUT_STATUS:	// 0x02 读多个输入的状态
                  TxByteCnt = 6;
                  if(s5 &0x07) 
                  {
                    RxByteCnt = (s5>>3) +1;
                  }
                 else 
                 {
                   RxByteCnt = (s5>>3);
                 }
                 tmpu16 = 0;
                 break;
           case MB_READ_HOLDING_REGISTER:	              // 0x03 读取多个寄存器
           case MB_READ_MULTIPLE_INPUT_REGISTER:    // 0x04 读取多个输入寄存器
                 TxByteCnt = 6;
                 RxByteCnt = (s5<<1);
                 tmpu16 = 0;
                 break;
           case MB_WRITE_SINGLE_COIL:				// 0x05 写单个位元件（线圈）
                 TxByteCnt = 6;
                 RxByteCnt = 0;    	
                 tmpu16 = 1;
                 break;
           case MB_WRITE_REGISTER:			             // 0x06 写单个寄存器
                 TxByteCnt = 6;
                 RxByteCnt = 0;
                 tmpu16 = 2;
                 break;
           case MB_WRITE_MULTIPLE_COILS:				// 0x0f 写多个线圈
                 if(s5 &0x07) 
                 {
                     tmpu16 = (s5>>3) +1;
                 }
                 else 
                 {
                     tmpu16 = (s5>>3);
                 }
                 TxByteCnt = 7 + tmpu16;
                 RxByteCnt = 0;
                 break;
           case MB_WRITE_MULTIPLE_REGISTERS:			// 0x10 写多个寄存器
                 tmpu16 = (s5<<1);
                 TxByteCnt = 7 + (s5<<1);
                 RxByteCnt = 0;
                 break;
           default:

              SET_EDGE_VALUE(lsv_EdgeNum);                                               // 保存当前能流
              SET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH);  // Modbus完成
              /*设置主从模式错误标志*/
              SET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR); // Modbus错误
              SET_UART_SD_VALUE(lsv_PortNum, UART_SD_MASTER_ERROR_CODE, MB_ILIEGAL_CODE);  // 功能码错误
              return ERR_OPERANDS;

        }

         // 取开始地址
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC +14, &s4, 0, 1);                             
        if(lcv_Ret != pdPASS)
        {
            LOGW_MODBUS(TAG, "Leave %s()...S4", __func__);   
            return lcv_Ret;
        } 
        // 取"元件个数"或者"元件值"
       lcv_Ret = get_word(ltp_RunEnv->mcp_PC +18, &s5, 0, 1);                             
        if(lcv_Ret != pdPASS)
       {
           LOGW_MODBUS(TAG, "Leave %s()...S5", __func__);   
           return lcv_Ret;
       }

       //保存地址(S6)
       switch( *(ltp_RunEnv->mcp_PC + 23) ) 
       {
       case ADDR_D:      // D元件
            lsv_RxStartElement = GET_PU16_DATA(ltp_RunEnv->mcp_PC +24);   // 为MODBUS发送地址越界添加
            lsv_RxElementRang = D_RANG;
            if(lsv_RxStartElement >=D_RANG)   // 判断地址是否越界
            {                             
                SET_EDGE_VALUE(lsv_EdgeNum);
                LOGW_MODBUS(TAG, "Leave %s()...004", __func__);
                return ERR_OVER_ELEMENT_RANG;
            }
            lsp_RxBuff = &GET_D_ELEMENT_VALUE(lsv_RxStartElement);
            break;
	
       case ADDR_DZ:     // D元件
            lsv_RxStartElement = GET_PU16_DATA(ltp_RunEnv->mcp_PC+24);  
            lsv_RxStartElement += GET_Z_ELEMENT_VALUE(*(ltp_RunEnv->mcp_PC+22)); // 为MODBUS发送地址越界添加
            lsv_RxElementRang = D_RANG;
            if(lsv_RxStartElement >=D_RANG)                                   // 判断地址是否越界
            {     
                SET_EDGE_VALUE(lsv_EdgeNum);
                LOGW_MODBUS(TAG, "Leave %s()...004", __func__);
                return ERR_OVER_ELEMENT_RANG;
            }
            lsp_RxBuff = &GET_D_ELEMENT_VALUE(lsv_RxStartElement);
            break;
			
       case ADDR_V:      // V元件
            lsv_RxStartElement = GET_PU16_DATA(ltp_RunEnv->mcp_PC+24);      // 为MODBUS发送地址越界添加
            lsv_RxElementRang = V_RANG;
            if(lsv_RxStartElement >=V_RANG)   // 判断地址是否越界
            {                              
                SET_EDGE_VALUE(lsv_EdgeNum);
                LOGW_MODBUS(TAG, "Leave %s()...004", __func__);
                return ERR_OVER_ELEMENT_RANG;
            }
            lsp_RxBuff = &GET_V_ELEMENT_VALUE(gtp_CallInsInfoPtr->msv_SbrNestedNum,lsv_RxStartElement);
            break;

       case ADDR_VZ:     // V元件
            lsv_RxStartElement = GET_PU16_DATA(ltp_RunEnv->mcp_PC+24);  
            lsv_RxStartElement += GET_Z_ELEMENT_VALUE(*(ltp_RunEnv->mcp_PC+22));     // 为MODBUS发送地址越界添加
            lsv_RxElementRang = V_RANG;
            if(lsv_RxStartElement >=V_RANG)
            {                                            // 判断地址是否越界
                SET_EDGE_VALUE(lsv_EdgeNum);
                LOGW_MODBUS(TAG, "Leave %s()...004", __func__);
                return ERR_OVER_ELEMENT_RANG;
            }
            lsp_RxBuff = &GET_V_ELEMENT_VALUE(gtp_CallInsInfoPtr->msv_SbrNestedNum,lsv_RxStartElement);
            break;

       case ADDR_R:      // R元件
            lsv_RxStartElement = GET_PU16_DATA(ltp_RunEnv->mcp_PC+24);  // 为MODBUS发送地址越界添加
            lsv_RxElementRang = R_RANG;
            if(lsv_RxStartElement >=R_RANG) 
            {                                            // 判断地址是否越界
                SET_EDGE_VALUE(lsv_EdgeNum);
                LOGW_MODBUS(TAG, "Leave %s()...004", __func__);
                return ERR_OVER_ELEMENT_RANG;
            }
            lsp_RxBuff = &GET_R_ELEMENT_VALUE(lsv_RxStartElement);
            break;
			
       case ADDR_RZ:     // R元件
            lsv_RxStartElement = GET_PU16_DATA(ltp_RunEnv->mcp_PC+24);  
            lsv_RxStartElement += GET_Z_ELEMENT_VALUE(*(ltp_RunEnv->mcp_PC+22));  // 为MODBUS发送地址越界添加
            lsv_RxElementRang = R_RANG;
            if(lsv_RxStartElement >=R_RANG) 
            {                                            // 判断地址是否越界
                SET_EDGE_VALUE(lsv_EdgeNum);
                LOGW_MODBUS(TAG, "Leave %s()...004", __func__);
                return ERR_OVER_ELEMENT_RANG;
            }
            lsp_RxBuff = &GET_R_ELEMENT_VALUE(lsv_RxStartElement);
            break;

       default:
            SET_EDGE_VALUE(lsv_EdgeNum);                                            // 保存当前能流
            return ERR_OVER_ELEMENT_RANG;
       }

        if(lsv_RxStartElement >(lsv_RxElementRang-RxByteCnt)) 
        {                                  // 判断起始地址是否越界
            SET_EDGE_VALUE(lsv_EdgeNum);                                         // 保存当前能流
            return ERR_OVER_ELEMENT_RANG;
        }
        if(lsv_RxStartElement >(lsv_RxElementRang-(tmpu16>>1))) 
        {                               // 判断起始地址是否越界
            SET_EDGE_VALUE(lsv_EdgeNum);                                           // 保存当前能流
            return ERR_OVER_ELEMENT_RANG;
        }


            /*获取串口发送缓冲区*/
        lcp_UartTxBuff = ring_buffer_get_write_mem(&gtp_UartPort[lsv_PortNum].mtv_SendBuff);
        configASSERT(lcp_UartTxBuff != NULL);


        switch(s3)  // 根据功能码,填写数据发送                                                     
        {                                                       
        case MB_READ_COILS_STATUS:		              // 0x01 读位元件（线圈）
        case MB_READ_DESCRETE_INPUT_STATUS:	// 0x02 读多个输入的状态
        case MB_READ_HOLDING_REGISTER:         	// 0x03 读取多个寄存器
        case MB_READ_MULTIPLE_INPUT_REGISTER:    // 0x04 读取多个输入寄存器
        case MB_WRITE_SINGLE_COIL:				// 0x05 写单个位元件（线圈）
        case MB_WRITE_REGISTER:			              // 0x06 写单个寄存器
            lcp_UartTxBuff[0] = s2;
            lcp_UartTxBuff[1] = s3;
            lcp_UartTxBuff[2] = (unsigned char)(s4>>8);
            lcp_UartTxBuff[3] = (unsigned char)(s4);
            lcp_UartTxBuff[4] = (unsigned char)(s5>>8);
            lcp_UartTxBuff[5] = (unsigned char)(s5);

            lcv_TxLength = 6;
            break;

        case MB_WRITE_MULTIPLE_COILS:				// 0x0f 写多个线圈
            lcp_UartTxBuff[0] = s2;
            lcp_UartTxBuff[1] = s3;
            lcp_UartTxBuff[2] = (unsigned char)(s4>>8);
            lcp_UartTxBuff[3] = (unsigned char)(s4);
            lcp_UartTxBuff[4] = (unsigned char)(s5>>8);
            lcp_UartTxBuff[5] = (unsigned char)(s5);    	
            lcp_UartTxBuff[6] = (unsigned char)(tmpu16);

            pu8Tmp = (unsigned char *)(lsp_RxBuff);
            k = 0;
            for(i=7; i<TxByteCnt; i++) 
            {
                lcp_UartTxBuff[i] = pu8Tmp[k];
                k++;
            }
            lcv_TxLength = TxByteCnt;
            break;
        case MB_WRITE_MULTIPLE_REGISTERS:			// 0x10 写多个寄存器
            lcp_UartTxBuff[0] = s2;
            lcp_UartTxBuff[1] = s3;
            lcp_UartTxBuff[2] = (unsigned char)(s4>>8);
            lcp_UartTxBuff[3] = (unsigned char)(s4);
            lcp_UartTxBuff[4] = (unsigned char)(s5>>8);
            lcp_UartTxBuff[5] = (unsigned char)(s5);

            lcp_UartTxBuff[6] = (unsigned char)(tmpu16);
            k = 0;
            for(i=7; i<TxByteCnt; i+=2) 
            {
                lcp_UartTxBuff[i] = (unsigned char)(lsp_RxBuff[k]>>8);
                lcp_UartTxBuff[i+1] = (unsigned char)(lsp_RxBuff[k]);
                k++;
            }
            lcv_TxLength = TxByteCnt;
            break;
        default:
            SET_EDGE_VALUE(lsv_EdgeNum);                                                 // 保存当前能流
            SET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH);    // Modbus完成
            SET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR);                               // Modbus错误
            SET_UART_SD_VALUE(lsv_PortNum, UART_SD_MASTER_ERROR_CODE, MB_ILIEGAL_CODE);         // 功能码错误
             //返回系统块配置错误
            return ERR_OPERANDS;
        }

        lsv_TxCrc = calc_crc16(lcp_UartTxBuff, TxByteCnt);   // CRC校验
        lcp_UartTxBuff[TxByteCnt + 1] = (unsigned char)(lsv_TxCrc >> 8);
        lcp_UartTxBuff[TxByteCnt] = (unsigned char)lsv_TxCrc;

        lcv_TxLength= TxByteCnt + 2;

        /*设置接收参数*/
        bsp_set_free_port_receive_para(lsv_PortNum, lsp_RxBuff, (lsv_RxElementRang - lsv_RxStartElement), 0);

        /*清除相关标志位*/
        RST_UART_SM_FLAG(lsv_PortNum, UART_SM_TX_FINISH);
        RST_UART_SM_FLAG(lsv_PortNum, UART_SM_RX_FINISH);
        RST_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH);
        RST_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR);
        SET_UART_SD_VALUE(lsv_PortNum, UART_SD_MASTER_ERROR_CODE, 0);

        /*发送数据*/
        #if 0
        if(gtp_UartPort[lsv_PortNum].pSendFunc)
        {
            gtp_UartPort[lsv_PortNum].pSendFunc(lcp_UartTxBuff, lcv_TxLength + 2);
        }
        #else
        bsp_uart_modbus_send(lsv_PortNum, lcp_UartTxBuff, lcv_TxLength + 2);
        #endif


        SET_EDGE_VALUE(lsv_EdgeNum);                                                // 保存当前能流
        
        return pdPASS;
    }
    else if(GET_POWER_FLOW(ltp_RunEnv))
    {
        LOGI_MODBUS(TAG, "POWER FLOW ON");
        /*能流有效，保存当前能流*/
        SET_EDGE_VALUE(lsv_EdgeNum);
    }
    else
    {
        LOGI_MODBUS(TAG, "POWER FLOW OFF");
        RST_EDGE_VALUE(lsv_EdgeNum);
    }

    LOGW_MODBUS(TAG, "Leave %s()...027", __func__);
    return pdPASS;
}

