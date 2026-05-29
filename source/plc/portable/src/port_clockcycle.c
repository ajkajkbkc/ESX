/**
  ******************************************************************************
  * @file    port_clockcycle.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   定时中断
  ******************************************************************************
  */
#include "fsl_debug_console.h"
#include "daisy_uart_task.h"
#include "plc_variable.h"
#include "plc_internalmanage.h"
#include "plc_element.h"
#include "plc_interrupt.h"
#include "bsp_led.h"
#include "bsp_uart.h"

#include "main.h"

static unsigned short ssv_TimeIntSM66Value = 0;
static unsigned short ssv_TimeIntSM67Value = 0;
static unsigned short ssv_TimeIntSM68Value = 0;

extern volatile TickType_t xTickCount;
extern volatile TickType_t gUart3BeginTick;
extern volatile uint8_t gDaisyUart3Status;
/**
  * @brief  当前系统的时钟节拍位1ms,使用Free RTOS提供的回掉函数实现定时中断
  * @param  None
  * @retval None
  */
void vApplicationTickHook( void )
{
#if (KALYKE_FEATURE_PLC_INTERRUPT_TASK == 0)
    return;
#endif

#if 0
    static uint32_t he = 0;
    if (he++ % 1000 == 0)
    {
        PRINTF("vApplicationTickHook: %u, interrupt:%d\r\n", he, gtp_InterruptInfo->mcv_IntEnable);
        //bsp_toggle_blue_led();
    }
    //return;
#endif

#if (DAISY_UART3_WORKER_LOGIC == 1)
    if (gDaisyUart3Status == UART_RX)
    {
        if (xTickCount - gUart3BeginTick > 0)
        {
            uart3_callback_func(NULL);
        }
    }
#endif

    if(!IS_INTERRUPT_ENABLE()) {
        return;
    }

    if(gtv_PlcRunStatus.mcv_PlcCurrentStatus != PLC_RUN_STATUS) {
        return;
    }

    if(plc_get_bit_element_value(SM_ELEMENT, 66) && (GET_SD_ELEMENT_VALUE(66) > 0)) {
        ssv_TimeIntSM66Value++;

        if(ssv_TimeIntSM66Value > GET_SD_ELEMENT_VALUE(66)) {
            plc_add_int_to_interrupt_queue(TIME_0_INT, 1);
            ssv_TimeIntSM66Value = 0;
        }
    }

    if(plc_get_bit_element_value(SM_ELEMENT, 67) && (GET_SD_ELEMENT_VALUE(67) > 0)) {
        ssv_TimeIntSM67Value++;

        if(ssv_TimeIntSM67Value > GET_SD_ELEMENT_VALUE(67)) {
            plc_add_int_to_interrupt_queue(TIME_1_INT, 1);
            ssv_TimeIntSM67Value = 0;
        }
    }

    if(plc_get_bit_element_value(SM_ELEMENT, 68) && (GET_SD_ELEMENT_VALUE(68) > 0)) {
        ssv_TimeIntSM68Value++;

        if(ssv_TimeIntSM68Value > GET_SD_ELEMENT_VALUE(68)) {
            plc_add_int_to_interrupt_queue(TIME_2_INT, 1);
            ssv_TimeIntSM68Value = 0;
        }
    }
}

