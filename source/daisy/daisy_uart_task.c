/**
  ******************************************************************************
  * @file    daisy_uart_task.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   背板总线串口任务
  ******************************************************************************
  */

#include "daisy_uart_task.h"
#include "daisy_task.h"
#include "fsl_lpuart_edma.h"
#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
#include "fsl_dmamux.h"
#endif
#include "fsl_cache.h"

#include "fsl_lpuart_freertos.h"
#include "kalyke_event.h"
#include "plc_sysblock.h"
#include "plc_errormsg.h"
#include "bsp_led.h"
#include "bsp_uart.h"
#include "bsp_gpio.h"
#include "verify_func.h"
#include "bsp_iwdg.h"
#include "plc_internalmanage.h"


#define LPUART1_TX_DMA_CHANNEL          0U
#define LPUART3_RX_DMA_CHANNEL          2U

#define HANDLE_DATA_LOGIC    1

//#define DEBUG_UART_TASK
#ifdef DEBUG_UART_TASK
#define LOGE_U    LOGE
#define LOGW_U    LOGW
#define LOGI_U    LOGI
#define LOGD_U    LOGD
#define LOGV_U    LOGV
#else
#define LOGE_U(...)
#define LOGW_U(...)
#define LOGI_U(...)
#define LOGD_U(...)
#define LOGV_U(...)
#endif

#if (LOG_OPEN == 1)
/*
 * 0 = close
 * 1 = 以微秒为单位计算响应时间
 * 2 = 以毫秒为单位计算响应时间
 * 3 = 同时以毫秒、微秒计算响应时间
*/
#define DAISY_UART_COMUNICATION_COUNT    3
#endif


/*uart任务消息队列结构体*/
typedef struct _DAISY_UART_MSG_ST
{
    /*设备UART端口号*/
    uint16_t port;
    /*数据长度*/
    uint16_t length;
    /*数据缓存区指针*/
    uint8_t *data;
} daisy_uart_msg_st;

static const char *TAG = "DaisyUart";

static uint32_t gDaisyUartTick0;
static uint32_t gDaisyUartTickMS;

static uint8_t gDaisyUartSendBuffer[128];

static uint8_t gLoopBuf[1024];

TaskHandle_t gDaisyUartLoopTaskHandler = NULL;

static uint8_t background_buffer1[64];
static lpuart_rtos_config_t lpuart_config1 =
{
    .baudrate = 921600,
    .parity = kLPUART_ParityDisabled,
    .stopbits = kLPUART_OneStopBit,
    .buffer = background_buffer1,
    .buffer_size = sizeof(background_buffer1),
    //.enableRxRTS = false,
    //.enableTxCTS = false,
};
static TaskHandle_t g_Uart1WorkerHandler = NULL;
static lpuart_rtos_handle_t gLpuart1RTOSHandle;
static struct _lpuart_handle t_handle1;

static uint8_t background_buffer3[4096];
static uint8_t recv_buffer3[1024];
static uint8_t gUart3RecvBufferCPY[1024];
static uint8_t gUart3RecvBuffer[1024] = {0};
//AT_NONCACHEABLE_SECTION_INIT(uint8_t gUart3RecvBuffer[1024]) = {0};

volatile TickType_t gUart3BeginTick = 0;
static lpuart_rtos_config_t lpuart_config3 =
{
    .baudrate = 921600,
    .parity = kLPUART_ParityDisabled,
    .stopbits = kLPUART_OneStopBit,
    .buffer = background_buffer3,
    .buffer_size = sizeof(background_buffer3),
    //.enableRxRTS = false,
    //.enableTxCTS = false,
};
static TaskHandle_t g_Uart3WorkerHandler = NULL;
static lpuart_rtos_handle_t gLpuart3RTOSHandle;
static struct _lpuart_handle t_handle3;
static volatile uint16_t gUart3RecvLength = 0;
volatile uint8_t gDaisyUart3Status = UART_IDLE;

static TimerHandle_t gUart3RxTimer =  NULL;

#if 0
#if (HANDLE_DATA_LOGIC == 1)
static void daisyHandleRS485ReceiveData(void);
#endif
#endif

static void daisy_uart_handle_RS485_received_data(void);

static inline void before_cal_uart_resp_time(void)
{
#if (DAISY_UART_COMUNICATION_COUNT == 2)
    gDaisyUartTick0 = xTaskGetTickCount();
#elif (DAISY_UART_COMUNICATION_COUNT == 1)
    gDaisyUartTick0 = SysTick->VAL;
#elif (DAISY_UART_COMUNICATION_COUNT == 3)
    gDaisyUartTick0 = SysTick->VAL;
    gDaisyUartTickMS = xTaskGetTickCount();
#endif
}

static inline void calculate_uart_resp_time(void)
{
    static uint32_t lastTick = 0;
#if (DAISY_UART_COMUNICATION_COUNT == 2)
    uint32_t tick1 = xTaskGetTickCount();
    if (tick1 - lastTick > 2000)
    {
        uint32_t interval = tick1 - gDaisyUartTick0;
        LOGV(TAG, "Enter %s(), response interval = %u(ms)", __func__, interval);
        hexdump(gLoopBuf, gUart3RecvLength);
        lastTick = tick1;
    }
#elif (DAISY_UART_COMUNICATION_COUNT == 1)
    uint32_t tickInterval;
    uint32_t tick1 = SysTick->VAL;
    uint32_t tickMS = xTaskGetTickCount();
    if (gDaisyUartTick0 >= tick1)
    {
        tickInterval = gDaisyUartTick0 - tick1;
    }
    else
    {
        tickInterval = 600000 - tick1 + gDaisyUartTick0;
    }
    double us = tickInterval / 600.0;
    if(gDaisyUartFSM != DAISY_FSM_LOOP)
    {
        LOGI(TAG, "Enter %s(), response interval = %.1f(us), cpu ticks: %u", __func__, us, tickInterval);
    }
    else
    {
        //if (gtv_PlcElement.msp_SDElement[SD234] == 1999)
        if (tickMS - lastTick > 2000)
        {
            LOGI(TAG, "Enter %s(), response interval = %.1f(us), cpu ticks: %u", __func__, us, tickInterval);
            lastTick = tickMS;
        }
    }
#elif (DAISY_UART_COMUNICATION_COUNT == 3)
    uint32_t tick1 = SysTick->VAL;
    uint32_t tickMS = xTaskGetTickCount();
    uint32_t intervalMS = tickMS - gDaisyUartTickMS;
    uint32_t tickInterval;
    if (gDaisyUartTick0 >= tick1)
    {
        tickInterval = gDaisyUartTick0 - tick1;
    }
    else
    {
        tickInterval = 600000 - tick1 + gDaisyUartTick0;
    }
    double us = tickInterval / 600.0;
    if(gMasterRunInfo.daisy_uart_status != MASTER_LOOP_EXT)
    {
        LOGI(TAG, "Enter %s(), response interval = %.1f(us) | %u(ms), cpu ticks: %u", __func__, us, intervalMS, tickInterval);
    }
    else
    {
        //if (gtv_PlcElement.msp_SDElement[SD234] == 1999)
        if (tickMS - lastTick > 2000)
        {
            LOGI(TAG, "Enter %s(), response interval = %.1f(us) | %u(ms), cpu ticks: %u", __func__, us, intervalMS, tickInterval);
            lastTick = tickMS;

            if (guv_NonStopError.msv_Error != 0)
            {
                LOGE(TAG, "guv_NonStopError.msv_Error = 0x%04X", guv_NonStopError.msv_Error);
            }
        }
    }
#endif
}

static void daisy_uart3_rx_enable(void)
{
    GPIO_PortClear(MCU_UART3_DE_GPIO, MCU_UART3_DE_MASK);
}

static void daisy_uart3_tx_enable(void)
{
    //GPIO_PortSet(MCU_UART3_DE_GPIO, MCU_UART3_DE_MASK);
}

static inline void start_uart3_recv_timeout_timer(void)
{
    BaseType_t ret = xTimerStart(gUart3RxTimer, 0);
    if (ret == pdFAIL)
    {
        LOGE(TAG, "Start gUart3RxTimer ERROR!!!!");
    }
}

static inline void reset_uart3_recv_timeout_timer(void)
{
    BaseType_t ret = xTimerReset(gUart3RxTimer, 0);
    if (ret == pdFAIL)
    {
        LOGE(TAG, "Reset gUart3RxTimer ERROR!!!!");
    }
}

#if (DAISY_UART3_WORKER_LOGIC == 1)
static void daisy_uart3_worker_task(void *pvParameters)
{
    LOGI(TAG, "daisy_uart3_worker_task RUN. Free heap size is %d bytes.", xPortGetFreeHeapSize());
    unsigned char lcv_RecvChar;
    int error;
    size_t n;

    taskENTER_CRITICAL();
    uint32_t clock = BOARD_DebugConsoleSrcFreq();
    LOGI(TAG, "UART3 clock = %u", clock);
    lpuart_config3.srcclk = clock;
    lpuart_config3.base = LPUART3;

    if (0 > LPUART_RTOS_Init(&gLpuart3RTOSHandle, &t_handle3, &lpuart_config3))
    {
        taskEXIT_CRITICAL();
        while (1)
        {
            LOGE(TAG, "LPUART_RTOS_Init ERROR!! (LPUART3)");
            vTaskDelay(1000);
        }
    }
    LOGI(TAG, "lpuart_config3.buffer_size = %u", lpuart_config3.buffer_size);
    taskEXIT_CRITICAL();

    daisy_uart3_rx_enable();
    /* Receive just one byte. */
    do
    {
        error = LPUART_RTOS_Receive(&gLpuart3RTOSHandle, recv_buffer3, 1, &n);
        if (error != kStatus_Success)
        {
            LOGE(TAG, "error = %d", error);
            if (error == kStatus_LPUART_RxHardwareOverrun)
            {
                /* Notify about hardware buffer overrun */
                LOGE(TAG, "Hardware buffer overrun! -- n = %u", n);
            }
            if (error == kStatus_LPUART_RxRingBufferOverrun)
            {
                /* Notify about ring buffer overrun */
                LOGE(TAG, "Ring buffer overrun! -- n = %u", n);
            }
            gUart3BeginTick = xTickCount;
        }
        if (n > 0)
        {
            if (gUart3RecvLength >= 1024)
            {
                continue;
            }
            lcv_RecvChar = (unsigned char)recv_buffer3[0];
            if (gDaisyUart3Status == UART_IDLE)
            {
                gDaisyUart3Status = UART_RX;
                gUart3RecvBuffer[gUart3RecvLength++] = lcv_RecvChar;
                gUart3BeginTick = xTickCount;
                continue;
            }

            gUart3RecvBuffer[gUart3RecvLength++] = lcv_RecvChar;
            gUart3BeginTick = xTickCount;
        }
    }
    while(1);
}
#else
static void daisy_uart3_worker_task(void *pvParameters)
{
    LOGW(TAG, "daisy_uart3_worker_task RUN. Free heap size is %d bytes.", xPortGetFreeHeapSize());
    unsigned char lcv_RecvChar;
    int error;
    size_t n;

    taskENTER_CRITICAL();
    uint32_t clock = BOARD_DebugConsoleSrcFreq();
    LOGI(TAG, "UART3 clock = %u", clock);
    lpuart_config3.srcclk = clock;
    lpuart_config3.base = LPUART3;

    if (0 > LPUART_RTOS_Init(&gLpuart3RTOSHandle, &t_handle3, &lpuart_config3))
    {
        taskEXIT_CRITICAL();
        while (1)
        {
            LOGE(TAG, "LPUART_RTOS_Init ERROR!! (LPUART3)");
            vTaskDelay(1000);
        }
    }
    LOGI(TAG, "lpuart_config3.buffer_size = %u", lpuart_config3.buffer_size);
    taskEXIT_CRITICAL();

    daisy_uart3_rx_enable();
    /* Receive just one byte. */
    do
    {
        error = LPUART_RTOS_Receive(&gLpuart3RTOSHandle, recv_buffer3, 1, &n);
        if (error == kStatus_LPUART_RxHardwareOverrun)
        {
            /* Notify about hardware buffer overrun */
            LOGE(TAG, "Hardware buffer overrun! -- n = %u", n);
        }
        if (error == kStatus_LPUART_RxRingBufferOverrun)
        {
            /* Notify about ring buffer overrun */
            LOGE(TAG, "Ring buffer overrun! -- n = %u", n);
        }
        if (n > 0)
        {
            if (gUart3RecvLength >= 1024)
            {
                continue;
            }
            lcv_RecvChar = (unsigned char)recv_buffer3[0];
            if (gDaisyUart3Status == UART_IDLE)
            {
                gDaisyUart3Status = UART_RX;
                start_uart3_recv_timeout_timer();
                gUart3RecvBuffer[gUart3RecvLength++] = lcv_RecvChar;
                continue;
            }
            reset_uart3_recv_timeout_timer();
            gUart3RecvBuffer[gUart3RecvLength++] = lcv_RecvChar;
        }
    }
    while(1);
}
#endif

#if (DAISY_UART3_RECV_LOGIC == 1)
lpuart_transfer_t gUart3receiveXfer; //定义接收传输结构体

static edma_handle_t g_lpuartRxEdmaHandle;
static lpuart_edma_handle_t g_lpuart3EdmaHandle;

/* 接收字节数计数 */
uint32_t g_bufflength = 0;
void LPUART3_IRQHandler(void)
{
    /* 判断中断源 */
    if (LPUART_GetStatusFlags(LPUART3) & kLPUART_IdleLineFlag)
    {
        /* 清除空闲中断标志位 */
        LPUART_ClearStatusFlags(LPUART3, kLPUART_IdleLineFlag);
        // LPUART_ClearStatusFlags(LPUART3, kLPUART_IdleLineInterruptEnable);
        /* 刷新D-Cache，确保g_rxBuffer数据一致性 */
        //DCACHE_CleanInvalidateByRange((uint32_t)gUart3RecvBuffer, sizeof(gUart3RecvBuffer));

        /* 获取当前已接收字节数 */
        LPUART_TransferGetReceiveCountEDMA(LPUART3, &g_lpuart3EdmaHandle, &g_bufflength);
        //LOGE(TAG, "g_bufflength = %u", g_bufflength);

        //LPUART3->STAT |= LPUART_STAT_RDRF_MASK;
        LPUART3->STAT |= LPUART_STAT_TC_MASK;
        //LPUART3->STAT |= LPUART_STAT_TDRE_MASK;

        gUart3RecvLength = g_bufflength;

        /* 关闭DMA传输 */
        LPUART_TransferAbortReceiveEDMA(LPUART3, &g_lpuart3EdmaHandle);

        /*
        *数据处理
        */
#if (LOG_OPEN == 1)
        calculate_uart_resp_time();
#endif
        //LOGE(TAG, "gUart3RecvLength = %u", gUart3RecvLength);
        //memcpy(gUart3RecvBufferCPY, gUart3receiveXfer.data, gUart3RecvLength);
#if 0
        hexdump(gUart3receiveXfer.data, 16);
#endif
        daisy_uart_handle_RS485_received_data();

        /* 刷新D-Cache，确保g_txBuffer数据一致性 */
        //DCACHE_CleanInvalidateByRange((uint32_t)gUart3RecvBuffer, sizeof(gUart3RecvBuffer));

        /* 重新开始DMA接收传输 */
        //gUart3receiveXfer.data = gUart3RecvBuffer;
        //gUart3receiveXfer.dataSize = sizeof(gUart3RecvBuffer);
        //g_bufflength = 0;
        //memset(gUart3RecvBuffer, 0, sizeof(gUart3RecvBuffer));
        status_t ret = LPUART_ReceiveEDMA(LPUART3, &g_lpuart3EdmaHandle, &gUart3receiveXfer);
        if (ret != kStatus_Success)
        {
            LOGE(TAG, "LPUART_ReceiveEDMA return %d", ret);
        }
    }
    SDK_ISR_EXIT_BARRIER;
}

/* 对扩展模块总线用到的串口初始化
 * 该串口是458，仅用于接收
*/
static void daisy_LPUART3_init(uint32_t llv_BaudRate, uint8_t lcv_Parity, uint8_t lcv_WordLength, uint8_t lcv_StopBits)
{
    LOGV(TAG, "Enter %s, llv_BaudRate=%d, lcv_Parity=%d, lcv_WordLength=%d, lcv_StopBits=%d", __func__, llv_BaudRate, lcv_Parity, lcv_WordLength, lcv_StopBits);
    daisy_uart3_rx_enable();
    NVIC_SetPriority(LPUART3_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);

    lpuart_config_t lpuartConfig;
    //edma_config_t config;
    //lpuart_transfer_t xfer;
    //lpuart_transfer_t sendXfer;
    //lpuart_transfer_t receiveXfer;

    /* Initialize the LPUART. */
    /*
     * lpuartConfig.baudRate_Bps = 115200U;
     * lpuartConfig.parityMode = kLPUART_ParityDisabled;
     * lpuartConfig.stopBitCount = kLPUART_OneStopBit;
     * lpuartConfig.txFifoWatermark = 0;
     * lpuartConfig.rxFifoWatermark = 0;
     * lpuartConfig.enableTx = false;
     * lpuartConfig.enableRx = false;
     */
    LPUART_GetDefaultConfig(&lpuartConfig);
    lpuartConfig.baudRate_Bps = llv_BaudRate;
    switch(lcv_Parity)
    {
    case 0:
        lpuartConfig.parityMode = kLPUART_ParityDisabled;
        break;

    case 1:
        lpuartConfig.parityMode = kLPUART_ParityOdd;//奇
        break;

    case 2:
        lpuartConfig.parityMode = kLPUART_ParityEven;//偶
        break;
    }
    if (lcv_WordLength == 8)
    {
        lpuartConfig.dataBitsCount = kLPUART_EightDataBits;
    }
    else
    {
        lpuartConfig.dataBitsCount = kLPUART_SevenDataBits;
    }
    switch(lcv_StopBits)
    {
    case UART_STB_1:
        lpuartConfig.stopBitCount = kLPUART_OneStopBit;
        break;
    case UART_STB_2:
        lpuartConfig.stopBitCount = kLPUART_TwoStopBit;
        break;
    }
    //lpuartConfig.enableTx     = true;
    lpuartConfig.enableRx     = true;
    lpuartConfig.rxIdleConfig = kLPUART_IdleCharacter2;
    lpuartConfig.rxIdleType = kLPUART_IdleTypeStopBit;

    uint32_t uart3_clock = BOARD_DebugConsoleSrcFreq();
    LOGD(TAG, "uart3_clock = %u", uart3_clock);
    LPUART_Init(LPUART3, &lpuartConfig, uart3_clock);
    /* 清除可能残留的标志位，并使能空闲中断 */
    LPUART_ClearStatusFlags(LPUART3, kLPUART_IdleLineFlag);
    LPUART_EnableInterrupts(LPUART3, kLPUART_IdleLineInterruptEnable);
    /* 打开串口中断 */
    EnableIRQ(LPUART3_IRQn);

#if 0 // in daisy_LPUART_DMA_init()
#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
    /* Init DMAMUX */
    DMAMUX_Init(DMAMUX);
    /* Set channel for LPUART */
    //DMAMUX_SetSource(DMAMUX, LPUART_TX_DMA_CHANNEL, kDmaRequestMuxLPUART1Tx);
    DMAMUX_SetSource(DMAMUX, LPUART3_RX_DMA_CHANNEL, kDmaRequestMuxLPUART3Rx);
    //DMAMUX_EnableChannel(DMAMUX, LPUART_TX_DMA_CHANNEL);
    DMAMUX_EnableChannel(DMAMUX, LPUART3_RX_DMA_CHANNEL);
#endif
    /* Init the EDMA module */
    EDMA_GetDefaultConfig(&config);
    EDMA_Init(DMA0, &config);

    //EDMA_CreateHandle(&g_lpuartTxEdmaHandle, DMA0, LPUART_TX_DMA_CHANNEL);
    EDMA_CreateHandle(&g_lpuartRxEdmaHandle, DMA0, LPUART3_RX_DMA_CHANNEL);
    /* Create LPUART DMA handle. */
    LPUART_TransferCreateHandleEDMA(LPUART3, &g_lpuart3EdmaHandle, NULL, NULL, NULL, &g_lpuartRxEdmaHandle);

    memset(gUart3RecvBuffer, 0, sizeof(gUart3RecvBuffer));
    gUart3receiveXfer.data = gUart3RecvBuffer;
    gUart3receiveXfer.dataSize = sizeof(gUart3RecvBuffer);
    LPUART_ReceiveEDMA(LPUART3, &g_lpuart3EdmaHandle, &gUart3receiveXfer);
#endif

    LOGV(TAG, "Leave %s()", __func__);
}
#else
/* 对扩展模块总线用到的串口初始化
 * 该串口是458，仅用于接收
*/
static void daisy_LPUART3_init(uint32_t llv_BaudRate, uint8_t lcv_Parity, uint8_t lcv_WordLength, uint8_t lcv_StopBits)
{
    LOGV(TAG, "Enter %s, llv_BaudRate=%d, lcv_Parity=%d, lcv_WordLength=%d, lcv_StopBits=%d", __func__, llv_BaudRate, lcv_Parity, lcv_WordLength, lcv_StopBits);
    if (g_Uart3WorkerHandler != NULL)
    {
        LOGW(TAG, "UART worker 0 is running, so just return");
        return;
    }
    NVIC_SetPriority(LPUART3_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    lpuart_config3.baudrate = llv_BaudRate;
    switch(lcv_StopBits)
    {
    case UART_STB_1:
        lpuart_config3.stopbits = kLPUART_OneStopBit;
        break;
    case UART_STB_2:
        lpuart_config3.stopbits = kLPUART_TwoStopBit;
        break;
    }
    switch(lcv_Parity)
    {
    case 0:
        lpuart_config3.parity = kLPUART_ParityDisabled;
        break;

    case 1:
        lpuart_config3.parity = kLPUART_ParityOdd;//奇
        break;

    case 2:
        lpuart_config3.parity = kLPUART_ParityEven;//偶
        break;
    }
    if (lcv_WordLength == 8)
    {
        lpuart_config3.dataBits = kLPUART_EightDataBits;
    }
    else
    {
        lpuart_config3.dataBits = kLPUART_SevenDataBits;
    }
    if (xTaskCreate(daisy_uart3_worker_task, "Uart3Worker",
                    DAISY_UART3_TASK_STACK_SIZE, NULL,
                    DAISY_UART3_TASK_PRIO, (TaskHandle_t *)&g_Uart3WorkerHandler) != pdPASS)
    {
        while (1)
        {
            LOGE(TAG, "Uart3Worker task creation failed!");
            vTaskDelay(1000);
        }
    }
}
#endif /* DAISY_UART3_RECV_LOGIC */

#if (DAISY_UART1_SEND_LOGIC == 1)
static edma_handle_t g_lpuartTxEdmaHandle;
static lpuart_edma_handle_t g_lpuart1EdmaHandle;
static volatile bool txOnGoing = false;

static void LPUART1_UserCallback(LPUART_Type *base, lpuart_edma_handle_t *handle, status_t status, void *userData)
{
    userData = userData;
    LOGI(TAG, "Enter %s(), status = %d", __func__, status);
    if (kStatus_LPUART_TxIdle == status)
    {
        //txBufferFull = false;
        //txOnGoing    = false;
    }

    if (kStatus_LPUART_RxIdle == status)
    {
        //rxBufferEmpty = false;
        //rxOnGoing     = false;
    }
}

/* 对扩展模块总线用到的串口初始化
 * 该串口仅用于发送
*/
static void daisy_LPUART1_init(uint32_t llv_BaudRate, uint8_t lcv_Parity, uint8_t lcv_WordLength, uint8_t lcv_StopBits)
{
    LOGV(TAG, "Enter %s, llv_BaudRate=%d, lcv_Parity=%d, lcv_WordLength=%d, lcv_StopBits=%d", __func__, llv_BaudRate, lcv_Parity, lcv_WordLength, lcv_StopBits);

    NVIC_SetPriority(LPUART1_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);

    lpuart_config_t lpuartConfig;
    //edma_config_t config;
    //lpuart_transfer_t xfer;
    //lpuart_transfer_t sendXfer;
    //lpuart_transfer_t receiveXfer;

    /* Initialize the LPUART. */
    /*
     * lpuartConfig.baudRate_Bps = 115200U;
     * lpuartConfig.parityMode = kLPUART_ParityDisabled;
     * lpuartConfig.stopBitCount = kLPUART_OneStopBit;
     * lpuartConfig.txFifoWatermark = 0;
     * lpuartConfig.rxFifoWatermark = 0;
     * lpuartConfig.enableTx = false;
     * lpuartConfig.enableRx = false;
     */
    LPUART_GetDefaultConfig(&lpuartConfig);
    lpuartConfig.baudRate_Bps = llv_BaudRate;
    switch(lcv_Parity)
    {
    case 0:
        lpuartConfig.parityMode = kLPUART_ParityDisabled;
        break;

    case 1:
        lpuartConfig.parityMode = kLPUART_ParityOdd;//奇
        break;

    case 2:
        lpuartConfig.parityMode = kLPUART_ParityEven;//偶
        break;
    }
    if (lcv_WordLength == 8)
    {
        lpuartConfig.dataBitsCount = kLPUART_EightDataBits;
    }
    else
    {
        lpuartConfig.dataBitsCount = kLPUART_SevenDataBits;
    }
    switch(lcv_StopBits)
    {
    case UART_STB_1:
        lpuartConfig.stopBitCount = kLPUART_OneStopBit;
        break;
    case UART_STB_2:
        lpuartConfig.stopBitCount = kLPUART_TwoStopBit;
        break;
    }
    lpuartConfig.enableTx     = true;
    //lpuartConfig.enableRx     = true;

    uint32_t uart1_clock = BOARD_DebugConsoleSrcFreq();
    LOGD(TAG, "uart1_clock = %u", uart1_clock);
    LPUART_Init(LPUART1, &lpuartConfig, uart1_clock);

#if 0 // in daisy_LPUART_DMA_init()
#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
    /* Init DMAMUX */
    DMAMUX_Init(DMAMUX);
    /* Set channel for LPUART */
    DMAMUX_SetSource(DMAMUX, LPUART1_TX_DMA_CHANNEL, kDmaRequestMuxLPUART1Tx);
    //DMAMUX_SetSource(DMAMUX, LPUART_RX_DMA_CHANNEL, kDmaRequestMuxLPUART1Rx);
    DMAMUX_EnableChannel(DMAMUX, LPUART1_TX_DMA_CHANNEL);
    //DMAMUX_EnableChannel(DMAMUX, LPUART_RX_DMA_CHANNEL);
#endif
    /* Init the EDMA module */
    EDMA_GetDefaultConfig(&config);
    EDMA_Init(DMA0, &config);
    EDMA_CreateHandle(&g_lpuartTxEdmaHandle, DMA0, LPUART1_TX_DMA_CHANNEL);
    //EDMA_CreateHandle(&g_lpuartRxEdmaHandle, DMA0, LPUART_RX_DMA_CHANNEL);

    /* Create LPUART DMA handle. */
    LPUART_TransferCreateHandleEDMA(LPUART1, &g_lpuart1EdmaHandle, NULL, NULL, &g_lpuartTxEdmaHandle, NULL);
#endif

    LOGV(TAG, "Leave %s()", __func__);
}
#else
static void daisy_uart1_worker_task(void *pvParameters)
{
    LOGW(TAG, "daisy_uart1_worker_task RUN. Free heap size is %d bytes.", xPortGetFreeHeapSize());

    taskENTER_CRITICAL();
    uint32_t clock = BOARD_DebugConsoleSrcFreq();

    lpuart_config1.srcclk = clock;
    lpuart_config1.base = LPUART1;

    if (0 > LPUART_RTOS_Init(&gLpuart1RTOSHandle, &t_handle1, &lpuart_config1))
    {
        taskEXIT_CRITICAL();
        while (1)
        {
            LOGE(TAG, "LPUART_RTOS_Init ERROR!! (LPUART1)");
            vTaskDelay(1001);
        }
    }
    LOGI(TAG, "lpuart_config1.buffer_size = %u", lpuart_config1.buffer_size);
    taskEXIT_CRITICAL();

    // 因为LPUART1只用于发送，故取消其接收功能
    LPUART_EnableRx(gLpuart1RTOSHandle.base, false);
    vTaskDelay(2021);
    vTaskDelete(NULL);
}

/* 对扩展模块总线用到的串口初始化
 * 该串口仅用于发送
*/
static void daisy_LPUART1_init(uint32_t llv_BaudRate, uint8_t lcv_Parity, uint8_t lcv_WordLength, uint8_t lcv_StopBits)
{
    LOGV(TAG, "Enter %s, llv_BaudRate=%d, lcv_Parity=%d, lcv_WordLength=%d, lcv_StopBits=%d", __func__, llv_BaudRate, lcv_Parity, lcv_WordLength, lcv_StopBits);
    if (g_Uart1WorkerHandler != NULL)
    {
        LOGW(TAG, "g_Uart1WorkerHandler is running, so just return");
        return;
    }
    NVIC_SetPriority(LPUART1_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);
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
    case 0:
        lpuart_config1.parity = kLPUART_ParityDisabled;
        break;

    case 1:
        lpuart_config1.parity = kLPUART_ParityOdd;//奇
        break;

    case 2:
        lpuart_config1.parity = kLPUART_ParityEven;//偶
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
    if (xTaskCreate(daisy_uart1_worker_task, "Uart1Worker",
                    DAISY_UART3_TASK_STACK_SIZE, NULL,
                    DAISY_UART3_TASK_PRIO, (TaskHandle_t *)&g_Uart1WorkerHandler) != pdPASS)
    {
        for (;;)
        {
            LOGE(TAG, "Uart1Worker task creation failed!");
            vTaskDelay(1000);
        }
    }
}
#endif /* DAISY_UART1_SEND_LOGIC */

static void daisy_uart1_send_crcdata(uint8_t *pData, uint16_t len)
{
    LOGV_U(TAG, "Enter %s(), len = %u", __func__, len);
    gUart3RecvLength = 0;

    uint16_t crc16 = calc_crc16(pData, len);
    pData[len] = (uint8_t)crc16; // 低位在前
    pData[len + 1] = (uint8_t)(crc16 >> 8);

    lpuart_transfer_t xfer;
    /* Send g_tipString out. */
    xfer.data     = pData;
    xfer.dataSize = len + 2;
#if (LOG_OPEN == 1)
    if (gMasterRunInfo.daisy_uart_status != MASTER_LOOP_EXT)
    {
        hexdump(xfer.data, xfer.dataSize);
    }
#endif
    status_t ret = LPUART_SendEDMA(LPUART1, &g_lpuart1EdmaHandle, &xfer);
    if (ret != kStatus_Success)
    {
        LOGE(TAG, "LPUART_SendEDMA fail(ret = %d)", ret);
    }
}


#if (DAISY_UART3_RECV_LOGIC == 1)
static void notify_daisy_loop_task(void)
{
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(gDaisyUartLoopTaskHandler, &higherPriorityTaskWoken);
    if (higherPriorityTaskWoken == pdTRUE)
    {
        portYIELD_FROM_ISR (higherPriorityTaskWoken);
    }
}
#else
static void notify_daisy_loop_task(void)
{
#if 0
    xTaskNotifyGive(gDaisyUartLoopTaskHandler);
#else
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(gDaisyUartLoopTaskHandler, &higherPriorityTaskWoken);
    if (higherPriorityTaskWoken == pdTRUE)
    {
        portYIELD_FROM_ISR (higherPriorityTaskWoken);
    }
#endif
}
#endif

#if (DAISY_UART1_SEND_LOGIC == 1) || (DAISY_UART3_RECV_LOGIC == 1)
static void daisy_LPUART_DMA_init(void)
{
    edma_config_t config;

#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
    /* Init DMAMUX */
    DMAMUX_Init(DMAMUX);
    /* Set channel for LPUART */

#if (DAISY_UART1_SEND_LOGIC == 1)
    DMAMUX_SetSource(DMAMUX, LPUART1_TX_DMA_CHANNEL, kDmaRequestMuxLPUART1Tx);
    DMAMUX_EnableChannel(DMAMUX, LPUART1_TX_DMA_CHANNEL);
#endif
#if (DAISY_UART3_RECV_LOGIC == 1)
    DMAMUX_SetSource(DMAMUX, LPUART3_RX_DMA_CHANNEL, kDmaRequestMuxLPUART3Rx);
    DMAMUX_EnableChannel(DMAMUX, LPUART3_RX_DMA_CHANNEL);
#endif
#endif /* defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT */

    /* Init the EDMA module */
    EDMA_GetDefaultConfig(&config);
    EDMA_Init(DMA0, &config);

#if (DAISY_UART1_SEND_LOGIC == 1)
    EDMA_CreateHandle(&g_lpuartTxEdmaHandle, DMA0, LPUART1_TX_DMA_CHANNEL);
    /* Create LPUART DMA handle. */
    LPUART_TransferCreateHandleEDMA(LPUART1, &g_lpuart1EdmaHandle, NULL, NULL, &g_lpuartTxEdmaHandle, NULL);
#endif
#if (DAISY_UART3_RECV_LOGIC == 1)
    EDMA_CreateHandle(&g_lpuartRxEdmaHandle, DMA0, LPUART3_RX_DMA_CHANNEL);
    /* Create LPUART DMA handle. */
    LPUART_TransferCreateHandleEDMA(LPUART3, &g_lpuart3EdmaHandle, NULL, NULL, NULL, &g_lpuartRxEdmaHandle);

    memset(gUart3RecvBuffer, 0, sizeof(gUart3RecvBuffer));
    gUart3receiveXfer.data = gUart3RecvBuffer;
    gUart3receiveXfer.dataSize = sizeof(gUart3RecvBuffer);
    LPUART_ReceiveEDMA(LPUART3, &g_lpuart3EdmaHandle, &gUart3receiveXfer);
#endif
}
#endif

// If check OK, return true.
static inline bool crc16_check(uint8_t *pBuf, uint16_t len)
{
    uint16_t crc16Calc = calc_crc16(pBuf, len - 2); //计算得到的CRC16的值
    uint16_t crc16Recv = (*(pBuf + len - 1) << 8) | *(pBuf + len - 2);
    if (crc16Calc == crc16Recv)
    {
        return true;
    }
    LOGE(TAG, "crc16Calc = 0x%X, crc16Recv = 0x%X, len = %d", crc16Calc, crc16Recv, len);
    hexdump(pBuf, len);
    return false;
}

#if (HANDLE_DATA_LOGIC == 0)
static void daisy_uart_task(void *p_arg)
{
    LOGD(TAG, "daisy_uart_task RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());
    daisy_uart_msg_st daisyUartMsg;

    /*创建消息队列*/
    gDaisyUartQueueHandle = xQueueCreate(10, sizeof(daisy_uart_msg_st));
    LOGI(TAG, "gDaisyUartQueueHandle = 0x%08X", gDaisyUartQueueHandle);
    configASSERT(gDaisyUartQueueHandle != NULL);

    for (;;)
    {
        /*等待串口消息*/
        LOGV_U(TAG, "Wait daisy uart message...");
        xQueueReceive(gDaisyUartQueueHandle, &daisyUartMsg, portMAX_DELAY);
        //LOGD(TAG, "daisyUartMsg.port = %d", daisyUartMsg.port);
#ifdef DEBUG_UART_TASK
        hexdump(daisyUartMsg.data, daisyUartMsg.length);
#endif
        daisyHandleRS485ReceiveData(&daisyUartMsg);
    }
}
#endif

/**
  * @brief  触发扩展事件
  * @param  event 事件类型
  * @retval None
  */
static void daisy_set_event_ext_receive(void)
{
    BaseType_t xHigherPriorityTaskWoken, xResult;
    xHigherPriorityTaskWoken = pdFALSE;
    xResult                  = pdFAIL;

    xResult = xEventGroupSetBitsFromISR(g_kalyke_event_group, DAISY_EVENT_WAIT_EXT_RECEIVE, &xHigherPriorityTaskWoken);
    if (xResult != pdFAIL)
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    else
    {
        LOGE(TAG, "daisy_set_event_ext_receive return %d.", xResult);
    }
}

/**
  * @brief  等待扩展事件
  * @param  timeout 超时时间
  * @retval 1：事件出发
            0：等待超时
  */
static bool daisy_wait_event_ext_receive(uint32_t timeout)
{
    EventBits_t uxBits;

    uxBits = xEventGroupWaitBits(g_kalyke_event_group, DAISY_EVENT_WAIT_EXT_RECEIVE, pdTRUE, pdFALSE, timeout);
    if (( uxBits & DAISY_EVENT_WAIT_EXT_RECEIVE ) != 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
  * @brief  设置背板总线错误
  * @param  errCode 错误码
  * @param  errAddr 模块地址
  * @param  errContent 错误内容
  * @retval None
  */
static void daisy_uart_set_error(uint16_t errCode, uint16_t errAddr,  uint16_t errContent)
{
    LOGV(TAG, "Enter %s", __func__);
    LOGE(TAG, "errCode: 0x%04X, errAddr: 0x%04X, errContent: 0x%04X", errCode, errAddr, errContent);

    gMasterRunInfo.daisy_uart_err_flag = 1;

    SET_SD_ELEMENT_VALUE(SD240, errCode);
    saveErrorCodeFIFO(errCode);
    SET_SD_ELEMENT_VALUE(SD241, errAddr);
    SET_SD_ELEMENT_VALUE(SD242, errContent);
    plc_refresh_error_msg(ERR_EXTEND_ERR);

    guv_NonStopError.bit.extend_bus_err = 1;
}

/**
  * @brief  清除背板总线错误
  * @param  None
  * @retval None
  */
static inline void daisy_uart_clear_error(void)
{
    gMasterRunInfo.daisy_uart_err_flag = 0;

    SET_SD_ELEMENT_VALUE(SD240, 0);
    SET_SD_ELEMENT_VALUE(SD241, 0);
    SET_SD_ELEMENT_VALUE(SD242, 0);

    if(gMasterRunInfo.daisy_err_flag)
    {
        return;
    }
    else
    {
        if(guv_NonStopError.bit.extend_bus_err == 1)
        {
            guv_NonStopError.bit.extend_bus_err = 0;
            bsp_close_err_led();
        }
    }
}

/**
  * @brief  扫描扩展
  * @param  None
  * @retval None
  */
static void daisy_uart_task_handle_scan_ext(void)
{
    uint16_t *pBuf;
    uint8_t i;

    gMasterRunInfo.daisy_uart_status = MASTER_IN_SCAN_EXT;

    //send 0101, not receive
    pBuf = (uint16_t *)gDaisyUartSendBuffer;
    *pBuf++ = DAISY_CMD_0101;
    *pBuf++ = 0;
    *pBuf++ = DAISY_MAX_EXTEND_NUM;
    daisy_uart1_send_crcdata(gDaisyUartSendBuffer, 6);

    vTaskDelay(10);

    //send 0303, receive module ID
    gMasterRunInfo.scan_ext_num = 0;
    for(i = 0; i < DAISY_MAX_EXTEND_NUM; i++)
    {
        gMasterRunInfo.daisy_uart_index = i + 1;
        pBuf = (uint16_t *)gDaisyUartSendBuffer;
        *pBuf++ = DAISY_CMD_0303;
        *pBuf++ = gMasterRunInfo.daisy_uart_index;
        *pBuf++ = 0;
        *pBuf++ = 0;
        before_cal_uart_resp_time();
        daisy_uart1_send_crcdata(gDaisyUartSendBuffer, 8);
        if(daisy_wait_event_ext_receive(DAISY_WAIT_EXT_0303_MAX_TIME))
        {
            if(gMasterRunInfo.daisy_uart_err_flag) //收到错误帧
            {
                gMasterRunInfo.scan_ext_num = 0;
                vTaskDelay(DAISY_SCAN_EXT_ERR_DELAY_TIME);
                break;
            }
            else
            {
                //LOGD(TAG, "Scan ext num[%d]++", gMasterRunInfo.scan_ext_num);
                gMasterRunInfo.scan_ext_num++;
                //vTaskDelay(DAISY_WAIT_EXT_0303_MAX_TIME - 1);
            }
        }
        else //后面没有扩展了
        {
            //LOGW(TAG, "Scan ext num[%d] end", gMasterRunInfo.scan_ext_num);
            break;
        }
    }

    LOGD(TAG, "Scan ext number: %d", gMasterRunInfo.scan_ext_num);
    if(gMasterRunInfo.scan_ext_num == 0) //没有扫描到扩展
    {
        gMasterRunInfo.daisy_uart_index = 0;
        gMasterRunInfo.daisy_uart_status = MASTER_SCAN_EXT_OVER;
        return;
    }
    for(i = 0; i < gMasterRunInfo.scan_ext_num; i++)
    {
        LOGI(TAG, "Ext_ID_VERSION_Table[%d] = 0x%08X", i, gMasterRunInfo.scan_ext_id_version[i]);
    }

    //send 0101, wait receive, to tell the last module it is last
    gMasterRunInfo.daisy_uart_status = MASTER_IN_SCAN_NUM_EXT;
    pBuf = (uint16_t *)gDaisyUartSendBuffer;
    *pBuf++ = DAISY_CMD_0101;
    *pBuf++ = 0;
    *pBuf++ = gMasterRunInfo.scan_ext_num;
    before_cal_uart_resp_time();
    daisy_uart1_send_crcdata(gDaisyUartSendBuffer, 6);
    if(daisy_wait_event_ext_receive(DAISY_WAIT_EXT_0101_MAX_TIME))
    {
        if(gMasterRunInfo.daisy_uart_err_flag) //收到错误帧
        {
            gMasterRunInfo.scan_ext_num = 0;
            vTaskDelay(DAISY_SCAN_EXT_ERR_DELAY_TIME);
        }
        else
        {
            LOGD(TAG, "Master number ext success!");
        }
    }
    else
    {
        daisy_uart_set_error(DAISY_TIMEOUT_ERR_MASTER_NUM_EXT, 0, DAISY_WAIT_EXT_0101_MAX_TIME);
        gMasterRunInfo.scan_ext_num = 0;
        vTaskDelay(DAISY_SCAN_EXT_ERR_DELAY_TIME);
    }

    gMasterRunInfo.daisy_uart_index = 0;
    gMasterRunInfo.daisy_uart_status = MASTER_SCAN_EXT_OVER;
}

/**
  * @brief  扫描扩展结束
  * @param  None
  * @retval None
  */
static void daisy_uart_task_handle_scan_ext_over(void)
{
    gMasterRunInfo.daisy_uart_status = MASTER_NUM_EXT;
}

/**
  * @brief  编址扩展
  * @param  None
  * @retval None
  */
static void daisy_uart_task_handle_num_ext(void)
{
    uint16_t *pBuf;

    gMasterRunInfo.daisy_uart_status = MASTER_IN_NUM_EXT;
    pBuf = (uint16_t *)gDaisyUartSendBuffer;
    *pBuf++ = DAISY_CMD_0101;
    *pBuf++ = 0;
    *pBuf++ = gBusConfig.nMasterExtCount;
    before_cal_uart_resp_time();
    daisy_uart1_send_crcdata(gDaisyUartSendBuffer, 6);
    if(daisy_wait_event_ext_receive(DAISY_WAIT_EXT_0101_MAX_TIME))
    {
        if(gMasterRunInfo.daisy_uart_err_flag) //返回异常
        {
            CLEAR_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_EXT_NUM_Msk);
            gMasterRunInfo.daisy_uart_status = MASTER_NUM_EXT;
            vTaskDelay(DAISY_NUM_EXT_ERR_DELAY_TIME); //等待一下 重新编址
        }
        else //返回正常
        {
            LOGD(TAG, "Master number ext success!");
            daisy_uart_clear_error();
            SET_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_EXT_NUM_Msk);
            gMasterRunInfo.daisy_uart_status = MASTER_NUM_EXT_OVER;
            vTaskDelay(DAISY_WAIT_EXT_0101_MAX_TIME - 1);
        }
    }
    else
    {
        CLEAR_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_EXT_NUM_Msk);
        daisy_uart_set_error(DAISY_TIMEOUT_ERR_MASTER_NUM_EXT, 0, DAISY_WAIT_EXT_0101_MAX_TIME);
        gMasterRunInfo.daisy_uart_status = MASTER_NUM_EXT;
        vTaskDelay(DAISY_NUM_EXT_ERR_DELAY_TIME); //等待一下 重新编址
    }
}

/**
  * @brief  编址扩展结束
  * @param  None
  * @retval None
  */
static void daisy_uart_task_handle_num_ext_over(void)
{
    gMasterRunInfo.daisy_uart_err_times = 0;
    gMasterRunInfo.daisy_uart_index = 0;
    gMasterRunInfo.daisy_uart_status = MASTER_CONFIG_EXT;
}

/**
  * @brief  配置扩展
  * @param  None
  * @retval None
  */
static void daisy_uart_task_handle_config_ext(void)
{
    LOGW(TAG, "Enter %s, daisy_uart_status = %d", __func__, gMasterRunInfo.daisy_uart_status);
    uint16_t *pu16Data;
    uint8_t  *pu8Data;

    gMasterRunInfo.daisy_uart_status = MASTER_IN_CONFIG_EXT;

    pu16Data = (uint16_t *)gDaisyUartSendBuffer;
    *pu16Data++ = DAISY_CMD_1C1C;
    pu8Data = (uint8_t *)pu16Data;
    memcpy(pu8Data, gBusConfig.item[gMasterRunInfo.daisy_uart_index].pSubStationbuf, gBusConfig.item[gMasterRunInfo.daisy_uart_index].len);
    //hexdump(gDaisyUartSendBuffer, gBusConfig.item[gMasterRunInfo.daisy_uart_index].len + 2);
    before_cal_uart_resp_time();
    daisy_uart1_send_crcdata(gDaisyUartSendBuffer, 2 + gBusConfig.item[gMasterRunInfo.daisy_uart_index].len);
    if(daisy_wait_event_ext_receive(DAISY_WAIT_EXT_1C1C_MAX_TIME))
    {
        if(gMasterRunInfo.daisy_uart_err_flag) //返回异常
        {
            gMasterRunInfo.daisy_uart_err_times++;
            if(gMasterRunInfo.daisy_uart_err_times > DAISY_UART_CONFIG_ERR_MAX_TIMES)
            {
                CLEAR_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_EXT_CONFIG_Msk);
                //gMasterRunInfo.daisy_uart_index = 0;
                gMasterRunInfo.daisy_uart_err_times = 0;
                gMasterRunInfo.daisy_uart_status = MASTER_CONFIG_EXT; //异常超过次数后
            }
            else
            {
                gMasterRunInfo.daisy_uart_status = MASTER_CONFIG_EXT; //再次配置
            }
            vTaskDelay(DAISY_CONFIG_EXT_ERR_DELAY_TIME);
        }
        else //返回正常
        {
            LOGD(TAG, "Master config ext success!");
            daisy_uart_clear_error();
            gMasterRunInfo.daisy_uart_err_times = 0;
            gMasterRunInfo.daisy_uart_index++;
            if(gMasterRunInfo.daisy_uart_index >= gBusConfig.nMasterExtCount) //配置好最后一个模块
            {
                gMasterRunInfo.daisy_uart_index = 0;
                gMasterRunInfo.daisy_uart_status = MASTER_CONFIG_EXT_OVER;
                SET_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_EXT_CONFIG_Msk);
            }
            else
            {
                gMasterRunInfo.daisy_uart_status = MASTER_CONFIG_EXT; //继续配置下一个模块
            }
            vTaskDelay(DAISY_WAIT_EXT_1C1C_MAX_TIME - 1);
        }
    }
    else
    {
        gMasterRunInfo.daisy_uart_err_times++;
        if(gMasterRunInfo.daisy_uart_err_times > DAISY_UART_CONFIG_ERR_MAX_TIMES)
        {
            CLEAR_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_EXT_CONFIG_Msk);
            gMasterRunInfo.daisy_uart_err_times = 0;
            //gMasterRunInfo.daisy_uart_index = 0;
            daisy_uart_set_error(DAISY_TIMEOUT_ERR_MASTER_CONFIG_EXT, GET_SMLPU16_DATA(gBusConfig.item[gMasterRunInfo.daisy_uart_index].pSubStationbuf), DAISY_WAIT_EXT_1C1C_MAX_TIME);
            gMasterRunInfo.daisy_uart_status = MASTER_CONFIG_EXT; //超时后
        }
        else
        {
            gMasterRunInfo.daisy_uart_status = MASTER_CONFIG_EXT; //再次配置
        }
        vTaskDelay(DAISY_CONFIG_EXT_ERR_DELAY_TIME);
    }
}

/**
  * @brief  配置扩展结束
  * @param  None
  * @retval None
  */
static void daisy_uart_task_handle_config_ext_over(void)
{
    gMasterRunInfo.daisy_uart_err_times = 0;
    gMasterRunInfo.daisy_uart_status = MASTER_PRELOOP_EXT;

}

/**
  * @brief  预启动扩展
  * @param  None
  * @retval None
  */
static void daisy_uart_task_handle_preloop_ext(void)
{
    uint16_t *pBuf;

    gMasterRunInfo.daisy_uart_status = MASTER_IN_PRELOOP_EXT;
    pBuf = (uint16_t *)gDaisyUartSendBuffer;
    *pBuf++ = DAISY_CMD_2222;
    *pBuf++ = 0;
    *pBuf++ = gBusConfig.nMasterExtCount;
    before_cal_uart_resp_time();
    daisy_uart1_send_crcdata(gDaisyUartSendBuffer, 6);
    if(daisy_wait_event_ext_receive(DAISY_WAIT_EXT_2222_MAX_TIME))
    {
        if(gMasterRunInfo.daisy_uart_err_flag) //返回异常
        {
            gMasterRunInfo.daisy_uart_err_times++;
            if(gMasterRunInfo.daisy_uart_err_times > DAISY_UART_PRELOOP_ERR_MAX_TIMES)
            {
                CLEAR_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_EXT_PRELOOP_Msk);
                gMasterRunInfo.daisy_uart_err_times = 0;
                gMasterRunInfo.daisy_uart_status = MASTER_PRELOOP_EXT; //异常超过次数
            }
            else
            {
                gMasterRunInfo.daisy_uart_status = MASTER_PRELOOP_EXT; //再次预启动
            }
            vTaskDelay(DAISY_PRELOOP_EXT_ERR_DELAY_TIME);
        }
        else //返回正常
        {
            LOGD(TAG, "Master preloop ext success!");
            daisy_uart_clear_error();
            gMasterRunInfo.daisy_uart_err_times = 0;
            SET_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_EXT_PRELOOP_Msk);
            gMasterRunInfo.daisy_uart_status = MASTER_PRELOOP_EXT_OVER;
            vTaskDelay(DAISY_WAIT_EXT_2222_MAX_TIME - 1);
        }
    }
    else
    {
        gMasterRunInfo.daisy_uart_err_times++;
        if(gMasterRunInfo.daisy_uart_err_times > DAISY_UART_PRELOOP_ERR_MAX_TIMES)
        {
            CLEAR_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_EXT_PRELOOP_Msk);
            daisy_uart_set_error(DAISY_TIMEOUT_ERR_MASTER_PRELOOP_EXT, 0, DAISY_WAIT_EXT_2222_MAX_TIME);
            gMasterRunInfo.daisy_uart_err_times = 0;
            gMasterRunInfo.daisy_uart_status = MASTER_PRELOOP_EXT; //超时后
        }
        else
        {
            gMasterRunInfo.daisy_uart_status = MASTER_PRELOOP_EXT; //再次预启动
        }
        vTaskDelay(DAISY_PRELOOP_EXT_ERR_DELAY_TIME);
    }
}

/**
  * @brief  预启动扩展结束
  * @param  None
  * @retval None
  */
static void daisy_uart_task_handle_preloop_ext_over(void)
{
    LOGW(TAG, "Enter %s, daisy_uart_status = %d", __func__, gMasterRunInfo.daisy_uart_status);

    if(READ_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_SLAVE_CAN_LOOP_Msk) != MASTER_RUN_FLAG_SLAVE_CAN_LOOP_Msk) //从站未启动好 等待
    {
        vTaskDelay(1000);
        return;
    }

    gMasterRunInfo.daisy_uart_err_times = 0;
    memset(gLoopBuf, 0, 4 + gBusConfig.nPdoBytesExt);
    gLoopBuf[0] = DAISY_CMD_3333 & 0xFF;
    gLoopBuf[1] = DAISY_CMD_3333 >> 8;
    gMasterRunInfo.daisy_uart_status = MASTER_LOOP_EXT;
}

static inline uint8_t daisy_uart_get_8bit_element_value(uint16_t *baseAddr, uint16_t element)
{
    uint8_t lcv_ElementValue;

    lcv_ElementValue = (*(baseAddr + (element >> 4)) >> (element & 0x0F));

    return lcv_ElementValue;
}

/**
  * @brief  计算响应时间并添加延时
            |------------ LoopTime -------------|
            |______________|____________________|
              ResponseTime        DelayTime

            DelayTime = SD[234] - ResponseTime
            SD[235]   = LoopTime
  * @param  None
  * @retval None
  */
static inline void daisy_uart_loop_delay(void)
{
    static uint32_t msTick_start = 0;
    uint32_t msInterval, msTick_end;

    if(msTick_start == 0)
    {
        msTick_start = xTaskGetTickCount();
        return;
    }

    //ResponseTime
    msTick_end = xTaskGetTickCount();
    msInterval = msTick_end - msTick_start;

    //DelayTime
    if(msInterval < gtv_PlcElement.msp_SDElement[SD234])
    {
        vTaskDelay(gtv_PlcElement.msp_SDElement[SD234] - msInterval);
    }
    else
    {
        vTaskDelay(1); //give plc task some time
    }

    //LoopTime
    msTick_end = xTaskGetTickCount();
    msInterval = msTick_end - msTick_start;
    /*刷新当前时间和最大时间*/
    gtv_PlcElement.msp_SDElement[SD235] = msInterval;
    if(msInterval > GET_SD_ELEMENT_VALUE(SD237))
    {
        SET_SD_ELEMENT_VALUE(SD237, msInterval);
    }
    msTick_start = msTick_end;
}

/**
  * @brief  数据交互
  * @param  None
  * @retval None
  */
static inline void daisy_uart_task_handle_loop_ext(void)
{
    plc_refresh_force_element_value();

    uint8_t *pData;
    uint16_t LoopLen;

    LoopLen = gBusConfig.nPdoBytesExt + 2; // '+2'是因为加入了0x3333两个字节
    pData = gLoopBuf + 2;
    for (int i = 0; i < gBusConfig.pdo1BCountExt; i++)
    {
        switch (gBusConfig.pPDO1B_Ext[i].eType)
        {
        case DAISY_E_TYPE_Y:
            pData[gBusConfig.pPDO1B_Ext[i].offset] = daisy_uart_get_8bit_element_value(Y_ELEMENT, gBusConfig.pPDO1B_Ext[i].eAddr);
            break;

        case DAISY_E_TYPE_D:
            if(gBusConfig.pPDO1B_Ext[i].len == 4)
            {
                pData[gBusConfig.pPDO1B_Ext[i].offset] = (gtv_PlcElement.msp_DElement[gBusConfig.pPDO1B_Ext[i].eAddr + 1] & 0xFF00) >> 8;
                pData[gBusConfig.pPDO1B_Ext[i].offset + 1] = gtv_PlcElement.msp_DElement[gBusConfig.pPDO1B_Ext[i].eAddr + 1] & 0x00FF;
                pData[gBusConfig.pPDO1B_Ext[i].offset + 2] = (gtv_PlcElement.msp_DElement[gBusConfig.pPDO1B_Ext[i].eAddr] & 0xFF00) >> 8;
                pData[gBusConfig.pPDO1B_Ext[i].offset + 3] = gtv_PlcElement.msp_DElement[gBusConfig.pPDO1B_Ext[i].eAddr] & 0x00FF;
            }
            else
            {
                pData[gBusConfig.pPDO1B_Ext[i].offset] = (gtv_PlcElement.msp_DElement[gBusConfig.pPDO1B_Ext[i].eAddr] & 0xFF00) >> 8;
                pData[gBusConfig.pPDO1B_Ext[i].offset + 1] = gtv_PlcElement.msp_DElement[gBusConfig.pPDO1B_Ext[i].eAddr] & 0x00FF;
            }
            break;

        case DAISY_E_TYPE_SD:
            pData[gBusConfig.pPDO1B_Ext[i].offset] = (gtv_PlcElement.msp_SDElement[gBusConfig.pPDO1B_Ext[i].eAddr] & 0xFF00) >> 8;
            pData[gBusConfig.pPDO1B_Ext[i].offset + 1] = gtv_PlcElement.msp_SDElement[gBusConfig.pPDO1B_Ext[i].eAddr] & 0x00FF;
            break;

        case DAISY_E_TYPE_R:
            pData[gBusConfig.pPDO1B_Ext[i].offset] = (gtv_PlcElement.msp_RElement[gBusConfig.pPDO1B_Ext[i].eAddr] & 0xFF00) >> 8;
            pData[gBusConfig.pPDO1B_Ext[i].offset + 1] = gtv_PlcElement.msp_RElement[gBusConfig.pPDO1B_Ext[i].eAddr] & 0x00FF;
            break;

        default:
            LOGE(TAG, "ERROR: eType = %u", gBusConfig.pPDO1B_Ext[i].eType);
            break;
        }
    }

    before_cal_uart_resp_time();
    daisy_uart1_send_crcdata(gLoopBuf, LoopLen);
    if(daisy_wait_event_ext_receive(gBusConfig.nBfm608A))
    {
        if(gMasterRunInfo.daisy_uart_err_flag)
        {
            gMasterRunInfo.daisy_uart_err_times++;
            gtv_PlcElement.msp_SDElement[SD246] = ++gMasterRunInfo.daisy_uart_totalerr_times;
            if(gMasterRunInfo.daisy_uart_err_times > DAISY_UART_LOOP_ERR_MAX_TIMES)
            {
                gMasterRunInfo.daisy_uart_err_times = 0;
                //gMasterRunInfo.daisy_uart_status = MASTER_IDLE; //错误超限后
            }
            else
            {
                gMasterRunInfo.daisy_uart_status = MASTER_LOOP_EXT; //继续发送
            }
            CLEAR_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_EXT_RUNNING_Msk);
        }
        else
        {
            gMasterRunInfo.daisy_uart_err_times = 0;
            SET_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_EXT_RUNNING_Msk);
            //LOGD(TAG, "Loop ext success!");
        }
    }
    else
    {
        gMasterRunInfo.daisy_uart_err_times++;
        gtv_PlcElement.msp_SDElement[SD246] = ++gMasterRunInfo.daisy_uart_totalerr_times;
        if(gMasterRunInfo.daisy_uart_err_times > DAISY_UART_LOOP_ERR_MAX_TIMES)
        {
            daisy_uart_set_error(DAISY_TIMEOUT_ERR_MASTER_LOOP_EXT, 0, gBusConfig.nBfm608A);
            gMasterRunInfo.daisy_uart_err_times = 0;
            //gMasterRunInfo.daisy_uart_status = MASTER_IDLE; //错误超限后
        }
        else
        {
            gMasterRunInfo.daisy_uart_status = MASTER_LOOP_EXT; //继续发送
        }
        CLEAR_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_EXT_RUNNING_Msk);
    }

    daisy_uart_loop_delay();
}

/**
  * @brief  背板总线任务
  * @param  None
  * @retval None
  */
static void daisy_uart_task(void *p_arg)
{
    LOGV(TAG, "daisy_uart_loop_task RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());

    bsp_feed_watch_dog();
    vTaskDelay(DAISY_UART_READY_START_DELAY_TIME); //等待智能设备启动
    daisy_uart_task_handle_scan_ext();
    gMasterRunInfo.daisy_uart_status = MASTER_NUM_EXT;

    if(gBusConfig.nPdoCountExt == 0) //没有扩展模块
    {
        LOGW(TAG, "There is no extend module.");
        SET_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_EXT_CAN_LOOP_Msk);
        gMasterRunInfo.daisy_uart_status = MASTER_IDLE;
    }

    for(;;)
    {
        if (gGUHUAing == 1 || gSuspendFlag == true)
        {
            LOGE(TAG, "In %s(),GUHUAing.........vTaskDelay(1000)..", __func__);
            vTaskDelay(1000);
            continue;
        }
        switch(gMasterRunInfo.daisy_uart_status)
        {
        case MASTER_IDLE:
            //LOGD(TAG, "daisy uart task idle.");
            vTaskDelay(1000);
            break;

        case MASTER_SCAN_EXT:
            daisy_uart_task_handle_scan_ext();
            break;

        case MASTER_SCAN_EXT_OVER:
            daisy_uart_task_handle_scan_ext_over();
            break;

        case MASTER_NUM_EXT:
            daisy_uart_task_handle_num_ext();
            break;

        case MASTER_NUM_EXT_OVER:
            daisy_uart_task_handle_num_ext_over();
            break;

        case MASTER_CONFIG_EXT:
            daisy_uart_task_handle_config_ext();
            break;

        case MASTER_CONFIG_EXT_OVER:
            daisy_uart_task_handle_config_ext_over();
            break;

        case MASTER_PRELOOP_EXT:
            daisy_uart_task_handle_preloop_ext();
            break;

        case MASTER_PRELOOP_EXT_OVER:
            daisy_uart_task_handle_preloop_ext_over();
            break;

        case MASTER_LOOP_EXT:
            daisy_uart_task_handle_loop_ext();
            break;

        default:
            LOGW(TAG, "Other gMasterRunInfo.daisy_uart_status = %d", gMasterRunInfo.daisy_uart_status);
            break;
        }

        //vTaskDelay(1); //解析操作里有添加阻塞 不需要额外加延时
    }
}

/**
  * @brief  判断是否是5555错误帧
  * @param  pBuf 数据
  * @param  len 长度
  * @retval None
  */
static bool is_ext_err_frame_5555(uint8_t *pBuf, uint16_t len)
{
    if(GET_SMLPU16_DATA(pBuf) == DAISY_CMD_5555)
    {
        return true;
    }

    return false;
}

/**
  * @brief  解析扩展返回的错误
  * @param  pBuf 数据
  * @retval None
  */
static void in_ext_err_handle_5555(uint8_t *pBuf)
{
    uint8_t *pTemp;
    uint16_t errCode;
    uint16_t errAddr;
    uint16_t errContent;

    pTemp = pBuf + 2;
    errCode = GET_SMLPU16_DATA(pTemp);

    pTemp += 2;
    errAddr = GET_SMLPU16_DATA(pTemp);

    pTemp += 2;
    errContent = GET_SMLPU16_DATA(pTemp);

    daisy_uart_set_error(errCode, errAddr, errContent);
}

/**
  * @brief  解析扩展响应的编址数据(0101)
  * @param  pBuf 数据
  * @param  Len 长度
  * @retval None
  */
static void in_num_ext_handle_0101(uint8_t *pBuf, uint16_t Len)
{
    uint8_t *pTemp;
    uint16_t Cmd;
    uint16_t NodeNum;

    if(Len < 4)
    {
        daisy_uart_set_error(DAISY_LEN_ERR_MASTER_NUM_EXT, 0, Len);
        return;
    }

    pTemp = pBuf;
    Cmd = GET_SMLPU16_DATA(pTemp);

    if(crc16_check(pBuf, Len) == false)
    {
        daisy_uart_set_error(DAISY_CRC_ERR_MASTER_NUM_EXT, 0, Cmd);
        return;
    }

    if(Cmd != DAISY_CMD_0101)
    {
        if(is_ext_err_frame_5555(pTemp, Len))
        {
            in_ext_err_handle_5555(pTemp);
        }
        else //不该收到其他指令
        {
            daisy_uart_set_error(DAISY_CMD_ERR_MASTER_NUM_EXT, 0, Cmd);
        }
        return;
    }

    pTemp += 2;
    NodeNum = GET_SMLPU16_DATA(pTemp);
    if(NodeNum != gBusConfig.nMasterExtCount) //返回模块数不对
    {
        daisy_uart_set_error(DAISY_NODEADDR_ERR_MASTER_NUM_EXT, 0, NodeNum);
    }
    else
    {
        daisy_uart_clear_error();
    }
}

/**
  * @brief  解析扫描扩展时响应的编址数据(0101)
  * @param  pBuf 数据
  * @param  Len 长度
  * @retval None
  */
static void in_scan_num_ext_handle_0101(uint8_t *pBuf, uint16_t Len)
{
    uint8_t *pTemp;
    uint16_t Cmd;
    uint16_t NodeNum;

    if(Len < 4)
    {
        daisy_uart_set_error(DAISY_LEN_ERR_MASTER_NUM_EXT, 0, Len);
        return;
    }

    pTemp = pBuf;
    Cmd = GET_SMLPU16_DATA(pTemp);

    if(crc16_check(pBuf, Len) == false)
    {
        daisy_uart_set_error(DAISY_CRC_ERR_MASTER_NUM_EXT, 0, Cmd);
        return;
    }

    if(Cmd != DAISY_CMD_0101)
    {
        if(is_ext_err_frame_5555(pTemp, Len))
        {
            in_ext_err_handle_5555(pTemp);
        }
        else //不该收到其他指令
        {
            daisy_uart_set_error(DAISY_CMD_ERR_MASTER_NUM_EXT, 0, Cmd);
        }
        return;
    }

    pTemp += 2;
    NodeNum = GET_SMLPU16_DATA(pTemp);
    if(NodeNum != gMasterRunInfo.scan_ext_num)
    {
        daisy_uart_set_error(DAISY_NODEADDR_ERR_MASTER_NUM_EXT, 0, NodeNum);
    }
    else
    {
        daisy_uart_clear_error();
    }
}

/**
  * @brief  解析扩展响应的扫描数据(0303)
  * @param  pBuf 数据
  * @param  Len 长度
  * @retval None
  */
static void in_scan_ext_handle_0303(uint8_t *pBuf, uint16_t Len)
{
    uint8_t *pTemp;
    uint16_t Cmd;
    uint16_t NodeNum;

    if(Len < 4)
    {
        daisy_uart_set_error(DAISY_LEN_ERR_MASTER_SCAN_EXT, gMasterRunInfo.daisy_uart_index, Len);
        return;
    }

    pTemp = pBuf;
    Cmd = GET_SMLPU16_DATA(pTemp);

    if(crc16_check(pBuf, Len) == false)
    {
        daisy_uart_set_error(DAISY_CRC_ERR_MASTER_SCAN_EXT, gMasterRunInfo.daisy_uart_index, Cmd);
        return;
    }

    if(Cmd != DAISY_CMD_0303)
    {
        if(is_ext_err_frame_5555(pTemp, Len))
        {
            in_ext_err_handle_5555(pTemp);
        }
        else //不该收到其他指令
        {
            daisy_uart_set_error(DAISY_CMD_ERR_MASTER_SCAN_EXT, gMasterRunInfo.daisy_uart_index, Cmd);
        }
        return;
    }

    pTemp += 2;
    NodeNum = GET_SMLPU16_DATA(pTemp);
    if(NodeNum == gMasterRunInfo.daisy_uart_index)
    {
        pTemp += 2;
        gMasterRunInfo.scan_ext_id_version[NodeNum - 1] = GET_SMLPU16_DATA(pTemp) << 16; //get id
        pTemp += 2;
        gMasterRunInfo.scan_ext_id_version[NodeNum - 1] |= GET_SMLPU16_DATA(pTemp); //get version
        daisy_uart_clear_error();
    }
    else
    {
        daisy_uart_set_error(DAISY_NODEADDR_ERR_MASTER_SCAN_EXT, gMasterRunInfo.daisy_uart_index, NodeNum);
    }
}

/**
  * @brief  解析扩展响应的配置数据(1C1C)
  * @param  pBuf 数据
  * @param  Len 长度
  * @retval None
  */
static void in_config_ext_handle_1C1C(uint8_t *pBuf, uint16_t Len)
{
    uint8_t *pTemp;
    uint16_t Cmd;
    uint16_t ErrNum;

    if(Len < 4)
    {
        daisy_uart_set_error(DAISY_LEN_ERR_MASTER_CONFIG_EXT, 0, Len);
        return;
    }

    pTemp = pBuf;
    Cmd = GET_SMLPU16_DATA(pTemp);

    if(crc16_check(pBuf, Len) == false)
    {
        daisy_uart_set_error(DAISY_CRC_ERR_MASTER_CONFIG_EXT, 0, Cmd);
        return;
    }

    if(Cmd != DAISY_CMD_1C1C)
    {
        if(is_ext_err_frame_5555(pTemp, Len))
        {
            in_ext_err_handle_5555(pTemp);
        }
        else //不该收到其他指令
        {
            daisy_uart_set_error(DAISY_CMD_ERR_MASTER_CONFIG_EXT, GET_SMLPU16_DATA(gBusConfig.item[gMasterRunInfo.daisy_uart_index].pSubStationbuf), Cmd);
        }
        return;
    }

    pTemp += 2;
    ErrNum = GET_SMLPU16_DATA(pTemp);
    if(ErrNum == DAISY_COMMON_NO_ERR)
    {
        daisy_uart_clear_error();
    }
    else
    {
        daisy_uart_set_error(DAISY_OTHER_ERR_MASTER_CONFIG_EXT, GET_SMLPU16_DATA(gBusConfig.item[gMasterRunInfo.daisy_uart_index].pSubStationbuf), ErrNum);
    }
}

/**
  * @brief  解析扩展响应的预启动数据(2222)
  * @param  pBuf 数据
  * @param  Len 长度
  * @retval None
  */
static void in_preloop_ext_handle_2222(uint8_t *pBuf, uint16_t Len)
{
    uint8_t *pTemp;
    uint16_t Cmd;
    uint16_t NodeNum;

    if(Len < 4)
    {
        daisy_uart_set_error(DAISY_LEN_ERR_MASTER_PRELOOP_EXT, 0, Len);
        return;
    }

    pTemp = pBuf;
    Cmd = GET_SMLPU16_DATA(pTemp);

    if(crc16_check(pBuf, Len) == false)
    {
        daisy_uart_set_error(DAISY_CRC_ERR_MASTER_PRELOOP_EXT, 0, Cmd);
        return;
    }

    if(Cmd != DAISY_CMD_2222)
    {
        if(is_ext_err_frame_5555(pTemp, Len))
        {
            in_ext_err_handle_5555(pTemp);
        }
        else //不该收到其他指令
        {
            daisy_uart_set_error(DAISY_CMD_ERR_MASTER_PRELOOP_EXT, 0, Cmd);
        }
        return;
    }

    pTemp += 2;
    NodeNum = GET_SMLPU16_DATA(pTemp);
    if(NodeNum == gBusConfig.nMasterExtCount)
    {
        daisy_uart_clear_error();
    }
    else
    {
        daisy_uart_set_error(DAISY_NODEADDR_ERR_MASTER_PRELOOP_EXT, 0, NodeNum);
    }
}

static inline void daisy_uart_set_8bit_element_value(unsigned short *baseAddr, unsigned short bitElement, unsigned char value)
{
    baseAddr += (bitElement >> 4);
    if ((bitElement & 0x0F) == 0)
    {
        *baseAddr &= 0xFF00;
        *baseAddr |= value;
    }
    else
    {
        *baseAddr &= 0x00FF;
        *baseAddr |= (value << 8);
    }
}

/**
  * @brief  解析扩展响应的集数帧数据(3333)
  * @param  pBuf 数据
  * @param  Len 长度
  * @retval None
  */
static inline void loop_ext_handle_3333(uint8_t *pBuf, uint16_t Len)
{
    uint8_t *pTemp;
    uint16_t Cmd;

    if(Len < 4)
    {
        daisy_uart_set_error(DAISY_LEN_ERR_MASTER_LOOP_EXT, 0, Len);
        return;
    }

    pTemp = pBuf;
    Cmd = GET_SMLPU16_DATA(pTemp);

    if(crc16_check(pBuf, Len) == false)
    {
        daisy_uart_set_error(DAISY_CRC_ERR_MASTER_LOOP_EXT, 0, Cmd);
        return;
    }

    if(Cmd != DAISY_CMD_3333)
    {
        if(is_ext_err_frame_5555(pTemp, Len))
        {
            in_ext_err_handle_5555(pTemp);
        }
        else //不该收到其他指令
        {
            daisy_uart_set_error(DAISY_CMD_ERR_MASTER_LOOP_EXT, 0, Cmd);
        }
        return;
    }

    pTemp += 2;
    for (uint16_t i = 0; i < gBusConfig.pdo1ACountExt; i++)
    {
        //LOGW(TAG, "gBusConfig.pPDO1A_Ext[%u].eType = %u", i, gBusConfig.pPDO1A_Ext[i].eType);
        switch (gBusConfig.pPDO1A_Ext[i].eType)
        {
        case DAISY_E_TYPE_X:
            daisy_uart_set_8bit_element_value(X_ELEMENT, gBusConfig.pPDO1A_Ext[i].eAddr, pTemp[gBusConfig.pPDO1A_Ext[i].offset]);
            break;

        case DAISY_E_TYPE_D:
            if(gBusConfig.pPDO1A_Ext[i].len == 4)
            {
                gtv_PlcElement.msp_DElement[gBusConfig.pPDO1A_Ext[i].eAddr] = GET_BIGPU16_DATA(&pTemp[gBusConfig.pPDO1A_Ext[i].offset + 2]);
                gtv_PlcElement.msp_DElement[gBusConfig.pPDO1A_Ext[i].eAddr + 1] = GET_BIGPU16_DATA(&pTemp[gBusConfig.pPDO1A_Ext[i].offset]);
            }
            else
            {
                gtv_PlcElement.msp_DElement[gBusConfig.pPDO1A_Ext[i].eAddr] = GET_BIGPU16_DATA(&pTemp[gBusConfig.pPDO1A_Ext[i].offset]);
            }
            break;

        case DAISY_E_TYPE_SD:
            gtv_PlcElement.msp_SDElement[gBusConfig.pPDO1A_Ext[i].eAddr] = GET_BIGPU16_DATA(&pTemp[gBusConfig.pPDO1A_Ext[i].offset]);
            break;

        case DAISY_E_TYPE_R:
            gtv_PlcElement.msp_RElement[gBusConfig.pPDO1A_Ext[i].eAddr] = GET_BIGPU16_DATA(&pTemp[gBusConfig.pPDO1A_Ext[i].offset]);
            break;

        default:
            LOGE(TAG, "%s(): element type error(%u)", __func__, gBusConfig.pPDO1A_Ext[i].eType);
            break;
        }
    }
    daisy_uart_clear_error();
}

/**
  * @brief  主站在其他状态下解析扩展响应的数据
  * @param  pBuf 数据
  * @param  Len 长度
  * @retval None
  */
static void in_other_status_ext_handle(uint8_t *pBuf, uint16_t Len)
{
    if(Len < 2) //长度小于2，取cmd可能会死机
    {
        daisy_uart_set_error(DAISY_STATE_LEN_ERR_MASTER_RECV_EXT, 0, gMasterRunInfo.daisy_uart_status);
    }
    else if(Len < 4)
    {
        daisy_uart_set_error(DAISY_STATE_LEN_ERR_MASTER_RECV_EXT, 0, gMasterRunInfo.daisy_uart_status);
    }
    else if(is_ext_err_frame_5555(pBuf, Len))
    {
        in_ext_err_handle_5555(pBuf);
    }
    else
    {
        daisy_uart_set_error(DAISY_STATE_ERR_MASTER_RECV_EXT, 0, gMasterRunInfo.daisy_uart_status);
    }
}

/**
  * @brief  RS485串口数据处理
  * @param  None
  * @retval None
  */
static inline void daisy_uart_handle_RS485_received_data(void)
{
    uint8_t *pBuf;
    uint16_t Len;

    pBuf = gUart3RecvBuffer;
    Len = gUart3RecvLength;

    if(gMasterRunInfo.daisy_uart_status != MASTER_LOOP_EXT) /* log print */
    {
        LOGD(TAG, "In status(%d) RS485_received_data", gMasterRunInfo.daisy_uart_status);
        hexdump(pBuf, Len);
    }

    switch(gMasterRunInfo.daisy_uart_status)
    {
    case MASTER_IN_NUM_EXT:
        in_num_ext_handle_0101(pBuf, Len);
        break;

    case MASTER_IN_SCAN_NUM_EXT:
        in_scan_num_ext_handle_0101(pBuf, Len);
        break;

    case MASTER_IN_SCAN_EXT:
        in_scan_ext_handle_0303(pBuf, Len);
        break;

    case MASTER_IN_CONFIG_EXT:
        in_config_ext_handle_1C1C(pBuf, Len);
        break;

    case MASTER_IN_PRELOOP_EXT:
        in_preloop_ext_handle_2222(pBuf, Len);
        break;

    case MASTER_LOOP_EXT:
        loop_ext_handle_3333(pBuf, Len);
        break;

    default:
        in_other_status_ext_handle(pBuf, Len);
        break;
    }

    daisy_set_event_ext_receive();
}

/**
  * @brief  创建背板总线任务
  * @param  None
  * @retval None
  */
void start_daisy_uart_task(void)
{
    LOGV(TAG, "Enter %s(), gDaisyUartLoopTaskHandler = 0x%08X", __func__, gDaisyUartLoopTaskHandler);

    if (gDaisyUartLoopTaskHandler != NULL)
    {
        return;
    }
    daisy_LPUART3_init(DAISY_BUS_BAUDRATE_485, 0, 8, 1);
    daisy_LPUART1_init(DAISY_BUS_BAUDRATE, 0, 8, 1);
    daisy_LPUART_DMA_init();

    //create task
    BaseType_t ret = xTaskCreate((TaskFunction_t)daisy_uart_task,
                                 (const char *)"daisyUartLoop",
                                 DAISY_UART_TASK_STACK_SIZE,
                                 (void *)NULL,
                                 DAISY_UART_TASK_PRIO,
                                 (TaskHandle_t *)&gDaisyUartLoopTaskHandler);
    if (ret != pdPASS)
    {
        LOGE(TAG, "Create daisy_uart_loop_task error!");
    }
    LOGD(TAG, "gDaisyUartLoopTaskHandler = 0x%08X", gDaisyUartLoopTaskHandler);
}


