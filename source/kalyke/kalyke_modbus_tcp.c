
#include <stdio.h>
#include "lwip/netifapi.h"

#include "kalyke_opts.h"
#include "kalyke_internet_task.h"
#include "kalyke_event.h"
#include "plc_variable.h"
#include "plc_errormsg.h"
#include "verify_func.h"
#include "kalyke_tool.h"
#include "plc_sysblock.h"

static const char *TAG = "MODBUSTCP";


typedef struct _NETCONN_MODBUSTCP_ST
{
    struct netconn *modbus_conn;
    uint16_t ti; //Transaction Identifier
} netconn_modbustcp_st;

typedef struct _CREATE_RECV_TASK
{
    uint32_t clientid;
    uint32_t snum;
} create_recv_task_st;

enum _MODBUSTCP_RESULT_E
{
    MODBUSTCP_NOT_CONNECTED = 0, //尚未和从站建立连接
    MODBUSTCP_SLAVE_HAD_RESP,//从站已响应（对应MODBUS的正常）   
    MODBUSTCP_CONNECTED, //已和从站建立连接
    MODBUSTCP_WAIT_RESP, //刚给从站发完信息  ，等待从站响应
    MODBUSTCP_SLAVE_NO_RESP = 255, //从站没有响应，超时了
};

static bool gDebug = false;
static uint8_t gTcpSendBuf[128];
TaskHandle_t gModbusSendHandle = NULL;

#if 0
static TimerHandle_t gTCPTimeoutTimer0 = NULL;
static TimerHandle_t gTCPTimeoutTimer1 = NULL;
static TimerHandle_t gTCPTimeoutTimer2 = NULL;
#endif
static TimerHandle_t gTCPSendDataTimeoutTimers[MAX_MODBUS_TCP_ITEM];

static struct netconn *g_connModbus[MAX_MODBUS_TCP_ITEM + 1];
static TaskHandle_t gModbusRecvTaskHandle[MAX_MODBUS_TCP_ITEM + 1];
static uint8_t gModbusTCPSendBuf[1024];

static uint8_t get_item_number(uint8_t clientID);


#if 1
static void tcpSendTimeoutCB(TimerHandle_t ltv_TimeHandle)
{
    uint8_t clientID = (unsigned int)pvTimerGetTimerID(ltv_TimeHandle);

    //LOGE(TAG, "Enter %s(), clientID = %u", __func__, clientID);
#if 0
    uint8_t inum = get_item_number(clientID);
#else
    uint8_t inum = g_modbusTCP_cfg.index[clientID];
#endif
    g_modbusTCP_cfg.listPtr[inum].execState = MODBUSTCP_SLAVE_NO_RESP;
    *g_modbusTCP_cfg.listPtr[inum].execResult = MODBUSTCP_SLAVE_NO_RESP;

}

static void stop_tcp_send_timeout_timer(uint8_t clientID)
{
    if (gTCPSendDataTimeoutTimers[clientID] != NULL)
    {
        xTimerStop(gTCPSendDataTimeoutTimers[clientID], 1);
    }
}

static void start_tcp_send_timeout_timer(uint8_t clientID)
{
    if (gTCPSendDataTimeoutTimers[clientID] == NULL)
    {
        char name[32];
        sprintf(name, "TimeOut%u", clientID);
        gTCPSendDataTimeoutTimers[clientID] = xTimerCreate(name,
                                             (TickType_t  )5000 / portTICK_PERIOD_MS,
                                             (UBaseType_t )pdFALSE,
                                             (void *      )(uint32_t)clientID,
                                             (TimerCallbackFunction_t)tcpSendTimeoutCB);
        xTimerStart(gTCPSendDataTimeoutTimers[clientID], 10);
    }
    else
    {
        xTimerStart(gTCPSendDataTimeoutTimers[clientID], 10);
    }
}
#endif

//得到已发送完数据、等待响应的item号
static uint8_t get_item_number(uint8_t clientID)
{
    for (int i = 0; i < g_modbusTCP_cfg.listNum; i++)
    {
        if (g_modbusTCP_cfg.listPtr[i].client_id == clientID)
        {
            if (g_modbusTCP_cfg.listPtr[i].execState == MODBUSTCP_WAIT_RESP)
            {
                return i;
            }
        }
    }
    return 0xFF;
}

/*
 读寄存器返回  02 03 06 00 0B 00 16 00 21 B1 98
 读线圈返回       02 01 01 03 11 CD
*/
static void modbusTcpHandleReceivedData(uint8_t *data, uint16_t len, uint8_t clientID)
{
    stop_tcp_send_timeout_timer(clientID);
    LOGI(TAG, "Enter %s(), len = %u, clientID = %u, funcCode = 0x%X", __func__, len, clientID, g_modbusTCP_cfg.listPtr[clientID].funcCode);
#if 0
    if (len > 128)
    {
        hexdump(data, 128);
    }
    else
    {
        hexdump(data, len);
    }
#endif
#if 0
    uint8_t sNum = get_item_number(clientID);
#else
    uint8_t sNum = g_modbusTCP_cfg.index[clientID];
#endif
    if (sNum == 0xFF) // 异常处理
    {
        g_modbusTCP_cfg.listPtr[sNum].execState = MODBUSTCP_SLAVE_NO_RESP;
        *g_modbusTCP_cfg.listPtr[sNum].execResult = MODBUSTCP_SLAVE_NO_RESP;
        return;
    }
    g_modbusTCP_cfg.listPtr[sNum].execState = MODBUSTCP_SLAVE_HAD_RESP;
    *g_modbusTCP_cfg.listPtr[sNum].execResult = MODBUSTCP_SLAVE_HAD_RESP;
    if (g_modbusTCP_cfg.listPtr[sNum].funcCode == MB_WRITE_MULTIPLE_REGISTERS ||
            g_modbusTCP_cfg.listPtr[sNum].funcCode == MB_WRITE_MULTIPLE_COILS ||
            g_modbusTCP_cfg.listPtr[sNum].funcCode == MB_WRITE_REGISTER ||
            g_modbusTCP_cfg.listPtr[sNum].funcCode == MB_WRITE_SINGLE_COIL) // 写数据的返回
    {
        return;
    }
    uint8_t *aBuf = data + MB_TCP_UID;
    int n;
    if (aBuf[1] == 3)//读寄存器返回
    {
        n = aBuf[2] / 2; // 读取的元件个数
    }
    else if (aBuf[1] == 1 || aBuf[1] == 2)//读线圈返回
    {
        n = aBuf[2];
    }
    else
    {
        LOGE(TAG, "We do not know this function code: 0x%X, so just return!!!", aBuf[1]);
        return;
    }

    uint8_t *pBuf = aBuf + 3;
    for (int i = 0; i < n; i++)
    {
        if (aBuf[1] == 3)
        {
            //gMBCLinkSend[clientID].resultData[i] = GET_BIGPU16_DATA(pBuf);
            g_modbusTCP_cfg.listPtr[sNum].masterBuf[i] = GET_BIGPU16_DATA(pBuf);
            pBuf++;
            pBuf++;
        }
        else
        {
            //gMBCLinkSend[clientID].resultData[i] = GET_PU8_DATA(pBuf);
            g_modbusTCP_cfg.listPtr[sNum].masterBuf[i] = GET_PU8_DATA(pBuf);
            pBuf++;
        }
    }
}

static void modbus_tcp_send_data(uint8_t *pBuff, uint16_t len, uint8_t sNum)
{
    LOGV(TAG, "Enter %s(), len = %u, sNum = %u", __func__, len, sNum);
#if 0
    if (len > 128)
    {
        hexdump(pBuff, 128);
    }
    else
    {
        hexdump(pBuff, len);
    }
#endif
//    uint16_t ti = rand();
    uint16_t ti = (uint16_t)GET_1MS_TICKS_COUNT();
    gModbusTCPSendBuf[0] = ti >> 8;  // Transaction Identifier - 2 Byte
    gModbusTCPSendBuf[1] = ti & 0xFF;// Transaction Identifier - 2 Byte
    gModbusTCPSendBuf[2] = 0; // Protocol Identifier - 2 Byte
    gModbusTCPSendBuf[3] = 0; // Protocol Identifier - 2 Byte
    len -= 2;
    gModbusTCPSendBuf[4] = len >> 8;     //Number of bytes - 2 Byte
    gModbusTCPSendBuf[5] = len & 0x00FF; //Number of bytes - 2 Byte
    memcpy(gModbusTCPSendBuf + 6, pBuff, len); // Delete CRC
    uint16_t sendLen = len + 6;
#if 0
    if (sendLen > 128)
    {
        hexdump(gModbusTCPSendBuf, 128);
    }
    else
    {
        hexdump(gModbusTCPSendBuf, sendLen);
    }
#endif
    netconn_write(g_modbusTCP_cfg.listPtr[sNum].conn, gModbusTCPSendBuf, sendLen, NETCONN_COPY);

    start_tcp_send_timeout_timer(g_modbusTCP_cfg.listPtr[sNum].client_id);
}

static bool ifTriggered(modbus_tcp_item_st *pModbusItem)
{
    //LOGV(TAG, "Enter %s(), triggerType = %u, triggerAddr = %u", __func__, pModbusItem->triggerType, pModbusItem->triggerAddr);
    if (pModbusItem->commType == 0)
    {
        return true;
    }
    if (pModbusItem->triggerType == 1) // M element
    {
        if (plc_get_bit_element_value(M_ELEMENT, pModbusItem->triggerAddr))
        {
            return true;
        }
    }
    else // S element
    {
        if (plc_get_bit_element_value(S_ELEMENT, pModbusItem->triggerAddr))
        {
            return true;
        }
    }
    return false;
}

#if 0
static int32_t send_data(uint32_t sNum)
{
    LOGV(TAG, "Enter %s(), sNum = %u", __func__, sNum);
    uint16_t lsv_TxCrc;
    uint16_t lsv_TxLength, lsv_RxLength;
    uint8_t *lcp_UartTxBuff;
    int i, j;

    modbus_tcp_item_st *pModBusItem = &g_modbusTCP_cfg.listPtr[sNum];
    lcp_UartTxBuff = gTcpSendBuf;
    switch(pModBusItem->funcCode)
    {
    case MB_READ_COILS_STATUS:
    case MB_READ_DESCRETE_INPUT_STATUS:
        lcp_UartTxBuff[0] = 1;//pModBusItem->slaveRegAddr;
        lcp_UartTxBuff[1] = pModBusItem->funcCode;
        lcp_UartTxBuff[2] = (unsigned char)(pModBusItem->slaveRegAddr >> 8);
        lcp_UartTxBuff[3] = (unsigned char)pModBusItem->slaveRegAddr;
        lcp_UartTxBuff[4] = (unsigned char)(pModBusItem->elementCnt >> 8);
        lcp_UartTxBuff[5] = (unsigned char)pModBusItem->elementCnt;
        lsv_TxLength = 6;

        if(pModBusItem->elementCnt % 8)
        {
            lsv_RxLength = (pModBusItem->elementCnt >> 3) + 1 + 5;
        }
        else
        {
            lsv_RxLength = (pModBusItem->elementCnt >> 3) + 5;
        }

        /*接收缓冲区越界检查*/
        if(pModBusItem->masterBuf + lsv_RxLength > pModBusItem->masterBufBoundary)
        {
            *(pModBusItem->execResult) = MODLINK_PARA_ERR;
            LOGE(TAG, "Leave %s()...008", __func__);
            //mbclink_save_Error_D2(ltp_RunEnv->mcp_PC, MBCLINK_ERROR_BUFFER_OVER);
            return ERR_OVER_ELEMENT_RANG;
        }
        break;

    case MB_READ_HOLDING_REGISTER:
    case MB_READ_MULTIPLE_INPUT_REGISTER:
        lcp_UartTxBuff[0] = 2;//pModBusItem->mcv_SlaveAddr;
        lcp_UartTxBuff[1] = pModBusItem->funcCode;
        lcp_UartTxBuff[2] = (unsigned char)(pModBusItem->slaveRegAddr >> 8);
        lcp_UartTxBuff[3] = (unsigned char)pModBusItem->slaveRegAddr;
        lcp_UartTxBuff[4] = (unsigned char)(pModBusItem->elementCnt >> 8);
        lcp_UartTxBuff[5] = (unsigned char)pModBusItem->elementCnt;
        lsv_TxLength = 6;

        lsv_RxLength = (pModBusItem->elementCnt << 1) + 5;

        /*接收缓冲区越界检查*/
        if(pModBusItem->masterBuf + lsv_RxLength > pModBusItem->masterBufBoundary)
        {
            *(pModBusItem->execResult) = MODLINK_PARA_ERR;
            LOGE(TAG, "Leave %s()...009", __func__);
            //mbclink_save_Error_D2(ltp_RunEnv->mcp_PC, MBCLINK_ERROR_BUFFER_OVER2);
            return ERR_OVER_ELEMENT_RANG;
        }
        break;

    case MB_WRITE_SINGLE_COIL:
        lcp_UartTxBuff[0] = 3;//pModBusItem->mcv_SlaveAddr;
        lcp_UartTxBuff[1] = pModBusItem->funcCode;
        lcp_UartTxBuff[2] = (unsigned char)(pModBusItem->slaveRegAddr >> 8);
        lcp_UartTxBuff[3] = (unsigned char)pModBusItem->slaveRegAddr;
        lcp_UartTxBuff[4] = 0x00;
        lcp_UartTxBuff[5] = 0x00;
        if(pModBusItem->elementCnt > 0)
        {
            if (pModBusItem->masterBuf[0] != 0)
            {
                lcp_UartTxBuff[4] = 0xFF;
                lcp_UartTxBuff[5] = 0x00;
            }
        }
        lsv_TxLength = 6;
        lsv_RxLength = 8;
        break;

    case MB_WRITE_REGISTER:
        lcp_UartTxBuff[0] = 4;//pModBusItem->mcv_SlaveAddr;
        lcp_UartTxBuff[1] = pModBusItem->funcCode;
        lcp_UartTxBuff[2] = (unsigned char)(pModBusItem->slaveRegAddr >> 8);
        lcp_UartTxBuff[3] = (unsigned char)pModBusItem->slaveRegAddr;
        lcp_UartTxBuff[4] = (unsigned char)(pModBusItem->elementCnt >> 8);
        lcp_UartTxBuff[5] = (unsigned char)pModBusItem->elementCnt;
        lsv_TxLength = 6;
        lsv_RxLength = 8;
        break;

    case MB_WRITE_MULTIPLE_COILS: // 02 0F 07 D0 00 03 01 00 0F 27
        lcp_UartTxBuff[0] = 5;//pModBusItem->mcv_SlaveAddr;
        lcp_UartTxBuff[1] = pModBusItem->funcCode;
        lcp_UartTxBuff[2] = (unsigned char)(pModBusItem->slaveRegAddr >> 8);
        lcp_UartTxBuff[3] = (unsigned char)pModBusItem->slaveRegAddr;
        lcp_UartTxBuff[4] = (unsigned char)(pModBusItem->elementCnt >> 8);
        lcp_UartTxBuff[5] = (unsigned char)pModBusItem->elementCnt;

        if(pModBusItem->elementCnt % 8)
        {
            lsv_TxLength = (pModBusItem->elementCnt >> 3) + 1;
        }
        else
        {
            lsv_TxLength = (pModBusItem->elementCnt >> 3);
        }

        lcp_UartTxBuff[6] = lsv_TxLength;

        /*发送数据缓冲区越界检查*/
        if(pModBusItem->masterBuf + lsv_TxLength > pModBusItem->masterBufBoundary)
        {
            *(pModBusItem->execResult) = MODLINK_PARA_ERR;
            LOGE(TAG, "Leave %s()...010", __func__);
            //mbclink_save_Error_D2(ltp_RunEnv->mcp_PC, MBCLINK_ERROR_BUFFER_OVER3);
            return ERR_OVER_ELEMENT_RANG;
        }

        lsv_TxLength += 7;
        j = 0;
        for(i = 7; i < lsv_TxLength; i += 2)
        {
            lcp_UartTxBuff[i] = (unsigned char)pModBusItem->masterBuf[j];
            lcp_UartTxBuff[i + 1] = (unsigned char)(pModBusItem->masterBuf[j] >> 8);
            j++;
        }
        lsv_RxLength = 8;
        break;

    case MB_WRITE_MULTIPLE_REGISTERS:
        lcp_UartTxBuff[0] = 6;//pModBusItem->mcv_SlaveAddr;
        lcp_UartTxBuff[1] = pModBusItem->funcCode;
        lcp_UartTxBuff[2] = (unsigned char)(pModBusItem->slaveRegAddr >> 8);
        lcp_UartTxBuff[3] = (unsigned char)pModBusItem->slaveRegAddr;
        lcp_UartTxBuff[4] = (unsigned char)(pModBusItem->elementCnt >> 8);
        lcp_UartTxBuff[5] = (unsigned char)pModBusItem->elementCnt;

        lsv_TxLength = pModBusItem->elementCnt << 1;

        lcp_UartTxBuff[6] = lsv_TxLength;

        /*发送数据缓冲区越界检查*/
        if(pModBusItem->masterBuf + lsv_TxLength > pModBusItem->masterBufBoundary)
        {
            *(pModBusItem->execResult) = MODLINK_PARA_ERR;
            LOGE(TAG, "Leave %s()...011", __func__);
            //mbclink_save_Error_D2(ltp_RunEnv->mcp_PC, MBCLINK_ERROR_BUFFER_OVER4);
            return ERR_OVER_ELEMENT_RANG;
        }

        lsv_TxLength += 7;
        j = 0;
        for(i = 7; i < lsv_TxLength; i += 2)
        {
            lcp_UartTxBuff[i] = (unsigned char)(pModBusItem->masterBuf[j] >> 8);
            lcp_UartTxBuff[i + 1] = (unsigned char)pModBusItem->masterBuf[j];
            j++;
        }
        lsv_RxLength = 8;
        break;

    default:
        *(pModBusItem->execResult) = MODLINK_PARA_ERR;
        LOGE(TAG, "Leave %s()...012", __func__);
        return -1;
    }
    lsv_TxCrc = calc_crc16(lcp_UartTxBuff, lsv_TxLength);
    lcp_UartTxBuff[lsv_TxLength + 1] = (unsigned char)(lsv_TxCrc >> 8);
    lcp_UartTxBuff[lsv_TxLength] = (unsigned char)lsv_TxCrc;

    //pModBusItem->timeOut = GET_1MS_TICKS_COUNT() + 5000;
    pModBusItem->execState = MODBUSTCP_WAIT_RESP;
    *(pModBusItem->execResult) = MODBUSTCP_WAIT_RESP;
    /* 发送数据 */
    modbus_tcp_send_data(lcp_UartTxBuff, lsv_TxLength + 2, sNum);
    return 0;
}
#else
static void next_index(uint32_t clientID)
{
/*
    for (int i = 0; i < g_modbusTCP_cfg.listNum; i++)
    {
        if (clientID == g_modbusTCP_cfg.listPtr[i].client_id)
        {
            if (i > g_modbusTCP_cfg.index[clientID])
            {
                g_modbusTCP_cfg.index[clientID] = i;
                return;
            }
        }
    }

    for (int i = 0; i < g_modbusTCP_cfg.listNum; i++)
    {
        if (clientID == g_modbusTCP_cfg.listPtr[i].client_id)
        {
            g_modbusTCP_cfg.index[clientID] = i;
            break;
        }
    }
*/
    for (int i = g_modbusTCP_cfg.index[clientID] + 1; i < g_modbusTCP_cfg.listNum; i++)
    {
        if (clientID == g_modbusTCP_cfg.listPtr[i].client_id)
        {
            g_modbusTCP_cfg.index[clientID] = i;
            return;
        }
    }

    for (int i = 0; i <= g_modbusTCP_cfg.index[clientID]; i++)
    {
        if (clientID == g_modbusTCP_cfg.listPtr[i].client_id)
        {
            g_modbusTCP_cfg.index[clientID] = i;
            break;
        }
    }

}

static int32_t send_data(modbus_tcp_item_st *pModBusItem, uint32_t clientID)
{
    //LOGV(TAG, "Enter %s(), sNum = %u, clientID = %u", __func__, g_modbusTCP_cfg.index[clientID], clientID);
    uint16_t lsv_TxCrc;
    uint16_t lsv_TxLength, lsv_RxLength;
    uint8_t *lcp_UartTxBuff;
    int i, j;

    if (MODBUSTCP_NOT_CONNECTED == pModBusItem->execState)
    {
        return pdPASS;
    }

    if (ifTriggered(pModBusItem) == false)
    {
        *(pModBusItem->execResult) = MODLINK_NOT_RUN;
        pModBusItem->isExec = 0;
        next_index(clientID);
        return pdPASS;
    }
    if (pModBusItem->isExec)
    {
        if (pModBusItem->execState == MODBUSTCP_WAIT_RESP)//继续等待
        {
            return pdPASS;
        }
        else if (pModBusItem->execState == MODBUSTCP_SLAVE_HAD_RESP ||
                 pModBusItem->execState == MODBUSTCP_SLAVE_NO_RESP)
        {
            pModBusItem->isExec = 0;
            next_index(clientID);
            return pdPASS;
        }
    }
    
    lcp_UartTxBuff = gTcpSendBuf;
    switch(pModBusItem->funcCode)
    {
    case MB_READ_COILS_STATUS:
    case MB_READ_DESCRETE_INPUT_STATUS:
        lcp_UartTxBuff[0] = 1;//pModBusItem->slaveRegAddr;
        lcp_UartTxBuff[1] = pModBusItem->funcCode;
        lcp_UartTxBuff[2] = (unsigned char)(pModBusItem->slaveRegAddr >> 8);
        lcp_UartTxBuff[3] = (unsigned char)pModBusItem->slaveRegAddr;
        lcp_UartTxBuff[4] = (unsigned char)(pModBusItem->elementCnt >> 8);
        lcp_UartTxBuff[5] = (unsigned char)pModBusItem->elementCnt;
        lsv_TxLength = 6;

        if(pModBusItem->elementCnt % 8)
        {
            lsv_RxLength = (pModBusItem->elementCnt >> 3) + 1 + 5;
        }
        else
        {
            lsv_RxLength = (pModBusItem->elementCnt >> 3) + 5;
        }

        /*接收缓冲区越界检查*/
        if(pModBusItem->masterBuf + lsv_RxLength > pModBusItem->masterBufBoundary)
        {
            *(pModBusItem->execResult) = MODLINK_PARA_ERR;
            LOGE(TAG, "Leave %s()...008", __func__);
            //mbclink_save_Error_D2(ltp_RunEnv->mcp_PC, MBCLINK_ERROR_BUFFER_OVER);
            return ERR_OVER_ELEMENT_RANG;
        }
        break;

    case MB_READ_HOLDING_REGISTER:
    case MB_READ_MULTIPLE_INPUT_REGISTER:
        lcp_UartTxBuff[0] = 2;//pModBusItem->mcv_SlaveAddr;
        lcp_UartTxBuff[1] = pModBusItem->funcCode;
        lcp_UartTxBuff[2] = (unsigned char)(pModBusItem->slaveRegAddr >> 8);
        lcp_UartTxBuff[3] = (unsigned char)pModBusItem->slaveRegAddr;
        lcp_UartTxBuff[4] = (unsigned char)(pModBusItem->elementCnt >> 8);
        lcp_UartTxBuff[5] = (unsigned char)pModBusItem->elementCnt;
        lsv_TxLength = 6;

        lsv_RxLength = (pModBusItem->elementCnt << 1) + 5;

        /*接收缓冲区越界检查*/
        if(pModBusItem->masterBuf + lsv_RxLength > pModBusItem->masterBufBoundary)
        {
            *(pModBusItem->execResult) = MODLINK_PARA_ERR;
            LOGE(TAG, "Leave %s()...009", __func__);
            //mbclink_save_Error_D2(ltp_RunEnv->mcp_PC, MBCLINK_ERROR_BUFFER_OVER2);
            return ERR_OVER_ELEMENT_RANG;
        }
        break;

    case MB_WRITE_SINGLE_COIL:
        lcp_UartTxBuff[0] = 3;//pModBusItem->mcv_SlaveAddr;
        lcp_UartTxBuff[1] = pModBusItem->funcCode;
        lcp_UartTxBuff[2] = (unsigned char)(pModBusItem->slaveRegAddr >> 8);
        lcp_UartTxBuff[3] = (unsigned char)pModBusItem->slaveRegAddr;
        lcp_UartTxBuff[4] = 0x00;
        lcp_UartTxBuff[5] = 0x00;
        if(pModBusItem->elementCnt > 0)
        {
            if (pModBusItem->masterBuf[0] != 0)
            {
                lcp_UartTxBuff[4] = 0xFF;
                lcp_UartTxBuff[5] = 0x00;
            }
        }
        lsv_TxLength = 6;
        lsv_RxLength = 8;
        break;

    case MB_WRITE_REGISTER:
        lcp_UartTxBuff[0] = 4;//pModBusItem->mcv_SlaveAddr;
        lcp_UartTxBuff[1] = pModBusItem->funcCode;
        lcp_UartTxBuff[2] = (unsigned char)(pModBusItem->slaveRegAddr >> 8);
        lcp_UartTxBuff[3] = (unsigned char)pModBusItem->slaveRegAddr;
    //    lcp_UartTxBuff[4] = (unsigned char)(pModBusItem->elementCnt >> 8);
    //    lcp_UartTxBuff[5] = (unsigned char)pModBusItem->elementCnt;
        lcp_UartTxBuff[4] = (unsigned char)(pModBusItem->masterBuf[0] >> 8);
        lcp_UartTxBuff[5] = (unsigned char)pModBusItem->masterBuf[0];
        lsv_TxLength = 6;
        lsv_RxLength = 8;
        break;

    case MB_WRITE_MULTIPLE_COILS: // 02 0F 07 D0 00 03 01 00 0F 27
        lcp_UartTxBuff[0] = 5;//pModBusItem->mcv_SlaveAddr;
        lcp_UartTxBuff[1] = pModBusItem->funcCode;
        lcp_UartTxBuff[2] = (unsigned char)(pModBusItem->slaveRegAddr >> 8);
        lcp_UartTxBuff[3] = (unsigned char)pModBusItem->slaveRegAddr;
        lcp_UartTxBuff[4] = (unsigned char)(pModBusItem->elementCnt >> 8);
        lcp_UartTxBuff[5] = (unsigned char)pModBusItem->elementCnt;

        if(pModBusItem->elementCnt % 8)
        {
            lsv_TxLength = (pModBusItem->elementCnt >> 3) + 1;
        }
        else
        {
            lsv_TxLength = (pModBusItem->elementCnt >> 3);
        }

        lcp_UartTxBuff[6] = lsv_TxLength;

        /*发送数据缓冲区越界检查*/
        if(pModBusItem->masterBuf + lsv_TxLength > pModBusItem->masterBufBoundary)
        {
            *(pModBusItem->execResult) = MODLINK_PARA_ERR;
            LOGE(TAG, "Leave %s()...010", __func__);
            //mbclink_save_Error_D2(ltp_RunEnv->mcp_PC, MBCLINK_ERROR_BUFFER_OVER3);
            return ERR_OVER_ELEMENT_RANG;
        }

        lsv_TxLength += 7;
        j = 0;
        for(i = 7; i < lsv_TxLength; i += 2)
        {
            lcp_UartTxBuff[i] = (unsigned char)pModBusItem->masterBuf[j];
            lcp_UartTxBuff[i + 1] = (unsigned char)(pModBusItem->masterBuf[j] >> 8);
            j++;
        }
        lsv_RxLength = 8;
        break;

    case MB_WRITE_MULTIPLE_REGISTERS:
        lcp_UartTxBuff[0] = 6;//pModBusItem->mcv_SlaveAddr;
        lcp_UartTxBuff[1] = pModBusItem->funcCode;
        lcp_UartTxBuff[2] = (unsigned char)(pModBusItem->slaveRegAddr >> 8);
        lcp_UartTxBuff[3] = (unsigned char)pModBusItem->slaveRegAddr;
        lcp_UartTxBuff[4] = (unsigned char)(pModBusItem->elementCnt >> 8);
        lcp_UartTxBuff[5] = (unsigned char)pModBusItem->elementCnt;

        lsv_TxLength = pModBusItem->elementCnt << 1;

        lcp_UartTxBuff[6] = lsv_TxLength;

        /*发送数据缓冲区越界检查*/
        if(pModBusItem->masterBuf + lsv_TxLength > pModBusItem->masterBufBoundary)
        {
            *(pModBusItem->execResult) = MODLINK_PARA_ERR;
            LOGE(TAG, "Leave %s()...011", __func__);
            //mbclink_save_Error_D2(ltp_RunEnv->mcp_PC, MBCLINK_ERROR_BUFFER_OVER4);
            return ERR_OVER_ELEMENT_RANG;
        }

        lsv_TxLength += 7;
        j = 0;
        for(i = 7; i < lsv_TxLength; i += 2)
        {
            lcp_UartTxBuff[i] = (unsigned char)(pModBusItem->masterBuf[j] >> 8);
            lcp_UartTxBuff[i + 1] = (unsigned char)pModBusItem->masterBuf[j];
            j++;
        }
        lsv_RxLength = 8;
        break;

    default:
        *(pModBusItem->execResult) = MODLINK_PARA_ERR;
        LOGE(TAG, "Leave %s()...012", __func__);
        return -1;
    }
    lsv_TxCrc = calc_crc16(lcp_UartTxBuff, lsv_TxLength);
    lcp_UartTxBuff[lsv_TxLength + 1] = (unsigned char)(lsv_TxCrc >> 8);
    lcp_UartTxBuff[lsv_TxLength] = (unsigned char)lsv_TxCrc;

    //pModBusItem->timeOut = GET_1MS_TICKS_COUNT() + 5000;
    pModBusItem->execState = MODBUSTCP_WAIT_RESP;
    *(pModBusItem->execResult) = MODBUSTCP_WAIT_RESP;
    pModBusItem->isExec = 1;
    /* 发送数据 */
    modbus_tcp_send_data(lcp_UartTxBuff, lsv_TxLength + 2, g_modbusTCP_cfg.index[clientID]);
    return 0;
}
#endif

#if 1
static void modbus_send_task(void *arg)
{
    LOGV(TAG, "Enter %s()", __func__);

    modbus_tcp_item_st *pModbusItem;
    while (1)
    {

        /* 不同的连接5ms发一包，同一连接10ms发一包 */
        for (uint32_t clientID = 0; clientID < g_modbusTCP_cfg.clientNum; clientID++)
        {        
            if (gGUHUAing == 1 || gSuspendFlag == true)
            {
                LOGE(TAG, "Enter %s(),GUHUAing...........vTaskDelay(1000)", __func__);
                vTaskDelay(1000);
                continue;
            }
            uint16_t curIndex = g_modbusTCP_cfg.index[clientID];
            pModbusItem = &g_modbusTCP_cfg.listPtr[curIndex];
            //从send_data移植而来，这种情况无需Delay，可以提升发送效率
            if (pModbusItem->isExec)
            {
                if (pModbusItem->execState == MODBUSTCP_SLAVE_HAD_RESP ||
                   pModbusItem->execState == MODBUSTCP_SLAVE_NO_RESP)
                {
                    pModbusItem->isExec = 0;
                    next_index(clientID);
                    continue;
                }
            }
            send_data(pModbusItem, clientID);
            vTaskDelay(1);
        }
//      vTaskDelay(5);
    }
}
#elif 0
static void modbus_send_task(void *arg)
{
    LOGV(TAG, "Enter %s()", __func__);

    modbus_tcp_item_st *pModbusItem;
    while (1)
    {
        for (uint32_t i = 0; i < g_modbusTCP_cfg.listNum; i++)
        {
            pModbusItem = &g_modbusTCP_cfg.listPtr[i];
            //LOGI(TAG, "commType = %u", pModbusItem->commType);
            if (ifTriggered(pModbusItem) == true)
            {
                if (pModbusItem->execState == MODBUSTCP_SLAVE_HAD_RESP ||
                     pModbusItem->execState == MODBUSTCP_SLAVE_NO_RESP ||
                     pModbusItem->execState == MODBUSTCP_CONNECTED)
                {
                    send_data(i);
                }                
            }
            vTaskDelay(100);
        }
        vTaskDelay(1000);
    }
}
#else
/* 一条一条地发送 */
static void modbus_send_task(void *arg)
{
    LOGV(TAG, "Enter %s()", __func__);

    modbus_tcp_item_st *pModbusItem;
    while (1)
    {
        pModbusItem = &g_modbusTCP_cfg.listPtr[g_modbusTCP_cfg.index];
        if (ifTriggered(pModbusItem) == false)
        {
            g_modbusTCP_cfg.index++;
            if (g_modbusTCP_cfg.index >= g_modbusTCP_cfg.listNum)
            {
                g_modbusTCP_cfg.index = 0;
            }
            continue;
        }
        if (pModbusItem->execState == MODBUSTCP_WAIT_RESP)
        {
            vTaskDelay(100);
            continue;
        }
        if (pModbusItem->execState == MODBUSTCP_SLAVE_HAD_RESP ||
             pModbusItem->execState == MODBUSTCP_SLAVE_NO_RESP ||
             pModbusItem->execState == MODBUSTCP_CONNECTED)
        {
            g_modbusTCP_cfg.index++;
            if (g_modbusTCP_cfg.index >= g_modbusTCP_cfg.listNum)
            {
                g_modbusTCP_cfg.index = 0;
            }
            send_data(g_modbusTCP_cfg.index);
        }
        vTaskDelay(1000);
    }
}
#endif

static void start_modbus_send_task(void)
{
    LOGV(TAG, "Enter %s()", __func__);

    BaseType_t ret = xTaskCreate((TaskFunction_t)modbus_send_task,
                                 (const char *)"modbus_send_task",
                                 MODBUSTCPTBL_TASK_STACK_SIZE,
                                 NULL,
                                 MODBUSTCPTBL_TASK_PRIO,
                                 &gModbusSendHandle);
    if (ret != pdPASS)
    {
        LOGE(TAG, "Create modbus_send_task ERROR!");
    }
    LOGD(TAG, "Leave %s()", __func__);
}

static void set_execState(modbus_tcp_item_st *pModbusItem, uint8_t state)
{
    LOGV(TAG, "Enter %s()", __func__);
    for (int i = 0; i < g_modbusTCP_cfg.listNum; i++)
    {
        if (g_modbusTCP_cfg.listPtr[i].client_id == pModbusItem->client_id)
        {
            g_modbusTCP_cfg.listPtr[i].execState = state;
            *(g_modbusTCP_cfg.listPtr[i].execResult) = state;
        }
    }
}

static void set_Conn(uint8_t clientID, struct netconn *conn)
{
    for (int i = 0; i < g_modbusTCP_cfg.listNum; i++)
    {
        if (g_modbusTCP_cfg.listPtr[i].client_id == clientID)
        {
            g_modbusTCP_cfg.listPtr[i].conn = conn;
        }
    }
}

static void modbus_recv_task(void *arg)
{
    create_recv_task_st *pCRT = (create_recv_task_st *)arg;
    uint8_t clientID = pCRT->clientid;
    uint8_t sNum = pCRT->snum;
    vPortFree(pCRT);
    char mTAG[32];
    sprintf(mTAG, "modbus_recv_task%u", clientID);

    LOGD(mTAG, "Enter %s(), clientID = %u, sNum = %u", __func__, clientID, sNum);

    modbus_tcp_item_st *pModbusItem = &g_modbusTCP_cfg.listPtr[sNum];
    struct netbuf *buf;
    void *data;
    uint16_t len;
    err_t err;
    uint32_t tryCont = 0;
    uint16_t wanOrLan = pModbusItem->wanOrLan;//0：WAN， 1：LAN
    LOGD(mTAG, "wanOrLan = %u", wanOrLan);

    //pModbusItem->client_id = clientID;
    for (;;)
    {
        tryCont++;
        LOGD(mTAG, "Connecting to server : %s:%u......%u", ipaddr_ntoa(&pModbusItem->slaveIP), pModbusItem->slavePort, tryCont);
        /* Create a new connection identifier. */
        pModbusItem->conn = netconn_new(NETCONN_TCP);
        g_connModbus[clientID] = pModbusItem->conn;
        LOGV(mTAG, "clientID = %u, pModbusItem->conn = 0x%08X", clientID, pModbusItem->conn);
        set_Conn(clientID, pModbusItem->conn);
        if (pModbusItem->conn != NULL)
        {
            if (wanOrLan == 0)
            {
                LOGI(mTAG, "Let us bind : %s", ipaddr_ntoa(&g_plc_netcfg.wan.ip));
                netconn_bind(pModbusItem->conn, &g_plc_netcfg.wan.ip, 0);
            }
            else
            {
                LOGI(mTAG, "Let us bind : %s", ipaddr_ntoa(&g_plc_netcfg.lan.ip));
                netconn_bind(pModbusItem->conn, &g_plc_netcfg.lan.ip, 0);
            }
            /* Netconn connection to Server IP , port number 502. */
            err = netconn_connect(pModbusItem->conn, &pModbusItem->slaveIP, pModbusItem->slavePort);
            if(err == ERR_OK)
            {
                LOGD(mTAG, "Server: %s:%u connected success!", ipaddr_ntoa(&pModbusItem->slaveIP), pModbusItem->slavePort);
                set_execState(pModbusItem, MODBUSTCP_CONNECTED);
                while ((err = netconn_recv(pModbusItem->conn, &buf)) == ERR_OK)
                {
                    do
                    {
                        netbuf_data(buf, &data, &len);
                        modbusTcpHandleReceivedData(data, len, clientID);
                    }
                    while (netbuf_next(buf) >= 0);
                    netbuf_delete(buf);
                }
                LOGE(mTAG, "netconn_recv err = %d", err);
                netbuf_delete(buf);
                LOGE(mTAG, "Disconnected from server: %s:%u !!!", ipaddr_ntoa(&pModbusItem->slaveIP), pModbusItem->slavePort);
                set_execState(pModbusItem, MODBUSTCP_NOT_CONNECTED);
            }
            else
            {
                LOGE(mTAG, "Connect server: %s:%u ERROR!!, errNumber =%d , tryCont=%u", ipaddr_ntoa(&pModbusItem->slaveIP), pModbusItem->slavePort, err, tryCont);
                set_execState(pModbusItem, MODBUSTCP_NOT_CONNECTED);
            }
            netconn_close(pModbusItem->conn);
            netconn_delete(pModbusItem->conn);
            g_connModbus[clientID] = NULL;

            vTaskDelay(3000 / portTICK_PERIOD_MS);
        }
        else
        {
            LOGI(mTAG, "Can not create TCP connection. Let us try again.");
            vTaskDelay(4000 / portTICK_PERIOD_MS);
        }
    }

    vTaskDelete(NULL);
}

static void start_modbus_recv_task(uint32_t clientID, uint32_t sNum)
{
    LOGV(TAG, "Enter %s(), gModbusRecvTaskHandle[%u] = 0x%08X", __func__, clientID, gModbusRecvTaskHandle[clientID]);
    if (clientID >= MAX_MODBUS_TCP_ITEM)
    {
        return;
    }
    if (gModbusRecvTaskHandle[clientID] != NULL)
    {
        return;
    }
    char taskName[configMAX_TASK_NAME_LEN] = {0};
    sprintf(taskName, "modbus_recv_task%u", clientID);
    create_recv_task_st *pCRT = pvPortMalloc(sizeof(create_recv_task_st));
    pCRT->clientid = clientID;
    pCRT->snum = sNum;
    BaseType_t ret = xTaskCreate((TaskFunction_t)modbus_recv_task,
                                 (const char *)taskName,
                                 MODBUSTCPTBL_TASK_STACK_SIZE,
                                 (void *)pCRT,
                                 MODBUSTCPTBL_TASK_PRIO,
                                 &gModbusRecvTaskHandle[clientID]);
    if (ret != pdPASS)
    {
        LOGE(TAG, "Create %s ERROR!", taskName);
    }
}

static void modbus_connect_tcp_task(void *p_arg)
{
    LOGV(TAG, "Enter %s()", __func__);

    memset(g_connModbus, 0, sizeof(g_connModbus));
    memset(gModbusRecvTaskHandle, 0, sizeof(gModbusRecvTaskHandle));

    vTaskDelay(500);
    for (uint32_t i = 0; i < g_modbusTCP_cfg.listNum; i++)
    {
        if (i >= MAX_MODBUS_ITEM_NUM)
        {
            break;
        }
        start_modbus_recv_task(g_modbusTCP_cfg.listPtr[i].client_id, i);
        vTaskDelay(200);
    }

    // Set client number
    for (uint32_t i = 0; i < MAX_MODBUS_TCP_ITEM; i++)
    {
        if (gModbusRecvTaskHandle[i] == NULL)
        {
            break;
        }
        g_modbusTCP_cfg.clientNum++;
    }
    LOGW(TAG, "g_modbusTCP_cfg.clientNum = %u", g_modbusTCP_cfg.clientNum);

    // Set the current sending index
    for (uint8_t clientID = 0; clientID < g_modbusTCP_cfg.clientNum; clientID++)
    {
        for (uint16_t snum = 0; snum < g_modbusTCP_cfg.listNum; snum++)
        {
            if (g_modbusTCP_cfg.listPtr[snum].client_id == clientID)
            {
                g_modbusTCP_cfg.index[clientID] = snum;
                break;
            }
        }
    }
    LOGV(TAG, "Let us dump g_modbusTCP_cfg.index :");
    hexdump(g_modbusTCP_cfg.index, MAX_MODBUS_TCP_ITEM);

    start_modbus_send_task();
    LOGD(TAG, "Leave %s()", __func__);
    vTaskDelete(NULL);
}

void start_modbus_tcp(void)
{
    LOGV(TAG, "Enter %s(), g_modbusTCP_cfg.listNum = %u", __func__, g_modbusTCP_cfg.listNum);
    if (g_modbusTCP_cfg.listNum == 0)
    {
        return;
    }
#if ETHERCAT_SOEM == 1
    if (g_plc_netcfg.lan.ioExp == LAN_CONFIG_IO_EXP_ETHERCAT && gSoemConfig.nSubStationCount != 0 )
    {
         while ( !gSOEMInProcess )
         {
              LOGE(TAG, "start_modbus_tcp is waiting ethercat reach to opertational!!");
              vTaskDelay(1000);
         }
       LOGW(TAG, "start create modbusTCP task !!!");
    }
#endif
    vTaskDelay(1000);
    BaseType_t ret = xTaskCreate((TaskFunction_t)modbus_connect_tcp_task,
                                 (const char *)"MODBUS_TCP_CONNECT_TASK",
                                 MODBUSTCPTBL_TASK_STACK_SIZE,
                                 (void *)NULL,
                                 MODBUSTCPTBL_TASK_PRIO,
                                 NULL);
    if (ret != pdPASS)
    {
        LOGE(TAG, "Create modbus_connect_tcp_task ERROR!");
    }

    LOGD(TAG, "Leave %s()", __func__);
}

void stop_modbus_tcp(void)
{
    LOGV(TAG, "Enter %s()", __func__);
    if (gModbusSendHandle != NULL)
    {
        vTaskDelete(gModbusSendHandle);
        gModbusSendHandle = NULL;
    }
    for (uint32_t i = 0; i < MAX_MODBUS_TCP_ITEM; i++)
    {
        if (g_connModbus[i] != NULL)
        {
            netconn_close(g_connModbus[i]);
            netconn_delete(g_connModbus[i]);
            g_connModbus[i] = NULL;
            if (gModbusRecvTaskHandle[i] != NULL)
            {
                vTaskDelete(gModbusRecvTaskHandle[i]);
                gModbusRecvTaskHandle[i] = NULL;
            }
        }
    }
}

