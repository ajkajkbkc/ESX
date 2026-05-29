
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "board.h"
#include "pin_mux.h"
#include "fsl_debug_console.h"
#include "kalyke_opts.h"
#include "kalyke_tool.h"
#include "kalyke_internet_task.h"
#include "kalyke_version.h"
#include "bsp.h"
#include "kalyke_sd_card_task.h"
#include "plc_task.h"
#include "pid_task.h"
#include "kalyke_led_task.h"
#include "kalyke_ota.h"
#include "kalyke_event.h"
#include "kalyke_uart_task.h"
#include "plc_interrupt.h"
#include "kalyke_monitor_task.h"
#include "kalyke_sntp_task.h"
#include "kalyke_ad_task.h"
#include "bsp_flash.h"
#include "bsp_dct.h"
#include "bsp_gpio.h"
#include "plsd_task.h"
#include "kalyke_4G_task.h"
#include "kalyke_usb_task.h"
#include "ether_cat_task.h"

#include "kalyke_oled_key_task.h"
#include "kalyke_collect_task.h"
/*------------------------------------------------------------------------------
*   开始任务相关全局变量定义
*-----------------------------------------------------------------------------*/
#define START_TASK_STACK_SIZE 2048
#define START_TASK_PRIO       3


/**
  * @brief  开始任务,创建系统运行各种资源
  * @param  None
  * @retval None
  */
void start_task(void *p_arg)
{
    BaseType_t ret;
    //taskENTER_CRITICAL();
    LOGV("main", "Enter %s(), p_arg = %u", __func__, p_arg);
#if (KALYKE_FEATURE_PLC_TASK == 1)
    bsp_flash_init();
    bsp_get_device_config_table();
#endif

/*创建 plc task*/
#if (KALYKE_FEATURE_PLC_TASK == 1)
    xTaskCreate((TaskFunction_t)plc_task,
                (const char *)"plc_task",
                (uint16_t)PLC_TASK_STACK_SIZE,
                (void *)NULL,
                (UBaseType_t )PLC_TASK_PRIO,
                (TaskHandle_t *)&gtv_PlcTaskHandler);
#endif               

/*创建 collect task*/                
#if (KALYKE_FEATURE_COLLECT_TASK == 1)                
    xTaskCreate((TaskFunction_t)collect_task,
                (const char *)"collect_task",
                (uint16_t)COLLECT_TASK_STACK_SIZE,
                (void *)NULL,
                (UBaseType_t )COLLECT_TASK_PRIO,
                (TaskHandle_t *)&gtv_CollectTaskHandler);
#endif

/*创建 PID task*/
#if (KALYKE_FEATURE_PID_TASK == 1)
    if (pid_open_check())
    {
        xTaskCreate((TaskFunction_t)pid_task,
                    (const char *)"pid_task",
                    (uint16_t)PID_TASK_STACK_SIZE,
                    (void *)NULL,
                    (UBaseType_t )PID_TASK_PRIO,
                    (TaskHandle_t *)&gPIDTaskHandler);
    }
#endif


#if (KALYKE_FEATURE_OTA == 1)
    xTaskCreate((TaskFunction_t)ota_task,
                (const char *)"OTA_task",
                1024,
                (void *)NULL,
                (UBaseType_t )INTERRUPT_TASK_PRIO + 1,
                NULL);
#endif

    /*创建用户中断程序处理 task*/
#if (KALYKE_FEATURE_PLC_INTERRUPT_TASK == 1)
    xTaskCreate((TaskFunction_t)interrupt_task,
                (const char *)"interrupt_task",
                (uint16_t)INTERRUPT_TASK_STACK_SIZE,
                (void *)NULL,
                (UBaseType_t )INTERRUPT_TASK_PRIO,
                (TaskHandle_t *)&gtv_InterruptTaskHandler);
#endif

    /*创建UART task*/
#if (KALYKE_FEATURE_UART_TASK == 1)
    xTaskCreate((TaskFunction_t)uart_task,
                (const char *)"uart_task",
                (uint16_t)UART_TASK_STACK_SIZE,
                (void *)NULL,
                (UBaseType_t )UART_TASK_PRIO,
                (TaskHandle_t *)&gtv_UartTaskHandler);
    
#endif

    /* USB task */
#if (KALYKE_FEATURE_USB == 1)
    xTaskCreate((TaskFunction_t)usb_task,
                (const char *)"usb_task",
                (uint16_t)USB_TASK_STACK_SIZE,
                (void *)NULL,
                (UBaseType_t )USB_TASK_PRIORITY,
                (TaskHandle_t *)&gUSBTaskHandler);
#endif

    /*Create sd card task*/
#if (KALYKE_FEATURE_SD_CARD_TASK == 1)
    xTaskCreate((TaskFunction_t)kalyke_sd_task,
                (const char *)"SD_task",
                SD_TASK_STACK_SIZE,
                (void *)NULL,
                (UBaseType_t )SD_TASK_PRIO,
                NULL);
#endif
    /*Ethernet task任务创建*/
#if (KALYKE_FEATURE_INTERNET_TASK == 1)
    ret = xTaskCreate((TaskFunction_t)kalyke_internet_task,
                      (const char *)"kalyke_internet_task",
                      INTERNET_TASK_STACK_SIZE,
                      (void *)NULL,
                      INTERNET_TASK_PRIO,
                      (TaskHandle_t *)&gKalykeInternetTaskHandle);
    if (ret != pdPASS)
    {
        LOGE("main", "Create kalyke_internet_task error!\r\n");
    }
#endif

#if (KALYKE_FEATURE_LED_TASK == 1)
    /*创建 led task*/
    ret = xTaskCreate((TaskFunction_t)led_task,
                (const char *)"led_task",
                (uint16_t)LED_TASK_STACK_SIZE,
                (void *)NULL,
                (UBaseType_t )LED_TASK_PRIO,
                (TaskHandle_t *)&gtv_LedTaskHandler);
    if (ret != pdPASS)
    {
        PRINTF("Create led task error!\r\n");
    }
#endif
#if (KALYKE_FEATURE_MONITOR_TASK == 1)
    // 创建动态内存监测task
    ret = xTaskCreate((TaskFunction_t)kalyke_monitor_task,
                      (const char *)"monitor_task",
                      MONITOR_TASK_STACK_SIZE,
                      (void *)NULL,
                      MONITOR_TASK_PRIO,
                      (TaskHandle_t *)&gKalykeMonitorTaskHandle);
    if (ret != pdPASS)
    {
        PRINTF("Create kalyke_monitor_task error!\r\n");
    }
#endif

#if (KALYKE_FEATURE_SNTP_TASK == 1)
    // Create SNTP task.
    ret = xTaskCreate((TaskFunction_t)kalyke_sntp_task,
                      (const char *)"kalyke_sntp_task",
                      256,
                      (void *)NULL,
                      3,
                      NULL);
    if (ret != pdPASS)
    {
        PRINTF("Create kalyke_sntp_task error!\r\n");
    }
#endif

#if (KALYKE_FEATURE_4G_TASK == 1)
    xTaskCreate((TaskFunction_t)kalyke_4G_task,
                (const char *)"4G_task",
                (uint16_t)TASK_4G_STACK_SIZE,
                (void *)NULL,
                (UBaseType_t )TASK_4G_PRIO,
                (TaskHandle_t *)&gKalyke4GTaskHandle);
#endif

/*创建掉电检测任务*/
#if (KALYKE_FEATURE_LOW_POWER_TASK == 1)
    //if(gtv_DeviceConfigTable.mcv_IsSupportPlsd)
    {
        xTaskCreate((TaskFunction_t)plsd_task,
                    (const char *)"plsd_task",
                    (uint16_t)PLSD_TASK_STACK_SIZE,
                    (void *)NULL,
                    (UBaseType_t )PLSD_TASK_PRIO,
                    (TaskHandle_t *)&gtv_PlsdTaskHandler);
    }
#endif

#if 0
    /* 创建flash测试task */
    ret = xTaskCreate((TaskFunction_t)flash_test_task,
                      (const char *)"flash_test_task",
                      (uint16_t)configMINIMAL_STACK_SIZE + 10,
                      (void *)NULL,
                      (UBaseType_t )tskIDLE_PRIORITY + 2,
                      (TaskHandle_t *)NULL);
    if (ret != pdPASS)
    {
        PRINTF("Create flash_test_task error!\r\n");
    }

    ret = xTaskCreate((TaskFunction_t)uart_test_task,
                      (const char *)"uart_test_task",
                      1024,
                      (void *)NULL,
                      4,
                      (TaskHandle_t *)NULL);
    if (ret != pdPASS)
    {
        PRINTF("Create uart_test_task error!\r\n");
    }
#endif
    /*创建USB通信Task*/
    // TODO:
#if 0
    xTaskCreate((TaskFunction_t)usb_task,
                (const char *)"usb task",
                (uint16_t)USB_TASK_STACK_SIZE,
                (void *)NULL,
                (UBaseType_t )USB_TASK_PRIO,
                (TaskHandle_t *)&gtv_UsbTaskHandler);
#endif

#if (KALYKE_FEATURE_OLED_KEY_TASK == 1)
    // 创建按键OLED显示task
    ret = xTaskCreate((TaskFunction_t)kalyke_oled_key_task,
                      (const char *)"oled_key_task",
                      OLED_KEY_TASK_STACK_SIZE,
                      (void *)NULL,
                      OLED_KEY_TASK_PRIO,
                      (TaskHandle_t *)&gKalykeOledKeyTaskHandle);
    if (ret != pdPASS)
    {
        PRINTF("Create kalyke_oled_key_task error!\r\n");
    }
#endif


    //PRINTF("gtv_InterruptTaskHandler=0x%x, gtv_UartTaskHandler=0x%x, gtv_PlcTaskHandler = 0x%x\r\n", gtv_InterruptTaskHandler, gtv_UartTaskHandler, gtv_PlcTaskHandler);
    //PRINTF("gtv_LedTaskHandler = 0x%x\r\n", gtv_LedTaskHandler);
    LOGV("main", "Leave %s(), gKalykeInternetTaskHandle = 0x%08X", __func__, gKalykeInternetTaskHandle);
    /*删除start task*/
    //vTaskDelete(NULL);

    //taskEXIT_CRITICAL();
}

/* Addresses for VECTOR_TABLE and VECTOR_RAM come from the linker file */
    extern uint32_t Image$$VECTOR_ROM$$Base[];
    extern uint32_t Image$$VECTOR_RAM$$Base[];
    extern uint32_t Image$$RW_m_data$$Base[];

#define __KALYKE_VECTOR_TABLE          Image$$VECTOR_ROM$$Base
#define __KALYKE_VECTOR_RAM            Image$$VECTOR_RAM$$Base
#define __RAM_VECTOR_TABLE_SIZE (((uint32_t)Image$$RW_m_data$$Base - (uint32_t)Image$$VECTOR_RAM$$Base))

#if 1
void SystemInitHook (void)
{
    uint32_t n;
    uint32_t ret;
    uint32_t irqMaskValue;

    irqMaskValue = DisableGlobalIRQ();
    if (SCB->VTOR != (uint32_t)__KALYKE_VECTOR_RAM)
    {
        /* Copy the vector table from ROM to RAM */
        for (n = 0; n < ((uint32_t)__RAM_VECTOR_TABLE_SIZE) / sizeof(uint32_t); n++)
        {
            __KALYKE_VECTOR_RAM[n] = __KALYKE_VECTOR_TABLE[n];
        }
        /* Point the VTOR to the position of vector table */
        SCB->VTOR = (uint32_t)__KALYKE_VECTOR_RAM;
    }
#if 0
    ret = __VECTOR_RAM[irq + 16];
    /* make sure the __KALYKE_VECTOR_RAM is noncachable */
    __KALYKE_VECTOR_RAM[irq + 16] = irqHandler;
#endif
    EnableGlobalIRQ(irqMaskValue);
    SDK_ISR_EXIT_BARRIER;
}
#endif

void logKalykeInterruptVectorRemap(void)
{
    LOGD("main", "Enter %s()", __func__);
    LOGI("main", "__RAM_VECTOR_TABLE_SIZE = %u", __RAM_VECTOR_TABLE_SIZE);
    LOGV("main", "__KALYKE_VECTOR_TABLE = 0x%08X, __KALYKE_VECTOR_RAM = 0x%08X", __KALYKE_VECTOR_TABLE, __KALYKE_VECTOR_RAM);
    LOGW("main", "SCB->VTOR = 0x%08X", SCB->VTOR);
}

/**
  * @brief  Main 函数
  * @param  None
  * @retval None
  */
int main(void)
{
    /*初始化系统运行必备资源*/
#if 1
    bsp_init_pre();
#endif
    logKalykeInterruptVectorRemap();
    /*按照设备配置表，初始化其他剩余资源*/
#if 1
    bsp_init_post();
#endif
    LOGE("main", "SCB->VTOR = 0x%08X", SCB->VTOR);
#if SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_SDK
    NVIC_SetPriority(LPUART4_IRQn, 6);
    void (*pFuncMain)(void *p_arg);
    pFuncMain = start_task;
    LOGV("main", "pFuncMain = 0x%08X\r\n", pFuncMain);
    bool aBoolTrue = true;
    bool aBoolFalse = false;
    LOGV("main", "aBoolTrue = %d, aBoolFalse = %d", aBoolTrue, aBoolFalse);
    uint32_t lpUart4Priority = NVIC_GetPriority(LPUART4_IRQn);
    LOGD("main", "lpUart4Priority = %d", lpUart4Priority);
    LOGW("main", "configPRIO_BITS = %u, configLIBRARY_LOWEST_INTERRUPT_PRIORITY = 0x%X", configPRIO_BITS, configLIBRARY_LOWEST_INTERRUPT_PRIORITY);
    LOGW("main", "configKERNEL_INTERRUPT_PRIORITY = %X, configMAX_SYSCALL_INTERRUPT_PRIORITY = %X", configKERNEL_INTERRUPT_PRIORITY, configMAX_SYSCALL_INTERRUPT_PRIORITY);
#endif
    //NVIC_SetPriority(LPUART1_IRQn, 5);
    LOGV("main", "Before create start_task. sizeof(double) = %d, sizeof(long)=%d, sizeof(float)=%d, sizeof(uint64_t)=%d", sizeof(double), sizeof(long), sizeof(float), sizeof(uint64_t));
    LOGD("main", "Power pin: %u, %u", GPIO_PinReadPadStatus(LOW_POWER_DETECT_GPIO, LOW_POWER_DETECT_PIN), GPIO_PinRead(LOW_POWER_DETECT_GPIO, LOW_POWER_DETECT_PIN));
    
    PRINTF("SW version: %s.....................RAND_MAX=0x%X\r\n", SW_VERSION, RAND_MAX);
    g_kalyke_event_group = xEventGroupCreate();
    LOGV("main", "g_kalyke_event_group = 0x%08X", g_kalyke_event_group);
    LOGV("main", "g_plc_netcfg.mqtt.isParsed = %d", g_plc_netcfg.mqtt.isParsed);
    NVIC_SetPriority(LPUART2_IRQn, 6);
    /*创建开始任务*/
#if 0
    xTaskCreate((TaskFunction_t)start_task,
                (const char *)"start task",
                START_TASK_STACK_SIZE,
                (void *)NULL,
                (UBaseType_t )START_TASK_PRIO,
                (TaskHandle_t *)NULL);
#else
    start_task((void*)1);
#endif
    LOGI("main", "Let us to do vTaskStartScheduler()...\r\n");
    vTaskStartScheduler();

    /* Will not get here unless a task calls vTaskEndScheduler ()*/
    configASSERT(0);
}

void assertHappened(const char *func, int line)
{
    
    LOGE("main", "Assert Happened At Function:%s, Line:%d\r\n", func, line);
    print_vTaskList(__func__, __LINE__);
    //vTaskDelay(100);
}

