/**
  ******************************************************************************
  * @file    kalyke_led_task.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   系统状态LED维护任务
  ******************************************************************************
  */
#include "fsl_debug_console.h"
#include "kalyke_led_task.h"
#include "plc_variable.h"
#include "bsp_led.h"
#include "bsp_iwdg.h"
#include "event_groups.h"
#include "kalyke_event.h"
#include "daisy_task.h"
#include "plc_netcfg.h"

/*------------------------------------------------------------------------------
*   LED TASK相关全局变量定义
*-----------------------------------------------------------------------------*/
TaskHandle_t gtv_LedTaskHandler;
static TaskHandle_t gLedStartTaskHandler;
static TaskHandle_t gLedDaisyTaskHandler;

#if 0 // TODO:

/**
  * @brief  usb_task
  * @param  None
  * @retval None
  * 停止状态：绿灯长亮
  * 运行状态：绿灯200ms闪亮
  * 停机错误停止：红灯长亮
  * 非停机错误运行态： 蓝灯200ms闪亮
  * 非停机错误停止态：蓝灯长亮
  */
void led_task(void *p_arg)
{
    /*bit0:红灯 bit1:绿灯 bit2：蓝灯*/
    unsigned char lcv_LedStatus = 0x00;

    while(1) {
        switch(gtv_PlcRunStatus.mcv_PlcCurrentStatus) {
            case PLC_STOP_STATUS:
            case PLC_STOP_TO_RUN_STATUS:
                if(gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.error_status_stop) {
                    /*停机错误停止,红灯长亮*/
                    if(!(lcv_LedStatus & 0x01))
                        SYS_RED_LED_ON;

                    if(lcv_LedStatus & 0x02)
                        SYS_GREEN_LED_OFF;

                    if(lcv_LedStatus & 0x04)
                        SYS_BLUE_LED_OFF;

                    lcv_LedStatus = 0x01;
                } else if(guv_NonStopError.msv_Error) {
                    /*非停机错误停止, 蓝灯长亮*/
                    if(lcv_LedStatus & 0x01)
                        SYS_RED_LED_OFF;
                    if(lcv_LedStatus & 0x02)
                        SYS_GREEN_LED_OFF;
                    if(!(lcv_LedStatus & 0x04))
                        SYS_BLUE_LED_ON;

                    lcv_LedStatus = 0x04;
                } else {
                    /*常规停机，绿灯长亮*/
                    if(lcv_LedStatus & 0x01)
                        SYS_RED_LED_OFF;
                    if(!(lcv_LedStatus & 0x02))
                        SYS_GREEN_LED_ON;
                    if(lcv_LedStatus & 0x04)
                        SYS_BLUE_LED_OFF;

                    lcv_LedStatus = 0x02;
                }
                break;

            case PLC_RUN_STATUS:
            case PLC_RUN_TO_STOP_STATUS:
                if(guv_NonStopError.msv_Error) {
                    /*非停机错误，蓝灯闪亮*/
                    if(lcv_LedStatus & 0x01)
                        SYS_RED_LED_OFF;
                    if(lcv_LedStatus & 0x02)
                        SYS_GREEN_LED_OFF;

                    if(lcv_LedStatus & 0x04) {
                        SYS_BLUE_LED_OFF;
                        lcv_LedStatus = 0x00;
                    } else {
                        SYS_BLUE_LED_ON;
                        lcv_LedStatus = 0x04;
                    }

                } else {
                    /*正常运行，绿灯闪亮*/
                    if(lcv_LedStatus & 0x01)
                        SYS_RED_LED_OFF;
                    if(lcv_LedStatus & 0x04)
                        SYS_BLUE_LED_OFF;

                    if(lcv_LedStatus & 0x02) {
                        SYS_GREEN_LED_OFF;
                        lcv_LedStatus = 0x00;
                    } else {
                        SYS_GREEN_LED_ON;
                        lcv_LedStatus = 0x02;
                    }
                }
                break;
        }

        vTaskDelay(200/portTICK_RATE_MS);
    }

}
#else

volatile bool g_pinIfSet = false;
void led_toggle_RUN_and_ERR(void)
{
    if (g_pinIfSet)
    {
        LED_ERR->DR_CLEAR = LED_ERR_PIN_MASK;
        LED_RUN->DR_CLEAR = LED_RUN_PIN_MASK;
        g_pinIfSet = false;
    }
    else
    {
        LED_ERR->DR_SET = LED_ERR_PIN_MASK;
        LED_RUN->DR_SET = LED_RUN_PIN_MASK;
        g_pinIfSet = true;
    }
}

#if 0
static void led_GPIO1_09_task(void *p_arg)
{
    for(;;)
    {
        bsp_open_led_4();
        vTaskDelay(99);
        bsp_close_led_4();
        vTaskDelay(1399);
    }
}
#else
// You can see: http://www.tandemmaster.org/morse_code.html
char ALPHA[] = { 'a', 'b', 'c', 'd', 'e',
                 'f', 'g', 'h', 'i', 'j',
                 'k', 'l', 'm', 'n', 'o',
                 'p', 'q', 'r', 's', 't',
                 'u', 'v', 'w', 'x', 'y', 'z',
                 '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
                 '?', '!', '.',
                 ',', ':', '-',
                 '+', '*', '\\',
                 '%', '=', '(',
                 ')',
                 '~',
                 '\'', '`',
                 '@', '&', '|',
                 ';', '/', '_',
                 '"', '$',
               };
/* The symbols !, $ and & are not defined inside the ITU recommendation on Morse code, but conventions for them exist.
 * See : https://en.wikipedia.org/wiki/Morse_code
 */
char *MORSE[] = { ".-", "-...", "-.-.", "-..", ".", // a, b, c, d, e
                  "..-.", "--.", "....", "..", ".---", //f, g, h, i, j
                  "-.-", ".-..", "--", "-.", "---", // k, l, m, n, o
                  ".--.", "--.-", ".-.", "...", "-",// p, q, r, s, t
                  "..-", "...-", ".--",  // u, v, w
                  "-..-", "-.--", "--..", //x, y, z
                  ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----.", "-----", // 1, 2, 3, 4, 5, 6, 7, 8, 9, 0

                  "..--..", "-.-.--", ".-.-.-", // '?', '!', '.'
                  "--..--", "---...", "-....-",// ',', ':', '-'
                  ".-.-.", "...-.", "-.-.-", // '+', '*', '\'
                  "---.-", "-...-", "-.--.",// '%', '=', '('
                  "-.--.-", // ')'
                  ".---..", // '~'
                  ".----.", "-..-.-", // ''', '`'
                  ".--.-.", ".-...", "--.-.-", // '@', '&', '|'
                  "-.-.-.", "-..-.", "..--.-", // ';', '/', '_'
                  ".-..-.", "...-..-", // '"', '$'
                };

#define DIT_LIGHTEN_TIME 200 // 点
#define DAH_LIGHTEN_TIME 600 // 划 3 * DIT_LIGHTEN_TIME
#define DIT_AND_DAH      DIT_LIGHTEN_TIME // 点和划之间停顿
#define LetterInterval   DAH_LIGHTEN_TIME // 字符之间的停顿
#define WordInterval     1400 // 单词之间的停顿 7 * DIT_LIGHTEN_TIME
#define NEXT_CIRCLE      4000
int gLedIdx = 0;
int gMorseIdx = -1;
int gALPHALen;
int gStringLen;
//char gLedShowString[] = "happy new year 2020!";
//char gLedShowString[] = "abcde";
//char gLedShowString[] = "abcdefghijklmnopqrstuvwxyz happy new year 2019!";
//char gLedShowString[] = "a b c d e f g h i j k l m n o p q r s t u v w x y z";
//char gLedShowString[] = "xyz";
char gLedShowString[] = "fexlink";

static void setup(void)
{
    gALPHALen = sizeof(ALPHA);
    gStringLen = strlen(gLedShowString);
}

static void doNext(void)
{
    gLedIdx++;
    if (gLedIdx >= gStringLen)   // 没有更多的字符了，等待几秒，进行下一轮儿
    {
        gLedIdx = 0;
        vTaskDelay(NEXT_CIRCLE);
    }
}

static char theMorse[32];
static void led_GPIO1_09_task(void *p_arg)
{
    LOGV("led_task", "led_GPIO1_09_task RUN. Free heap size is %d bytes\r\n", xPortGetFreeHeapSize());
    setup();
    while(1)
    {
        gMorseIdx = -1;
        if (gLedShowString[gLedIdx] != ' ')
        {
            if (gLedShowString[gLedIdx] >= 'a' && gLedShowString[gLedIdx] <= 'z')
            {
                gMorseIdx = gLedShowString[gLedIdx] - 'a';
                goto DO_WORK;
            }
            for (int i = 0; i < gALPHALen; i++)
            {
                if (ALPHA[i] == gLedShowString[gLedIdx])
                {
                    gMorseIdx = i;
                    break;
                }
            }
        }

DO_WORK:
        if (gMorseIdx == -1)   // We find ' ' --- blank
        {
            vTaskDelay(WordInterval);
            doNext();
            continue;
        }

        // We find next letter
        vTaskDelay(LetterInterval);

        // 显示一个字母
        #if 0
        String theMorse = String(MORSE[gMorseIdx]);
        int len = theMorse.length();
        #else
        memset(theMorse, 0, sizeof(theMorse));
        strcpy(theMorse, MORSE[gMorseIdx]);
        //LOGV("led_task", "theMorse = %s", theMorse);
        int len = strlen(theMorse);
        //LOGD("led_task", "len = %d", len);
        #endif
        for (int i = 0; i < len; i++)
        {
            //digitalWrite(LED_PIN, HIGH);
            bsp_open_led_mqtt();
            //if ('.' == theMorse.charAt(i))
            if ('.' == theMorse[i])
            {
                vTaskDelay(DIT_LIGHTEN_TIME);
            }
            else
            {
                vTaskDelay(DAH_LIGHTEN_TIME);
            }
            //digitalWrite(LED_PIN, LOW);
            bsp_close_led_mqtt();
            if (i >= (len - 1)) break;
            vTaskDelay(DIT_AND_DAH);
        }

        doNext();
    }
}

#endif

static void led_sys_init_task(void *p_arg)
{
    while(1)
    {
        bsp_toggle_all_led();
        vTaskDelay(800);
    }
}

static void led_daisy_task(void *p_arg)
{
    while(1)
    {
        daisy_handle_run_led();
        vTaskDelay(100);
    }
}

/**
  * @brief  led_task
  * @param  None
  * @retval None
  * 停止状态：绿灯长亮
  * 运行状态：绿灯200ms闪亮
  * 停机错误停止：红灯长亮
  * 非停机错误运行态： 蓝灯200ms闪亮
  * 非停机错误停止态：蓝灯长亮
  */
void led_task(void *p_arg)
{
    LOGV("led_task", "led_task RUN. Free heap size is %d bytes\r\n", xPortGetFreeHeapSize());

//    if (bsp_get_deviceID() == 0xFFFF)
//    {
//        xTaskCreate((TaskFunction_t)led_GPIO1_09_task,
//                (const char *)"led_GPIO",
//                128,
//                (void *)NULL,
//                (UBaseType_t )1,
//                NULL);
//    }
    xTaskCreate((TaskFunction_t)led_sys_init_task,
                (const char *)"led_init_task",
                512,
                (void *)NULL,
                (UBaseType_t )14,
                (TaskHandle_t *)&gLedStartTaskHandler);
    xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_ENET_INIT_DONE_LED, pdTRUE, pdFALSE, portMAX_DELAY);
    LOGV("led_task", "KALYKE_EVENT_ENET_INIT_DONE_LED happened, gLedStartTaskHandler = 0x%08X", gLedStartTaskHandler);
    if (gLedStartTaskHandler)
    {
        vTaskDelete(gLedStartTaskHandler);
        bsp_close_all_led();
    }

#if (DAISY_MASTER_FEATURE == 1)
    xTaskCreate((TaskFunction_t)led_daisy_task,
                (const char *)"led_daisy_task",
                (uint16_t)LED_TASK_STACK_SIZE,
                (void *)NULL,
                (UBaseType_t )LED_TASK_PRIO,
                (TaskHandle_t *)&gLedDaisyTaskHandler);
#endif

    while(1)
    {
#if 0
    #if (LOG_OPEN == 1)
        static uint32_t lastTick = 0;
        if (xTaskGetTickCount() - lastTick > 2000)
        {
            LOGV("led_task", "mcv_PlcCurrentStatus = %d, msv_Error = 0x%X, error_status_stop = 0x%X\r\n", gtv_PlcRunStatus.mcv_PlcCurrentStatus, 
                guv_NonStopError.msv_Error,
                gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.error_status_stop);
            lastTick = xTaskGetTickCount();
        }
    #endif
#endif
/*
    #if (DAISY_MASTER_FEATURE == 1)
        if (g_plc_netcfg.lan.ioExp == LAN_CONFIG_IO_EXP_FEXLINK)
        {
            if (gDaisyFSM != DAISY_FSM_LOOP)
            {
                if (guv_NonStopError.bit.extend_io_num_err)
                {
                    LED_RUN->DR_SET = LED_1_PIN_MASK;
                    bsp_toggle_led_ERR();
                }
                else if (guv_NonStopError.bit.extend_cfg_err)
                {
                    LED_RUN->DR_SET = LED_1_PIN_MASK;
                    bsp_toggle_led_ERR();
                }
                else if (guv_NonStopError.bit.extend_bus_err)
                {
                    LED_RUN->DR_SET = LED_1_PIN_MASK;
                    bsp_toggle_led_ERR();
                }
                else
                {
                    bsp_toggle_led_RUN();
                }
                goto DELAY_800MS;
            }
        }
    #endif
    #if (ETHERCAT_SOEM == 1)
        if (g_plc_netcfg.lan.ioExp == LAN_CONFIG_IO_EXP_ETHERCAT)
        {
            #if 1
            if (guv_NonStopError.bit.ecat_err)
            {
                LED_RUN->DR_SET = LED_1_PIN_MASK;
                bsp_toggle_led_ERR();
                goto DELAY_800MS;
            }
            #endif
        }
    #endif
*/
PLC_RUN_HERE:
        switch(gtv_PlcRunStatus.mcv_PlcCurrentStatus)
        {
            case PLC_STOP_STATUS:
            case PLC_STOP_TO_RUN_STATUS:
                if(gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.error_status_stop)
                {
                    /*停机错误停止,MCU_LED_ERR灯长亮*/
                    LED_ERR->DR_CLEAR = LED_ERR_PIN_MASK;
                    LED_RUN->DR_CLEAR = LED_RUN_PIN_MASK;
                    goto DELAY_1000MS;
                }
                else if(guv_NonStopError.msv_Error) 
                {
                    /*非停机错误停止, MCU_LED_ERR灯闪亮，RUN灯常亮*/
                    bsp_toggle_led_ERR();
                    LED_RUN->DR_CLEAR = LED_RUN_PIN_MASK;
                    goto DELAY_800MS;
                } 
                else 
                {
                    /*常规停机，MCU_LED_RUN灯长亮*/
                    LED_RUN->DR_CLEAR = LED_RUN_PIN_MASK;
                    LED_ERR->DR_SET = LED_ERR_PIN_MASK;
                    goto DELAY_1000MS;
                }
                //break;

            case PLC_RUN_STATUS:
            case PLC_RUN_TO_STOP_STATUS:
                if(guv_NonStopError.msv_Error)
                {
                    /*非停机错误，MCU_LED_ERR灯闪亮*/
                    led_toggle_RUN_and_ERR();
                } 
                else 
                {
                    /*正常运行，MCU_LED_RUN灯闪亮*/
                    bsp_toggle_led_RUN();
                }
                break;
        }
        vTaskDelay(201 / portTICK_PERIOD_MS);
        continue;
DELAY_800MS:
        vTaskDelay(801 / portTICK_PERIOD_MS);
        continue;
DELAY_1000MS:
        vTaskDelay(1001 / portTICK_PERIOD_MS);
    }
}
#endif
