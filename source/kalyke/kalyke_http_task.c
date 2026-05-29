/**
  ******************************************************************************
  * @file    kalyke_http_task.c
  * @author  pj
  * @version V0.0.1
  * @date    2024-04-01
  * @brief   http·¢ËÍ
  ******************************************************************************
  */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "event_groups.h"
#include "kalyke_event.h"

#include "kalyke_http_task.h"
#include "kalyke_4G_task.h"
#include "fsl_debug_console.h"
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/


/*******************************************************************************
 * Code
 ******************************************************************************/

#define LOGE_4G    LOGE
#define LOGW_4G    LOGW
#define LOGI_4G    LOGI
#define LOGD_4G    LOGD
#define LOGV_4G    LOGV

static void SetJsonformat(void)
{
    char *sendBuf = pvPortMalloc(512);
    while(1)
    {
        if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
        {
            LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR...00", __func__);
            vTaskDelay(1000);
            continue;
        }
        sendAT("AT+QHTTPCFG=\"contenttype\",4");
        
        int ret = waitResponse(5000, "OK", NULL, NULL, NULL, NULL);
        if (ret == 1)
        {
            LOGE_4G("4G_task", "SetJsonformat=OK!");
            vTaskDelay(1000);
            break;
        }
        xSemaphoreGive(g4GMutex);
        vTaskDelay(4000);
    }
    xSemaphoreGive(g4GMutex);
}

static void HTTP_url_buf(void)
{
    char *sendBuf = pvPortMalloc(512);
    while(1)
    {
        if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
        {
            LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR...00", __func__);
            vTaskDelay(1000);
            continue;
        }
        sprintf((char*)sendBuf,"https://qytt.sipac.gov.cn/api/yqfk/photovoltaics");
        sendAT(sendBuf);
        
        int Ret = waitResponse(5000, "OK", NULL, NULL, NULL, NULL);
        if (Ret == 1)
        {
            LOGE_4G("4G_task", "HTTP_url_buf=OK!");
            vTaskDelay(1000);
            break;
        }
        xSemaphoreGive(g4GMutex);
        vTaskDelay(4000);
    }
    xSemaphoreGive(g4GMutex);
}

static void AT_HTTP_url_buf(void)
{
    char *sendBuf = pvPortMalloc(512);
    char *sendBuf1 = pvPortMalloc(512);
    while(1)
    {
        if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
        {
            LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR...00", __func__);
            vTaskDelay(1000);
            continue;
        }
        
        sprintf((char*)sendBuf,"https://qytt.sipac.gov.cn/api/yqfk/photovoltaics");
        sprintf((char*)sendBuf1,"AT+QHTTPURL=%d,5",strlen(sendBuf));
        sendAT(sendBuf1);
        
        int Ret = waitResponse(5000, "CONNECT", NULL, NULL, NULL, NULL);
        if (Ret == 1)
        {
            LOGE_4G("4G_task", "AT_HTTP_url_buf=OK!");
            vTaskDelay(1000);
            break;
        }
        xSemaphoreGive(g4GMutex);
        vTaskDelay(4000);
    }
    xSemaphoreGive(g4GMutex);
}

static void HTTP_POST_buf(char *payload)
{
    char *sendBuf = pvPortMalloc(512);
    while(1)
    {
        if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
        {
            LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR...00", __func__);
            vTaskDelay(1000);
            continue;
        }
        sprintf((char*)sendBuf,payload);
        sendAT(sendBuf);
        
        int Ret = waitResponse(5000, "OK", NULL, NULL, NULL, NULL);
        if (Ret == 1)
        {
            LOGE_4G("4G_task", "HTTP_POST_buf=OK!");
            vTaskDelay(1000);
            break;
        }
        xSemaphoreGive(g4GMutex);
        vTaskDelay(4000);
    }
    xSemaphoreGive(g4GMutex);
}

static void AT_HTTP_POST_buf(char *payload)
{
    char *sendBuf = pvPortMalloc(512);
    char *sendBuf1 = pvPortMalloc(512);
    while(1)
    {
        if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
        {
            LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR...00", __func__);
            vTaskDelay(1000);
            continue;
        }
        
        sprintf((char*)sendBuf,payload);
        sprintf((char*)sendBuf1,"AT+QHTTPPOST=%d,10,10",strlen(sendBuf));
        sendAT(sendBuf1);
        
        int Ret = waitResponse(5000, "CONNECT", NULL, NULL, NULL, NULL);
        if (Ret == 1)
        {
            LOGE_4G("4G_task", "AT_HTTP_POST_buf=OK!");
            vTaskDelay(1000);
            break;
        }
        xSemaphoreGive(g4GMutex);
        vTaskDelay(4000);
    }
    xSemaphoreGive(g4GMutex);
}

static void AT_QHTTPREAD(void)
{
    char *sendBuf = pvPortMalloc(512);

    while(1)
    {
        if (xSemaphoreTake(g4GMutex, 2600/portTICK_PERIOD_MS) == pdFALSE)
        {
            LOGE_4G("4G_task", "%s : xSemaphoreTake ERROR...00", __func__);
            vTaskDelay(1000);
            continue;
        }

        sprintf((char*)sendBuf,"AT+QHTTPREAD=10");
        sendAT(sendBuf);
        
        int Ret = waitResponse(5000, "CONNECT", NULL, NULL, NULL, NULL);
        if (Ret == 1)
        {
            LOGE_4G("4G_task", "AT+QHTTPREAD=OK!");
            vTaskDelay(1000);
            break;
        }
        xSemaphoreGive(g4GMutex);
        vTaskDelay(4000);
    }
    xSemaphoreGive(g4GMutex);
}

void HTTP_POST(char *payload)
{	
    SetJsonformat();
    
    AT_HTTP_url_buf();
    
    HTTP_url_buf();
    
    AT_HTTP_POST_buf(payload);
    
    HTTP_POST_buf(payload);
    
    AT_QHTTPREAD();     
}	

