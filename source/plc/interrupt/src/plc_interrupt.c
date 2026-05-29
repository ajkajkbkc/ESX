/**
  ******************************************************************************
  * @file    plc_interrupt.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   用户中断处理模块
  ******************************************************************************
  */

#include "fsl_debug_console.h"
#include "plc_variable.h"
#include "plc_commonfunc.h"
#include "plc_interrupt.h"
#include "plc_executer.h"

/*------------------------------------------------------------------------------
* 变量声明
*-----------------------------------------------------------------------------*/
#if 0
plc_interrupt_info_st *gtp_InterruptInfo = NULL;
#else
plc_interrupt_info_st *gtp_InterruptInfo = NULL;
plc_interrupt_info_st gInterruptInfo;

#endif
TaskHandle_t gtv_InterruptTaskHandler;
QueueHandle_t gtv_InterruptMsgQueueHandle;

/*------------------------------------------------------------------------------
* 用户中断处理函数定义
*-----------------------------------------------------------------------------*/
/**
  * @brief  用户中断系统初始化
  * @param  None
  * @retval None
  */
void plc_user_interrupt_init(void)
{
    LOGD("plc_interrupt", "Enter %s(), sizeof(plc_interrupt_info_st) = %d", __func__, sizeof(plc_interrupt_info_st));
#if 0
    if(gtp_InterruptInfo == NULL) {
        gtp_InterruptInfo = (plc_interrupt_info_st *)pvPortMalloc(sizeof(plc_interrupt_info_st));
        configASSERT(gtp_InterruptInfo != NULL);
    }
#else
    gtp_InterruptInfo = &gInterruptInfo;
#endif
    /*默认禁止用户中断*/
    gtp_InterruptInfo->mcv_IntEnable = 0;

    for(uint8_t i = 0; i < MAX_SYS_INT_NUM; i++) {
        gtp_InterruptInfo->mtv_IntSource[i].mcv_Priority = INT_PRI_LOW;
        /*禁止同一中断源多个未处理中断入队*/
        gtp_InterruptInfo->mtv_IntSource[i].mcv_IntFlag = 0x01;
        gtp_InterruptInfo->mtv_IntSource[i].mcv_Count = 0;
        gtp_InterruptInfo->mtv_IntSource[i].mcp_IntPara = NULL;
        gtp_InterruptInfo->mtv_IntSource[i].pIntFunc = plc_run_user_program;
    }

    /*重置中断队列*/

}

/**
  * @brief  用户中断使能、禁止
  * @param  None
  * @retval None
  */
void plc_user_interrupt_enable(unsigned char lcv_Enable)
{
    gtp_InterruptInfo->mcv_IntEnable = lcv_Enable;
}

/**
  * @brief  用户中断使能、禁止
  * @param  None
  * @retval None
  */
void plc_add_int_to_interrupt_queue(unsigned char lcv_IntNum, unsigned char lcv_IsFromISR)
{
    plc_interrupt_source_st *ltp_Interrupt;
    int_msg_st ltv_IntMsg;
    BaseType_t ltv_HigherPriorityTaskWoken = pdFALSE;
    LOGD("plc_interrupt", "Enter %s(), lcv_IntNum = %u, lcv_IsFromISR = %u, mcv_IntEnable = %u", __func__, lcv_IntNum, lcv_IsFromISR, gtp_InterruptInfo->mcv_IntEnable);
    if(!gtp_InterruptInfo->mcv_IntEnable) {
        return;
    }

    if(lcv_IntNum > MAX_SYS_INT_NUM) {
        return;
    }

    ltp_Interrupt = &gtp_InterruptInfo->mtv_IntSource[lcv_IntNum];
    LOGD("plc_interrupt", "mcv_IntFlag = %u", ltp_Interrupt->mcv_IntFlag);
    /*中断未使能*/
    if(!(ltp_Interrupt->mcv_IntFlag & 0x02)) {
        return;
    }

    /*不允许中断未处理完成，触发新中断*/
    if((ltp_Interrupt->mcv_IntFlag & 0x01) && ltp_Interrupt->mcv_Count) {
        return;
    }

    ltp_Interrupt->mcv_Count++;
    ltv_IntMsg.mcv_IntNum = lcv_IntNum;
    ltv_IntMsg.mvp_Data = NULL;

    if(lcv_IsFromISR) {
        if(ltp_Interrupt->mcv_Priority == INT_PRI_LOW) {
            xQueueSendToBackFromISR(gtv_InterruptMsgQueueHandle, &ltv_IntMsg, &ltv_HigherPriorityTaskWoken);
        } else {
            xQueueSendToFrontFromISR(gtv_InterruptMsgQueueHandle, &ltv_IntMsg, &ltv_HigherPriorityTaskWoken);
        }
        portEND_SWITCHING_ISR(ltv_HigherPriorityTaskWoken);
    } else {
        if(ltp_Interrupt->mcv_Priority == INT_PRI_LOW) {
            xQueueSendToBack(gtv_InterruptMsgQueueHandle, &ltv_IntMsg, 0);
        } else {
            xQueueSendToFront(gtv_InterruptMsgQueueHandle, &ltv_IntMsg, 0);
        }
    }
}

/**
  * @brief  中断任务
  * @param  None
  * @retval None
  */
void interrupt_task(void *p_arg)
{
    int_msg_st ltv_IntMsg;
    plc_interrupt_source_st *ltp_Interrupt;

    /*创建消息队列*/
    gtv_InterruptMsgQueueHandle = xQueueCreate(30, sizeof(int_msg_st));
    configASSERT(gtv_InterruptMsgQueueHandle != NULL);

    PRINTF("interrupt_task RUN. Free heap size is %d bytes\r\n", xPortGetFreeHeapSize());

    while(1) {
        /*等待串口消息*/
        xQueueReceive(gtv_InterruptMsgQueueHandle, &ltv_IntMsg, portMAX_DELAY);

        /*禁止中断状态，不做任何处理*/
        if(!gtp_InterruptInfo->mcv_IntEnable) {
            continue;
        }

        ltp_Interrupt = &gtp_InterruptInfo->mtv_IntSource[ltv_IntMsg.mcv_IntNum];

        if(!ltp_Interrupt->mcv_Count) {
            continue;
        }
        LOGW("plc_interrupt", "Let us handle interrupt %u", ltv_IntMsg.mcv_IntNum);
        /*中断处理*/
        taskENTER_CRITICAL();
        ltp_Interrupt->pIntFunc(ltp_Interrupt->mcp_IntPara, 2);
        taskEXIT_CRITICAL();

        ltp_Interrupt->mcv_Count-- ;
    }
}

