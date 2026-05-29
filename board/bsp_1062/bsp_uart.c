/**
  ******************************************************************************
  * @file    bsp_uart.c
  * @author  lixianyu
  * @version V0.0.2
  * @date    2019-05-25
  * @brief   串口驱动程序
  ******************************************************************************
  */

#include "FreeRTOS.h"
#include "timers.h"

#include "board.h"
#include "fsl_debug_console.h"
#include "bsp_uart.h"
#include "fsl_lpuart_freertos.h"
#include "plc_variable.h"
#include "bsp_dct.h"
#include "bsp_gpio.h"
#include "kalyke_uart_task.h"
#include "plc_commonfunc.h"
#include "bsp_led.h"
#include "kalyke_tool.h"
#include "kalyke_opts.h"
#include "verify_func.h"
#include "kalyke_event.h"
#include "kalyke_internet_task.h"
#include "kalyke_collect_task.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void bsp_send_finished(uint8_t portNum);
static void bsp_uart_rx_enable(uint8_t portNum);
static void kalyke_ring_buffer_switch_write_mem_uart1(ring_buffer_st *ltp_RingBuff);
static void kalyke_ring_buffer_switch_write_mem_uart2(ring_buffer_st *ltp_RingBuff);
static unsigned char *kalyke_ring_buffer_get_write_mem_uart1(ring_buffer_st *ltp_RingBuff);
static unsigned char *kalyke_ring_buffer_get_write_mem_uart2(ring_buffer_st *ltp_RingBuff);

void bsp_uart_tx_enable(uint8_t portNum);

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define UART0_RX_TIMER_ID   1
#define UART0_TX_TIMER_ID   2
#define UART1_RX_TIMER_ID   3
#define UART1_TX_TIMER_ID   4
#define UART2_RX_TIMER_ID   5
#define UART2_TX_TIMER_ID   6


/*******************************************************************************
 * Variables
 ******************************************************************************/
static bsp_uart_status_info_st stv_UartPortStatus[MAX_SUPPORT_UART_PORT];

/*定时器句柄*/
TimerHandle_t ltv_Uart0RxTime;
TimerHandle_t ltv_Uart0TxTime;

TimerHandle_t ltv_Uart1RxTime;
TimerHandle_t ltv_Uart1TxTime;

TimerHandle_t ltv_Uart2RxTime;
TimerHandle_t ltv_Uart2TxTime;

TimerHandle_t gModLinkDelayTime0 = NULL;
TimerHandle_t gModLinkDelayTime1 = NULL;
TimerHandle_t gModLinkDelayTime2 = NULL;

/*UART接收缓冲区定义*/
ring_buffer_st gtv_Uart0ReadBuffer;
ring_buffer_st gtv_Uart1ReadBuffer;
ring_buffer_st gtv_Uart2ReadBuffer;

static TaskHandle_t g_Uart0WorkerHandler = NULL;
static uint8_t background_buffer0[1024];
static uint8_t recv_buffer0[16];
static lpuart_rtos_handle_t gLpuart0RTOSHandle;
static struct _lpuart_handle t_handle;
static lpuart_rtos_config_t lpuart_config0 =
{
    .baudrate = 115200,
    .parity = kLPUART_ParityDisabled,
    .stopbits = kLPUART_OneStopBit,
    .buffer = background_buffer0,
    .buffer_size = sizeof(background_buffer0),
    //.enableRxRTS = false,
    //.enableTxCTS = false,
};
static volatile TickType_t gTick;

// Just for COM0 use!
static uint8_t gRing1[1024];
static uint8_t gRing2[1024];
static void kalyke_ring_buffer_init_uart0(ring_buffer_st *ltp_RingBuff, unsigned short lsv_BuffSize)
{
    ltp_RingBuff->mcp_Buff[0] = gRing1;
    ltp_RingBuff->mcp_Buff[1] = gRing2;
    ltp_RingBuff->mcv_Index = 0;
}
static void kalyke_ring_buffer_switch_write_mem_uart0(ring_buffer_st *ltp_RingBuff)
{
    if(ltp_RingBuff->mcv_Index + 1 < 2)
    {
        ltp_RingBuff->mcv_Index += 1;
    }
    else
    {
        ltp_RingBuff->mcv_Index = 0;
    }
}
static unsigned char *kalyke_ring_buffer_get_write_mem_uart0(ring_buffer_st *ltp_RingBuff)
{
    return ltp_RingBuff->mcp_Buff[ltp_RingBuff->mcv_Index];
}

/*
 读寄存器返回  02 03 06 00 0B 00 16 00 21 B1 98
 读线圈返回       02 01 01 03 11 CD
*/
static void modlink_copy(uint16_t port)
{
    LOGV("bsp_uart", "Enter %s(), port = %u, aFlag = %u", __func__, port, stv_UartPortStatus[port].aFlag);
    if (stv_UartPortStatus[port].aFlag == 0)
    {
        if (stv_UartPortStatus[port].aBuf)
        {
            vPortFree(stv_UartPortStatus[port].aBuf);
            stv_UartPortStatus[port].aBuf = NULL;
        }
        return;
    }

    hexdump(stv_UartPortStatus[port].aBuf, stv_UartPortStatus[port].msv_RxLength);
    uint16_t recvCrc = *(unsigned short *)(stv_UartPortStatus[port].aBuf + stv_UartPortStatus[port].msv_RxLength - 2);
    uint16_t calculateCrc = calc_crc16(stv_UartPortStatus[port].aBuf, stv_UartPortStatus[port].msv_RxLength - 2);
    LOGI("bsp_uart", "recvCrc = 0x%X, calculateCrc = 0x%X", recvCrc, calculateCrc);
    if (recvCrc != calculateCrc)
    {
        LOGE("bsp_uart", "OMG, CRC ERROR!!!");
        return;
    }
    int n;

    if (stv_UartPortStatus[port].aBuf[1] == 0x03)
    {
        if (stv_UartPortStatus[port].aBuf[2] == 0x16)
        {
            Handle_FindFlkRtu_RecvData(stv_UartPortStatus[port].aBuf);
        }
    }

    if (stv_UartPortStatus[port].aBuf[1] == 3 ||
        stv_UartPortStatus[port].aBuf[1] == 4) //读寄存器返回
    {
        n = stv_UartPortStatus[port].aBuf[2] / 2; // 读取的元件个数
    }
    else if (stv_UartPortStatus[port].aBuf[1] == 1 || //读线圈返回
             stv_UartPortStatus[port].aBuf[1] == 2)
    {
        n = stv_UartPortStatus[port].aBuf[2];
    }
    else
    {
        LOGE("bsp_uart", "We do not know 0x%X, so just return!!!", stv_UartPortStatus[port].aBuf[1]);
        return;
    }
    uint8_t *pBuf = &stv_UartPortStatus[port].aBuf[3];
    for (int i = 0; i < n; i++)
    {
        if (stv_UartPortStatus[port].aBuf[1] == 3 ||
            stv_UartPortStatus[port].aBuf[1] == 4)
        {
            stv_UartPortStatus[port].msp_FreeRxBuff[i] = GET_BIGPU16_DATA(pBuf);
            pBuf++;
            pBuf++;
        }
        else
        {
            stv_UartPortStatus[port].msp_FreeRxBuff[i] = GET_PU8_DATA(pBuf);
            pBuf++;
        }
    }


    vPortFree(stv_UartPortStatus[port].aBuf);
    stv_UartPortStatus[port].aBuf = NULL;
}

static void modbusDelay_timer_callback_func(TimerHandle_t ltv_TimeHandle)
{
    LOGV("bsp_uart", "Enter %s(), ltv_TimeHandle = 0x%08X", __func__, ltv_TimeHandle);
    uint32_t liv_TimerId = (unsigned int)pvTimerGetTimerID(ltv_TimeHandle);
    LOGI("bsp_uart", "liv_TimerId = %u", liv_TimerId);
    SET_UART_SM_FLAG(liv_TimerId, UART_SM_MODBUS_FINISH);

    xEventGroupSetBits(g_kalyke_event_group, KALYKE_EVENT_UART_COM0_HAD_RESP << liv_TimerId);
}

static void bsp_uart_set_modbus_finished(uint8_t port)
{
    LOGV("bsp_uart", "Enter %s(), port = %u", __func__, port);
    bsp_uart_tx_enable(port);
    if (port == 0)
    {
        if(gModLinkDelayTime0 == NULL)
        {
            gModLinkDelayTime0 = xTimerCreate((const char *)"modbus_delay0",
                                             (TickType_t  )2 / portTICK_PERIOD_MS,
                                             (UBaseType_t )pdFALSE,
                                             (void *      )(uint32_t)port,
                                             (TimerCallbackFunction_t)modbusDelay_timer_callback_func);
        }
        xTimerStart(gModLinkDelayTime0, 0);
    }
    else if (port == 1)
    {
        if(gModLinkDelayTime1 == NULL)
        {
            gModLinkDelayTime1 = xTimerCreate((const char *)"modbus_delay1",
                                             (TickType_t  )2 / portTICK_PERIOD_MS,
                                             (UBaseType_t )pdFALSE,
                                             (void *)(uint32_t)port,
                                             (TimerCallbackFunction_t)modbusDelay_timer_callback_func);
        }
        xTimerStart(gModLinkDelayTime1, 100);
    }
    else if (port == 2)
    {
        if(gModLinkDelayTime2 == NULL)
        {
            gModLinkDelayTime2 = xTimerCreate((const char *)"modbus_delay2",
                                             (TickType_t  )2 / portTICK_PERIOD_MS,
                                             (UBaseType_t )pdFALSE,
                                             (void *      )(uint32_t)port,
                                             (TimerCallbackFunction_t)modbusDelay_timer_callback_func);
        }
        xTimerStart(gModLinkDelayTime2, 100);
    }
}

static void bsp_handle_timer_callback_TouChuan(unsigned int timerID)
{
    BaseType_t ltv_Err;
    uart_msg_st ltv_UartMsg;
    uint8_t port;
    switch (timerID)
    {
        case UART0_RX_TIMER_ID:
            port = 0;
            /*切换接收缓冲区，为下一次接收准备*/
            kalyke_ring_buffer_switch_write_mem_uart0(&gtv_Uart0ReadBuffer);
            break;

        case UART1_RX_TIMER_ID:
            port = 1;
            /*切换接收缓冲区，为下一次接收准备*/
            kalyke_ring_buffer_switch_write_mem_uart1(&gtv_Uart1ReadBuffer);
            break;

        case UART2_RX_TIMER_ID:
            port = 2;
            /*切换接收缓冲区，为下一次接收准备*/
            kalyke_ring_buffer_switch_write_mem_uart2(&gtv_Uart2ReadBuffer);
            break;

        default:
            LOGE("bsp_uart", "timerID = %d, ERROR!");
            return;
            break;
    }

    ltv_UartMsg.mcv_UartPort = port;
    ltv_UartMsg.msv_MsgLength = stv_UartPortStatus[port].msv_RxLength;
    ltv_UartMsg.mcp_DataBuff = stv_UartPortStatus[port].mcp_RxBuff;

    ltv_Err = xQueueSend(gtv_UartTaskMsgQueueHandle, &ltv_UartMsg, 1);
    if(ltv_Err != pdPASS)
    {
        LOGE("bsp_uart", "xQueueSend to gtv_UartTaskMsgQueueHandle ERROR!! (Line: %d)", __LINE__);
    }

    stv_UartPortStatus[port].mcv_Status = UART_IDLE;
    SET_UART_SM_FLAG(port, UART_SM_IDLE);
}

/**
  * @brief  定时器回掉函数
  * @param  None
  * @retval None
  */
static void bsp_timer_callback_func(TimerHandle_t ltv_TimeHandle)
{
    unsigned int liv_TimerId;
    uart_msg_st ltv_UartMsg;
    BaseType_t ltv_Err;

    liv_TimerId = (unsigned int)pvTimerGetTimerID(ltv_TimeHandle);
    TickType_t curTick = xTaskGetTickCount();
    //LOGV("bsp_uart", "Enter %s, liv_TimerId = %d, Elapse = %ums\r\n", __func__, liv_TimerId, (curTick-gTick));

    if (gTouChuan == 1 || gTouChuan == 0 || gTouChuan == 2)
    {
        bsp_handle_timer_callback_TouChuan(liv_TimerId);
        return;
    }

    switch(liv_TimerId)
    {
    /*自由口字符间超时定时器，modbus T35超时*/
    case UART0_RX_TIMER_ID:
        switch(gtp_UartPort[0].mcv_Mode)
        {
        case UART_TYPE_FREE_PORT:
            /*置标志位*/
            SET_UART_SM_FLAG(0, UART_SM_RX_FINISH);
            SET_UART_SD_VALUE(0, UART_SD_FINISH_RX_CHAR, FREE_COM_FINISH_RX_WORD_TIMEOUT);
            /*清除接收使能标志*/
            RST_UART_SM_FLAG(0, UART_SM_FREE_PORT_RX_EN);

            stv_UartPortStatus[0].mcv_Status = UART_IDLE;
            SET_UART_SM_FLAG(0, UART_SM_IDLE);
            break;

        case UART_TYPE_MB_SLAVE:
#if 0
            if (stv_UartPortStatus[0].msv_RxLength < 5)
            {
                stv_UartPortStatus[0].mcv_Status = UART_IDLE;
                SET_UART_SM_FLAG(0, UART_SM_IDLE);
                break;
            }
#endif
            ltv_UartMsg.mcv_UartPort = 0;
            ltv_UartMsg.msv_MsgLength = stv_UartPortStatus[0].msv_RxLength;
            ltv_UartMsg.mcp_DataBuff = stv_UartPortStatus[0].mcp_RxBuff;
            //PRINTF("Current task handle is : 0x%x\r\n", xTaskGetCurrentTaskHandle());

            ltv_Err = xQueueSend(gtv_UartTaskMsgQueueHandle, &ltv_UartMsg, 10);
            if(ltv_Err != pdPASS)
            {
                /*错误处理*/
                LOGE("bsp_uart", "xQueueSend to gtv_UartTaskMsgQueueHandle ERROR!\r\n");
            }
            else
            {
                //LOGI("bsp_uart", "xQueueSend : msv_MsgLength = %u", ltv_UartMsg.msv_MsgLength);
            }

            /*切换接收缓冲区，为下一次接收准备*/
            kalyke_ring_buffer_switch_write_mem_uart0(&gtv_Uart0ReadBuffer);

            stv_UartPortStatus[0].mcv_Status = UART_IDLE;
            SET_UART_SM_FLAG(0, UART_SM_IDLE);
            break;

        case UART_TYPE_MB_MASTER:
            modlink_copy(0);
            SET_UART_SM_FLAG(0, UART_SM_RX_FINISH);
            //SET_UART_SM_FLAG(0, UART_SM_MODBUS_FINISH);
            bsp_uart_set_modbus_finished(0);

            stv_UartPortStatus[0].mcv_Status = UART_IDLE;
            SET_UART_SM_FLAG(0, UART_SM_IDLE);
            break;
        }

        //bsp_refresh_communication_led(0, UART0_A_LED_MASK);
        break;

    /*自由口接收帧超时，modbus 发送超时*/
    case UART0_TX_TIMER_ID:
        switch(gtp_UartPort[0].mcv_Mode)
        {
        case UART_TYPE_FREE_PORT:
            /*置标志位*/
            SET_UART_SM_FLAG(0, UART_SM_RX_FINISH);
            SET_UART_SD_VALUE(0, UART_SD_FINISH_RX_CHAR, FREE_COM_FINISH_RX_FRAME_TIMEOUT);
            /*清除接收使能标志*/
            RST_UART_SM_FLAG(0, UART_SM_FREE_PORT_RX_EN);

            stv_UartPortStatus[0].mcv_Status = UART_IDLE;
            SET_UART_SM_FLAG(0, UART_SM_IDLE);

            //bsp_refresh_communication_led(0, UART0_A_LED_MASK);
            break;
        case UART_TYPE_MB_SLAVE:

            break;

        case UART_TYPE_MB_MASTER:
        //    LOGV("bsp_uart", "stv_UartPortStatus[0].mcv_RetryCnt = %u", stv_UartPortStatus[0].mcv_RetryCnt);
            /*发送超时，重发*/
            if(stv_UartPortStatus[0].mcv_RetryCnt != 0)
            {
                stv_UartPortStatus[0].mcv_RetryCnt --;
                /*重新发送*/
                stv_UartPortStatus[0].mcv_Status = UART_TX;
                stv_UartPortStatus[0].msv_TxLength = 0;
                xTimerStart(ltv_Uart0TxTime, 1);
            }
            else
            {
                LOGV("bsp_uart", "uart0 send timeout!");
                stv_UartPortStatus[0].mcv_Status = UART_IDLE;
                stv_UartPortStatus[0].msv_TxLength = 0;
                stv_UartPortStatus[0].msv_TxTotalLength = 0;
                stv_UartPortStatus[0].mcv_RetryCnt = 0;

                /*默认使能RX*/
                bsp_uart_rx_enable(0);

                /*超时返回*/
                //SET_UART_SM_FLAG(0, UART_SM_MODBUS_FINISH);
                bsp_uart_set_modbus_finished(0);

                /*设置模式错误标志*/
                SET_UART_SM_FLAG(0, UART_SM_MODBUS_ERROR);
                SET_UART_SD_VALUE(0, UART_SD_MASTER_ERROR_CODE, MB_ERROR_SLAVE_BUSY);

                stv_UartPortStatus[0].mcv_Status = UART_IDLE;
                SET_UART_SM_FLAG(0, UART_SM_IDLE);
            }
        }
        break;

    /*自由口字符间超时定时器，modbus T35超时*/
    case UART1_RX_TIMER_ID:
        switch(gtp_UartPort[1].mcv_Mode)
        {
        case UART_TYPE_FREE_PORT:
            /*置标志位*/
            SET_UART_SM_FLAG(1, UART_SM_RX_FINISH);
            SET_UART_SD_VALUE(1, UART_SD_FINISH_RX_CHAR, FREE_COM_FINISH_RX_WORD_TIMEOUT);
            /*清除接收使能标志*/
            RST_UART_SM_FLAG(1, UART_SM_FREE_PORT_RX_EN);

            stv_UartPortStatus[1].mcv_Status = UART_IDLE;
            SET_UART_SM_FLAG(1, UART_SM_IDLE);
            break;

        case UART_TYPE_MB_SLAVE:
            ltv_UartMsg.mcv_UartPort = 1;
            ltv_UartMsg.msv_MsgLength = stv_UartPortStatus[1].msv_RxLength;
            ltv_UartMsg.mcp_DataBuff = stv_UartPortStatus[1].mcp_RxBuff;

            ltv_Err = xQueueSend(gtv_UartTaskMsgQueueHandle, &ltv_UartMsg, 10);
            if(ltv_Err != pdPASS)
            {
                /*错误处理*/
                LOGE("bsp_uart", "xQueueSend to gtv_UartTaskMsgQueueHandle ERROR!!(Line: %d)", __LINE__);
            }

            /*切换接收缓冲区，为下一次接收准备*/
            kalyke_ring_buffer_switch_write_mem_uart1(&gtv_Uart1ReadBuffer);
            stv_UartPortStatus[1].mcv_Status = UART_IDLE;
            SET_UART_SM_FLAG(1, UART_SM_IDLE);
            break;

        case UART_TYPE_MB_MASTER:
            //LOGE("bsp_uart", "uart1 recv finished. Because times up...");
            modlink_copy(1);
            SET_UART_SM_FLAG(1, UART_SM_RX_FINISH);
            //SET_UART_SM_FLAG(1, UART_SM_MODBUS_FINISH);
            bsp_uart_set_modbus_finished(1);

            stv_UartPortStatus[1].mcv_Status = UART_IDLE;
            SET_UART_SM_FLAG(1, UART_SM_IDLE);
            break;
        }

        //bsp_refresh_communication_led(0, UART1_A_LED_MASK);
        break;

    /*自由口接收帧超时，modbus 发送超时*/
    case UART1_TX_TIMER_ID:
        switch(gtp_UartPort[1].mcv_Mode)
        {
        case UART_TYPE_FREE_PORT:
            /*置标志位*/
            SET_UART_SM_FLAG(1, UART_SM_RX_FINISH);
            SET_UART_SD_VALUE(1, UART_SD_FINISH_RX_CHAR, FREE_COM_FINISH_RX_FRAME_TIMEOUT);
            /*清除接收使能标志*/
            RST_UART_SM_FLAG(1, UART_SM_FREE_PORT_RX_EN);

            stv_UartPortStatus[1].mcv_Status = UART_IDLE;
            SET_UART_SM_FLAG(1, UART_SM_IDLE);

            //bsp_refresh_communication_led(0, UART1_A_LED_MASK);
            break;
        case UART_TYPE_MB_SLAVE:

            break;

        case UART_TYPE_MB_MASTER:
            LOGV("bsp_uart", "stv_UartPortStatus[1].mcv_RetryCnt = %u", stv_UartPortStatus[1].mcv_RetryCnt);
            /*发送超时，重发*/
            if(stv_UartPortStatus[1].mcv_RetryCnt != 0)
            {
                stv_UartPortStatus[1].mcv_RetryCnt --;
                /*重新发送*/
                stv_UartPortStatus[1].mcv_Status = UART_TX;
                stv_UartPortStatus[1].msv_TxLength = 0;
                xTimerStart(ltv_Uart1TxTime, 100);
            }
            else
            {
                LOGV("bsp_uart", "uart1 send timeout!");
                stv_UartPortStatus[1].mcv_Status = UART_IDLE;
                stv_UartPortStatus[1].msv_TxLength = 0;
                stv_UartPortStatus[1].msv_TxTotalLength = 0;
                stv_UartPortStatus[1].mcv_RetryCnt = 0;

                /*默认使能RX*/
                bsp_uart_rx_enable(1);

                /*超时返回*/
                //SET_UART_SM_FLAG(1, UART_SM_MODBUS_FINISH);
                bsp_uart_set_modbus_finished(1);
                /*设置模式错误标志*/
                SET_UART_SM_FLAG(1, UART_SM_MODBUS_ERROR);
                SET_UART_SD_VALUE(1, UART_SD_MASTER_ERROR_CODE, MB_ERROR_SLAVE_BUSY);

                stv_UartPortStatus[1].mcv_Status = UART_IDLE;
                SET_UART_SM_FLAG(1, UART_SM_IDLE);
            }
        }
        break;

    case UART2_RX_TIMER_ID:
        switch(gtp_UartPort[2].mcv_Mode)
        {
        case UART_TYPE_FREE_PORT:
            /*置标志位*/
            SET_UART_SM_FLAG(2, UART_SM_RX_FINISH);
            SET_UART_SD_VALUE(2, UART_SD_FINISH_RX_CHAR, FREE_COM_FINISH_RX_WORD_TIMEOUT);
            /*清除接收使能标志*/
            RST_UART_SM_FLAG(2, UART_SM_FREE_PORT_RX_EN);

            stv_UartPortStatus[2].mcv_Status = UART_IDLE;
            SET_UART_SM_FLAG(2, UART_SM_IDLE);
            break;

        case UART_TYPE_MB_SLAVE:
            ltv_UartMsg.mcv_UartPort = 2;
            ltv_UartMsg.msv_MsgLength = stv_UartPortStatus[2].msv_RxLength;
            ltv_UartMsg.mcp_DataBuff = stv_UartPortStatus[2].mcp_RxBuff;

            ltv_Err = xQueueSend(gtv_UartTaskMsgQueueHandle, &ltv_UartMsg, 10);
            if(ltv_Err != pdPASS)
            {
                /*错误处理*/
                LOGE("bsp_uart", "xQueueSend to gtv_UartTaskMsgQueueHandle ERROR!!(Line: %d)", __LINE__);
            }

            /*切换接收缓冲区，为下一次接收准备*/
            kalyke_ring_buffer_switch_write_mem_uart2(&gtv_Uart2ReadBuffer);
            stv_UartPortStatus[2].mcv_Status = UART_IDLE;
            SET_UART_SM_FLAG(2, UART_SM_IDLE);
            break;

        case UART_TYPE_MB_MASTER:
//            LOGV("bsp_uart", "uart2 recv finished. Because times up...");
            modlink_copy(2);
            SET_UART_SM_FLAG(2, UART_SM_RX_FINISH);
            //SET_UART_SM_FLAG(1, UART_SM_MODBUS_FINISH);
            bsp_uart_set_modbus_finished(2);

            stv_UartPortStatus[2].mcv_Status = UART_IDLE;
            SET_UART_SM_FLAG(2, UART_SM_IDLE);
            break;
        }
        break;

    case UART2_TX_TIMER_ID:
        switch(gtp_UartPort[2].mcv_Mode)
        {
        case UART_TYPE_FREE_PORT:
            /*置标志位*/
            SET_UART_SM_FLAG(2, UART_SM_RX_FINISH);
            SET_UART_SD_VALUE(2, UART_SD_FINISH_RX_CHAR, FREE_COM_FINISH_RX_FRAME_TIMEOUT);
            /*清除接收使能标志*/
            RST_UART_SM_FLAG(2, UART_SM_FREE_PORT_RX_EN);

            stv_UartPortStatus[2].mcv_Status = UART_IDLE;
            SET_UART_SM_FLAG(2, UART_SM_IDLE);
            break;
        case UART_TYPE_MB_SLAVE:

            break;

        case UART_TYPE_MB_MASTER:
            //LOGV("bsp_uart", "stv_UartPortStatus[1].mcv_RetryCnt = %u", stv_UartPortStatus[1].mcv_RetryCnt);
            /*发送超时，重发*/
            if(stv_UartPortStatus[2].mcv_RetryCnt != 0)
            {
                stv_UartPortStatus[2].mcv_RetryCnt--;
                /*重新发送*/
                stv_UartPortStatus[2].mcv_Status = UART_TX;
                stv_UartPortStatus[2].msv_TxLength = 0;
                xTimerStart(ltv_Uart2TxTime, 100);
            }
            else
            {
                LOGV("bsp_uart", "uart2 send timeout!");
#if 0 // TODO:
                USART_ITConfig(UART4, USART_IT_TXE, DISABLE);
                USART_ITConfig(UART4, USART_IT_TC, DISABLE);
#endif
                stv_UartPortStatus[2].mcv_Status = UART_IDLE;
                stv_UartPortStatus[2].msv_TxLength = 0;
                stv_UartPortStatus[2].msv_TxTotalLength = 0;
                stv_UartPortStatus[2].mcv_RetryCnt = 0;

                /*默认使能RX*/
                bsp_uart_rx_enable(2);

                /*超时返回*/
                //SET_UART_SM_FLAG(1, UART_SM_MODBUS_FINISH);
                bsp_uart_set_modbus_finished(2);
                /*设置模式错误标志*/
                SET_UART_SM_FLAG(2, UART_SM_MODBUS_ERROR);
                SET_UART_SD_VALUE(2, UART_SD_MASTER_ERROR_CODE, MB_ERROR_SLAVE_BUSY);

                stv_UartPortStatus[2].mcv_Status = UART_IDLE;
                SET_UART_SM_FLAG(2, UART_SM_IDLE);
            }
        }
        break;
    }
}

static void handle_touchuan(uint16_t port, uint8_t recvChar)
{
    TimerHandle_t timerHandle;
    switch (port)
    {
        case 0:
            timerHandle = ltv_Uart0RxTime;
            break;
        case 1:
            timerHandle = ltv_Uart1RxTime;
            break;
        case 2:
            timerHandle = ltv_Uart2RxTime;
            break;
        default:
            timerHandle = NULL;
            break;
    }
    if (timerHandle == NULL)
    {
        LOGE("handle_touchuan", "timerHandle == NULL, so just return");
        return;
    }
    /*帧开始*/
    if(stv_UartPortStatus[port].mcv_Status == UART_IDLE)
    {
        /*申请缓存区*/
        switch (port)
        {
            case 0:
                stv_UartPortStatus[port].mcp_RxBuff = kalyke_ring_buffer_get_write_mem_uart0(&gtv_Uart0ReadBuffer);
                break;
            case 1:
                stv_UartPortStatus[port].mcp_RxBuff = kalyke_ring_buffer_get_write_mem_uart1(&gtv_Uart1ReadBuffer);
                break;
            case 2:
                stv_UartPortStatus[port].mcp_RxBuff = kalyke_ring_buffer_get_write_mem_uart2(&gtv_Uart2ReadBuffer);
                break;
            default:
                break;
        }

        stv_UartPortStatus[port].msv_RxLength = 0;

        //gTick = xTaskGetTickCount();
        /*启动定时器*/
        xTimerStart(timerHandle, 0);

        stv_UartPortStatus[port].mcv_Status = UART_RX;
        RST_UART_SM_FLAG(port, UART_SM_IDLE);
    }

    /*复位定时器*/
    BaseType_t ret = xTimerReset(timerHandle, 0);
    if (ret == pdFAIL)
    {
        LOGE("handle_touchuan", "Reset timer ERROR!!!!");
    }

    stv_UartPortStatus[port].mcp_RxBuff[stv_UartPortStatus[port].msv_RxLength++] = recvChar;
}

static void uart0_worker_task(void *pvParameters)
{
    BaseType_t ret;
    unsigned short lsv_FreeComFinishFlag = 0;
    unsigned char lcv_RecvChar;
    int error;
    size_t n;
    taskENTER_CRITICAL();
    uint32_t clock = BOARD_DebugConsoleSrcFreq();

    NVIC_SetPriority(LPUART4_IRQn, 5);
    lpuart_config0.base = LPUART4;
    lpuart_config0.srcclk = clock;
    if (0 > LPUART_RTOS_Init(&gLpuart0RTOSHandle, &t_handle, &lpuart_config0))
    {
        LOGE("bsp_uart", "LPUART_RTOS_Init ERROR!\r\n");
        vTaskSuspend(NULL);
    }
    bsp_uart_rx_enable(0);
    stv_UartPortStatus[0].mcv_Status = UART_IDLE;
    kalyke_ring_buffer_init_uart0(&gtv_Uart0ReadBuffer, 1024);

    LOGV("bsp_uart", "uart0_worker_task RUN! Free heap size is %d bytes\r\n", xPortGetFreeHeapSize());
    taskEXIT_CRITICAL();
    /* Receive user input just one byte. */
    do
    {
        error = LPUART_RTOS_Receive(&gLpuart0RTOSHandle, recv_buffer0, 1, &n);
        //PRINTF("error = %d, n = %d, recv = 0x%x, mcv_Mode = %d\r\n", error, n, recv_buffer0[0],gtp_UartPort[0].mcv_Mode);
        if (error == kStatus_LPUART_RxHardwareOverrun)
        {
            /* Notify about hardware buffer overrun */
#if 0
            if (kStatus_Success !=
                    LPUART_RTOS_Send(&gLpuartRTOSHandle, (uint8_t *)send_hardware_overrun, strlen(send_hardware_overrun)))
            {
                vTaskSuspend(NULL);
            }
#endif
            LOGE("bsp_uart", "Hardware buffer overrun!\r\n");
        }
        if (error == kStatus_LPUART_RxRingBufferOverrun)
        {
            /* Notify about ring buffer overrun */
#if 0
            if (kStatus_Success != LPUART_RTOS_Send(&gLpuartRTOSHandle, (uint8_t *)send_ring_overrun, strlen(send_ring_overrun)))
            {
                vTaskSuspend(NULL);
            }
#endif
            LOGE("bsp_uart", "Ring buffer overrun!\r\n");
        }
        if (n > 0)
        {
            lcv_RecvChar = (unsigned char)recv_buffer0[0];
            if (gTouChuan == 0)
            {
                handle_touchuan(0, lcv_RecvChar);
                continue;
            }

            switch(gtp_UartPort[0].mcv_Mode)
            {
            case UART_TYPE_MB_SLAVE:
                /*帧开始*/
                if(stv_UartPortStatus[0].mcv_Status == UART_IDLE)
                {
                    /*申请缓存区*/
                    stv_UartPortStatus[0].mcp_RxBuff = kalyke_ring_buffer_get_write_mem_uart0(&gtv_Uart0ReadBuffer);
                    configASSERT(stv_UartPortStatus[0].mcp_RxBuff != NULL);

                    stv_UartPortStatus[0].msv_RxLength = 0;

                    gTick = xTaskGetTickCount();
                    /*启动定时器*/
                    //LOGV("bsp_uart", "xTimerStart: ltv_Uart0RxTime...................ms = %u\r\n", gTick);
                    xTimerStart(ltv_Uart0RxTime, 0);

                    stv_UartPortStatus[0].mcv_Status = UART_RX;
                    RST_UART_SM_FLAG(0, UART_SM_IDLE);
                }

                /*复位定时器*/
                ret = xTimerReset(ltv_Uart0RxTime, 0);
                if (ret == pdFAIL)
                {
                    LOGE("bsp_uart", "Reset timer ERROR!!!!");
                }

                stv_UartPortStatus[0].mcp_RxBuff[stv_UartPortStatus[0].msv_RxLength++] = lcv_RecvChar;
                break;

            case UART_TYPE_FREE_PORT:
              //  LOGV("uart0Worker", "msv_StartChar = 0x%X, lcv_RecvChar = 0x%X", gtp_UartPort[0].mtv_ModeInfo.FreePort.msv_StartChar, lcv_RecvChar);
              //  LOGW("uart0Worker", "msv_Flag = 0x%X, mcv_Status = 0x%X", gtp_UartPort[0].mtv_ModeInfo.FreePort.msv_Flag, stv_UartPortStatus[0].mcv_Status);
                /*帧开始*/
                if(stv_UartPortStatus[0].mcv_Status == UART_IDLE)
                {
                    if(gtp_UartPort[0].mtv_ModeInfo.FreePort.msv_Flag & FREE_COM_START_CHAR_EN)
                    {
                        /*非开始字符，直接跳出*/
                        if(gtp_UartPort[0].mtv_ModeInfo.FreePort.msv_StartChar != lcv_RecvChar)
                        {
                            break;
                        }
                    }

                    stv_UartPortStatus[0].msv_RxLength = 0;

                    /*字符间超时使能*/
                    if(gtp_UartPort[0].mtv_ModeInfo.FreePort.msv_Flag & FREE_COM_WORD_TIMEOUT)
                    {
                        gTick = xTaskGetTickCount();
                        /*启动定时器*/
                        xTimerStart(ltv_Uart0RxTime, 100);
                    }

                    /*帧间超时使能*/
                    if(gtp_UartPort[0].mtv_ModeInfo.FreePort.msv_Flag & FREE_COM_FRAME_TIMEOUT)
                    {
                        gTick = xTaskGetTickCount();
                        /*启动定时器*/
                        xTimerStart(ltv_Uart0TxTime, 100);
                    }

                    stv_UartPortStatus[0].mcv_Status = UART_RX;
                    RST_UART_SM_FLAG(0, UART_SM_IDLE);
                    lsv_FreeComFinishFlag = 0;
                }

                if(!GET_UART_SM_FLAG(0, UART_SM_FREE_PORT_RX_EN))
                {
                    lsv_FreeComFinishFlag = FREE_COM_FINISH_RX_USER_STOP;
                }
               // LOGD("uart0Worker", "lsv_FreeComFinishFlag1 = 0x%X", lsv_FreeComFinishFlag);
                if(!lsv_FreeComFinishFlag)
                {
                    /*复位定时器*/
                    if(gtp_UartPort[0].mtv_ModeInfo.FreePort.msv_Flag & FREE_COM_WORD_TIMEOUT)
                    {
                        xTimerReset(ltv_Uart0RxTime, 100);
                    }
                    if(gtp_UartPort[0].mtv_ModeInfo.FreePort.msv_Flag & FREE_COM_FRAME_TIMEOUT)
                    {
                        xTimerReset(ltv_Uart0TxTime, 100);
                    }

                    stv_UartPortStatus[0].msp_FreeRxBuff[stv_UartPortStatus[0].msv_RxLength++] = lcv_RecvChar;

                    SET_UART_SD_VALUE(0, UART_SD_CURRENT_RX_CHAR, lcv_RecvChar);
                    SET_UART_SD_VALUE(0, UART_SD_CURRENT_RX_LEN, stv_UartPortStatus[0].msv_RxLength);

                    if(gtp_UartPort[0].mtv_ModeInfo.FreePort.msv_Flag & FREE_COM_END_CHAR_EN)
                    {
                        /*接收到结束字符，帧结束*/
                        if(gtp_UartPort[0].mtv_ModeInfo.FreePort.msv_EndChar == lcv_RecvChar)
                        {
                            lsv_FreeComFinishFlag = FREE_COM_FINISH_RX_RCV_END_CHAR;
                        }
                    }

                    if(stv_UartPortStatus[0].msv_RxLength >= stv_UartPortStatus[0].msv_FreeRxMaxCnt)
                    {
                        lsv_FreeComFinishFlag = FREE_COM_FINISH_RX_MAX_RCV_CNT;
                    }
                }
              //  LOGW("uart0Worker", "lsv_FreeComFinishFlag2 = 0x%X", lsv_FreeComFinishFlag);
                if(lsv_FreeComFinishFlag)
                {
                    /*帧结束*/
                    xTimerStop(ltv_Uart0RxTime, 100);
                    xTimerStop(ltv_Uart0TxTime, 100);

                    /*置标志位*/
                    if(lsv_FreeComFinishFlag != FREE_COM_FINISH_RX_USER_STOP)
                    {
                        SET_UART_SM_FLAG(0, UART_SM_RX_FINISH);
                    }
                    SET_UART_SD_VALUE(0, UART_SD_FINISH_RX_CHAR, lsv_FreeComFinishFlag);
                    /*清除接收使能标志*/
                    RST_UART_SM_FLAG(0, UART_SM_FREE_PORT_RX_EN);

                    stv_UartPortStatus[0].mcv_Status = UART_IDLE;
                    SET_UART_SM_FLAG(0, UART_SM_IDLE);

                    //bsp_refresh_communication_led(0, UART0_A_LED_MASK);
                }
                break;

            case UART_TYPE_MB_MASTER:
                if(GET_UART_SM_FLAG(0, UART_SM_MODBUS_FINISH))
                {
                    LOGE("bsp_uart", "OMG, just break!!!");
                    break;
                }

                if(stv_UartPortStatus[0].msv_RxLength == 0)
                {
                    /*停止TX超时计时器*/
                    xTimerStop(ltv_Uart0TxTime, 0);

                    gTick = xTaskGetTickCount();
                    /*启动定时器*/
                  //  LOGV("bsp_uart", "xTimerStart: ltv_Uart0RxTime. Current ms = %u\r\n", gTick);
                    /*启动帧接收定时器*/
                    xTimerStart(ltv_Uart0RxTime, 0);
                }

                /*复位定时器*/
                xTimerReset(ltv_Uart0RxTime, 0);

                if (stv_UartPortStatus[0].aFlag == 0)
                {
                    stv_UartPortStatus[0].msp_FreeRxBuff[stv_UartPortStatus[0].msv_RxLength++] = lcv_RecvChar;
                }
                else
                {
                    stv_UartPortStatus[0].aBuf[stv_UartPortStatus[0].msv_RxLength++] = lcv_RecvChar;
                }
                //LOGV("bsp_uart", "lcv_RecvChar = 0x%X", lcv_RecvChar);

                if(stv_UartPortStatus[0].msv_RxLength >= stv_UartPortStatus[0].msv_FreeRxMaxCnt)
                {
                  //  LOGV("bsp_uart", "uart0 recv finished...001");
                    /*帧结束*/
                    xTimerStop(ltv_Uart0RxTime, 0);

                    modlink_copy(0);
                    SET_UART_SM_FLAG(0, UART_SM_RX_FINISH);
                    bsp_uart_set_modbus_finished(0);
                    RST_UART_SM_FLAG(0, UART_SM_MODBUS_ERROR);

                    stv_UartPortStatus[0].mcv_Status = UART_IDLE;
                    SET_UART_SM_FLAG(0, UART_SM_IDLE);
                }
                break;
            }
        }
        //PRINTF("%dB\r\n", n);
    }
    while(1); // while (kStatus_Success == error);

    //LPUART_RTOS_Deinit(&gLpuart0RTOSHandle);
    //vTaskSuspend(NULL);
}

// This is RS485
static void bsp_uart0_init(unsigned long llv_BaudRate, unsigned char lcv_Parity, unsigned char lcv_WordLength, unsigned char lcv_StopBits)
{
    //uint32_t lpUart1Priority;
    LOGV("bsp_uart", "Enter %s, llv_BaudRate=%d, lcv_Parity=%d, lcv_WordLength=%d, lcv_StopBits=%d.\r\n", __func__, llv_BaudRate, lcv_Parity, lcv_WordLength, lcv_StopBits);
    if (g_Uart0WorkerHandler != NULL)
    {
        LOGV("bsp_uart", "COM0 worker is running, so just return.\r\n");
        return;
    }

    lpuart_config0.baudrate = llv_BaudRate;
    switch(lcv_StopBits)
    {
    case UART_STB_1:
        lpuart_config0.stopbits = kLPUART_OneStopBit;
        break;
    case UART_STB_2:
        lpuart_config0.stopbits = kLPUART_TwoStopBit;
        break;
    }
    switch(lcv_Parity)
    {
    case UART_PARITY_NO:
        lpuart_config0.parity = kLPUART_ParityDisabled;
        break;

    case UART_PARITY_EVEN:
        lpuart_config0.parity = kLPUART_ParityEven;//偶
        break;

    case UART_PARITY_ODD:
        lpuart_config0.parity = kLPUART_ParityOdd;//奇
        break;
    }
    if (lcv_WordLength == 8)
    {
        lpuart_config0.dataBits = kLPUART_EightDataBits;
    }
    else
    {
        lpuart_config0.dataBits = kLPUART_SevenDataBits;
    }
    //lpUart1Priority = NVIC_GetPriority(LPUART1_IRQn);
    //LOGD("bsp_uart", "lpUart1Priority = %d", lpUart1Priority);
INIT_DONE:
    if (xTaskCreate(uart0_worker_task, "Uart_worker0",
                    UART_WORKER_TASK_STACK_SIZE, NULL,
                    UART_WORKER_TASK_PRIORITY, (TaskHandle_t *)&g_Uart0WorkerHandler) != pdPASS)
    {
        LOGV("bsp_uart", "Uart_worker0 task creation failed!\r\n");
        while (1)
            ;
    }
}

void bsp_uart0_deinit(void)
{
    //LOGV("bsp_uart", "Enter %s()\r\n", __func__);
    LPUART_RTOS_Deinit(&gLpuart0RTOSHandle);
    vTaskDelete(g_Uart0WorkerHandler);
}

/**
  * @brief  485模式RX使能
  * @param  None
  * @retval None
  */
void bsp_uart_rx_enable(uint8_t portNum)
{
    //LOGI("bsp_uart", "Enter %s(), portNum = %d, mcv_RS485Mode = %d", __func__, portNum, gtv_DeviceConfigTable.mtv_UartPort.mta_PortInfo[portNum].mcv_RS485Mode);
    if(gtv_DeviceConfigTable.mtv_UartPort.mta_PortInfo[portNum].mcv_RS485Mode)
    {
    #if 0
        if (portNum == 0)
        {
#if (KALYKE_BOARD_P0 == 1)
            GPIO_PortClear(MCU_UART1_DE_GPIO, MCU_UART1_DE_MASK);
#endif
        }
        else if (portNum == 1)
        {
            GPIO_PortClear(MCU_UART2_DE_GPIO, MCU_UART2_DE_MASK);
        }
    #endif
    }
}

/**
  * @brief  485模式TX使能
  * @param  None
  * @retval None
  */
void bsp_uart_tx_enable(uint8_t portNum)
{
    //LOGI("bsp_uart", "Enter %s(), portNum = %d, mcv_RS485Mode = %d", __func__, portNum, gtv_DeviceConfigTable.mtv_UartPort.mta_PortInfo[portNum].mcv_RS485Mode);
    if(gtv_DeviceConfigTable.mtv_UartPort.mta_PortInfo[portNum].mcv_RS485Mode)
    {
    #if 0 // LiXianyu 20210808
        if (portNum == 0)
        {
#if (KALYKE_BOARD_P0 == 1)
            GPIO_PortSet(MCU_UART1_DE_GPIO, MCU_UART1_DE_MASK);
#endif
        }
        else if (portNum == 1)
        {
            GPIO_PortSet(MCU_UART2_DE_GPIO, MCU_UART2_DE_MASK);
        }
    #endif
    }
}

void bsp_uart_delay(void)
{
    volatile uint32_t i = 0;
    for (i = 0; i < 9000000; ++i)
    {
        __asm("NOP"); /* delay */
    }
}

void bsp_com0_send_data(uint8_t *pData, uint16_t len)
{
    //LOGD("bsp_uart", "Enter %s()", __func__);
    int ret = LPUART_RTOS_Send(&gLpuart0RTOSHandle, (uint8_t *)pData, len);
    //LOGI("bsp_uart", "Leave %s(), LPUART_RTOS_Send return : %d", __func__, ret);
}

void bsp_uart0_send_buffer(unsigned char *lcp_SendBuff, unsigned short lsv_Length)
{
    LOGV("bsp_uart", "Enter %s(), lsv_Length = %d", __func__, lsv_Length);
    hexdump(lcp_SendBuff, lsv_Length);
    bsp_uart_tx_enable(0);
    LPUART_RTOS_Send(&gLpuart0RTOSHandle, (uint8_t *)lcp_SendBuff, lsv_Length);
    bsp_send_finished(0);
}

void bsp_uart0_time_init(unsigned short lsv_RxPeriod, unsigned short lsv_TxPeriod)
{
    LOGV("bsp_uart", "Enter %s, lsv_RxPeriod = %d, lsv_TxPeriod = %d\r\n", __func__, lsv_RxPeriod, lsv_TxPeriod);
    if (lsv_RxPeriod == 0)
    {
        lsv_RxPeriod = 10;
    }
    if((ltv_Uart0RxTime == NULL) && (lsv_RxPeriod > 0))
    {
        ltv_Uart0RxTime = xTimerCreate((const char *)"Uart0 RxTime",
                                       (TickType_t  )lsv_RxPeriod / portTICK_PERIOD_MS,
                                       (UBaseType_t )pdFALSE,
                                       (void *      )UART0_RX_TIMER_ID,
                                       (TimerCallbackFunction_t)bsp_timer_callback_func);
    }

    if (lsv_TxPeriod == 0)
    {
        lsv_TxPeriod = 1000;
    }
    if((ltv_Uart0TxTime == NULL) && (lsv_TxPeriod > 0))
    {
        ltv_Uart0TxTime = xTimerCreate((const char *)"Uart0 TxTime",
                                       (TickType_t  )lsv_TxPeriod / portTICK_PERIOD_MS,
                                       (UBaseType_t )pdFALSE,
                                       (void *      )UART0_TX_TIMER_ID,
                                       (TimerCallbackFunction_t)bsp_timer_callback_func);
    }
}

/* For COM1 begin */
static TaskHandle_t g_Uart1WorkerHandler = NULL;
static lpuart_rtos_handle_t gLpuart1RTOSHandle;
static struct _lpuart_handle t_handle1;
static uint8_t recv_buffer1[16];
static uint8_t background_buffer1[1024];
static lpuart_rtos_config_t lpuart_config1 =
{
    .baudrate = 115200,
    .parity = kLPUART_ParityDisabled,
    .stopbits = kLPUART_OneStopBit,
    .buffer = background_buffer1,
    .buffer_size = sizeof(background_buffer1),
    //.enableRxRTS = false,
    //.enableTxCTS = false,
};

// Just for COM1 use!
static uint8_t gRing3[1024];
static uint8_t gRing4[1024];
static void kalyke_ring_buffer_init_uart1(ring_buffer_st *ltp_RingBuff, unsigned short lsv_BuffSize)
{
    ltp_RingBuff->mcp_Buff[0] = gRing3;
    ltp_RingBuff->mcp_Buff[1] = gRing4;
    ltp_RingBuff->mcv_Index = 0;
}
static void kalyke_ring_buffer_switch_write_mem_uart1(ring_buffer_st *ltp_RingBuff)
{
    if(ltp_RingBuff->mcv_Index + 1 < 2)
    {
        ltp_RingBuff->mcv_Index += 1;
    }
    else
    {
        ltp_RingBuff->mcv_Index = 0;
    }
}
static unsigned char *kalyke_ring_buffer_get_write_mem_uart1(ring_buffer_st *ltp_RingBuff)
{
    return ltp_RingBuff->mcp_Buff[ltp_RingBuff->mcv_Index];
}

static void uart1_worker_task(void *pvParameters)
{
    LOGV("bsp_uart", "uart1_worker_task RUN! Free heap size is %d bytes\r\n", xPortGetFreeHeapSize());
    uint8_t curPort = 1;
    BaseType_t ret;
    unsigned short lsv_FreeComFinishFlag = 0;
    unsigned char lcv_RecvChar;
    int error;
    size_t n;
    uint32_t clock = BOARD_DebugConsoleSrcFreq();

    NVIC_SetPriority(LPUART3_IRQn, 5);
    lpuart_config1.srcclk = clock;
    lpuart_config1.base = LPUART3;

    if (0 > LPUART_RTOS_Init(&gLpuart1RTOSHandle, &t_handle1, &lpuart_config1))
    {
        LOGV("bsp_uart", "LPUART_RTOS_Init ERROR!\r\n");
        vTaskSuspend(NULL);
    }
    bsp_uart_rx_enable(curPort);
    stv_UartPortStatus[curPort].mcv_Status = UART_IDLE;
    kalyke_ring_buffer_init_uart1(&gtv_Uart1ReadBuffer, 1024);


    /* Receive user input just one byte. */
    do
    {
        error = LPUART_RTOS_Receive(&gLpuart1RTOSHandle, recv_buffer1, 1, &n);
        //PRINTF("error = %d, n = %d, recv = 0x%x, mcv_Mode = %d\r\n", error, n, recv_buffer1[0],gtp_UartPort[0].mcv_Mode);
        if (error == kStatus_LPUART_RxHardwareOverrun)
        {
            /* Notify about hardware buffer overrun */
#if 0
            if (kStatus_Success !=
                    LPUART_RTOS_Send(&gLpuartRTOSHandle, (uint8_t *)send_hardware_overrun, strlen(send_hardware_overrun)))
            {
                vTaskSuspend(NULL);
            }
#endif
            LOGV("bsp_uart", "Hardware buffer overrun!\r\n");
        }
        if (error == kStatus_LPUART_RxRingBufferOverrun)
        {
            /* Notify about ring buffer overrun */
#if 0
            if (kStatus_Success != LPUART_RTOS_Send(&gLpuartRTOSHandle, (uint8_t *)send_ring_overrun, strlen(send_ring_overrun)))
            {
                vTaskSuspend(NULL);
            }
#endif
            LOGV("bsp_uart", "Ring buffer overrun!\r\n");
        }
        if (n > 0)
        {
            lcv_RecvChar = (unsigned char)recv_buffer1[0];
            if (gTouChuan == 1)
            {
                handle_touchuan(1, lcv_RecvChar);
                continue;
            }
//            LOGD("bsp_uart", "gtp_UartPort[curPort].mcv_Mode = %d", gtp_UartPort[curPort].mcv_Mode);
            switch(gtp_UartPort[curPort].mcv_Mode)
            {
            case UART_TYPE_MB_SLAVE:
                /*帧开始*/
                if(stv_UartPortStatus[curPort].mcv_Status == UART_IDLE)
                {
                    /*申请缓存区*/
                    stv_UartPortStatus[curPort].mcp_RxBuff = kalyke_ring_buffer_get_write_mem_uart1(&gtv_Uart1ReadBuffer);
                    configASSERT(stv_UartPortStatus[curPort].mcp_RxBuff != NULL);

                    stv_UartPortStatus[curPort].msv_RxLength = 0;

                    gTick = xTaskGetTickCount();
                    /*启动定时器*/
                  //  LOGD("bsp_uart", "xTimerStart: ltv_Uart1RxTime...................ms = %u\r\n", gTick);
                    xTimerStart(ltv_Uart1RxTime, 0);

                    stv_UartPortStatus[curPort].mcv_Status = UART_RX;
                    RST_UART_SM_FLAG(curPort, UART_SM_IDLE);
                }

                /*复位定时器*/
                ret = xTimerReset(ltv_Uart1RxTime, 0);
                if (ret == pdFAIL)
                {
                    LOGV("bsp_uart", "Reset timer ERROR!!!!");
                }

                stv_UartPortStatus[curPort].mcp_RxBuff[stv_UartPortStatus[curPort].msv_RxLength++] = lcv_RecvChar;
                break;
#if 1
            case UART_TYPE_FREE_PORT:
                /*帧开始*/
                if(stv_UartPortStatus[curPort].mcv_Status == UART_IDLE)
                {
                //    LOGW("bsp_uart", "msv_StartChar = %u, lcv_RecvChar = %u", gtp_UartPort[curPort].mtv_ModeInfo.FreePort.msv_StartChar, lcv_RecvChar);
                    //LOGI("bsp_uart", "msv_Flag = %X", gtp_UartPort[curPort].mtv_ModeInfo.FreePort.msv_Flag);
                    if(gtp_UartPort[curPort].mtv_ModeInfo.FreePort.msv_Flag & FREE_COM_START_CHAR_EN)
                    {
                        /*非开始字符，直接跳出*/
                        if(gtp_UartPort[curPort].mtv_ModeInfo.FreePort.msv_StartChar != lcv_RecvChar)
                            break;
                    }

                    stv_UartPortStatus[curPort].msv_RxLength = 0;

                    /*字符间超时使能*/
                    if(gtp_UartPort[curPort].mtv_ModeInfo.FreePort.msv_Flag & FREE_COM_WORD_TIMEOUT)
                    {
                        /*启动定时器*/
                        xTimerStart(ltv_Uart1RxTime, 10);
                    }

                    /*帧间超时使能*/
                    if(gtp_UartPort[curPort].mtv_ModeInfo.FreePort.msv_Flag & FREE_COM_FRAME_TIMEOUT)
                    {
                        /*启动定时器*/
                        xTimerStart(ltv_Uart1TxTime, 1);
                    }

                    stv_UartPortStatus[curPort].mcv_Status = UART_RX;
                    RST_UART_SM_FLAG(curPort, UART_SM_IDLE);
                    lsv_FreeComFinishFlag = 0;
                }

                if(!GET_UART_SM_FLAG(curPort, UART_SM_FREE_PORT_RX_EN))
                {
                    lsv_FreeComFinishFlag = FREE_COM_FINISH_RX_USER_STOP;
                }

                if(!lsv_FreeComFinishFlag)
                {
                    /*复位定时器*/
                    if(gtp_UartPort[curPort].mtv_ModeInfo.FreePort.msv_Flag & FREE_COM_WORD_TIMEOUT)
                    {
                        xTimerReset(ltv_Uart1RxTime, 10);
                    }
                    if(gtp_UartPort[curPort].mtv_ModeInfo.FreePort.msv_Flag & FREE_COM_FRAME_TIMEOUT)
                    {
                        xTimerReset(ltv_Uart1TxTime, 10);
                    }

                    stv_UartPortStatus[curPort].msp_FreeRxBuff[stv_UartPortStatus[curPort].msv_RxLength++] = lcv_RecvChar;

                    SET_UART_SD_VALUE(curPort, UART_SD_CURRENT_RX_CHAR, lcv_RecvChar);
                    SET_UART_SD_VALUE(curPort, UART_SD_CURRENT_RX_LEN, stv_UartPortStatus[curPort].msv_RxLength);

                    if(gtp_UartPort[curPort].mtv_ModeInfo.FreePort.msv_Flag & FREE_COM_END_CHAR_EN)
                    {
                        /*接收到结束字符，帧结束*/
                        if(gtp_UartPort[curPort].mtv_ModeInfo.FreePort.msv_EndChar == lcv_RecvChar)
                        {
                            lsv_FreeComFinishFlag = FREE_COM_FINISH_RX_RCV_END_CHAR;
                        }
                    }

                    if(stv_UartPortStatus[curPort].msv_RxLength >= stv_UartPortStatus[curPort].msv_FreeRxMaxCnt)
                    {
                        lsv_FreeComFinishFlag = FREE_COM_FINISH_RX_MAX_RCV_CNT;
                    }
                }

                if(lsv_FreeComFinishFlag)
                {
                    /*帧结束*/
                    xTimerStop(ltv_Uart1RxTime, 10);
                    xTimerStop(ltv_Uart1TxTime, 10);

                    /*置标志位*/
                    if(lsv_FreeComFinishFlag != FREE_COM_FINISH_RX_USER_STOP)
                    {
                        SET_UART_SM_FLAG(curPort, UART_SM_RX_FINISH);
                    }
                    SET_UART_SD_VALUE(curPort, UART_SD_FINISH_RX_CHAR, lsv_FreeComFinishFlag);
                    /*清除接收使能标志*/
                    RST_UART_SM_FLAG(curPort, UART_SM_FREE_PORT_RX_EN);

                    stv_UartPortStatus[curPort].mcv_Status = UART_IDLE;
                    SET_UART_SM_FLAG(curPort, UART_SM_IDLE);

                    //bsp_refresh_communication_led(0, UART1_A_LED_MASK);
                }
                break;

            case UART_TYPE_MB_MASTER:
                if(GET_UART_SM_FLAG(curPort, UART_SM_MODBUS_FINISH))
                {
                    LOGE("bsp_uart", "OMG, just break!!!");
                    break;
                }

                if(stv_UartPortStatus[curPort].msv_RxLength == 0)
                {
                    /*停止TX超时计时器*/
                    xTimerStop(ltv_Uart1TxTime, 100);

                    gTick = xTaskGetTickCount();
                    /*启动定时器*/
                    //LOGV("bsp_uart", "xTimerStart: ltv_Uart1RxTime. Current ms = %u\r\n", gTick);
                    /*启动帧接收定时器*/
                    xTimerStart(ltv_Uart1RxTime, 10);
                }

                /*复位定时器*/
                xTimerReset(ltv_Uart1RxTime, 10);

                if (stv_UartPortStatus[curPort].aFlag == 0)
                {
                    stv_UartPortStatus[curPort].msp_FreeRxBuff[stv_UartPortStatus[curPort].msv_RxLength++] = lcv_RecvChar;
                }
                else
                {
                    stv_UartPortStatus[curPort].aBuf[stv_UartPortStatus[curPort].msv_RxLength++] = lcv_RecvChar;
                }
               // LOGV("bsp_uart", "lcv_RecvChar = 0x%X", lcv_RecvChar);

                if(stv_UartPortStatus[curPort].msv_RxLength >= stv_UartPortStatus[curPort].msv_FreeRxMaxCnt)
                {
                    //LOGE("bsp_uart", "uart1 recv finished...001");
                    /*帧结束*/
                    xTimerStop(ltv_Uart1RxTime, 10);

                    modlink_copy(curPort);

                    SET_UART_SM_FLAG(curPort, UART_SM_RX_FINISH);
                    //SET_UART_SM_FLAG(curPort, UART_SM_MODBUS_FINISH);
                    bsp_uart_set_modbus_finished(1);
                    RST_UART_SM_FLAG(curPort, UART_SM_MODBUS_ERROR);

                    stv_UartPortStatus[curPort].mcv_Status = UART_IDLE;
                    SET_UART_SM_FLAG(curPort, UART_SM_IDLE);

                    //bsp_refresh_communication_led(0, UART1_A_LED_MASK);
                }
                break;
#endif
            }
        }
        //PRINTF("%dB\r\n", n);
    }
    while(1); // while (kStatus_Success == error);

    //LPUART_RTOS_Deinit(&gLpuart0RTOSHandle);
    //vTaskSuspend(NULL);
}

void suspend_uart1_task(void)
{
    LOGV("bsp_uart", "Enter %s(), g_Uart1WorkerHandler = 0x%08X", __func__, g_Uart1WorkerHandler);
    if (g_Uart1WorkerHandler)
    {
        vTaskSuspend(g_Uart1WorkerHandler);
    }
}
void resume_uart1_task(void)
{
    if (g_Uart1WorkerHandler)
    {
        vTaskResume(g_Uart1WorkerHandler);
    }
}

static void set485_delay(uint32_t baudRate)
{
#if (LOG_OPEN == 1)    
    if (baudRate <= 9600)
    {
        if (baudRate == 1200)
        {
            SET_SD_ELEMENT_VALUE(SD400, 3);
        }
        else if (baudRate == 2400)
        {
            SET_SD_ELEMENT_VALUE(SD400, 2);
        }
        else if (baudRate == 4800)
        {
            SET_SD_ELEMENT_VALUE(SD400, 2);
        }
        else if (baudRate == 9600)
        {
            SET_SD_ELEMENT_VALUE(SD400, 1);
        }
    }
    else
    {
        SET_SD_ELEMENT_VALUE(SD400, 1);
    }
#elif (LOG_OPEN == 0)
    if (baudRate <= 9600)
    {
        if (baudRate == 1200)
        {
            SET_SD_ELEMENT_VALUE(SD400, 3);
        }
        else if (baudRate == 2400)
        {
            SET_SD_ELEMENT_VALUE(SD400, 2);
        }
        else if (baudRate == 4800)
        {
            SET_SD_ELEMENT_VALUE(SD400, 2);
        }
        else if (baudRate == 9600)
        {
            SET_SD_ELEMENT_VALUE(SD400, 1);
        }
    }
    else
    {
        SET_SD_ELEMENT_VALUE(SD400, 1);
    }
#endif
}

// This is RS485
void bsp_uart1_init(unsigned long llv_BaudRate, unsigned char lcv_Parity, unsigned char lcv_WordLength, unsigned char lcv_StopBits)
{
    LOGV("bsp_uart", "Enter %s, llv_BaudRate=%d, lcv_Parity=%d, lcv_WordLength=%d, lcv_StopBits=%d.\r\n", __func__, llv_BaudRate, lcv_Parity, lcv_WordLength, lcv_StopBits);
    set485_delay(llv_BaudRate);
    if (g_Uart1WorkerHandler != NULL)
    {
        LOGV("bsp_uart", "COM1 worker is running, so just return.\r\n");
        return;
    }

    lpuart_config1.baudrate = llv_BaudRate;
    switch(lcv_StopBits)
    {
    case UART_STB_1:
        lpuart_config1.stopbits = kLPUART_OneStopBit;
        break;
    case UART_STB_2:
        lpuart_config1.stopbits = kLPUART_TwoStopBit;
        break;
    }
    switch(lcv_Parity)
    {
    case UART_PARITY_NO:
        lpuart_config1.parity = kLPUART_ParityDisabled;
        break;

    case UART_PARITY_EVEN:
        lpuart_config1.parity = kLPUART_ParityEven;//偶
        break;

    case UART_PARITY_ODD:
        lpuart_config1.parity = kLPUART_ParityOdd;//奇
        break;
    }
    if (lcv_WordLength == 8)
    {
        lpuart_config1.dataBits = kLPUART_EightDataBits;
    }
    else
    {
        lpuart_config1.dataBits = kLPUART_SevenDataBits;
    }
    if (xTaskCreate(uart1_worker_task, "Uart_worker1",
                    UART_WORKER_TASK_STACK_SIZE, NULL,
                    UART_WORKER_TASK_PRIORITY, (TaskHandle_t *)&g_Uart1WorkerHandler) != pdPASS)
    {
        LOGE("bsp_uart", "Uart_worker1 task creation failed!\r\n");
        while (1)
            ;
    }
}

void bsp_com1_send_data(uint8_t *pData, uint16_t len)
{
    LOGD("bsp_uart", "Enter %s()", __func__);
    bsp_uart_tx_enable(1);
    int ret = LPUART_RTOS_Send(&gLpuart1RTOSHandle, (uint8_t *)pData, len);
    TickType_t delays = GET_SD_ELEMENT_VALUE(SD400);
    vTaskDelay(delays / portTICK_PERIOD_MS);
    bsp_uart_rx_enable(1);

    LOGI("bsp_uart", "Leave %s(), LPUART_RTOS_Send return : %d", __func__, ret);
}

TickType_t gUart1SendFinishDelay = 0;

void bsp_uart1_send_buffer(unsigned char *lcp_SendBuff, unsigned short lsv_Length)
{
    LOGI("bsp_uart", "Enter %s(), lsv_Length = %d, mcv_Mode = %u\r\n", __func__, lsv_Length, gtp_UartPort[1].mcv_Mode);
    hexdump(lcp_SendBuff, lsv_Length);
    bsp_uart_tx_enable(1);
    LPUART_RTOS_Send(&gLpuart1RTOSHandle, (uint8_t *)lcp_SendBuff, lsv_Length);
    bsp_send_finished(1);
}

void bsp_uart1_time_init(unsigned short lsv_RxPeriod, unsigned short lsv_TxPeriod)
{
    LOGV("bsp_uart", "Enter %s, lsv_RxPeriod = %d, lsv_TxPeriod = %d\r\n", __func__, lsv_RxPeriod, lsv_TxPeriod);
    if (lsv_RxPeriod == 0)
    {
        lsv_RxPeriod = 10;
    }
    if((ltv_Uart1RxTime == NULL) && (lsv_RxPeriod > 0))
    {
        ltv_Uart1RxTime = xTimerCreate((const char *)"Uart1 RxTime",
                                       (TickType_t  )lsv_RxPeriod / portTICK_PERIOD_MS,
                                       (UBaseType_t )pdFALSE,
                                       (void *      )UART1_RX_TIMER_ID,
                                       (TimerCallbackFunction_t)bsp_timer_callback_func);
    }

    if (lsv_TxPeriod == 0)
    {
        lsv_TxPeriod = 1000;
    }
    if((ltv_Uart1TxTime == NULL) && (lsv_TxPeriod > 0))
    {
        ltv_Uart1TxTime = xTimerCreate((const char *)"Uart1 TxTime",
                                       (TickType_t  )lsv_TxPeriod / portTICK_PERIOD_MS,
                                       (UBaseType_t )pdFALSE,
                                       (void *      )UART1_TX_TIMER_ID,
                                       (TimerCallbackFunction_t)bsp_timer_callback_func);
    }
    //LOGD("bsp_uart", "Leave %s(), ltv_Uart1RxTime = 0x%08X, ltv_Uart1TxTime = 0x%08X", __func__, ltv_Uart1RxTime, ltv_Uart1TxTime);
}

/* For COM2 begin */
static TaskHandle_t g_Uart2WorkerHandler = NULL;
static lpuart_rtos_handle_t gLpuart2RTOSHandle;
static struct _lpuart_handle t_handle2;
static uint8_t recv_buffer2[16];
static uint8_t background_buffer2[1024];
static lpuart_rtos_config_t lpuart_config2 =
{
    .baudrate = 115200,
    .parity = kLPUART_ParityDisabled,
    .stopbits = kLPUART_OneStopBit,
    .buffer = background_buffer2,
    .buffer_size = sizeof(background_buffer2),
    //.enableRxRTS = false,
    //.enableTxCTS = false,
};

// Just for COM2 use!
static uint8_t gRing5[1024];
static uint8_t gRing6[1024];
static void kalyke_ring_buffer_init_uart2(ring_buffer_st *ltp_RingBuff, unsigned short lsv_BuffSize)
{
    ltp_RingBuff->mcp_Buff[0] = gRing5;
    ltp_RingBuff->mcp_Buff[1] = gRing6;
    ltp_RingBuff->mcv_Index = 0;
}
static void kalyke_ring_buffer_switch_write_mem_uart2(ring_buffer_st *ltp_RingBuff)
{
    if(ltp_RingBuff->mcv_Index + 1 < 2)
    {
        ltp_RingBuff->mcv_Index += 1;
    }
    else
    {
        ltp_RingBuff->mcv_Index = 0;
    }
}
static unsigned char *kalyke_ring_buffer_get_write_mem_uart2(ring_buffer_st *ltp_RingBuff)
{
    return ltp_RingBuff->mcp_Buff[ltp_RingBuff->mcv_Index];
}

static void uart2_worker_task(void *pvParameters)
{
    LOGV("COM2", "uart2_worker_task RUN! Free heap size is %d bytes\r\n", xPortGetFreeHeapSize());
    uint8_t curPort = 2;
    BaseType_t ret;
    unsigned short lsv_FreeComFinishFlag = 0;
    unsigned char lcv_RecvChar;
    int error;
    size_t n;
    uint32_t clock = BOARD_DebugConsoleSrcFreq();

    NVIC_SetPriority(LPUART8_IRQn, 5);
    lpuart_config2.srcclk = clock;
    lpuart_config2.base = LPUART8;

    if (0 > LPUART_RTOS_Init(&gLpuart2RTOSHandle, &t_handle2, &lpuart_config2))
    {
        LOGV("COM2", "LPUART_RTOS_Init 2 ERROR!\r\n");
        vTaskSuspend(NULL);
    }
    bsp_uart_rx_enable(curPort);
    stv_UartPortStatus[curPort].mcv_Status = UART_IDLE;
    kalyke_ring_buffer_init_uart2(&gtv_Uart2ReadBuffer, 1024);

    /* Receive user input just one byte. */
    do
    {
        error = LPUART_RTOS_Receive(&gLpuart2RTOSHandle, recv_buffer2, 1, &n);
        //LOGV("COM2", "error = %d, n = %d, recv = 0x%x, mcv_Mode = %d\r\n", error, n, recv_buffer2[0], gtp_UartPort[2].mcv_Mode);
        if (error == kStatus_LPUART_RxHardwareOverrun)
        {
            /* Notify about hardware buffer overrun */
#if 0
            if (kStatus_Success !=
                    LPUART_RTOS_Send(&gLpuartRTOSHandle, (uint8_t *)send_hardware_overrun, strlen(send_hardware_overrun)))
            {
                vTaskSuspend(NULL);
            }
#endif
            LOGE("COM2", "Hardware buffer overrun!");
        }
        if (error == kStatus_LPUART_RxRingBufferOverrun)
        {
            /* Notify about ring buffer overrun */
#if 0
            if (kStatus_Success != LPUART_RTOS_Send(&gLpuartRTOSHandle, (uint8_t *)send_ring_overrun, strlen(send_ring_overrun)))
            {
                vTaskSuspend(NULL);
            }
#endif
            LOGE("COM2", "Ring buffer overrun!");
        }
        if (n > 0)
        {
            lcv_RecvChar = (unsigned char)recv_buffer2[0];
            if (gTouChuan == 2)
            {
                handle_touchuan(2, lcv_RecvChar);
                continue;
            }
            //LOGD("COM2", "gtp_UartPort[curPort].mcv_Mode = %d", gtp_UartPort[curPort].mcv_Mode);
            switch(gtp_UartPort[curPort].mcv_Mode)
            {
            case UART_TYPE_MB_SLAVE:
                /*帧开始*/
                if(stv_UartPortStatus[curPort].mcv_Status == UART_IDLE)
                {
                    /*申请缓存区*/
                    stv_UartPortStatus[curPort].mcp_RxBuff = kalyke_ring_buffer_get_write_mem_uart2(&gtv_Uart2ReadBuffer);
                    configASSERT(stv_UartPortStatus[curPort].mcp_RxBuff != NULL);

                    stv_UartPortStatus[curPort].msv_RxLength = 0;

                    gTick = xTaskGetTickCount();
                    /*启动定时器*/
                  //  LOGD("COM2", "xTimerStart: ltv_Uart2RxTime...................ms = %u\r\n", gTick);
                    xTimerStart(ltv_Uart2RxTime, 100);

                    stv_UartPortStatus[curPort].mcv_Status = UART_RX;
                    RST_UART_SM_FLAG(curPort, UART_SM_IDLE);
                }

                /*复位定时器*/
                ret = xTimerReset(ltv_Uart2RxTime, 100);
                if (ret == pdFAIL)
                {
                    LOGV("COM2", "Reset timer ERROR!!!!");
                }

                stv_UartPortStatus[curPort].mcp_RxBuff[stv_UartPortStatus[curPort].msv_RxLength++] = lcv_RecvChar;
                break;
#if 1
            case UART_TYPE_FREE_PORT:
                /*帧开始*/
                if(stv_UartPortStatus[curPort].mcv_Status == UART_IDLE)
                {
                  //  LOGW("bsp_uart", "msv_StartChar = %u, lcv_RecvChar = %u", gtp_UartPort[curPort].mtv_ModeInfo.FreePort.msv_StartChar, lcv_RecvChar);
                    //LOGD("bsp_uart", "msv_Flag = %X", gtp_UartPort[curPort].mtv_ModeInfo.FreePort.msv_Flag);
                    if(gtp_UartPort[curPort].mtv_ModeInfo.FreePort.msv_Flag & FREE_COM_START_CHAR_EN)
                    {
                        /*非开始字符，直接跳出*/
                        if(gtp_UartPort[curPort].mtv_ModeInfo.FreePort.msv_StartChar != lcv_RecvChar)
                            break;
                    }

                    stv_UartPortStatus[curPort].msv_RxLength = 0;

                    /*字符间超时使能*/
                    if(gtp_UartPort[curPort].mtv_ModeInfo.FreePort.msv_Flag & FREE_COM_WORD_TIMEOUT)
                    {
                        /*启动定时器*/
                        xTimerStart(ltv_Uart2RxTime, 1);
                    }

                    /*帧间超时使能*/
                    if(gtp_UartPort[curPort].mtv_ModeInfo.FreePort.msv_Flag & FREE_COM_FRAME_TIMEOUT)
                    {
                        /*启动定时器*/
                        xTimerStart(ltv_Uart2TxTime, 1);
                    }

                    stv_UartPortStatus[curPort].mcv_Status = UART_RX;
                    RST_UART_SM_FLAG(curPort, UART_SM_IDLE);
                    lsv_FreeComFinishFlag = 0;
                }

                if(!GET_UART_SM_FLAG(curPort, UART_SM_FREE_PORT_RX_EN))
                {
                    lsv_FreeComFinishFlag = FREE_COM_FINISH_RX_USER_STOP;
                }

                if(!lsv_FreeComFinishFlag)
                {
                    /*复位定时器*/
                    if(gtp_UartPort[curPort].mtv_ModeInfo.FreePort.msv_Flag & FREE_COM_WORD_TIMEOUT)
                    {
                        xTimerReset(ltv_Uart2RxTime, 1);
                    }
                    if(gtp_UartPort[curPort].mtv_ModeInfo.FreePort.msv_Flag & FREE_COM_FRAME_TIMEOUT)
                    {
                        xTimerReset(ltv_Uart2TxTime, 1);
                    }

                    stv_UartPortStatus[curPort].msp_FreeRxBuff[stv_UartPortStatus[curPort].msv_RxLength++] = lcv_RecvChar;

                    SET_UART_SD_VALUE(curPort, UART_SD_CURRENT_RX_CHAR, lcv_RecvChar);
                    SET_UART_SD_VALUE(curPort, UART_SD_CURRENT_RX_LEN, stv_UartPortStatus[curPort].msv_RxLength);

                    if(gtp_UartPort[curPort].mtv_ModeInfo.FreePort.msv_Flag & FREE_COM_END_CHAR_EN)
                    {
                        /*接收到结束字符，帧结束*/
                        if(gtp_UartPort[curPort].mtv_ModeInfo.FreePort.msv_EndChar == lcv_RecvChar)
                        {
                            lsv_FreeComFinishFlag = FREE_COM_FINISH_RX_RCV_END_CHAR;
                        }
                    }

                    if(stv_UartPortStatus[curPort].msv_RxLength >= stv_UartPortStatus[curPort].msv_FreeRxMaxCnt)
                    {
                        lsv_FreeComFinishFlag = FREE_COM_FINISH_RX_MAX_RCV_CNT;
                    }
                }

                if(lsv_FreeComFinishFlag)
                {
                    /*帧结束*/
                    xTimerStop(ltv_Uart2RxTime, 1);
                    xTimerStop(ltv_Uart2TxTime, 1);

                    /*置标志位*/
                    if(lsv_FreeComFinishFlag != FREE_COM_FINISH_RX_USER_STOP)
                    {
                        SET_UART_SM_FLAG(curPort, UART_SM_RX_FINISH);
                    }
                    SET_UART_SD_VALUE(curPort, UART_SD_FINISH_RX_CHAR, lsv_FreeComFinishFlag);
                    /*清除接收使能标志*/
                    RST_UART_SM_FLAG(curPort, UART_SM_FREE_PORT_RX_EN);

                    stv_UartPortStatus[curPort].mcv_Status = UART_IDLE;
                    SET_UART_SM_FLAG(curPort, UART_SM_IDLE);
                }
                break;

            case UART_TYPE_MB_MASTER:
                if(GET_UART_SM_FLAG(curPort, UART_SM_MODBUS_FINISH))
                {
                    LOGE("COM2", "OMG, just break!!!");
                    break;
                }

                if(stv_UartPortStatus[curPort].msv_RxLength == 0)
                {
                    /*停止TX超时计时器*/
                    xTimerStop(ltv_Uart2TxTime, 100);

                    gTick = xTaskGetTickCount();
                    /*启动定时器*/
                    //LOGV("COM2", "xTimerStart: ltv_Uart1RxTime. Current ms = %u\r\n", gTick);
                    /*启动帧接收定时器*/
                    xTimerStart(ltv_Uart2RxTime, 100);
                }

                /*复位定时器*/
                xTimerReset(ltv_Uart2RxTime, 100);

                if (stv_UartPortStatus[curPort].aFlag == 0)
                {
                    stv_UartPortStatus[curPort].msp_FreeRxBuff[stv_UartPortStatus[curPort].msv_RxLength++] = lcv_RecvChar;
                }
                else
                {
                    stv_UartPortStatus[curPort].aBuf[stv_UartPortStatus[curPort].msv_RxLength++] = lcv_RecvChar;
                }

                if(stv_UartPortStatus[curPort].msv_RxLength >= stv_UartPortStatus[curPort].msv_FreeRxMaxCnt)
                {
                   // LOGV("COM2", "uart2 recv finished...001");
                    /*帧结束*/
                    xTimerStop(ltv_Uart2RxTime, 100);

                    modlink_copy(curPort);

                    SET_UART_SM_FLAG(curPort, UART_SM_RX_FINISH);
                    //SET_UART_SM_FLAG(curPort, UART_SM_MODBUS_FINISH);
                    bsp_uart_set_modbus_finished(2);
                    RST_UART_SM_FLAG(curPort, UART_SM_MODBUS_ERROR);

                    stv_UartPortStatus[curPort].mcv_Status = UART_IDLE;
                    SET_UART_SM_FLAG(curPort, UART_SM_IDLE);

                    //bsp_refresh_communication_led(0, UART1_A_LED_MASK);
                }
                break;
#endif
            }
        }
        //LOGW("COM2", "n = %u \r\n", n);
    }
    while(1); // while (kStatus_Success == error);

    //LPUART_RTOS_Deinit(&gLpuart0RTOSHandle);
    //vTaskSuspend(NULL);
}

void suspend_uart2_task(void)
{
    if (g_Uart2WorkerHandler)
    {
        vTaskSuspend(g_Uart2WorkerHandler);
    }
}
void resume_uart2_task(void)
{
    if (g_Uart2WorkerHandler)
    {
        vTaskResume(g_Uart2WorkerHandler);
    }
}

void bsp_uart2_init(unsigned long llv_BaudRate, unsigned char lcv_Parity, unsigned char lcv_WordLength, unsigned char lcv_StopBits)
{
    LOGV("bsp_uart", "Enter %s, llv_BaudRate=%d, lcv_Parity=%d, lcv_WordLength=%d, lcv_StopBits=%d.\r\n", __func__, llv_BaudRate, lcv_Parity, lcv_WordLength, lcv_StopBits);

    if (g_Uart2WorkerHandler != NULL)
    {
        LOGV("bsp_uart", "COM2 worker is running, so just return.\r\n");
        return;
    }

    lpuart_config2.baudrate = llv_BaudRate;
    switch(lcv_StopBits)
    {
    case UART_STB_1:
        lpuart_config2.stopbits = kLPUART_OneStopBit;
        break;
    case UART_STB_2:
        lpuart_config2.stopbits = kLPUART_TwoStopBit;
        break;
    }
    switch(lcv_Parity)
    {
    case UART_PARITY_NO:
        lpuart_config2.parity = kLPUART_ParityDisabled;
        break;

    case UART_PARITY_EVEN:
        lpuart_config2.parity = kLPUART_ParityEven;//偶
        break;

    case UART_PARITY_ODD:
        lpuart_config2.parity = kLPUART_ParityOdd;//奇
        break;
    }
    if (lcv_WordLength == 8)
    {
        lpuart_config2.dataBits = kLPUART_EightDataBits;
    }
    else
    {
        lpuart_config2.dataBits = kLPUART_SevenDataBits;
    }
    if (xTaskCreate(uart2_worker_task, "Uart_worker2",
                    UART_WORKER_TASK_STACK_SIZE, NULL,
                    UART_WORKER_TASK_PRIORITY, (TaskHandle_t *)&g_Uart2WorkerHandler) != pdPASS)
    {
        LOGE("bsp_uart", "Uart_worker2 task creation failed!\r\n");
        while (1)
            ;
    }
}

void bsp_com2_send_data(uint8_t *pData, uint16_t len)
{
    LOGD("bsp_uart", "Enter %s()", __func__);
    int ret = LPUART_RTOS_Send(&gLpuart2RTOSHandle, (uint8_t *)pData, len);
    LOGI("bsp_uart", "Leave %s(), LPUART_RTOS_Send return : %d", __func__, ret);
}

void bsp_uart2_send_buffer(unsigned char *lcp_SendBuff, unsigned short lsv_Length)
{
    LOGV("bsp_uart", "Enter %s(), lsv_Length = %d, mcv_Mode = %u\r\n", __func__, lsv_Length, gtp_UartPort[2].mcv_Mode);
    hexdump(lcp_SendBuff, lsv_Length);	
    bsp_uart_tx_enable(2);
    LPUART_RTOS_Send(&gLpuart2RTOSHandle, (uint8_t *)lcp_SendBuff, lsv_Length);
    //RST_UART_SM_FLAG(1, UART_SM_IDLE);
    if (gtp_UartPort[2].mcv_Mode == UART_TYPE_MB_MASTER)
    {
        xTimerStart(ltv_Uart2TxTime, 100);
    }

    bsp_send_finished(2);
}

void bsp_uart2_time_init(unsigned short lsv_RxPeriod, unsigned short lsv_TxPeriod)
{
    LOGV("bsp_uart", "Enter %s, lsv_RxPeriod = %d, lsv_TxPeriod = %d\r\n", __func__, lsv_RxPeriod, lsv_TxPeriod);
    if (lsv_RxPeriod == 0)
    {
        lsv_RxPeriod = 10;
    }
    if((ltv_Uart2RxTime == NULL) && (lsv_RxPeriod > 0))
    {
        ltv_Uart2RxTime = xTimerCreate((const char *)"Uart2 RxTime",
                                       (TickType_t  )lsv_RxPeriod / portTICK_PERIOD_MS,
                                       (UBaseType_t )pdFALSE,
                                       (void *      )UART2_RX_TIMER_ID,
                                       (TimerCallbackFunction_t)bsp_timer_callback_func);
    }

    if (lsv_TxPeriod == 0)
    {
        lsv_TxPeriod = 1000;
    }
    if((ltv_Uart2TxTime == NULL) && (lsv_TxPeriod > 0))
    {
        ltv_Uart2TxTime = xTimerCreate((const char *)"Uart2 TxTime",
                                       (TickType_t  )lsv_TxPeriod / portTICK_PERIOD_MS,
                                       (UBaseType_t )pdFALSE,
                                       (void *      )UART2_TX_TIMER_ID,
                                       (TimerCallbackFunction_t)bsp_timer_callback_func);
    }
    //LOGD("bsp_uart", "Leave %s(), ltv_Uart2RxTime = 0x%08X, ltv_Uart2TxTime = 0x%08X", __func__, ltv_Uart2RxTime, ltv_Uart2TxTime);
}

static void bsp_send_finished(uint8_t portNum)
{
    /*发送完成*/
    //LOGI("bsp_uart", "Enter %s(), port = %d, mode = %d", __func__, portNum, gtp_UartPort[portNum].mcv_Mode);
    switch(gtp_UartPort[portNum].mcv_Mode)
    {
    case UART_TYPE_MB_MASTER:
        /*设置标志位*/
        SET_UART_SM_FLAG(portNum, UART_SM_TX_FINISH);

        /*发送完成进入等待接收状态*/
        stv_UartPortStatus[portNum].mcv_Status = UART_RX;
        stv_UartPortStatus[portNum].msv_TxLength = 0;
        stv_UartPortStatus[portNum].msv_TxTotalLength = 0;

        /*默认使能RX*/
        bsp_uart_rx_enable(portNum);

        /*复位TX超时时间*/
        switch (portNum)
        {
        case 0:
            //xTimerReset(ltv_Uart0TxTime, 10);
            break;
        case 1:
            //xTimerReset(ltv_Uart1TxTime, 100);
            //xTimerStop(ltv_Uart1TxTime, 1);
            break;
        case 2:
            xTimerReset(ltv_Uart2TxTime, 100);
            break;
        case 3:
            break;
        case 4:
            break;
        default:
            LOGE("bsp_uart", "Bad port number : %u", portNum);
            break;
        }

        break;

    default:
        stv_UartPortStatus[portNum].mcv_Status = UART_IDLE;
        stv_UartPortStatus[portNum].msv_TxLength = 0;
        stv_UartPortStatus[portNum].msv_TxTotalLength = 0;
        SET_UART_SM_FLAG(portNum, UART_SM_IDLE);

        /*默认使能RX*/
        bsp_uart_rx_enable(portNum);

        /*设置标志位*/
        SET_UART_SM_FLAG(portNum, UART_SM_TX_FINISH);
        RST_UART_SM_FLAG(portNum, UART_SM_FREE_PORT_TX_EN);
    }

    //bsp_refresh_communication_led(0, UART0_B_LED_MASK);
}

void bsp_uart_init_dummy(unsigned long llv_BaudRate, unsigned char lcv_Parity, unsigned char lcv_WordLength, unsigned char lcv_StopBits)
{
    //LOGV("bsp_uart", "Enter %s()", __func__);
}
void bsp_uart_send_buffer_dummy(unsigned char *lcp_SendBuff, unsigned short lsv_Length)
{
    //LOGV("bsp_uart", "Enter %s()", __func__);
}
void bsp_uart_time_init_dummy(unsigned short lsv_RxPeriod, unsigned short lsv_TxPeriod)
{
    //LOGV("bsp_uart", "Enter %s()", __func__);
}

/**
  * @brief  获取串口函数指针
  * @param  None
  * @retval None
  */
void bsp_get_uart_configure_info(uart_port_info_st *ltp_UartInfo)
{
    /* 用户串口0     RS485*/
#if (LOG_OPEN == 1) && (LOG_USE_LPUART4 == 1)
    ltp_UartInfo[0].pConfigFunc = bsp_uart_init_dummy;
    ltp_UartInfo[0].pSendFunc = bsp_uart_send_buffer_dummy;
    ltp_UartInfo[0].pTimerInitFunc = bsp_uart_time_init_dummy;
#else
    ltp_UartInfo[0].pConfigFunc = bsp_uart0_init;
    ltp_UartInfo[0].pSendFunc = bsp_uart0_send_buffer;
    ltp_UartInfo[0].pTimerInitFunc = bsp_uart0_time_init;
#endif
    ltp_UartInfo[0].msv_SmFlagStartELement = 110;
    ltp_UartInfo[0].msv_SdFlagStartELement = 110;

    /* 用户串口1 */
#if (LOG_OPEN == 1) && (LOG_USE_LPUART3 == 1)
    ltp_UartInfo[1].pConfigFunc = bsp_uart_init_dummy;
    ltp_UartInfo[1].pSendFunc = bsp_uart_send_buffer_dummy;
    ltp_UartInfo[1].pTimerInitFunc = bsp_uart_time_init_dummy;
#else
    ltp_UartInfo[1].pConfigFunc = bsp_uart1_init;
    ltp_UartInfo[1].pSendFunc = bsp_uart1_send_buffer;
    ltp_UartInfo[1].pTimerInitFunc = bsp_uart1_time_init;
#endif
    ltp_UartInfo[1].msv_SmFlagStartELement = 120;
    ltp_UartInfo[1].msv_SdFlagStartELement = 120;

    /* 用户串口2       */
#if (LOG_OPEN == 1) && (LOG_USE_LPUART8 == 1)
    ltp_UartInfo[2].pConfigFunc = bsp_uart_init_dummy;
    ltp_UartInfo[2].pSendFunc = bsp_uart_send_buffer_dummy;
    ltp_UartInfo[2].pTimerInitFunc = bsp_uart_time_init_dummy;
#elif 1 //CC100没有COM2
    ltp_UartInfo[2].pConfigFunc = bsp_uart_init_dummy;
    ltp_UartInfo[2].pSendFunc = bsp_uart_send_buffer_dummy;
    ltp_UartInfo[2].pTimerInitFunc = bsp_uart_time_init_dummy;
#else
    ltp_UartInfo[2].pConfigFunc = bsp_uart2_init;
    ltp_UartInfo[2].pSendFunc = bsp_uart2_send_buffer;
    ltp_UartInfo[2].pTimerInitFunc = bsp_uart2_time_init;
#endif
    ltp_UartInfo[2].msv_SmFlagStartELement = 130;
    ltp_UartInfo[2].msv_SdFlagStartELement = 130;
}

void bsp_uart_modbus_send(uint16_t port, unsigned char *lcp_SendBuff, unsigned short lsv_Length)
{
    TickType_t delays;
    LOGV("bsp_uart", "Enter %s(), lsv_Length = %d", __func__, lsv_Length);
    bsp_uart_tx_enable(port);
    //hexdump(lcp_SendBuff, lsv_Length);

    LOGD("bsp_uart", "gtp_UartPort[%d].mcv_Mode = %u", port, gtp_UartPort[port].mcv_Mode);
    switch(port)
    {
        case 0:
            LPUART_RTOS_Send(&gLpuart0RTOSHandle, (uint8_t *)lcp_SendBuff, lsv_Length);
            if (gtp_UartPort[0].mcv_Mode == UART_TYPE_MB_MASTER)
            {
                xTimerStart(ltv_Uart0TxTime, 100);
            }
            break;
        case 1:
            //hexdump(lcp_SendBuff, lsv_Length);
            LPUART_RTOS_Send(&gLpuart1RTOSHandle, (uint8_t *)lcp_SendBuff, lsv_Length);
            LOGI("bsp_uart", "lpuart_config1.baudrate = %u", lpuart_config1.baudrate);

            delays = GET_SD_ELEMENT_VALUE(SD400);
            LOGD("bsp_uart", "delays = %u", delays);
            vTaskDelay(delays / portTICK_PERIOD_MS);
            if (gtp_UartPort[1].mcv_Mode == UART_TYPE_MB_MASTER)
            {
                xTimerStart(ltv_Uart1TxTime, 0);
            }
            break;
        case 2:
            LPUART_RTOS_Send(&gLpuart2RTOSHandle, (uint8_t *)lcp_SendBuff, lsv_Length);
            if (gtp_UartPort[2].mcv_Mode == UART_TYPE_MB_MASTER)
            {
                xTimerStart(ltv_Uart2TxTime, 100);
            }
            break;
    }
    
    bsp_send_finished(port);
}

/**
  * @brief  获取串口状态
  * @param  None
  * @retval None
  */
unsigned char bsp_get_uart_port_status(unsigned short lsv_PortNum)
{
    return stv_UartPortStatus[lsv_PortNum].mcv_Status;
}

/**
  * @brief  设置自由口、modbus master模式接收参数
  * @param  flag  0,以modbus模式接收数据
  * @retval None
  */
void bsp_set_free_port_receive_para(unsigned short lsv_PortNum, unsigned short *lsp_RxBuff, unsigned short lsv_RxCnt, uint8_t flag)
{
    LOGD("bsp_uart", "Enter %s(), lsv_PortNum = %u, lsp_RxBuff = 0x%08X, lsv_RxCnt = %u, flag = %u", __func__, lsv_PortNum, lsp_RxBuff, lsv_RxCnt, flag);
    stv_UartPortStatus[lsv_PortNum].msp_FreeRxBuff = lsp_RxBuff;
    stv_UartPortStatus[lsv_PortNum].msv_FreeRxMaxCnt = lsv_RxCnt;
    stv_UartPortStatus[lsv_PortNum].msv_RxLength = 0;

    if (stv_UartPortStatus[lsv_PortNum].aBuf)
    {
        vPortFree(stv_UartPortStatus[lsv_PortNum].aBuf);
    }
    stv_UartPortStatus[lsv_PortNum].aBuf = pvPortMalloc(1024);
    stv_UartPortStatus[lsv_PortNum].aFlag = flag;
}

