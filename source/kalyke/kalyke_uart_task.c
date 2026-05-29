/**
  ******************************************************************************
  * @file    kalyke_uart_task.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   串口任务
  ******************************************************************************
  */
#include <limits.h>
#include "FreeRTOS.h"
#include "event_groups.h"
#include "kalyke_event.h"

#include "fsl_gpt.h"
#include "fsl_debug_console.h"
#include "plc_variable.h"
#include "mb.h"
#include "bsp_dct.h"
#include "bsp_uart.h"
#include "mb.h"
#include "queue.h"
#include "kalyke_uart_task.h"
#include "plc_commonfunc.h"
#include "kalyke_tool.h"
#include "fsl_qtmr.h"
#include "kalyke_internet_task.h"
#include "bsp_gpio.h"
#include "kalyke_collect_task.h"
/*------------------------------------------------------------------------------
*   UART TASK相关全局变量定义
*-----------------------------------------------------------------------------*/
TaskHandle_t gtv_UartTaskHandler;
QueueHandle_t gtv_UartTaskMsgQueueHandle;

#if 0 // TODO:
/**
  * @brief  串口数据处理
  * @param  None
  * @retval None
  */
void uart_task_handle_receive_data(uart_msg_st *ltp_UartMsg)
{
    md_slave_msg_pack ltv_UartMbSlaveMsg = {0x00, };

    switch(gtp_UartPort[ltp_UartMsg->mcv_UartPort].mcv_Mode) {
        case UART_TYPE_FREE_PORT:

            break;

        case UART_TYPE_MB_SLAVE:
            /*申请发送缓存区*/
            ltv_UartMbSlaveMsg.mcp_RespBuff = ring_buffer_get_write_mem(&gtp_UartPort[ltp_UartMsg->mcv_UartPort].mtv_SendBuff);
            configASSERT(ltv_UartMbSlaveMsg.mcp_RespBuff != NULL);

            /*切换发送缓冲区，为下次消息准备*/
            ring_buffer_switch_write_mem(&gtp_UartPort[ltp_UartMsg->mcv_UartPort].mtv_SendBuff);

            ltv_UartMbSlaveMsg.msv_ReceiveLen = ltp_UartMsg->msv_MsgLength;
            ltv_UartMbSlaveMsg.mcp_ReceiveBuff = ltp_UartMsg->mcp_DataBuff;
            /*标定信息发送者*/
            ltv_UartMbSlaveMsg.mcv_Sender = ltp_UartMsg->mcv_UartPort;

            /*Modbus诊断校验信息统计*/
            gtp_ModbusSlaveDiagInfo[ltp_UartMsg->mcv_UartPort].msv_BusPackageCnt += 1;

            if(ltv_UartMbSlaveMsg.mcp_ReceiveBuff[0] == 0x00) {
                ltv_UartMbSlaveMsg.mcv_IsBroadcastInfo = 1;
            }

            /*广播消息或者发送至本站消息*/
            if((ltv_UartMbSlaveMsg.mcp_ReceiveBuff[0] == gtp_ModbusSlaveDiagInfo[ltp_UartMsg->mcv_UartPort].mcv_SlaveId)
               || ltv_UartMbSlaveMsg.mcv_IsBroadcastInfo) {

                gtp_ModbusSlaveDiagInfo[ltp_UartMsg->mcv_UartPort].msv_SlavePackageCnt += 1;

                ltv_UartMbSlaveMsg.resp_func = gtp_UartPort[ltp_UartMsg->mcv_UartPort].pSendFunc;

                mb_slave_msg_handler(&ltv_UartMbSlaveMsg);
            }
            break;

        case UART_TYPE_MB_MASTER:

            break;
    }
}

/**
  * @brief  uart_task
  * @param  None
  * @retval None
  */
void uart_task(void *p_arg)
{
    uart_msg_st ltv_UartMsg;
    unsigned char i;

    /*串口结构体内存申请*/
    gtp_UartPort = (uart_port_info_st *)pvPortMalloc(sizeof(uart_port_info_st)*gtv_DeviceConfigTable.mtv_UartPort.mcv_SupportUartNum);
    configASSERT(gtp_UartPort != NULL);
    /*初始化*/
    for(i=0; i<sizeof(uart_port_info_st); i++) {
        *(((unsigned char *)gtp_UartPort)+i) = 0;
    }

    /*获取串口操作函数指针*/
    bsp_get_uart_configure_info(gtp_UartPort);

    /*创建消息队列*/
    gtv_UartTaskMsgQueueHandle = xQueueCreate(10, sizeof(uart_msg_st));
    configASSERT(gtv_UartTaskMsgQueueHandle != NULL);

    /*发送缓冲区初始化*/
    for(i=0; i<gtv_DeviceConfigTable.mtv_UartPort.mcv_SupportUartNum; i++) {
        ring_buffer_init(&gtp_UartPort[i].mtv_SendBuff, 1024);
    }

    while(1) {
        /*等待串口消息*/
        xQueueReceive(gtv_UartTaskMsgQueueHandle, &ltv_UartMsg, portMAX_DELAY);

        /*串口号不匹配，不做处理*/
        if(ltv_UartMsg.mcv_UartPort > gtv_DeviceConfigTable.mtv_UartPort.mcv_SupportUartNum) {
            continue;
        }

        /*数据处理*/
        uart_task_handle_receive_data(&ltv_UartMsg);
    }
}
#else  
static void handle_uart_recv(uint8_t *pBuf, uint16_t len)
{
    //LOGD("mb", "Enter %s\r\n", __func__);
    if (memcmp(pBuf, "resetme", 7) == 0)
    {
        NVIC_SystemReset();
    }
}

/**
  * @brief  串口数据处理
  * @param  None
  * @retval None
  */
void uart_task_handle_receive_data(uart_msg_st *ltp_UartMsg)
{
    PRINTF("Enter %s, mode = %d\r\n", __func__, gtp_UartPort[ltp_UartMsg->mcv_UartPort].mcv_Mode);
    md_slave_msg_pack ltv_UartMbSlaveMsg = {0x00, };

    switch(gtp_UartPort[ltp_UartMsg->mcv_UartPort].mcv_Mode) {
        case UART_TYPE_FREE_PORT:

            break;

        case UART_TYPE_MB_SLAVE:
            /*申请发送缓存区*/
            ltv_UartMbSlaveMsg.mcp_RespBuff = ring_buffer_get_write_mem(&gtp_UartPort[ltp_UartMsg->mcv_UartPort].mtv_SendBuff);
            configASSERT(ltv_UartMbSlaveMsg.mcp_RespBuff != NULL);
            
            /*切换发送缓冲区，为下次消息准备*/
            ring_buffer_switch_write_mem(&gtp_UartPort[ltp_UartMsg->mcv_UartPort].mtv_SendBuff);

            ltv_UartMbSlaveMsg.msv_ReceiveLen = ltp_UartMsg->msv_MsgLength;
            ltv_UartMbSlaveMsg.mcp_ReceiveBuff = ltp_UartMsg->mcp_DataBuff;
       
        #if 0
            LOGI("uart_task","msv_ReceiveLen = %d", ltv_UartMbSlaveMsg.msv_ReceiveLen);
            if (ltv_UartMbSlaveMsg.msv_ReceiveLen > 64)
            {
                hexdump(ltv_UartMbSlaveMsg.mcp_ReceiveBuff, 64);
            }
            else
            {
                hexdump(ltv_UartMbSlaveMsg.mcp_ReceiveBuff, ltv_UartMbSlaveMsg.msv_ReceiveLen);
            }
        #endif
            
            /*标定信息发送者*/
            ltv_UartMbSlaveMsg.mcv_Sender = ltp_UartMsg->mcv_UartPort;

            /*Modbus诊断校验信息统计*/
            gtp_ModbusSlaveDiagInfo[ltp_UartMsg->mcv_UartPort].msv_BusPackageCnt += 1;

            if(ltv_UartMbSlaveMsg.mcp_ReceiveBuff[0] == 0x00) {
                ltv_UartMbSlaveMsg.mcv_IsBroadcastInfo = 1;
            }
            
            //PRINTF("mcv_SlaveId = %d\r\n", gtp_ModbusSlaveDiagInfo[ltp_UartMsg->mcv_UartPort].mcv_SlaveId);
            //handle_uart_recv(ltv_UartMbSlaveMsg.mcp_ReceiveBuff, ltv_UartMbSlaveMsg.msv_ReceiveLen);

            /*广播消息或者发送至本站消息*/
            if((ltv_UartMbSlaveMsg.mcp_ReceiveBuff[0] == gtp_ModbusSlaveDiagInfo[ltp_UartMsg->mcv_UartPort].mcv_SlaveId)
               || ltv_UartMbSlaveMsg.mcv_IsBroadcastInfo) {
                ltv_UartMbSlaveMsg.slaveID = gtp_ModbusSlaveDiagInfo[ltp_UartMsg->mcv_UartPort].mcv_SlaveId;

                gtp_ModbusSlaveDiagInfo[ltp_UartMsg->mcv_UartPort].msv_SlavePackageCnt += 1;

                ltv_UartMbSlaveMsg.resp_func = gtp_UartPort[ltp_UartMsg->mcv_UartPort].pSendFunc;

                mb_slave_msg_handler(&ltv_UartMbSlaveMsg);
            }
            break;

        case UART_TYPE_MB_MASTER:
            break;
    }
}

#if 0
static void test_delay_count(uint32_t count)
{
    while(count--);
}

void skalyke_delay_20_ns(void)
{
    __asm("NOP");//1
    __asm("NOP");//2
    __asm("NOP");//3
#if 0
    __asm("NOP");//4
    __asm("NOP");//5
    __asm("NOP");//6
    __asm("NOP");//7
    __asm("NOP");//8
    __asm("NOP");//9

    __asm("NOP");//10
    __asm("NOP");//11
    __asm("NOP");//12
    __asm("NOP");//13
    __asm("NOP");//14
    __asm("NOP");//15
    __asm("NOP");//16
    __asm("NOP");//17
    __asm("NOP");//18

    __asm("NOP");//19
    __asm("NOP");//20
    __asm("NOP");//21
    __asm("NOP");//22
    __asm("NOP");//23
    __asm("NOP");//24
#endif
}

uint32_t gNS;
void test_tick(void)
{
    static uint32_t val = 0;
#if 1
    uint32_t beginTick, endTick;
    //uint32_t valBak;;
    float interval, interval2;
    gNS = val;
    //valBak = val;
    //cpu_tick_start_count();
    beginTick = SysTick->VAL;
#if 0
    kalyke_delay_count(val);
#elif 1
    kalyke_delay_ns2(225);
#elif 0
    for (uint32_t i = 0; i < val; ++i)
    {
        __asm("NOP"); /* delay */
    }
#elif 0
    while (gNS--);
#elif 0
    while (valBak--);
#elif 0
    __asm("NOP");//2 ticks, 3.3ns
#if 0
    __asm("NOP");//3 ticks
    __asm("NOP");//3 ticks, 5ns
    __asm("NOP");//4 ticks
    __asm("NOP");//4 ticks, 6.6ns
    __asm("NOP");//5 ticks
    __asm("NOP");//5 ticks, 8.3ns
    __asm("NOP");//6 ticks
    __asm("NOP");//6 ticks, 10ns
    __asm("NOP");//7 ticks
    __asm("NOP");//7 ticks, 11.6ns
    __asm("NOP");//8 ticks
    __asm("NOP");//8 ticks, 13.3ns
#endif
#elif 0
    skalyke_delay_20_ns();
#elif 0
    test_delay_count(val);
#elif 0
    delay();
#endif
    //interval = cpu_tick_stop_count();
    endTick = SysTick->VAL;
    interval = (beginTick - endTick) * 1.6666667f;
    interval2 = (beginTick - endTick) * 5.0 / 3.0;
    LOGW("test", "Delay %u need %f(ns), endTick = %u, beginTick = %u, tick:%u", 225, interval, endTick, beginTick, (beginTick - endTick));
    LOGD("test", "interval2 = %f(ns)", interval2);
    val++;
    if (interval2 > 1000000)
    {
        val = 0;
    }
#endif
}

static void test_ns_tick_task(void *p_arg)
{
    while(1)
    {
        test_tick();
        vTaskDelay(5000);
    }
}

static void test_ns_tick(void)
{
    xTaskCreate((TaskFunction_t)test_ns_tick_task,
                (const char *)"test_tick",
                512,
                (void *)NULL,
                2,
                NULL);
}
#endif
TaskHandle_t gGPTHandle;

#define GPT_IRQ_ID      GPT1_IRQn
#define EXAMPLE_GPT     GPT1
/* Get source clock for GPT driver (GPT prescaler = 0) */
#define EXAMPLE_GPT_CLK_FREQ CLOCK_GetFreq(kCLOCK_PerClk)

static volatile bool gptIsrFlag = false;
static void GPT1_IRQHandler(void)
{
    if (GPT_GetStatusFlags(EXAMPLE_GPT, kGPT_OutputCompare1Flag))
    {
        //LOGE("gpt", "timer1");
        /* Clear interrupt flag.*/
        GPT_ClearStatusFlags(EXAMPLE_GPT, kGPT_OutputCompare1Flag);
        GPIO_PortToggle(Y0_GPIO, Y0_PIN_MASK);
        //vTaskNotifyGiveFromISR(gGPTHandle, NULL);
        xTaskNotifyFromISR(gGPTHandle, kGPT_OutputCompare1Flag, eSetBits, NULL);
    }
    if (GPT_GetStatusFlags(EXAMPLE_GPT, kGPT_OutputCompare2Flag))
    {
        //LOGE("gpt", "timer2");
        GPT_ClearStatusFlags(EXAMPLE_GPT, kGPT_OutputCompare2Flag);
        //GPT_SetOutputCompareValue(EXAMPLE_GPT, kGPT_OutputCompare_Channel1, 5000000);
        //GPIO_PortToggle(Y0_GPIO, Y0_PIN_MASK);
        //vTaskNotifyGiveFromISR(gGPTHandle, NULL);
        xTaskNotifyFromISR(gGPTHandle, kGPT_OutputCompare2Flag, eSetBits, NULL);
    }
    if (GPT_GetStatusFlags(EXAMPLE_GPT, kGPT_OutputCompare3Flag))
    {
        //LOGE("gpt", "timer3");
        GPT_ClearStatusFlags(EXAMPLE_GPT, kGPT_OutputCompare3Flag);
        //vTaskNotifyGiveFromISR(gGPTHandle, NULL);
        //GPT_StopTimer(EXAMPLE_GPT);
        xTaskNotifyFromISR(gGPTHandle, kGPT_OutputCompare3Flag, eSetBits, NULL);
        //GPT_StartTimer(EXAMPLE_GPT);
    }
    
    SDK_ISR_EXIT_BARRIER;
}

static void testGPT_task(void)
{
    LOGV("gpt", "testGPT_task RUN. Free heap size is %d bytes\r\n", xPortGetFreeHeapSize());
    xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_ENET_INIT_DONE_PLC, pdTRUE, pdFALSE, portMAX_DELAY);
    LOGD("TMR", "*********GPT TIMER EXAMPLE will START, but wait 4 second...*********");
    vTaskDelay(4000);
    
    uint32_t gptDivider = 1;
    uint32_t gptFreq;
    gpt_config_t gptConfig;
    GPT_GetDefaultConfig(&gptConfig);
    gptConfig.enableFreeRun = false;
    gptConfig.clockSource = kGPT_ClockSource_Periph;
    /* Initialize GPT module */
    GPT_Init(EXAMPLE_GPT, &gptConfig);

    /* Divide GPT clock source frequency by 3 inside GPT module */
    GPT_SetClockDivider(EXAMPLE_GPT, gptDivider);

    /* Get GPT clock frequency */
    gptFreq = EXAMPLE_GPT_CLK_FREQ;
    PRINTF("gptFreq = %u\r\n", gptFreq);//375000 Hz
    /* GPT frequency is divided by 3 inside module */
    gptFreq /= gptDivider;
    LOGE("gpt", "gptFreq = %u", gptFreq);//125000 Hz
    /* Set both GPT modules to 1 second duration */
    GPT_SetOutputCompareValue(EXAMPLE_GPT, kGPT_OutputCompare_Channel1, 4 * gptFreq);
    GPT_SetOutputCompareValue(EXAMPLE_GPT, kGPT_OutputCompare_Channel2, 2 * gptFreq);
    //GPT_SetOutputCompareValue(EXAMPLE_GPT, kGPT_OutputCompare_Channel3, 3 * gptFreq);

    /* Enable GPT Output Compare1 interrupt */
    GPT_EnableInterrupts(EXAMPLE_GPT, kGPT_OutputCompare1InterruptEnable);
    GPT_EnableInterrupts(EXAMPLE_GPT, kGPT_OutputCompare2InterruptEnable);
    //GPT_EnableInterrupts(EXAMPLE_GPT, kGPT_OutputCompare3InterruptEnable);

    /* Enable at the Interrupt */
    EnableIRQ(GPT_IRQ_ID);
    NVIC_SetPriority(GPT_IRQ_ID, 2);
    //uint32_t gptPriority = NVIC_GetPriority(GPT_IRQ_ID);
    //LOGD("gpt", "gptPriority = %u", gptPriority);
    GPT_StartTimer(EXAMPLE_GPT);
    uint32_t uFlag = 0;
    while(1)
    {
    #if 0
        /* Check whether occur interupt and toggle LED */
        if (true == gptIsrFlag)
        {
            LOGW("gpt", "GPT interrupt is occurred !");
            gptIsrFlag = false;
        }
    #elif 0
        //uint32_t ret = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        uint32_t value;
        uint32_t ret = xTaskNotifyWait(0, ULONG_MAX, &value, portMAX_DELAY);
        LOGW("gpt", "ulTaskNotifyTake ret = %u, value = 0x%X", ret, value);
        if (value & kGPT_OutputCompare1Flag)
        {
            if (uFlag == 4)
            {
                LOGI("gpt", "Change to 2s......");
                uFlag++;
                GPT_SetOutputCompareValue(EXAMPLE_GPT, kGPT_OutputCompare_Channel1, gptFreq * 2); 
            }
            else if (uFlag == 5)
            {
                LOGW("gpt", "GPT interrupt 2s is occurred !");
            }
            else
            {
                uFlag++;
                LOGW("gpt", "GPT interrupt 4s is occurred !");
            }
        }
        else if (value & kGPT_OutputCompare2Flag)
        {
            LOGW("gpt", "GPT interrupt 2s is occurred !");
        }
        else if (value & kGPT_OutputCompare3Flag)
        {
            LOGW("gpt", "GPT interrupt 3s is occurred !");
        }
    #else
        uint32_t value;
        uint32_t ret = xTaskNotifyWait(0, ULONG_MAX, &value, portMAX_DELAY);
        LOGW("gpt", "ulTaskNotifyTake ret = %u, value = 0x%X", ret, value);
        if (value & kGPT_OutputCompare1Flag)
        {
            LOGD("gpt", "GPT interrupt 'kGPT_OutputCompare1Flag' is occurred !");
        }
        if (value & kGPT_OutputCompare2Flag)
        {
            LOGD("gpt", "GPT interrupt 'kGPT_OutputCompare2Flag' is occurred !");
            //GPT_SetOutputCompareValue(EXAMPLE_GPT, kGPT_OutputCompare_Channel1, 5000000);
        }
        if (value & kGPT_OutputCompare3Flag)
        {
            LOGD("gpt", "GPT interrupt 'kGPT_OutputCompare3Flag' is occurred !");
            //GPT_SetOutputCompareValue(EXAMPLE_GPT, kGPT_OutputCompare_Channel1, 2000000);
        }
    #endif
    }
}

static void testGPT(void)
{
    xTaskCreate((TaskFunction_t)testGPT_task,
                (const char *)"testGPT_task",
                512,
                (void *)NULL,
                2,
                &gGPTHandle);
}

/**********************************TMR test********************************/
volatile bool qtmrIsrFlag = false;
TaskHandle_t gTMRHandle;
/* The QTMR instance/channel used for board */
#define BOARD_QTMR_BASEADDR         TMR3
#define BOARD_FIRST_QTMR_CHANNEL    kQTMR_Channel_0
#define BOARD_SECOND_QTMR_CHANNEL   kQTMR_Channel_1
#define QTMR_ClockCounterOutput     kQTMR_ClockCounter0Output

/* Interrupt number and interrupt handler for the QTMR instance used */
#define QTMR_IRQ_ID TMR3_IRQn
#define QTMR_IRQ_HANDLER TMR3_IRQHandler

/* Get source clock for QTMR driver */
#define QTMR_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_IpgClk) //150000000 Hz

static void QTMR_IRQ_HANDLER(void)
{
    /* Clear interrupt flag.*/
    QTMR_ClearStatusFlags(BOARD_QTMR_BASEADDR, BOARD_SECOND_QTMR_CHANNEL, kQTMR_CompareFlag);

    qtmrIsrFlag = true;
/* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
  exception return operation might vector to incorrect interrupt */
#if defined __CORTEX_M && (__CORTEX_M == 4U || __CORTEX_M == 7U)
    __DSB();
#endif
}

void testTMR_task(void *p_arg)
{
    LOGV("TMR", "%s RUN. Free heap size is %d bytes\r\n", __func__, xPortGetFreeHeapSize());
    
    LOGD("TMR", "QTMR_SOURCE_CLOCK = %u Hz", QTMR_SOURCE_CLOCK); // 150,000,000 Hz
    uint8_t i = 0;
    qtmr_config_t qtmrConfig;
    xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_ENET_INIT_DONE_PLC, pdTRUE, pdFALSE, portMAX_DELAY);
    LOGD("TMR", "*********QUADTIMER EXAMPLE will START, but wait 4s...*********");
    vTaskDelay(4000);

    /*
     * qtmrConfig.debugMode = kQTMR_RunNormalInDebug;
     * qtmrConfig.enableExternalForce = false;
     * qtmrConfig.enableMasterMode = false;
     * qtmrConfig.faultFilterCount = 0;
     * qtmrConfig.faultFilterPeriod = 0;
     * qtmrConfig.primarySource = kQTMR_ClockDivide_2;
     * qtmrConfig.secondarySource = kQTMR_Counter0InputPin;
     */
    QTMR_GetDefaultConfig(&qtmrConfig);
    /* Use IP bus clock div by 128 */
    qtmrConfig.primarySource = kQTMR_ClockDivide_128; //1171875 Hz

    LOGV("TMR", "****Timer use-case, 50 millisecond tick.****");
    QTMR_Init(BOARD_QTMR_BASEADDR, BOARD_SECOND_QTMR_CHANNEL, &qtmrConfig);

    //一个TMR最大记录55ms
    /* Set timer period to be 50 millisecond */
    uint16_t ticks = MSEC_TO_COUNT(50U, (QTMR_SOURCE_CLOCK / 128));
    LOGW("TMR", "ticks of 50ms = %u", ticks);
    QTMR_SetTimerPeriod(BOARD_QTMR_BASEADDR, BOARD_SECOND_QTMR_CHANNEL, ticks);

    /* Enable at the NVIC */
    EnableIRQ(QTMR_IRQ_ID);
    uint32_t gptPriority = NVIC_GetPriority(QTMR_IRQ_ID);
    LOGI("TMR", "gptPriority = %u", gptPriority);

    /* Enable timer compare interrupt */
    QTMR_EnableInterrupts(BOARD_QTMR_BASEADDR, BOARD_SECOND_QTMR_CHANNEL, kQTMR_CompareInterruptEnable);

    /* Start the second channel to count on rising edge of the primary source clock */
    LOGV("TMR", "Before QTMR_StartTimer(BOARD_QTMR_BASEADDR, BOARD_SECOND_QTMR_CHANNEL, kQTMR_PriSrcRiseEdge)");
    QTMR_StartTimer(BOARD_QTMR_BASEADDR, BOARD_SECOND_QTMR_CHANNEL, kQTMR_PriSrcRiseEdge);

    for (i = 0; i < 10; i++)
    {
        /* Check whether occur interupt */
        while (!(qtmrIsrFlag))
        {
        }
        LOGV("TMR", "Timer 50ms interrupt has occurred !");
        qtmrIsrFlag = false;
    }
    QTMR_Deinit(BOARD_QTMR_BASEADDR, BOARD_SECOND_QTMR_CHANNEL);

    LOGV("TMR", "****Chain Timer use-case, 10 second tick.****\n");
    /* Init the first channel to use the IP Bus clock div by 128 */
    qtmrConfig.primarySource = kQTMR_ClockDivide_128; //1171875 Hz
    QTMR_Init(BOARD_QTMR_BASEADDR, BOARD_FIRST_QTMR_CHANNEL, &qtmrConfig);

    /* Init the second channel to use output of the first channel as we are chaining the first channel and the second
     * channel */
    qtmrConfig.primarySource = QTMR_ClockCounterOutput;
    QTMR_Init(BOARD_QTMR_BASEADDR, BOARD_SECOND_QTMR_CHANNEL, &qtmrConfig);

    /* Set the first channel period to be 1 millisecond */
    ticks = MSEC_TO_COUNT(1U, (QTMR_SOURCE_CLOCK / 128));
    LOGE("TMR", "ticks of 1ms = %u", ticks);
    QTMR_SetTimerPeriod(BOARD_QTMR_BASEADDR, BOARD_FIRST_QTMR_CHANNEL, ticks);

    /* Set the second channel count which increases every millisecond, set compare event for 5 second */
    QTMR_SetTimerPeriod(BOARD_QTMR_BASEADDR, BOARD_SECOND_QTMR_CHANNEL, 5000);

    /* Enable the second channel compare interrupt */
    QTMR_EnableInterrupts(BOARD_QTMR_BASEADDR, BOARD_SECOND_QTMR_CHANNEL, kQTMR_CompareInterruptEnable);

    /* Start the second channel in cascase mode, chained to the first channel as set earlier via the primary source
     * selection */
    QTMR_StartTimer(BOARD_QTMR_BASEADDR, BOARD_SECOND_QTMR_CHANNEL, kQTMR_CascadeCount);

    /* Start the first channel to count on rising edge of the primary source clock */
    LOGV("TMR", "Before QTMR_StartTimer(BOARD_QTMR_BASEADDR, BOARD_FIRST_QTMR_CHANNEL, kQTMR_PriSrcRiseEdge)");
    QTMR_StartTimer(BOARD_QTMR_BASEADDR, BOARD_FIRST_QTMR_CHANNEL, kQTMR_PriSrcRiseEdge);

    for (i = 0; i < 5; i++)
    {
        /* Check whether occur interupt */
        while (!(qtmrIsrFlag))
        {
        }
        LOGV("TMR", "Timer 5s interrupt has occurred !");
        qtmrIsrFlag = false;
    }
    LOGV("TMR", "*********QUADTIMER EXAMPLE END.*********\r\n");
    while (1)
    {
        vTaskDelay(5000);
    }
}

static void testTMR(void)
{
    xTaskCreate((TaskFunction_t)testTMR_task,
                (const char *)"testTMR_task",
                768,
                (void *)NULL,
                2,
                &gTMRHandle);
}

void uart_task(void *p_arg)
{
    PRINTF("uart_task RUN. Free heap size is %d bytes\r\n", xPortGetFreeHeapSize());

    uart_msg_st ltv_UartMsg;
    //unsigned char i;

    /*串口结构体内存申请*/
    //gtp_UartPort = (uart_port_info_st *)pvPortMalloc(sizeof(uart_port_info_st)*gtv_DeviceConfigTable.mtv_UartPort.mcv_SupportUartNum);
    
    PRINTF("gtp_UartPort = 0x%08X, &gtp_UartPort = 0x%08X\r\n", gtp_UartPort, &gtp_UartPort);


    /*获取串口操作函数指针*/
    bsp_get_uart_configure_info(gtp_UartPort);

    /*创建消息队列*/
    gtv_UartTaskMsgQueueHandle = xQueueCreate(10, sizeof(uart_msg_st));
    configASSERT(gtv_UartTaskMsgQueueHandle != NULL);

    /*发送缓冲区初始化*/
    for(uint8_t i=0; i<gtv_DeviceConfigTable.mtv_UartPort.mcv_SupportUartNum; i++) {
        //ring_buffer_init(&gtp_UartPort[i].mtv_SendBuff, 1024);
        ring_buffer_init(&gtp_UartPort[i].mtv_SendBuff, i);
    }
    //test_ns_tick();
    //testGPT();
    //testTMR();
    while(1) {
        /*等待串口消息*/
        //LOGV("uart_task", "Wait uart message...");
        xQueueReceive(gtv_UartTaskMsgQueueHandle, &ltv_UartMsg, portMAX_DELAY);

        //PRINTF("ltv_UartMsg.mcv_UartPort = %d\r\n", ltv_UartMsg.mcv_UartPort);
        /*串口号不匹配，不做处理*/
        if(ltv_UartMsg.mcv_UartPort >= gtv_DeviceConfigTable.mtv_UartPort.mcv_SupportUartNum) {
            continue;
        }

        /*数据处理*/
        if (gTouChuan != 0xFF)
        {
            if (gTouChuan == ltv_UartMsg.mcv_UartPort)
            {
                MicroLink_touchuan_send_data_2_tcp(ltv_UartMsg.mcp_DataBuff, ltv_UartMsg.msv_MsgLength);
                continue;
            }
        }
        
        uart_task_handle_receive_data(&ltv_UartMsg);
    }
    //vTaskSuspend(NULL);
}

#endif
