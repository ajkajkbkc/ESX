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

#include "lwip/apps/sntp.h"
#include "fsl_debug_console.h"
#include "kalyke_event.h"
#include "plc_task.h"
#include "kalyke_internet_task.h"
#include "fsl_snvs_hp.h"
#include "fsl_snvs_lp.h"
#include "kalyke_monitor_task.h"
#include "bsp.h"
#include "kalyke_DS1302.h"


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
TaskHandle_t gKalykeSNTPTaskHandle;

/*******************************************************************************
 * Code
 ******************************************************************************/

#if 0
void start_sntp(void)
{
    xTaskCreate((TaskFunction_t)kalyke_sntp_task,
                      (const char *)"kalyke_sntp_task",
                      2048,
                      (void *)NULL,
                      3,
                      (TaskHandle_t *)&gKalykeSNTPTaskHandle);
}
#endif
void kalyke_test_same_name_task(void *p_arg)
{
    uint32_t val = ((uint32_t)p_arg);
    while(1)
    {
        if (val == 1)
        {
            LOGV("sn_test", "val = %u.", val);
            vTaskDelay(3000 / portTICK_PERIOD_MS);
        }
        else
        {
            LOGI("sn_test", "val = %u.", val);
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }
}

static const char *TAG = "DS1302";
static void _ds1302_init(void)
{
    LOGV(TAG, "Enter %s()", __func__);
    DS1302_Init();
#if 0
    uint8_t buf[8] = {0};
    DS1302_ReadTime2(buf);
    LOGI(TAG, "DS1302 Time : %04u-%02u-%02u  %02u:%02u:%02u, week:%u", buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
    LOGD(TAG, "DS1302 Time : %x-%x-%x  %x:%x:%x, week:%u", buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
#endif
#if 0
    buf[1] = 21;
    buf[2] = 9;
    buf[3] = 17;
    buf[4] = 16;
    buf[5] = 37;
    buf[6] = 0;
    buf[7] = 5;
    DS1302_WriteTime(buf);
#endif
}

#if (ONLY_READ_TIME_FROM_DS1302 == 1)
void kalyke_get_ds1302_times(snvs_hp_rtc_datetime_t *times)
{
    LOGV(TAG, "Enter %s()", __func__);

    uint8_t buf[8] = {0};
    DS1302_ReadTimeBurst(buf);
    LOGW(TAG, "DS1302: %04u-%02u-%02u  %02u:%02u:%02u, week:%u\r\n", buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);

    times->year = buf[1] + 2000;
    times->month = buf[2];
    times->day = buf[3];
    times->hour = buf[4];
    times->minute = buf[5];
    times->second = buf[6] & 0x7F;
    LOGV(TAG, "Leave %s()", __func__);
}
#endif

void kalyke_Synch_ds1302_time(void)
{
    LOGV(TAG, "Enter %s()", __func__);

    uint8_t buf[8] = {0};
    DS1302_ReadTimeBurst(buf);
    LOGW(TAG, "DS1302: %04u-%02u-%02u  %02u:%02u:%02u, week:%u\r\n", buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);

    snvs_lp_srtc_datetime_t srtcDate;
    srtcDate.year = buf[1] + 2000;
    srtcDate.month = buf[2];
    srtcDate.day = buf[3];
    srtcDate.hour = buf[4];
    srtcDate.minute = buf[5];
    srtcDate.second = buf[6] % 60;
    if (buf[6] >= 60)
    {
        LOGE(TAG, "second > 60, so special control.");
        buf[6] = srtcDate.second;
        DS1302_WriteTimeBurst(buf);
    }
    LOGE(TAG, "DS1302: %04u-%02u-%02u  %02u:%02u:%02u, week:%u\r\n", srtcDate.year, srtcDate.month, srtcDate.day, srtcDate.hour, srtcDate.minute, srtcDate.second, buf[7]);
    if (buf[1] < 100)  //DS1302 不焊接时，数据为165
    {
        LOGD(TAG, "Let set RT1061 RTC...");
        SNVS_LP_SRTC_SetDatetime(SNVS, &srtcDate);
        SNVS_HP_RTC_TimeSynchronize(SNVS);
    }
}

void kalyke_ds1302_set_time(snvs_hp_rtc_datetime_t *g_rtcDate)
{
    LOGV(TAG, "Enter %s()", __func__);
    uint8_t buf[8] = {0};
    buf[1] = g_rtcDate->year - 2000;
    buf[2] = g_rtcDate->month;
    buf[3] = g_rtcDate->day;
    buf[4] = g_rtcDate->hour;
    buf[5] = g_rtcDate->minute;
    buf[6] = g_rtcDate->second;
    buf[7] = get_WeekDayNum(g_rtcDate->year, g_rtcDate->month, g_rtcDate->day);
    DS1302_WriteTime(buf);
}

void kalyke_ds1302_init(void)
{
    _ds1302_init();
    //vTaskDelay(100);
    kalyke_Synch_ds1302_time();
}

#if 1
#if (KALYKE_FEATURE_SNTP_TASK == 1)
void kalyke_sntp_task(void *p_arg)
{
    LOGV("Kalyke_SNTP", "kalyke_sntp_task RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());
    _ds1302_init();
    vTaskDelay(1001);
    kalyke_Synch_ds1302_time();
    vTaskDelay(1001);
    while (1)
    {
        vTaskDelay(86400000); // 24 * 60 * 60 * 1000 = 86,400,000
        //vTaskDelay(1800000); // 30 * 60 * 1000 = 1,800,000
        kalyke_Synch_ds1302_time();
    }
}
#endif
#else
void kalyke_sntp_task(void *p_arg)
{
    LOGV("Kalyke_SNTP", "kalyke_sntp_task RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());
    _ds1302_init();
    xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_GOT_IP_SNTP, pdTRUE, pdFALSE, portMAX_DELAY);
    LOGD("Kalyke_SNTP", "Let us do SNTP because we had got the IP address.");
    vTaskDelay(5222);

#if 1
    snvs_hp_rtc_datetime_t rtcDate;
    SNVS_HP_RTC_GetDatetime(SNVS, &rtcDate);
    LOGD("Kalyke_SNTP", "gBspIam1970 = %u, rtcDate.year = %u", gBspIam1970, rtcDate.year);
    if (gBspIam1970 == true) //此时说明RT1061的时间已丢失
    {
        kalyke_Synch_ds1302_time();
        goto DO_SNTP_GET;
    }
    if (rtcDate.year != 2020)
    {
        LOGW("Kalyke_SNTP", "rtcDate.year != 2020, so just return");
        vTaskDelete(NULL);
        return;
    }
#endif

DO_SNTP_GET:
    while (1)
    {
        //if (link_is_up(&phyHandle1))
        if (gWanOK == true)
        {
            break;
        }
        else
        {
            vTaskDelay(5222);
        }
    }
    //vTaskDelay(3000);
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    //sntp_setservername(0, "cn.pool.ntp.org");
    sntp_setservername(0, "pool.ntp.org");
    //sntp_setservername(0, "148.251.69.45");
    //sntp_setservername(0, "185.209.85.222");
    
    //ipaddr_aton("193.228.143.12", &ip_addr);
    //ipaddr_aton("94.130.49.186", &ip_addr);
    //sntp_setserver(0, &ip_addr);
    //sntp_servermode_dhcp(0);
#if 0
#if LWIP_DHCP
    //sntp_setserver(0, &ip_addr);
    sntp_setservername(0, "cn.pool.ntp.org");

    sntp_servermode_dhcp(1); /* get SNTP server via DHCP */
    

#else /* LWIP_DHCP */
#if LWIP_IPV4
    //sntp_setserver(0, netif_ip_gw4(netif_default));
    //sntp_setserver(0, &ip_addr);
    
    sntp_setservername(0, "cn.pool.ntp.org");
#endif /* LWIP_IPV4 */
#endif /* LWIP_DHCP */
#endif
    LOGD("Kalyke_SNTP", "Before call sntp_init()");
    plc_re_run();
    sntp_init();
    plc_re_run();
    LOGD("Kalyke_SNTP", "After call sntp_init()");
    print_vTaskList(__func__, __LINE__);
    LOGW("Kalyke_SNTP", "Free heap size is %d bytes", xPortGetFreeHeapSize());
    vTaskDelay(30000);
#if 0
    for(;;)
    {
        vTaskDelay(100000);
        LOGV("Kalyke_SNTP", "Running");
    }
#else
    LOGW("Kalyke_SNTP", "Delete myself");
    vTaskDelete(NULL);
#endif
}
#endif
