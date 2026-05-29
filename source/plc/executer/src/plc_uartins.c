/**
  ******************************************************************************
  * @file    plc_uartins.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   UART 相关指令实现
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

/**
  * @brief  自由口发送数据指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_xmt_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    unsigned char *lcp_UartTxBuff = NULL;
    unsigned short lsv_PortNum;
    uart_port_info_st *ltp_UartPort;
    short lsv_TxLength, lsv_StartElement;
    unsigned short *lsp_SrcPtr;
    unsigned char lcv_ZValue;
    unsigned short i;

    /*判断当前能流*/
    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    /*取COM口号*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_PortNum, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_PortNum >= gtv_DeviceConfigTable.mtv_UartPort.mcv_SupportUartNum) {
        return ERR_OPERANDS;
    }

    ltp_UartPort = &gtp_UartPort[lsv_PortNum];

    if((ltp_UartPort->mcv_Mode != UART_TYPE_FREE_PORT) || (bsp_get_uart_port_status(lsv_PortNum) != UART_IDLE)) {
        return pdPASS;
    }

    /*取需要发送字节数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsv_TxLength, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_TxLength < 0) {
        return ERR_OPERANDS;
    } else if(lsv_TxLength == 0) {
        return pdPASS;
    }

    switch(GET_PU8_DATA(ltp_RunEnv->mcp_PC+7)) {
        case ADDR_D:
            lsv_StartElement = GET_PU16_DATA(ltp_RunEnv->mcp_PC+8);
            if(lsv_StartElement + lsv_TxLength > D_RANG) {
                return ERR_OVER_ELEMENT_RANG;
            } else {
                lsp_SrcPtr = &GET_D_ELEMENT_VALUE(lsv_StartElement);
            }
            break;

        case ADDR_V:
            lsv_StartElement = GET_PU16_DATA(ltp_RunEnv->mcp_PC+8);
            if(lsv_StartElement + lsv_TxLength > V_RANG) {
                return ERR_OVER_ELEMENT_RANG;
            } else {
                lsp_SrcPtr = GET_V_ELEMENT_ADDR(gtp_CallInsInfoPtr->msv_SbrNestedNum,lsv_StartElement);
            }
            break;

        case ADDR_DZ:
            lcv_ZValue = GET_PU8_DATA(ltp_RunEnv->mcp_PC+6);
            if(lcv_ZValue > Z_RANG) {
                return ERR_OVER_ELEMENT_RANG;
            }

            lsv_StartElement = GET_Z_ELEMENT_VALUE(lcv_ZValue) + GET_PU16_DATA(ltp_RunEnv->mcp_PC+8);
            if((lsv_StartElement + lsv_TxLength > D_RANG) || (lsv_StartElement < 0)) {
                return ERR_OVER_ELEMENT_RANG;
            } else {
                lsp_SrcPtr = &GET_D_ELEMENT_VALUE(lsv_StartElement);
            }

            break;

        case ADDR_VZ:
            lcv_ZValue = GET_PU8_DATA(ltp_RunEnv->mcp_PC+6);
            if(lcv_ZValue > Z_RANG) {
                return ERR_OVER_ELEMENT_RANG;
            }

            lsv_StartElement = GET_Z_ELEMENT_VALUE(lcv_ZValue) + GET_PU16_DATA(ltp_RunEnv->mcp_PC+8);
            if((lsv_StartElement + lsv_TxLength > V_RANG) || (lsv_StartElement < 0)) {
                return ERR_OVER_ELEMENT_RANG;
            } else {
                lsp_SrcPtr = GET_V_ELEMENT_ADDR(gtp_CallInsInfoPtr->msv_SbrNestedNum,lsv_StartElement);
            }
            break;

        case ADDR_R:
            lsv_StartElement = GET_PU16_DATA(ltp_RunEnv->mcp_PC+8);
            if(lsv_StartElement + lsv_TxLength > R_RANG) {
                return ERR_OVER_ELEMENT_RANG;
            } else {
                lsp_SrcPtr = &GET_R_ELEMENT_VALUE(lsv_StartElement);
            }
            break;

        case ADDR_RZ:
            lcv_ZValue = GET_PU8_DATA(ltp_RunEnv->mcp_PC+6);
            if(lcv_ZValue > Z_RANG) {
                return ERR_OVER_ELEMENT_RANG;
            }

            lsv_StartElement = GET_Z_ELEMENT_VALUE(lcv_ZValue) + GET_PU16_DATA(ltp_RunEnv->mcp_PC+8);
            if((lsv_StartElement + lsv_TxLength > R_RANG) || (lsv_StartElement < 0)) {
                return ERR_OVER_ELEMENT_RANG;
            } else {
                lsp_SrcPtr = &GET_R_ELEMENT_VALUE(lsv_StartElement);
            }
            break;
    }

    /*获取串口发送缓冲区*/
    lcp_UartTxBuff = ring_buffer_get_write_mem(&gtp_UartPort[lsv_PortNum].mtv_SendBuff);
    configASSERT(lcp_UartTxBuff != NULL);

    /*切换发送缓冲区，为下次消息准备*/
    ring_buffer_switch_write_mem(&gtp_UartPort[lsv_PortNum].mtv_SendBuff);

    /*拷贝发送内容*/
    for(i=0; i< lsv_TxLength; i++) {
        lcp_UartTxBuff[i] = lsp_SrcPtr[i];
    }

    SET_UART_SM_FLAG(lsv_PortNum, UART_SM_FREE_PORT_TX_EN);
    RST_UART_SM_FLAG(lsv_PortNum, UART_SM_TX_FINISH);

    if(gtp_UartPort[lsv_PortNum].pSendFunc)
        gtp_UartPort[lsv_PortNum].pSendFunc(lcp_UartTxBuff, lsv_TxLength);

    return pdPASS;
}

/**
  * @brief  自由口接收数据指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_rcv_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    unsigned short lsv_PortNum;
    uart_port_info_st *ltp_UartPort;
    short lsv_RxLength, lsv_StartElement;
    unsigned short *lsp_DestPtr;
    unsigned char lcv_ZValue;

    /*判断当前能流*/
    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    /*取COM口号*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_PortNum, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_PortNum >= gtv_DeviceConfigTable.mtv_UartPort.mcv_SupportUartNum) {
        return ERR_OPERANDS;
    }

    ltp_UartPort = &gtp_UartPort[lsv_PortNum];

    if((ltp_UartPort->mcv_Mode != UART_TYPE_FREE_PORT) || (bsp_get_uart_port_status(lsv_PortNum) != UART_IDLE)) {
        return pdPASS;
    }

    /*取需要发送字节数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsv_RxLength, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_RxLength < 0) {
        return ERR_OPERANDS;
    } else if(lsv_RxLength == 0) {
        return pdPASS;
    }

    switch(GET_PU8_DATA(ltp_RunEnv->mcp_PC+7)) {
        case ADDR_D:
            lsv_StartElement = GET_PU16_DATA(ltp_RunEnv->mcp_PC+8);
            if(lsv_StartElement + lsv_RxLength > D_RANG) {
                return ERR_OVER_ELEMENT_RANG;
            } else {
                lsp_DestPtr = &GET_D_ELEMENT_VALUE(lsv_StartElement);
            }
            break;

        case ADDR_V:
            lsv_StartElement = GET_PU16_DATA(ltp_RunEnv->mcp_PC+8);
            if(lsv_StartElement + lsv_RxLength > V_RANG) {
                return ERR_OVER_ELEMENT_RANG;
            } else {
                lsp_DestPtr = GET_V_ELEMENT_ADDR(gtp_CallInsInfoPtr->msv_SbrNestedNum,lsv_StartElement);
            }
            break;

        case ADDR_DZ:
            lcv_ZValue = GET_PU8_DATA(ltp_RunEnv->mcp_PC+6);
            if(lcv_ZValue > Z_RANG) {
                return ERR_OVER_ELEMENT_RANG;
            }

            lsv_StartElement = GET_Z_ELEMENT_VALUE(lcv_ZValue) + GET_PU16_DATA(ltp_RunEnv->mcp_PC+8);
            if((lsv_StartElement + lsv_RxLength > D_RANG) || (lsv_StartElement < 0)) {
                return ERR_OVER_ELEMENT_RANG;
            } else {
                lsp_DestPtr = &GET_D_ELEMENT_VALUE(lsv_StartElement);
            }

            break;

        case ADDR_VZ:
            lcv_ZValue = GET_PU8_DATA(ltp_RunEnv->mcp_PC+6);
            if(lcv_ZValue > Z_RANG) {
                return ERR_OVER_ELEMENT_RANG;
            }

            lsv_StartElement = GET_Z_ELEMENT_VALUE(lcv_ZValue) + GET_PU16_DATA(ltp_RunEnv->mcp_PC+8);
            if((lsv_StartElement + lsv_RxLength > V_RANG) || (lsv_StartElement < 0)) {
                return ERR_OVER_ELEMENT_RANG;
            } else {
                lsp_DestPtr = GET_V_ELEMENT_ADDR(gtp_CallInsInfoPtr->msv_SbrNestedNum,lsv_StartElement);
            }
            break;

        case ADDR_R:
            lsv_StartElement = GET_PU16_DATA(ltp_RunEnv->mcp_PC+8);
            if(lsv_StartElement + lsv_RxLength > R_RANG) {
                return ERR_OVER_ELEMENT_RANG;
            } else {
                lsp_DestPtr = &GET_R_ELEMENT_VALUE(lsv_StartElement);
            }
            break;

        case ADDR_RZ:
            lcv_ZValue = GET_PU8_DATA(ltp_RunEnv->mcp_PC+6);
            if(lcv_ZValue > Z_RANG) {
                return ERR_OVER_ELEMENT_RANG;
            }

            lsv_StartElement = GET_Z_ELEMENT_VALUE(lcv_ZValue) + GET_PU16_DATA(ltp_RunEnv->mcp_PC+8);
            if((lsv_StartElement + lsv_RxLength > R_RANG) || (lsv_StartElement < 0)) {
                return ERR_OVER_ELEMENT_RANG;
            } else {
                lsp_DestPtr = &GET_R_ELEMENT_VALUE(lsv_StartElement);
            }
            break;
    }

    SET_UART_SM_FLAG(lsv_PortNum, UART_SM_FREE_PORT_RX_EN);

    bsp_set_free_port_receive_para(lsv_PortNum, lsp_DestPtr, lsv_RxLength, 0);

    return pdPASS;
}


