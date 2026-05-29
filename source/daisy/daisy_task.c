/**
  ******************************************************************************
  * @file    daisy_task.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2020-04-18
  * @brief
  ******************************************************************************
  */
#include <stdio.h>
#include "lwip/opt.h"

#include "lwip/api.h"
#include "lwip/apps/mqtt.h"
#include "lwip/dhcp.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/netifapi.h"
#include "lwip/prot/dhcp.h"
#include "lwip/tcpip.h"
#include "lwip/timeouts.h"
#include "netif/ethernet.h"
#include "enet_ethernetif.h"

//#include "ctype.h"

#include "board.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"
#include "fsl_pit.h"

#include "kalyke_opts.h"
#include "kalyke_event.h"

#include "daisy_task.h"
#include "fsl_debug_console.h"
#include "kalyke_version.h"
#include "kalyke_tool.h"
#include "kalyke_internet_task.h"
#include "kalyke_monitor_task.h"
#include "plc_sysblock.h"
#include "plc_errormsg.h"
#include "plc_variable.h"

#include "bsp.h"
#include "plc_element.h"
#include "bsp_dct.h"
#include "bsp_led.h"
#include "bsp_gpio.h"
#include "daisy_uart_task.h"
#include "plc_internalmanage.h"

TaskHandle_t gDaisyTaskHandle = NULL;
TaskHandle_t gDaisyWANTaskHandle = NULL;
TaskHandle_t gDaisyLANTaskHandle = NULL;

static struct pbuf *gWANpbuf = NULL;//WAN口收到的数据
static struct pbuf *gLANpbuf = NULL;//LAN口收到的数据

volatile master_run_info_st gMasterRunInfo;
smart_bfm_st gSmartBFM;

#if (DAISY_MASTER_FEATURE == 1)
#if (LOG_OPEN == 1)
/*
 * 0 = close
 * 1 = 以微秒为单位计算响应时间
 * 2 = 以毫秒为单位计算响应时间
 * 3 = 同时以毫秒、微秒计算响应时间
*/
#define DAISY_COMUNICATION_COUNT    3
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
typedef enum _DAISY_STATE
{
    DAISY_WAIT,
    DAISY_WORKING,
} daisy_state_e;

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define PHY_ADDRESS_1 (0x02U) /* Phy address of enet port 0. */
#define PHY_ADDRESS_2 (0x01U)

#if (DAISY_CONFIG_WHEN_LOOP == 1)
#define LOOP_CONFIG_FLAG_MOD    19
#endif
#define RESP_TIME_OUT           20 //(ms)
/*******************************************************************************
 * Variables
 ******************************************************************************/
static const char *TAG = "DaisyMaster";

static TimerHandle_t gPLCStartTimer;

static uint32_t gDaisyTick0;
static uint32_t gDaisyTickMS;

static struct netif gNetifWan;
static struct netif gNetifLan;
//static uint64_t gMcuID;

uint8_t gEthBroadcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
uint8_t gEthOther[6] = {0x0C, 0x1C, 0x2B, 0x3F, 0x4D, 0x51};

/* 本地Mac   */
static uint8_t gDaisyMacWAN[6] = {0x8C, 0xEC, 0x4B, 0xAF, 0x8D, 0x01};
static uint8_t gDaisyMacLAN[6] = {0x8C, 0xEC, 0x4B, 0xAF, 0x8D, 0x33};

static uint8_t gDaisySendBuffer[1600];
static uint8_t *gpLoopData;
struct pbuf *gpbufLoop;

uint8_t gDaisyLANRecvBuffer[512];

uint16_t gSlaveIDUpgrade = 0; //给哪个从站升级

#if (DAISY_CONFIG_WHEN_LOOP == 1)
static uint32_t gLoopConfigflag = 0;
#endif

static TimerHandle_t gDaisyBusTimer = NULL;


static void daisy_handle_lan_received_data(uint8_t *pData, uint16_t length);
static void daisy_handle_wan_received_data(uint8_t *pData, uint16_t length);

/*******************************************************************************
 * Code
 ******************************************************************************/
void kalyke_daisy_init(void)
{
    static bool justReboot = true;
    LOGV(TAG, "Enter %s(), g_plc_netcfg.lan.ioExp = %u", __func__, g_plc_netcfg.lan.ioExp);
    if (g_plc_netcfg.lan.ioExp != LAN_CONFIG_IO_EXP_FEXLINK)
    {
        LOGW(TAG, "g_plc_netcfg.lan.ioExp != LAN_CONFIG_IO_EXP_FEXLINK, just return.");
        return;
    }
    LOGI(TAG, "justReboot = %u, gPLCStartTimer = 0x%08X", justReboot, gPLCStartTimer);
    if (justReboot == false)
    {
        if (gPLCStartTimer != NULL)
        {
            xTimerStart(gPLCStartTimer, 0);
        }
        return;
    }
    justReboot = false;
}

void kalyke_daisy_stop(void)
{
    LOGV(TAG, "Enter %s()", __func__);
    SET_SD_ELEMENT_VALUE(SD234, 101);
}

static void daisy_plc_start(TimerHandle_t ltv_TimeHandle)
{
    LOGV(TAG, "Enter %s(), ltv_TimeHandle = 0x%08X", __func__, ltv_TimeHandle);
#if (PLC_RUN_WAIT_DAISY == 1)
    xEventGroupSetBits(g_kalyke_event_group, KALYKE_EVENT_PLC_TASK_WAIT_DAISY);
#endif
}

static void log_pbuf(struct pbuf *p)
{
    LOGD(TAG, "Enter %s(), p = 0x%08X", __func__, p);
    LOGV(TAG, "p->next = 0x%08X", p->next);
    LOGD(TAG, "p->payload = 0x%08X", p->payload);
    LOGV(TAG, "p->tot_len = %u, p->len = %u", p->tot_len, p->len);
    hexdump(p->payload, p->len);
    LOGD(TAG, "p->type_internal = 0x%X", p->type_internal);
    LOGD(TAG, "p->flags = 0x%X", p->flags);
    LOGV(TAG, "p->ref = %u", p->ref);
    LOGD(TAG, "p->if_idx = %u", p->if_idx);
}

static inline void calculate_resp_time(struct netif *netif)
{
    static uint32_t lastTick = 0;
#if (DAISY_COMUNICATION_COUNT == 2)
    uint32_t tick1 = xTaskGetTickCount();
    uint32_t interval = tick1 - gDaisyTick0;
    if (tick1 - lastTick > 2000)
    {
        LOGV(TAG, "Enter %s(), netif = 0x%08X, response interval = %u(ms)", __func__, netif, interval);
        lastTick = tick1;
    }

#elif (DAISY_COMUNICATION_COUNT == 1)
    uint32_t tickInterval;
    uint32_t tick1 = SysTick->VAL;
    if (gDaisyTick0 >= tick1)
    {
        tickInterval = gDaisyTick0 - tick1;
    }
    else
    {
        tickInterval = 600000 - tick1 + gDaisyTick0;
    }
    double us = tickInterval / 600.0;
    if(gDaisyFSM != DAISY_FSM_LOOP)
    {
        LOGI(TAG, "Enter %s(), netif = 0x%08X, response interval = %.1f(us), cpu ticks: %u", __func__, netif, us, tickInterval);
    }
    else
    {
        if (gtv_PlcElement.msp_SDElement[SD234] == 1999)
        {
            LOGI(TAG, "Enter %s(), netif = 0x%08X, response interval = %.1f(us), cpu ticks: %u", __func__, netif, us, tickInterval);
        }
    }
#elif (DAISY_COMUNICATION_COUNT == 3)
    uint32_t tick1 = SysTick->VAL;
    uint32_t tickMS = xTaskGetTickCount();
    uint32_t intervalMS = tickMS - gDaisyTickMS;
    uint32_t tickInterval;
    if (gDaisyTick0 >= tick1)
    {
        tickInterval = gDaisyTick0 - tick1;
    }
    else
    {
        tickInterval = 600000 - tick1 + gDaisyTick0;
    }
    double us = tickInterval / 600.0;
    if(gMasterRunInfo.daisy_status != MASTER_LOOP_SLAVE)
    {
        LOGI(TAG, "Enter %s(), netif = 0x%08X, response interval = %.1f(us) | %u(ms), cpu ticks: %u", __func__, netif, us, intervalMS, tickInterval);
    }
    else
    {
        //if (gtv_PlcElement.msp_SDElement[SD234] == 1999)
        if (tickMS - lastTick > 2000)
        {
            LOGI(TAG, "Enter %s(), netif = 0x%08X, response interval = %.1f(us) | %u(ms), cpu ticks: %u", __func__, netif, us, intervalMS, tickInterval);
            lastTick = tickMS;
        }
    }
#endif
}

//This funciton called in interrupt, so we can't call FreeRTOS API.
err_t daisy_ethernet_input(struct pbuf *p, struct netif *netif)
{
#if (LOG_OPEN == 1)
    if(g_plc_netcfg.wan.ioExp != WAN_CONFIG_IO_EXP_FEXLINK)

    {
        calculate_resp_time(netif);
    }
#endif

    /* points to packet payload, which starts with an Ethernet header */
    struct eth_hdr *ethhdr = (struct eth_hdr *)p->payload;
    if (ETHTYPE_ETHER_KALYKE != lwip_htons(ethhdr->type))
    {
        LOGE(TAG, "lwip_htons(ethhdr->type) = 0x%04X, ethhdr->type = 0x%04X", lwip_htons(ethhdr->type), ethhdr->type);
        pbuf_free(p);
        return ERR_OK;
    }

    if (gtv_PlcElement.msp_SDElement[SD234] == 1999 || (gMasterRunInfo.daisy_status != MASTER_LOOP_SLAVE))
    {
#if 0
        log_pbuf(p);

        LOGW(TAG, "ethernet_input: dest:%"X8_F":%"X8_F":%"X8_F":%"X8_F":%"X8_F":%"X8_F", src:%"X8_F":%"X8_F":%"X8_F":%"X8_F":%"X8_F":%"X8_F", type:%"X16_F"\n",
             (unsigned char)ethhdr->dest.addr[0], (unsigned char)ethhdr->dest.addr[1], (unsigned char)ethhdr->dest.addr[2],
             (unsigned char)ethhdr->dest.addr[3], (unsigned char)ethhdr->dest.addr[4], (unsigned char)ethhdr->dest.addr[5],
             (unsigned char)ethhdr->src.addr[0],  (unsigned char)ethhdr->src.addr[1],  (unsigned char)ethhdr->src.addr[2],
             (unsigned char)ethhdr->src.addr[3],  (unsigned char)ethhdr->src.addr[4],  (unsigned char)ethhdr->src.addr[5],
             lwip_htons(ethhdr->type));
#endif
    }

    if (netif == (&fsl_netif1)) //lan
    {
        gLANpbuf = p;
        daisy_handle_lan_received_data((uint8_t *)p->payload + SIZEOF_ETH_HDR, p->len - SIZEOF_ETH_HDR);
    }
    else //wan
    {
        gWANpbuf = p;
        daisy_handle_wan_received_data((uint8_t *)p->payload + SIZEOF_ETH_HDR, p->len - SIZEOF_ETH_HDR);
    }

    //pbuf_free(p);

    return ERR_OK;
}

/* 如果LAN口插着网线则返回true，否则返回false
   This function takes about 50us run time. */
static inline bool lan_is_up(void)
{
    bool ifUP;

    ifUP = link_is_up(&phyHandle2);

    if (ifUP)//LAN
    {
        return true;
    }
    return false;
}

/* Just for testing */
static void daisy_send(uint8_t *pBuf, uint16_t len, struct netif *src_netif, struct netif *dst_netif)
{
    LOGV(TAG, "Enter %s(), src_netif = 0x%08X, dst_netif = 0x%08X", __func__, src_netif, dst_netif);
    //hexdump(pBuf, len);

    struct pbuf *p = NULL;
    p = pbuf_alloc(PBUF_LINK, len, PBUF_RAM);
    //LOGD(TAG, "p = 0x%08X, p->payload = 0x%08X, p->len = %u", p, p->payload, p->len);
    if (p && p->payload)
    {
        memcpy(p->payload, pBuf, len);
    }
    log_pbuf(p);

    err_t ret = ethernet_output(src_netif, p, (struct eth_addr *)(src_netif->hwaddr), (struct eth_addr *)(dst_netif->hwaddr), ETHTYPE_ETHER_KALYKE);
    pbuf_free(p);

    LOGD(TAG, "Leave %s(), ret = %d", __func__, ret);
}

static inline void daisy_send_directly_by_wan(struct pbuf *p)
{
#if (LOG_OPEN == 1)
    //LOGV(TAG, "Enter %s()", __func__);
    //hexdump(p->payload, p->len);
#endif

    //err_t ret = ethernet_output_directly(&gNetifWan, p);
    ethernet_output_daisy(&fsl_netif0, p);

    //LOGD(TAG, "Leave %s(), ret = %d", __func__, ret);
}

static inline void daisy_send_directly_by_lan(struct pbuf *p)
{
#if (LOG_OPEN == 1)
    //LOGV(TAG, "Enter %s()", __func__);
    //hexdump(p->payload, p->len);
#endif

    //err_t ret = ethernet_output_directly(&gNetifLan, p);
    ethernet_output_daisy(&fsl_netif1, p);

#if 0
    if (lan_is_up() == false)
    {
        daisy_send_directly_by_wan(p);
    }
#endif
    //LOGD(TAG, "Leave %s(), ret = %d", __func__, ret);
}

/**
  * @brief  WAN口数据发送
  * @param  pBuf 数据
  * @param  len 长度
  * @retval None
  */
static void daisy_WAN_send(uint8_t *pBuf, uint16_t len)
{
    //LOGV(TAG, "Enter %s()", __func__);
    struct pbuf *p = pbuf_alloc(PBUF_LINK, len, PBUF_RAM);
    if (p && p->payload)
    {
        memcpy(p->payload, pBuf, len);
    }
    else
    {
        pbuf_free(p);
        LOGE(TAG, "%s: ERROR, p = 0x%08X, p->payload = 0x%08X", __func__, p, p->payload);
        return;
    }

#if (LOG_OPEN == 1)
    //log_pbuf(p);
    //hexdump(p->payload, p->len);
#endif

    err_t ret = ethernet_output(&fsl_netif0, p, (struct eth_addr *)(fsl_netif0.hwaddr), (struct eth_addr *)(gEthBroadcast), ETHTYPE_ETHER_KALYKE);
    pbuf_free(p);

    //LOGD(TAG, "Leave %s(), ret = %d", __func__, ret);
}


/**
  * @brief  LAN口数据发送
  * @param  pBuf 数据
  * @param  len 长度
  * @retval None
  */
static void daisy_LAN_send(uint8_t *pBuf, uint16_t len)
{
    //LOGV(TAG, "Enter %s()", __func__);
    struct pbuf *p = pbuf_alloc(PBUF_LINK, len, PBUF_RAM);
    if (p && p->payload)
    {
        memcpy(p->payload, pBuf, len);
    }
    else
    {
        pbuf_free(p);
        LOGE(TAG, "%s: ERROR, p = 0x%08X, p->payload = 0x%08X", __func__, p, p->payload);
        return;
    }

#if (LOG_OPEN == 1)

    //log_pbuf(p);
    //hexdump(p->payload, p->len);

#if (DAISY_COMUNICATION_COUNT == 2)
    gDaisyTick0 = xTaskGetTickCount();
#elif (DAISY_COMUNICATION_COUNT == 1)
    gDaisyTick0 = SysTick->VAL;
#elif (DAISY_COMUNICATION_COUNT == 3)
    gDaisyTick0 = SysTick->VAL;
    gDaisyTickMS = xTaskGetTickCount();
#endif

#endif

    err_t ret = ethernet_output(&fsl_netif1, p, (struct eth_addr *)(fsl_netif1.hwaddr), (struct eth_addr *)(gEthBroadcast), ETHTYPE_ETHER_KALYKE);
    pbuf_free(p);

    //LOGD(TAG, "Leave %s(), ret = %d", __func__, ret);
}

void daisy_LAN_send_bin(uint8_t *pBuf, uint16_t len)
{
    //LOGV(TAG, "Enter %s()", __func__);
    struct pbuf *p = pbuf_alloc(PBUF_LINK, len + 4, PBUF_RAM);
    if (p && p->payload)
    {
        uint16_t *pData = (uint16_t *)p->payload;
        *pData++ = DAISY_CMD_8888;
        *pData++ = gSlaveIDUpgrade;
        memcpy(pData, pBuf, len + 4);
    }
    else
    {
        pbuf_free(p);
        LOGE(TAG, "%s: ERROR, p = 0x%08X, p->payload = 0x%08X", __func__, p, p->payload);
        return;
    }
    //log_pbuf(p);
#if (DAISY_COMUNICATION_COUNT == 2)
    gDaisyTick0 = xTaskGetTickCount();
#elif (DAISY_COMUNICATION_COUNT == 1)
    gDaisyTick0 = SysTick->VAL;
#elif (DAISY_COMUNICATION_COUNT == 3)
    gDaisyTick0 = SysTick->VAL;
    gDaisyTickMS = xTaskGetTickCount();
#endif
    err_t ret = ethernet_output(&fsl_netif1, p, (struct eth_addr *)(fsl_netif1.hwaddr), (struct eth_addr *)(gEthBroadcast), ETHTYPE_ETHER_KALYKE);
    pbuf_free(p);

    //LOGD(TAG, "Leave %s(), ret = %d", __func__, ret);
}

static void test_speed(uint16_t length)
{
    LOGD(TAG, "Enter %s(), length = %u", __func__, length);
    memset(gDaisySendBuffer, 0, sizeof(gDaisySendBuffer));
    uint16_t cnt;
    if (length % 100 == 0)
    {
        cnt = length / 100;
    }
    else
    {
        cnt = length / 100;
        cnt++;
    }
    for (uint16_t i = 0; i < cnt; i++)
    {
        strcpy((char *)gDaisySendBuffer + i * 100, "1234567890_Nice to meet you!!!!0123456789.&&&&((((1234567890_Nice to meet you!!!!0123456789.&&&&(((("); //100 bytes
    }
    daisy_LAN_send(gDaisySendBuffer, length);
    //sprintf((char *)gDaisySendBuffer, "1234567890_Nice to meet you!!!!0123456789.0x%08X", gKalykeSecondTickCurrent);
    //sprintf((char *)gDaisySendBuffer, "1234567890_Nice to meet you!!!!0123456789.&&&&"); //46 bytes
    //daisy_LAN_send(gDaisySendBuffer, strlen((char *)gDaisySendBuffer));
}

/**
  * @brief  MyStudio获取扫描信息
  * @param  pMsg 数据
  * @retval None
  */
void daisy_get_info(md_slave_msg_pack *pMsg)
{
    LOGV(TAG, "Enter %s(), pMsg = 0x%08X", __func__, pMsg);
    uint16_t module_num;
    uint16_t buff_index;
    int i, j;

    buff_index = 0;
    for(i = 0; i < 3; i++)
    {
        pMsg->mcp_RespBuff[buff_index++] = pMsg->mcp_ReceiveBuff[i];
    }

    //模块总数
    module_num = 1 + gMasterRunInfo.scan_ext_num; //主站+主站下扩展 数量
    for(i = 0; i < gMasterRunInfo.scan_slave_num; i++) //每个从站
    {
        module_num += gMasterRunInfo.scan_slave_ext_id_version[i].num; //从站+从站下扩展 数量
    }
    pMsg->mcp_RespBuff[buff_index++] = (uint8_t)(module_num & 0xFF);
    pMsg->mcp_RespBuff[buff_index++] = (uint8_t)(module_num >> 8);

    //各模块ID和版本号
    pMsg->mcp_RespBuff[buff_index++] = (uint8_t)(gtv_DeviceConfigTable.mtv_DevInfo.mlv_DeviceTypeId & 0xFF); //主站id
    pMsg->mcp_RespBuff[buff_index++] = (uint8_t)(gtv_DeviceConfigTable.mtv_DevInfo.mlv_DeviceTypeId >> 8);
    pMsg->mcp_RespBuff[buff_index++] = (uint8_t)(SW_VERSION_ID); //主站version，用0填充
    pMsg->mcp_RespBuff[buff_index++] = (uint8_t)(SW_VERSION_ID >> 8);
    for(i = 0; i < gMasterRunInfo.scan_ext_num; i++)
    {
        pMsg->mcp_RespBuff[buff_index++] = (uint8_t)(gMasterRunInfo.scan_ext_id_version[i] >> 16);
        pMsg->mcp_RespBuff[buff_index++] = (uint8_t)(gMasterRunInfo.scan_ext_id_version[i] >> 24);
        pMsg->mcp_RespBuff[buff_index++] = (uint8_t)(gMasterRunInfo.scan_ext_id_version[i] & 0xFF);
        pMsg->mcp_RespBuff[buff_index++] = (uint8_t)(gMasterRunInfo.scan_ext_id_version[i] >> 8);
    }
    for(i = 0; i < gMasterRunInfo.scan_slave_num; i++)
    {
        for(j = 0; j < gMasterRunInfo.scan_slave_ext_id_version[i].num; j++)
        {
            pMsg->mcp_RespBuff[buff_index++] = (uint8_t)(gMasterRunInfo.scan_slave_ext_id_version[i].pIdList[j] >> 16);
            pMsg->mcp_RespBuff[buff_index++] = (uint8_t)(gMasterRunInfo.scan_slave_ext_id_version[i].pIdList[j] >> 24);
            pMsg->mcp_RespBuff[buff_index++] = (uint8_t)(gMasterRunInfo.scan_slave_ext_id_version[i].pIdList[j] & 0xFF);
            pMsg->mcp_RespBuff[buff_index++] = (uint8_t)(gMasterRunInfo.scan_slave_ext_id_version[i].pIdList[j] >> 8);
        }
    }

    pMsg->msv_RespLen = buff_index;
    mb_slave_verify_resp_msg(pMsg);
    hexdump(pMsg->mcp_RespBuff, pMsg->msv_RespLen);
    LOGD(TAG, "Leave %s()", __func__);
}

/**
  * @brief  触发从站事件
  * @param  event 事件类型
  * @retval None
  */
static void daisy_set_event_slave_receive(void)
{
    BaseType_t xHigherPriorityTaskWoken, xResult;
    xHigherPriorityTaskWoken = pdFALSE;
    xResult                  = pdFAIL;

    xResult = xEventGroupSetBitsFromISR(g_kalyke_event_group, DAISY_EVENT_WAIT_SLAVE_RECEIVE, &xHigherPriorityTaskWoken);
    if (xResult != pdFAIL)
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    else
    {
        LOGE(TAG, "daisy_set_event_slave_receive return %d.", xResult);
    }
}

/**
  * @brief  等待从站事件
  * @param  timeout 超时时间
  * @retval 1：事件出发
            0：等待超时
  */
static bool daisy_wait_event_slave_receive(uint32_t timeout)
{
    EventBits_t uxBits;

    uxBits = xEventGroupWaitBits(g_kalyke_event_group, DAISY_EVENT_WAIT_SLAVE_RECEIVE, pdTRUE, pdFALSE, timeout);
    if (( uxBits & DAISY_EVENT_WAIT_SLAVE_RECEIVE ) != 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
  * @brief  设置以太网总线错误
  * @param  errCode 错误码
  * @param  errAddr 模块地址
  * @param  errContent 错误内容
  * @retval None
  */
static void daisy_set_error(uint16_t errCode, uint16_t errAddr,  uint16_t errContent)
{
    LOGV(TAG, "Enter %s", __func__);
    LOGE(TAG, "errCode: 0x%04X, errAddr: 0x%04X, errContent: 0x%04X", errCode, errAddr, errContent);

    gMasterRunInfo.daisy_err_flag = 1;

    SET_SD_ELEMENT_VALUE(SD243, errCode);
    saveErrorCodeFIFO(errCode);
    SET_SD_ELEMENT_VALUE(SD244, errAddr);
    SET_SD_ELEMENT_VALUE(SD245, errContent);
    plc_refresh_error_msg(ERR_SLAVE_ERR);
    guv_NonStopError.bit.extend_bus_err = 1;
}

/**
  * @brief  清除以太网总线错误
  * @param  None
  * @retval None
  */
static inline void daisy_clear_error(void)
{
    gMasterRunInfo.daisy_err_flag = 0;

    SET_SD_ELEMENT_VALUE(SD243, 0);
    SET_SD_ELEMENT_VALUE(SD244, 0);
    SET_SD_ELEMENT_VALUE(SD245, 0);

    if(gMasterRunInfo.daisy_uart_err_flag)
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
  * @brief  扫描从站
  * @param  None
  * @retval None
  */
static void daisy_task_handle_scan_slave(void)
{
    uint16_t *pBuf;
    uint16_t i;
    uint16_t j;

    //初始化
    gMasterRunInfo.scan_slave_num = 0;
    for(i = 0; i < DAISY_MAX_SLAVE_NUM; i++)
    {
        gMasterRunInfo.scan_slave_ext_id_version[i].num = 0;
        if(gMasterRunInfo.scan_slave_ext_id_version[i].pIdList != NULL)
        {
            vPortFree(gMasterRunInfo.scan_slave_ext_id_version[i].pIdList);
        }
    }

    //send 0101, receive slave number
    gMasterRunInfo.daisy_status = MASTER_IN_SCAN_NUM_SLAVE;
    pBuf = (uint16_t *)gDaisySendBuffer;
    *pBuf++ = DAISY_CMD_0101;
    *pBuf++ = 0;
    *pBuf = 0;
    daisy_LAN_send(gDaisySendBuffer, 6);
    if(daisy_wait_event_slave_receive(DAISY_WAIT_SLAVE_0101_MAX_TIME))
    {
        if(gMasterRunInfo.daisy_err_flag) //收到错误帧
        {
            gMasterRunInfo.scan_slave_num = 0;
            vTaskDelay(DAISY_SCAN_SLAVE_ERR_DELAY_TIME);
        }
        else
        {
            LOGD(TAG, "Master number slave success!");
            //vTaskDelay(DAISY_WAIT_SLAVE_0101_MAX_TIME - 1);
        }
    }
    else //一个从站也没有
    {
        gMasterRunInfo.scan_slave_num = 0;
    }

    LOGD(TAG, "Scan slave number: %d", gMasterRunInfo.scan_slave_num);
    if(gMasterRunInfo.scan_slave_num == 0)
    {
        gMasterRunInfo.daisy_index = 0;
        gMasterRunInfo.daisy_status = MASTER_SCAN_SLAVE_OVER;
        return;
    }

    //send 0707, receive slave and extend ID
    gMasterRunInfo.daisy_status = MASTER_IN_SCAN_SLAVE;
    for(i = 0; i < gMasterRunInfo.scan_slave_num; i++)
    {
        pBuf = (uint16_t *)gDaisySendBuffer;
        gMasterRunInfo.daisy_index = i + 1;
        *pBuf++ = DAISY_CMD_0707;
        *pBuf++ = gMasterRunInfo.daisy_index;
        daisy_LAN_send(gDaisySendBuffer, 4);
        if(daisy_wait_event_slave_receive(DAISY_WAIT_SLAVE_0707_MAX_TIME))
        {
            if(gMasterRunInfo.daisy_err_flag) //收到错误帧
            {
                gMasterRunInfo.scan_slave_num = 0;
                vTaskDelay(DAISY_SCAN_SLAVE_ERR_DELAY_TIME);
                break;
            }
            else
            {
                LOGD(TAG, "Scan slave[%d] ID success!", i);
                uint16_t malloc_size = gMasterRunInfo.scan_slave_ext_id_version[i].num * sizeof(id_version_t);
                LOGI(TAG, "Malloc size: %d", malloc_size);
                gMasterRunInfo.scan_slave_ext_id_version[i].pIdList = (uint32_t *)pvPortMalloc(malloc_size);
                LOGD(TAG, "scan_slave_ext_id[%d].pIdList -> 0x%08X", i, gMasterRunInfo.scan_slave_ext_id_version[i].pIdList)
                for(j = 0; j < gMasterRunInfo.scan_slave_ext_id_version[i].num; j++)
                {
                    gMasterRunInfo.scan_slave_ext_id_version[i].pIdList[j] = gMasterRunInfo.scan_slave_ext_id_version_temp[j];
                }
                //vTaskDelay(DAISY_WAIT_SLAVE_0707_MAX_TIME - 1);
            }
        }
        else
        {
            daisy_set_error(DAISY_TIMEOUT_ERR_MASTER_SCAN_SLAVE, gMasterRunInfo.daisy_index, DAISY_WAIT_SLAVE_0707_MAX_TIME);
            gMasterRunInfo.scan_slave_num = 0;
            vTaskDelay(DAISY_SCAN_SLAVE_ERR_DELAY_TIME);
            break;
        }
    }

    //Log
    LOGD(TAG, "Scan slave number: %d", gMasterRunInfo.scan_slave_num);
    for(i = 0; i < gMasterRunInfo.scan_slave_num; i++)
    {
        LOGI(TAG, "Slave[%d]", i);
        for(j = 0; j < gMasterRunInfo.scan_slave_ext_id_version[i].num; j++)
        {
            if(j == 0)
            {
                LOGD(TAG, "Slave_ID_Table[%d] = 0x%08X", j, gMasterRunInfo.scan_slave_ext_id_version[i].pIdList[j]);
            }
            else
            {
                LOGD(TAG, "Ext_ID_Table[%d] = 0x%08X", j, gMasterRunInfo.scan_slave_ext_id_version[i].pIdList[j]);
            }
        }
    }

    gMasterRunInfo.daisy_index = 0;
    gMasterRunInfo.daisy_status = MASTER_SCAN_SLAVE_OVER;
}

/**
  * @brief  扫描从站结束
  * @param  None
  * @retval None
  */
static void daisy_task_handle_scan_slave_over(void)
{
    gMasterRunInfo.daisy_status = MASTER_NUM_SLAVE;
}

/**
  * @brief  编址从站
  * @param  None
  * @retval None
  */
static void daisy_task_handle_num_slave(void)
{
    uint16_t *pBuf;

    gMasterRunInfo.daisy_status = MASTER_IN_NUM_SLAVE;
    pBuf = (uint16_t *)gDaisySendBuffer;
    *pBuf++ = DAISY_CMD_0101;
    *pBuf++ = 0;
    *pBuf = gBusConfig.nSlaveCount;
    daisy_LAN_send(gDaisySendBuffer, 6);
    if(daisy_wait_event_slave_receive(DAISY_WAIT_SLAVE_0101_MAX_TIME))
    {
        if(gMasterRunInfo.daisy_err_flag) //返回异常
        {
            CLEAR_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_SLAVE_NUM_Msk);
            gMasterRunInfo.daisy_status = MASTER_NUM_SLAVE;
            vTaskDelay(DAISY_NUM_SLAVE_ERR_DELAY_TIME); //等待一下 重新编址
        }
        else //返回正常
        {
            LOGD(TAG, "Master number slave success!");
            SET_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_SLAVE_NUM_Msk);
            gMasterRunInfo.daisy_status = MASTER_NUM_SLAVE_OVER;
            //vTaskDelay(DAISY_WAIT_SLAVE_0101_MAX_TIME - 1);
        }
    }
    else
    {
        CLEAR_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_SLAVE_NUM_Msk);
        daisy_set_error(DAISY_TIMEOUT_ERR_MASTER_NUM_SLAVE, 0, DAISY_WAIT_SLAVE_0101_MAX_TIME);
        gMasterRunInfo.daisy_status = MASTER_NUM_SLAVE;
        vTaskDelay(DAISY_NUM_SLAVE_ERR_DELAY_TIME); //等待一下 重新编址
    }
}

/**
  * @brief  编址从站结束
  * @param  None
  * @retval None
  */
static void daisy_task_handle_num_slave_over(void)
{
    gMasterRunInfo.daisy_err_times = 0;
    gMasterRunInfo.daisy_index = gBusConfig.nMasterExtCount;
    gMasterRunInfo.daisy_status = MASTER_CONFIG_SLAVE;
}

/**
  * @brief  配置从站
  * @param  None
  * @retval None
  */
static void daisy_task_handle_config_slave(void)
{
    LOGW(TAG, "Enter %s, daisy_status = %d", __func__, gMasterRunInfo.daisy_status);
    uint16_t *pu16Data;
    uint8_t  *pu8Data;

    gMasterRunInfo.daisy_status = MASTER_IN_CONFIG_SLAVE;

    pu16Data = (uint16_t *)gDaisySendBuffer;
    *pu16Data++ = DAISY_CMD_1C1C;
    pu8Data = (uint8_t *)pu16Data;
    memcpy(pu8Data, gBusConfig.item[gMasterRunInfo.daisy_index].pSubStationbuf, gBusConfig.item[gMasterRunInfo.daisy_index].len);
    //hexdump(gDaisySendBuffer, gBusConfig.item[gMasterRunInfo.daisy_index].len + 2);
    daisy_LAN_send(gDaisySendBuffer, 2 + gBusConfig.item[gMasterRunInfo.daisy_index].len);
    if(daisy_wait_event_slave_receive(DAISY_WAIT_SLAVE_1C1C_MAX_TIME))
    {
        if(gMasterRunInfo.daisy_err_flag) //返回异常
        {
            gMasterRunInfo.daisy_err_times++;
            if(gMasterRunInfo.daisy_err_times > DAISY_CONFIG_ERR_MAX_TIMES)
            {
                CLEAR_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_SLAVE_CONFIG_Msk);
                //gMasterRunInfo.daisy_index = 0;
                gMasterRunInfo.daisy_err_times = 0;
                gMasterRunInfo.daisy_status = MASTER_CONFIG_SLAVE; //异常超过次数后
            }
            else
            {
                gMasterRunInfo.daisy_status = MASTER_CONFIG_SLAVE; //再次配置
            }
            vTaskDelay(DAISY_CONFIG_SLAVE_ERR_DELAY_TIME);
        }
        else //返回正常
        {
            LOGD(TAG, "Master config slave success!");
            gMasterRunInfo.daisy_err_times = 0;
            gMasterRunInfo.daisy_index++;
            if(gMasterRunInfo.daisy_index >= gBusConfig.nSubStationCount) //配置好最后一个模块
            {
                gMasterRunInfo.daisy_index = 0;
                gMasterRunInfo.daisy_status = MASTER_CONFIG_SLAVE_OVER;
                SET_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_SLAVE_CONFIG_Msk);
            }
            else
            {
                gMasterRunInfo.daisy_status = MASTER_CONFIG_SLAVE; //继续配置下一个模块
            }
            //vTaskDelay(DAISY_WAIT_SLAVE_1C1C_MAX_TIME - 1);
        }
    }
    else
    {
        gMasterRunInfo.daisy_err_times++;
        if(gMasterRunInfo.daisy_err_times > DAISY_CONFIG_ERR_MAX_TIMES)
        {
            CLEAR_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_SLAVE_CONFIG_Msk);
            gMasterRunInfo.daisy_err_times = 0;
            //gMasterRunInfo.daisy_index = 0;
            daisy_set_error(DAISY_TIMEOUT_ERR_MASTER_CONFIG_SLAVE, GET_SMLPU16_DATA(gBusConfig.item[gMasterRunInfo.daisy_index].pSubStationbuf), DAISY_WAIT_SLAVE_1C1C_MAX_TIME);
            gMasterRunInfo.daisy_status = MASTER_CONFIG_SLAVE; //超时后
        }
        else
        {
            gMasterRunInfo.daisy_status = MASTER_CONFIG_SLAVE; //再次配置
        }
        vTaskDelay(DAISY_CONFIG_SLAVE_ERR_DELAY_TIME);
    }
}

/**
  * @brief  配置从站结束
  * @param  None
  * @retval None
  */
static void daisy_task_handle_config_slave_over(void)
{
    gMasterRunInfo.daisy_err_times = 0;
    gMasterRunInfo.daisy_index = 0;
    gMasterRunInfo.daisy_status = MASTER_PRELOOP_SLAVE;
}

/**
  * @brief  预启动从站
  * @param  None
  * @retval None
  */
static void daisy_task_handle_preloop_slave(void)
{
    LOGW(TAG, "Enter %s, daisy_status = %d", __func__, gMasterRunInfo.daisy_status);
    uint16_t *pBuf;

    gMasterRunInfo.daisy_status = MASTER_IN_PRELOOP_SLAVE;
    pBuf = (uint16_t *)gDaisySendBuffer;
    *pBuf++ = DAISY_CMD_2222;
    *pBuf++ = 0;
    *pBuf = gBusConfig.nSlaveCount;
    daisy_LAN_send(gDaisySendBuffer, 6);
    if(daisy_wait_event_slave_receive(DAISY_WAIT_SLAVE_2222_MAX_TIME))
    {
        if(gMasterRunInfo.daisy_err_flag) //返回异常
        {
            gMasterRunInfo.daisy_err_times++;
            if(gMasterRunInfo.daisy_err_times > DAISY_PRELOOP_ERR_MAX_TIMES)
            {
                CLEAR_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_SLAVE_PRELOOP_Msk);
                gMasterRunInfo.daisy_err_times = 0;
                gMasterRunInfo.daisy_status = MASTER_PRELOOP_SLAVE; //异常超过次数
            }
            else
            {
                gMasterRunInfo.daisy_status = MASTER_PRELOOP_SLAVE; //再次预启动
            }
            vTaskDelay(DAISY_PRELOOP_SLAVE_ERR_DELAY_TIME);
        }
        else //返回正常
        {
            LOGD(TAG, "Master preloop slave success!");
            gMasterRunInfo.daisy_err_times = 0;
            SET_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_SLAVE_PRELOOP_Msk);
            gMasterRunInfo.daisy_status = MASTER_PRELOOP_SLAVE_OVER;
            //vTaskDelay(DAISY_WAIT_SLAVE_2222_MAX_TIME - 1);
        }
    }
    else
    {
        gMasterRunInfo.daisy_err_times++;
        if(gMasterRunInfo.daisy_err_times > DAISY_PRELOOP_ERR_MAX_TIMES)
        {
            CLEAR_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_SLAVE_PRELOOP_Msk);
            daisy_set_error(DAISY_TIMEOUT_ERR_MASTER_PRELOOP_SLAVE, 0, DAISY_WAIT_SLAVE_2222_MAX_TIME);
            gMasterRunInfo.daisy_err_times = 0;
            gMasterRunInfo.daisy_status = MASTER_PRELOOP_SLAVE; //超时后
        }
        else
        {
            gMasterRunInfo.daisy_status = MASTER_PRELOOP_SLAVE; //再次预启动
        }
        vTaskDelay(DAISY_PRELOOP_SLAVE_ERR_DELAY_TIME);
    }
}

/**
  * @brief  预启动从站结束
  * @param  None
  * @retval None
  */
static void daisy_task_handle_preloop_slave_over(void)
{
    LOGW(TAG, "Enter %s, daisy_status = %d", __func__, gMasterRunInfo.daisy_status);

    if(READ_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_EXT_CAN_LOOP_Msk) != MASTER_RUN_FLAG_EXT_CAN_LOOP_Msk) //扩展未启动好 等待
    {
        vTaskDelay(1000);
        return;
    }

    if(gpbufLoop != NULL)
    {
        pbuf_free(gpbufLoop);
    }
    gpbufLoop = pbuf_alloc(PBUF_LINK, 4 + gBusConfig.nSynBuffLen, PBUF_RAM);
    LOGV(TAG, "gpbufLoop = 0x%08X", gpbufLoop);
    memset(gpbufLoop->payload, 0, 4 + gBusConfig.nSynBuffLen);
    gpLoopData = (uint8_t *)gpbufLoop->payload;
    *gpLoopData++ = DAISY_CMD_3333 & 0xFF;
    *gpLoopData++ = DAISY_CMD_3333 >> 8;
    gpLoopData++;
    gpLoopData++;

    struct eth_hdr *ethhdr;
    u16_t eth_type_be = lwip_htons(ETHTYPE_ETHER_KALYKE);
    LOGD(TAG, "eth_type_be = 0x%04X", eth_type_be);
    pbuf_add_header(gpbufLoop, SIZEOF_ETH_HDR);
    ethhdr = (struct eth_hdr *)gpbufLoop->payload;
    ethhdr->type = eth_type_be;
    SMEMCPY(&ethhdr->dest, (struct eth_addr *)(gEthBroadcast), ETH_HWADDR_LEN);
    SMEMCPY(&ethhdr->src,  (struct eth_addr *)(fsl_netif1.hwaddr), ETH_HWADDR_LEN);

    gMasterRunInfo.daisy_err_times = 0;
    gMasterRunInfo.daisy_status = MASTER_LOOP_SLAVE;
}


/**
  * @brief  计算响应时间并添加延时
            |------------ LoopTime -------------|
            |______________|____________________|
              ResponseTime        DelayTime

            DelayTime = SD[234] - ResponseTime
            SD[236]	= LoopTime
  * @param  None
  * @retval None
  */
static void daisy_loop_delay(void)
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
    gtv_PlcElement.msp_SDElement[SD236] = msInterval;
    if(msInterval > GET_SD_ELEMENT_VALUE(SD238))
    {
        SET_SD_ELEMENT_VALUE(SD238, msInterval);
    }
    msTick_start = msTick_end;
}

/**
  * @brief  数据交互
  * @param  None
  * @retval None
  */
static inline void daisy_task_handle_loop_slave(void)
{
    plc_refresh_force_element_value();

    uint8_t *pData;

    pData = gpLoopData;
    *(pData - 1) = 0;
    *(pData - 2) = 0;

    for (int i = 0; i < gBusConfig.pdo1BCount; i++)
    {
        switch (gBusConfig.pPDO1B[i].eType)
        {
        case DAISY_E_TYPE_X:
            pData[gBusConfig.pPDO1B[i].offset] = daisy_get_8bit_element_value(X_ELEMENT, gBusConfig.pPDO1B[i].eAddr);
            break;

        case DAISY_E_TYPE_Y:
            pData[gBusConfig.pPDO1B[i].offset] = daisy_get_8bit_element_value(Y_ELEMENT, gBusConfig.pPDO1B[i].eAddr);
            break;

        case DAISY_E_TYPE_M:
            pData[gBusConfig.pPDO1B[i].offset] = daisy_get_8bit_element_value(M_ELEMENT, gBusConfig.pPDO1B[i].eAddr);
            break;

        case DAISY_E_TYPE_D:
            if(gBusConfig.pPDO1B[i].len == 4)
            {
                pData[gBusConfig.pPDO1B[i].offset] = (gtv_PlcElement.msp_DElement[gBusConfig.pPDO1B[i].eAddr + 1] & 0xFF00) >> 8;
                pData[gBusConfig.pPDO1B[i].offset + 1] = gtv_PlcElement.msp_DElement[gBusConfig.pPDO1B[i].eAddr + 1] & 0x00FF;
                pData[gBusConfig.pPDO1B[i].offset + 2] = (gtv_PlcElement.msp_DElement[gBusConfig.pPDO1B[i].eAddr] & 0xFF00) >> 8;
                pData[gBusConfig.pPDO1B[i].offset + 3] = gtv_PlcElement.msp_DElement[gBusConfig.pPDO1B[i].eAddr] & 0x00FF;
            }
            else
            {
                pData[gBusConfig.pPDO1B[i].offset] = (gtv_PlcElement.msp_DElement[gBusConfig.pPDO1B[i].eAddr] & 0xFF00) >> 8;
                pData[gBusConfig.pPDO1B[i].offset + 1] = gtv_PlcElement.msp_DElement[gBusConfig.pPDO1B[i].eAddr] & 0x00FF;
            }
            break;

        case DAISY_E_TYPE_SD:
            pData[gBusConfig.pPDO1B[i].offset] = (gtv_PlcElement.msp_SDElement[gBusConfig.pPDO1B[i].eAddr] & 0xFF00) >> 8;
            pData[gBusConfig.pPDO1B[i].offset + 1] = gtv_PlcElement.msp_SDElement[gBusConfig.pPDO1B[i].eAddr] & 0x00FF;
            break;

        case DAISY_E_TYPE_R:
            pData[gBusConfig.pPDO1B[i].offset] = (gtv_PlcElement.msp_RElement[gBusConfig.pPDO1B[i].eAddr] & 0xFF00) >> 8;
            pData[gBusConfig.pPDO1B[i].offset + 1] = gtv_PlcElement.msp_RElement[gBusConfig.pPDO1B[i].eAddr] & 0x00FF;
            break;

        default:
            LOGE(TAG, "ERROR: eType = %u", gBusConfig.pPDO1B[i].eType);
            break;
        }
    }

#if (LOG_OPEN == 1)
#if (DAISY_COMUNICATION_COUNT == 2)
    gDaisyTick0 = xTaskGetTickCount();
#elif (DAISY_COMUNICATION_COUNT == 1)
    gDaisyTick0 = SysTick->VAL;
#elif (DAISY_COMUNICATION_COUNT == 3)
    gDaisyTick0 = SysTick->VAL;
    gDaisyTickMS = xTaskGetTickCount();
#endif
#endif

    err_t ret = ethernet_output_daisy(&fsl_netif1, gpbufLoop);
    //LOGV(TAG, "Leave %s(), ret = %d", __func__, ret);

    if(daisy_wait_event_slave_receive(gBusConfig.nBfm608A))
    {
        if(gMasterRunInfo.daisy_err_flag)
        {
            gMasterRunInfo.daisy_err_times++;
            gtv_PlcElement.msp_SDElement[SD247] = ++gMasterRunInfo.daisy_totalerr_times;
            if(gMasterRunInfo.daisy_err_times > DAISY_LOOP_ERR_MAX_TIMES)
            {
                gMasterRunInfo.daisy_err_times = 0;
                //gMasterRunInfo.daisy_status = MASTER_IDLE; //错误超限后
            }
            else
            {
                gMasterRunInfo.daisy_status = MASTER_LOOP_SLAVE; //继续发送
            }
            CLEAR_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_SLAVE_RUNNING_Msk);
        }
        else
        {
            gMasterRunInfo.daisy_err_times = 0;
            SET_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_SLAVE_RUNNING_Msk);
            //LOGD(TAG, "Loop slave success!");
        }
    }
    else
    {
        gMasterRunInfo.daisy_err_times++;
        gtv_PlcElement.msp_SDElement[SD247] = ++gMasterRunInfo.daisy_totalerr_times;
        if(gMasterRunInfo.daisy_err_times > DAISY_LOOP_ERR_MAX_TIMES)
        {
            daisy_set_error(DAISY_TIMEOUT_ERR_MASTER_LOOP_SLAVE, 0, gBusConfig.nBfm608A);
            gMasterRunInfo.daisy_err_times = 0;
            //gMasterRunInfo.daisy_status = MASTER_IDLE; //错误超限后
        }
        else
        {
            gMasterRunInfo.daisy_status = MASTER_LOOP_SLAVE; //继续发送
        }
        CLEAR_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_SLAVE_RUNNING_Msk);
    }

    daisy_loop_delay();
}

/**
  * @brief  以太网总线任务
  * @param  None
  * @retval None
  */
static void daisy_task(void *p_arg)
{
    LOGV(TAG, "daisy_task RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());

    vTaskDelay(DAISY_READY_START_DELAY_TIME); //等待智能设备启动
    daisy_task_handle_scan_slave();
    gMasterRunInfo.daisy_status = MASTER_NUM_SLAVE;

    if(gBusConfig.nPdoCount == 0) //没有从站
    {
        LOGW(TAG, "There is no slave module.");
        SET_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_SLAVE_CAN_LOOP_Msk);
        gMasterRunInfo.daisy_status = MASTER_IDLE;
        vTaskDelete(NULL); //删除自身任务
    }

    for(;;)
    {
        if (gGUHUAing == 1 || gSuspendFlag == true)
        {
            LOGE(TAG, "In %s(),GUHUAing.........vTaskDelay(1000).", __func__);
            vTaskDelay(1000);
            continue;
        }
        switch(gMasterRunInfo.daisy_status)
        {
        case MASTER_IDLE:
            //LOGD(TAG, "daisy task idle.");
            vTaskDelay(1000);
            break;

        case MASTER_SCAN_SLAVE:
            daisy_task_handle_scan_slave();
            break;

        case MASTER_SCAN_SLAVE_OVER:
            daisy_task_handle_scan_slave_over();
            break;

        case MASTER_NUM_SLAVE:
            daisy_task_handle_num_slave();
            break;

        case MASTER_NUM_SLAVE_OVER:
            daisy_task_handle_num_slave_over();
            break;

        case MASTER_CONFIG_SLAVE:
            daisy_task_handle_config_slave();
            break;

        case MASTER_CONFIG_SLAVE_OVER:
            daisy_task_handle_config_slave_over();
            break;

        case MASTER_PRELOOP_SLAVE:
            daisy_task_handle_preloop_slave();
            break;

        case MASTER_PRELOOP_SLAVE_OVER:
            daisy_task_handle_preloop_slave_over();
            break;

        case MASTER_LOOP_SLAVE:
            daisy_task_handle_loop_slave();
            break;

        default:
            LOGW(TAG, "Other gMasterRunInfo.daisy_status = %d", gMasterRunInfo.daisy_status);
            break;
        }

        //vTaskDelay(1); //解析操作里有添加阻塞 不需要额外加延时
    }

}

/**
  * @brief  判断是否是5555错误帧
  * @param  None
  * @retval None
  */
static bool is_slave_err_frame_5555(uint8_t *pBuf, uint16_t len)
{
    if(GET_SMLPU16_DATA(pBuf) == DAISY_CMD_5555)
    {
        return true;
    }

    return false;
}

/**
  * @brief  解析扩展返回的错误
  * @param  None
  * @retval None
  */
static void in_slave_err_handle_5555(uint8_t *pBuf)
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

    daisy_set_error(errCode, errAddr, errContent);
}

/**
  * @brief  解析从站响应的编址数据(0101)
  * @param  pBuf 数据
  * @param  Len 长度
  * @retval None
  */
static void in_num_slave_handle_0101(uint8_t *pBuf, uint16_t Len)
{
    uint8_t *pTemp;
    uint16_t Cmd;
    uint16_t NodeNum;

    if(Len < 4)
    {
        daisy_set_error(DAISY_LEN_ERR_MASTER_NUM_SLAVE, 0, Len);
        return;
    }

    pTemp = pBuf;
    Cmd = GET_SMLPU16_DATA(pTemp);
    if(Cmd != DAISY_CMD_0101)
    {
        if(is_slave_err_frame_5555(pTemp, Len))
        {
            in_slave_err_handle_5555(pTemp);
        }
        else //不该收到其他指令
        {
            daisy_set_error(DAISY_CMD_ERR_MASTER_NUM_SLAVE, 0, Cmd);
        }
        return;
    }

    pTemp += 2;
    NodeNum = GET_SMLPU16_DATA(pTemp);
    if(NodeNum != gBusConfig.nSlaveCount)
    {
        daisy_set_error(DAISY_NODEADDR_ERR_MASTER_NUM_SLAVE, 0, NodeNum);
    }
    else
    {
        daisy_clear_error();
    }
}

/**
  * @brief  解析扫描从站时响应的编址数据(0101)
  * @param  pBuf 数据
  * @param  Len 长度
  * @retval None
  */
static void in_scan_num_slave_handle_0101(uint8_t *pBuf, uint16_t Len)
{
    uint8_t *pTemp;
    uint16_t Cmd;
    uint16_t NodeNum;

    if(Len < 4)
    {
        daisy_set_error(DAISY_LEN_ERR_MASTER_NUM_SLAVE, 0, Len);
        return;
    }

    pTemp = pBuf;
    Cmd = GET_SMLPU16_DATA(pTemp);
    if(Cmd != DAISY_CMD_0101)
    {
        if(is_slave_err_frame_5555(pTemp, Len))
        {
            in_slave_err_handle_5555(pTemp);
        }
        else //不该收到其他指令
        {
            daisy_set_error(DAISY_CMD_ERR_MASTER_NUM_SLAVE, 0, Cmd);
        }
        return;
    }

    pTemp += 2;
    NodeNum = GET_SMLPU16_DATA(pTemp);
    if(NodeNum > DAISY_MAX_SLAVE_NUM)
    {
        gMasterRunInfo.scan_slave_num = DAISY_MAX_SLAVE_NUM;
    }
    else
    {
        gMasterRunInfo.scan_slave_num = NodeNum;
    }
    daisy_clear_error();
}

/**
  * @brief  解析从站响应的扫描数据(0707)
  * @param  pBuf 数据
  * @param  Len 长度
  * @retval None
  */
static void in_scan_slave_handle_0707(uint8_t *pBuf, uint16_t Len)
{
    uint8_t *pTemp;
    uint16_t Cmd;
    uint16_t SlaveAddr;

    if(Len < 4)
    {
        daisy_set_error(DAISY_LEN_ERR_MASTER_SCAN_SLAVE, 0, Len);
        return;
    }

    pTemp = pBuf;
    Cmd = GET_SMLPU16_DATA(pTemp);
    if(Cmd != DAISY_CMD_0707)
    {
        if(is_slave_err_frame_5555(pTemp, Len))
        {
            in_slave_err_handle_5555(pTemp);
        }
        else //不该收到其他指令
        {
            daisy_set_error(DAISY_CMD_ERR_MASTER_SCAN_SLAVE, gMasterRunInfo.daisy_index, Cmd);
        }
        return;
    }

    pTemp += 2;
    SlaveAddr = GET_SMLPU16_DATA(pTemp);
    if(SlaveAddr != gMasterRunInfo.daisy_index)
    {
        daisy_set_error(DAISY_NODEADDR_ERR_MASTER_SCAN_SLAVE, gMasterRunInfo.daisy_index, SlaveAddr);
        return;
    }

    pTemp += 2;
    uint16_t scan_slave_index = gMasterRunInfo.daisy_index - 1;
    gMasterRunInfo.scan_slave_ext_id_version[scan_slave_index].num = GET_SMLPU16_DATA(pTemp); //1+n 从站自身+从站下扩展数
    for(uint16_t i = 0; i < gMasterRunInfo.scan_slave_ext_id_version[scan_slave_index].num; i++)
    {
        pTemp += 2;
        gMasterRunInfo.scan_slave_ext_id_version_temp[i] = GET_SMLPU16_DATA(pTemp) << 16; //get id
        pTemp += 2;
        gMasterRunInfo.scan_slave_ext_id_version_temp[i] |= GET_SMLPU16_DATA(pTemp); //get version
    }
    daisy_clear_error();
}

/**
  * @brief  解析从站响应的配置数据(1C1C)
  * @param  pBuf 数据
  * @param  Len 长度
  * @retval None
  */
static void in_config_slave_handle_1C1C(uint8_t *pBuf, uint16_t Len)
{
    uint8_t *pTemp;
    uint16_t Cmd;
    uint16_t ErrNum;

    if(Len < 4)
    {
        daisy_set_error(DAISY_LEN_ERR_MASTER_CONFIG_SLAVE, 0, Len);
        return;
    }

    pTemp = pBuf;
    Cmd = GET_SMLPU16_DATA(pTemp);
    if(Cmd != DAISY_CMD_1C1C)
    {
        if(is_slave_err_frame_5555(pTemp, Len))
        {
            in_slave_err_handle_5555(pTemp);
        }
        else //不该收到其他指令
        {
            daisy_set_error(DAISY_CMD_ERR_MASTER_CONFIG_SLAVE, GET_SMLPU16_DATA(gBusConfig.item[gMasterRunInfo.daisy_index].pSubStationbuf), Cmd);
        }
        return;
    }

    pTemp += 2;
    ErrNum = GET_SMLPU16_DATA(pTemp);
    if(ErrNum == DAISY_COMMON_NO_ERR)
    {
        daisy_clear_error();
    }
    else
    {
        daisy_set_error(DAISY_OTHER_ERR_MASTER_CONFIG_SLAVE, GET_SMLPU16_DATA(gBusConfig.item[gMasterRunInfo.daisy_index].pSubStationbuf), ErrNum);
    }
}

/**
  * @brief  解析扩展响应的预启动数据(2222)
  * @param  pBuf 数据
  * @param  Len 长度
  * @retval None
  */
static void in_preloop_slave_handle_2222(uint8_t *pBuf, uint16_t Len)
{
    uint8_t *pTemp;
    uint16_t Cmd;
    uint16_t NodeNum;

    if(Len < 4)
    {
        daisy_set_error(DAISY_LEN_ERR_MASTER_PRELOOP_SLAVE, 0, Len);
        return;
    }

    pTemp = pBuf;
    Cmd = GET_SMLPU16_DATA(pTemp);
    if(Cmd != DAISY_CMD_2222)
    {
        if(is_slave_err_frame_5555(pTemp, Len))
        {
            in_slave_err_handle_5555(pTemp);
        }
        else //不该收到其他指令
        {
            daisy_set_error(DAISY_CMD_ERR_MASTER_PRELOOP_SLAVE, 0, Cmd);
        }
        return;
    }

    pTemp += 2;
    NodeNum = GET_SMLPU16_DATA(pTemp);
    if(NodeNum == gBusConfig.nSlaveCount)
    {
        daisy_clear_error();
    }
    else
    {
        daisy_set_error(DAISY_NODEADDR_ERR_MASTER_PRELOOP_SLAVE, 0, NodeNum);
    }
}

/**
  * @brief  判断掉线
  * @param  pBuf 数据
  * @retval None
  */
static inline bool daisy_judge_wkc_lost(uint8_t *pBuf)
{
    uint16_t wkc;
    uint16_t slaveExtMoudles;

    slaveExtMoudles = gBusConfig.nSubStationCount - gBusConfig.nSlaveCount - gBusConfig.nMasterExtCount; //从站下的扩展总数
    wkc = GET_SMLPU16_DATA(pBuf);
    //LOGI(TAG, "Enter %s(), *pWkc = %u, slaveExtMoudles = %u", __func__, wkc, slaveExtMoudles);

    if(wkc != slaveExtMoudles)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
  * @brief  解析扩展响应的交互数据(3333)
  * @param  pBuf 数据
  * @param  Len 长度
  * @retval None
  */
static void loop_slave_handle_3333(uint8_t *pBuf, uint16_t Len)
{
    uint8_t *pTemp;
    uint16_t Cmd;

    if(Len < 4)
    {
        daisy_set_error(DAISY_LEN_ERR_MASTER_LOOP_SLAVE, 0, Len);
        return;
    }

    pTemp = pBuf;
    Cmd = GET_SMLPU16_DATA(pTemp);
    if(Cmd != DAISY_CMD_3333)
    {
        if(is_slave_err_frame_5555(pTemp, Len))
        {
            in_slave_err_handle_5555(pTemp);
        }
        else //不该收到其他指令
        {
            hexdump(pBuf, Len);
            daisy_set_error(DAISY_CMD_ERR_MASTER_LOOP_SLAVE, 0, Cmd);
        }
        return;
    }

    pTemp += 2;
    if(daisy_judge_wkc_lost(pTemp))
    {
        daisy_set_error(DAISY_WKC_LOST_ERR_MASTER_LOOP_SLAVE, 0, GET_SMLPU16_DATA(pTemp));
        return;
    }

    pTemp += 2;
    for (uint16_t i = 0; i < gBusConfig.pdo1ACount; i++)
    {
        //LOGW(TAG, "gBusConfig.pPDO1A[%u].eType = %u", i, gBusConfig.pPDO1A[i].eType);
        switch (gBusConfig.pPDO1A[i].eType)
        {
        case DAISY_E_TYPE_X:
            daisy_set_8bit_element_value(X_ELEMENT, gBusConfig.pPDO1A[i].eAddr, pTemp[gBusConfig.pPDO1A[i].offset]);
            break;

        case DAISY_E_TYPE_Y:
            daisy_set_8bit_element_value(Y_ELEMENT, gBusConfig.pPDO1A[i].eAddr, pTemp[gBusConfig.pPDO1A[i].offset]);
            break;

        case DAISY_E_TYPE_M:
            daisy_set_8bit_element_value(M_ELEMENT, gBusConfig.pPDO1A[i].eAddr, pTemp[gBusConfig.pPDO1A[i].offset]);
            break;

        case DAISY_E_TYPE_D:
            if(gBusConfig.pPDO1A[i].len == 4)
            {
                gtv_PlcElement.msp_DElement[gBusConfig.pPDO1A[i].eAddr] = GET_BIGPU16_DATA(&pTemp[gBusConfig.pPDO1A[i].offset + 2]);
                gtv_PlcElement.msp_DElement[gBusConfig.pPDO1A[i].eAddr + 1] = GET_BIGPU16_DATA(&pTemp[gBusConfig.pPDO1A[i].offset]);
            }
            else
            {
                gtv_PlcElement.msp_DElement[gBusConfig.pPDO1A[i].eAddr] = GET_BIGPU16_DATA(&pTemp[gBusConfig.pPDO1A[i].offset]);
            }
            break;

        case DAISY_E_TYPE_SD:
            gtv_PlcElement.msp_SDElement[gBusConfig.pPDO1A[i].eAddr] = GET_BIGPU16_DATA(&pTemp[gBusConfig.pPDO1A[i].offset]);
            break;

        case DAISY_E_TYPE_R:
            gtv_PlcElement.msp_RElement[gBusConfig.pPDO1A[i].eAddr] = GET_BIGPU16_DATA(&pTemp[gBusConfig.pPDO1A[i].offset]);
            break;

        default:
            LOGE(TAG, "%s(): element type error(%u)", __func__, gBusConfig.pPDO1A[i].eType);
            break;
        }
    }
    daisy_clear_error();
}

/**
  * @brief  主站在其他状态下解析从站响应的数据
  * @param  pBuf 数据
  * @param  Len 长度
  * @retval None
  */
static void in_other_status_slave_handle(uint8_t *pBuf, uint16_t Len)
{
    if(Len < 2) //长度小于2，取cmd可能会死机
    {
        daisy_set_error(DAISY_STATE_LEN_ERR_MASTER_RECV_SLAVE, 0, gMasterRunInfo.daisy_status);
    }
    else if(Len < 4)
    {
        daisy_set_error(DAISY_STATE_LEN_ERR_MASTER_RECV_SLAVE, 0, gMasterRunInfo.daisy_status);
    }
    else if(is_slave_err_frame_5555(pBuf, Len))
    {
        in_slave_err_handle_5555(pBuf);
    }
    else
    {
        daisy_set_error(DAISY_STATE_ERR_MASTER_RECV_SLAVE, 0, gMasterRunInfo.daisy_status);
    }
}

/**
  * @brief  LAN口数据处理
  * @param  pBuf 数据
  * @param  length 长度
  * @retval None
  */
static inline void daisy_handle_lan_received_data(uint8_t *pData, uint16_t length)
{
    uint8_t *pBuf;
    uint16_t Len;

    pBuf = pData;
    Len = length;

    //智能设备Lan口解析
    if(g_plc_netcfg.wan.ioExp == WAN_CONFIG_IO_EXP_FEXLINK)
    {
        if(gDaisyLANTaskHandle == NULL)
        {
            return;
        }
        BaseType_t higherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(gDaisyLANTaskHandle, &higherPriorityTaskWoken); //触发LAN口接收任务
        if (higherPriorityTaskWoken == pdTRUE)
        {
            portYIELD_FROM_ISR(higherPriorityTaskWoken);
        }
        //daisy_toggle_LAN_com_LED();
        /* 不能在此pbuf_free(gLANpbuf);，要发送完再free */
    }
    //主站设备Lan口解析
    else
    {
#if (LOG_OPEN == 1)
        if(gMasterRunInfo.daisy_status != MASTER_LOOP_SLAVE) /* log print */
        {
            LOGD(TAG, "In status(%d) lan_received_data", gMasterRunInfo.daisy_status);
            hexdump(pBuf, Len);
        }
#endif

        switch(gMasterRunInfo.daisy_status)
        {
        case MASTER_IN_NUM_SLAVE:
            in_num_slave_handle_0101(pBuf, Len);
            break;

        case MASTER_IN_SCAN_NUM_SLAVE:
            in_scan_num_slave_handle_0101(pBuf, Len);
            break;

        case MASTER_IN_SCAN_SLAVE:
            in_scan_slave_handle_0707(pBuf, Len);
            break;

        case MASTER_IN_CONFIG_SLAVE:
            in_config_slave_handle_1C1C(pBuf, Len);
            break;

        case MASTER_IN_PRELOOP_SLAVE:
            in_preloop_slave_handle_2222(pBuf, Len);
            break;

        case MASTER_LOOP_SLAVE:
            loop_slave_handle_3333(pBuf, Len);
            break;

        default:
            in_other_status_slave_handle(pBuf, Len);
            break;
        }

        daisy_set_event_slave_receive();
        pbuf_free(gLANpbuf);
    }
}

/* ---------------------------------- 智能设备 -------------------------------------------- */

static void kalyke_daisy_bus_timer_callback(TimerHandle_t ltv_TimeHandle)
{
    LOGE(TAG, "Enter %s", __func__);
    CLEAR_BIT(gMasterRunInfo.flag, SMART_RUN_FLAG_SMART_RUNNING_Msk);
    gMasterRunInfo.daisy_status = SMART_LOOP_OVER;
    //daisy_start_blink_err_led();

    //可能需要重启网络
    //netifapi_netif_remove(&gNetifWan);
    //daisy_wan_init();
}

void kalyke_daisy_bus_timer_init(uint16_t timeout)
{
    LOGV(TAG, "Enter %s, timeout=%d\r\n", __func__, timeout);
    if (timeout < 4)
    {
        timeout = 4;
    }
    if(gDaisyBusTimer != NULL)
    {
        xTimerDelete(gDaisyBusTimer, 100);
    }
    gDaisyBusTimer = xTimerCreate((const char *)"gDaisyBusTimer",
                                  (TickType_t  )timeout / portTICK_PERIOD_MS,
                                  (UBaseType_t )pdFALSE,
                                  (void *      )1,
                                  (TimerCallbackFunction_t)kalyke_daisy_bus_timer_callback);
    if(gDaisyBusTimer == NULL)
    {
        assertHappened(__func__, __LINE__);
    }
}

static void kalyke_daisy_bus_timer_start(void)
{
    if(gDaisyBusTimer == NULL)
    {
        assertHappened(__func__, __LINE__);
    }

    if( xTimerStart( gDaisyBusTimer, 100 ) != pdPASS )
    {
        // The start command was not executed successfully.  Take appropriate
        // action here.
        LOGE("uart", "Start gDaisyBusTimer not executed successfully");
    }
}

static void kalyke_daisy_bus_timer_reset(void)
{
    if(gDaisyBusTimer == NULL)
    {
        assertHappened(__func__, __LINE__);
    }

    if( xTimerReset( gDaisyBusTimer, 100 ) != pdPASS )
    {
        // The start command was not executed successfully.  Take appropriate
        // action here.
        LOGE("uart", "Reset gDaisyBusTimer not executed successfully");
    }
}

/**
  * @brief  返回错误给主站
  * @param  errCode 错误码
  * @param  errAddr 模块地址
  * @param  errContent 错误内容
  * @retval None
  */
static void daisy_handle_err_to_master(uint16_t errCode, uint16_t errAddr,  uint16_t errContent)
{
    //LOGV(TAG, "errCode = 0x%04X, errAddr = 0x%04X, errContent = %d", errCode, errAddr, errContent);
    uint8_t data[10] = {0};
    uint16_t *pBuf = (uint16_t *)data;

    *pBuf++ = DAISY_CMD_5555;
    *pBuf++ = errCode;
    *pBuf++ = errAddr;
    *pBuf++ = errContent;

    gMasterRunInfo.daisy_err_flag = 1;
    guv_NonStopError.bit.extend_bus_err = 1;
    daisy_WAN_send(data, 10);
}

/**
  * @brief  清除背板总线错误
  * @param  None
  * @retval None
  */
static inline void daisy_smart_clear_error(void)
{
    gMasterRunInfo.daisy_err_flag = 0;

    if(gMasterRunInfo.daisy_uart_err_flag)
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
  * @brief  一些变量复位
  * @param  None
  * @retval None
  */
static void daisy_handle_parameter_reset(void)
{
    gSmartBFM.nPdoBytesSmart = 0;
    gSmartBFM.nPdoCountSmart = 0;
    gSmartBFM.pdo1ACountSmart = 0;
    gSmartBFM.pdo1BCountSmart = 0;

    if(gSmartBFM.pPDO1A_Smart != NULL)
    {
        vPortFree(gSmartBFM.pPDO1A_Smart);
    }
    if(gSmartBFM.pPDO1B_Smart != NULL)
    {
        vPortFree(gSmartBFM.pPDO1B_Smart);
    }

    gSmartBFM.bfm6087 = 0;
    gSmartBFM.bfm608A = 50; //超时时间

    CLEAR_BIT(gMasterRunInfo.flag, SMART_RUN_FLAG_SMART_NUM_Msk);
    CLEAR_BIT(gMasterRunInfo.flag, SMART_RUN_FLAG_SMART_CONFIG_Msk);
    CLEAR_BIT(gMasterRunInfo.flag, SMART_RUN_FLAG_SMART_RUNNING_Msk);
}

/**
  * @brief  解析主站发送的编址数据(0101)
  * @param  pBuf 数据
  * @param  len 长度
  * @retval None
  */
static void handle_cmd_0101(uint8_t *pBuf, uint16_t len)
{
    LOGV(TAG, "Enter %s()", __func__);
    uint8_t *pTemp;
    uint16_t NodeAddr;

    gMasterRunInfo.daisy_status = SMART_IN_NUM;
    pTemp = pBuf + 2;
    NodeAddr = GET_SMLPU16_DATA(pTemp);
    NodeAddr += 1;
    *(pTemp) = NodeAddr & 0xFF;
    *(pTemp + 1) = NodeAddr >> 8;

    gMasterRunInfo.node_addr = NodeAddr;
    LOGV(TAG, "gMasterRunInfo.node_addr = %u", gMasterRunInfo.node_addr);
    daisy_handle_parameter_reset();

    pTemp += 2;
    NodeAddr = GET_SMLPU16_DATA(pTemp); //模块总数
    NodeAddr = (uint8_t)NodeAddr;
    if(NodeAddr == 0) //根据网口状态返回
    {
        if (lan_is_up())
        {
            CLEAR_BIT(gMasterRunInfo.flag, SMART_RUN_FLAG_LAST_NODE_Msk);
            daisy_LAN_send(pBuf, 6);
        }
        else
        {
            SET_BIT(gMasterRunInfo.flag, SMART_RUN_FLAG_LAST_NODE_Msk);
            daisy_WAN_send(pBuf, 6);
        }
    }
    else //根据主站发的模块数返回
    {
        if(gMasterRunInfo.node_addr >= NodeAddr)
        {
            SET_BIT(gMasterRunInfo.flag, SMART_RUN_FLAG_LAST_NODE_Msk);
            daisy_WAN_send(pBuf, 6);
        }
        else
        {
            if(!lan_is_up()) //lan口没插
            {
                daisy_handle_err_to_master(DAISY_NODEADDR_ERR_IN_NUM_SMART, DAISY_NODE_ADDRESS(gMasterRunInfo.node_addr, 0), DAISY_NODE_ADDRESS(gMasterRunInfo.node_addr + 1, 0));
            }
            else
            {
                daisy_LAN_send(pBuf, 6);//把收到的数据直接转发给下一级
            }
            CLEAR_BIT(gMasterRunInfo.flag, SMART_RUN_FLAG_LAST_NODE_Msk);
        }
    }

    daisy_smart_clear_error();
    SET_BIT(gMasterRunInfo.flag, SMART_RUN_FLAG_SMART_NUM_Msk);
    gMasterRunInfo.daisy_status = SMART_NUM_OVER;
}

/**
  * @brief  返回智能设备信息给主站
  * @param  None
  * @retval None
  */
static void daisy_response_scan_to_master(void)
{
    LOGV(TAG, "Enter %s()", __func__);
    uint16_t *pBuf;
    uint16_t scan_num;
    uint16_t buf_len;

    scan_num = 1; //自身
    buf_len = 6 + (scan_num * sizeof(id_version_t)); //cmd+addr+数量+各模块ID

    struct pbuf *p = pbuf_alloc(PBUF_LINK, buf_len, PBUF_RAM);
    if (p && p->payload)
    {
        pBuf = (uint16_t *)p->payload;
        *pBuf++ = DAISY_CMD_0707; //cmd
        *pBuf++ = gMasterRunInfo.node_addr; //addr
        *pBuf++ = scan_num; //数量
        *pBuf++ = SLAVE_NUMBER; //智能设备ID
        *pBuf++ = SW_VERSION_ID; //智能设备版本
    }
    else
    {
        pbuf_free(p);
        LOGE(TAG, "%s: ERROR, p = 0x%08X, p->payload = 0x%08X", __func__, p, p->payload);
        return;
    }
    //log_pbuf(p);
    hexdump(p->payload, p->len);

    err_t ret = ethernet_output(&fsl_netif0, p, (struct eth_addr *)(fsl_netif0.hwaddr), (struct eth_addr *)(gEthBroadcast), ETHTYPE_ETHER_KALYKE);
    pbuf_free(p);

    LOGD(TAG, "Leave %s(), ret = %d", __func__, ret);
}

/**
  * @brief  解析主站发送的扫描数据(0707)
  * @param  pBuf 数据
  * @param  len 长度
  * @retval None
  */
static void handle_cmd_0707(uint8_t *pBuf, uint16_t len)
{
    LOGV(TAG, "Enter %s()", __func__);
    uint8_t *pTemp;
    uint16_t NodeAddr;

    pTemp = pBuf + 2;
    NodeAddr = GET_SMLPU16_DATA(pTemp);
    if(NodeAddr == gMasterRunInfo.node_addr)  //从站node号匹配，才返回智能设备的信息
    {
        daisy_smart_clear_error();
        daisy_response_scan_to_master();
    }
    else if(!READ_BIT(gMasterRunInfo.flag, SMART_RUN_FLAG_SMART_NUM_Msk)) //node号未编址
    {
        daisy_handle_err_to_master(DAISY_SMART_NOT_ADDR_ERR_IN_SCAN_SMART, DAISY_NODE_ADDRESS(NodeAddr, 0), gMasterRunInfo.daisy_status);
    }
    else
    {
        if(!lan_is_up()) //lan口没插
        {
            daisy_handle_err_to_master(DAISY_NODEADDR_ERR_IN_SCAN_SMART, DAISY_NODE_ADDRESS(NodeAddr, 0), DAISY_NODE_ADDRESS(NodeAddr + 1, 0));
        }
        else
        {
            daisy_smart_clear_error();
            daisy_LAN_send(pBuf, 4); //把收到的数据直接转发给下一级
        }
    }
}

static void log_bfm(void)
{
    LOGV(TAG, "Enter %s()", __func__);
    uint32_t i;

    LOGI(TAG, "gMasterRunInfo.node_addr = %u", gMasterRunInfo.node_addr);
    LOGV(TAG, "gSmartBFM.bfm6081 = 0x%04X", gSmartBFM.bfm6081);
    LOGV(TAG, "gSmartBFM.bfm6082 = 0x%04X", gSmartBFM.bfm6082);
    LOGV(TAG, "gSmartBFM.bfm6084 = 0x%04X", gSmartBFM.bfm6084);
    LOGV(TAG, "gSmartBFM.bfm6085 = %u", gSmartBFM.bfm6085);
    LOGV(TAG, "gSmartBFM.bfm6086 = %u", gSmartBFM.bfm6086);
    LOGV(TAG, "gSmartBFM.bfm6087 = %u", gSmartBFM.bfm6087);
    LOGV(TAG, "gSmartBFM.bfm6088 = %u", gSmartBFM.bfm6088);

    for(i = 0; i < gSmartBFM.pdo1ACountSmart; i++)
    {
        LOGV(TAG, "pPDO1A_Smart[%d].sType = 0x%02X", i, gSmartBFM.pPDO1A_Smart[i].sType);
        LOGV(TAG, "pPDO1A_Smart[%d].eType = 0x%02X", i, gSmartBFM.pPDO1A_Smart[i].eType);
        LOGV(TAG, "pPDO1A_Smart[%d].eAddr = %d", i, gSmartBFM.pPDO1A_Smart[i].eAddr);
        LOGV(TAG, "pPDO1A_Smart[%d].offset = %d", i, gSmartBFM.pPDO1A_Smart[i].offset);
        LOGV(TAG, "pPDO1A_Smart[%d].len = %d", i, gSmartBFM.pPDO1A_Smart[i].len);
    }
    for(i = 0; i < gSmartBFM.pdo1BCountSmart; i++)
    {
        LOGV(TAG, "pPDO1B_Smart[%d].sType = 0x%02X", i, gSmartBFM.pPDO1B_Smart[i].sType);
        LOGV(TAG, "pPDO1B_Smart[%d].eType = 0x%02X", i, gSmartBFM.pPDO1B_Smart[i].eType);
        LOGV(TAG, "pPDO1B_Smart[%d].eAddr = %d", i, gSmartBFM.pPDO1B_Smart[i].eAddr);
        LOGV(TAG, "pPDO1B_Smart[%d].offset = %d", i, gSmartBFM.pPDO1B_Smart[i].offset);
        LOGV(TAG, "pPDO1B_Smart[%d].len = %d", i, gSmartBFM.pPDO1B_Smart[i].len);
    }
}

/**
  * @brief  解析主站发送的配置数据(1C1C)
  * @param  pBuf 数据
  * @param  len 长度
  * @retval None
  */
static void handle_cmd_1C1C(uint8_t *pBuf, uint16_t len)
{
    LOGV(TAG, "Enter %s()", __func__);
    uint16_t ErrCode;
    uint16_t ErrContent;
    uint8_t SlaveAddr;
    uint8_t ExtendAddr;

    ExtendAddr = *(pBuf + 2);
    SlaveAddr = *(pBuf + 3);
    LOGV(TAG, "Enter %s(), SlaveNum = %u, gMasterRunInfo.node_addr = %u, ExtendNum = %u", __func__, SlaveAddr, gMasterRunInfo.node_addr, ExtendAddr);
    if(!READ_BIT(gMasterRunInfo.flag, SMART_RUN_FLAG_SMART_NUM_Msk)) //从站未编址
    {
        daisy_handle_err_to_master(DAISY_SMART_NOT_ADDR_ERR_IN_CONFIG_SMART, DAISY_NODE_ADDRESS(SlaveAddr, ExtendAddr), gMasterRunInfo.daisy_status);
    }
    else if(SlaveAddr != gMasterRunInfo.node_addr) //其他从站
    {
        if(!lan_is_up()) //lan口没插
        {
            daisy_handle_err_to_master(DAISY_NODEADDR_ERR_IN_CONFIG_SMART, DAISY_NODE_ADDRESS(SlaveAddr, ExtendAddr), DAISY_NODE_ADDRESS(SlaveAddr + 1, ExtendAddr));
        }
        else
        {
            daisy_LAN_send(pBuf, len);
        }
    }
    else if(ExtendAddr == 0) //本从站
    {
        gMasterRunInfo.daisy_status = SMART_IN_CONFIG;
        ErrCode = daisy_handle_config(pBuf + 2, &ErrContent);
        gMasterRunInfo.daisy_status = SMART_CONFIG_OVER;
        if(ErrCode == DAISY_COMMON_NO_ERR) //解析没问题
        {
            log_bfm();
            daisy_smart_clear_error();
            SET_BIT(gMasterRunInfo.flag, SMART_RUN_FLAG_SMART_CONFIG_Msk);

            *(pBuf + 2) = DAISY_COMMON_NO_ERR;
            *(pBuf + 3) = DAISY_COMMON_NO_ERR >> 8;
            daisy_WAN_send(pBuf, 4);
            kalyke_daisy_bus_timer_init(gSmartBFM.bfm608A);
        }
        else
        {
            CLEAR_BIT(gMasterRunInfo.flag, SMART_RUN_FLAG_SMART_CONFIG_Msk);
            daisy_handle_err_to_master(ErrCode, DAISY_NODE_ADDRESS(SlaveAddr, ExtendAddr), ErrContent);
        }
    }
    else //智能设备下没有扩展
    {
        CLEAR_BIT(gMasterRunInfo.flag, SMART_RUN_FLAG_SMART_CONFIG_Msk);
        daisy_handle_err_to_master(DAISY_HAVE_EXT_ERR_IN_CONFIG_SMART, DAISY_NODE_ADDRESS(SlaveAddr, 0), DAISY_NODE_ADDRESS(SlaveAddr, ExtendAddr));
    }
}

/**
  * @brief  解析主站发送的预启动数据(2222)
  * @param  pBuf 数据
  * @param  len 长度
  * @retval None
  */
static void handle_cmd_2222(uint8_t *pBuf, uint16_t len)
{
    LOGV(TAG, "Enter %s()", __func__);
    uint8_t *pTemp;
    uint16_t NodeAddr;

    if(!READ_BIT(gMasterRunInfo.flag, SMART_RUN_FLAG_SMART_NUM_Msk)) //node号未编址
    {
        daisy_handle_err_to_master(DAISY_SMART_NOT_ADDR_ERR_IN_PRELOOP_SMART, DAISY_NODE_ADDRESS(0, 0), gMasterRunInfo.daisy_status);
        return;
    }
    if(!READ_BIT(gMasterRunInfo.flag, SMART_RUN_FLAG_SMART_CONFIG_Msk)) //未配置
    {
        daisy_handle_err_to_master(DAISY_SMART_NOT_CONFIG_ERR_IN_PRELOOP_SMART, DAISY_NODE_ADDRESS(gMasterRunInfo.node_addr, 0), gMasterRunInfo.daisy_status);
        return;
    }

    pTemp = pBuf + 2;
    NodeAddr = GET_SMLPU16_DATA(pTemp);
    NodeAddr += 1;
    *(pTemp) = NodeAddr & 0xFF;
    *(pTemp + 1) = NodeAddr >> 8;

    if (READ_BIT(gMasterRunInfo.flag, SMART_RUN_FLAG_LAST_NODE_Msk))
    {
        daisy_WAN_send(pBuf, 6); //如果是尾节点，则直接返回
    }
    else
    {
        if(!lan_is_up())
        {
            daisy_handle_err_to_master(DAISY_NODEADDR_ERR_IN_PRELOOP_SMART, DAISY_NODE_ADDRESS(gMasterRunInfo.node_addr, 0), DAISY_NODE_ADDRESS(gMasterRunInfo.node_addr + 1, 0));
        }
        else
        {
            daisy_smart_clear_error();
            daisy_LAN_send(pBuf, 6);
        }
    }
}

/**
  * @brief  解析交互数据
  * @param  pBuf 数据
  * @retval None
  */
static inline void daisy_handle_data(uint8_t *pBuf)
{
    uint32_t i;

    //TxPdo  master <- smart
    for(i = 0; i < gSmartBFM.pdo1ACountSmart; i++)
    {
        switch (gSmartBFM.pPDO1A_Smart[i].eType)
        {
        case DAISY_E_TYPE_X:
            pBuf[gSmartBFM.pPDO1A_Smart[i].offset] = daisy_get_8bit_element_value(X_ELEMENT, gSmartBFM.pPDO1A_Smart[i].eAddr);
            break;

        case DAISY_E_TYPE_Y:
            pBuf[gSmartBFM.pPDO1A_Smart[i].offset] = daisy_get_8bit_element_value(Y_ELEMENT, gSmartBFM.pPDO1A_Smart[i].eAddr);
            break;

        case DAISY_E_TYPE_M:
            pBuf[gSmartBFM.pPDO1A_Smart[i].offset] = daisy_get_8bit_element_value(M_ELEMENT, gSmartBFM.pPDO1A_Smart[i].eAddr);
            break;

        case DAISY_E_TYPE_D:
            if(gSmartBFM.pPDO1A_Smart[i].len == 4)
            {
                pBuf[gSmartBFM.pPDO1A_Smart[i].offset] = (gtv_PlcElement.msp_DElement[gSmartBFM.pPDO1A_Smart[i].eAddr + 1] & 0xFF00) >> 8;
                pBuf[gSmartBFM.pPDO1A_Smart[i].offset + 1] = gtv_PlcElement.msp_DElement[gSmartBFM.pPDO1A_Smart[i].eAddr + 1] & 0x00FF;
                pBuf[gSmartBFM.pPDO1A_Smart[i].offset + 2] = (gtv_PlcElement.msp_DElement[gSmartBFM.pPDO1A_Smart[i].eAddr] & 0xFF00) >> 8;
                pBuf[gSmartBFM.pPDO1A_Smart[i].offset + 3] = gtv_PlcElement.msp_DElement[gSmartBFM.pPDO1A_Smart[i].eAddr] & 0x00FF;
            }
            else
            {
                pBuf[gSmartBFM.pPDO1A_Smart[i].offset] = (gtv_PlcElement.msp_DElement[gSmartBFM.pPDO1A_Smart[i].eAddr] & 0xFF00) >> 8;
                pBuf[gSmartBFM.pPDO1A_Smart[i].offset + 1] = gtv_PlcElement.msp_DElement[gSmartBFM.pPDO1A_Smart[i].eAddr] & 0x00FF;
            }
            break;

        case DAISY_E_TYPE_SD:
            pBuf[gSmartBFM.pPDO1A_Smart[i].offset] = (gtv_PlcElement.msp_SDElement[gSmartBFM.pPDO1A_Smart[i].eAddr] & 0xFF00) >> 8;
            pBuf[gSmartBFM.pPDO1A_Smart[i].offset + 1] = gtv_PlcElement.msp_SDElement[gSmartBFM.pPDO1A_Smart[i].eAddr] & 0x00FF;
            break;

        case DAISY_E_TYPE_R:
            pBuf[gSmartBFM.pPDO1A_Smart[i].offset] = (gtv_PlcElement.msp_RElement[gSmartBFM.pPDO1A_Smart[i].eAddr] & 0xFF00) >> 8;
            pBuf[gSmartBFM.pPDO1A_Smart[i].offset + 1] = gtv_PlcElement.msp_RElement[gSmartBFM.pPDO1A_Smart[i].eAddr] & 0x00FF;
            break;

        default:
            LOGE(TAG, "ERROR: eType = %u", gSmartBFM.pPDO1A_Smart[i].eType);
            break;
        }
    }
    //RxPdo  master -> smart
    for(i = 0; i < gSmartBFM.pdo1BCountSmart; i++)
    {
        switch (gSmartBFM.pPDO1B_Smart[i].eType)
        {
        case DAISY_E_TYPE_X:
            daisy_set_8bit_element_value(X_ELEMENT, gSmartBFM.pPDO1B_Smart[i].eAddr, pBuf[gSmartBFM.pPDO1B_Smart[i].offset]);
            break;

        case DAISY_E_TYPE_Y:
            daisy_set_8bit_element_value(Y_ELEMENT, gSmartBFM.pPDO1B_Smart[i].eAddr, pBuf[gSmartBFM.pPDO1B_Smart[i].offset]);
            break;

        case DAISY_E_TYPE_M:
            daisy_set_8bit_element_value(M_ELEMENT, gSmartBFM.pPDO1B_Smart[i].eAddr, pBuf[gSmartBFM.pPDO1B_Smart[i].offset]);
            break;

        case DAISY_E_TYPE_D:
            if(gSmartBFM.pPDO1B_Smart[i].len == 4)
            {
                gtv_PlcElement.msp_DElement[gSmartBFM.pPDO1B_Smart[i].eAddr] = GET_BIGPU16_DATA(&pBuf[gSmartBFM.pPDO1B_Smart[i].offset + 2]);
                gtv_PlcElement.msp_DElement[gSmartBFM.pPDO1B_Smart[i].eAddr + 1] = GET_BIGPU16_DATA(&pBuf[gSmartBFM.pPDO1B_Smart[i].offset]);
            }
            else
            {
                gtv_PlcElement.msp_DElement[gSmartBFM.pPDO1B_Smart[i].eAddr] = GET_BIGPU16_DATA(&pBuf[gSmartBFM.pPDO1B_Smart[i].offset]);
            }
            break;

        case DAISY_E_TYPE_SD:
            gtv_PlcElement.msp_SDElement[gSmartBFM.pPDO1B_Smart[i].eAddr] = GET_BIGPU16_DATA(&pBuf[gSmartBFM.pPDO1B_Smart[i].offset]);
            break;

        case DAISY_E_TYPE_R:
            gtv_PlcElement.msp_RElement[gSmartBFM.pPDO1B_Smart[i].eAddr] = GET_BIGPU16_DATA(&pBuf[gSmartBFM.pPDO1B_Smart[i].offset]);
            break;

        default:
            LOGE(TAG, "%s(): element type error(%u)", __func__, gSmartBFM.pPDO1B_Smart[i].eType);
            break;

        }
    }
}

/**
  * @brief  解析主站发送的交互数据(3333)
  * @param  pBuf 数据
  * @param  len 长度
  * @retval None
  */
static inline void handle_cmd_3333(uint8_t *pBuf, uint16_t len)
{
    //LOGV(TAG, "Enter %s()", __func__);
    uint16_t wkc;

    if(READ_BIT(gMasterRunInfo.flag, SMART_RUN_FLAG_SMART_CAN_LOOP_Msk) != SMART_RUN_FLAG_SMART_CAN_LOOP_Msk) //未完成启动
    {
        if(!READ_BIT(gMasterRunInfo.flag, SMART_RUN_FLAG_SMART_NUM_Msk))
        {
            daisy_handle_err_to_master(DAISY_SMART_NOT_ADDR_ERR_IN_LOOP_SMART, DAISY_NODE_ADDRESS(0, 0), gMasterRunInfo.daisy_status);
        }
        else
        {
            daisy_handle_err_to_master(DAISY_SMART_NOT_CONFIG_ERR_IN_LOOP_SMART, DAISY_NODE_ADDRESS(gMasterRunInfo.node_addr, 0), gMasterRunInfo.daisy_status);
        }
        return;
    }

#if 0 //智能设备没有扩展, wkc不需要加
    wkc = GET_SMLPU16_DATA(pBuf + 2) + gSmartBFM.bfm6087;
    *(pBuf + 2) = wkc & 0xFF;
    *(pBuf + 3) = wkc >> 8;
#endif

    daisy_handle_data((uint8_t *)(pBuf + 4));

    SET_BIT(gMasterRunInfo.flag, SMART_RUN_FLAG_SMART_RUNNING_Msk);
    gMasterRunInfo.daisy_status = SMART_IN_LOOP;

    if (READ_BIT(gMasterRunInfo.flag, SMART_RUN_FLAG_LAST_NODE_Msk))
    {
        //daisy_WAN_send(pBuf, len); //如果是尾节点，则直接返回
        daisy_send_directly_by_wan(gWANpbuf);
    }
    else
    {
        if(!lan_is_up())
        {
            daisy_handle_err_to_master(DAISY_NODEADDR_ERR_IN_LOOP_SMART, DAISY_NODE_ADDRESS(gMasterRunInfo.node_addr, 0), DAISY_NODE_ADDRESS(gMasterRunInfo.node_addr + 1, 0));
        }
        else
        {
            //daisy_LAN_send(pBuf, len);
            daisy_send_directly_by_lan(gWANpbuf);
        }
    }

    daisy_smart_clear_error();
    kalyke_daisy_bus_timer_reset();
}

/**
  * @brief  智能设备WAN口接收任务
  * @param  None
  * @retval None
  */
static void daisy_WAN_task(void *p_arg)
{
    LOGV(TAG, "daisy_WAN_task RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());
    uint8_t *pBuf;
    uint16_t Len;

    for(;;)
    {
        //LOGI(TAG, "%s sleep in ulTaskNotifyTake...", __func__);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        pBuf = (uint8_t *)((uint8_t *)gWANpbuf->payload + SIZEOF_ETH_HDR);
        Len = gWANpbuf->len;

        //LOGD(TAG, "In status(%d) wan_received_data", gMasterRunInfo.daisy_status);
        //hexdump(pBuf, Len);

        if(Len < 4)
        {
            daisy_handle_err_to_master(DAISY_LEN_ERR_IN_SMART, DAISY_NODE_ADDRESS(gMasterRunInfo.node_addr, 0), Len);
            return;
        }

        switch(GET_SMLPU16_DATA(pBuf))
        {
        case DAISY_CMD_0101:
            handle_cmd_0101(pBuf, Len);
            break;

        case DAISY_CMD_0707:
            handle_cmd_0707(pBuf, Len);
            break;

        case DAISY_CMD_1C1C:
            handle_cmd_1C1C(pBuf, Len);
            break;

        case DAISY_CMD_2222:
            handle_cmd_2222(pBuf, Len);
            break;

        case DAISY_CMD_3333:
            handle_cmd_3333(pBuf, Len);
            break;

        default:
            daisy_handle_err_to_master(DAISY_CMD_ERR_IN_SMART, DAISY_NODE_ADDRESS(gMasterRunInfo.node_addr, 0), GET_SMLPU16_DATA(pBuf));
            break;
        }

        pbuf_free(gWANpbuf);
        //daisy_LAN_send(pBuf, Len); //test
    }
}

/**
  * @brief  智能设备LAN口接收任务 当LAN task醒的时候，要迅速地把LAN刚收到的数据发给上面
  * @param  None
  * @retval None
  */
RAMFUNCTION_SECTION_CODE(static void daisy_LAN_task(void *p_arg))
{
    //LOGV(TAG, "daisy_LAN_task RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());
    uint32_t ret;
    //uint8_t *pBuf;
    //uint16_t Len;
    for(;;)
    {
        //LOGI(TAG, "%s sleep in ulTaskNotifyTake...", __func__);
        ret = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        //LOGI(TAG, "%s wakeup from ulTaskNotifyTake, ret = %u, *gpCmd = 0x%X", __func__, ret, *gpCmd);
        //pBuf = (uint8_t *)((uint8_t *)gLANpbuf->payload + SIZEOF_ETH_HDR);
        //Len = gLANpbuf->len;

        //daisy_WAN_send(pBuf, Len);
        daisy_send_directly_by_wan(gLANpbuf);
        pbuf_free((struct pbuf *)gLANpbuf);
#if 0
        //if (*gpCmd != DAISY_CMD_3333 && *gpCmd != DAISY_CMD_1111)
        if (*gpCmd == DAISY_CMD_0101 || *gpCmd == DAISY_CMD_1C1C)
        {
            daisy_blink_LAN_led();
        }
        else
        {
        }
#endif
    }
}

/**
  * @brief  WAN口数据处理
  * @param  None
  * @retval None
  */
static inline void daisy_handle_wan_received_data(uint8_t *pData, uint16_t length)
{
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    if(gDaisyWANTaskHandle == NULL)
    {
        return;
    }
    vTaskNotifyGiveFromISR(gDaisyWANTaskHandle, &higherPriorityTaskWoken); //触发WAN口接收任务
    if (higherPriorityTaskWoken == pdTRUE)
    {
        portYIELD_FROM_ISR (higherPriorityTaskWoken);
    }
    //daisy_toggle_WAN_com_LED();
}

/**
  * @brief  创建以太网接收任务
  * @param  None
  * @retval None
  */
static void start_LAN_WAN_task(void)
{
    BaseType_t ret;

    if((gDaisyWANTaskHandle == NULL) && (gDaisyLANTaskHandle == NULL))
    {
        gMasterRunInfo.daisy_status = SMART_IDLE;
    }

    if(gDaisyWANTaskHandle == NULL)
    {
        ret = xTaskCreate((TaskFunction_t)daisy_WAN_task,
                          (const char *)"WAN_task",
                          DAISY_TASK_STACK_SIZE,
                          (void *)NULL,
                          DAISY_TASK_PRIO,
                          (TaskHandle_t *)&gDaisyWANTaskHandle);
    }
    if(gDaisyLANTaskHandle == NULL)
    {
        ret = xTaskCreate((TaskFunction_t)daisy_LAN_task,
                          (const char *)"LAN_task",
                          DAISY_TASK_STACK_SIZE,
                          (void *)NULL,
                          DAISY_TASK_PRIO + 1,
                          (TaskHandle_t *)&gDaisyLANTaskHandle);
    }
}

/**
  * @brief  创建以太网总线任务
  * @param  None
  * @retval None
  */
void start_daisy_task(void)
{
    LOGV(TAG, "Enter %s(), gDaisyTaskHandle = 0x%08X", __func__, gDaisyTaskHandle);

    start_daisy_uart_task();

    if(g_plc_netcfg.wan.ioExp == WAN_CONFIG_IO_EXP_FEXLINK) //智能设备 以太网总线不需要主动
    {
        LOGW(TAG, "There is smart device.");
        start_LAN_WAN_task();
        SET_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_SLAVE_CAN_LOOP_Msk);
        return;
    }

    if(g_plc_netcfg.lan.ioExp == LAN_CONFIG_IO_EXP_FEXLINK)
    {
        if(gDaisyTaskHandle == NULL)
        {
            BaseType_t ret = xTaskCreate((TaskFunction_t)daisy_task,
                                         (const char *)"daisy_task",
                                         DAISY_TASK_STACK_SIZE,
                                         (void *)NULL,
                                         DAISY_TASK_PRIO,
                                         (TaskHandle_t *)&gDaisyTaskHandle);
            if (ret != pdPASS)
            {
                LOGE(TAG, "Create daisy_task error!");
            }
            LOGD(TAG, "gDaisyTaskHandle = 0x%08X", gDaisyTaskHandle);
        }
    }
    else //让背板总线跑起来
    {
        SET_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_SLAVE_CAN_LOOP_Msk);
    }
}
void saveErrorCodeFIFO(uint16_t nErrorCode)
{
    if(GET_SD_ELEMENT_VALUE(SD248) != nErrorCode)
    {
        for(int i = SD257; i > SD248; i--)
        {
            SET_SD_ELEMENT_VALUE(i, GET_SD_ELEMENT_VALUE(i - 1));
        }
        SET_SD_ELEMENT_VALUE(SD248, nErrorCode);
    }
}

/**
  * @brief  总线指示灯
  * @param  None
  * @retval None
  */
void daisy_handle_run_led(void)
{
    //daisy uart led / CC100自上而下数第3个灯
    if(READ_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_EXT_RUNNING_Msk))
    {
        GPIO_PortToggle(LED_2, LED_2_PIN_MASK);
    }
    else
    {
        GPIO_PortSet(LED_2, LED_2_PIN_MASK); //close led
    }

    //daisy led / CC100自上而下数第4个灯
    if(g_plc_netcfg.wan.ioExp == WAN_CONFIG_IO_EXP_FEXLINK)
    {
        if(READ_BIT(gMasterRunInfo.flag, SMART_RUN_FLAG_SMART_RUNNING_Msk))
        {
            GPIO_PortToggle(LED_3, LED_3_PIN_MASK);
        }
        else
        {
            GPIO_PortSet(LED_3, LED_3_PIN_MASK); //close led
        }
    }
    else
    {
        if(READ_BIT(gMasterRunInfo.flag, MASTER_RUN_FLAG_SLAVE_RUNNING_Msk))
        {
            GPIO_PortToggle(LED_3, LED_3_PIN_MASK);
        }
        else
        {
            GPIO_PortSet(LED_3, LED_3_PIN_MASK); //close led
        }
    }
}

#else
void start_daisy_task(void)
{
}
void kalyke_daisy_init(void) {}
void kalyke_daisy_stop(void) {}
void daisy_get_info(md_slave_msg_pack *pMsg) {}
void daisy_LAN_send_bin(uint8_t *pBuf, uint16_t len) {}
void daisy_handle_run_led(void) {}
void saveErrorCodeFIFO(uint16_t nErrorCode) {}
#endif

