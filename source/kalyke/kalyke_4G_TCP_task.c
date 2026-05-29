/**
  ******************************************************************************
  * @file    kalyke_sntp_task.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-05-11
  * @brief   SNTP
  ******************************************************************************
  */
#include <stdio.h>
#include "kalyke_4G_TCP_task.h"
#include "kalyke_4G_task.h"

#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_snvs_hp.h"
#include "fsl_snvs_lp.h"
#include "kalyke_tool.h"
#include "fsl_lpuart_freertos.h"
#include "bsp_uart.h"
#include "kalyke_event.h"
#include "plc_netcfg.h"
#include "kalyke_internet_task.h"
#include "bsp_led.h"
#include "kalyke_monitor_task.h"
#include "kalyke_opts.h"
#include "verify_func.h"

#if (KALYKE_FEATURE_4G_TCP_TASK == 1)
#define DEBUG_4G_TCP

#ifdef DEBUG_4G_TCP
#define LOGE_4G    LOGE
#define LOGW_4G    LOGW
#define LOGI_4G    LOGI
#define LOGD_4G    LOGD
#define LOGV_4G    LOGV
#else
#define LOGE_4G(...)
#define LOGW_4G(...)
#define LOGI_4G(...)
#define LOGD_4G(...)
#define LOGV_4G(...)
#endif

#define AT_LOGIC    2

/*******************************************************************************
 * Prototypes
 ******************************************************************************/


/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Variables
 ******************************************************************************/
static const char *TAG = "4G_TCP";
static uint8_t gTCPContext = 2;
static uint8_t gTCPConnectID = 6;
QueueHandle_t g4GTCPQueue;
static TimerHandle_t g4GHeartTimer = NULL;
static TimerHandle_t g4GReConnectTimer = NULL;
/* 一旦uart4_callback_func函数收到+QIURC:，则暂停MQTT发送5s */
static TimerHandle_t g4GMutexTimer = NULL;
volatile bool g4GTCPBusy = false;
static uint8_t gTCPRecvBuf4G[1024];
static uint8_t gMODBUSTCPHead4G[16];
static uint8_t gTCPSendBuf4G[128];
static uint8_t gTCPResponseBuf4G[128];

static const char *QIOPENErrCodeStrings[] =
{
    "Unknown error",                 // 550
    "Operation blocked",             // 551
    "Invalid parameters",            // 552
    "Memory not enough",             // 553
    "Create socket failed",          // 554
    "Operation not supported",       // 555
    "Socket bind failed",            // 556
    "Socket listen failed",          // 557
    "Socket write failed",           // 558
    "Socket read failed",            // 559
    "Socket accept failed",          // 560
    "Open PDP context failed",       // 561
    "Close PDP context failed",      // 562
    "Socket identity has been used", // 563
    "DNS busy",                      // 564
    "DNS parse failed",              // 565
    "Socket connect failed",         // 566
    "Socket has been closed",        // 567
    "Operation busy",                // 568
    "Operation timeout",             // 569
    "PDP context broken down",       // 570
    "Cancel send",                   // 571
    "Operation not allowed",         // 572
    "APN not configured",            // 573
    "Port busy",                     // 574
};
/*******************************************************************************
 * Code
 ******************************************************************************/
static void showQIOPENErrCode(int errCode)
{
    if (errCode == 0)
    {
        LOGI(TAG, "0 : Operation successful");
    }
    else
    {
        if (errCode > 574)
        {
            LOGE(TAG, "We do not know this errCode : %u", errCode);
            return;
        }
        int idx = errCode - 550;
        LOGE(TAG, "%u : %s", errCode, QIOPENErrCodeStrings[idx]);
    }
}

static void setContextID(void)
{
    gTCPContext++;
    if (gTCPContext > 3)
    {
        gTCPContext = 1;
    }
}

static void setConnectID(void)
{
    gTCPConnectID++;
    if (gTCPConnectID > 11)
    {
        gTCPConnectID = 0;
    }
}

void kalyke_stop_4G_for_PLC(void)
{
    LOGV("4G_task", "Enter %s()", __func__);
    if (g_plc_netcfg.cloud.ifConnect == 0)
    {
        return;
    }
    if (bspIsHave4G() && g_plc_netcfg.surfing == 1)
    {
        at_QIDEACT();
        vTaskDelay(110);
    }
}

/*
  +QIACT: 1,1,1,"10.15.163.170"
  +QIACT: 2,1,1,"10.103.55.62"
  +QIACT: 3,1,1,"10.103.55.62"

  OK
*/
bool check_QIACT(void)
{
    LOGV(TAG, "Enter %s()", __func__);
    char sendBuf[16];
    for(;;)
    {
        sendAT("AT+QIACT?");
        int ret = waitResponse(2000, "+QIACT:", "OK", NULL, NULL, NULL);
        if (ret == 1)
        {
            break;
        }
        else if (ret == 2) // We only got 'OK', so there is no context active
        {
        }
        break;
    }
    return true;
}

void at_QIDEACT(void)
{
    LOGV(TAG, "Enter %s()", __func__);
#if 0
    sendAT("AT+QIDEACT=1");
    sendAT("AT+QIDEACT=2");
    sendAT("AT+QIDEACT=3");
#else
    char sendBuf[32];
    sprintf(sendBuf, "AT+QIDEACT=%u", gTCPContext);
    sendAT(sendBuf);
    int ret = waitResponse(40000, "OK", "ERROR", NULL, NULL, NULL);
    if (ret == 1)
    {
        LOGD(TAG, "QIDEACT success");
    }
    else if (ret == 2)
    {
        LOGE(TAG, "QIDEACT failed");
    }
#endif
}

static void at_QIACT(void)
{
    char sendBuf[16];
    LOGV(TAG, "Enter %s()", __func__);
    while(1)
    {
        if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
        {
            LOGW(TAG, "%s : xSemaphoreTake Fail, waiting...", __func__);
            vTaskDelay(5000);
            continue;
        }
        //setContextID();

        check_QIACT();

        sprintf(sendBuf, "AT+QIACT=%u", gTCPContext);
        sendAT(sendBuf);
        int ret = waitResponse(150000, "OK", "ERROR", NULL, NULL, NULL);
        if (ret == 1)
        {
            LOGV_4G(TAG, "QIACT SUCCESS!");
            //vTaskDelay(123);
            break;
        }
        else if (ret == 2)
        {
            LOGE_4G(TAG, "QIACT ERROR!");
        }
        xSemaphoreGive(g4GMutex);
        vTaskDelay(5000); // Try again.
    }
    xSemaphoreGive(g4GMutex);
    LOGV(TAG, "Leave %s()", __func__);
}

// +QIOPEN: 6,0
static int16_t getQIOPENResult(void)
{
    char *pBuf = (char *)gUart8RecvBuffer;
    char *pQIOPEN = strstr(pBuf, "+QIOPEN:");
    LOGV_4G(TAG, "pQIOPEN = %s", pQIOPEN);
    int connect_id;
    int errCode;
    char temp[16];
    sscanf(pQIOPEN, "%s %d,%d", temp, &connect_id, &errCode);

    LOGD_4G(TAG, "Leave %s(), errCode = %d", __func__, errCode);
    showQIOPENErrCode(errCode);
    return errCode;
}

static void at_QICLOSE(void)
{
    char *sendBuf = pvPortMalloc(128);
    while (1)
    {
        sprintf(sendBuf, "AT+QICLOSE=%u,1000", gTCPConnectID);
        sendAT(sendBuf);
        int ret = waitResponse(2000, "OK", "ERROR", NULL, NULL, NULL);
        if (ret == 1)
        {
            LOGI(TAG, "QICLOSE OK!");
        }
        else if (ret == 2)
        {
            LOGE(TAG, "QICLOSE ERROR!");
        }
        else
        {
            LOGE(TAG, "QICLOSE time out!");
        }
        break;
    }
    vPortFree(sendBuf);
}

static int8_t at_QIOPEN(void)
{
    int8_t retVal = 0;
    char *sendBuf = pvPortMalloc(512);

    while(1)
    {
        if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
        {
            LOGW(TAG, "%s : xSemaphoreTake Fail, waiting...", __func__);
            vTaskDelay(5000);
            continue;
        }
        //setConnectID();
        if (g_plc_netcfg.cloud.ipOrDomain == 1)
        {
            sprintf(sendBuf, "AT+QIOPEN=%u,%u,\"TCP\",\"%s\",%u,0,1", gTCPContext, gTCPConnectID, g_plc_netcfg.cloud.domain, g_plc_netcfg.cloud.port);
        }
        else
        {
            sprintf(sendBuf, "AT+QIOPEN=%u,%u,\"TCP\",\"%s\",%u,0,1", gTCPContext, gTCPConnectID, ipaddr_ntoa(&g_plc_netcfg.cloud.ip), g_plc_netcfg.cloud.port);
        }
        sendAT(sendBuf);
        int ret = waitResponse(3000, "OK\r\n\r\n+QIOPEN:", NULL, NULL, NULL, NULL);
        if (ret == 1)
        {
            int16_t result = getQIOPENResult();
            if (result == 0) // Opened successfully
            {
                LOGW_4G(TAG, "AT+QIOPEN OK!");
                retVal = 0;
            }
            else
            {
                LOGE_4G(TAG, "AT+QIOPEN FAILED!");
                if (result == 563)
                {
                    //setConnectID();
                    at_QICLOSE();
                }
                retVal = -1;
            }
        }
        else
        {
            LOGE_4G(TAG, "AT+QIOPEN Time out!");
            at_QICLOSE();
            retVal = -2;
        }
        break;
    }
    vPortFree(sendBuf);
    xSemaphoreGive(g4GMutex);
    return retVal;
}

static int waitResponseTCPSend(uint32_t timeout_ms,
                 char *r1, char *r2,
                 char *r3, char *r4, char *r5)
{
    LOGV_4G(TAG, "Enter %s()", __func__);
    char *pRecvBuf = (char*)gUart8RecvBuffer;
    int index = -1;
    TickType_t lastMS = xTaskGetTickCount();

    while (1)
    {
        vTaskDelay(1);
        if (gUartStatus == UART_RX)
        {
            continue;
        }
        if (r1 && strstr(pRecvBuf, r1))
        {
            index = 1;
            break;
        }
        else if (r2 && strstr(pRecvBuf, r2))
        {
            index = 2;
            break;
        }
        else if (r3 && strstr(pRecvBuf, r3))
        {
            index = 3;
            break;
        }
        else if (r4 && strstr(pRecvBuf, r4))
        {
            index = 4;
            break;
        }
        else if (r5 && strstr(pRecvBuf, r5))
        {
            index = 5;
            break;
        }
        else if (xTaskGetTickCount() - lastMS > timeout_ms)
        {
            index = 0;
            break;
        }
        vTaskDelay(5);
    }
    LOGD_4G(TAG, "Leave %s(), index = %d", __func__, index);
    return index;
}

static int8_t at_QISEND(uint8_t *pBuf, uint32_t len)
{
    char sendBuf[64];
    int8_t funRet = 0;
    while (1)
    {
        if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
        {
            LOGW(TAG, "%s : xSemaphoreTake Fail, return.", __func__);
            //LOGW(TAG, "%s : xSemaphoreTake Fail, waiting...", __func__);
            //vTaskDelay(998);
            //continue;
            return -1;
        }
        sprintf(sendBuf, "AT+QISEND=%u,%u\r\n", gTCPConnectID, len);
        //sendAT(sendBuf);
        init_uart8_receive_buffer_small();
        uart8_send_buffer(sendBuf, strlen(sendBuf));
        int retWait = waitResponseTCPSend(5000, ">", NULL, NULL, NULL, NULL);
        if (retWait == 1) // Got '>'
        {
            LOGV(TAG, "OMG, We got '>'...");
            init_uart8_receive_buffer();
            uart8_send_buffer((char *)pBuf, len);
            funRet = 0;
            break;
        }
        else
        {
            LOGW(TAG, "QISEND time out.");
            funRet = -2;
            break;
        }
    }
    xSemaphoreGive(g4GMutex);
    return funRet;
}

static void ec20_TCP_init(void)
{
    LOGV(TAG, "Enter %s()", __func__);
    isEC20ModuleOK();
    closeATEchoMode();
    isSIMCardReady();

    at_QIDEACT();
    at_QIACT();

    while (1)
    {
        if (at_QIOPEN() == 0)
        {
            break;
        }
        vTaskDelay(10000);
    }
}

static void login_fexlink(void)
{
    LOGV(TAG, "Enter %s()", __func__);

    uint16_t sendLen = 0;
    char tcpSendBuf[128] = {0};
    tcpSendBuf[0] = 0x00;
    tcpSendBuf[1] = 0x10;
    tcpSendBuf[2] = 0x24;
    tcpSendBuf[3] = 0x62;

    uint16_t len = 26;
    tcpSendBuf[4] = len & 0xFF;
    tcpSendBuf[5] = len >> 8;
    tcpSendBuf[6] = 0x01;
    tcpSendBuf[7] = 0x02;

    strcpy(&tcpSendBuf[8], "MiLink");
    //strcpy(&tcpSendBuf[8], "LiXink");

    memcpy(&tcpSendBuf[20], gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId, 12);

    sendLen = 32;
    while (1)
    {
        if (at_QISEND((uint8_t *)tcpSendBuf, sendLen) >= 0)
        {
            break;
        }
        vTaskDelay(2000);
    }
}

static void reconnect_fexlink(TimerHandle_t ltv_TimeHandle)
{
    LOGV_4G(TAG, "Enter %s()", __func__);
    mqtt_msg_st Msg;
    Msg.type = 5;
    xQueueSendToFront(g4GTCPQueue, &Msg, 0);
}

static void heart_fexlink(TimerHandle_t ltv_TimeHandle)
{
    LOGV_4G(TAG, "Enter %s()", __func__);
    mqtt_msg_st Msg;
    Msg.type = 4;
    xQueueSendToBack(g4GTCPQueue, &Msg, 0);
}

static void mutex_timer_cb(TimerHandle_t ltv_TimeHandle)
{
    LOGI(TAG, "Enter %s()", __func__);
    g4GTCPBusy = false;
}

void tcp_4G_start_busy_timer(void)
{
    LOGI(TAG, "Enter %s(), g4GTCPBusy = %u", __func__, g4GTCPBusy);
    if (g4GMutexTimer == NULL)
    {
        return;
    }
    if (g4GTCPBusy == true)
    {
        xTimerReset(g4GMutexTimer, 0);
    }
    else
    {
        g4GTCPBusy = true;
        xTimerStart(g4GMutexTimer, 0);
    }
}

static void MicroLink_4G_heart(void)
{
    uint16_t ti = gKalykeSecondTick;
    LOGV(TAG, "Enter %s()", __func__);
    char tcpSendBuf[16] = {0};

    tcpSendBuf[0] = ti >> 8;  // Transaction Identifier - 2 Byte
    tcpSendBuf[1] = ti & 0xFF;// Transaction Identifier - 2 Byte
    tcpSendBuf[2] = 0; // Protocol Identifier - 2 Byte
    tcpSendBuf[3] = 0; // Protocol Identifier - 2 Byte
    tcpSendBuf[4] = 0; // Number of bytes - 2 Byte
    tcpSendBuf[5] = 3; // Number of bytes - 2 Byte
    tcpSendBuf[6] = '{'; //0x7B
    tcpSendBuf[7] = '#'; //0x23
    tcpSendBuf[8] = '}'; //0x7D

    at_QISEND((uint8_t *)tcpSendBuf, 9);
}

static void MicroLink_4G_StartHeart(void)
{
    LOGV(TAG, "Enter %s()", __func__);
    if (g4GHeartTimer == NULL)
    {
        g4GHeartTimer = xTimerCreate((const char *)"login_4Glink",
                                       (TickType_t  )90000 / portTICK_PERIOD_MS,
                                       (UBaseType_t )pdTRUE,
                                       (void *      )3,
                                       (TimerCallbackFunction_t)heart_fexlink);
        xTimerStart(g4GHeartTimer, 0);
    }
    else
    {
        xTimerStart(g4GHeartTimer, 0);
    }
}

void reset_4G_heart_timer(void)
{
    if (g4GHeartTimer)
    {
        xTimerReset(g4GHeartTimer, 0);
    }
}

static void tcp_4G_client_send_buffer(unsigned char *pBuff, unsigned short len)
{
    LOGV_4G(TAG, "Enter %s()", __func__);
    if (len > 128)
    {
        hexdump(pBuff, 128);
    }
    else
    {
        hexdump(pBuff, len);
    }
    gTCPSendBuf4G[0] = gMODBUSTCPHead4G[0];
    gTCPSendBuf4G[1] = gMODBUSTCPHead4G[1];
    gTCPSendBuf4G[2] = gMODBUSTCPHead4G[2];
    gTCPSendBuf4G[3] = gMODBUSTCPHead4G[3];
    len -= 2; // Delete CRC
    gTCPSendBuf4G[4] = len >> 8;
    gTCPSendBuf4G[5] = len & 0x00FF;

    memcpy(gTCPSendBuf4G + 6, pBuff, len);
    uint16_t sendLen = len + 6;

    at_QISEND(gTCPSendBuf4G, sendLen);
}

/*
    +QIURC: "recv",<connectID>,<length><CR><LF><data>
    +QIURC: "recv",0,49

    0D 0A 2B 51 49 55 52 43 3A 20 22 72 65 63 76 22
    2C 30 2C 34 39 0D 0A 00 10 24 62 00 2B 01 02 00
    00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    00 00 00 00 00 00 00 00 0D 0A
*/
static int32_t getRealTCPData(char *at, char *pRecvData)
{
    LOGV_4G(TAG, "Enter %s()", __func__);
    char *pBuf = (char *)at;
    char *pBegin = strstr(pBuf, "\"closed\"");
    if (pBegin != NULL)
    {
        return -1;
    }
    pBegin = strstr(pBuf, "\"pdpdeact\"");
    if (pBegin != NULL)
    {
        return -1;
    }

    pBegin = strstr(pBuf, "\"recv\""); // Point to : "recv",0,49
    pBegin += 7; // Point to : 0,49......
    //LOGD_4G(TAG, "pBegin = %s", pBegin);
    pBegin = strstr(pBegin, ",");
    pBegin++; // Point to : 49......
    char *pEnd = strstr(pBegin, "\r\n");
    char lenStr[32] = {0};
    memcpy(lenStr, pBegin, pEnd - pBegin);
    int len = atoi(lenStr);

    pBegin = pEnd + 2; // Point to : 00 10 24 62 00 2B 01 02 00 ......
    memcpy(pRecvData, pBegin, len);

    LOGV_4G(TAG, "Leave %s(), len = %d", __func__, len);
    return len;
}

static void handle_4G_TCP_recv_data(mqtt_msg_st *pMqttMsg)
{
    LOGV_4G(TAG, "Enter %s()", __func__);
    uint8_t *realDataBuf = pvPortMalloc(1600);
    memset(realDataBuf, 0, 1600);
    hexdump1(pMqttMsg->dataBuff, 256);
    int32_t len = getRealTCPData((char *)pMqttMsg->dataBuff, (char *)realDataBuf);
    LOGI_4G(TAG, "len = %d", len);
    if (len == 0)
    {
        LOGE_4G(TAG, "The real TCP data is 0, so just return.");
        vPortFree(realDataBuf);
        return;
    }
    if (len == -1)
    {
        SET_SD_ELEMENT_VALUE(SD224, 0);
        vPortFree(realDataBuf);
        LOGE_4G(TAG, "The TCP connection had just closed.");
        xTimerStart(g4GReConnectTimer, 0); //Let's reconnect
        return;
    }
    PRINTF("TCP Real Data is :\r\n");
    hexdump1(realDataBuf, 128);

    md_slave_msg_pack ltv_TCPModbusPack = { 0x0, };
    ltv_TCPModbusPack.mcp_RespBuff = gTCPResponseBuf4G;
    ltv_TCPModbusPack.resp_func = tcp_4G_client_send_buffer;
    ltv_TCPModbusPack.mcv_Sender = MB_SENDER_TCP_4G;
    uint16_t allLen = 0;
    uint8_t modbusTCP[2] = {0x00, 0x00};
    uint8_t modbusTCP2[2] = {0x30, 0x00};
    uint16_t recvLen = 0;
    uint8_t *pChar = realDataBuf;
    if (memcmp(modbusTCP,  pChar + MB_TCP_PID, 2) == 0 ||
        memcmp(modbusTCP2, pChar + MB_TCP_PID, 2) == 0) // This is MODBUSTCP protocol
    {
        //LOGI("tcpHandleReceivedData", "I am MODBUSTCP protocol!");
        allLen = (pChar[MB_TCP_LEN] << 8) | pChar[MB_TCP_LEN + 1];
        allLen += MB_TCP_UID;
        recvLen = 0;
        memcpy(gMODBUSTCPHead4G, realDataBuf, MB_TCP_UID);
        //PRINTF("OH, Got the head!  allLen = %u\r\n", allLen);
    }
    else
    {
        LOGE(TAG, "This is not MODBUSTCP protocol!");
        if (is_MicroLinkLogin(realDataBuf, len))
        {
            SET_SD_ELEMENT_VALUE(SD224, 1);
            MicroLink_4G_StartHeart();
            xEventGroupSetBits(g_kalyke_event_group, KALYKE_EVENT_WAIT_MICROLINK_CONNECTED);
        }
        vPortFree(realDataBuf);
        return ;
    }
    memcpy(gTCPRecvBuf4G + recvLen, realDataBuf, len);
    recvLen += len;
    //PRINTF("%s: recvLen = %u\r\n", __func__, recvLen);

    if (recvLen < allLen)
    {
        vPortFree(realDataBuf);
        return ;
    }

    uint16_t crc16 = calc_crc16(gTCPRecvBuf4G + MB_TCP_UID, recvLen - MB_TCP_UID);
    gTCPRecvBuf4G[recvLen++] = crc16 & 0xFF;
    gTCPRecvBuf4G[recvLen++] = crc16 >> 8;
    //LOGW("tcpHandleReceivedData", "crc16 = %X", crc16);
    // We have got all data.
    ltv_TCPModbusPack.mcp_ReceiveBuff = gTCPRecvBuf4G + MB_TCP_UID;
    ltv_TCPModbusPack.msv_ReceiveLen = recvLen - MB_TCP_UID;
    ltv_TCPModbusPack.mcv_IsBroadcastInfo = 0;
    ltv_TCPModbusPack.isTcpClient = 1;
    //hexdump(ltv_TCPModbusPack.mcp_ReceiveBuff, ltv_TCPModbusPack.msv_ReceiveLen);
    mb_slave_msg_handler(&ltv_TCPModbusPack);
    vPortFree(realDataBuf);
}

void kalyke_4G_tcp_task(void *p_arg)
{
    LOGV(TAG, "kalyke_4G_tcp_task RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());
    if (bspIsHave4G() == false)
    {
        LOGW(TAG, "Just return because this product dose not support 4G feature.");
        vTaskDelete(NULL);
        return;
    }

    if (g_plc_netcfg.surfing == 0)
    {
        LOGW(TAG, "Just return because surfing mode is not 4G.");
        vTaskDelete(NULL);
        return;
    }
    g4GTCPBusy = false;
    LOGV(TAG, "Let us wait 5s...");
    vTaskDelay(5000);

    g4GReConnectTimer = xTimerCreate((const char *)"login_4Glink",
                                       (TickType_t  )3000 / portTICK_PERIOD_MS,
                                       (UBaseType_t )pdFALSE,
                                       (void *      )3,
                                       (TimerCallbackFunction_t)reconnect_fexlink);
    g4GMutexTimer = xTimerCreate((const char *)"login_4Glink",
                                       (TickType_t  )5000 / portTICK_PERIOD_MS,
                                       (UBaseType_t )pdFALSE,
                                       (void *      )0,
                                       (TimerCallbackFunction_t)mutex_timer_cb);
    LOGI(TAG, "g4GReConnectTimer = 0x%08X, g4GMutexTimer = 0x%08X", g4GReConnectTimer, g4GMutexTimer);
    LOGW(TAG, "Let us wait : KALYKE_EVENT_TCP_WAIT_4G_MQTT");
    xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_TCP_WAIT_4G_MQTT, pdTRUE, pdFALSE, portMAX_DELAY);

    ec20_TCP_init();
    vTaskDelay(1000);
    login_fexlink();

    g4GTCPQueue = xQueueCreate(10, sizeof(mqtt_msg_st));
    LOGD(TAG, "g4GTCPQueue = 0x%08X", g4GTCPQueue);
    mqtt_msg_st mMsg = {0};
    for(;;)
    {
        LOGI_4G(TAG, "Let us wait 4G TCP receive something happen.");
        if (xQueueReceive(g4GTCPQueue, &mMsg, portMAX_DELAY) == pdFALSE)
        {
            continue;
        }
        switch (mMsg.type)
        {
            case 3:
                handle_4G_TCP_recv_data(&mMsg);
                vPortFree(mMsg.dataBuff);
                break;

            case 4: //Heart
                MicroLink_4G_heart();
                break;

            case 5: // Reconnect
                while (1)
                {
                    if (at_QIOPEN() == 0)
                    {
                        break;
                    }
                    vTaskDelay(10000);
                }
                login_fexlink();
                break;

            default:
                LOGE(TAG, "Error type: %u", mMsg.type);
                break;
        }
    }
}
#else
void kalyke_stop_4G_for_PLC(void){}
/*
  +QIACT: 1,1,1,"10.15.163.170"
  +QIACT: 2,1,1,"10.103.55.62"
  +QIACT: 3,1,1,"10.103.55.62"

  OK
*/
bool check_QIACT(void)
{
    LOGV("4G_TCP", "Enter %s()", __func__);
    char sendBuf[16];
    for(;;)
    {
        sendAT("AT+QIACT?");
        int ret = waitResponse(2000, "+QIACT:", "OK", NULL, NULL, NULL);
        if (ret == 1)
        {
            break;
        }
        else if (ret == 2) // We only got 'OK', so there is no context active
        {
        }
        break;
    }
    return true;
}

#endif /* KALYKE_FEATURE_4G_TCP_TASK */

