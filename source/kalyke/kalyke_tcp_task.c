/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define TCP_SERVER_LOGIC_2      0

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "lwip/opt.h"
#include "lwip/api.h"
#include "lwip/apps/mqtt.h"
#include "lwip/dhcp.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/netifapi.h"
#include "lwip/prot/dhcp.h"
#include "lwip/tcpip.h"
#include "lwip/timeouts.h"
#include "netif/ethernet.h"
#include "enet_ethernetif.h"

#include "ctype.h"

#include "board.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"

#include "kalyke_opts.h"
#include "kalyke_internet_task.h"
#include "kalyke_event.h"

#include "bsp_led.h"
#include "plc_sysblock.h"
#include "plc_element.h"
#include "kalyke_monitor_task.h"
#include "kalyke_json.h"
#include "kalyke_tool.h"
#include "mb.h"
#include "verify_func.h"
#include "bsp_gpio.h"
#include "plc_modbustcpins.h"
#include "kalyke_tcp_task.h"
#include "plc_parseaddr.h"
#include "kalyke_4G_task.h"
#include "kalyke_4G_TCP_task.h"
#include "kalyke_version.h"
#include "bsp_uart.h"
#include "dev_sign_api.h"
#include "daisy_task.h"
#include "fsl_enet_mdio.h"
#include "fsl_phyksz8081.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void tcp_client_set_connected_bit(uint8_t clientID, uint8_t value);
static void tcp_client_set_connectting_bit(uint8_t clientID, uint8_t value);


/*******************************************************************************
 * Variables
 ******************************************************************************/
static const char *TAG = "KALYKE_TCP";
TaskHandle_t gKalykeTCPClientHandle[MAX_TCP_CONFIG_ITEM + 1];
static struct netconn *g_connJoanna[MAX_TCP_CONFIG_ITEM + 1];
static uint8_t *gMBCCONN_PC[MAX_TCP_CONFIG_ITEM];
static mbc_link_send_st gMBCLinkSend[MAX_TCP_CONFIG_ITEM];

static TimerHandle_t gTCPTimeoutTimer0 = NULL;
static TimerHandle_t gTCPTimeoutTimer1 = NULL;
static TimerHandle_t gTCPTimeoutTimer2 = NULL;
static TimerHandle_t gTCPConnectedTimer0 = NULL;
static TimerHandle_t gTCPConnectedTimer1 = NULL;
static TimerHandle_t gTCPConnectedTimer2 = NULL;
static TimerHandle_t gDisconnectTcpDelayTimer = NULL;

/*******************************************************************************
 * Code
 ******************************************************************************/


#if 0
void stop_tcp_client_all(void)
{
    LOGV("internet", "Enter %s()", __func__);
    for (uint8_t i = 0; i < MAX_TCP_CONFIG_ITEM; i++)
    {
        if (g_connClient[i] != NULL)
        {
            tcp_client_set_connected_bit(i, 0);
            tcp_client_set_connectting_bit(i, 0);
            netconn_close(g_connClient[i]);
            netconn_delete(g_connClient[i]);
            g_connClient[i] = NULL;
            if (gKalykeTCPClientHandle[i] != NULL)
            {
                vTaskDelete(gKalykeTCPClientHandle[i]);
                gKalykeTCPClientHandle[i] = NULL;
            }
        }
    }
}
#endif

/*
 val=0 : sending data
*/
void mbclink_save_Done_D1(uint8_t *pc, uint8_t val)
{
    save_char(pc + 14, &val, 0, 1);
}

/* Error:错误代码 */
void mbclink_save_Error_D2(uint8_t *pc, uint16_t val)
{
    save_word(pc + 18, &val, 0, 1);
}

static void tcpSendTimeoutCB(TimerHandle_t ltv_TimeHandle)
{
    uint8_t clientID = (unsigned int)pvTimerGetTimerID(ltv_TimeHandle);

    LOGE(TAG, "Enter %s(), clientID = %u", __func__, clientID);
    mbclink_save_Error_D2(gMBCLinkSend[clientID].pc, MBCLINK_ERROR_TIME_OUT);
    mbclink_save_Done_D1(gMBCLinkSend[clientID].pc, 1);
}

static void stop_tcp_send_timeout_timer_joanna(uint8_t clientID)
{
    if (clientID == 0)
    {
        xTimerStop(gTCPTimeoutTimer0, 1);
    }
    else if (clientID == 1)
    {
        xTimerStop(gTCPTimeoutTimer1, 1);
    }
    else if (clientID == 2)
    {
        xTimerStop(gTCPTimeoutTimer2, 10);
    }
}

static void start_tcp_send_timeout_timer_joanna(uint8_t clientID)
{
    if (clientID == 0)
    {
        if (gTCPTimeoutTimer0 == NULL)
        {
            gTCPTimeoutTimer0 = xTimerCreate((const char *)"tcpSendTimeOut",
                                           (TickType_t  )5000 / portTICK_PERIOD_MS,
                                           (UBaseType_t )pdFALSE,
                                           (void *      )(uint32_t)clientID,
                                           (TimerCallbackFunction_t)tcpSendTimeoutCB);
            xTimerStart(gTCPTimeoutTimer0, 10);
        }
        else
        {
            xTimerStart(gTCPTimeoutTimer0, 10);
        }
    }
    else if (clientID == 1)
    {
        if (gTCPTimeoutTimer1 == NULL)
        {
            gTCPTimeoutTimer1 = xTimerCreate((const char *)"tcpSendTimeOut",
                                           (TickType_t  )5000 / portTICK_PERIOD_MS,
                                           (UBaseType_t )pdFALSE,
                                           (void *      )(uint32_t)clientID,
                                           (TimerCallbackFunction_t)tcpSendTimeoutCB);
            xTimerStart(gTCPTimeoutTimer1, 10);
        }
        else
        {
            xTimerStart(gTCPTimeoutTimer1, 10);
        }
    }
    else if (clientID == 2)
    {
        if (gTCPTimeoutTimer2 == NULL)
        {
            gTCPTimeoutTimer2 = xTimerCreate((const char *)"tcpSendTimeOut",
                                           (TickType_t  )5000 / portTICK_PERIOD_MS,
                                           (UBaseType_t )pdFALSE,
                                           (void *      )(uint32_t)clientID,
                                           (TimerCallbackFunction_t)tcpSendTimeoutCB);
            xTimerStart(gTCPTimeoutTimer2, 10);
        }
        else
        {
            xTimerStart(gTCPTimeoutTimer2, 10);
        }
    }
}

void joanna_tcp_send(uint8_t *pBuff, uint16_t len, uint8_t clientID, uint8_t *ucode_pc, uint8_t flag, uint8_t *recvBuf)
{
    //kalyke_set_LAN_default();
    LOGV(TAG, "Enter %s(), clientID = %u", __func__, clientID);
    gMBCLinkSend[clientID].pc = ucode_pc;
    gMBCLinkSend[clientID].flag = flag;
    gMBCLinkSend[clientID].resultData = recvBuf;
    if (len > 128)
    {
        hexdump(pBuff, 128);
    }
    else
    {
        hexdump(pBuff, len);
    }

#if 0
    uint16_t ti = (uint16_t)GET_1MS_TICKS_COUNT();
    gTCPSendBuf[0] = ti >> 8;  // Transaction Identifier - 2 Byte
    gTCPSendBuf[1] = ti & 0xFF;// Transaction Identifier - 2 Byte
    gTCPSendBuf[2] = 0; // Protocol Identifier - 2 Byte
    gTCPSendBuf[3] = 0; // Protocol Identifier - 2 Byte
    len -= 2;
    gTCPSendBuf[4] = len >> 8;     //Number of bytes - 2 Byte
    gTCPSendBuf[5] = len & 0x00FF; //Number of bytes - 2 Byte

    memcpy(gTCPSendBuf + 6, pBuff, len); // Delete CRC
    uint16_t sendLen = len + 6;
    if (sendLen > 128)
    {
        hexdump(gTCPSendBuf, 128);
    }
    else
    {
        hexdump(gTCPSendBuf, sendLen);
    }
#endif
    //netconn_write(g_connJoanna[clientID], gTCPSendBuf, len, NETCONN_COPY);
    netconn_write(g_connJoanna[clientID], pBuff, len, NETCONN_COPY);

    //start_tcp_send_timeout_timer_joanna(clientID);
}

static void tcpConnectedCB(TimerHandle_t ltv_TimeHandle)
{
    uint8_t clientID = (unsigned int)pvTimerGetTimerID(ltv_TimeHandle);
    LOGD(TAG, "Enter %s(), clientID = %u", __func__, clientID);
    tcp_client_set_connected_bit(clientID, 1);
    tcp_client_set_connectting_bit(clientID, 0);
}

static void start_tcp_connected_timer(uint8_t clientID)
{
    if (clientID == 0)
    {
        if (gTCPConnectedTimer0 == NULL)
        {
            gTCPConnectedTimer0 = xTimerCreate((const char *)"tcpConnected",
                                           (TickType_t  )1000 / portTICK_PERIOD_MS,
                                           (UBaseType_t )pdFALSE,
                                           (void *      )(uint32_t)clientID,
                                           (TimerCallbackFunction_t)tcpConnectedCB);
            xTimerStart(gTCPConnectedTimer0, 100);
        }
        else
        {
            xTimerStart(gTCPConnectedTimer0, 100);
        }
    }
    else if (clientID == 1)
    {
        if (gTCPConnectedTimer1 == NULL)
        {
            gTCPConnectedTimer1 = xTimerCreate((const char *)"tcpConnected",
                                           (TickType_t  )1000 / portTICK_PERIOD_MS,
                                           (UBaseType_t )pdFALSE,
                                           (void *      )(uint32_t)clientID,
                                           (TimerCallbackFunction_t)tcpConnectedCB);
            xTimerStart(gTCPConnectedTimer1, 100);
        }
        else
        {
            xTimerStart(gTCPConnectedTimer1, 100);
        }
    }
    else if (clientID == 2)
    {
        if (gTCPConnectedTimer2 == NULL)
        {
            gTCPConnectedTimer2 = xTimerCreate((const char *)"tcpConnected",
                                           (TickType_t  )1000 / portTICK_PERIOD_MS,
                                           (UBaseType_t )pdFALSE,
                                           (void *      )(uint32_t)clientID,
                                           (TimerCallbackFunction_t)tcpConnectedCB);
            xTimerStart(gTCPConnectedTimer2, 100);
        }
        else
        {
            xTimerStart(gTCPConnectedTimer2, 100);
        }
    }
}

#if 0
/*
 读寄存器返回  02 03 06 00 0B 00 16 00 21 B1 98
 读线圈返回       02 01 01 03 11 CD
*/
static void tcpHandleClientReceivedData(uint8_t *data, uint16_t len, uint8_t clientID)
{
    stop_tcp_send_timeout_timer_joanna(clientID);
    LOGI("internet", "Enter %s(), len = %u, clientID = %u, flag = %u", __func__, len, clientID, gMBCLinkSend[clientID].flag);
    if (len > 128)
    {
        hexdump(data, 128);
    }
    else
    {
        hexdump(data, len);
    }
    
    if (gMBCLinkSend[clientID].flag == 0) // 写数据的返回
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
        LOGE("internet", "We do not know this function code: 0x%X, so just return!!!", aBuf[1]);
        return;
    }
    uint8_t *pBuf = aBuf + 3;
    for (int i = 0; i < n; i++)
    {
        if (aBuf[1] == 3)
        {
            //stv_UartPortStatus[port].msp_FreeRxBuff[i] = GET_BIGPU16_DATA(pBuf);
            gMBCLinkSend[clientID].resultData[i] = GET_BIGPU16_DATA(pBuf);
            pBuf++;pBuf++;
        }
        else
        {
            //stv_UartPortStatus[port].msp_FreeRxBuff[i] = GET_PU8_DATA(pBuf);
            gMBCLinkSend[clientID].resultData[i] = GET_PU8_DATA(pBuf);
            pBuf++;
        }
    }
}
#else
static void tcpHandleClientReceivedData(uint8_t *data, uint16_t len, uint8_t clientID)
{
    //stop_tcp_send_timeout_timer_joanna(clientID);
    LOGI(TAG, "Enter %s(), len = %u, clientID = %u, flag = %u", __func__, len, clientID, gMBCLinkSend[clientID].flag);
    if (len > 128)
    {
        hexdump(data, 128);
    }
    else
    {
        hexdump(data, len);
    }
    memcpy(gMBCLinkSend[clientID].resultData, data, len);
    gMBCLinkSend[clientID].ifHaveData = 1;
    tcp_client_set_received_bit(clientID, 1);
}

#endif

static void tcp_client_thread(void *arg)
{
    uint8_t clientID = (uint32_t)arg;
    
    LOGD(TAG, "Enter %s(), clientID = %u", __func__, clientID);
    modbus_tcp_config_item_st *pTcpItem = &gModbusTcpConfig.item[clientID];

    struct netbuf *buf;
    void *data;
    u16_t len;
    err_t err;
    uint8_t tryCont = 0;
    uint16_t wanOrLan;//0：WAN， 1：LAN
    get_word_default(gMBCCONN_PC[clientID] + 14, &wanOrLan);
    LOGD(TAG, "wanOrLan = %u", wanOrLan);

    for(;;)
    {
        tcp_client_set_connectting_bit(clientID, 1);
        LOGD(TAG, "Connecting to server : %s:%u", ipaddr_ntoa(&pTcpItem->ipTarget), pTcpItem->portTarget);
        /* Create a new connection identifier. */
        g_connJoanna[clientID] = netconn_new(NETCONN_TCP);
        LOGV(TAG, "g_connJoanna[%u] = 0x%08X\r\n", clientID, g_connJoanna[clientID]);
        if (g_connJoanna[clientID] != NULL)
        {
            #if 1
            if (wanOrLan == 0)
            {
                //netconn_bind(g_connJoanna[clientID], &g_plc_netcfg.wan.ip, pTcpItem->portLocal);
                netconn_bind(g_connJoanna[clientID], &g_plc_netcfg.wan.ip, 0);
            }
            else
            {
                netconn_bind(g_connJoanna[clientID], &g_plc_netcfg.lan.ip, 0);
            }
            #endif
            /* Netconn connection to Server IP , port number 502. */
            err = netconn_connect(g_connJoanna[clientID], &pTcpItem->ipTarget, pTcpItem->portTarget);
            if(err == ERR_OK)
            {
                int16_t mD3 = err;
                save_word_default(gMBCCONN_PC[clientID] + 26, (uint16_t*)&mD3);
                start_tcp_connected_timer(clientID);
                LOGD(TAG, "Server: %s:%u connected success!", ipaddr_ntoa(&pTcpItem->ipTarget), pTcpItem->portTarget);
                while ((err = netconn_recv(g_connJoanna[clientID], &buf)) == ERR_OK)
                {
                    do
                    {
                        netbuf_data(buf, &data, &len);
                        tcpHandleClientReceivedData(data, len, clientID);
                    } while (netbuf_next(buf) >= 0);
                    netbuf_delete(buf);
                    mbclink_save_Error_D2(gMBCLinkSend[clientID].pc, MBCLINK_NO_ERROR);
                    mbclink_save_Done_D1(gMBCLinkSend[clientID].pc, 1);
                }
                LOGE(TAG, "err = %d", err);
                netbuf_delete(buf);
                LOGE(TAG, "Disconnected from server: %s:%u !!!", ipaddr_ntoa(&pTcpItem->ipTarget), pTcpItem->portTarget);
                vTaskDelay(3000 / portTICK_PERIOD_MS);
                tcp_client_set_connected_bit(clientID, 0);
                //tcp_client_set_connectting_bit(clientID, 0);
                netconn_close(g_connJoanna[clientID]);
                netconn_delete(g_connJoanna[clientID]);
                vTaskDelete(NULL);
                return;
            }
            LOGE(TAG, "Connect server: %s:%u ERROR!!,errNumber =%d , tryCont=%u", ipaddr_ntoa(&pTcpItem->ipTarget), pTcpItem->portTarget, err, tryCont);
            netconn_close(g_connJoanna[clientID]);
            netconn_delete(g_connJoanna[clientID]);

        #if 0
            vTaskDelay(3000 / portTICK_PERIOD_MS);
            tcp_client_set_connected_bit(clientID, 0);
            tcp_client_set_connectting_bit(clientID, 0);
            break;
        #else
            int16_t mD3 = err;
            save_word_default(gMBCCONN_PC[clientID] + 26, (uint16_t*)&mD3);
            tryCont++;
            if (tryCont >= 10)
            {
                tcp_client_set_connectting_bit(clientID, 0);
                break;
            }
            vTaskDelay(3000 / portTICK_PERIOD_MS);
        #endif
        }
        else // (g_connJoanna == NULL)
        {
            int16_t mD3 = MBCCONNECT_NO_LINK;
            save_word_default(gMBCCONN_PC[clientID] + 26, (uint16_t*)&mD3);
            tryCont++;
            if (tryCont >= 10)
            {
                tcp_client_set_connectting_bit(clientID, 0);
                break;
            }
            LOGI(TAG, "Can not create TCP connection.\r\n");
            vTaskDelay(4000 / portTICK_PERIOD_MS);
        }
    }

    vTaskDelete(NULL);
}

#define MAGIC_NUM    4

static void tcp_client_set_connected_bit(uint8_t clientID, uint8_t value)
{
    uint16_t element = SM271 + clientID * MAGIC_NUM;
    //LOGV(TAG, "Enter %s(), clientID = %u, element = %u, value = %u", __func__, clientID, element, value);
    plc_set_bit_element_value(SM_ELEMENT, element, value);
    save_char_default(gMBCCONN_PC[clientID] + 22, &value);
}

uint8_t tcp_client_get_connected_bit(uint8_t clientID)
{
    uint16_t element = SM271 + clientID * MAGIC_NUM;
    uint8_t value = plc_get_bit_element_value(SM_ELEMENT, element);
    //LOGV(TAG, "Enter %s(), clientID = %u, element = %u, value = %u", __func__, clientID, element, value);
    return value;
}

/* 0 <= clientID <= MAX_TCP_CONFIG_ITEM */
static void tcp_client_set_connectting_bit(uint8_t clientID, uint8_t value)
{
    uint16_t element = SM272 + clientID * MAGIC_NUM;
    //LOGI(TAG, "Enter %s(), clientID = %u, element = %u, value = %u", __func__, clientID, element, value);
    plc_set_bit_element_value(SM_ELEMENT, element, value);
    save_char_default(gMBCCONN_PC[clientID] + 18, &value);
}

uint8_t tcp_client_get_connectting_bit(uint8_t clientID)
{
    uint16_t element = SM272 + clientID * MAGIC_NUM;
    uint8_t value = plc_get_bit_element_value(SM_ELEMENT, element);
    //LOGI(TAG, "Enter %s(), clientID = %u, element = %u, value = %u", __func__, clientID, element, value);
    return value;
}

void tcp_client_set_received_bit(uint8_t clientID, uint8_t value)
{
    uint16_t element = SM273 + clientID * MAGIC_NUM;
    plc_set_bit_element_value(SM_ELEMENT, element, value);
}

uint8_t tcp_client_get_received_bit(uint8_t clientID)
{
    uint16_t element = SM273 + clientID * MAGIC_NUM;
    uint8_t value = plc_get_bit_element_value(SM_ELEMENT, element);
    return value;
}

void tcp_client_set_sending_bit(uint8_t clientID, uint8_t value)
{
    uint16_t element = SM274 + clientID * MAGIC_NUM;
    plc_set_bit_element_value(SM_ELEMENT, element, value);
}

uint8_t tcp_client_get_sending_bit(uint8_t clientID)
{
    uint16_t element = SM274 + clientID * MAGIC_NUM;
    uint8_t value = plc_get_bit_element_value(SM_ELEMENT, element);
    return value;
}


static void disconnectDelayTimerCB(TimerHandle_t ltv_TimeHandle)
{
    uint8_t clientID = (unsigned int)pvTimerGetTimerID(ltv_TimeHandle);

    LOGV(TAG, "Enter %s(), clientID = %u", __func__, clientID);
    
    tcp_client_set_connected_bit(clientID, 0);
    tcp_client_set_connectting_bit(clientID, 0);
}

void stop_tcp_client(uint8_t clientID)
{
    LOGV(TAG, "Enter %s(), clientID = %u", __func__, clientID);

    tcp_client_set_connectting_bit(clientID, 1);
    if (gKalykeTCPClientHandle[clientID] != NULL)
    {
        vTaskDelete(gKalykeTCPClientHandle[clientID]);
        gKalykeTCPClientHandle[clientID] = NULL;
    }
    if (g_connJoanna[clientID] != NULL)
    {
        err_t ret = netconn_close(g_connJoanna[clientID]);
        ret = netconn_delete(g_connJoanna[clientID]);
        g_connJoanna[clientID] = NULL;
    }
    if (gDisconnectTcpDelayTimer == NULL)
    {
        gDisconnectTcpDelayTimer = xTimerCreate((const char *)"stop_client",
                                       (TickType_t  )2019 / portTICK_PERIOD_MS,
                                       (UBaseType_t )pdFALSE,
                                       (void *      )(uint32_t)clientID,
                                       (TimerCallbackFunction_t)disconnectDelayTimerCB);
        xTimerStart(gDisconnectTcpDelayTimer, 100);
    }
    else
    {
        xTimerStart(gDisconnectTcpDelayTimer, 100);
    }
}

void start_tcp_client(uint8_t clientID, uint8_t *pc)
{
    gMBCCONN_PC[clientID] = pc;
    tcp_client_set_connectting_bit(clientID, 1);

    LOGV(TAG, "Enter %s(), clientID = %u", __func__, clientID);
    char taskName[configMAX_TASK_NAME_LEN] = {0};
    sprintf(taskName, "tcp_client%u", clientID);
    BaseType_t ret = xTaskCreate((TaskFunction_t)tcp_client_thread,
                          (const char *)taskName,
                          1024,
                          (void *)(uint32_t)clientID,
                          INTERNET_TASK_PRIO,
                          &gKalykeTCPClientHandle[clientID]);
    if (ret != pdPASS)
    {
        LOGE(TAG, "Create tcp_client_thread ERROR!  clientID = %u", clientID);
    }
}

