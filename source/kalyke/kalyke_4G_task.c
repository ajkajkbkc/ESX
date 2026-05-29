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
#include "kalyke_4G_TCP_task.h"
#include "kalyke_oled_key_task.h"
#if (KALYKE_FEATURE_4G_TASK == 1)

#define DEBUG_4G

#ifdef DEBUG_4G
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
static void ec20_sntp_init(void);
static void mqtt_4G_QMTCLOSE(void);


typedef enum _AT_COMMAND_NAME
{
    AT_NONE        = 0,
    AT_QMTRECV     = 1,
    AT_QMTSTAT     = 2
}at_command_t;
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define CPIN_BASE           100
#define QMTOPEN_ERR_BASE    200
#define QMTSTATE_BASE       300
#define QMTPUBEX_BASE       400
#define QMTCONN_ERR_BASE    500
#define CCEREG_BASE         600

#define CME_ERROR_BASE      1000
#define CME_GPS_ERROR_BASE  2000


/*
* Connection is closed or reset by peer.
* Execute AT+QMTOPEN command and reopen
* MQTT connection
*/
#define ERROR_QMTSTATE_1    1
/*
* Sending PINGREQ packet timed out or failed.
* Deactivate PDP first, and then active PDP and
* reopen MQTT connection.
*/
#define ERROR_QMTSTATE_2    2

/*
* Sending CONNECT packet timed out or failed.
* 1. Check whether the inputted user name and
*    password are correct.
* 2. Make sure the client ID is not used.
* 3. Reopen MQTT connection and try to send
*    CONNECT packet to server again.
*/
#define ERROR_QMTSTATE_3    3

/*
* Receiving CONNECK packet timed out or failed.
* 1. Check whether the inputted user name and
*    password are correct.
* 2. Make sure the client ID is not used.
* 3. Reopen MQTT connection and try to send
*    CONNECT packet to server again.
*/
#define ERROR_QMTSTATE_4    4

/*
* The client sends DISCONNECT packet to sever and the server is
  initiative to close MQTT connection.
* 
* This is a normal process.
*/
#define ERROR_QMTSTATE_5    5

/*
* The client is initiative to close MQTT connection due to packet
  sending failure all the time.
* 1. Make sure the data is correct.
* 2. Try to reopen MQTT connection since there
     may be network congestion or an error.
*/
#define ERROR_QMTSTATE_6    6

/*
* The link is not alive or the server is unavailable.
* Make sure the link is alive or the server is available
  currently.
*/
#define ERROR_QMTSTATE_7    7

/*******************************************************************************
 * Variables
 ******************************************************************************/
TaskHandle_t gKalyke4GTaskHandle = NULL;
static TaskHandle_t g_Uart8WorkerHandler = NULL;
TimerHandle_t gUart8RxTimer =  NULL;

static uint8_t background_buffer8[8192];
static lpuart_rtos_config_t lpuart_config8 =
{
    .baudrate = 115200,
    .parity = kLPUART_ParityDisabled,
    .stopbits = kLPUART_OneStopBit,
    .buffer = background_buffer8,
    .buffer_size = sizeof(background_buffer8),
    //.enableRxRTS = false,
    //.enableTxCTS = false,
};
static lpuart_rtos_handle_t gLpuart8RTOSHandle;
static uint8_t recv_buffer8[16];
static struct _lpuart_handle t_handle8;
volatile uint8_t gUartStatus = UART_IDLE;
uint8_t gUart8RecvBuffer[AT_RECV_MAX_BYTE];
static uint16_t gUart8RecvLength = 0;

volatile bool gIs4GMqttConnected = false;
volatile static bool g4GMqttPublishing = false;
volatile static bool gIsReConnecting = false;
static void handleErrorQMTSTATE(uint8_t errCode);

static QueueHandle_t g4GMqttMsgQueue;
SemaphoreHandle_t g4GMutex;

gps_st gGpsValue;

/*******************************************************************************
 * Code
 ******************************************************************************/

// +CME ERROR: 10
static uint16_t getCME_ERROR_code(void)
{
    char *pBuf = (char *)gUart8RecvBuffer;
    char temp[16];
    int errCode = 0;
    char *pCME = strstr(pBuf, "ERROR:");
    if(pCME == NULL)  //不加这句时不打log会跑飞
    {
        LOGE_4G("4G_task", "pCME == NULL");
        return 0;
    }
    LOGD_4G("4G_task", "pCME = %s", pCME);
    sscanf(pCME, "%s %d", temp, &errCode);
    LOGV_4G("4G_task", "Leave %s(), %s %d", __func__, temp, errCode);
    return errCode;
}

static void setCME_ERROR_SD(void)
{
    uint16_t err = getCME_ERROR_code();
    err += CME_ERROR_BASE;
    SET_SD_ELEMENT_VALUE(SD230, err);
}

static void notify_QMTSTAT(char *pBuf)
{
    mqtt_msg_st mqttMsg;
    mqttMsg.type = 0;
    mqttMsg.msgLength = strlen(pBuf);
    mqttMsg.dataBuff = pvPortMalloc(64);
    memset(mqttMsg.dataBuff, 0, 64);
    memcpy(mqttMsg.dataBuff, pBuf, mqttMsg.msgLength);
    BaseType_t ret = xQueueSendToFront(g4GMqttMsgQueue, &mqttMsg, 100);
    if (ret == pdFALSE)
    {
        vPortFree(mqttMsg.dataBuff);
    }
}

#if (AT_LOGIC == 1)
static void uart4_callback_func(TimerHandle_t ltv_TimeHandle)
{
    LOGV_4G("4G_task", "Enter %s, gUart4RecvLength = %u", __func__, gUart4RecvLength);
#if 0
    char *pATResp = pvPortMalloc(gUart4RecvLength + 2);
    memset(pATResp, 0, gUart4RecvLength + 2);
    memcpy(pATResp, gUart4RecvBuffer, gUart4RecvLength);
    LOGI_4G("4G_task", "pATResp = %s", pATResp);
    gUartStatus = UART_IDLE;
    vPortFree(pATResp);
#endif
    LOGI_4G("4G_task", "pATResp = \r\n%s", gUart4RecvBuffer);
    hexdump(gUart4RecvBuffer, gUart4RecvLength);
    xEventGroupSetBits(g_kalyke_event_group, KALYKE_EVENT_AT_RESPONSE_OCCUR);
    gUartStatus = UART_IDLE;
    LOGV_4G("4G_task", "Leave %s", __func__);
}
#elif (AT_LOGIC == 2)
static void uart8_callback_func(TimerHandle_t ltv_TimeHandle)
{
    LOGV_4G("4G_task", "Enter %s, gUart8RecvLength = %u", __func__, gUart8RecvLength);
    LOGE_4G("4G_task", "pATResp = %s", gUart8RecvBuffer);
#ifdef DEBUG_4G
    hexdump(gUart8RecvBuffer, gUart8RecvLength);
#endif
    char *pQMTRECV = NULL;
    if ((pQMTRECV = strstr((char *)gUart8RecvBuffer, "+QIURC:")) != NULL)//收到TCP数据
    {
        if (g4GMqttPublishing == true)
        {
            LOGW("4G_task", "g4GMqttPublishing is true, so do not handle +QIURC");
            gUart8RecvLength = 0;
            goto EXITME;
        }
        if (gUart8RecvLength < 22)
        {
            LOGE("4G_task", "data length less than 22, so do not handle +QIURC");
            gUart8RecvLength = 0;
            goto EXITME;
        }
    #if (KALYKE_FEATURE_4G_TCP_TASK == 1)
        tcp_4G_start_busy_timer();
    #endif
        mqtt_msg_st Msg;
        Msg.type = 3;
        Msg.msgLength = gUart8RecvLength;
        Msg.dataBuff = pvPortMalloc(1600);
        memset(Msg.dataBuff, 0, 1600);
        memcpy(Msg.dataBuff, pQMTRECV, Msg.msgLength);
    #if (KALYKE_FEATURE_4G_TCP_TASK == 1)
        if (g4GTCPQueue)
        {
            BaseType_t ret = xQueueSendToBack(g4GTCPQueue, &Msg, 0);
            if (ret == pdFALSE)
            {
                vPortFree(Msg.dataBuff);
            }
        }
    #endif
        gUart8RecvLength = 0;
    }
#if (KALYKE_FEATURE_4G_TCP_TASK == 1)
    else if ((pQMTRECV = strstr((char *)gUart8RecvBuffer, "SEND ")) != NULL)//TCP每发送一次数据便会返回SEND OK，把这个过滤掉
    {
        gUart8RecvLength = 0;
        reset_4G_heart_timer();
    }
#endif
    else if ((pQMTRECV = strstr((char *)gUart8RecvBuffer, "+QMTRECV:")) != NULL)
    {
        mqtt_msg_st mqttMsg;
        mqttMsg.type = 0;
        mqttMsg.msgLength = strlen(pQMTRECV);
        mqttMsg.dataBuff = pvPortMalloc(2048);
        memset(mqttMsg.dataBuff, 0, 2048);
        memcpy(mqttMsg.dataBuff, pQMTRECV, mqttMsg.msgLength);
        BaseType_t ret = xQueueSendToBack(g4GMqttMsgQueue, &mqttMsg, 100);
        if (ret == pdFALSE)
        {
            vPortFree(mqttMsg.dataBuff);
        }
        gUart8RecvLength = 0;
    }
    else if ((pQMTRECV = strstr((char *)gUart8RecvBuffer, "+QMTSTAT:")) != NULL)
    {
        notify_QMTSTAT(pQMTRECV);
        gUart8RecvLength = 0;
    }
EXITME:
    gUartStatus = UART_IDLE;
    LOGD_4G("4G_task", "Leave %s", __func__);
}
#endif

static void start_uart8_recv_timeout_timer(void)
{
    if(gUart8RxTimer == NULL)
    {
        gUart8RxTimer = xTimerCreate((const char *)"Uart8 RxTime",
                                     (TickType_t  )10 / portTICK_PERIOD_MS,
                                     (UBaseType_t )pdFALSE,
                                     (void *      )1,
                                     (TimerCallbackFunction_t)uart8_callback_func);
    }
    BaseType_t ret = xTimerStart(gUart8RxTimer, 0);
    if (ret == pdFAIL)
    {
        LOGE_4G("4G_task", "Start gUart8RxTimer ERROR!!!!");
    }
}

void stop_uart8_recv_timeout_timer(void)
{
    BaseType_t ret = xTimerStop(gUart8RxTimer, 100);
    if (ret == pdFAIL)
    {
        LOGE_4G("4G_task", "Stop gUart8RxTimer ERROR!!!!");
    }
}

static void reset_uart4_recv_timeout_timer(void)
{
    BaseType_t ret = xTimerReset(gUart8RxTimer, 0);
    if (ret == pdFAIL)
    {
        LOGE_4G("4G_task", "Reset gUart8RxTimer ERROR!!!!");
    }
}

#if (AT_LOGIC == 1)
static void uart4_worker_task(void *pvParameters)
{
    unsigned char lcv_RecvChar;
    int error;
    size_t n;
    taskENTER_CRITICAL();
    uint32_t clock = BOARD_DebugConsoleSrcFreq();

    lpuart_config4.srcclk = clock;
    lpuart_config4.base = LPUART8;

    if (0 > LPUART_RTOS_Init(&gLpuart4RTOSHandle, &t_handle4, &lpuart_config4))
    {
        LOGE_4G("4G_task", "LPUART_RTOS_Init ERROR!\r\n");
        vTaskSuspend(NULL);
    }

    LOGV_4G("4G_task", "uart4_worker_task RUN! Free heap size is %d bytes\r\n", xPortGetFreeHeapSize());
    taskEXIT_CRITICAL();
    /* Receive just one byte. */
    do
    {
        error = LPUART_RTOS_Receive(&gLpuart4RTOSHandle, recv_buffer4, 1, &n);
        if (error == kStatus_LPUART_RxHardwareOverrun)
        {
            /* Notify about hardware buffer overrun */
            LOGE_4G("4G_task", "\r\nHardware buffer overrun!\r\n");
        }
        if (error == kStatus_LPUART_RxRingBufferOverrun)
        {
            /* Notify about ring buffer overrun */
            LOGE_4G("4G_task", "\r\nRing buffer overrun!\r\n");
        }
        if (n > 0)
        {
            lcv_RecvChar = (unsigned char)recv_buffer4[0];
            if (gUartStatus == UART_IDLE)
            {
                gUartStatus = UART_RX;
                gUart4RecvLength = 0;
                memset(gUart4RecvBuffer, 0, sizeof(gUart4RecvBuffer));
                start_uart4_recv_timeout_timer();
                gUart4RecvBuffer[gUart4RecvLength++] = lcv_RecvChar;
                continue;
            }
            reset_uart4_recv_timeout_timer();
            gUart4RecvBuffer[gUart4RecvLength++] = lcv_RecvChar;
        }
    }
    while(1);
}
#elif (AT_LOGIC == 2)
static void uart8_worker_task(void *pvParameters)
{
    unsigned char lcv_RecvChar;
    int error;
    size_t n;
    taskENTER_CRITICAL();
    uint32_t clock = BOARD_DebugConsoleSrcFreq();

    lpuart_config8.srcclk = clock;
    lpuart_config8.base = LPUART8;

    if (0 > LPUART_RTOS_Init(&gLpuart8RTOSHandle, &t_handle8, &lpuart_config8))
    {
        LOGE_4G("4G_task", "LPUART_RTOS_Init ERROR!\r\n");
        vTaskSuspend(NULL);
    }
    LOGI("4G_task", "lpuart_config8.buffer_size = %u", lpuart_config8.buffer_size);
    LOGV_4G("4G_task", "uart8_worker_task RUN! Free heap size is %d bytes\r\n", xPortGetFreeHeapSize());
    taskEXIT_CRITICAL();
    /* Receive just one byte. */
    do
    {
        error = LPUART_RTOS_Receive(&gLpuart8RTOSHandle, recv_buffer8, 1, &n);
        if (error == kStatus_LPUART_RxHardwareOverrun)
        {
            /* Notify about hardware buffer overrun */
            LOGE_4G("4G_task", "Hardware buffer overrun! -- n = %u", n);
        }
        if (error == kStatus_LPUART_RxRingBufferOverrun)
        {
            /* Notify about ring buffer overrun */
            LOGE_4G("4G_task", "Ring buffer overrun! -- n = %u", n);
        }
        if (n > 0)
        {
            if (gUart8RecvLength >= AT_RECV_MAX_BYTE)
            {
                continue;
            }
            lcv_RecvChar = (unsigned char)recv_buffer8[0];
            if (gUartStatus == UART_IDLE)
            {
                gUartStatus = UART_RX;
                start_uart8_recv_timeout_timer();
                gUart8RecvBuffer[gUart8RecvLength++] = lcv_RecvChar;
                continue;
            }
            reset_uart4_recv_timeout_timer();
            gUart8RecvBuffer[gUart8RecvLength++] = lcv_RecvChar;
        }
    }
    while(1);
}

#endif

void init_uart8_receive_buffer(void)
{
    gUartStatus = UART_IDLE;
    gUart8RecvLength = 0;
    memset(gUart8RecvBuffer, 0, sizeof(gUart8RecvBuffer));
}

void init_uart8_receive_buffer_small(void)
{
    gUartStatus = UART_IDLE;
    gUart8RecvLength = 0;
    memset(gUart8RecvBuffer, 0, 128);
}

void uart8_send_buffer(char *lcp_SendBuff, unsigned short lsv_Length)
{
    //LOGV_4G("4G_task", "Enter %s(), lcp_SendBuff = %s, lsv_Length = %d\r\n", __func__, lcp_SendBuff, lsv_Length);
    LOGV_4G("4G_task", "Enter %s(), lsv_Length = %u", __func__, lsv_Length);
#ifdef DEBUG_4G
    hexdump(lcp_SendBuff, lsv_Length);
#endif
    LPUART_RTOS_Send(&gLpuart8RTOSHandle, (uint8_t *)lcp_SendBuff, lsv_Length);
}

/* 对EC20用到的串口初始化 */
static void ec20_LPUART8_init(unsigned long llv_BaudRate, unsigned char lcv_Parity, unsigned char lcv_WordLength, unsigned char lcv_StopBits)
{
    LOGV_4G("4G_task", "Enter %s, llv_BaudRate=%d, lcv_Parity=%d, lcv_WordLength=%d, lcv_StopBits=%d.\r\n", __func__, llv_BaudRate, lcv_Parity, lcv_WordLength, lcv_StopBits);
    if (g_Uart8WorkerHandler != NULL)
    {
        LOGV_4G("4G_task", "UART worker 0 is running, so just return.\r\n");
        return;
    }
    NVIC_SetPriority(LPUART8_IRQn, 5);
    lpuart_config8.baudrate = llv_BaudRate;
    switch(lcv_StopBits)
    {
    case UART_STB_1:
        lpuart_config8.stopbits = kLPUART_OneStopBit;
        break;
    case UART_STB_2:
        lpuart_config8.stopbits = kLPUART_TwoStopBit;
        break;
    }
    switch(lcv_Parity)
    {
    case 0:
        lpuart_config8.parity = kLPUART_ParityDisabled;
        break;

    case 1:
        lpuart_config8.parity = kLPUART_ParityOdd;//奇
        break;

    case 2:
        lpuart_config8.parity = kLPUART_ParityEven;//偶
        break;
    }
    if (lcv_WordLength == 8)
    {
        lpuart_config8.dataBits = kLPUART_EightDataBits;
    }
    else
    {
        lpuart_config8.dataBits = kLPUART_SevenDataBits;
    }
    if (xTaskCreate(uart8_worker_task, "Uart8_worker",
                    UART_WORKER_4G_TASK_STACK_SIZE, NULL,
                    UART_WORKER_4G_TASK_PRIORITY, (TaskHandle_t *)&g_Uart8WorkerHandler) != pdPASS)
    {
        LOGE_4G("4G_task", "Uart8_worker task creation failed!\r\n");
        while (1)
            ;
    }
}

/*!
 * brief 等待已发出的AT命令的响应
 *
 * param timeout_ms 最长等待超时时间
 * param r1 希望等待得到的字符串1
 * param r2 希望等待得到的字符串2
 * param r3 希望等待得到的字符串3
 * param r4 希望等待得到的字符串4
 * param r5 希望等待得到的字符串5
 *
 * return 1 ~ 5对应得到r1 ~ r5的字符串响应
 *        0: 超时
 *       -1: 没有得到希望得到的响应字符串
 */
#if (AT_LOGIC == 1)
int waitResponse(uint32_t timeout_ms,
                 char *r1, char *r2,
                 char *r3, char *r4, char *r5)
{
    LOGD_4G("4G_task", "Enter %s()", __func__);
    char *pRecvBuf = (char*)gUart4RecvBuffer;
    //char *strx = NULL;
    int index = -1;
    TickType_t xTicksToWait = timeout_ms / portTICK_PERIOD_MS;
    EventBits_t uxBits = 0;
    TickType_t curTick = xTaskGetTickCount();
    uxBits = xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_AT_RESPONSE_OCCUR,
                                 pdTRUE, pdFALSE, xTicksToWait);
    if( ( uxBits & KALYKE_EVENT_AT_RESPONSE_OCCUR ) == 0 )// Time out
    {
        LOGE_4G("4G_task", "waitResponse time out : 0x%08X", uxBits);
        return 0;
    }
    //Got AT response
    if (r1 && strstr(pRecvBuf, r1))
    {
        index = 1;
    }
    else if (r2 && strstr(pRecvBuf, r2))
    {
        index = 2;
    }
    else if (r3 && strstr(pRecvBuf, r3))
    {
        index = 3;
    }
    else if (r4 && strstr(pRecvBuf, r4))
    {
        index = 4;
    }
    else if (r5 && strstr(pRecvBuf, r5))
    {
        index = 5;
    }
    LOGD_4G("4G_task", "Leave %s(), index = %d", __func__, index);
    return index;
}
#elif (AT_LOGIC == 2)
int waitResponse(uint32_t timeout_ms,
                 char *r1, char *r2,
                 char *r3, char *r4, char *r5)
{
    LOGD_4G("4G_task", "Enter %s(), g4GMqttPublishing = %u", __func__, g4GMqttPublishing);
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
        #if 0
        if (g4GMqttPublishing)
        {
            vTaskDelay(976);
            continue;
        }
        #endif
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
        vTaskDelay(100);
    }
    LOGD_4G("4G_task", "Leave %s(), index = %d", __func__, index);
    return index;
}
#endif

int waitResponsePublish(uint32_t timeout_ms,
                 char *r1, char *r2,
                 char *r3, char *r4, char *r5)
{
    LOGD_4G("4G_task", "Enter %s()", __func__);
    char *pRecvBuf = (char*)gUart8RecvBuffer;
    int index = -1;
    TickType_t lastMS = xTaskGetTickCount();

    while (1)
    {
        vTaskDelay(100);
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
        vTaskDelay(200);
    }
    LOGD_4G("4G_task", "Leave %s(), index = %d", __func__, index);
    return index;
}

void sendAT(char *pAT)
{
    LOGE_4G("4G_task", "Enter %s(), pAT = %s", __func__, pAT);
    static char atBuf[512];

    init_uart8_receive_buffer();
    strcpy(atBuf, pAT);
    strcat(atBuf, "\r\n");
    uart8_send_buffer(atBuf, strlen(atBuf));
}

static void mqtt_QMTCFG(void)
{
    char *sendBuf = pvPortMalloc(512);
    int waitRet;
    while(1)
    {
        if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
        {
            LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR...00", __func__);
            vTaskDelay(1000);
            continue;
        }
        sendAT("AT+QMTCFG=\"recv/mode\",0,0,1");
        waitRet = waitResponse(10000, "OK", "+CME ERROR:", "ERROR", NULL, NULL);
        if (waitRet == 1)
        {
            LOGW_4G("4G_task", "AT+QMTCFG=\"recv/mode\" OK!");
            vTaskDelay(1000);
            break;
        }
        else if (waitRet == 2)
        {
            setCME_ERROR_SD();
        }
        else if (waitRet == 3)
        {
            LOGW_4G("4G_task", "AT+QMTCFG=\"recv/mode\" OK because mqtt is connected!");
            vTaskDelay(100);
            break;
        }
        xSemaphoreGive(g4GMutex);
        vTaskDelay(4000);
    }
    xSemaphoreGive(g4GMutex);
    while(1)
    {
        if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
        {
            LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR...01", __func__);
            vTaskDelay(1000);
            continue;
        }
        sprintf(sendBuf, "AT+QMTCFG=\"keepalive\",0,%u", g_plc_netcfg.mqtt.keepalive);
        sendAT(sendBuf);
        waitRet = waitResponse(10000, "OK", "+CME ERROR:", "ERROR", NULL, NULL);
        if (waitRet == 1)
        {
            LOGW_4G("4G_task", "AT+QMTCFG=\"keepalive\" OK!");
            break;
        }
        else if (waitRet == 2)
        {
            setCME_ERROR_SD();
        }
        else if (waitRet == 3)
        {
            LOGW_4G("4G_task", "AT+QMTCFG=\"keepalive\" OK because mqtt is connected!");
            vTaskDelay(100);
            break;
        }
        xSemaphoreGive(g4GMutex);
        vTaskDelay(4000);
    }

    vPortFree(sendBuf);
    xSemaphoreGive(g4GMutex);
}

static void http_QHTTPCFG(void)
{
    while(1)
    {
        if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
        {
            LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR...00", __func__);
            vTaskDelay(1000);
            continue;
        }
        sendAT("AT+QHTTPCFG=\"contextid\",1");
        int Ret1 = waitResponse(5000, "OK", NULL, NULL, NULL, NULL);
        if (Ret1 == 1)
        {
            LOGE_4G("4G_task", "AT+QHTTPCFG=\"contextid\" OK!");
            vTaskDelay(1000);
            break;
        }
        xSemaphoreGive(g4GMutex);
        vTaskDelay(4000);
    }
    xSemaphoreGive(g4GMutex);
    
    while(1)
    {
        if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
        {
            LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR...00", __func__);
            vTaskDelay(1000);
            continue;
        }
        sendAT("AT+QHTTPCFG=\"responseheader\",0");
        int Ret2 = waitResponse(5000, "OK", NULL, NULL, NULL, NULL);
        if (Ret2 == 1)
        {
            LOGE_4G("4G_task", "AT+QHTTPCFG=\"responseheader\" OK!");
            vTaskDelay(1000);
            break;
        }
        xSemaphoreGive(g4GMutex);
        vTaskDelay(4000);
    }
    xSemaphoreGive(g4GMutex);
    
    while(1)
    {
        if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
        {
            LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR...00", __func__);
            vTaskDelay(1000);
            continue;
        }
        sendAT("AT+QICSGP=1,1,\"UNINET\",\"\",\"\",1");
        int Ret3 = waitResponse(5000, "OK", NULL, NULL, NULL, NULL);
        if (Ret3 == 1)
        {
            LOGE_4G("4G_task", "AT+QICSGP=OK!");
            vTaskDelay(1000);
            break;
        }
        xSemaphoreGive(g4GMutex);
        vTaskDelay(4000);
    }
    xSemaphoreGive(g4GMutex);
    
    while(1)
    {
        if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
        {
            LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR...00", __func__);
            vTaskDelay(1000);
            continue;
        }
        sendAT("AT+CFUN=1,1");
        int Ret4 = waitResponse(5000, "OK", NULL, NULL, NULL, NULL);
        if (Ret4 == 1)
        {
            LOGE_4G("4G_task", "AT+CFUN=OK!");
            vTaskDelay(1000);
            break;
        }
        xSemaphoreGive(g4GMutex);
        vTaskDelay(4000);
    }
    xSemaphoreGive(g4GMutex);
}

/*
 * +QMTOPEN: <client_idx>,<result>
 * +QMTOPEN: 0,4
 * result: -1 = Failed to open network
            0 = Network opened successfully
            1 = Wrong parameter
            2 = MQTT identifier is occupied
            3 = Failed to activate PDP
            4 = Failed to parse domain name
            5 = Network connection error
*/
static int8_t getQMTOPENResult(void)
{
    char *pBuf = (char *)gUart8RecvBuffer;
    char *pOMTOPEN = strstr(pBuf, "+QMTOPEN:");
    LOGV_4G("4G_task", "pOMTOPEN = %s", pOMTOPEN);
    int client_id;
    int errCode;
    char temp[32];
    sscanf(pOMTOPEN, "%s %d,%d", temp, &client_id, &errCode);

    LOGD_4G("4G_task", "Leave %s(), result=%d", __func__, errCode);
    return errCode;
}

/*
* return 0: Kalyke冷重启 ,  1: Kalyke热重启
*/
static uint8_t mqtt_QMTOPEN(void)
{
    if (memcmp(g_plc_netcfg.mqtt.vender, "AliMQTT", 7) == 0)
    {
        Ali_MQTT_init();
    }

    uint8_t retVal = 0;
    char *sendBuf = pvPortMalloc(512);
    while(1)
    {
        if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
        {
            LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR", __func__);
            vTaskDelay(1000);
            continue;
        }
        sprintf(sendBuf, "AT+QMTOPEN=0,\"%s\",%u", g_plc_netcfg.mqtt.host, g_plc_netcfg.mqtt.port);
        sendAT(sendBuf);
        int ret = waitResponse(120000, "OK\r\n\r\n+QMTOPEN:", "+CME ERROR:", NULL, NULL, NULL);
        if (ret == 1)
        {
            int8_t result = getQMTOPENResult();
            if (result == 0) // Network opened successfully
            {
                LOGW_4G("4G_task", "AT+QMTOPEN OK!");
                SET_SD_ELEMENT_VALUE(SD230, 0);
                break;
            }
            else if (result == 2) // MQTT identifier is occupied
            {
                LOGW_4G("4G_task", "MQTT identifier is occupied, so Kalyke just reset.");
                gIs4GMqttConnected = true;
                SET_SD_ELEMENT_VALUE(SD225, 1);
                SET_SD_ELEMENT_VALUE(SD230, 0);
                retVal = 1;
                break;
            }
            else
            {
                int16_t err;
                if (result == -1)
                {
                    err = QMTOPEN_ERR_BASE + 6;
                }
                else
                {
                    err = QMTOPEN_ERR_BASE + result;
                }
                SET_SD_ELEMENT_VALUE(SD230, err);
            }
        }
        else if (ret == 2)
        {
            setCME_ERROR_SD();
        }
        xSemaphoreGive(g4GMutex);
        xEventGroupSetBits(g_kalyke_event_group, KALYKE_EVENT_TCP_WAIT_4G_MQTT);
        vTaskDelay(6000);
    }
    vPortFree(sendBuf);
    xSemaphoreGive(g4GMutex);
    return retVal;
}


static uint8_t mqtt_QMTOPEN2(int nTimes)
{
    uint8_t retVal = 0;
    char *sendBuf = pvPortMalloc(512);
    while(1)
    {
        if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
        {
            LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR", __func__);
            vTaskDelay(1000);
            continue;
        }
        sprintf(sendBuf, "AT+QMTOPEN=0,\"%s\",%u", g_plc_netcfg.mqtt.host, g_plc_netcfg.mqtt.port);
        sendAT(sendBuf);
        int ret = waitResponse(120000, "OK\r\n\r\n+QMTOPEN:", "+CME ERROR:", NULL, NULL, NULL);
        if (ret == 1)
        {
            int8_t result = getQMTOPENResult();
            if (result == 0) // Network opened successfully
            {
                LOGW_4G("4G_task", "AT+QMTOPEN OK!");
                SET_SD_ELEMENT_VALUE(SD230, 0);
                retVal = 1;
                break;
            }
            else if (result == 2) // MQTT identifier is occupied
            {
                LOGE_4G("4G_task", "AT+QMTOPEN ERR1, MQTT identifier is occupied, so  reset 4G.");
                //gIs4GMqttConnected = true;
                //SET_SD_ELEMENT_VALUE(SD225, 1);
                //SET_SD_ELEMENT_VALUE(SD230, 0);
                xSemaphoreGive(g4GMutex);
                if (nTimes == 1)
                    mqtt_4G_QMTCLOSE();
                else
                    mqtt_4G_rst();
                break;
            }
            else
            {
                LOGE_4G("4G_task", "AT+QMTOPEN ERR2, So reset 4G.");
                int16_t err;
                if (result == -1)
                {
                    err = QMTOPEN_ERR_BASE + 6;
                }
                else
                {
                    err = QMTOPEN_ERR_BASE + result;
                }
                SET_SD_ELEMENT_VALUE(SD230, err);
                xSemaphoreGive(g4GMutex);
                if (nTimes == 1)
                    mqtt_4G_QMTCLOSE();
                else
                    mqtt_4G_rst();
                break;
            }
        }
        else
        {
            LOGE_4G("4G_task", "AT+QMTOPEN ERR3, ret = %u", ret);
            setCME_ERROR_SD();
            xSemaphoreGive(g4GMutex);
            if (nTimes == 1)
                mqtt_4G_QMTCLOSE();
                else
                mqtt_4G_rst();
                break;
        }
        //xSemaphoreGive(g4GMutex);
        //xEventGroupSetBits(g_kalyke_event_group, KALYKE_EVENT_TCP_WAIT_4G_MQTT);
        //vTaskDelay(6000);
        break;
    }
    vPortFree(sendBuf);
    xSemaphoreGive(g4GMutex);
    return retVal;
}
/*
 * +QMTCLOSE: <client_idx>,<result>
 * +QMTCLOSE: 0,0
 * result: -1 = Failed to close network
            0 = Network closed successfully
*/
static int8_t getQMTCLOSEResult(void)
{
    char *pBuf = (char *)gUart8RecvBuffer;
    char *pOMTCLOSE = strstr(pBuf, "+QMTCLOSE:");
    LOGV_4G("4G_task", "pOMTCLOSE = %s", pOMTCLOSE);

    int client_idx;
    int result;
    char temp[32];
    sscanf(pOMTCLOSE, "%s %d,%d", temp, &client_idx, &result);

    LOGD_4G("4G_task", "Leave %s(), client_idx = %d, result=%d", __func__, client_idx, result);
    return result;
}

static void mqtt_4G_QMTCLOSE(void)
{
    LOGE_4G("4G_task", "Enter %s!", __func__);

    uint8_t retVal = 0;
    char *sendBuf = pvPortMalloc(512);
    while(1)
    {
        if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
        {
            LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR", __func__);
            vTaskDelay(1000);
            continue;
        }
        sprintf(sendBuf, "AT+QMTCLOSE=%u", 0);
        sendAT(sendBuf);
        int ret = waitResponse(30000, "OK\r\n\r\n+QMTCLOSE:", "+CME ERROR:", NULL, NULL, NULL);
        if (ret == 1)
        {
            int8_t result = getQMTCLOSEResult();
            if (result == 0) // Network closed successfully
            {
                LOGI_4G("4G_task", "AT+QMTCLOSE successfully!");
                break;
            }
            else if (result == -1) // Failed to close network
            {
                int16_t err;
                err = QMTOPEN_ERR_BASE + 7;
                SET_SD_ELEMENT_VALUE(SD230, err);
                LOGE_4G("4G_task", "AT+QMTCLOSE failed!!...00");
                break;
            }
        }
        else
        {
            LOGE_4G("4G_task", "AT+QMTCLOSE failed!!...ret = %u",ret);
            setCME_ERROR_SD();
            break;
        }
        //xSemaphoreGive(g4GMutex);
        //vTaskDelay(6000);
        break;
    }
    vPortFree(sendBuf);
    xSemaphoreGive(g4GMutex);
}

void mqtt_4G_QMTDISC(void)
{
AGAIN:
    LOGW_4G("4G_task", "Enter %s()", __func__);
    if (xSemaphoreTake(g4GMutex, 5600/portTICK_PERIOD_MS) == pdFALSE)
    {
        LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR", __func__);
        vTaskDelay(1000);
        goto AGAIN;
    }
    char sendBuf[64] = {0};
    strcpy(sendBuf, "AT+QMTDISC=0");
    sendAT(sendBuf);
    xSemaphoreGive(g4GMutex);
}
void mqtt_4G_rst(void)
{
AGAIN:
    LOGW_4G("4G_task", "Enter %s()", __func__);
    if (xSemaphoreTake(g4GMutex, 5600/portTICK_PERIOD_MS) == pdFALSE)
    {
        LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR", __func__);
        vTaskDelay(1000);
        goto AGAIN;
    }
    char sendBuf[64] = {0};
    strcpy(sendBuf, "AT+CFUN=1,1\r\n");
    sendAT(sendBuf);
    int ret = waitResponse(20000, "OK", "+CME ERROR", NULL, NULL, NULL);
    if (ret == 1)
    {
        LOGE("4G_task", "%s success", sendBuf);
        SET_SD_ELEMENT_VALUE(SD232, GET_SD_ELEMENT_VALUE(SD232)+1 );
    }
    else
    {
        LOGE("4G_task", "%s failed", sendBuf);
        setCME_ERROR_SD();
        SET_SD_ELEMENT_VALUE(SD232, GET_SD_ELEMENT_VALUE(SD232)+1000 );
    }
//    vTaskDelay(10000);
    xSemaphoreGive(g4GMutex);
}

/*
 * +QMTCONN: <client_idx>,<result>[,<ret_code>]
 * +QMTCONN: 0,0,0
 * ret_code: 0 = Connection Accepted
             1 = Connection Refused: Unacceptable Protocol Version
             2 = Connection Refused: Identifier Rejected
             3 = Connection Refused: Server Unavailable
             4 = Connection Refused: Bad User Name or Password
             5 = Connection Refused: Not Authorized
*/
static uint8_t getQMTCONNRet_code(void)
{
    char *pBuf = (char *)gUart8RecvBuffer;
    char *pOMTCONN = strstr(pBuf, "+QMTCONN:");
    LOGV_4G("4G_task", "pOMTCONN = %s", pOMTCONN);

    int client_idx;
    int result;
    int ret_code;
    char temp[32];
    sscanf(pOMTCONN, "%s %d,%d,%d", temp, &client_idx, &result, &ret_code);

    LOGD_4G("4G_task", "Leave %s(), client_idx=%d, result = %d, ret_code = %d", __func__, client_idx, result, ret_code);
    return ret_code;
}

static uint8_t mqtt_QMTCONN(void)
{
    if (memcmp(g_plc_netcfg.mqtt.vender, "AliMQTT", 7) == 0)
    {
        Ali_MQTT_init();
    }
    uint8_t retVal = 0;
    char *sendBuf = pvPortMalloc(512);
    while(1)
    {
        if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
        {
            LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR", __func__);
            vTaskDelay(1000);
            continue;
        }
        sprintf(sendBuf, "AT+QMTCONN=0,\"%s\",\"%s\",\"%s\"", g_plc_netcfg.mqtt.client_id, g_plc_netcfg.mqtt.username, g_plc_netcfg.mqtt.password);
        sendAT(sendBuf);
        int ret = waitResponse(10000, "OK\r\n\r\n+QMTCONN:", "+CME ERROR:", "ERROR", NULL, NULL);
        if (ret == 1)
        {
            uint8_t ret_code = getQMTCONNRet_code();
            if (ret_code != 0)
            {
                SET_SD_ELEMENT_VALUE(SD230, QMTCONN_ERR_BASE + ret_code);
                LOGE_4G("4G_task", "AT+QMTCONN ret Fail, let's try again!");
            }
            else
            {
               LOGI_4G("4G_task", "AT+QMTCONN ret OK!");
               retVal = 1;
               break;
            }
        }
        else if (ret == 2)
        {
            LOGE_4G("4G_task", "AT+QMTCONN ret 2!");
            setCME_ERROR_SD();
        }
        else if (ret == 3)
        {
            LOGE_4G("4G_task", "AT+QMTCONN ret = 3!");
            SET_SD_ELEMENT_VALUE(SD230, QMTCONN_ERR_BASE + 6);//ret_code最大值是5，这里用6
        }
        else
        {
            LOGE_4G("4G_task", "AT+QMTCONN ret = else!");
            SET_SD_ELEMENT_VALUE(SD230, QMTCONN_ERR_BASE + 7);//ret_code最大值是5，这里用7
        }
        break; 
    }
    vPortFree(sendBuf);
    xSemaphoreGive(g4GMutex);
    return retVal;
}


static void mqtt_QMTSUB_do(char *pTopic)
{
    if (strlen(pTopic) == 0)
    {
        return;
    }
    char *sendBuf = pvPortMalloc(2048);
    while(1)
    {
        if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
        {
            LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR", __func__);
            vTaskDelay(1000);
            continue;
        }
        sprintf(sendBuf, "AT+QMTSUB=0,1,\"%s\",0", pTopic);
        sendAT(sendBuf);
        int ret = waitResponse(15000, "OK\r\n\r\n+QMTSUB:", "+CME ERROR:", NULL, NULL, NULL);
        if (ret == 1)
        {
            gIs4GMqttConnected = true;
            SET_SD_ELEMENT_VALUE(SD225, 1);
            LOGW_4G("4G_task", "AT+QMTSUB OK!");
            break;
        }
        else if (ret == 2)
        {
            setCME_ERROR_SD();
        }
        xSemaphoreGive(g4GMutex);
        vTaskDelay(4000);
    }
    xSemaphoreGive(g4GMutex);
    vPortFree(sendBuf);
}

static void mqtt_QMTSUB(void)
{
    mqtt_QMTSUB_do(g_plc_netcfg.mqtt.subscribe_topic);
    if (strcmp(g_plc_netcfg.mqtt.vender, "HANYU") == 0 || strcmp(g_plc_netcfg.mqtt.vender, "DEFAULTMQTT") == 0 || strcmp(g_plc_netcfg.mqtt.vender, "GDMQTT") == 0)
    {
        mqtt_QMTSUB_do(g_plc_netcfg.mqtt.subscribe_topic_reboot);
        mqtt_QMTSUB_do(g_plc_netcfg.mqtt.subscribe_topic_pub_cycle);
        mqtt_QMTSUB_do(g_plc_netcfg.mqtt.subscribe_topic_pub_now);
        mqtt_QMTSUB_do(g_plc_netcfg.mqtt.subscribe_topic_pause);
    }
}

static void mqtt_QMTSUB_Other(void)
{
    char *sendBuf = pvPortMalloc(2048);
    while(1)
    {
        if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
        {
            LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR", __func__);
            vTaskDelay(1000);
            continue;
        }
        sprintf(sendBuf, "AT+QMTSUB=0,1,\"%s\",0", g_plc_netcfg.mqtt.subscribe_topic_reboot);
        sendAT(sendBuf);
        int ret = waitResponse(15000, "OK\r\n\r\n+QMTSUB:", "+CME ERROR:", NULL, NULL, NULL);
        if (ret == 1)
        {
            gIs4GMqttConnected = true;
            SET_SD_ELEMENT_VALUE(SD225, 1);
            LOGW_4G("4G_task", "AT+QMTSUB OK!");
            break;
        }
        else if (ret == 2)
        {
            setCME_ERROR_SD();
        }
        xSemaphoreGive(g4GMutex);
        vTaskDelay(4000);
    }
    xSemaphoreGive(g4GMutex);
    vPortFree(sendBuf);
}

bool isEC20ModuleOK(void)
{
    while(1)
    {
        if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
        {
            LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR", __func__);
            vTaskDelay(1000);
            continue;
        }
        sendAT("AT");
        int ret = waitResponse(15000, "OK", "RDY", NULL, NULL, NULL);
        if (ret == 1 || ret == 2)
        {
            LOGE_4G("4G_task", "4G module OK!");
            vTaskDelay(123);
            break;
        }
        xSemaphoreGive(g4GMutex);
        vTaskDelay(2000);
    }
    xSemaphoreGive(g4GMutex);
    return true;
}

static uint8_t getCPINCode(char *simCode, uint8_t bufSize)
{
    uint8_t code = 0;
    if (strncmp(simCode, "SIM PIN", bufSize) == 0)
    {
        code = 1;
    }
    else if (strncmp(simCode, "SIM PUK", bufSize) == 0)
    {
        code = 2;
    }
    else if (strncmp(simCode, "SIM PIN2", bufSize) == 0)
    {
        code = 3;
    }
    else if (strncmp(simCode, "SIM PUK2", bufSize) == 0)
    {
        code = 4;
    }
    else if (strncmp(simCode, "PH-NET PIN", bufSize) == 0)
    {
        code = 5;
    }
    else if (strncmp(simCode, "PH-NET PUK", bufSize) == 0)
    {
        code = 6;
    }
    else if (strncmp(simCode, "PH-NETSUB PIN", bufSize) == 0)
    {
        code = 7;
    }
    else if (strncmp(simCode, "PH-NETSUB PUK", bufSize) == 0)
    {
        code = 8;
    }
    else if (strncmp(simCode, "PH-SP PIN", bufSize) == 0)
    {
        code = 9;
    }
    else if (strncmp(simCode, "PH-SP PUK", bufSize) == 0)
    {
        code = 10;
    }
    else if (strncmp(simCode, "PH-CORP PIN", bufSize) == 0)
    {
        code = 11;
    }
    else if (strncmp(simCode, "PH-CORP PUK", bufSize) == 0)
    {
        code = 12;
    }
    return code;
}

// simCode looks like: +CPIN: READY
static void getCPINCodeStr(char *simCode)
{
    char *pBuf = (char *)gUart8RecvBuffer;
    char *pCPIN = strstr(pBuf, "+CPIN:");
    LOGV_4G("4G_task", "pCPIN = %s", pCPIN);
    
    char temp[32];
    char temp2[32];
    sscanf(pCPIN, "%s %s\r\n\r\n%s", temp, simCode, temp2);

    LOGD_4G("4G_task", "Leave %s(), temp=%s, simCode=%s, temp2=%s", __func__, temp, simCode, temp2);
}

// +CPIN: READY
bool isSIMCardReady(void)
{
    int retWait;
    uint16_t err = 0;
    while(1)
    {
        if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
        {
            LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR", __func__);
            vTaskDelay(1000);
            continue;
        }
        err = CPIN_BASE + 13; // Time out
        sendAT("AT+CPIN?");
        retWait = waitResponse(15000, "+CPIN:", "+CME ERROR:", NULL, NULL, NULL);
        if (retWait == 1)
        {
            char simCode[32] = {0};
            getCPINCodeStr(simCode);
            if (strncmp(simCode, "READY", sizeof(simCode)) == 0)
            {
                break;
            }
            LOGE_4G("4G_task", "SIM care not READY now!  Try again");
            err = getCPINCode(simCode, sizeof(simCode));
            err += CPIN_BASE;
        }
        else if (retWait == 2) // +CME ERROR: 10
        {
            uint16_t cmeCode = getCME_ERROR_code();
            err = CME_ERROR_BASE + cmeCode;
            LOGE_4G("4G_task", "SIM not inserted. err = %u", err);

            xSemaphoreGive(g4GMutex);
            mqtt_4G_rst();
        }
        
        SET_SD_ELEMENT_VALUE(SD230, err);
        xSemaphoreGive(g4GMutex);
        vTaskDelay(5000);
    }

    SET_SD_ELEMENT_VALUE(SD230, 0);
    xSemaphoreGive(g4GMutex);
    return true;
}

// simCode looks like: +CEREG:0,1
static int getNETRegisterStr(char *simCode)
{
    char *pBuf = (char *)gUart8RecvBuffer;
    char *pCEREG = strstr(pBuf, "+CEREG:");
    LOGV_4G("4G_task", "pCEREG = %s", pCEREG);
    int client_id;
    int errCode;
    char temp[32];
    sscanf(pCEREG, "%s %d,%d", temp, &client_id, &errCode);

    LOGD_4G("4G_task", "Leave %s(), result=%d", __func__, errCode);
    return errCode;

}


// +CEREG? 查询网络注册
bool isNETRegister(void)
{
    int retWait;
    uint16_t err = 0;

    sendAT("AT+CEREG?");
    retWait = waitResponse(15000, "+CEREG:", "0,1", NULL, NULL, NULL);
    if (retWait == 1)
    {
        char simCode[32] = {0};
        int errCode = getNETRegisterStr(simCode);
        if (errCode == 1)
        {
          return true;
        }
        LOGE_4G("4G_task", "%s %u : CEREG ERROR", __func__,errCode);
        err = CCEREG_BASE + errCode;
    }
    else
    {
        LOGE_4G("4G_task", "%s: CEREG state ERROR", __func__);
    }

    SET_SD_ELEMENT_VALUE(SD230, err);
    return false;
}


void ec20_mqtt_publish(char *topic, char *payload, uint8_t qos)
{
    LOGV_4G("4G_task", "Enter %s(), gIs4GMqttConnected = %u, g4GMqttPublishing = %u,gIsReConnecting =%u", __func__, gIs4GMqttConnected, g4GMqttPublishing, gIsReConnecting);

    if (gIs4GMqttConnected == false || gIsReConnecting)
    {
        LOGE_4G("4G_task", "4G MQTT not connected, just return.");
        return;
    }
    if (g4GMqttPublishing)
    {
        LOGE_4G("4G_task", "4G MQTT is publishing, so just return.");
        return;
    }
    g4GMqttPublishing = true;
    uint32_t topicLen = strlen(topic);
    uint32_t payloadLen = strlen(payload);
    mqtt_msg_st mqttMsg;
    mqttMsg.type = 1;
    mqttMsg.qos = qos;
    mqttMsg.topic = pvPortMalloc(topicLen + 2);
    memcpy(mqttMsg.topic, topic, topicLen);
    mqttMsg.topic[topicLen] = 0;
    mqttMsg.topic[topicLen + 1] = 0;

    mqttMsg.dataBuff = pvPortMalloc(payloadLen + 2);
    memcpy(mqttMsg.dataBuff, payload, payloadLen);
    mqttMsg.dataBuff[payloadLen] = 0;
    mqttMsg.dataBuff[payloadLen + 1] = 0;
    mqttMsg.msgLength = payloadLen;
    BaseType_t ret = xQueueSendToBack(g4GMqttMsgQueue, &mqttMsg, 100);
    if (ret == pdFALSE)
    {
        vPortFree(mqttMsg.topic);
        vPortFree(mqttMsg.dataBuff);
        LOGE_4G("4G_task", "xQueueSendToBack ERROR!");
        g4GMqttPublishing = false;
    }
    LOGV_4G("4G_task", "xQueueSendToBack return %d\r\n", ret);
}

static void ec20_mqtt_publish_do(char *topic, char *payload, uint16_t length, uint8_t qos)
{
    LOGV_4G("4G_task", "Enter %s(), gIs4GMqttConnected = %u, g4GMqttPublishing = %u", __func__, gIs4GMqttConnected, g4GMqttPublishing);
    if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
    {
        g4GMqttPublishing = false;
        LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR", __func__);
        return;
    }
    char *atSend = pvPortMalloc(AT_SEND_MAX_BYTE);
    uint16_t payloadLen = length + 2;
    if(payloadLen > AT_SEND_MAX_BYTE)
    {
        LOGE_4G("4G_task", "payloadLen = %d > %d", payloadLen, AT_SEND_MAX_BYTE);
        //SET_SD_ELEMENT_VALUE(SD230, 1);
        payloadLen = AT_SEND_MAX_BYTE;
    }
    sprintf(atSend, "AT+QMTPUBEX=0,0,%u,0,\"%s\",%u\r\n", qos, topic, payloadLen);
    LOGD_4G("4G_task", "topic = %s", topic);
    init_uart8_receive_buffer();
    uart8_send_buffer(atSend, strlen(atSend));
    int retWait = waitResponsePublish(2000, ">", "+CME ERROR:", "ERROR", NULL, NULL);
    //vTaskDelay(100);
    if (retWait == 1) // Got ">"
    {
        LOGW_4G("4G_task", "retWait == 1");
        init_uart8_receive_buffer();
        sprintf(atSend, "%s\r\n", payload);
        uart8_send_buffer(atSend, payloadLen);
        retWait = waitResponsePublish(2500, "OK\r\n\r\n+QMTPUBEX:", "+CME ERROR:", NULL, NULL, NULL);
        if (retWait == 1)
        {
            LOGI_4G("4G_task", "4G mqtt publish success!");
        }
        else if (retWait == 2)
        {
            LOGE_4G("4G_task", "4G mqtt publish ERROR!");
            setCME_ERROR_SD();
        }
        else
        {
            SET_SD_ELEMENT_VALUE(SD230, QMTPUBEX_BASE+2);
        }
    }
    else if (retWait == 2) // Got "+CME ERROR:"
    {
        LOGW_4G("4G_task", "retWait == 2");
        LOGE_4G("4G_task", "4G mqtt publish ERROR!");
        setCME_ERROR_SD();
    }
    else if (retWait == 3) // Got "ERROR"
    {
        LOGW_4G("4G_task", "retWait == 3");
        gIs4GMqttConnected = false;
        SET_SD_ELEMENT_VALUE(SD225, 0);
        g4GMqttPublishing = false;
        init_uart8_receive_buffer();
        //strcpy((char*)gUart8RecvBuffer, "+QMTSTAT: 0,1");
        //notify_QMTSTAT("+QMTSTAT: 0,1");
        SET_SD_ELEMENT_VALUE(SD230, QMTPUBEX_BASE);
    }
    else // Time out
    {
        LOGW_4G("4G_task", "retWait == 0");
        #if 0
        gIs4GMqttConnected = false;
        SET_SD_ELEMENT_VALUE(SD225, 0);
        g4GMqttPublishing = false;
        init_uart8_receive_buffer();
        //notify_QMTSTAT("+QMTSTAT: 0,1");
        #endif
        SET_SD_ELEMENT_VALUE(SD230, QMTPUBEX_BASE+1);
    }
    vPortFree(atSend);
    g4GMqttPublishing = false;
    init_uart8_receive_buffer();
    xSemaphoreGive(g4GMutex);
    LOGW_4G("4G_task", "Leave %s()", __func__);
}

void closeATEchoMode(void)
{
AGAIN:
    if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
    {
        LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR", __func__);
        vTaskDelay(1000);
        goto AGAIN;
    }
    sendAT("ATE0");
    if (waitResponse(10000, "OK", NULL, NULL, NULL, NULL) == 1)
    {
        LOGI_4G("4G_task", "AT echo mode closed");
    }
    xSemaphoreGive(g4GMutex);
}

// +QGPS: 1
static int getQGPS_result(void)
{
    char *pBuf = (char *)gUart8RecvBuffer;
    char *pQGPS = strstr(pBuf, "+QGPS:");
    char at[16] = {0};
    int result;
    sscanf(pQGPS, "%s %d", at, &result);

    LOGV_4G("4G_task", "Leave %s(), result = %d", __func__, result);
    return result;
}

static void gps_QGPS(void)
{
    sendAT("AT+QGPS?");
    int retWait = waitResponse(15000, "+QGPS:", NULL, NULL, NULL, NULL);
    if (retWait == 1)
    {
        int result = getQGPS_result();
        if (result == 0)
        {
            sendAT("AT+QGPS=1");
            retWait = waitResponse(5000, "OK", "+CME ERROR:", NULL, NULL, NULL);
            if (retWait == 2)
            {
                uint16_t cmeCode = getCME_ERROR_code();
                SET_SD_ELEMENT_VALUE(SD230, cmeCode + CME_GPS_ERROR_BASE);
            }
        }
    }
}

// +QGPSCFG: "nmeasrc",1
static int getQGPSCFG_nmeasrc_result(void)
{
    char *pBuf = (char *)gUart8RecvBuffer;
    char *pComma = strstr(pBuf, ",");
    LOGV_4G("4G_task", "pComma = %s", pComma);
    pComma++;
    int result = atoi(pComma);
    LOGV_4G("4G_task", "Leave %s(), result = %d", __func__, result);
    return result;
}

static void gps_QGPSCFG(void)
{
    sendAT("AT+QGPSCFG=\"nmeasrc\"");
    int retWait = waitResponse(5000, "+QGPSCFG:", NULL, NULL, NULL, NULL);
    if (retWait == 1)
    {
        int result = getQGPSCFG_nmeasrc_result();
        if (result == 0)
        {//Enable nmeasrc, obtain NMEA sentences by AT+QGPSGNMEA
            sendAT("AT+QGPSCFG=\"nmeasrc\",1");
            retWait = waitResponse(5000, "OK:", "+CME ERROR:", NULL, NULL, NULL);
            if (retWait == 2)
            {
                uint16_t cmeCode = getCME_ERROR_code();
                SET_SD_ELEMENT_VALUE(SD230, cmeCode + CME_GPS_ERROR_BASE);
            }
        }
    }
}

//             $GPGGA,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,M,<10>,M,<11>,<12>*hh<CR><LF>
// +QGPSGNMEA: $GPGGA,,,,,,0,,,,,,,,*66
// +QGPSGNMEA: $GPGGA,103647.0,3150.721154,N,11711.925873,E,1,02,4.7,59.8,M,-2.0,M,96,0297*77
/**
$GPGGA,
103647.0,   //UTC时间，hhmmss（时分秒）格式
3150.721154,//纬度ddmm.mmmm（度分）格式（前面的0也将被传输）
N,          //纬度半球N（北半球）或S（南半球） 
11711.925873,//经度dddmm.mmmm（度分）格式（前面的0也将被传输）
E,          //经度半球E（东经）或W（西经） 
1,          //GPS状态：0=未定位，1=非差分定位，2=差分定位，6=正在估算 
02,         //正在使用解算位置的卫星数量（00~12）（前面的0也将被传输）
4.7,        //HDOP水平精度因子（0.5~99.9）
59.8,       //海拔高度（-9999.9~99999.9）
M,
-2.0,       //地球椭球面相对大地水准面的高度
M,          //差分时间（从最近一次接收到差分信号开始的秒数，如果不是差分定位将为空）
,           //差分站ID号0000~1023（前面的0也将被传输，如果不是差分定位将为空）
*77
***/
static void getQGPSGNMEA_GGA(char *pGGAs)
{
    LOGI_4G("4G_task", "Enter %s()", __func__);
#if 0
    char *pBuf = pGGAs;
#else
    char *pBuf = (char *)gUart8RecvBuffer;
#endif
    char *pEnd = strstr(pBuf, "OK");
    if (pEnd == NULL)
    {
        LOGE_4G("4G_task", "There is no OK, just return");
        return;
    }

    char theStr[512] = {0};
    memset(theStr, 0, sizeof(theStr));
    pEnd = strstr(pBuf, "$GPGGA,");
    //LOGD_4G("4G_task", "pEnd = %s", pEnd);
    pEnd = strstr(pEnd, "\r\n");
    //LOGV_4G("4G_task", "pEnd = %s", pEnd);

    memcpy(theStr, pBuf, pEnd - pBuf);
    char *pGGA = strstr(theStr, "$GPGGA,");
    if (pGGA == NULL)
    {
        LOGE_4G("4G_task", "pGGA == NULL, just return");
        return;
    }
    pGGA += 7;
    
    LOGV_4G("4G_task", "pGGA1 = %s", pGGA);
    double utc, longitude, latitude, HDOP, altitude, ellipseHeight;
    char NS[16] = {0};
    char EW[16] = {0};
    char M1[16] = {0};
    char M2[16] = {0};
    //char end[32];
    int gpsState, satelliteCount, chafenTime, chafenStation, endCode;
#if 0
    char tar[512] = {0};
    pEnd = strstr(pGGA, "*");
    int len = pEnd - pGGA;
    memcpy(tar, pGGA, len);
    LOGV_4G("4G_task", "tar = %s", tar);
    int iN, iE, iM1, iM2;
    sscanf(tar, "%lf,%lf,%d,%lf,%d,%d,%d,%lf,%lf,%d,%lf,%d,%d,%d", &utc, &longitude, &iN, &latitude, &iE, &gpsState, &satelliteCount, &HDOP, &altitude, &iM1, &ellipseHeight, &iM2, &chafenTime, &chafenStation);
    hexdump(NS, 16);
    hexdump(EW, 16);
    hexdump(M1, 16);
    hexdump(M2, 16);
    //LOGW_4G("4G_task", "iN = %d, iE = %d, iM1 = %d, iM2 = %d", iN, iE, iM1, iM2);
#else
    /* 103647.0,3150.721154,N,11711.925873,E,1,02,4.7,59.8,M,-2.0,M,96,0297*77  */
    char tempStr[32] = {0};
    pEnd = strstr(pGGA, ",");
    if (pEnd == NULL) return;
    int len = pEnd - pGGA;// utc长度
    memcpy(tempStr, pGGA, len);
    uint32_t utc32 = atoi(tempStr);

    pGGA = pEnd + 1;// pGGA指向了纬度
    //LOGD_4G("4G_task", "pGGA2 = %s", pGGA);
    pEnd = strstr(pGGA, ",");
    if (pEnd == NULL) return;
    len = pEnd - pGGA;
    memset(tempStr, 0, sizeof(tempStr));
    memcpy(tempStr, pGGA, len);
    latitude = atof(tempStr);

    pGGA = pEnd + 1;// pGGA指向了N(S)
    //LOGI_4G("4G_task", "pGGA3 = %s", pGGA);
    pEnd = strstr(pGGA, ",");
    if (pEnd == NULL) return;
    len = pEnd - pGGA;
    memset(tempStr, 0, sizeof(tempStr));
    memcpy(tempStr, pGGA, len);
    memcpy(NS, tempStr, len);

    pGGA = pEnd + 1;// pGGA指向了经度
    //LOGW_4G("4G_task", "pGGA4 = %s", pGGA);
    pEnd = strstr(pGGA, ",");
    if (pEnd == NULL) return;
    len = pEnd - pGGA;
    memset(tempStr, 0, sizeof(tempStr));
    memcpy(tempStr, pGGA, len);
    longitude = atof(tempStr);

    pGGA = pEnd + 1;// pGGA指向了W(E)
    //LOGE_4G("4G_task", "pGGA5 = %s", pGGA);
    pEnd = strstr(pGGA, ",");
    if (pEnd == NULL) return;
    len = pEnd - pGGA;
    memset(tempStr, 0, sizeof(tempStr));
    memcpy(tempStr, pGGA, len);
    memcpy(EW, tempStr, len);

    pGGA = pEnd + 1;// pGGA指向了：GPS状态：0=未定位，1=非差分定位，2=差分定位，6=正在估算 
    //LOGV_4G("4G_task", "pGGA6 = %s", pGGA);
    pEnd = strstr(pGGA, ",");
    if (pEnd == NULL) return;
    len = pEnd - pGGA;
    memset(tempStr, 0, sizeof(tempStr));
    memcpy(tempStr, pGGA, len);
    gpsState = atoi(tempStr);

    pGGA = pEnd + 1;// pGGA指向了：正在使用解算位置的卫星数量（00~12）（前面的0也将被传输）
    //LOGD_4G("4G_task", "pGGA7 = %s", pGGA);
    pEnd = strstr(pGGA, ",");
    if (pEnd == NULL) return;
    len = pEnd - pGGA;
    memset(tempStr, 0, sizeof(tempStr));
    memcpy(tempStr, pGGA, len);
    satelliteCount = atoi(tempStr);

    pGGA = pEnd + 1;// pGGA指向了：HDOP水平精度因子（0.5~99.9）
    LOGI_4G("4G_task", "pGGA8 = %s", pGGA);
    pEnd = strstr(pGGA, ",");
    if (pEnd == NULL) return;
    len = pEnd - pGGA;
    memset(tempStr, 0, sizeof(tempStr));
    memcpy(tempStr, pGGA, len);
    HDOP = atof(tempStr);

    pGGA = pEnd + 1;// pGGA指向了：海拔高度（-9999.9~99999.9）
    //LOGW_4G("4G_task", "pGGA9 = %s", pGGA);
    pEnd = strstr(pGGA, ",");
    if (pEnd == NULL) return;
    len = pEnd - pGGA;
    memset(tempStr, 0, sizeof(tempStr));
    memcpy(tempStr, pGGA, len);
    altitude = atof(tempStr);

    pGGA = pEnd + 1;
    pEnd = strstr(pGGA, ",");
    if (pEnd == NULL) return;
    pGGA = pEnd + 1;// pGGA指向了：地球椭球面相对大地水准面的高度
    //LOGE_4G("4G_task", "pGGA10 = %s", pGGA);
    pEnd = strstr(pGGA, ",");
    if (pEnd == NULL) return;
    len = pEnd - pGGA;
    memset(tempStr, 0, sizeof(tempStr));
    memcpy(tempStr, pGGA, len);
    ellipseHeight = atof(tempStr);

    // 103647.0,3150.721154,N,11711.925873,E,1,02,4.7,59.8,M,-2.0,M,96,0297*77
    pGGA = pEnd + 1;
    pEnd = strstr(pGGA, ",");
    if (pEnd == NULL) return;
    pGGA = pEnd + 1;// pGGA指向了：差分时间（从最近一次接收到差分信号开始的秒数，如果不是差分定位将为空）
    //LOGV_4G("4G_task", "pGGA11 = %s", pGGA);
    pEnd = strstr(pGGA, ",");
    if (pEnd == NULL) return;
    len = pEnd - pGGA;
    memset(tempStr, 0, sizeof(tempStr));
    memcpy(tempStr, pGGA, len);
    chafenTime = atoi(tempStr);

    pGGA = pEnd + 1;// pGGA指向了：差分站ID号0000~1023（前面的0也将被传输，如果不是差分定位将为空）
    //LOGD_4G("4G_task", "pGGA12 = %s", pGGA);
    pEnd = strstr(pGGA, "*");
    if (pEnd == NULL) return;
    len = pEnd - pGGA;
    memset(tempStr, 0, sizeof(tempStr));
    memcpy(tempStr, pGGA, len);
    chafenStation = atoi(tempStr);
#endif
    //vTaskDelay(900);
    //LOGD_4G("4G_task", "utc=%f,longitude=%f,NS=%s,latitude=%f,EW=%s,gpsState=%d,satelliteCount=%d,HDOP=%f,altitude=%f,M1=%s,ellipseHeight=%f,M2=%s,chafenTime=%d,chafenStation=%d,endCode=%d", utc, longitude, NS, latitude, EW, gpsState, satelliteCount, HDOP, altitude, M1, ellipseHeight, M2, chafenTime, chafenStation, endCode);
    LOGV_4G("4G_task", "longitude=%f,latitude=%f", longitude, latitude);
    LOGD_4G("4G_task", "HDOP=%f, altitude=%f", HDOP, altitude);
    LOGI_4G("4G_task", "gpsState=%d, satelliteCount=%d", gpsState, satelliteCount);
    LOGW_4G("4G_task", "ellipseHeight=%f, utc = %u", ellipseHeight, utc32);
    LOGI_4G("4G_task", "chafenTime=%d, chafenStation=%d", chafenTime, chafenStation);
    hexdump(NS, 16);
    hexdump(EW, 16);
    gGpsValue.gpsState = gpsState;
    gGpsValue.satelliteCount = satelliteCount;
    gGpsValue.latitude = latitude;
    gGpsValue.longitude = longitude;
    gGpsValue.altitude = altitude;
}

// +CSQ: 19,99
static int getCSQ_result(void)
{
    char *pBuf = (char *)gUart8RecvBuffer;
    char *pCSQ = strstr(pBuf, "+CSQ:");
    if (pCSQ == NULL)
    {
        return 0;
    }
    char at[16] = {0};
    int rssi, ber;
    sscanf(pCSQ, "%s %d,%d", at, &rssi, &ber);

    LOGV_4G("4G_task", "Leave %s(), rssi = %d, ber = %d", __func__, rssi, ber);
    return rssi;
}

static inline int map(int x, int in_min, int in_max, int out_min, int out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static void setRSSI(uint16_t RSSI)
{
    int16_t realRssi = 0;
    if (RSSI == 0)//-113dBm or less
    {
        realRssi = -133;
    }
    else if (RSSI == 1) // -111dBm
    {
        realRssi = -111;
    }
    else if (RSSI >= 2 && RSSI <= 30)// -109dBm...-53dBm
    {
        realRssi = map(RSSI, 2, 30, -109, -53);
    }
    else if (RSSI == 31)// -51dBm or greater
    {
        realRssi = -51;
    }
    else if (RSSI == 99)// Not known or not detectable
    {
        realRssi = 0;
    }
    else if (RSSI == 100)// -116dBm or less
    {
        realRssi = -116;
    }
    else if (RSSI == 101)// -115dBm
    {
        realRssi = -115;
    }
    else if (RSSI >= 102 && RSSI <= 190) // -114dBm…-26dBm
    {
        realRssi = map(RSSI, 102, 190, -114, -26);
    }
    else if (RSSI == 191)// -25dBm or greater
    {
        realRssi = -25;
    }
    else if (RSSI == 199)// Not known or not detectable
    {
        realRssi = 0;
    }
    if (realRssi == 0)
    {
        SET_SD_ELEMENT_VALUE(SD226, 0xFFFF);
        SET_SD_ELEMENT_VALUE(SD227, 0);
    }
    else
    {
        SET_SD_ELEMENT_VALUE(SD226, realRssi);
        SET_SD_ELEMENT_VALUE(SD227, 1);
    }
    LOGI_4G("4G_task", "Leave %s(), realRssi = %d", __func__, realRssi);
}

static void AT_CSQ(void)
{
    LOGV_4G("4G_task", "Enter %s()", __func__);
    if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
    {
        LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR", __func__);
        return;
    }
    sendAT("AT+CSQ");
    int retWait = waitResponse(5000, "+CSQ:", "+CME ERROR:", NULL, NULL, NULL);
    if (retWait== 1)
    {
        uint16_t rssi = getCSQ_result();
        setRSSI(rssi);
    }
    else if (retWait == 2)
    {
        uint16_t cmeCode = getCME_ERROR_code();
        SET_SD_ELEMENT_VALUE(SD230, cmeCode + CME_ERROR_BASE);
    }
    
    xSemaphoreGive(g4GMutex);
    LOGI_4G("4G_task", "Leave %s()", __func__);
}

static void gps_QGPSGNMEA_GGA(void)
{
    static uint32_t counts = 0;
    LOGV_4G("4G_task", "Enter %s()", __func__);
    if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
    {
        LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR", __func__);
        return;
    }
    
    int retWait;
    uint16_t err = 0;
    while(1)
    {
        sendAT("AT+QGPSGNMEA=\"GGA\"");
        retWait = waitResponse(5000, "+QGPSGNMEA:", "+CME ERROR:", NULL, NULL, NULL);
        if (retWait == 1)
        {
            char GGA[512] = "+QGPSGNMEA: $GPGGA,103647.0,3150.721154,N,11711.925873,E,1,02,4.7,59.8,M,-2.0,M,96,0297*77";
            if (counts++ % 2 == 0)
            {
                strcpy(GGA, "+QGPSGNMEA: $GPGGA,103647.0,3150.721154,N,11711.925873,E,1,02,4.7,59.8,M,-2.0,M,96,0297*77");
            }
            else
            {
                strcpy(GGA, "+QGPSGNMEA: $GPGGA,,,,,,0,,,,,,,,*66");
            }
            getQGPSGNMEA_GGA(GGA);
            //LOGD_4G("4G_task", "GGA = %s", GGA);
            break;
        }
        else if (retWait == 2) // +CME ERROR: 10
        {
            uint16_t cmeCode = getCME_ERROR_code();
            err = CME_GPS_ERROR_BASE + cmeCode;
            SET_SD_ELEMENT_VALUE(SD230, err);
            break;
        }
        else
        {
            err = CME_GPS_ERROR_BASE; // Time out error
            SET_SD_ELEMENT_VALUE(SD230, err);
            break;
        }
    }

    xSemaphoreGive(g4GMutex);
    LOGV_4G("4G_task", "Leave %s()", __func__);
}

/*
+QGPSLOC: 102957.0,39.97258,116.43717,0.6,91.0,2,330.21,0.0,0.0,151119,07

OK

*/
static void setQGPSLOC_result(void)
{
    LOGI_4G("4G_task", "Enter %s()", __func__);
    char *pBuf = (char *)gUart8RecvBuffer;
    char *pEnd = strstr(pBuf, "OK");
    if (pEnd == NULL)
    {
        LOGE_4G("4G_task", "There is no OK, just return");
        return;
    }

    char theStr[512] = {0};
    memset(theStr, 0, sizeof(theStr));
    char *pLOC = strstr(pBuf, "+QGPSLOC:");
    if (pLOC == NULL)
    {
        LOGE_4G("4G_task", "There is no '+QGPSLOC:', just return");
        return;
    }
    pLOC += 9;// 102957.0,39.97258,116.43717,0.6,91.0,2,330.21,0.0,0.0,151119,07
    LOGV_4G("4G_task", "pLOC1 = %s", pLOC);

    char tempStr[32] = {0};
    float latitude;
    float longitude;
    float hdop;//Horizontal Precision, 0.5-99.9
    float altitude;
    int fix;//2=2D positioning, 3=3D positioning
    float cog;//Ground heading based on true north. ddd.mm(degree:minute)
    float spkm;//Speed over ground. Km/h
    float spkn;//Speed over ground. knots
    int nsat;//Number of satellites, from 00 to 12
    
    pLOC = strstr(pLOC, ",");
    if (pLOC == NULL)
    {
        LOGE_4G("4G_task", "error return 0");
        return;
    }
    pLOC++;//39.97258,116.43717,0.6,91.0,2,330.21,0.0,0.0,151119,07
    pEnd = strstr(pLOC, ",");
    if (pEnd == NULL) 
    {
        LOGE_4G("4G_task", "error return 1");
        return;
    }
    int len = pEnd - pLOC;// 纬度长度
    memcpy(tempStr, pLOC, len);
    latitude = atof(tempStr);

    pLOC = pEnd + 1;//116.43717,0.6,91.0,2,330.21,0.0,0.0,151119,07
    pEnd = strstr(pLOC, ",");
    if (pEnd == NULL) 
    {
        LOGE_4G("4G_task", "error return 2");
        return;
    }
    len = pEnd - pLOC;// 经度长度
    memset(tempStr, 0, sizeof(tempStr));
    memcpy(tempStr, pLOC, len);
    longitude = atof(tempStr);

    pLOC = pEnd + 1;//0.6,91.0,2,330.21,0.0,0.0,151119,07
    pEnd = strstr(pLOC, ",");
    if (pEnd == NULL) 
    {
        LOGE_4G("4G_task", "error return 3");
        return;
    }
    len = pEnd - pLOC;// HDOP长度
    memset(tempStr, 0, sizeof(tempStr));
    memcpy(tempStr, pLOC, len);
    hdop = atof(tempStr);

    pLOC = pEnd + 1;//91.0,2,330.21,0.0,0.0,151119,07
    pEnd = strstr(pLOC, ",");
    if (pEnd == NULL) 
    {
        LOGE_4G("4G_task", "error return 4");
        return;
    }
    len = pEnd - pLOC;// 海拔高度长度
    memset(tempStr, 0, sizeof(tempStr));
    memcpy(tempStr, pLOC, len);
    altitude = atof(tempStr);

    pLOC = pEnd + 1;//2,330.21,0.0,0.0,151119,07
    pEnd = strstr(pLOC, ",");
    if (pEnd == NULL) 
    {
        LOGE_4G("4G_task", "error return 5");
        return;
    }
    len = pEnd - pLOC;// fix长度
    memset(tempStr, 0, sizeof(tempStr));
    memcpy(tempStr, pLOC, len);
    fix = atoi(tempStr);

    pLOC = pEnd + 1;//330.21,0.0,0.0,151119,07
    pEnd = strstr(pLOC, ",");
    if (pEnd == NULL) 
    {
        LOGE_4G("4G_task", "error return 6");
        return;
    }
    len = pEnd - pLOC;// cog长度
    memset(tempStr, 0, sizeof(tempStr));
    memcpy(tempStr, pLOC, len);
    cog = atof(tempStr);

    pLOC = pEnd + 1;//0.0,0.0,151119,07
    pEnd = strstr(pLOC, ",");
    if (pEnd == NULL) 
    {
        LOGE_4G("4G_task", "error return 7");
        return;
    }
    len = pEnd - pLOC;// spkm长度
    memset(tempStr, 0, sizeof(tempStr));
    memcpy(tempStr, pLOC, len);
    spkm = atof(tempStr);

    pLOC = pEnd + 1;//0.0,151119,07
    pEnd = strstr(pLOC, ",");
    if (pEnd == NULL) 
    {
        LOGE_4G("4G_task", "error return 8");
        return;
    }
    len = pEnd - pLOC;// spkn长度
    memset(tempStr, 0, sizeof(tempStr));
    memcpy(tempStr, pLOC, len);
    spkn = atof(tempStr);

    pLOC = pEnd + 1;//151119,07
    pEnd = strstr(pLOC, ",");
    pLOC = pEnd + 1;//07
    pEnd = strstr(pLOC, "\r\n");
    if (pEnd == NULL) 
    {
        LOGE_4G("4G_task", "error return 9");
        return;
    }
    len = pEnd - pLOC;// nsat长度
    memset(tempStr, 0, sizeof(tempStr));
    memcpy(tempStr, pLOC, len);
    nsat = atoi(tempStr);
    LOGV_4G("4G_task", "longitude=%f,latitude=%f", longitude, latitude);
    LOGD_4G("4G_task", "hdop=%.1f, altitude=%.1f", hdop, altitude);
    LOGI_4G("4G_task", "fix=%d, satelliteCount=%d", fix, nsat);
    LOGW_4G("4G_task", "cog=%.2f, spkm=%.1f, spkn=%.1f", cog, spkm, spkn);
    SET_SD_ELEMENT_VALUE(SD216, nsat);
    float *pDValue = (float *)&gtv_PlcElement.msp_SDElement[SD217];
    *pDValue = altitude;
    pDValue = (float *)&gtv_PlcElement.msp_SDElement[SD219];
    *pDValue = longitude;
    pDValue = (float *)&gtv_PlcElement.msp_SDElement[SD221];
    *pDValue = latitude;
    gGpsValue.gpsState = 1;
    gGpsValue.satelliteCount = nsat;
    gGpsValue.latitude = latitude;
    gGpsValue.longitude = longitude;
    gGpsValue.altitude = altitude;
}

static void AT_QGPSLOC(void)
{
    LOGV_4G("4G_task", "Enter %s()", __func__);
    if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
    {
        LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR, just return.", __func__);
        return;
    }

    sendAT("AT+QGPSLOC=2");
    int retWait = waitResponse(5000, "+QGPSLOC:", "+CME ERROR:", NULL, NULL, NULL);
    if (retWait== 1)
    {
        SET_SD_ELEMENT_VALUE(SD215, 1);
        setQGPSLOC_result();
    }
    else if (retWait == 2)
    {
        uint16_t cmeCode = getCME_ERROR_code();
        SET_SD_ELEMENT_VALUE(SD230, cmeCode + CME_GPS_ERROR_BASE);
        SET_SD_ELEMENT_VALUE(SD215, 0);
        SET_SD_ELEMENT_VALUE(SD216, 0);
        float *pDValue = (float *)&gtv_PlcElement.msp_SDElement[SD217];
        *pDValue = 0.0;
        pDValue = (float *)&gtv_PlcElement.msp_SDElement[SD219];
        *pDValue = 0.0;
        pDValue = (float *)&gtv_PlcElement.msp_SDElement[SD221];
        *pDValue = 0.0;
        gGpsValue.gpsState = 0;
        gGpsValue.satelliteCount = 0;
        gGpsValue.latitude = 0;
        gGpsValue.longitude = 0;
    }
    
    xSemaphoreGive(g4GMutex);
    LOGI_4G("4G_task", "Leave %s()", __func__);
}

static void gps_task(void *p_arg)
{
    LOGV_4G("4G_task", "gps_task RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());
    while(1)
    {
        vTaskDelay(59000);
        //gps_QGPSGNMEA_GGA();
    #if (KALYKE_FEATURE_4G_TCP_TASK == 1)
        while (g4GTCPBusy == true)
        {
            vTaskDelay(5000);
        }
    #endif
        AT_QGPSLOC();
        AT_CSQ();

    }
}

static void ec20_GPS_init(void)
{
    LOGI_4G("4G_task", "Enter %s()", __func__);
    isEC20ModuleOK();
    gps_QGPSCFG();
    gps_QGPS();
    xTaskCreate((TaskFunction_t)gps_task,
                (const char *)"gps_task",
                1024,
                (void *)NULL,
                TASK_4G_PRIO,
                NULL);
}

static void kalyke_4G_sntp_task(void *p_arg)
{
    LOGV_4G("4G_task", "kalyke_4G_sntp_task RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());
    vTaskDelay(1000);
    ec20_sntp_init();
    vTaskDelete(NULL);
}

static void ec20_MQTT_init(void)
{
    LOGI_4G("4G_task", "Enter %s()", __func__);
    if (g_plc_netcfg.mqtt.isParsed == 0)
    {
        xEventGroupSetBits(g_kalyke_event_group, KALYKE_EVENT_TCP_WAIT_4G_MQTT);
    }
    while (g_plc_netcfg.mqtt.isParsed == 0)
    {
        LOGW_4G("4G_task", "MQTT config is not parsed, waiting...");
        vTaskDelay(5000);
    }
    isEC20ModuleOK();

    closeATEchoMode();    
    
    isSIMCardReady();

//    check_QIACT();

//    if (check_QIACT() == false || isNETRegister() == false)
//    {
//        LOGE_4G("4G_task", "check_QIACT or NETRegister err...");
//        at_QIACT_mqtt();
//        vTaskDelay(5000);
//    }
    mqtt_QMTCFG();

    vTaskDelay(2000);
    if (mqtt_QMTOPEN() == 1)
    {
        ec20_sntp_init();
        return;
    }
    
    mqtt_QMTCONN();

    mqtt_QMTSUB();

    //ec20_sntp_init();
    xTaskCreate((TaskFunction_t)kalyke_4G_sntp_task,
                (const char *)"SNTP4Gtask",
                TASK_4G_STACK_SIZE,
                (void *)NULL,
                TASK_4G_PRIO,
                NULL);
    vTaskDelay(2000);  //wait sntp
    LOGI_4G("4G_task", "Leave %s()", __func__);
}


static void ec20_HTTP_init(void)
{
    LOGI_4G("4G_task", "Enter %s()", __func__);
    if (g_plc_netcfg.mqtt.isParsed == 0)
    {
        xEventGroupSetBits(g_kalyke_event_group, KALYKE_EVENT_TCP_WAIT_4G_MQTT);
    }
    while (g_plc_netcfg.mqtt.isParsed == 0)
    {
        LOGW_4G("4G_task", "MQTT config is not parsed, waiting...");
        vTaskDelay(5000);
    }
    isEC20ModuleOK();

    closeATEchoMode();    
    
    isSIMCardReady();
    
    http_QHTTPCFG();

    check_QIACT();

    if (check_QIACT() == false || isNETRegister() == false)
    {
        LOGE_4G("4G_task", "check_QIACT or NETRegister err...");
        at_QIACT_mqtt();
        vTaskDelay(5000);
    }
    
//    http_QHTTPCFG();
//    vTaskDelay(2000);
    xTaskCreate((TaskFunction_t)kalyke_4G_sntp_task,
                (const char *)"SNTP4Gtask",
                TASK_4G_STACK_SIZE,
                (void *)NULL,
                TASK_4G_PRIO,
                NULL);
    vTaskDelay(2000);  //wait sntp
    
    LOGI_4G("4G_task", "Leave %s()", __func__);
}

static void at_QIDEACT_mqtt(void)
{
    LOGV("4G_task", "Enter %s()", __func__);
#if 0
    sendAT("AT+QIDEACT=1");
    sendAT("AT+QIDEACT=2");
    sendAT("AT+QIDEACT=3");
#else
    char sendBuf[32];
    sprintf(sendBuf, "AT+QIDEACT=%u", 0);
    sendAT(sendBuf);
    int ret = waitResponse(30000, "OK", "ERROR", NULL, NULL, NULL);
    if (ret == 1)
    {
        LOGD("4G_task", "QIDEACT success");
    }
    else if (ret == 2)
    {
        LOGE("4G_task", "QIDEACT failed");
    }
#endif
}

void at_QIACT_mqtt(void)
{
    LOGV("4G_task", "Enter %s()", __func__);
#if 0
    sendAT("AT+QIDEACT=1");
    sendAT("AT+QIDEACT=2");
    sendAT("AT+QIDEACT=3");
#else
    char sendBuf[32];
    sprintf(sendBuf, "AT+QIACT=%u", 1);
    sendAT(sendBuf);
    int ret = waitResponse(40000, "OK", "ERROR", NULL, NULL, NULL);
    if (ret == 1)
    {
        LOGD("4G_task", "QIACT success");
    }
    else if (ret == 2)
    {
        LOGE("4G_task", "QIDEACT failed");
    }
#endif
}

static bool check_QIACT_mqtt(void)
{
    LOGV("4G_task", "Enter %s()", __func__);
    char sendBuf[16];
    bool retVal = true;
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
            retVal = false;
        }
        break;
    }
    return retVal;
}

static void at_CFUN(uint32_t fun)
{
    LOGV_4G("4G_task", "Enter %s(), fun = %u", __func__, fun);
    char sendBuf[32];
    sprintf(sendBuf, "AT+CFUN=%u", fun);
    sendAT(sendBuf);
    int ret = waitResponse(15000, "OK", "+CME ERROR", NULL, NULL, NULL);
    if (ret == 1)
    {
        LOGD("4G_task", "%s success", sendBuf);
    }
    else if (ret == 2)
    {
        LOGE("4G_task", "%s failed", sendBuf);
        setCME_ERROR_SD();
    }
}


static void handleErrorQMTSTATE(uint8_t errCode)
{
    if (gIsReConnecting)
    {
        return;
    }
    gIsReConnecting = true;
    SET_SD_ELEMENT_VALUE(266, 1);
    LOGE_4G("4G_task", "Enter %s(), errCode = %u", __func__, errCode);

    gIs4GMqttConnected = false;
    int nTimes = 0;
    while(gIs4GMqttConnected == false)
    {
         nTimes++;
         LOGE_4G("4G_task", "Again %s(), nTimes = %u", __func__, nTimes);
         if (nTimes > 1)
         {
             isEC20ModuleOK();
             closeATEchoMode();
             isSIMCardReady();
             check_QIACT();
             mqtt_QMTCFG();
         }
         SET_SD_ELEMENT_VALUE(SD225, 0);
         vTaskDelay(15000);
         if (mqtt_QMTOPEN2(nTimes) == 0)
         {
             continue;
         }
         if (mqtt_QMTCONN() == 0)
         {
             continue;
         }
         //在mqtt_QMTSUB内置为true，还有其他置位，在这里置位会出现4G灯ok，但不能publish数据的bug.
         //gIs4GMqttConnected = true;//此时已经连接MQTT服务器成功了，所以置为true
         mqtt_QMTSUB();
    }

    SET_SD_ELEMENT_VALUE(266, 0);
    gIsReConnecting = false;

    LOGI_4G("4G_task", "Leave %s(), errCode = %u", __func__, errCode);
}


// +QMTSTAT: 0,1
#if 1
static uint8_t getQMTSTATErrorCode(char *atRecv)
{
    char *pBuf = (char *)atRecv;
    char *pSTAT = strstr(pBuf, "+QMTSTAT:");
    char at[64] = {0};
    int client_id;
    int errCode;
    sscanf(pSTAT, "%s %d,%d", at, &client_id, &errCode);

    //LOGV_4G("4G_task", "Leave %s(), %s %d,%d", __func__, at, client, errCode);
    LOGV_4G("4G_task", "Leave %s(), errCode = %d", __func__, errCode);
    return errCode;
}
#else
static uint8_t getQMTSTATErrorCode(void)
{
    char *pBuf = (char *)gUart4RecvBuffer;
    char *pSTAT = strstr(pBuf, "+QMTSTAT:");
    char at[64] = {0};
    int client_id;
    int errCode;
    sscanf(pSTAT, "%s %d,%d", at, &client_id, &errCode);

    //LOGV_4G("4G_task", "Leave %s(), %s %d,%d", __func__, at, client, errCode);
    LOGV_4G("4G_task", "Leave %s(), errCode = %d", __func__, errCode);
    return errCode;
}
#endif

/*
+QMTRECV: 0,0,"sys/teiobCPILLff/device/wristband/set",10,"vTaskList2"

+QMTRECV: 0,0,"/dsd_9nj8gqSDBxoK0Rqx4C_A/300318120709/system/WriteData",39,"{"Data":[{"name":"A_A","value":"300"}]}"

*/
#if 1
static int getQMTRECVData(char *at, char *pRecvData, char *pTopic)
{
    char *pEnd;
    LOGE_4G("4G_task", "Enter %s(), gUart8RecvBuffer = %s", __func__, gUart8RecvBuffer);

    char *pBuf = (char *)at;
    char *pBegin = strstr(pBuf, "\""); //Point to : "/dsd_9nj8gqSDBxoK0Rqx4C_A/300318120709/system/WriteData",39,"{"Data":[{"name":"A_A","value":"300"}]}"
    LOGD_4G("4G_task", "pBegin = %s", pBegin);
    pBegin++;
    pEnd = strstr(pBegin, "\""); //Point to : ",39,"{"Data":[{"name":"A_A","value":"300"}]}"
    memcpy(pTopic, pBegin, pEnd - pBegin);
    
    pBegin = pEnd; //Point to : 39,"{"Data":[{"name":"A_A","value":"300"}]}"
    LOGV_4G("4G_task", "pBegin = %s", pBegin);

    pEnd = strstr(pBegin, ","); //Point to : ,"{"Data":[{"name":"A_A","value":"300"}]}"
    LOGD_4G("4G_task", "pEnd = %s", pEnd);
    char lenStr[32] = {0};
    memcpy(lenStr, pBegin, pEnd - pBegin);
    int len = atoi(lenStr);

    pBegin = pEnd + 2;
    pEnd = strstr(pBegin, "\r\n");
    memcpy(pRecvData, pBegin, pEnd - pBegin - 1);
    LOGI_4G("4G_task", "Leave %s(), len = %d, pRecvData = %s, pTopic = %s", __func__, len, pRecvData, pTopic);
    return len;
}

#elif 0
static int getQMTRECVData(char *pRecvData)
{
    LOGV_4G("4G_task", "Enter %s(), gUart4RecvBuffer = %s", __func__, gUart4RecvBuffer);
    char *pBuf = (char *)gUart4RecvBuffer;
    char *pBegin = strstr(pBuf, "\"");
    LOGD_4G("4G_task", "pBegin = %s", pBegin);
    pBegin++;
    pBegin = strstr(pBegin, "\"");
    LOGV_4G("4G_task", "pBegin = %s", pBegin);
    pBegin += 2;
    char *pEnd = strstr(pBegin, ",");
    LOGD_4G("4G_task", "pEnd = %s", pEnd);
    char lenStr[32] = {0};
    memcpy(lenStr, pBegin, pEnd - pBegin);
    int len = atoi(lenStr);

    pBegin = pEnd + 2;
    pEnd = strstr(pBegin, "\"");
    memcpy(pRecvData, pBegin, pEnd - pBegin);
    LOGI_4G("4G_task", "Leave %s(), len = %d, pRecvData = %s", __func__, len, pRecvData);
    return len;
}
#else
static int getQMTRECVData(char *pRecvData)
{
    LOGV_4G("4G_task", "Enter %s(), gUart4RecvBuffer = %s", __func__, gUart4RecvBuffer);
    char *pBuf = (char *)gUart4RecvBuffer;
    char *pRecv = strstr(pBuf, "+QMTRECV:");
    LOGD_4G("4G_task", "pRecv = %s", pRecv);
    int clientID;
    int msgID;
    char temp[32];
    char *topic = pvPortMalloc(2048);
    int len;
    sscanf(pRecv, "%s %d,%d,\"%s\",%d,\"%s\"", temp, &clientID, &msgID, topic, &len, pRecvData);
    vPortFree(topic);
    LOGI_4G("4G_task", "Leave %s(), clientID = %d, msgID = %d, topic = %s, len = %d, pRecvData = %s", __func__, clientID, msgID, topic, len ,pRecvData);
    return len;
}
#endif

#if (KALYKE_LP_RTC == 1)
static void judgeTheTime(snvs_lp_srtc_datetime_t *pRTC)
{
    /* Table of days in a month for a non leap year. First entry in the table is not used,
     * valid months start from 1
     */
    uint8_t daysPerMonth[] = {0U, 31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};
    pRTC->hour += 8;
    if (pRTC->hour > 23)
    {
        pRTC->hour -= 24;
        pRTC->day++;
        uint8_t monthDays = daysPerMonth[pRTC->month];
        if (pRTC->month == 2)
        {
            if (pRTC->year % 4 == 0)
            {
                monthDays++;
            }
        }
        if (pRTC->day > monthDays)
        {
            pRTC->day = 1;
            pRTC->month++;
            if (pRTC->month > 12)
            {
                pRTC->month = 1;
                pRTC->year++;
            }
        }
    }
    pRTC->second += 2;
    if (pRTC->second >= 60)
    {
        pRTC->second = 59;
    }
}
#else
static void judgeTheTime(snvs_hp_rtc_datetime_t *pRTC)
{
    /* Table of days in a month for a non leap year. First entry in the table is not used,
     * valid months start from 1
     */
    uint8_t daysPerMonth[] = {0U, 31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};
    pRTC->hour += 8;
    if (pRTC->hour > 23)
    {
        pRTC->hour -= 24;
        pRTC->day++;
        uint8_t monthDays = daysPerMonth[pRTC->month];
        if (pRTC->month == 2)
        {
            if (pRTC->year % 4 == 0)
            {
                monthDays++;
            }
        }
        if (pRTC->day > monthDays)
        {
            pRTC->day = 1;
            pRTC->month++;
            if (pRTC->month > 12)
            {
                pRTC->month = 1;
                pRTC->year++;
            }
        }
    }
    pRTC->second += 2;
    if (pRTC->second >= 60)
    {
        pRTC->second = 59;
    }
}
#endif

// +QNTP: 0,"2019/10/16,03:28:04+00"
static bool getQNTPResult(void)
{
    char *pBuf = (char *)gUart8RecvBuffer;
    char *pQNTP = strstr(pBuf, "+QNTP:");
    LOGV("4G_task", "pQNTP = %s", pQNTP);
    if (strlen(pQNTP) < 33) // 根据观察得到
    {
    }
    char *pTime = strstr(pQNTP, "\"");
    LOGV("4G_task", "pTime = %s", pTime);
    char *pTimeEnd = strstr(pTime + 1, "\"");
    if (pTimeEnd == NULL)
    {
        LOGE("4G_task", "pTimeEnd ERROR!");
        return false;
    }
    char time[32] = {0};
    memcpy((uint8_t*)time, pTime + 1, pTimeEnd - pTime - 1);
    LOGD("4G_task", "time = %s", time);

    int timeZone;
    
#if (KALYKE_LP_RTC == 1)
    snvs_lp_srtc_datetime_t srtcDate;
    sscanf(time, "%d/%d/%d,%d:%d:%d+%d", &srtcDate.year, &srtcDate.month, &srtcDate.day, &srtcDate.hour, &srtcDate.minute, &srtcDate.second, &timeZone);
    judgeTheTime(&srtcDate);
    LOGI("4G_task", "SRTC: %04u-%02u-%02u  %02u:%02u:%02u\r\n", srtcDate.year, srtcDate.month, srtcDate.day, srtcDate.hour, srtcDate.minute, srtcDate.second);
    if (SNVS_LP_SRTC_SetDatetime(SNVS, &srtcDate) == kStatus_Success)
    {
        SNVS_HP_RTC_TimeSynchronize(SNVS);
        LOGD("4G_task", "Set RTC Success!");
    }
    else
    {
        LOGE("4G_task", "Set RTC Failed!");
    }
#else
    snvs_hp_rtc_datetime_t rtcDate;
    sscanf(time, "%d/%d/%d,%d:%d:%d+%d", &rtcDate.year, &rtcDate.month, &rtcDate.day, &rtcDate.hour, &rtcDate.minute, &rtcDate.second, &timeZone);
    judgeTheTime(&rtcDate);
    LOGI_4G("4G_task", "RTC: %04u-%02u-%02u  %02u:%02u:%02u\r\n", rtcDate.year, rtcDate.month, rtcDate.day, rtcDate.hour, rtcDate.minute, rtcDate.second);
    SNVS_HP_RTC_SetDatetime(SNVS, &rtcDate);
#endif
    LOGI("4G_task", "timeZone = %d", timeZone);
    gWorldSecondTick = SNVS_HP_RTC_GetSeconds(SNVS) - 28800;
    LOGI("4G_task", "gWorldSecondTick  = %u", gWorldSecondTick );
    return true;
}

static void ec20_sntp_init(void)
{
    char *sendBuf = pvPortMalloc(512);
    char sntpServer[128];
    strcpy(sntpServer, "cn.pool.ntp.org");
    while(1)
    {
        if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
        {
            LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR", __func__);
            vTaskDelay(5000);
            continue;
        }
        sprintf(sendBuf, "AT+QNTP=1,\"%s\"", sntpServer);
        sendAT(sendBuf);
        int ret = waitResponse(5000, "+QNTP:", "ERROR\r\n\r\n+QNTP:", "ERROR", NULL, NULL);
        if (ret == 1)
        {
            if (getQNTPResult())
            {
                break;
            }
        }
        else if (ret == 2)
        {
            //setCME_ERROR_SD();
            if (getQNTPResult())
            {
                break;
            }
        }
        else if (ret == 3)
        {
            xEventGroupSetBits(g_kalyke_event_group, KALYKE_EVENT_TCP_WAIT_4G_MQTT);
        }
        xSemaphoreGive(g4GMutex);
        vTaskDelay(5001);
    }
    vPortFree(sendBuf);
    xSemaphoreGive(g4GMutex);
}

#if (WAN_4G_SWITCH_AUTO == 1)
static void led_4G_task(void *p_arg)
{
    for(;;)
    {
        if (gMqttSource != MQTT_4G)
        {
            vTaskDelay(10000);
            continue;
        }
        if (gWanOK)
        {
            vTaskDelay(10000);
            continue;
        }
        if (gIs4GMqttConnected)
        {
            bsp_toggle_led_2();
        }
        else
        {
            bsp_close_led_2();
        }
        vTaskDelay(800 / portTICK_PERIOD_MS);
    }
}
#else
static void led_4G_task(void *p_arg)
{
    LOGE_4G("4G_task", "led_4G_task RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());
    xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_ENET_INIT_DONE_LED, pdTRUE, pdFALSE, portMAX_DELAY);
    LOGV("4G_task", "%s : KALYKE_EVENT_ENET_INIT_DONE_LED", __func__);
    if (g_plc_netcfg.surfing == 0)
    {
        LOGW_4G("4G_task", "Just return because surfing mode is not 4G.");
        vTaskDelete(NULL);
        return;
    }

    for(;;)
    {
        if (gIs4GMqttConnected)
        {
            bsp_toggle_led_mqtt();
        }
        else
        {
            bsp_close_led_mqtt();
        }
        vTaskDelay(799 / portTICK_PERIOD_MS);
    }
}
#endif

static void led_4G_task_start(void)
{
    xTaskCreate((TaskFunction_t)led_4G_task,
                (const char *)"4G_LED_task",
                (uint16_t)LED_TASK_STACK_SIZE,
                (void *)NULL,
                (UBaseType_t )LED_TASK_PRIO,
                (TaskHandle_t *)NULL);
}

void init_UART8_4G(void)
{
    ec20_LPUART8_init(115200, 0, 8, 1);
}

#if 0
void kalyke_4G_task(void *p_arg)
{
    LOGV_4G("4G_task", "kalyke_4G_task RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());
    mqtt_msg_st mqttMsg;
    g4GMqttMsgQueue = xQueueCreate(5, sizeof(mqtt_msg_st));
    led_4G_task_start();
    ec20_LPUART4_init(115200, 0, 8, 1);
    ec20_MQTT_init();

    for(;;)
    {
        LOGV_4G("4G_task", "Running");
        init_uart4_receive_buffer();
        //int ret = waitResponse(portMAX_DELAY, "+QMTRECV:", "+QMTSTAT:", "OK", "ERROR", NULL);
        int ret = waitResponse(portMAX_DELAY, "+QMTRECV:", "+QMTSTAT:", NULL, NULL, NULL);
        if (ret == 1)
        {
            LOGD_4G("4G_task", "4G MQTT receive some data...");
            char *recvBuf = pvPortMalloc(2048);
            memset(recvBuf, 0, 2048);
            mqttMsg.msgLength = getQMTRECVData(recvBuf);
            mqttMsg.dataBuff = (uint8_t *)recvBuf;
            handle_mqtt_recv_data(&mqttMsg);
            vPortFree(recvBuf);
        }
        else if (ret == 2)
        {
            uint8_t errCode = getQMTSTATErrorCode();
            uint16_t err = QMTSTATE_BASE + errCode;
            SET_SD_ELEMENT_VALUE(SD199, err);
            handleErrorQMTSTATE(errCode);
        }
        else if (ret == 3)
        {
            // 调试用的。此时收到了其他的AT指令。
        }
    }
}
#else
/* Just handle +QMTRECV and +QMTSTAT
*/
static void handle_4g_mqtt_recv(mqtt_msg_st *pMqttMsg)
{
    LOGE("4G", "Enter %s(), pJson = %s", __func__, pMqttMsg->dataBuff);
    if (strstr((char *)pMqttMsg->dataBuff, "+QMTRECV:"))
    {
        mqtt_msg_st mqttMsg;
        char *recvBuf = pvPortMalloc(2048);
        memset(recvBuf, 0, 1024);
        char *topic = pvPortMalloc(1024);
        memset(topic, 0, 1024);
        mqttMsg.msgLength = getQMTRECVData((char *)pMqttMsg->dataBuff, recvBuf, topic);
        mqttMsg.dataBuff = (uint8_t *)recvBuf;
        mqttMsg.topic = (uint8_t *)topic;
        handle_mqtt_recv_data(&mqttMsg);
        vPortFree(recvBuf);
        vPortFree(topic);
    }
    else
    {
       LOGE_4G("4G_task", "in handle_4g_mqtt_recv(),  pMqttMsg->dataBuff = %s ",(char *)pMqttMsg->dataBuff);
       if (strstr((char *)pMqttMsg->dataBuff, "+QMTSTAT:"))
       {
         uint8_t errCode = getQMTSTATErrorCode((char *)pMqttMsg->dataBuff);
         uint16_t err = QMTSTATE_BASE + errCode;
         SET_SD_ELEMENT_VALUE(SD230, err);
         if (gIs4GMqttConnected == false && gIsReConnecting == false )
         {
               LOGE_4G("4G_task", "mqttMsg.type == 1 and call handleErrorQMTSTATE(0)");
               handleErrorQMTSTATE(err);
          }
       }
    }
    vPortFree(pMqttMsg->dataBuff);
}

void kalyke_4G_task(void *p_arg)
{
    LOGV("4G_task", "kalyke_4G_task RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());
#if 0
#if (WAN_4G_SWITCH_AUTO == 0)
    if (g_plc_netcfg.surfing == 0)
    {
        gMqttSource = MQTT_ENET;
        LOGW("4G_task", "Just return because surfing mode is not 4G.");
        vTaskDelete(NULL);
        return;
    }
#endif
#endif
    led_4G_task_start();
    xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_ENET_INIT_DONE_PLC, pdTRUE, pdFALSE, portMAX_DELAY);
    vTaskDelay(10000); //useful, wait EC20 init
#if 0 //ESX都有4G
    //等待PLC启动后，gLeftModules.BIT.HaveCM010变为1
    if (bspIsHave4G() == false)
    {
        gMqttSource = MQTT_ENET;
        LOGW("4G_task", "Just return because this product dose not support 4G feature.");
        vTaskDelete(NULL);
        return;
    }
#endif
    gMqttSource = MQTT_4G;
    mqtt_msg_st mqttMsg;
    g4GMqttMsgQueue = xQueueCreate(10, sizeof(mqtt_msg_st));
    memset(&gGpsValue, 0, sizeof(gGpsValue));
    g4GMutex = xSemaphoreCreateMutex();
    ec20_LPUART8_init(115200, 0, 8, 1);

    //热启动（复位）时，需要重启4G模块。
    xSemaphoreGive(g4GMutex);
    mqtt_4G_rst();

#if 0
#if (KALYKE_FEATURE_4G_TCP_TASK == 1)
    xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_WAIT_MICROLINK_CONNECTED, pdTRUE, pdFALSE, portMAX_DELAY);
    LOGV("4G_task", "KALYKE_EVENT_WAIT_MICROLINK_CONNECTED happened.");
#endif
#endif

    ec20_GPS_init();

    while(1)
    {
        if (gMqttSource != MQTT_4G)
        {
            vTaskDelay(2000);
        }
        else
        {
            break;
        }
    }
    ec20_MQTT_init();
    //ec20_HTTP_init();
    AT_CSQ();
    
//    if (g_plc_netcfg.mqtt.reportingCycle < 30)
//    {
//        g_plc_netcfg.mqtt.reportingCycle = 30;//4G MQTT最小MQTT发送间隔为30s
//    }
#if (KALYKE_FEATURE_4G_TCP_TASK == 1)
    xEventGroupSetBits(g_kalyke_event_group, KALYKE_EVENT_TCP_WAIT_4G_MQTT);
#endif
    monitor_publish_now();/* 当连上MQTT服务器后立即publish一下 */
    for(;;)
    {
        LOGI_4G("4G_task", "Let us wait 4G MQTT receive something happen...");
        if (xQueueReceive(g4GMqttMsgQueue, &mqttMsg, portMAX_DELAY) == pdFALSE)
        {
            continue;
        }
        LOGI_4G("4G_task", "mqttMsg.type = %d", mqttMsg.type);
        if (gMqttSource != MQTT_4G)
        {
            LOGI_4G("4G_task", "gMqttSource != MQTT_4G, just continue!");
            g4GMqttPublishing = false;
            init_uart8_receive_buffer();
            continue;
        }
        if (mqttMsg.type == 0)
        {
            handle_4g_mqtt_recv(&mqttMsg);
        }
        else if (mqttMsg.type == 1)
        {
            ec20_mqtt_publish_do((char*)mqttMsg.topic, (char*)mqttMsg.dataBuff, mqttMsg.msgLength, mqttMsg.qos);
            vPortFree(mqttMsg.topic);
            vPortFree(mqttMsg.dataBuff);
            if (gIs4GMqttConnected == false && gIsReConnecting == false )
            {
               LOGE_4G("4G_task", "mqttMsg.type == 1 and call handleErrorQMTSTATE(0)");
               handleErrorQMTSTATE(0);
            }
        }
    }
}

#endif

#else
void ec20_mqtt_publish(char *topic, char *payload, uint8_t qos){}
void init_UART8_4G(void){}
void sendAT(char *pAT){}
#endif /* KALYKE_FEATURE_4G_TASK */
