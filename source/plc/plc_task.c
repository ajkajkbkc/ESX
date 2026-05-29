/**
  ******************************************************************************
  * @file    plc_task.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   创建系统运行中各任务
  ******************************************************************************
  */
#include "fsl_debug_console.h"
#include "plc_task.h"
#include "plc_sysinit.h"
#include "plc_executer.h"
#include "plc_variable.h"
#include "plc_element.h"
#include "bsp_led.h"
#include "bsp_iwdg.h"

#include "plc_internalmanage.h"
#include "timers.h"
#include "plc_executer.h"
#include "kalyke_version.h"
#include "plc_netcfg.h"
#include "kalyke_event.h"
#include "plc_sysblock.h"
#include "plc_errormsg.h"
#include "plc_interrupt.h"
#include "daisy_task.h"
#include "kalyke_modbus_com.h"
#include "bsp_uart.h"
#include "kalyke_uart_task.h"
#include "kalyke_collect_task.h"
#include "kalyke_oled_key_task.h"
/*------------------------------------------------------------------------------
*   PLC TASK相关全局变量定义
*-----------------------------------------------------------------------------*/

//static TimerHandle_t gPlcTimerHandle;
TaskHandle_t gtv_PlcTaskHandler;
void (*pFunc)(void);  /*定义一个函数指针*/

#if 0
static void timerCallback( TimerHandle_t xTimer )
{
    PRINTF("Enter %s()\r\n", __func__);
    plc_run_do();
}
#endif

void plc_run(void)
{
    #if 0
    PRINTF("Enter %s()\r\n", __func__);
    if (gPlcTimerHandle == NULL)
    {
        gPlcTimerHandle = xTimerCreate((const char *)"timer_plc",
                                (TickType_t  )4000 / portTICK_PERIOD_MS,
                                (UBaseType_t )pdFALSE,
                                (void *      )10,
                                (TimerCallbackFunction_t)timerCallback);
    }
    xTimerStart(gPlcTimerHandle, 100);
    #endif
}

void plc_re_run(void)
{
    #if 0
    xTimerReset(gPlcTimerHandle, 100);
    #endif
}

/**
  * @brief  plc_task
  * @param  None
  * @retval None
  */
void plc_task(void *p_arg)
{
    LOGW("plc_task", "plc_task RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());
    /*系统资源初始化*/
    plc_sys_init();
    plc_user_interrupt_init();
    plc_system_block_verify(1);
    guv_StopError.bit.sysblock_err = 0;	
    char ret = plc_parse_system_block(1);
    if (ret != pdPASS)
    {
        LOGE("plc_task", "plc_parse_system_block FAIL!");
        guv_StopError.bit.sysblock_err = 1;
        plc_refresh_error_msg(ERR_SYSTEM_BLOCK);
    }

    plc_netcfg_block_verify(1);
    ret = plc_parse_netcfg_block_NoUart();
    if (ret != pdPASS)
    {
        LOGE("plc_task", "plc_parse_netcfg_block_NoUart FAIL!");
        guv_NonStopError.bit.netconfig_err = 1;
        plc_refresh_error_msg(ERR_NET_CONFIG);
    }
    LOGW("plc_task", "Wait event: KALYKE_EVENT_ENET_INIT_DONE_PLC");
    xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_ENET_INIT_DONE_PLC, pdTRUE, pdFALSE, portMAX_DELAY);

    LOGD("plc_task", "g_plc_netcfg.lan.ioExp = %X", g_plc_netcfg.lan.ioExp);
#if (DAISY_MASTER_FEATURE == 1)
    start_daisy_task();
#endif
#if (KALYKE_MODBUS_TCP_SHEET == 1) && (KALYKE_FEATURE_INTERNET_TASK != 0)
    if (g_plc_netcfg.wan.ioExp != WAN_CONFIG_IO_EXP_FEXLINK) //智能设备网口不执行其他程序
    {
        start_modbus_tcp();
    }
#endif

    /*看门狗使能*/
    bsp_watch_dog_enable();
    pFunc = bsp_feed_watch_dog;
    LOGV("plc_task", "pFunc(bsp_feed_watch_dog) = 0x%08X\r\n", pFunc);
    while(1) {
        /*PLC运行态切换*/
        plc_sys_status_switch();
        /*喂狗*/
        bsp_feed_watch_dog();
        
        switch(gtv_PlcRunStatus.mcv_PlcCurrentStatus) {
            /*停止状态*/
            case PLC_STOP_STATUS:
                plc_status_stop();
                break;

            /*停止到运行*/
            case PLC_STOP_TO_RUN_STATUS:
                plc_status_stop_to_run();
                gtv_PlcRunStatus.mcv_PlcCurrentStatus = PLC_RUN_STATUS;
                break;

            /*运行状态*/
            case PLC_RUN_STATUS:
                //taskENTER_CRITICAL();
                plc_run_deamon();
                //taskEXIT_CRITICAL();
                break;

            /*运行到停止*/
            case PLC_RUN_TO_STOP_STATUS:
                plc_status_run_to_stop();
                break;
        }        
        /*喂狗*/
        bsp_feed_watch_dog();
#if 0
        if (gtv_PlcRunStatus.mcv_PlcCurrentStatus == PLC_STOP_STATUS)
        {
            vTaskDelay(100);
        }
        else
        {
            taskYIELD();
        }
#elif 1
        vTaskDelay(1);
#else
        taskYIELD();
#endif

    CONSTANT_SCAN_TAG:
        /*刷新强制元件*/
        plc_refresh_force_element_value();

        /*刷新系统I/O*/
        plc_refresh_io_port();

        /*刷新端口指示灯*/
        //bsp_refresh_io_port_led(gtv_PlcElement.msp_XElement[0], gtv_PlcElement.msp_YElement[0]);

        /*内务处理*/

        /* 刷新恒定扫描速率， 非恒定扫描时直接对plc_run_user_program函数计时 */
    #if (PLC_SCAN_TIME_LOGIC_2 == 1)
        if (plc_get_bit_element_value(SM_ELEMENT, SM8))
        {
            if(plc_refresh_sys_scan_time()) {
                /*恒定扫描*/
                goto CONSTANT_SCAN_TAG;
            }
        }
    #else
        if(plc_refresh_sys_scan_time())
        {
            /*恒定扫描*/
            goto CONSTANT_SCAN_TAG;
        }
    #endif
    }
}

