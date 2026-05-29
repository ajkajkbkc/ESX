/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define TCP_SERVER_LOGIC_2      0

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "lwip/opt.h"

#if LWIP_IPV4 && LWIP_RAW && LWIP_NETCONN && LWIP_DHCP && LWIP_DNS
#include "lwip/ip.h"

//#include "lwip/api.h"
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

#include "ctype.h"

#include "board.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"

#include "kalyke_opts.h"
#include "kalyke_internet_task.h"
#include "kalyke_event.h"

#include "bsp_led.h"
#include "plc_sysblock.h"
#include "plc_element.h"
#include "kalyke_monitor_task.h"
#include "kalyke_json.h"
#include "kalyke_tool.h"
#include "mb.h"
#include "verify_func.h"
#include "bsp_gpio.h"
#include "plc_modbustcpins.h"
#include "plc_parseaddr.h"
#include "kalyke_4G_task.h"
#include "kalyke_4G_TCP_task.h"
#include "kalyke_version.h"
#include "bsp_uart.h"
#include "dev_sign_api.h"
#include "daisy_task.h"
#include "fsl_enet_mdio.h"
#include "fsl_phyksz8081.h"
#include "ether_cat_task.h"
#include "plc_commonfunc.h"
#include "plc_errormsg.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define PHY_ADDRESS_1 (0x02U) /* Phy address of enet port 0. */
#define PHY_ADDRESS_2 (0x01U)

/* IP address configuration. */
#define configIP_ADDR0 192
#define configIP_ADDR1 168
#define configIP_ADDR2 0
#define configIP_ADDR3 102

/* Netmask configuration. */
#define configNET_MASK0 255
#define configNET_MASK1 255
#define configNET_MASK2 255
#define configNET_MASK3 0

/* Gateway address configuration. */
#define configGW_ADDR0 192
#define configGW_ADDR1 168
#define configGW_ADDR2 0
#define configGW_ADDR3 100

/* MAC address configuration. */
#define configMAC_ADDR1                    \
    {                                      \
        0x02, 0x12, 0x13, 0x10, 0x15, 0x11 \
    }
#define configMAC_ADDR2                    \
    {                                      \
        0x02, 0x12, 0x13, 0x10, 0x15, 0x12 \
    }

/* Address of PHY interface. */
#define EXAMPLE_PHY_ADDRESS BOARD_ENET0_PHY_ADDRESS

/* System clock name. */
//#define EXAMPLE_CLOCK_NAME kCLOCK_CoreSysClk

/* ENET clock frequency. */
#define EXAMPLE_CLOCK_FREQ CLOCK_GetFreq(kCLOCK_IpgClk)


/*! @brief MQTT client ID. */
//#define EXAMPLE_MQTT_CLIENT_ID "lwip_client-123"
#define EXAMPLE_MQTT_CLIENT_ID "kalyke009"

/*! @brief MQTT server host name or IP address. */
//#define EXAMPLE_MQTT_SERVER_HOST "test.mosquitto.org"
//#define EXAMPLE_MQTT_SERVER_HOST  "47.92.218.119"
//#define EXAMPLE_MQTT_SERVER_HOST2 "soldier.cloudmqtt.com"//"3.83.223.148"
#define KALYKE_MQTT_SERVER_HOST_NAME  g_plc_netcfg.mqtt.host

/*! @brief MQTT server port number. */
//#define EXAMPLE_MQTT_SERVER_PORT 1883
//#define EXAMPLE_MQTT_SERVER_PORT  2220
//#define EXAMPLE_MQTT_SERVER_PORT2 17380
#define KALYKE_MQTT_PORT g_plc_netcfg.mqtt.port


/*! @brief Stack size of the temporary lwIP initialization thread. */
#define APP_THREAD_STACKSIZE 1024

/*! @brief Priority of the temporary lwIP initialization thread. */
#define APP_THREAD_PRIO 4

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void connect_to_mqtt(void *ctx);
static void connect_to_mqtt2(void *ctx);
static void MicroLink_StartHeart(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t gGUHUAing = 0;
TaskHandle_t gKalykeInternetTaskHandle;
TaskHandle_t gKalykeTCPIPHandle = NULL;

TaskHandle_t gKalykeTCPServerHandle;
TaskHandle_t gKalykeTCPServer2Handle;
TaskHandle_t gKalykeTCPServer3Handle;

TaskHandle_t gKalykeEnet2TCPServerHandle;
TaskHandle_t gKalykeEnet2TCPServer2Handle;
TaskHandle_t gKalykeEnet2TCPServer3Handle;

SemaphoreHandle_t gTcpServerMutex;
SemaphoreHandle_t gEnet2TcpServerMutex;
static QueueHandle_t gMQTTMsgQueueHandle;
static QueueHandle_t gMQTTMsgQueueHandle2;

struct netif fsl_netif0; // For WAN
struct netif fsl_netif1; // For LAN

/*! @brief MQTT client data. */
static mqtt_client_t *mqtt_client = NULL;
static mqtt_client_t *mqtt_client2;

/*! @brief MQTT client information. */
static struct mqtt_connect_client_info_t mqtt_client_info =
{
    .client_id   = "enet0",
    .client_user = "DArimFUr3ZqrYe",
    .client_pass = "teiobCPILLff",
    .keep_alive  = 120,
    .will_topic  = NULL,
    .will_msg    = NULL,
    .will_qos    = 0,
    .will_retain = 0,
#if LWIP_ALTCP && LWIP_ALTCP_TLS
    .tls_config = NULL,
#endif
};
static struct mqtt_connect_client_info_t mqtt_client_info2 =
{
    .client_id   = "ENET2",
    .client_user = "wnlpfeaw",
    .client_pass = "DJ8K_em8Y1mk",
    .keep_alive  = 120,
    .will_topic  = NULL,
    .will_msg    = NULL,
    .will_qos    = 0,
    .will_retain = 0,
#if LWIP_ALTCP && LWIP_ALTCP_TLS
    .tls_config = NULL,
#endif
};

/*! @brief MQTT broker IP address. */
static ip_addr_t mqtt_addr;
static ip_addr_t mqtt_addr2;

/*! @brief Indicates connection to MQTT broker. */
static volatile bool gMqttConnected = false;

static struct netconn *g_connClient[MAX_TCP_CONFIG_ITEM + 1];

bool g_microLink_tcp_connected = false;
static struct netconn *g_connClientENET2;

static struct netconn *g_conn;
static struct netconn *g_connServer;
static struct netconn *g_connServer2;
static struct netconn *g_connServer3;


static struct netconn *g_connEnet2;
static struct netconn *g_connEnet2Server;
static struct netconn *g_connEnet2Server2;
static struct netconn *g_connEnet2Server3;


uint8_t gMODBUSTCPHead[16];
uint8_t gTCPRecvBuf[1024];
uint8_t gTCPSendBuf[1024];
uint8_t gTCPResponseBuf[1024];

/* For ENET2 */
static uint8_t gMODBUSTCPHeadENET2[16];
static uint8_t gTCPRecvBufENET2[1024];
static uint8_t gTCPSendBufENET2[1024];
static uint8_t gTCPResponseBufENET2[1024];


static TimerHandle_t gLoginTimer = NULL;
static TimerHandle_t gHeartTimer = NULL;

volatile mqtt_source_type_t gMqttSource = MQTT_ENET;

volatile bool gWanOK = false;
volatile bool gLanOK = false;
#if (TCP_SERVER_LOGIC_2 == 1)
volatile int8_t gTcpServerCont = 0;
volatile int8_t gEnet2TcpServerCont = 0;
#endif
uint64_t gMcuID;

/*
 * 0 = 透传至COM0
 * 1 = 透传至COM1
 * 2 = 透传至COM2
 * 3 = 透传至WAN
 * 4 = 透传至LAN
 * other = 关闭透传
*/
uint8_t gTouChuan = 0xFF;

static mdio_handle_t mdioHandle2 = {.ops = &enet_ops};
phy_handle_t phyHandle2   = {.phyAddr = PHY_ADDRESS_2, .mdioHandle = &mdioHandle2, .ops = &phyksz8081_ops};

static mdio_handle_t mdioHandle1 = {.ops = &enet_ops};
phy_handle_t phyHandle1   = {.phyAddr = PHY_ADDRESS_1, .mdioHandle = &mdioHandle1, .ops = &phyksz8081_ops};

static uint32_t gPHYCrashNumber[2] = {0};
/*******************************************************************************
 * Code
 ******************************************************************************/
#if 0
static inline void bsp_toggle_led_RUN(void)
{
    GPIO_PortToggle(LED_1, LED_1_PIN_MASK);
}
static inline void bsp_toggle_led_2(void)
{
    GPIO_PortToggle(LED_2, LED_2_PIN_MASK);
}

static inline void bsp_toggle_led_ERR(void)
{
    GPIO_PortToggle(LED_3, LED_3_PIN_MASK);
}

void bsp_led_init(void)
{
    gpio_pin_config_t led_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
    /* Init output LED GPIO. */
    GPIO_PinInit(LED_1, LED_1_PIN, &led_config);
    GPIO_PinInit(LED_2, LED_2_PIN, &led_config);
    GPIO_PinInit(LED_3, LED_3_PIN, &led_config);

    GPIO_PinWrite(LED_1, LED_1_PIN, 1U);
    GPIO_PinWrite(LED_2, LED_2_PIN, 1U);
    GPIO_PinWrite(LED_3, LED_3_PIN, 1U);
}
#endif

void BOARD_InitModuleClock(void)
{
    clock_enet_pll_config_t config = {.enableClkOutput = true, .enableClkOutput25M = false, .loopDivider = 1};
#if (USE_FIRST_ENET == 1)
    config.enableClkOutput = true;
    config.loopDivider = 1;
#endif
#if (USE_SECOND_ENET == 1)
    //config.enableClkOutput25M = true;
    config.enableClkOutput1 = true;
    config.loopDivider1 = 1;
#endif
    CLOCK_InitEnetPll(&config);
}

void delay(void)
{
    volatile uint32_t i = 0;
    for (i = 0; i < 1000000; ++i)
    {
        __asm("NOP"); /* delay */
    }
}

static void tcp_client_send_buffer(unsigned char *pBuff, unsigned short len, unsigned char clientID)
{
    LOGV("internet", "Enter %s(), clientID = %d", __func__, clientID);
    if (len > 128)
    {
        hexdump(pBuff, 128);
    }
    else
    {
        hexdump(pBuff, len);
    }
    gTCPSendBuf[0] = gMODBUSTCPHead[0];
    gTCPSendBuf[1] = gMODBUSTCPHead[1];
    gTCPSendBuf[2] = gMODBUSTCPHead[2];
    gTCPSendBuf[3] = gMODBUSTCPHead[3];
    len -= 2; // Delete CRC
    gTCPSendBuf[4] = len >> 8;
    gTCPSendBuf[5] = len & 0x00FF;

    memcpy(gTCPSendBuf + 6, pBuff, len);
    uint16_t sendLen = len + 6;
    netconn_write(g_connClient[clientID], gTCPSendBuf, sendLen, NETCONN_COPY);

    //netconn_write(g_connClient, gMODBUSTCPHead, 6, NETCONN_COPY);
    //netconn_write(g_connClient, pBuff, len, NETCONN_COPY);
}
static void enet2_tcp_client_send_buffer(unsigned char *pBuff, unsigned short len)
{
    PRINTF("Enter %s()\r\n", __func__);
    if (len > 128)
    {
        hexdump(pBuff, 128);
    }
    else
    {
        hexdump(pBuff, len);
    }
    gTCPSendBufENET2[0] = gMODBUSTCPHeadENET2[0];
    gTCPSendBufENET2[1] = gMODBUSTCPHeadENET2[1];
    gTCPSendBufENET2[2] = gMODBUSTCPHeadENET2[2];
    gTCPSendBufENET2[3] = gMODBUSTCPHeadENET2[3];
    len -= 2; // Delete CRC
    gTCPSendBufENET2[4] = len >> 8;
    gTCPSendBufENET2[5] = len & 0x00FF;

    memcpy(gTCPSendBufENET2 + 6, pBuff, len);
    uint16_t sendLen = len + 6;
    netconn_write(g_connClientENET2, gTCPSendBufENET2, sendLen, NETCONN_COPY);
}

static void tcp_server_send_buffer(unsigned char *pBuff, unsigned short len)
{
    //PRINTF("Enter %s()\r\n", __func__);
    if (len > 512)
    {
        //hexdump(pBuff, 512);
    }
    else
    {
        //hexdump(pBuff, len);
    }
    gTCPSendBuf[0] = gMODBUSTCPHead[0];
    gTCPSendBuf[1] = gMODBUSTCPHead[1];
    gTCPSendBuf[2] = gMODBUSTCPHead[2];
    gTCPSendBuf[3] = gMODBUSTCPHead[3];
    len -= 2; // Delete CRC
    gTCPSendBuf[4] = len >> 8;
    gTCPSendBuf[5] = len & 0x00FF;

    memcpy(gTCPSendBuf + 6, pBuff, len);
    uint16_t sendLen = len + 6;
    netconn_write(g_connServer, gTCPSendBuf, sendLen, NETCONN_COPY);
}
static void tcp_server_send_buffer2(unsigned char *pBuff, unsigned short len)
{
    //PRINTF("Enter %s()\r\n", __func__);
    if (len > 512)
    {
        //hexdump(pBuff, 512);
    }
    else
    {
        //hexdump(pBuff, len);
    }
    gTCPSendBuf[0] = gMODBUSTCPHead[0];
    gTCPSendBuf[1] = gMODBUSTCPHead[1];
    gTCPSendBuf[2] = gMODBUSTCPHead[2];
    gTCPSendBuf[3] = gMODBUSTCPHead[3];
    len -= 2; // Delete CRC
    gTCPSendBuf[4] = len >> 8;
    gTCPSendBuf[5] = len & 0x00FF;

    memcpy(gTCPSendBuf + 6, pBuff, len);
    uint16_t sendLen = len + 6;
    netconn_write(g_connServer2, gTCPSendBuf, sendLen, NETCONN_COPY);
}

static void tcp_server_send_buffer3(unsigned char *pBuff, unsigned short len)
{
    //PRINTF("Enter %s()\r\n", __func__);
    if (len > 512)
    {
        //hexdump(pBuff, 512);
    }
    else
    {
        //hexdump(pBuff, len);
    }
    gTCPSendBuf[0] = gMODBUSTCPHead[0];
    gTCPSendBuf[1] = gMODBUSTCPHead[1];
    gTCPSendBuf[2] = gMODBUSTCPHead[2];
    gTCPSendBuf[3] = gMODBUSTCPHead[3];
    len -= 2; // Delete CRC
    gTCPSendBuf[4] = len >> 8;
    gTCPSendBuf[5] = len & 0x00FF;

    memcpy(gTCPSendBuf + 6, pBuff, len);
    uint16_t sendLen = len + 6;
    netconn_write(g_connServer3, gTCPSendBuf, sendLen, NETCONN_COPY);
}

bool is_MicroLinkLogin(uint8_t *data, uint16_t len)
{
    if (data[0] == 0x00 && data[1] == 0x10 && data[2] == 0x24 && data[3] == 0x62)
    {
        LOGV("MicroLinkClient", "I am login respones");
        if (data[8] == 0) // Success
        {
            LOGD("MicroLinkClient", "Login success!");
            return true;
        }
        else if (data[8] == 1)// Device does not exist
        {
            LOGD("MicroLinkClient", "Device does not exist!");
        }
        else if (data[8] == 2)// Device invalid
        {
            LOGD("MicroLinkClient", "Device invalid!");
        }
    }
    else
    {
        LOGE("MicroLinkClient", "And this not MicroLink login response");
    }
    return false;
}

static QueueHandle_t gTouchuanQueueHandle;
static void touchuan_tcp_task(void *arg)
{
    touchuan_msg_st msg;
    gTouchuanQueueHandle = xQueueCreate(10, sizeof(touchuan_msg_st));
    while (1)
    {
        if (xQueueReceive(gTouchuanQueueHandle, &msg, portMAX_DELAY) == pdFALSE)
        {
            continue;
        }
        //vTaskDelay(100);
        MicroLink_touchuan_send_data_2_tcp(msg.dataBuff, msg.msgLength);
        //vPortFree(msg.dataBuff);
    }
}

static void start_touchuan_task(void)
{
    BaseType_t ret = xTaskCreate((TaskFunction_t)touchuan_tcp_task,
                                 (const char *)"touchuan_tcp_task",
                                 1024,
                                 (void *)(uint32_t)0,
                                 INTERNET_TASK_PRIO,
                                 NULL);
    if (ret != pdPASS)
    {
        LOGE("internet", "Create touchuan_tcp_task ERROR!");
    }
}

void MicroLink_touchuan_send_data_2_tcp(uint8_t *pData, uint16_t len)
{
    LOGV("MicroLinkClient", "Enter %s(), g_connClient[3] = 0x%08X", __func__, g_connClient[3]);
    if (len > 128)
    {
        hexdump(pData, 128);
    }
    else
    {
        hexdump(pData, len);
    }
    if (g_microLink_tcp_connected == false)
    {
        LOGW("MicroLinkClient", "MicroLink Server not connect...");
        return;
    }
    memcpy(gTCPSendBuf, pData, len);
    netconn_write(g_connClient[3], gTCPSendBuf, len, NETCONN_COPY);
}

static void MicroLink_touchuan_send_data_2_uart(uint8_t *pData, uint16_t len)
{
    if (gTouChuan == 0)
    {
        bsp_com0_send_data(pData, len);
    }
    else if (gTouChuan == 1)
    {
        bsp_com1_send_data(pData, len);
    }
    else if (gTouChuan == 2)
    {
        bsp_com2_send_data(pData, len);
    }
}

static void add_eth_head(struct pbuf *p)
{
    uint8_t ethBroadcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    
    uint16_t eth_type_be = lwip_htons(ETHTYPE_ETHER_KALYKE);
    //uint16_t eth_type_be = lwip_htons(ETHTYPE_IP);
    LOGD("add_eth_head", "eth_type_be = 0x%04X", eth_type_be);

    pbuf_add_header(p, SIZEOF_ETH_HDR);
    
    struct eth_hdr *ethhdr = (struct eth_hdr *)p->payload;
    ethhdr->type = eth_type_be;
    SMEMCPY(&ethhdr->dest, (struct eth_addr *)(ethBroadcast), ETH_HWADDR_LEN);
    SMEMCPY(&ethhdr->src,  (struct eth_addr *)(fsl_netif1.hwaddr), ETH_HWADDR_LEN);
}

static uint8_t tcpHandleReceivedData(uint8_t *data, uint16_t len, uint8_t isClient, uint8_t clientID, uint8_t whichServer)
{
//    LOGV("tcpHandleReceivedData", "Enter %s(), len = %u, isClient = %u, whichServer = %u, gTouChuan = %u", __func__, len, isClient, whichServer, gTouChuan);

#if (KALYKE_TOUCHUAN_WAN_LAN == 1)
    if (gTouChuan == 4) //透传至LAN
    {
        if (len > 512)
        {
             hexdump(data, 512);
        }
        else
        {
            hexdump(data, len);
        }

        struct pbuf *p = pbuf_alloc(PBUF_LINK, len + sizeof(struct eth_hdr), PBUF_RAM);
        memcpy(p->payload, data, len);
        add_eth_head(p);

        ethernet_output_daisy(&fsl_netif1, p);//Send to LAN

        pbuf_free(p);
        return 1;
    }
#endif

    md_slave_msg_pack ltv_TCPModbusPack = { 0x0, };
    /*申请发送缓冲区*/
    ltv_TCPModbusPack.mcp_RespBuff = gTCPResponseBuf;//(unsigned char *)pvPortMalloc(sizeof(unsigned char) * 1024);
    /*回掉函数赋值*/
    if (isClient == 1)
    {
        ltv_TCPModbusPack.tcp_resp_func = tcp_client_send_buffer;
        if (gTouChuan != 0xFF)
        {
            uint8_t closeTouChuan[9] = {0x00,0x00,0x00,0x00,0x00,0x03, 0x01, 0x6A, 0x7E};
            if (len < 9)
            {
                hexdump(data, len);
                MicroLink_touchuan_send_data_2_uart(data, len);
                return 1;
            }
            //hexdump(closeTouChuan, sizeof(closeTouChuan));
            hexdump(data, len);
            if ((memcmp(data + 2, closeTouChuan + 2, 4) == 0 && data[7] == 0x6A && data[8] == MB_CTRL_TOU_CHUAN_CLOSE))
            {
                goto HANDLES;
            }
            #if 0
            else if (len == 9 && (data[6] == 0x7B && data[7] == 0x23 && data[8] == 0x7D))// 心跳包
            {
                return 1; //直接返回
            }
            #endif
            else
            {
                MicroLink_touchuan_send_data_2_uart(data, len);
                return 1;
            }
        }
    }
    else
    {
        if (whichServer == 1)
        {
            ltv_TCPModbusPack.resp_func = tcp_server_send_buffer;
        }
        else if (whichServer == 2)
        {
            ltv_TCPModbusPack.resp_func = tcp_server_send_buffer2;
        }
        else
        {
            ltv_TCPModbusPack.resp_func = tcp_server_send_buffer3;
        }
    }
HANDLES:
    /*标定信息发送者*/
    ltv_TCPModbusPack.mcv_Sender = MB_SENDER_TCP;
    gtp_ModbusSlaveDiagInfo[MB_SENDER_TCP].mcv_SlaveId = 1;

    //接收到的数据 转发给串口1，来达到透传的目的
    //comSendBuf(COM1,data,len);
    if (len > 512)
    {
        //hexdump(data, 512);
    }
    else
    {
        //hexdump(data, len);
    }
    uint16_t allLen = 0;
    uint8_t modbusTCP[2] = {0x00, 0x00};
    uint8_t modbusTCP2[2] = {0x30, 0x00};
    uint16_t recvLen = 0;
    uint8_t *pChar = data;
    if (memcmp(modbusTCP,  pChar + MB_TCP_PID, 2) == 0 ||
        memcmp(modbusTCP2, pChar + MB_TCP_PID, 2) == 0) // This is MODBUSTCP protocol
    {
        //LOGI("tcpHandleReceivedData", "I am MODBUSTCP protocol!");
        allLen = (pChar[MB_TCP_LEN] << 8) | pChar[MB_TCP_LEN + 1];
        allLen += MB_TCP_UID;
        recvLen = 0;
        memcpy(gMODBUSTCPHead, data, MB_TCP_UID);
        //PRINTF("OH, Got the head!  allLen = %u\r\n", allLen);
    }
    else
    {
        LOGE("tcpHandleReceivedData", "This is not MODBUSTCP protocol!");
        if (is_MicroLinkLogin(data, len))
        {
            SET_SD_ELEMENT_VALUE(SD224, 1);
            MicroLink_StartHeart();
        }
        return 0;
    }
    memcpy(gTCPRecvBuf + recvLen, data, len);
    recvLen += len;
    //PRINTF("%s: recvLen = %u\r\n", __func__, recvLen);

    if (recvLen < allLen)
    {
        return 0;
    }

    uint16_t crc16 = calc_crc16(gTCPRecvBuf + MB_TCP_UID, recvLen - MB_TCP_UID);
    gTCPRecvBuf[recvLen++] = crc16 & 0xFF;
    gTCPRecvBuf[recvLen++] = crc16 >> 8;
    //LOGW("tcpHandleReceivedData", "crc16 = %X", crc16);
    // We have got all data.
    ltv_TCPModbusPack.mcp_ReceiveBuff = gTCPRecvBuf + MB_TCP_UID;
    ltv_TCPModbusPack.msv_ReceiveLen = recvLen - MB_TCP_UID;
    ltv_TCPModbusPack.mcv_IsBroadcastInfo = 0;
    ltv_TCPModbusPack.isTcpClient = isClient == 1 ? 1 : 0;
    ltv_TCPModbusPack.clientID = clientID;
    //hexdump(ltv_TCPModbusPack.mcp_ReceiveBuff, ltv_TCPModbusPack.msv_ReceiveLen);
    mb_slave_msg_handler(&ltv_TCPModbusPack);
    return 1;
}

static void enet2_tcp_server_send_buffer(unsigned char *pBuff, unsigned short len)
{
    PRINTF("Enter %s()\r\n", __func__);
    if (len > 512)
    {
        hexdump(pBuff, 512);
    }
    else
    {
        hexdump(pBuff, len);
    }
    gTCPSendBufENET2[0] = gMODBUSTCPHeadENET2[0];
    gTCPSendBufENET2[1] = gMODBUSTCPHeadENET2[1];
    gTCPSendBufENET2[2] = gMODBUSTCPHeadENET2[2];
    gTCPSendBufENET2[3] = gMODBUSTCPHeadENET2[3];
    len -= 2; // Delete CRC
    gTCPSendBufENET2[4] = len >> 8;
    gTCPSendBufENET2[5] = len & 0x00FF;

    memcpy(gTCPSendBufENET2 + 6, pBuff, len);
    uint16_t sendLen = len + 6;
    netconn_write(g_connEnet2Server, gTCPSendBufENET2, sendLen, NETCONN_COPY);
}

static void enet2_tcp_server_send_buffer2(unsigned char *pBuff, unsigned short len)
{
    PRINTF("Enter %s()\r\n", __func__);
    if (len > 512)
    {
        hexdump(pBuff, 512);
    }
    else
    {
        hexdump(pBuff, len);
    }
    gTCPSendBufENET2[0] = gMODBUSTCPHeadENET2[0];
    gTCPSendBufENET2[1] = gMODBUSTCPHeadENET2[1];
    gTCPSendBufENET2[2] = gMODBUSTCPHeadENET2[2];
    gTCPSendBufENET2[3] = gMODBUSTCPHeadENET2[3];
    len -= 2; // Delete CRC
    gTCPSendBufENET2[4] = len >> 8;
    gTCPSendBufENET2[5] = len & 0x00FF;

    memcpy(gTCPSendBufENET2 + 6, pBuff, len);
    uint16_t sendLen = len + 6;
    netconn_write(g_connEnet2Server2, gTCPSendBufENET2, sendLen, NETCONN_COPY);
}

static void enet2_tcp_server_send_buffer3(unsigned char *pBuff, unsigned short len)
{
    PRINTF("Enter %s()\r\n", __func__);
    if (len > 512)
    {
        hexdump(pBuff, 512);
    }
    else
    {
        hexdump(pBuff, len);
    }
    gTCPSendBufENET2[0] = gMODBUSTCPHeadENET2[0];
    gTCPSendBufENET2[1] = gMODBUSTCPHeadENET2[1];
    gTCPSendBufENET2[2] = gMODBUSTCPHeadENET2[2];
    gTCPSendBufENET2[3] = gMODBUSTCPHeadENET2[3];
    len -= 2; // Delete CRC
    gTCPSendBufENET2[4] = len >> 8;
    gTCPSendBufENET2[5] = len & 0x00FF;

    memcpy(gTCPSendBufENET2 + 6, pBuff, len);
    uint16_t sendLen = len + 6;
    netconn_write(g_connEnet2Server3, gTCPSendBufENET2, sendLen, NETCONN_COPY);
}

static uint8_t enet2_tcpHandleReceivedData(uint8_t *data, uint16_t len, uint8_t isClient, uint8_t whichServer)
{
    LOGV("enet2_tcpHandleReceivedData", "Enter %s(), len = %u, isClient = %u, whichServer = %u", __func__, len, isClient, whichServer);
    md_slave_msg_pack ltv_TCPModbusPack = { 0x0, };
    /*申请发送缓冲区*/
    ltv_TCPModbusPack.mcp_RespBuff = gTCPResponseBufENET2;//(unsigned char *)pvPortMalloc(sizeof(unsigned char) * 1024);
    /*回掉函数赋值*/
    if (isClient == 1)
    {
        ltv_TCPModbusPack.resp_func = enet2_tcp_client_send_buffer;
    }
    else
    {
        if (whichServer == 1)
        {
            ltv_TCPModbusPack.resp_func = enet2_tcp_server_send_buffer;
        }
        else if (whichServer == 2)
        {
            ltv_TCPModbusPack.resp_func = enet2_tcp_server_send_buffer2;
        }
        else
        {
            ltv_TCPModbusPack.resp_func = enet2_tcp_server_send_buffer3;
        }
    }
    /*标定信息发送者*/
    ltv_TCPModbusPack.mcv_Sender = MB_SENDER_TCP;
    gtp_ModbusSlaveDiagInfo[MB_SENDER_TCP].mcv_SlaveId = 1;

    //接收到的数据 转发给串口1，来达到透传的目的
    //comSendBuf(COM1,data,len);
    if (len > 512)
    {
        hexdump(data, 512);
    }
    else
    {
        hexdump(data, len);
    }
    uint16_t allLen = 0;
    uint8_t modbusTCP[2] = {0x00, 0x00};
    uint8_t modbusTCP2[2] = {0x30, 0x00};
    uint16_t recvLen = 0;
    uint8_t *pChar = data;
    if (memcmp(modbusTCP, pChar + MB_TCP_PID, 2) == 0 ||
            memcmp(modbusTCP2, pChar + MB_TCP_PID, 2) == 0) // This is MODBUSTCP protocol
    {
        allLen = (pChar[MB_TCP_LEN] << 8) | pChar[MB_TCP_LEN + 1];
        allLen += MB_TCP_UID;
        recvLen = 0;
        memcpy(gMODBUSTCPHeadENET2, data, MB_TCP_UID);
        LOGV("enet2_tcpHandleReceivedData", "OH, Got the head!  allLen = %u\r\n", allLen);
    }
    else
    {
        LOGE("enet2_tcpHandleReceivedData", "This is not MODBUSTCP protocol!");
        return 0;
    }
    memcpy(gTCPRecvBufENET2 + recvLen, data, len);
    recvLen += len;
    LOGI("enet2_tcpHandleReceivedData", "%s: recvLen = %u\r\n", __func__, recvLen);

    if (recvLen < allLen)
    {
        return 0;
    }

    uint16_t crc16 = calc_crc16(gTCPRecvBufENET2 + MB_TCP_UID, recvLen - MB_TCP_UID);
    gTCPRecvBufENET2[recvLen++] = crc16 & 0xFF;
    gTCPRecvBufENET2[recvLen++] = crc16 >> 8;
    LOGW("enet2_tcpHandleReceivedData", "crc16 = %X", crc16);
    // We have got all data.
    ltv_TCPModbusPack.mcp_ReceiveBuff = gTCPRecvBufENET2 + MB_TCP_UID;
    ltv_TCPModbusPack.msv_ReceiveLen = recvLen - MB_TCP_UID;
    ltv_TCPModbusPack.mcv_IsBroadcastInfo = 0;

    mb_slave_msg_handler(&ltv_TCPModbusPack);
    return 1;
}

static void login_kalyke(TimerHandle_t ltv_TimeHandle)
{
    LOGV("MicroLinkClient", "Enter %s()", __func__);

    uint16_t sendLen = 0;
    char tcpSendBuf[128] = {0};
    tcpSendBuf[0] = 0x00;
    tcpSendBuf[1] = 0x10;
    tcpSendBuf[2] = 0x24;
    tcpSendBuf[3] = 0x62;

    uint16_t len = 26;
    tcpSendBuf[4] = len & 0xFF;
    tcpSendBuf[5] = len >> 8;
    tcpSendBuf[6] = 0x01;
    tcpSendBuf[7] = 0x02;

    strcpy(&tcpSendBuf[8], "MiLink");

    memcpy(&tcpSendBuf[20], gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId, 12);

    sendLen = 32;
    netconn_write(g_connClient[3], tcpSendBuf, sendLen, NETCONN_COPY);
}

static void MicroLink_login(void)
{
    LOGV("MicroLinkClient", "Enter %s()", __func__);
    if (gLoginTimer == NULL)
    {
        gLoginTimer = xTimerCreate((const char *)"login_microlink",
                                       (TickType_t  )1000 / portTICK_PERIOD_MS,
                                       (UBaseType_t )pdFALSE,
                                       (void *      )3,
                                       (TimerCallbackFunction_t)login_kalyke);
        xTimerStart(gLoginTimer, 100);
    }
    else
    {
        xTimerStart(gLoginTimer, 100);
    }
}

static void MicroLink_heart(TimerHandle_t ltv_TimeHandle)
{
    uint16_t ti = gKalykeSecondTick;
    LOGV("MicroLinkClient", "Enter %s()", __func__);
    char tcpSendBuf[16] = {0};

    tcpSendBuf[0] = ti >> 8;  // Transaction Identifier - 2 Byte
    tcpSendBuf[1] = ti & 0xFF;// Transaction Identifier - 2 Byte
    tcpSendBuf[2] = 0; // Protocol Identifier - 2 Byte
    tcpSendBuf[3] = 0; // Protocol Identifier - 2 Byte
    tcpSendBuf[4] = 0; // Number of bytes - 2 Byte
    tcpSendBuf[5] = 3; // Number of bytes - 2 Byte
    tcpSendBuf[6] = '{'; //0x7B
    tcpSendBuf[7] = '#'; //0x23
    tcpSendBuf[8] = '}'; //0x7D

    netconn_write(g_connClient[3], tcpSendBuf, 9, NETCONN_COPY);
}

static void MicroLink_StartHeart(void)
{
    LOGV("MicroLinkClient", "Enter %s()", __func__);
    if (gHeartTimer == NULL)
    {
        gHeartTimer = xTimerCreate((const char *)"login_microlink",
                                       (TickType_t  )60000 / portTICK_PERIOD_MS,
                                       (UBaseType_t )pdTRUE,
                                       (void *      )3,
                                       (TimerCallbackFunction_t)MicroLink_heart);
        xTimerStart(gHeartTimer, 10);
    }
    else
    {
        xTimerStart(gHeartTimer, 10);
    }
}

static void MicroLink_StopHeart(void)
{
    LOGV("MicroLinkClient", "Enter %s()", __func__);
    if (gHeartTimer)
    {
        xTimerStop(gHeartTimer, 100);
    }
}

static void MicroLink_tcp_client_thread(void *arg)
{
    uint8_t clientID = (uint32_t)arg;
    
    LOGV("MicroLinkClient", "Enter %s(), clientID = %u", __func__, clientID);
    struct netbuf *buf;
    void *data;
    u16_t len;
    err_t err;
    ip4_addr_t ipTarget;
    uint16_t portTarget = g_plc_netcfg.cloud.port;
    
    if (g_plc_netcfg.cloud.ipOrDomain == 0)
    {
        ipTarget.addr = g_plc_netcfg.cloud.ip.addr;
        goto GOT_IP;
    }
    while(1)
    {
        LOGV("MicroLinkClient", "Resolving \"%s\"...", g_plc_netcfg.cloud.domain);
        err_t err = netconn_gethostbyname((char*)g_plc_netcfg.cloud.domain, &ipTarget);
        if (err != ERR_OK)
        {
            LOGE("MicroLinkClient", "Failed to obtain IP address: %d.", err);
            vTaskDelay(10000);
            continue;
        }
        break;
    }

GOT_IP:
    while(1)
    {
        LOGD("MicroLinkClient", "Connecting to server : %s:%u", ipaddr_ntoa(&ipTarget), portTarget);
        g_connClient[clientID] = netconn_new(NETCONN_TCP);
        LOGV("MicroLinkClient", "g_connClient[%u] = 0x%08X\r\n", clientID, g_connClient[clientID]);
        if (g_connClient[clientID] != NULL)
        {
            //netconn_bind(g_connClient[clientID], &g_plc_netcfg.wan.ip, 0);
            err = netconn_connect(g_connClient[clientID], &ipTarget, portTarget);
            if(err == ERR_OK)
            {
                g_microLink_tcp_connected = true;
                #if 0
                err = tcpip_callback(login_kalyke, NULL);
                LOGI("MicroLinkClient", "err = %d", err);
                #else
                MicroLink_login();
                #endif

                LOGD("MicroLinkClient", "Server: %s:%u connected success!", ipaddr_ntoa(&ipTarget), portTarget);
                while ((err = netconn_recv(g_connClient[clientID], &buf)) == ERR_OK)
                {
                    do
                    {
                        netbuf_data(buf, &data, &len);
                        xSemaphoreTake(gTcpServerMutex, portMAX_DELAY);
                        tcpHandleReceivedData(data, len, 1, clientID, 0);
                        xSemaphoreGive(gTcpServerMutex);
                } while (netbuf_next(buf) >= 0 && gGUHUAing == 0);
                    netbuf_delete(buf);
                }

                gTouChuan = 0xFF;
                SET_SD_ELEMENT_VALUE(SD224, 0);
                netbuf_delete(buf);
                LOGE("MicroLinkClient", "g_connClient[%d], Disconnected from server: %s:%u !!! err = %d",clientID, ipaddr_ntoa(&ipTarget), portTarget, err);
                MicroLink_StopHeart();
                netconn_close(g_connClient[clientID]);
                netconn_delete(g_connClient[clientID]);
                g_connClient[clientID] = NULL;
                g_microLink_tcp_connected = false;
                vTaskDelay(5000 / portTICK_PERIOD_MS);
            }
            else
            {
                g_microLink_tcp_connected = false;
                LOGE("MicroLinkClient", "Connect server: %s:%u ERROR!! err = %d", ipaddr_ntoa(&ipTarget), portTarget, err);
                MicroLink_StopHeart();
                netconn_close(g_connClient[clientID]);
                netconn_delete(g_connClient[clientID]);
                g_connClient[clientID] = NULL;
                vTaskDelay(5001 / portTICK_PERIOD_MS);
            }
        }
        else // (g_connClient == NULL)
        {
            MicroLink_StopHeart();
            LOGW("MicroLinkClient", "Can not create TCP connection.\r\n");
            vTaskDelay(10002 / portTICK_PERIOD_MS);
        }
    }
}

static const char *TAGTCPS1 = "tcpServerThread1";
static void tcpServerThread(void *arg)
{
    SET_SD_ELEMENT_VALUE(SD260, 2);
   //struct netconn *conn;
    err_t err;
    ip_addr_t address;
    u16_t port;
    LWIP_UNUSED_ARG(arg);
    LOGV(TAGTCPS1, "Enter %s()", __func__);
    xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_GOT_IP_TCP_SERVER, pdTRUE, pdFALSE, portMAX_DELAY);
    vTaskDelay(1001 / portTICK_PERIOD_MS);
LISTEN_ME_AGAIN:

    /* Create a new connection identifier. */
    g_conn = netconn_new(NETCONN_TCP);
    address.addr = g_plc_netcfg.wan.ip.addr;
    /* Bind connection to well known port number 502. */
    netconn_bind(g_conn, &address, 502);

    if (g_conn == NULL)
    {
        SET_SD_ELEMENT_VALUE(SD260, 1);
        LOGE(TAGTCPS1, "conn == NULL, so just return....Line:%d", __LINE__);
        vTaskDelete(NULL);
        return;
    }
    /* Tell connection to go into listening mode. */
    //netconn_listen(g_conn);
    netconn_set_keepalive(g_conn);
    netconn_listen_with_backlog(g_conn, 1);
    LOGV(TAGTCPS1, "Line:%d, listen on netconn : 0x%08X", __LINE__, g_conn);

    while (1)
    {
        /* Grab new connection. */
        LOGW(TAGTCPS1, "Wait remote to connect ME: %s:%u", ipaddr_ntoa(&address), 502);
        err = netconn_accept(g_conn, &g_connServer);
        LOGV(TAGTCPS1, "netconn_accept return: %d", err);
        SET_SD_ELEMENT_VALUE(SD260, err);
        /* Process the new connection. */
        if (err == ERR_OK)
        {
            LOGD(TAGTCPS1, "accepted new connection : 0x%08X", g_connServer);
        #if (LOG_OPEN == 1)
            ip_addr_t adr;
            netconn_peer(g_connServer, &adr, &port);
            LOGV(TAGTCPS1, "peer IPv4 Address  : %s,  port: %u\r\n", ipaddr_ntoa(&adr), port);
            netconn_addr(g_connServer, &adr, &port);
            LOGW(TAGTCPS1, "local IPv4 Address : %s,  port: %u\r\n", ipaddr_ntoa(&adr), port);
        #endif
            struct netbuf *buf;
            void *data;
            u16_t len;

            while ((err = netconn_recv(g_connServer, &buf)) == ERR_OK)
            {
                do
                {
                    netbuf_data(buf, &data, &len);
                    xSemaphoreTake(gTcpServerMutex, portMAX_DELAY);
                    tcpHandleReceivedData(data, len, 0, 0, 1);
                    xSemaphoreGive(gTcpServerMutex);
                } while (netbuf_next(buf) >= 0 && gGUHUAing == 0);
                netbuf_delete(buf);
            }
            LOGE(TAGTCPS1, "netconn_recv return: %d", err);
            SET_SD_ELEMENT_VALUE(SD260, err);
            /* Close connection and discard connection identifier. */
            netconn_close(g_connServer);
            netconn_delete(g_connServer);
        }
//        vTaskDelay(1001);
    }
}

static const char *TAGTCPS2 = "tcpServerThread2";
static void tcpServerThread2(void *arg)
{
    SET_SD_ELEMENT_VALUE(SD261, 2);
    err_t err;
    ip_addr_t addr;
    u16_t port;
    LWIP_UNUSED_ARG(arg);

    LOGV(TAGTCPS2, "Enter %s()", __func__);
    xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_GOT_IP_TCP_SERVER2, pdTRUE, pdFALSE, portMAX_DELAY);
    vTaskDelay(2001 / portTICK_PERIOD_MS);

    if (g_conn == NULL)
    {
        LOGE(TAGTCPS2, "conn == NULL, so just return.");
        SET_SD_ELEMENT_VALUE(SD261, 1);
        vTaskDelete(NULL);
        return;
    }

    while (1)
    {
        /* Grab new connection. */
        LOGW(TAGTCPS2, "Wait remote to connect ME!!!");
        err = netconn_accept(g_conn, &g_connServer2);
        LOGV(TAGTCPS2, "netconn_accept return: %d", err);
        SET_SD_ELEMENT_VALUE(SD261, err);
        /* Process the new connection. */
        if (err == ERR_OK)
        {
            LOGV(TAGTCPS2, "accepted new connection : 0x%08X", g_connServer2);

        #if (LOG_OPEN == 1)
            netconn_peer(g_connServer2, &addr, &port);
            LOGV(TAGTCPS2, "peer IPv4 Address  : %s,  port: %u", ipaddr_ntoa(&addr), port);
            netconn_addr(g_connServer2, &addr, &port);
            LOGW(TAGTCPS2, "IPv4 Address  : %s,  port: %u\r\n", ipaddr_ntoa(&addr), port);
        #endif
            
            struct netbuf *buf;
            void *data;
            u16_t len;

            while ((err = netconn_recv(g_connServer2, &buf)) == ERR_OK)
            {
                do
                {
                    netbuf_data(buf, &data, &len);
                    xSemaphoreTake(gTcpServerMutex, portMAX_DELAY);
                    tcpHandleReceivedData(data, len, 0, 0, 2);
                    xSemaphoreGive(gTcpServerMutex);
                } while (netbuf_next(buf) >= 0 && gGUHUAing == 0);
                netbuf_delete(buf);
            }
            LOGE(TAGTCPS2, "netconn_recv return: %d", err);
            SET_SD_ELEMENT_VALUE(SD261, err);
            /* Close connection and discard connection identifier. */
            netconn_close(g_connServer2);
            netconn_delete(g_connServer2);
        }
//        vTaskDelay(2001);
    }
}

static const char *TAGTCPS3 = "tcpServerThread3";
static void tcpServerThread3(void *arg)
{
    SET_SD_ELEMENT_VALUE(SD262, 2);
    //struct netconn *conn;
    err_t err;
    ip_addr_t addr;
    u16_t port;
    LWIP_UNUSED_ARG(arg);

    LOGV(TAGTCPS3, "Enter %s()", __func__);
    xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_GOT_IP_TCP_SERVER3, pdTRUE, pdFALSE, portMAX_DELAY);
    vTaskDelay(3001 / portTICK_PERIOD_MS);

    if (g_conn == NULL)
    {
        LOGE(TAGTCPS3, "conn == NULL, so just return.");
        SET_SD_ELEMENT_VALUE(SD262, 1);
        vTaskDelete(NULL);
        return;
    }

    while (1)
    {
        LOGW(TAGTCPS3, "Wait remote to connect ME!!!");
        err = netconn_accept(g_conn, &g_connServer3);
        LOGV(TAGTCPS3, "netconn_accept return: %d", err);
        SET_SD_ELEMENT_VALUE(SD262, err);

        if (err == ERR_OK)
        {
            LOGV(TAGTCPS3, "accepted new connection : 0x%08X", g_connServer3);
        #if (LOG_OPEN == 1)
            netconn_peer(g_connServer3, &addr, &port);
            LOGV(TAGTCPS3, "peer IPv4 Address  : %s,  port: %u\r\n", ipaddr_ntoa(&addr), port);
            netconn_addr(g_connServer3, &addr, &port);
            LOGW(TAGTCPS3, "IPv4 Address  : %s,  port: %u\r\n", ipaddr_ntoa(&addr), port);
        #endif

            struct netbuf *buf;
            void *data;
            u16_t len;

            while ((err = netconn_recv(g_connServer3, &buf)) == ERR_OK)
            {
                do
                {
                    netbuf_data(buf, &data, &len);
                    xSemaphoreTake(gTcpServerMutex, portMAX_DELAY);
                    tcpHandleReceivedData(data, len, 0, 0, 3);
                    xSemaphoreGive(gTcpServerMutex);
                } while (netbuf_next(buf) >= 0 && gGUHUAing == 0);
                netbuf_delete(buf);
            }
            LOGE(TAGTCPS3, "netconn_recv return: %d", err);
            SET_SD_ELEMENT_VALUE(SD262, err);
            /* Close connection and discard connection identifier. */
            netconn_close(g_connServer3);
            netconn_delete(g_connServer3);
        }
//        vTaskDelay(3001);
    }
}

static const char *TAGE2TCPS1 = "enet2_tcpServerThread1";
static void enet2_tcpServerThread(void *arg)
{
    SET_SD_ELEMENT_VALUE(SD263, 2);
    err_t err;
    ip_addr_t address;
    u16_t port;
    LWIP_UNUSED_ARG(arg);

    LOGV(TAGE2TCPS1, "Enter %s()", __func__);
    /* Create a new connection identifier. */
    /* Bind connection to well known port number 502. */
#if LWIP_IPV6
    g_connEnet2 = netconn_new(NETCONN_TCP_IPV6);
    netconn_bind(conn, IP6_ADDR_ANY, 7);
#else /* LWIP_IPV6 */
    g_connEnet2 = netconn_new(NETCONN_TCP);
    address.addr = g_plc_netcfg.lan.ip.addr;
    netconn_bind(g_connEnet2, &address, 502);
#endif /* LWIP_IPV6 */
    //LWIP_ERROR("tcpecho: invalid conn", (conn != NULL), return;);
    if (g_connEnet2 == NULL)
    {
        LOGE(TAGE2TCPS1, "g_connEnet2 == NULL, so just return.");
        vTaskDelete(NULL);
        SET_SD_ELEMENT_VALUE(SD263, 1);
        return;
    }
    /* Tell connection to go into listening mode. */
    //netconn_listen(g_connEnet2);
    netconn_set_keepalive(g_connEnet2);
    netconn_listen_with_backlog(g_connEnet2, 1);
    LOGV(TAGE2TCPS1, "listen on netconn : 0x%08X", g_connEnet2);
    //ip_set_option(g_connEnet2->pcb.tcp, SOF_KEEPALIVE);
    //g_connEnet2->pcb.tcp->so_options;// = (u8_t)((g_connEnet2.pcb.tcp)->so_options | (SOF_KEEPALIVE)))

    while (1)
    {
        /* Grab new connection. */
        LOGW(TAGE2TCPS1, "Wait remote to connect ME: %s:%u", ipaddr_ntoa(&address), 502);
        err = netconn_accept(g_connEnet2, &g_connEnet2Server);
        LOGV(TAGE2TCPS1, "netconn_accept return: %d", err);
        netconn_peer(g_connEnet2Server, &address, &port);
        LOGV(TAGE2TCPS1, "peer IPv4 Address  : %s,  port: %u\r\n", ipaddr_ntoa(&address), port);
        netconn_addr(g_connEnet2Server, &address, &port);
        LOGW(TAGE2TCPS1, "local IPv4 Address : %s,  port: %u\r\n", ipaddr_ntoa(&address), port);
        SET_SD_ELEMENT_VALUE(SD263, err);
        /* Process the new connection. */
        if (err == ERR_OK)
        {
            LOGV(TAGE2TCPS1, "accepted new connection : 0x%08X", g_connEnet2Server);
            struct netbuf *buf;
            void *data;
            u16_t len;

            while ((err = netconn_recv(g_connEnet2Server, &buf)) == ERR_OK)
            {
                do
                {
                    netbuf_data(buf, &data, &len);
                    xSemaphoreTake(gEnet2TcpServerMutex, portMAX_DELAY);
                    enet2_tcpHandleReceivedData(data, len, 0, 1);
                    xSemaphoreGive(gEnet2TcpServerMutex);
                } while (netbuf_next(buf) >= 0 && gGUHUAing == 0);
                netbuf_delete(buf);
            }
            SET_SD_ELEMENT_VALUE(SD263, err);
            /*printf("Got EOF, looping\n");*/
            /* Close connection and discard connection identifier. */
            netconn_close(g_connEnet2Server);
            netconn_delete(g_connEnet2Server);
        }
//        vTaskDelay(1001 / portTICK_PERIOD_MS);
    }
}

static const char *TAGE2TCPS2 = "enet2_tcpServerThread2";
static void enet2_tcpServerThread2(void *arg)
{
    SET_SD_ELEMENT_VALUE(SD264, 2);
    err_t err;
    ip_addr_t addr;
    u16_t port;
    LWIP_UNUSED_ARG(arg);

    LOGV(TAGE2TCPS2, "Enter %s()", __func__);
    vTaskDelay(1001 / portTICK_PERIOD_MS);

    if (g_connEnet2 == NULL)
    {
        LOGE(TAGE2TCPS2, "g_connEnet2 == NULL, so just return.");
        SET_SD_ELEMENT_VALUE(SD264, 1);
        vTaskDelete(NULL);
        return;
    }

    while (1)
    {
        /* Grab new connection. */
        LOGW(TAGE2TCPS2, "Wait remote to connect ME!!!");
        err = netconn_accept(g_connEnet2, &g_connEnet2Server2);
        LOGV(TAGE2TCPS2, "netconn_accept return: %d", err);
        netconn_peer(g_connEnet2Server2, &addr, &port);
        LOGV(TAGE2TCPS2, "peer IPv4 Address  : %s,  port: %u\r\n", ipaddr_ntoa(&addr), port);
        netconn_addr(g_connEnet2Server2, &addr, &port);
        LOGW(TAGE2TCPS2, "IPv4 Address  : %s,  port: %u\r\n", ipaddr_ntoa(&addr), port);
        /* Process the new connection. */
        SET_SD_ELEMENT_VALUE(SD264, err);
        if (err == ERR_OK)
        {
            LOGV(TAGE2TCPS2, "accepted new connection : 0x%08X", g_connEnet2Server2);
            struct netbuf *buf;
            void *data;
            u16_t len;

            while ((err = netconn_recv(g_connEnet2Server2, &buf)) == ERR_OK)
            {
                do
                {
                    netbuf_data(buf, &data, &len);
                    xSemaphoreTake(gEnet2TcpServerMutex, portMAX_DELAY);
                    enet2_tcpHandleReceivedData(data, len, 0, 2);
                    xSemaphoreGive(gEnet2TcpServerMutex);
                } while (netbuf_next(buf) >= 0 && gGUHUAing == 0);
                netbuf_delete(buf);
            }
            SET_SD_ELEMENT_VALUE(SD264, err);
            /*printf("Got EOF, looping\n");*/
            /* Close connection and discard connection identifier. */
            netconn_close(g_connEnet2Server2);
            netconn_delete(g_connEnet2Server2);
        }
//        vTaskDelay(1001 / portTICK_PERIOD_MS);
    }
}

static const char *TAGE2TCPS3 = "enet2_tcpServerThread3";
static void enet2_tcpServerThread3(void *arg)
{
    SET_SD_ELEMENT_VALUE(SD265, 2);
    err_t err;
    ip_addr_t addr;
    u16_t port;
    LWIP_UNUSED_ARG(arg);

    LOGV(TAGE2TCPS3, "Enter %s()", __func__);
    vTaskDelay(2001 / portTICK_PERIOD_MS);

    if (g_connEnet2 == NULL)
    {
        LOGE(TAGE2TCPS3, "g_connEnet2 == NULL, so just return.");
        SET_SD_ELEMENT_VALUE(SD265, 1);
        vTaskDelete(NULL);
        return;
    }

    while (1)
    {
        /* Grab new connection. */
        LOGW(TAGE2TCPS3, "Wait remote to connect ME!!!");
        err = netconn_accept(g_connEnet2, &g_connEnet2Server3);
        LOGV(TAGE2TCPS3, "netconn_accept return: %d", err);
        SET_SD_ELEMENT_VALUE(SD265, err);
        /* Process the new connection. */
        if (err == ERR_OK)
        {
            netconn_peer(g_connEnet2Server3, &addr, &port);
            LOGV(TAGE2TCPS3, "peer IPv4 Address  : %s,  port: %u\r\n", ipaddr_ntoa(&addr), port);
            netconn_addr(g_connEnet2Server3, &addr, &port);
            LOGW(TAGE2TCPS3, "IPv4 Address  : %s,  port: %u\r\n", ipaddr_ntoa(&addr), port);
            
            LOGV(TAGE2TCPS3, "accepted new connection : 0x%08X", g_connEnet2Server3);
            struct netbuf *buf;
            void *data;
            u16_t len;

            while ((err = netconn_recv(g_connEnet2Server3, &buf)) == ERR_OK)
            {
                do
                {
                    netbuf_data(buf, &data, &len);
                    xSemaphoreTake(gEnet2TcpServerMutex, portMAX_DELAY);
                    enet2_tcpHandleReceivedData(data, len, 0, 3);
                    xSemaphoreGive(gEnet2TcpServerMutex);
                } while (netbuf_next(buf) >= 0 && gGUHUAing == 0);
                netbuf_delete(buf);
            }
            SET_SD_ELEMENT_VALUE(SD265, err);
            /* Close connection and discard connection identifier. */
            netconn_close(g_connEnet2Server3);
            netconn_delete(g_connEnet2Server3);
        }
//        vTaskDelay(1001 / portTICK_PERIOD_MS);
    }
}

void start_tcp_server(void)
{
    PRINTF("Enter %s()\r\n", __func__);
    BaseType_t ret = xTaskCreate((TaskFunction_t)tcpServerThread,
                                 (const char *)"tcp_server",
                                 TCPSERVER_TASK_STACK_SIZE,
                                 (void *)NULL,
                                 TCPSERVER_TASK_PRIO,
                                 &gKalykeTCPServerHandle);
    if (ret != pdPASS)
    {
        LOGE("kalyke_internet_task", "Create tcpServerThread ERROR!");
    }
}

void start_tcp_server2(void)
{
    PRINTF("Enter %s()\r\n", __func__);
    BaseType_t ret = xTaskCreate((TaskFunction_t)tcpServerThread2,
                                 (const char *)"tcp_server2",
                                 TCPSERVER_TASK_STACK_SIZE,
                                 (void *)NULL,
                                 TCPSERVER_TASK_PRIO,
                                 &gKalykeTCPServer2Handle);
    if (ret != pdPASS)
    {
        LOGE("kalyke_internet_task", "Create tcpServerThread2 ERROR!");
    }
}

void start_tcp_server3(void)
{
    PRINTF("Enter %s()\r\n", __func__);
    BaseType_t ret = xTaskCreate((TaskFunction_t)tcpServerThread3,
                                 (const char *)"tcp_server3",
                                 TCPSERVER_TASK_STACK_SIZE,
                                 (void *)NULL,
                                 TCPSERVER_TASK_PRIO,
                                 &gKalykeTCPServer3Handle);
    if (ret != pdPASS)
    {
        LOGE("kalyke_internet_task", "Create tcpServerThread3 ERROR!");
    }
}

void start_enet2_tcp_server(void)
{
    PRINTF("Enter %s()\r\n", __func__);
    BaseType_t ret = xTaskCreate((TaskFunction_t)enet2_tcpServerThread,
                                 (const char *)"enet2_tcp_server",
                                 TCPSERVER_TASK_STACK_SIZE,
                                 (void *)NULL,
                                 TCPSERVER_TASK_PRIO,
                                 &gKalykeEnet2TCPServerHandle);
    if (ret != pdPASS)
    {
        LOGE("kalyke_internet_task", "Create enet2_tcpServerThread ERROR!");
    }
}

void start_enet2_tcp_server2(void)
{
    PRINTF("Enter %s()\r\n", __func__);
    BaseType_t ret = xTaskCreate((TaskFunction_t)enet2_tcpServerThread2,
                                 (const char *)"enet2_tcp_server2",
                                 TCPSERVER_TASK_STACK_SIZE,
                                 (void *)NULL,
                                 TCPSERVER_TASK_PRIO,
                                 &gKalykeEnet2TCPServer2Handle);
    if (ret != pdPASS)
    {
        LOGE("kalyke_internet_task", "Create enet2_tcpServerThread2 ERROR!");
    }
}

void start_enet2_tcp_server3(void)
{
    PRINTF("Enter %s()\r\n", __func__);
    BaseType_t ret = xTaskCreate((TaskFunction_t)enet2_tcpServerThread3,
                                 (const char *)"enet2_tcp_server3",
                                 TCPSERVER_TASK_STACK_SIZE,
                                 (void *)NULL,
                                 TCPSERVER_TASK_PRIO,
                                 &gKalykeEnet2TCPServer3Handle);
    if (ret != pdPASS)
    {
        LOGE("kalyke_internet_task", "Create enet2_tcpServerThread3 ERROR!");
    }
}

//这里的clientID必须为3
static void kalyke_start_MicroLink_client(void)
{
    SET_SD_ELEMENT_VALUE(SD224, 0);
    SET_SD_ELEMENT_VALUE(SD231, 0xFF);
    LOGV("MicroLinkClient", "Enter %s(), ifConnect = %d", __func__, g_plc_netcfg.cloud.ifConnect);
    if (g_plc_netcfg.cloud.ifConnect == 0)
    {
        return;
    }
#if (KALYKE_FEATURE_4G_TCP_TASK == 1)
    if (g_plc_netcfg.surfing == 1)
    {
        LOGD("MicroLinkClient", "surfing is 1, so we use 4G, here just return.");
        xTaskCreate((TaskFunction_t)kalyke_4G_tcp_task,
                (const char *)"tcp_4G_task",
                (uint16_t)TASK_4G_TCP_STACK_SIZE,
                (void *)NULL,
                (UBaseType_t )TASK_4G_TCP_PRIO,
                NULL);
        return;
    }
#endif
    uint8_t clientID = 3;
    BaseType_t ret = xTaskCreate((TaskFunction_t)MicroLink_tcp_client_thread,
                                 (const char *)"MicroLink_client",
                                 1024,
                                 (void *)(uint32_t)clientID,
                                 INTERNET_TASK_PRIO,
                                 NULL);
    if (ret != pdPASS)
    {
        PRINTF("Create MicroLink_client ERROR!\r\n");
    }
}

/*!
 * @brief Called when subscription request finishes.
 */
static void mqtt_topic_subscribed_cb(void *arg, err_t err)
{
    const char *topic = (const char *)arg;

    if (err == ERR_OK)
    {
        LOGV("MQTT", "Subscribed to the topic \"%s\".\r\n", topic);
    }
    else
    {
        LOGE("MQTT", "Failed to subscribe to the topic \"%s\": %d.\r\n", topic, err);
    }
    SET_SD_ELEMENT_VALUE(SD229, err);
}

static void mqtt_topic_subscribed_cb2(void *arg, err_t err)
{
    const char *topic = (const char *)arg;

    if (err == ERR_OK)
    {
        PRINTF("2,Subscribed to the topic \"%s\".\r\n", topic);
    }
    else
    {
        PRINTF("2,Failed to subscribe to the topic \"%s\": %d.\r\n", topic, err);
    }
}

char gRecvTopic[128];
/*!
 * @brief Called when there is a message on a subscribed topic.
 */
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
    const struct mqtt_connect_client_info_t *client_info = (const struct mqtt_connect_client_info_t *)arg;
    LWIP_UNUSED_ARG(client_info);
    //LWIP_UNUSED_ARG(arg);
    if (memcmp(g_plc_netcfg.mqtt.vender, "HANYU", 5) == 0 || memcmp(g_plc_netcfg.mqtt.vender, "DEFAULTMQTT", 11) == 0)
    {
        memset(gRecvTopic, 0, sizeof(gRecvTopic));
        strncpy(gRecvTopic, topic, sizeof(gRecvTopic) - 1);
    }
    //PRINTF("Received %u bytes from the topic \"%s\": \"", tot_len, topic);
    LOGV("MQTT", "MQTT client \"%s\" receive topic: %s, tot_len %d\r\n", client_info->client_id, topic, (int)tot_len);
}
static void mqtt_incoming_publish_cb2(void *arg, const char *topic, u32_t tot_len)
{
    const struct mqtt_connect_client_info_t *client_info = (const struct mqtt_connect_client_info_t *)arg;
    LWIP_UNUSED_ARG(client_info);

    //PRINTF("2,Received %u bytes from the topic \"%s\": \"", tot_len, topic);
    PRINTF("2, MQTT client \"%s\" receive topic: %s, len %d\r\n", client_info->client_id, topic, (int)tot_len);
}

/*!
 * @brief Called when recieved incoming published message fragment.
 */
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
    const struct mqtt_connect_client_info_t *client_info = (const struct mqtt_connect_client_info_t *)arg;
    LWIP_UNUSED_ARG(client_info);

    LOGI("MQTT", "MQTT client \"%s\" data cb: len %d, flags %d\r\n", client_info->client_id, (int)len, (int)flags);

    mqtt_msg_st mqttMsg = {0};
    mqttMsg.type = 0;
    mqttMsg.msgLength = len;
    mqttMsg.dataBuff = pvPortMalloc(1024);
    memcpy(mqttMsg.dataBuff, data, len);
    mqttMsg.dataBuff[len] = 0;
    if (memcmp(g_plc_netcfg.mqtt.vender, "HANYU", 5) == 0 || memcmp(g_plc_netcfg.mqtt.vender, "DEFAULTMQTT", 5) == 0)
    {
        mqttMsg.topic = pvPortMalloc(1024);
        strcpy((char *)mqttMsg.topic, gRecvTopic);
    }
    xQueueSend(gMQTTMsgQueueHandle, &mqttMsg, 0);
    //LOGW("ethernet", "data is :  %s\r\n", mqttMsg.dataBuff);

#if 0
    for (int i = 0; i < len; i++)
    {
        if (isprint(data[i]))
        {
            PRINTF("%c", (char)data[i]);
        }
        else
        {
            PRINTF("\\x%02x", data[i]);
        }
    }

    if (flags & MQTT_DATA_FLAG_LAST)
    {
        PRINTF("\"\r\n");
    }
#endif
}
static void mqtt_incoming_data_cb2(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
    const struct mqtt_connect_client_info_t *client_info = (const struct mqtt_connect_client_info_t *)arg;
    LWIP_UNUSED_ARG(client_info);

    LOGI("ethernet", "MQTT client \"%s\" data cb: len %d, flags %d\r\n", client_info->client_id, (int)len, (int)flags);

    mqtt_msg_st mqttMsg;
    mqttMsg.msgLength = len;
    mqttMsg.dataBuff = pvPortMalloc(1024);
    memcpy(mqttMsg.dataBuff, data, len);
    mqttMsg.dataBuff[len] = 0;
    xQueueSend(gMQTTMsgQueueHandle2, &mqttMsg, 0);
    LOGW("ethernet", "data is :  %s\r\n", mqttMsg.dataBuff);
}

/*!
 * @brief Subscribe to MQTT topics.
 */
static void mqtt_subscribe_topics(mqtt_client_t *client)
{
    //static const char *topics[] = {"lwip_topic/#", "lwip_other/#"};
    //int qos[]                   = {0, 1};
    //err_t err;
    //int i;
    LOGV("MQTT", "Enter %s()", __func__);
    mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb,
                            LWIP_CONST_CAST(void *, &mqtt_client_info));
#if 0
    for (i = 0; i < ARRAY_SIZE(topics); i++)
    {
        err = mqtt_subscribe(client, topics[i], qos[i], mqtt_topic_subscribed_cb, LWIP_CONST_CAST(void *, topics[i]));

        if (err == ERR_OK)
        {
            PRINTF("Subscribing to the topic \"%s\" with QoS %d...\r\n", topics[i], qos[i]);
        }
        else
        {
            PRINTF("Failed to subscribe to the topic \"%s\" with QoS %d: %d.\r\n", topics[i], qos[i], err);
        }
    }
#endif

    if (strlen(MQTT_TOPIC_SUB) != 0)
    {
        mqtt_sub_unsub(client,
                   MQTT_TOPIC_SUB, 0,
                   mqtt_topic_subscribed_cb, LWIP_CONST_CAST(void *, MQTT_TOPIC_SUB),
                   1);
    }
    if (memcmp(g_plc_netcfg.mqtt.vender, "HANYU", 5) == 0 || memcmp(g_plc_netcfg.mqtt.vender, "DEFAULTMQTT", 11) == 0 )
    {
        if (strlen(g_plc_netcfg.mqtt.subscribe_topic_reboot) != 0)
        {
            mqtt_sub_unsub(client,
                       g_plc_netcfg.mqtt.subscribe_topic_reboot, 0,
                       mqtt_topic_subscribed_cb, LWIP_CONST_CAST(void *, g_plc_netcfg.mqtt.subscribe_topic_reboot),
                       1);
        }
        #if 1
        if (strlen(g_plc_netcfg.mqtt.subscribe_topic_pub_cycle) != 0)
        {
            mqtt_sub_unsub(client,
                       g_plc_netcfg.mqtt.subscribe_topic_pub_cycle, 0,
                       mqtt_topic_subscribed_cb, LWIP_CONST_CAST(void *, g_plc_netcfg.mqtt.subscribe_topic_pub_cycle),
                       1);
        }
        #endif
        if (strlen(g_plc_netcfg.mqtt.subscribe_topic_pub_now) != 0)
        {
            mqtt_sub_unsub(client,
                       g_plc_netcfg.mqtt.subscribe_topic_pub_now, 0,
                       mqtt_topic_subscribed_cb, LWIP_CONST_CAST(void *, g_plc_netcfg.mqtt.subscribe_topic_pub_now),
                       1);
        }
        LOGD("MQTT", "subscribe_topic_pause = %s", g_plc_netcfg.mqtt.subscribe_topic_pause);
        if (strlen(g_plc_netcfg.mqtt.subscribe_topic_pause) != 0)
        {
            mqtt_sub_unsub(client,
                       g_plc_netcfg.mqtt.subscribe_topic_pause, 0,
                       mqtt_topic_subscribed_cb, LWIP_CONST_CAST(void *, g_plc_netcfg.mqtt.subscribe_topic_pause),
                       1);
        }
    }
}
static void mqtt_subscribe_topics2(mqtt_client_t *client)
{
    //static const char *topics[] = {"lwip_topic/#", "lwip_other/#"};
    //int qos[]                   = {0, 1};
    //err_t err;
    //int i;

    mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb2, mqtt_incoming_data_cb2,
                            LWIP_CONST_CAST(void *, &mqtt_client_info2));
    mqtt_sub_unsub(client,
                   MQTT_TOPIC_SUB, 0,
                   mqtt_topic_subscribed_cb2, LWIP_CONST_CAST(void *, MQTT_TOPIC_SUB),
                   1);
}

/*!
 * @brief Called when connection state changes.
 */
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
    LOGV("MQTT", "Enter %s(), client = 0x%08X\r\n", __func__, client);
    const struct mqtt_connect_client_info_t *client_info = (const struct mqtt_connect_client_info_t *)arg;

    gMqttConnected = (status == MQTT_CONNECT_ACCEPTED);
    LOGD("MQTT", "MQTT client \"%s\" connection cb: status %d\r\n", client_info->client_id, (int)status);

    mqtt_msg_st mqttMsg = {0, NULL};
    switch (status)
    {
    case MQTT_CONNECT_ACCEPTED:
        bsp_open_led_mqtt();
        LOGV("MQTT", "MQTT client \"%s\" connected.\r\n", client_info->client_id);
        SET_SD_ELEMENT_VALUE(SD225, 1);
        mqtt_subscribe_topics(client);
        monitor_publish_now();/* 当连上MQTT服务器后立即publish一下 */
        break;

    case MQTT_CONNECT_REFUSED_SERVER:
    case MQTT_CONNECT_DISCONNECTED:
    case MQTT_CONNECT_TIMEOUT:
        SET_SD_ELEMENT_VALUE(SD225, 0);
        LOGE("internet_task", "MQTT client \"%s\" not connected.\r\n", client_info->client_id);
        //sys_timeout(10000, connect_to_mqtt, NULL);
        bsp_close_led_mqtt();
    #if (WAN_4G_SWITCH_AUTO == 1)
        if (link_is_up(&phyHandle1))
        {
            mqttMsg.type = 3;
            mqttMsg.dataBuff = NULL;
            xQueueSend(gMQTTMsgQueueHandle, &mqttMsg, 0);
        }
    #else
        mqttMsg.type = 3;
        mqttMsg.dataBuff = NULL;
        xQueueSend(gMQTTMsgQueueHandle, &mqttMsg, 0);
    #endif
        break;

    case MQTT_CONNECT_REFUSED_PROTOCOL_VERSION:
    case MQTT_CONNECT_REFUSED_IDENTIFIER:
    case MQTT_CONNECT_REFUSED_USERNAME_PASS:
    case MQTT_CONNECT_REFUSED_NOT_AUTHORIZED_:
        SET_SD_ELEMENT_VALUE(SD229, status);
        LOGE("internet_task", "MQTT client \"%s\" connection refused: %d.\r\n", client_info->client_id, (int)status);
        /* Try again 10 seconds later */
        //sys_timeout(10000, connect_to_mqtt, NULL);
        break;

    default:
        //PRINTF("MQTT client \"%s\" connection status: %d.\r\n", client_info->client_id, (int)status);
        /* Try again 10 seconds later */
        //sys_timeout(10000, connect_to_mqtt, NULL);
        break;
    }
}

#if 0
static void mqtt_connection_cb2(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
    LOGV("ethernet", "Enter %s(), client = 0x%08X\r\n", __func__, client);
    const struct mqtt_connect_client_info_t *client_info = (const struct mqtt_connect_client_info_t *)arg;

    connected2 = (status == MQTT_CONNECT_ACCEPTED);
    LOGD("ethernet", "MQTT client \"%s\" connection cb: status %d\r\n", client_info->client_id, (int)status);

    mqtt_msg_st mqttMsg = {0, NULL};
    switch (status)
    {
    case MQTT_CONNECT_ACCEPTED:
        bsp_open_led_2();
        PRINTF("2,MQTT client \"%s\" connected.\r\n", client_info->client_id);
        mqtt_subscribe_topics2(client);
        break;

    case MQTT_CONNECT_DISCONNECTED:
    case MQTT_CONNECT_TIMEOUT:
        /* Try again 1 second later */
        //sys_timeout(1000, connect_to_mqtt2, NULL);
        LOGW("internet_task", "MQTT client \"%s\" not connected.\r\n", client_info->client_id);
        //sys_timeout(10000, connect_to_mqtt, NULL);
        bsp_close_led_2();
        xQueueSend(gMQTTMsgQueueHandle2, &mqttMsg, 0);
        break;

    case MQTT_CONNECT_REFUSED_PROTOCOL_VERSION:
    case MQTT_CONNECT_REFUSED_IDENTIFIER:
    case MQTT_CONNECT_REFUSED_SERVER:
    case MQTT_CONNECT_REFUSED_USERNAME_PASS:
    case MQTT_CONNECT_REFUSED_NOT_AUTHORIZED_:
        LOGE("internet_task", "MQTT client \"%s\" connection refused: %d.\r\n", client_info->client_id, (int)status);
        break;

    default:
        //PRINTF("2,MQTT client \"%s\" connection status: %d.\r\n", client_info->client_id, (int)status);
        /* Try again 10 seconds later */
        //sys_timeout(10000, connect_to_mqtt2, NULL);
        break;
    }
}
#endif

/*!
 * @brief Starts connecting to MQTT broker. To be called on tcpip_thread.
 */
static void connect_to_mqtt(void *ctx)
{
    LWIP_UNUSED_ARG(ctx);

    LOGD("MQTT", "Connecting to MQTT broker at %s:%d...", ipaddr_ntoa(&mqtt_addr), KALYKE_MQTT_PORT);

    mqtt_client_connect(mqtt_client, &mqtt_addr, KALYKE_MQTT_PORT, mqtt_connection_cb,
                        LWIP_CONST_CAST(void *, &mqtt_client_info), &mqtt_client_info);
}
#if 0
static void connect_to_mqtt2(void *ctx)
{
    LWIP_UNUSED_ARG(ctx);

    PRINTF("2,Connecting to MQTT broker at %s...\r\n", ipaddr_ntoa(&mqtt_addr2));

    mqtt_client_connect(mqtt_client2, &mqtt_addr2, KALYKE_MQTT_PORT, mqtt_connection_cb2,
                        LWIP_CONST_CAST(void *, &mqtt_client_info2), &mqtt_client_info2);
}
#endif

/*!
 * @brief Called when publish request finishes.
 */
static void mqtt_message_published_cb(void *arg, err_t err)
{
    const struct mqtt_connect_client_info_t *client_info = (const struct mqtt_connect_client_info_t *)arg;
    if (arg != NULL)
    {
        LOGD("MQTT", "Eenter %s(), arg = 0x%08X, err = %d, client_id = %s", __func__, arg, err, client_info->client_id);
    }
    else
    {
        LOGE("MQTT", "Eenter %s(), arg = 0x%08X, err = %d", __func__, arg, err);
    }
    SET_SD_ELEMENT_VALUE(SD229, err);
#if 0
    const char *topic = (const char *)arg;

    if (err == ERR_OK)
    {
        PRINTF("Published to the topic \"%s\".\r\n", topic);
    }
    else
    {
        PRINTF("Failed to publish to the topic \"%s\": %d.\r\n", topic, err);
    }
#endif
}
static void mqtt_message_published_cb2(void *arg, err_t err)
{
    const struct mqtt_connect_client_info_t *client_info = (const struct mqtt_connect_client_info_t *)arg;
    LOGD("kalyke_internet_task", "Eenter %s(), arg = 0x%08X, err = %d, client_id = %s", __func__, arg, err, client_info->client_id);
#if 0
    const char *topic = (const char *)arg;

    if (err == ERR_OK)
    {
        PRINTF("2,Published to the topic \"%s\".\r\n", topic);
    }
    else
    {
        PRINTF("2,Failed to publish to the topic \"%s\": %d.\r\n", topic, err);
    }
#endif
}

/*!
 * @brief Publishes a message. To be called on tcpip_thread.
 */
void publish_message(void *ctx)
{
    static const char *topic   = "lwip_topic/100";
    static const char *message = "message from board";

    LWIP_UNUSED_ARG(ctx);

    PRINTF("Going to publish to the topic \"%s\"...\r\n", topic);

    mqtt_publish(mqtt_client, topic, message, strlen(message), 1, 0, mqtt_message_published_cb, (void *)2);
}
void publish_message2(void *ctx)
{
    static const char *topic   = "lwip_topic/100";
    static const char *message = "message from board";

    LWIP_UNUSED_ARG(ctx);

    PRINTF("2, Going to publish to the topic \"%s\"...\r\n", topic);

    mqtt_publish(mqtt_client2, topic, message, strlen(message), 1, 0, mqtt_message_published_cb2, (void *)3);
}

static void kalyke_mqtt_disconnect(void)
{
    LOGV("internet", "Enter %s()", __func__);
    if (mqtt_client)
    {
        mqtt_disconnect(mqtt_client);
        mqtt_client_free(mqtt_client);
        //mqtt_client = NULL;
    }
}

static bool isWanMqttOK(void)
{
    if (mqtt_client == NULL)
    {
        LOGI("internet", "Just return...2...client = NULL");
        return false;
    }
    if (mqtt_client_is_connected(mqtt_client) == 0)
    {
        LOGI("internet", "Just return...3...MQTT not connected.");
        return false;
    }
    return true;
}

static void kalyke_mqtt_publish_in_tcp_task_do(void *ctx)
{
    //kalyke_set_WAN_default();
    LOGD("MQTT", "Enter %s(), ctx = 0x%08X", __func__, ctx);
    mqtt_msg_st *pMqttMsg = (mqtt_msg_st *)ctx;
    char *topic = (char *)pMqttMsg->topic;
    char *payload = (char *)pMqttMsg->dataBuff;
    uint8_t qos = pMqttMsg->qos;
    mqtt_client_t *client;
    struct mqtt_connect_client_info_t *pClientInfo;
    void (*pPublishedCB)(void *arg, err_t err);

    client = mqtt_client;
    pClientInfo = &mqtt_client_info;
    pPublishedCB = mqtt_message_published_cb;

    if (client == NULL)
    {
        vPortFree(pMqttMsg->topic);
        vPortFree(pMqttMsg->dataBuff);
        vPortFree(pMqttMsg);
        LOGI("MQTT", "Just return...0...client = NULL");
        return;
    }
    if (mqtt_client_is_connected(client) == 0)
    {
        vPortFree(pMqttMsg->topic);
        vPortFree(pMqttMsg->dataBuff);
        vPortFree(pMqttMsg);
        LOGI("MQTT", "Just return...1...MQTT not connected.");
        return;
    }
    err_t err;
    uint8_t retain = 0; /* No don't retain such crappy payload... */
    //err = mqtt_publish(client, topic, payload, strlen(payload), qos, retain, pPublishedCB, (void *)1);
    err = mqtt_publish(client, topic, payload, pMqttMsg->msgLength, qos, retain, pPublishedCB, LWIP_CONST_CAST(void *, pClientInfo));
    LOGI("MQTT", "mqtt_publish return %d", err);
    vPortFree(pMqttMsg->topic);
    vPortFree(pMqttMsg->dataBuff);
    vPortFree(pMqttMsg);
}

static void kalyke_mqtt_publish_do(char *topic, char *payload, uint8_t qos)
{
    LOGV("MQTT", "Enter %s()", __func__);
    mqtt_client_t *client;
    struct mqtt_connect_client_info_t *pClientInfo;
    void (*pPublishedCB)(void *arg, err_t err);
#if (USE_FIRST_ENET == 1)
    client = mqtt_client;
    pClientInfo = &mqtt_client_info;
    pPublishedCB = mqtt_message_published_cb;
#endif
#if (USE_SECOND_ENET == 1)
    #if 0
    client = mqtt_client2;
    pClientInfo = &mqtt_client_info2;
    pPublishedCB = mqtt_message_published_cb2;
    #endif
#endif
    if (client == NULL)
    {
        LOGI("MQTT", "Just return...0...client = NULL");
        return;
    }
    if (mqtt_client_is_connected(client) == 0)
    {
        LOGI("MQTT", "Just return...1...MQTT not connected.");
        return;
    }
    err_t err;
    uint8_t retain = 0; /* No don't retain such crappy payload... */
    //err = mqtt_publish(client, topic, payload, strlen(payload), qos, retain, pPublishedCB, (void *)1);
    err = mqtt_publish(client, topic, payload, strlen(payload), qos, retain, pPublishedCB, LWIP_CONST_CAST(void *, pClientInfo));
    if(err != ERR_OK)
    {
        LOGE("MQTT", "Publish err: %d", err);
    }
    else
    {
        LOGV("MQTT", "Publish Success: %d", err);
    }
    SET_SD_ELEMENT_VALUE(SD229, err);
}

#if (WAN_4G_SWITCH_AUTO == 1)
void kalyke_mqtt_publish(char *topic, char *payload, uint8_t qos)
{
    static mqtt_source_type_t mMqttLastSource = MQTT_IDLE;
    LOGI("MQTT", "Enter %s(), gMqttSource = %u, mMqttLastSource = %u, gMqttConnected = %d, gWanOK = %d, gLanOK = %d", __func__, gMqttSource, mMqttLastSource, gMqttConnected, gWanOK, gLanOK);
    if (bsp_get_deviceID() == 0xFFFF)
    {
        LOGV("MQTT", "Device ID = 0xFFFF, so just return.");
        return;
    }
    if (bspIsHave4G())
    {
        if (gWanOK)
        {
            gMqttSource = MQTT_ENET;
            if (!isWanMqttOK())
            {
                return;
            }
        }
        else
        {
            gMqttSource = MQTT_4G;
        }
    }
    else
    {
        if (!isWanMqttOK())
        {
            return;
        }
    }
    mMqttLastSource = gMqttSource;
    if (gMqttSource == MQTT_4G)
    {
        ec20_mqtt_publish(topic, payload, qos);
        return;
    }
#if 0
    if (gMqttSource == MQTT_ALL)
    {
        ec20_mqtt_publish(topic, payload, qos);
    }
#endif
    
    if (gMqttConnected == false)
    {
        LOGI("MQTT", "Just return...4...MQTT not connected.");
        return;
    }
    if (gWanOK == false)
    {
        LOGI("MQTT", "Just return...5...WAN not OK.");
        return;
    }
    uint32_t topicLen, payloadLen;
#if (WAN_MQTT_PUBLISH_IN_TCP_TASK == 1)
    mqtt_msg_st *pMqttMsg = pvPortMalloc(sizeof(mqtt_msg_st));
    pMqttMsg->type = 1;
    pMqttMsg->qos = qos;
    topicLen = strlen(topic);
    payloadLen = strlen(payload);
    pMqttMsg->topic = pvPortMalloc(topicLen + 2);
    memcpy(pMqttMsg->topic, topic, topicLen);
    pMqttMsg->topic[topicLen] = 0;
    pMqttMsg->topic[topicLen + 1] = 0;
    pMqttMsg->dataBuff = pvPortMalloc(payloadLen + 2);
    memcpy(pMqttMsg->dataBuff, payload, payloadLen);
    pMqttMsg->dataBuff[payloadLen] = 0;
    pMqttMsg->dataBuff[payloadLen + 1] = 0;
    pMqttMsg->msgLength = payloadLen;

    err_t err = tcpip_callback(kalyke_mqtt_publish_in_tcp_task_do, pMqttMsg);
    if (err != ERR_OK)
    {
        SET_SD_ELEMENT_VALUE(SD229, err);
        LOGE("MQTT", "Failed to invoke broker connection on the tcpip_thread: %d.\r\n", err);
    }
#else
    mqtt_msg_st mqttMsg;
    mqttMsg.type = 1;
    mqttMsg.qos = qos;
    topicLen = strlen(topic);
    payloadLen = strlen(payload);
    mqttMsg.topic = pvPortMalloc(topicLen + 2);
    memcpy(mqttMsg.topic, topic, topicLen);
    mqttMsg.topic[topicLen] = 0;
    mqttMsg.topic[topicLen + 1] = 0;
    mqttMsg.dataBuff = pvPortMalloc(payloadLen + 2);
    memcpy(mqttMsg.dataBuff, payload, payloadLen);
    mqttMsg.dataBuff[payloadLen] = 0;
    mqttMsg.dataBuff[payloadLen + 1] = 0;
    xQueueSend(gMQTTMsgQueueHandle, &mqttMsg, 0);
#endif
    
    LOGV("MQTT", "Leave %s()", __func__);
}
#else
void kalyke_mqtt_publish(char *topic, char *payload, uint8_t qos)
{
    LOGD("MQTT", "Enter %s(), gMqttConnected = %d, gWanOK = %d, gLanOK = %d, surfing = %d", __func__, gMqttConnected, gWanOK, gLanOK, g_plc_netcfg.surfing);
    if (bsp_get_deviceID() == 0xFFFF)
    {
        LOGV("MQTT", "Device ID = 0xFFFF, so just return.");
        return;
    }

    if (bspIsHave4G() && g_plc_netcfg.surfing == 1)
    {
        ec20_mqtt_publish(topic, payload, qos);
        return;
    }
    
    if (gMqttConnected == false)
    {
        LOGD("MQTT", "Just return...4...MQTT not connected.");
        return;
    }
    if (gWanOK == false)
    {
        LOGD("MQTT", "Just return...5...WAN not OK.");
        return;
    }
    uint32_t topicLen, payloadLen;
#if (WAN_MQTT_PUBLISH_IN_TCP_TASK == 1)
    mqtt_msg_st *pMqttMsg = pvPortMalloc(sizeof(mqtt_msg_st));
    pMqttMsg->type = 1;
    pMqttMsg->qos = qos;
    topicLen = strlen(topic);
    payloadLen = strlen(payload);
    pMqttMsg->topic = pvPortMalloc(topicLen + 2);
    memcpy(pMqttMsg->topic, topic, topicLen);
    pMqttMsg->topic[topicLen] = 0;
    pMqttMsg->topic[topicLen + 1] = 0;
    pMqttMsg->dataBuff = pvPortMalloc(payloadLen + 2);
    memcpy(pMqttMsg->dataBuff, payload, payloadLen);
    pMqttMsg->dataBuff[payloadLen] = 0;
    pMqttMsg->dataBuff[payloadLen + 1] = 0;
    pMqttMsg->msgLength = payloadLen;

    err_t err = tcpip_callback(kalyke_mqtt_publish_in_tcp_task_do, pMqttMsg);
    if (err != ERR_OK)
    {
        SET_SD_ELEMENT_VALUE(SD229, err);
        LOGE("MQTT", "Failed to invoke broker connection on the tcpip_thread: %d.\r\n", err);
    }
#else
    mqtt_msg_st mqttMsg;
    mqttMsg.type = 1;
    mqttMsg.qos = qos;
    topicLen = strlen(topic);
    payloadLen = strlen(payload);
    mqttMsg.topic = pvPortMalloc(topicLen + 2);
    memcpy(mqttMsg.topic, topic, topicLen);
    mqttMsg.topic[topicLen] = 0;
    mqttMsg.topic[topicLen + 1] = 0;
    mqttMsg.dataBuff = pvPortMalloc(payloadLen + 2);
    memcpy(mqttMsg.dataBuff, payload, payloadLen);
    mqttMsg.dataBuff[payloadLen] = 0;
    mqttMsg.dataBuff[payloadLen + 1] = 0;
    xQueueSend(gMQTTMsgQueueHandle, &mqttMsg, 0);
#endif
    
    LOGV("MQTT", "Leave %s()", __func__);
}

#endif

#if 0
static void led_task(void *arg)
{
    while(1)
    {
        //bsp_toggle_led_RUN();
        //bsp_toggle_led_2();
        //bsp_toggle_led_ERR();
        //sys_msleep(20U);
        vTaskDelay(800);
    }
}
#endif

void Ali_MQTT_init(void)
{
    iotx_mqtt_region_types_t region = IOTX_CLOUD_REGION_SHANGHAI;
    iotx_dev_meta_info_t meta;
    iotx_sign_mqtt_t sign_mqtt;

    memset(&meta,0,sizeof(iotx_dev_meta_info_t));
    memcpy(meta.product_key, g_plc_netcfg.mqtt.ProductKey, strlen(g_plc_netcfg.mqtt.ProductKey));
    memcpy(meta.product_secret, g_plc_netcfg.mqtt.ProductSecret, strlen(g_plc_netcfg.mqtt.ProductSecret));
    memcpy(meta.device_name, g_plc_netcfg.mqtt.DeviceName, strlen(g_plc_netcfg.mqtt.DeviceName));
    memcpy(meta.device_secret,g_plc_netcfg.mqtt.DeviceSecret, strlen(g_plc_netcfg.mqtt.DeviceSecret));
    if (IOT_Sign_MQTT(region, &meta, &sign_mqtt) < 0)
    {
        LOGE("ALIMQTT", "IOT_Sign_MQTT Error!");
    }
    g_plc_netcfg.mqtt.port = sign_mqtt.port;
    memcpy(g_plc_netcfg.mqtt.host, sign_mqtt.hostname, strlen(sign_mqtt.hostname));
    memcpy(g_plc_netcfg.mqtt.client_id, sign_mqtt.clientid, strlen(sign_mqtt.clientid));
    memcpy(g_plc_netcfg.mqtt.username, sign_mqtt.username, strlen(sign_mqtt.username));
    memcpy(g_plc_netcfg.mqtt.password, sign_mqtt.password, strlen(sign_mqtt.password));
    LOGV("ALIMQTT", "hostname = %s", g_plc_netcfg.mqtt.host);
    LOGD("ALIMQTT", "port = %d", g_plc_netcfg.mqtt.port);
    LOGI("ALIMQTT", "clientid = %s", g_plc_netcfg.mqtt.client_id);
    LOGW("ALIMQTT", "username = %s", g_plc_netcfg.mqtt.username);
    LOGV("ALIMQTT", "password = %s", g_plc_netcfg.mqtt.password);
}

static void mqtt_init(void)
{
    LOGV("MQTT", "Enter %s()", __func__);
#if (WAN_4G_SWITCH_AUTO == 0)
    while (1)
    {
        if (!gWanOK)
        {
            LOGW("MQTT", "Just waitting, because there is no WAN now.");
            vTaskDelay(10000);
        }
        else
        {
            break;
        }
    }
#else
    /* 如果未插网线，则直接退出 */
    if (!link_is_up(&phyHandle1))
    {
        LOGE("MQTT", "Just return, because no WAN");
        return;
    }
#endif
    err_t err;
    if (mqtt_client)
    {
        mqtt_client_free(mqtt_client);
    }
    mqtt_client = mqtt_client_new();
    LOGD("MQTT", "mqtt_client = 0x%08X\r\n", mqtt_client);
    if (memcmp(g_plc_netcfg.mqtt.vender, "AliMQTT", 7) == 0)
    {
        Ali_MQTT_init();
    }
AGAIN:
    if (strlen(KALYKE_MQTT_SERVER_HOST_NAME) == 0)
    {
        LOGW("MQTT", "Mqtt host name is null, so delete mqtt thread.");
        vTaskDelete(NULL);
    }
    /*
     * Check if we have an IP address or host name string configured.
     * Could just call netconn_gethostbyname() on both IP address or host name,
     * but we want to print some info if goint to resolve it.
     */
    if (ipaddr_aton(KALYKE_MQTT_SERVER_HOST_NAME, &mqtt_addr) && IP_IS_V4(&mqtt_addr))
    {
        /* Already an IP address */
        err = ERR_OK;
    }
    else
    {
        /* Resolve MQTT broker's host name to an IP address */
        LOGV("MQTT", "Resolving : %s...", KALYKE_MQTT_SERVER_HOST_NAME);
        err = netconn_gethostbyname(KALYKE_MQTT_SERVER_HOST_NAME, &mqtt_addr);
    }
    LOGI("MQTT", "After netconn_gethostbyname()....");
    mqtt_client_info.client_id = g_plc_netcfg.mqtt.client_id;
    mqtt_client_info.client_user = g_plc_netcfg.mqtt.username;
    if (strlen(g_plc_netcfg.mqtt.password) != 0)
    {
        mqtt_client_info.client_pass = g_plc_netcfg.mqtt.password;
    }
    else
    {
        mqtt_client_info.client_pass = NULL;
    }
    
    mqtt_client_info.keep_alive = g_plc_netcfg.mqtt.keepalive;
    if (strlen(g_plc_netcfg.mqtt.lwt_topic) != 0)
    {
        mqtt_client_info.will_topic = g_plc_netcfg.mqtt.lwt_topic;
        mqtt_client_info.will_msg = g_plc_netcfg.mqtt.lwt_msg;
        mqtt_client_info.will_qos = g_plc_netcfg.mqtt.lwt_qos;
        mqtt_client_info.will_retain = g_plc_netcfg.mqtt.lwt_retain;
    }
    else
    {
        LOGV("MQTT", "will_topic does not exist!");
        mqtt_client_info.will_topic = NULL;
        mqtt_client_info.will_msg = NULL;
        mqtt_client_info.will_qos = 0;
        mqtt_client_info.will_retain = 0;
    }
    if (err == ERR_OK)
    {
        /* Start connecting to MQTT broker from tcpip_thread */
        err = tcpip_callback(connect_to_mqtt, NULL);
        if (err != ERR_OK)
        {
            SET_SD_ELEMENT_VALUE(SD229, err);
            LOGE("MQTT", "Failed to invoke broker connection on the tcpip_thread: %d.\r\n", err);
        }
    }
    else
    {
        SET_SD_ELEMENT_VALUE(SD229, err);
        LOGE("MQTT", "Failed to obtain IP address: %d.\r\n", err);
        err = netifapi_netif_set_up(&fsl_netif0);
        LOGW("MQTT", "netifapi_netif_set_up return: %d", err);
        err = netifapi_netif_set_default(&fsl_netif0);
        LOGW("MQTT", "netifapi_netif_set_default return: %d", err);
        vTaskDelay(3000);
        goto AGAIN;
    }
}

#if 0
static void mqtt_init2(void)
{
    err_t err;
    mqtt_client2 = mqtt_client_new();
    PRINTF("mqtt_client2 = 0x%08X\r\n", mqtt_client2);
    /*
     * Check if we have an IP address or host name string configured.
     * Could just call netconn_gethostbyname() on both IP address or host name,
     * but we want to print some info if goint to resolve it.
     */
    if (ipaddr_aton(KALYKE_MQTT_SERVER_HOST_NAME, &mqtt_addr2) && IP_IS_V4(&mqtt_addr2))
    {
        /* Already an IP address */
        err = ERR_OK;
    }
    else
    {
        /* Resolve MQTT broker's host name to an IP address */
        PRINTF("Resolving \"%s\"...\r\n", KALYKE_MQTT_SERVER_HOST_NAME);
        err = netconn_gethostbyname(KALYKE_MQTT_SERVER_HOST_NAME, &mqtt_addr2);
    }

    mqtt_client_info2.client_id = g_plc_netcfg.mqtt.client_id;
    mqtt_client_info2.client_user = g_plc_netcfg.mqtt.username;
    mqtt_client_info2.client_pass = g_plc_netcfg.mqtt.password;
    mqtt_client_info2.keep_alive = g_plc_netcfg.mqtt.keepalive;
    if (strlen(g_plc_netcfg.mqtt.lwt_topic) != 0)
    {
        mqtt_client_info2.will_topic = g_plc_netcfg.mqtt.lwt_topic;
        mqtt_client_info2.will_msg = g_plc_netcfg.mqtt.lwt_msg;
        mqtt_client_info2.will_qos = g_plc_netcfg.mqtt.lwt_qos;
        mqtt_client_info2.will_retain = g_plc_netcfg.mqtt.lwt_retain;
    }
    else
    {
        mqtt_client_info2.will_topic = NULL;
        mqtt_client_info2.will_msg = NULL;
        mqtt_client_info2.will_qos = 0;
        mqtt_client_info2.will_retain = 0;
    }
    if (err == ERR_OK)
    {
        /* Start connecting to MQTT broker from tcpip_thread */
        err = tcpip_callback(connect_to_mqtt2, NULL);
        if (err != ERR_OK)
        {
            LOGE("internet_task", "Failed to invoke broker connection on the tcpip_thread: %d.\r\n", err);
        }
    }
    else
    {
        LOGE("internet_task", "Failed to obtain IP address: %d.\r\n", err);
    }
}
#endif

extern TickType_t gUart1SendFinishDelay;
static void handle_mqtt_recv_data_non_json(mqtt_msg_st *pMqttMsg)
{
    LOGV("MQTT", "Enter %s()", __func__);
    char *atCommand;
    char *msg = (char *)pMqttMsg->dataBuff;
    uint32_t len = pMqttMsg->msgLength;
    //(void)len;
    if (strcmp(g_plc_netcfg.mqtt.vender, "HANYU") == 0 || strcmp(g_plc_netcfg.mqtt.vender, "DEFAULTMQTT") == 0  )
    {
        char *mqttTopic = (char *)pMqttMsg->topic;
        LOGD("MQTT", "mqttTopic = %s", mqttTopic);
        if (strstr(mqttTopic, "Reboot"))
        {
        #if 0
            if (strcmp(msg, "True") == 0)
            {
                stop_tcp_client_all();
                NVIC_SystemReset();
            }
        #endif
        }
        else if (strstr(mqttTopic, "Pause"))
        {
            if (strcmp(msg, "Enable") == 0)
            {
                g_plc_netcfg.mqtt.paused = true;
            }
            else if (strcmp(msg, "Disable") == 0)
            {
                g_plc_netcfg.mqtt.paused = false;
            }
        }
        else if (strstr(mqttTopic, "MDataPubCycle"))
        {
            g_plc_netcfg.mqtt.reportingCycle = atol(msg);
            LOGI("MQTT", "g_plc_netcfg.mqtt.reportingCycle = %u", g_plc_netcfg.mqtt.reportingCycle);
        }
        else if (strstr(mqttTopic, "MDataPubNow"))
        {
            monitor_publish_now();
        }
    }
    
    if (strncmp(msg, "vTaskList", 9) == 0)
    {
#if ( ( configUSE_TRACE_FACILITY == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS > 0 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )
        char *buffers = pvPortMalloc(2048);
        buffers[0] = '\r';
        buffers[1] = '\n';
        vTaskList(buffers + 2);
        //PRINTF("%s", buffers);
        kalyke_mqtt_publish(MQTT_TOPIC_POST, buffers, 0);
        vPortFree(buffers);
#else
        char *buffers = pvPortMalloc(500);
        strcpy(buffers, "vTaskList not work\r\n");
        kalyke_mqtt_publish(MQTT_TOPIC_POST, buffers, 0);
        vPortFree(buffers);
#endif
    }
    else if (strncmp(msg, "version", 7) == 0)
    {
        char *buf = pvPortMalloc(64);
        sprintf(buf, "Kalyke current version: %s", SW_VERSION);
        LOGV("internet_task", "%s", buf);
        kalyke_mqtt_publish(MQTT_TOPIC_POST, buf, 0);
        vPortFree(buf);
    }
    else if (strncmp(msg, "adspeed", 7) == 0)
    {
        gt_InAdDaCfg.adSpeed = atoi(msg + 8);
        LOGV("ethernet_task", "adSpeed = %u(ms)", gt_InAdDaCfg.adSpeed);
    }
    else if (strncmp(msg, "uart1delay", 10) == 0)
    {
        gUart1SendFinishDelay = atoi(msg + 11);
        LOGV("ethernet_task", "gUart1SendFinishDelay = %u(ms)", gUart1SendFinishDelay);
    }
    else if (strncmp(msg, "stoptcpclient", 13) == 0)
    {
        uint8_t clientID = atoi(msg + 14);
        LOGV("ethernet_task", "clientID = %u(ms)", clientID);
        //stop_tcp_client(clientID - 1);
    }
    else if (strncmp(msg, "AT", 2) == 0 || strncmp(msg, "at", 2) == 0)
    {
        atCommand = pvPortMalloc(512);
        //hexdump(msg, len);
        memcpy(atCommand, msg, len);
        atCommand[len] = 0;
        //uart4_send_buffer(atCommand, len);
        sendAT(atCommand);
        vPortFree(atCommand);
    }
    else if (strncmp(msg, "inituart8", 9) == 0)
    {
        init_UART8_4G();
    }
    else if (strncmp(msg, "resetme", 7) == 0)
    {
    #if 0
        stop_tcp_client_all();
        NVIC_SystemReset();
    #endif
    }
}

static void log_mqtt_recv_st(mqtt_recv_st *pmr)
{
#if SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_SDK

    LOGW("mqtt", "mr.commandType : %s", pmr->commandType);
    LOGW("mqtt", "mr.uuid : %s", pmr->uuid);
    LOGW("mqtt", "mr.deviceCode : %s", pmr->deviceCode);

    LOGW("mqtt", "mr.cmdLength : %d", pmr->cmdLength);
    for (int i = 0; i < pmr->cmdLength; i++)
    {
        LOGV("mqtt", "pCmd[%d].name : %s", i, pmr->pCmd[i].name);
        hexdump(pmr->pCmd[i].name, sizeof(pmr->pCmd[i].name));
        LOGV("mqtt", "pCmd[%d].dataType : %s", i, pmr->pCmd[i].dataType);
        //hexdump(pmr->pCmd[i].dataType, sizeof(pmr->pCmd[i].dataType));
        LOGI("mqtt", "pCmd[%d].dataType : %u", i, pmr->pCmd[i].dataType);
        LOGV("mqtt", "pCmd[%d].address : %u\r\n", i, pmr->pCmd[i].address);
        LOGW("mqtt", "pCmd[%d].valueFloat : %f", i, pmr->pCmd[i].valueFloat);
        LOGI("mqtt", "pCmd[%d].valueInt32 : %d", i, pmr->pCmd[i].valueInt32);
        LOGV("mqtt", "pCmd[%d].valueInt16 : %d", i, pmr->pCmd[i].valueInt16);
        LOGD("mqtt", "pCmd[%d].valueBool : %d", i, pmr->pCmd[i].valueBool);
    }
#endif
}

static void setElementValue_int16(mqtt_command_st *pCmd)
{
    LOGV("MQTT", "Enter %s()\r\n", __func__);
    if (pCmd->element == ELEM_D)  //(memcmp(pCmd->element, "D", 1) == 0)
    {
        SET_D_ELEMENT_VALUE(pCmd->address, pCmd->valueInt16);
    }
    else if (pCmd->element == ELEM_R)  //(memcmp(pCmd->element, "R", 1) == 0)
    {
        SET_R_ELEMENT_VALUE(pCmd->address, pCmd->valueInt16);
    }
}

static void setElementValue_int32(mqtt_command_st *pCmd)
{
    LOGV("ethernet_task", "Enter %s()\r\n", __func__);
#if 0
    uint16_t low16 = pCmd->valueInt32 & 0x00FF;
    uint16_t high16 = (pCmd->valueInt32 >> 16) & 0x00FF;
    SET_D_ELEMENT_VALUE(pCmd->address, low16);
    SET_D_ELEMENT_VALUE(pCmd->address + 1, high16);
#else
    int32_t *pval;
    if (pCmd->element == ELEM_D)  //(memcmp(pCmd->element, "D", 1) == 0)
    {
        int32_t *pval = (int32_t *)&gtv_PlcElement.msp_DElement[pCmd->address];
        *pval = pCmd->valueInt32;
    }
    else if (pCmd->element == ELEM_R)  //(memcmp(pCmd->element, "R", 1) == 0)
    {
        int32_t *pval = (int32_t *)&gtv_PlcElement.msp_DElement[pCmd->address];
        *pval = pCmd->valueInt32;
    }
#endif
    LOGV("ethernet_task", "*pval = %d", *pval);
}

static void setElementValue_float(mqtt_command_st *pCmd)
{
    PRINTF("Enter %s()\r\n", __func__);
#if 0
    uint16_t low16 = (uint32_t)pCmd->valueFloat & 0x00FF;
    uint16_t high16 = ((uint32_t)pCmd->valueFloat >> 16) & 0x00FF;
    SET_D_ELEMENT_VALUE(pCmd->address, low16);
    SET_D_ELEMENT_VALUE(pCmd->address + 1, high16);
#else
    float *pval;
    if (pCmd->element == ELEM_D)  //(memcmp(pCmd->element, "D", 1) == 0)
    {
        pval = (float *)&gtv_PlcElement.msp_DElement[pCmd->address];
        *pval = pCmd->valueFloat;
    }
    else if (pCmd->element == ELEM_R)  //(memcmp(pCmd->element, "R", 1) == 0)
    {
        pval = (float *)&gtv_PlcElement.msp_RElement[pCmd->address];
        *pval = pCmd->valueFloat;

    }
#endif
    PRINTF("heheFloat = %f\r\n", *pval);
}

static void setElementValue_bool(mqtt_command_st *pCmd)
{    
    LOGV("MQTT", "Enter %s()\r\n", __func__);
    if (pCmd->element == ELEM_M)  //(memcmp(pCmd->element, "M", 1) == 0)
    {
        plc_set_bit_element_value(M_ELEMENT,pCmd->address,pCmd->valueBool);
    }
    else if (pCmd->element == ELEM_X)  //(memcmp(pCmd->element, "X", 1) == 0)
    {
        plc_set_bit_element_value(X_ELEMENT,pCmd->address,pCmd->valueBool);
    }
    else if (pCmd->element == ELEM_Y)  //(memcmp(pCmd->element, "Y", 1) == 0)
    {
        plc_set_bit_element_value(Y_ELEMENT,pCmd->address,pCmd->valueBool);
    }
}

static void process_mqtt_recv_data(mqtt_recv_st *pmr)
{
    for (int i = 0; i < pmr->cmdLength; i++)
    {
        if (pmr->pCmd[i].dataType == DTYPE_I16)  //(strcmp(pmr->pCmd[i].dataType, "int16") == 0)
        {
            setElementValue_int16(&pmr->pCmd[i]);
        }
        else if (pmr->pCmd[i].dataType == DTYPE_F32)  //(strcmp(pmr->pCmd[i].dataType, "float32") == 0)
        {
            setElementValue_float(&pmr->pCmd[i]);
        }
        else if (pmr->pCmd[i].dataType == DTYPE_I32)  //(strcmp(pmr->pCmd[i].dataType, "int32") == 0)
        {
            setElementValue_int32(&pmr->pCmd[i]);
        }
        else if (pmr->pCmd[i].dataType == DTYPE_BOOL)  //(strcmp(pmr->pCmd[i].dataType, "bool") == 0)
        {
            setElementValue_bool(&pmr->pCmd[i]);
        }
    }
}

#if 1 //KALYKE_NEW_MQTT
static void process_mqtt_recv_data_DEFAULTMQTT(mqtt_recv_st *pmr)
{
    LOGV("MQTT", "Enter %s()", __func__);

    if(strcmp(pmr->commandType, "keyword") == 0)
    {
        for (int j = 0; j < g_plc_netcfg.mqtt.configLength; j++) //分组循环
        {
            if(pmr->slave_id != g_plc_netcfg.mqtt.pConfigsHANYU[j].slave_id)  //slave_id对不上
            {
                LOGE("MQTT", "recv slave_id ERROR");
                continue;
            }

            if(strcmp(pmr->slave_name, g_plc_netcfg.mqtt.pConfigsHANYU[j].slave_name) != 0)  //slave_name对不上
            {
                LOGE("MQTT", "recv slave_name ERROR");
                continue;
            }

            for (int i = 0; i < pmr->cmdLength; i++)
            {
                for (int k = 0; k < g_plc_netcfg.mqtt.pConfigsHANYU[j].reportContentLen; k++) // 组内循环
                {
                    if (strcmp(pmr->pCmd[i].name, g_plc_netcfg.mqtt.pConfigsHANYU[j].pReportContent[k].name) != 0)  //name对不上
                    {
                        LOGE("MQTT", "recv data_name ERROR");
                        continue;
                    }

                    //char dataType[16] = {0};
                    //strcpy(dataType, g_plc_netcfg.mqtt.pConfigsHANYU[j].pReportContent[k].dataType);
                    uint8_t dataType = g_plc_netcfg.mqtt.pConfigsHANYU[j].pReportContent[k].dataType;

                    uint16_t adr = g_plc_netcfg.mqtt.pConfigsHANYU[j].pReportContent[k].address;
                    pmr->pCmd[i].address = adr;

                    pmr->pCmd[i].element = g_plc_netcfg.mqtt.pConfigsHANYU[j].pReportContent[k].element;  //strcpy(pmr->pCmd[i].element, g_plc_netcfg.mqtt.pConfigsHANYU[j].pReportContent[k].element);
                    if (dataType == DTYPE_I16)  //(strcmp(dataType, "int16") == 0)
                    {
                        setElementValue_int16(&pmr->pCmd[i]);
                    }
                    else if (dataType == DTYPE_F32)  //(strcmp(dataType, "float32") == 0)
                    {
                        setElementValue_float(&pmr->pCmd[i]);
                    }
                    else if (dataType == DTYPE_I32)  //(strcmp(dataType, "int32") == 0)
                    {
                        setElementValue_int32(&pmr->pCmd[i]);
                    }
                    else if (dataType == DTYPE_BOOL)  //(strcmp(dataType, "bool") == 0)
                    {
                        setElementValue_bool(&pmr->pCmd[i]);
                    }
                    break;
                }
            }
        }
    }
    else if (strcmp(pmr->commandType, "element") == 0)
    {
        for (int i = 0; i < pmr->cmdLength; i++)
        {
            if (pmr->pCmd[i].dataType == DTYPE_I16)  //(strcmp(pmr->pCmd[i].dataType, "int16") == 0)
            {
                setElementValue_int16(&pmr->pCmd[i]);
            }
            else if (pmr->pCmd[i].dataType == DTYPE_F32)  //(strcmp(pmr->pCmd[i].dataType, "float32") == 0)
            {
                setElementValue_float(&pmr->pCmd[i]);
            }
            else if (pmr->pCmd[i].dataType == DTYPE_I32)  //(strcmp(pmr->pCmd[i].dataType, "int32") == 0)
            {
                setElementValue_int32(&pmr->pCmd[i]);
            }
            else if (pmr->pCmd[i].dataType == DTYPE_BOOL)  //(strcmp(pmr->pCmd[i].dataType, "bool") == 0)
            {
                setElementValue_bool(&pmr->pCmd[i]);
            }
        }
    }
}

static void process_mqtt_recv_data_JIONTECH(mqtt_recv_st *pmr)
{
    for (int i = 0; i < pmr->cmdLength; i++)
    {
        if (pmr->pCmd[i].dataType == DTYPE_I16)  //(strcmp(pmr->pCmd[i].dataType, "int16") == 0)
        {
            setElementValue_int16(&pmr->pCmd[i]);
        }
        else if (pmr->pCmd[i].dataType == DTYPE_F32)  //(strcmp(pmr->pCmd[i].dataType, "float32") == 0)
        {
            setElementValue_float(&pmr->pCmd[i]);
        }
        else if (pmr->pCmd[i].dataType == DTYPE_I32)  //(strcmp(pmr->pCmd[i].dataType, "int32") == 0)
        {
            setElementValue_int32(&pmr->pCmd[i]);
        }
        else if (pmr->pCmd[i].dataType == DTYPE_BOOL)  //(strcmp(pmr->pCmd[i].dataType, "bool") == 0)
        {
            setElementValue_bool(&pmr->pCmd[i]);
        }
    }
}
#endif

#if 0
static void process_mqtt_recv_data_HANYU(mqtt_recv_st *pmr)
{
    LOGV("MQTT", "Enter %s()", __func__);
    for (int i = 0; i < pmr->cmdLength; i++)
    {
        for (int j = 0; j < g_plc_netcfg.mqtt.configLength; j++)
        {
            if (strcmp(pmr->pCmd[i].name, g_plc_netcfg.mqtt.pConfigs[j].name) == 0)
            {
                if (strcmp(g_plc_netcfg.mqtt.pConfigs[j].dataType, "int16") == 0)
                {
                    pmr->pCmd[i].address = g_plc_netcfg.mqtt.pConfigs[j].address;
                    setElementValue_int16(&pmr->pCmd[i]);
                }
                else if (strcmp(g_plc_netcfg.mqtt.pConfigs[j].dataType, "float32") == 0)
                {
                    pmr->pCmd[i].address = g_plc_netcfg.mqtt.pConfigs[j].address;
                    setElementValue_float(&pmr->pCmd[i]);
                }
                else if (strcmp(g_plc_netcfg.mqtt.pConfigs[j].dataType, "int32") == 0)
                {
                    pmr->pCmd[i].address = g_plc_netcfg.mqtt.pConfigs[j].address;
                    setElementValue_int32(&pmr->pCmd[i]);
                }
                else if (strcmp(g_plc_netcfg.mqtt.pConfigs[j].dataType, "bool") == 0)
                {
                    pmr->pCmd[i].address = g_plc_netcfg.mqtt.pConfigs[j].address;
                    setElementValue_bool(&pmr->pCmd[i]);
                }
                break;
            }
        }        
    }
}
#else
static void process_mqtt_recv_data_HANYU(mqtt_recv_st *pmr)
{
    LOGV("MQTT", "Enter %s()", __func__);
    bool flag = false;
    for (int i = 0; i < pmr->cmdLength; i++)
    {
        for (int j = 0; j < g_plc_netcfg.mqtt.configLength; j++) //分组循环
        {
            for (int k = 0; k < g_plc_netcfg.mqtt.pConfigsHANYU[j].reportContentLen; k++) // 组内循环
            {
                if (strcmp(pmr->pCmd[i].name, g_plc_netcfg.mqtt.pConfigsHANYU[j].pReportContent[k].name) == 0)
                {
                    //char dataType[16] = {0};
                    //strcpy(dataType, g_plc_netcfg.mqtt.pConfigsHANYU[j].pReportContent[k].dataType);
                    uint8_t dataType = g_plc_netcfg.mqtt.pConfigsHANYU[j].pReportContent[k].dataType;
                    uint16_t adr = g_plc_netcfg.mqtt.pConfigsHANYU[j].pReportContent[k].address;
                    pmr->pCmd[i].address = adr;

                    pmr->pCmd[i].element = g_plc_netcfg.mqtt.pConfigsHANYU[j].pReportContent[k].element;  //strcpy(pmr->pCmd[i].element, g_plc_netcfg.mqtt.pConfigsHANYU[j].pReportContent[k].element);
                    if (dataType == DTYPE_I16)  //(strcmp(dataType, "int16") == 0)
                    {
                        setElementValue_int16(&pmr->pCmd[i]);
                    }
                    else if (dataType == DTYPE_F32)  //(strcmp(dataType, "float32") == 0)
                    {
                        setElementValue_float(&pmr->pCmd[i]);
                    }
                    else if (dataType == DTYPE_I32)  //(strcmp(dataType, "int32") == 0)
                    {
                        setElementValue_int32(&pmr->pCmd[i]);
                    }
                    else if (dataType == DTYPE_BOOL)  //(strcmp(dataType, "bool") == 0)
                    {
                        setElementValue_bool(&pmr->pCmd[i]);
                    }
                    flag = true;
                    break;
                }
            }
            if (flag == true)
            {
                flag = false;
                break;
            }
        }        
    }
}
#endif

static void mqtt_recv_rsp(mqtt_recv_st *pmr, int status)
{
    char *pRspBuff = pvPortMalloc(1024);
    sprintf(pRspBuff, "{\"uuid\":\"%s\",\"deviceCode\":\"%s\",\"status\":%d,\"cmdBackTime\":%u}", pmr->uuid, pmr->deviceCode, status, gKalykeSecondTick);
    kalyke_mqtt_publish(MQTT_TOPIC_RSP, pRspBuff, 0);
    vPortFree(pRspBuff);
}

static void memory_free_mqtt_recv(mqtt_recv_st *pmr)
{
    if (pmr->cmdLength != 0 && pmr->pCmd != NULL)
    {
        vPortFree(pmr->pCmd);
    }
}

#if (KALYKE_CJSON == 0)
void handle_mqtt_recv_data(mqtt_msg_st *pMqttMsg)
{
    char *pJson = (char *)pMqttMsg->dataBuff;
    uint32_t len = pMqttMsg->msgLength;
    (void)len;
    int tokenCount = 0;
    LOGV("MQTT", "Enter %s(), dataBuff = %s\r\n", __func__, pJson);
    if (Kalyke_isJsonValidAndParse(pJson, &tokenCount) == false)
    {
        // This is not json data
        LOGV("MQTT", "This is not json data.");
        handle_mqtt_recv_data_non_json(pMqttMsg);
        return;
    }
    if (memcmp(g_plc_netcfg.mqtt.vender, "TLINK", 5) == 0)
    {
        LOGW("MQTT", "Do not handle temporarily when vender is TLINK");
    }
    else
    {
        mqtt_recv_st mr = {0};
        LOGV("MQTT", "sizeof(mqtt_recv_st) = %d\r\n", sizeof(mqtt_recv_st));
        int8_t ret = Kalyke_extractMqttRecv(pJson, tokenCount, &mr);
        LOGI("MQTT", "Kalyke_extractMqttRecv return : %d", ret);
        log_mqtt_recv_st(&mr);
        if (ret == 0)
        {
            //对数据进行处理
            process_mqtt_recv_data(&mr);
        }
        mqtt_recv_rsp(&mr, ret);
        memory_free_mqtt_recv(&mr);
    }
}
#else
void handle_mqtt_recv_data(mqtt_msg_st *pMqttMsg)
{
    char *pJson = (char *)pMqttMsg->dataBuff;
    LOGV("MQTT", "Enter %s(), pJson = %s", __func__, pJson);

    int tokenCount = 0;
    mqtt_recv_st mr = {0};
    int8_t ret = Kalyke_extractMqttRecv(pJson, tokenCount, &mr);
    LOGW("MQTT", "Kalyke_extractMqttRecv return %d", ret);
    if (ret == ERR_MQTT_JSON_NOT_OBJECT)
    {
        // This is not json data
        LOGV("MQTT", "This is not json data.");
        handle_mqtt_recv_data_non_json(pMqttMsg);
    }
    else if (ret < 0)
    {
        LOGE("MQTT", "Parse json error.");
    }
    else
    {
        log_mqtt_recv_st(&mr);
        if (strcmp(g_plc_netcfg.mqtt.vender, "TLINK") == 0)
        {
            LOGW("MQTT", "Do not handle temporarily when vender is TLINK");
        }
#if 1 //KALYKE_NEW_MQTT
        else if (strcmp(g_plc_netcfg.mqtt.vender, "DEFAULTMQTT") == 0 )
        {
            process_mqtt_recv_data_DEFAULTMQTT(&mr);
            //monitor_publish_now();
        }
        else if (strcmp(g_plc_netcfg.mqtt.vender, "HANYU") == 0)
        {
            process_mqtt_recv_data_HANYU(&mr);
            monitor_publish_now();
        }
#else
        else if (strcmp(g_plc_netcfg.mqtt.vender, "HANYU") == 0 || strcmp(g_plc_netcfg.mqtt.vender, "DEFAULTMQTT") == 0 )
        {
            process_mqtt_recv_data_HANYU(&mr);
        }
#endif
        else if (strcmp(g_plc_netcfg.mqtt.vender, "JIONTECH") == 0)
        {
            process_mqtt_recv_data_JIONTECH(&mr);
            monitor_publish_now_JIONTECH_reply(mr.uuid);
        }
        else
        {
            LOGV("MQTT", "sizeof(mqtt_recv_st) = %d\r\n", sizeof(mqtt_recv_st));
            if (ret == 0)
            {
                //对数据进行处理
                process_mqtt_recv_data(&mr);
            }
            mqtt_recv_rsp(&mr, ret);
        }
    }
    memory_free_mqtt_recv(&mr);
}

#endif

/*!
 * @brief Application thread.
 */
void mqtt_thread(void *arg)
{
    struct netif *netif = (struct netif *)arg;
    struct dhcp *dhcp = NULL;
    bool flag = false;

    LOGV("MQTT", "Enter mqtt_thread(), isParsed = %u, ioExp = %u", g_plc_netcfg.wan.isParsed, g_plc_netcfg.wan.ioExp);
    if (g_plc_netcfg.wan.isParsed != 1)
    {
        goto GOT_IP;
    }
    if (g_plc_netcfg.wan.ioExp == WAN_CONFIG_IO_EXP_STATIC_IP)
    {
        goto GOT_IP;
    }
    /* Wait for address from DHCP */
    #if (LOG_OPEN_CONSOLE == 1)
    LOGV("MQTT", "Getting IP address from DHCP...netif = 0x%08X\r\n", netif);
    #else
    printf("Getting IP address from DHCP...netif = 0x%08X\r\n", netif);
    #endif
    if (gWanOK == true)
    {
        flag = true;
    }
    do
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        if (flag == false)
        {
            err_t ret = netifapi_dhcp_start(&fsl_netif0);
        #if (LOG_OPEN_CONSOLE == 1)
            LOGW("MQTT", "netifapi_dhcp_start return = %d.", ret);
        #else
            printf("netifapi_dhcp_start return = %d.\r\n", ret);
        #endif
            if (ret == 0)
            {
                flag = true;
            }
        }
        if (dhcp == NULL)
        {
            if (netif_is_up(netif))
            {
                dhcp = netif_dhcp_data(netif);
            }
        }
    #if (LOG_OPEN_CONSOLE == 1)
        LOGV("MQTT", "dhcp = 0x%08X", dhcp);
    #else
        printf("dhcp = 0x%08X\r\n", dhcp);
    #endif
        if (dhcp != NULL)
        {
        #if (LOG_OPEN_CONSOLE == 1)
            LOGV("MQTT", "dhcp->state = 0x%08X", dhcp->state);
        #else
            printf("dhcp->state = 0x%08X\r\n", dhcp->state);
        #endif
        }
    }
    while ((dhcp == NULL) || (dhcp->state != DHCP_STATE_BOUND));

GOT_IP:
    if (g_plc_netcfg.wan.isParsed != 1)
    {
        ip_addr_t dnsIP;
        IP4_ADDR(&dnsIP, 192U, 168U, 0U, 1U);
        dns_setserver(0, &dnsIP);
    }
    else
    {
        LOGI("MQTT", "IPv4 Gateway     : %s", ipaddr_ntoa(&netif->gw));
        LOGV("MQTT", "g_plc_netcfg.wan.dns = %s", ipaddr_ntoa(&g_plc_netcfg.wan.dns));
        dns_setserver(0, &g_plc_netcfg.wan.dns);
    }
    g_plc_netcfg.wan.ip.addr = netif->ip_addr.addr;
    LOGV("MQTT1", "g_plc_netcfg.wan.ip.addr = %s", ipaddr_ntoa(&g_plc_netcfg.wan.ip));
    LOGI("MQTT1", "ip_addr.addr = %s", ipaddr_ntoa(&netif->ip_addr));

    //xEventGroupSetBits(g_kalyke_event_group, KALYKE_EVENT_GOT_IP_SNTP);
    xEventGroupSetBits(g_kalyke_event_group, KALYKE_EVENT_GOT_IP_TCP_SERVER);
    xEventGroupSetBits(g_kalyke_event_group, KALYKE_EVENT_GOT_IP_TCP_SERVER2);
    xEventGroupSetBits(g_kalyke_event_group, KALYKE_EVENT_GOT_IP_TCP_SERVER3);
    plc_set_bit_element_value(SM_ELEMENT, SM269, 1);
    gWanOK = true;
    
    LOGV("MQTT", "IPv4 Address     : %s", ipaddr_ntoa(&netif->ip_addr));
    LOGD("MQTT", "IPv4 Subnet mask : %s", ipaddr_ntoa(&netif->netmask));
    LOGI("MQTT", "IPv4 Gateway     : %s", ipaddr_ntoa(&netif->gw));

    for (;;)
    {
        if (g_plc_netcfg.mqtt.isParsed == 0)
        {
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            continue;
        }
        else
        {
            break;
        }
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS);
#if (WAN_4G_SWITCH_AUTO == 0)
    if (g_plc_netcfg.surfing == 1)
    {
        gMqttSource = MQTT_4G;
        vTaskDelay(5000);
        LOGW("MQTT", "g_plc_netcfg.surfing == 1, so just delete mqtt task.");
        vTaskDelete(NULL);
        return;
    }
    gMqttSource = MQTT_ENET;
#endif
    mqtt_msg_st mqttMsg;
    gMQTTMsgQueueHandle = xQueueCreate(5, sizeof(mqtt_msg_st));
    configASSERT(gMQTTMsgQueueHandle != NULL);

    print_vTaskList(__func__, __LINE__);
    mqtt_init();
    for (;;)
    {
        LOGD("MQTT", "Let us wait MQTT something happen...");
        if (xQueueReceive(gMQTTMsgQueueHandle, &mqttMsg, portMAX_DELAY) == pdFALSE)
        {
            continue;
        }

        LOGV("MQTT", "Received msg....type = %u, Free heap = %d(bytes)", mqttMsg.type, xPortGetFreeHeapSize());
        if (mqttMsg.type == 2)
        {
            mqtt_init();
            continue;
        }
        else if (mqttMsg.dataBuff == NULL) // MQTT had disconnected!
        {
        #if 0
            if (mqtt_client)
            {
                mqtt_client_free(mqtt_client);
                mqtt_client = NULL;
            }
        #endif
            vTaskDelay(15000 / portTICK_PERIOD_MS);
            
            mqtt_init();
            continue;
        }
        if (mqttMsg.type == 1)
        {
            kalyke_mqtt_publish_do((char *)mqttMsg.topic, (char *)mqttMsg.dataBuff, mqttMsg.qos);
            vPortFree(mqttMsg.topic);
        }
        else
        {
            handle_mqtt_recv_data(&mqttMsg);
        }
        vPortFree(mqttMsg.dataBuff);
        if (mqttMsg.topic)
        {
            vPortFree(mqttMsg.topic);
        }
    }

#if 0
    /* Publish some messages */
    for (i = 0; i < 5;)
    {
        if (connected)
        {
            err = tcpip_callback(publish_message, NULL);
            if (err != ERR_OK)
            {
                PRINTF("Failed to invoke publishing of a message on the tcpip_thread: %d.\r\n", err);
            }
            i++;
        }

        sys_msleep(1000U);
    }

    vTaskDelete(NULL);
#endif
}

#if 0
void mqtt_thread2(void *arg)
{
    struct netif *netif = (struct netif *)arg;
    struct dhcp *dhcp;
    //err_t err;
    //int i;

    if (g_plc_netcfg.wan.ioExp == WAN_CONFIG_IO_EXP_STATIC_IP)
    {
        goto GOT_IP;
    }

    /* Wait for address from DHCP */
    PRINTF("2,Getting IP address from DHCP...netif = 0x%08X\r\n", netif);
    do
    {
        if (netif_is_up(netif))
        {
            dhcp = netif_dhcp_data(netif);
        }
        else
        {
            dhcp = NULL;
        }
        LOGW("kalyke_internet_task", "2, dhcp = 0x%08X", dhcp);
        if (dhcp != NULL)
        {
            LOGV("kalyke_internet_task", "2, dhcp->state = 0x%08X", dhcp->state);
        }
        //sys_msleep(1000U);
        vTaskDelay(1000 / portTICK_PERIOD_MS);

    }
    while ((dhcp == NULL) || (dhcp->state != DHCP_STATE_BOUND));

GOT_IP:
    PRINTF("\r\n2,IPv4 Address     : %s\r\n", ipaddr_ntoa(&netif->ip_addr));
    PRINTF("2,IPv4 Subnet mask : %s\r\n", ipaddr_ntoa(&netif->netmask));
    PRINTF("2,IPv4 Gateway     : %s\r\n\r\n", ipaddr_ntoa(&netif->gw));
    mqtt_msg_st mqttMsg;
    gMQTTMsgQueueHandle2 = xQueueCreate(5, sizeof(mqtt_msg_st));
    configASSERT(gMQTTMsgQueueHandle2 != NULL);
    for (;;)
    {
        if (g_plc_netcfg.mqtt.isParsed == 0)
        {
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            continue;
        }
        else
        {
            break;
        }
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    mqtt_init2();

    for (;;)
    {
        LOGD("mqtt_task ENET", "Let us wait MQTT something happen...");
        if (xQueueReceive(gMQTTMsgQueueHandle2, &mqttMsg, portMAX_DELAY) == pdFALSE)
        {
            continue;
        }

        LOGV("mqtt_task", "Receive msg...");
        if (mqttMsg.dataBuff == NULL) // MQTT had disconnected!
        {
            mqtt_client_free(mqtt_client2);
            mqtt_client2 = NULL;
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            mqtt_init2();
            continue;
        }

        handle_mqtt_recv_data(&mqttMsg);
        vPortFree(mqttMsg.dataBuff);
    }
}
#endif

// enet : 1 = LAN, 0 = WAN
void get_Mac(uint8_t *pMac, uint32_t enet)
{
    PRINTF("Enter %s(), pMac = 0x%08X\r\n", __func__, pMac);
    uint64_t id0 = OCOTP->CFG0;
    uint64_t id1 = OCOTP->CFG1;
    PRINTF("The unique ID = %08llX, %08llX\r\n", id0, id1);// 65F8296A 0E44B9D2
    // 65F82484 1B09B9D2
    pMac[0] = (id0 >> 24) & 0xFEu;
    pMac[1] = (id0 >> 8) & 0xFFu;
    pMac[2] = id0 & 0xFFu;
    pMac[3] = (id1 >> 24) & 0xFFu;
    pMac[4] = (id1 >> 16) & 0xFFu;
    pMac[5] = (id1 >> 8) & 0xFFu;

    if (enet == 1)
    {
        pMac[5] += 1;
    }
    else
    {
        gMcuID = id1 | (id0 << 32);
        LOGV("internet", "gMcuID = 0x%016llX", gMcuID);
    }
}

static void re_init_ENET_WAN_task(void *arg)
{
    err_t ret;
    ip4_addr_t fsl_netif0_ipaddr, fsl_netif0_netmask, fsl_netif0_gw;
    ethernetif_config_t fsl_enet_config0;
    fsl_enet_config0.phyHandle = &phyHandle1;
    get_Mac(fsl_enet_config0.macAddress, 0);

    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};

    LOGV("WAN", "Enter %s()", __func__);

    //BOARD_InitModuleClock();

    IOMUXC_EnableMode(IOMUXC_GPR, kIOMUXC_GPR_ENET1TxClkOutputDir, true);
    /* ENET1 RST */
    GPIO_PinInit(ENET1_RST_GPIO, ENET1_RST_PIN, &gpio_config);
    GPIO_PinWrite(ENET1_RST_GPIO, ENET1_RST_PIN, 0);
    delay();
    GPIO_PinWrite(ENET1_RST_GPIO, ENET1_RST_PIN, 1);

    mdioHandle1.resource.csrClock_Hz = EXAMPLE_CLOCK_FREQ;
    LOGD("WAN", "mdioHandle1.resource.csrClock_Hz = %u", mdioHandle1.resource.csrClock_Hz);

    LOGI("WAN", "g_plc_netcfg.wan.ioExp = %u", g_plc_netcfg.wan.ioExp);
    if (g_plc_netcfg.wan.ioExp == WAN_CONFIG_IO_EXP_FEXLINK) //智能设备模式 wan和lan都用于Fexlink总线
    {
        IP4_ADDR(&fsl_netif0_ipaddr, 192U, 168U, 0U, 10U);
        IP4_ADDR(&fsl_netif0_netmask, 255U, 255U, 255U, 0U);
        IP4_ADDR(&fsl_netif0_gw, 192U, 168U, 0U, 1U);

        ret = netifapi_netif_add(&fsl_netif0, &fsl_netif0_ipaddr, &fsl_netif0_netmask, &fsl_netif0_gw, &fsl_enet_config0,
                                ethernetif0_init, ethernet_input);
        LOGI("WAN", "netifapi_netif_add return : %d", ret);

        ret = netifapi_netif_set_default(&fsl_netif0);
        LOGI("WAN", "netifapi_netif_set_default return : %d", ret);

        ret = netifapi_netif_set_up(&fsl_netif0);
        LOGD("WAN", "Leave %s(), &fsl_netif0 = 0x%08X", __func__, &fsl_netif0);

        return;
    }

    if (g_plc_netcfg.wan.isParsed != 1)
    {
        IP4_ADDR(&fsl_netif0_ipaddr, 192U, 168U, 0U, 10U);
        IP4_ADDR(&fsl_netif0_netmask, 255U, 255U, 255U, 0U);
        IP4_ADDR(&fsl_netif0_gw, 192U, 168U, 0U, 1U);
    }
    else if (g_plc_netcfg.wan.ioExp == WAN_CONFIG_IO_EXP_STATIC_IP)
    {
        fsl_netif0_ipaddr.addr = g_plc_netcfg.wan.ip.addr;
        fsl_netif0_netmask.addr = g_plc_netcfg.wan.mask.addr;
        fsl_netif0_gw.addr = g_plc_netcfg.wan.gate.addr;
    }
    else
    {
        IP4_ADDR(&fsl_netif0_ipaddr, 0U, 0U, 0U, 0U);
        IP4_ADDR(&fsl_netif0_netmask, 0U, 0U, 0U, 0U);
        IP4_ADDR(&fsl_netif0_gw, 0U, 0U, 0U, 0U);
    }
    LOGV("WAN", "gKalykeTCPIPHandle = 0x%08X", gKalykeTCPIPHandle);
#if 0
    if (gKalykeTCPIPHandle == NULL)
    {
        tcpip_init(NULL, NULL);
        gKalykeTCPIPHandle = (void*)0x01U;
    }
#endif
    ret = netifapi_netif_add(&fsl_netif0, &fsl_netif0_ipaddr, &fsl_netif0_netmask, &fsl_netif0_gw, &fsl_enet_config0,
                             ethernetif0_init, tcpip_input);
    LOGV("WAN", "ret = %d............000", ret);
    
    if (link_is_up(&phyHandle1))
    {
        gWanOK = true;
        plc_set_bit_element_value(SM_ELEMENT, SM269, 1);
        ret = netifapi_netif_set_up(&fsl_netif0);
        LOGI("WAN", "ret = %d............002", ret);
        LOGV("WAN", "WAN is up, so set it to default");
        ret = netifapi_netif_set_default(&fsl_netif0);
    }
    else
    {
        plc_set_bit_element_value(SM_ELEMENT, SM270, 1);
        LOGV("WAN", "LAN is up, so set it to default");
        ret = netifapi_netif_set_default(&fsl_netif1);
    }
    if (g_plc_netcfg.wan.isParsed == 1)//已解析了系统块了
    {
        if (g_plc_netcfg.wan.ioExp == WAN_CONFIG_IO_EXP_DHCP_IP)
        {
            ret = netifapi_dhcp_start(&fsl_netif0);
            LOGW("WAN", "ret = %d............003", ret);
        }
    }
/*  
    PRINTF("\r\n************************************************\r\n");
    PRINTF(" MQTT client use WAN.\r\n");
    PRINTF("************************************************\r\n");
    if (sys_thread_new("mqtt_task", mqtt_thread, &fsl_netif0, MQTT_TASK_STACK_SIZE, MQTT_TASK_PRIO) == NULL)
    {
        LOGE("WAN", "Mqtt Task creation failed.");
    }
    LOGD("WAN", "g_plc_netcfg.wan.mobbustcpServer = %u", g_plc_netcfg.wan.mobbustcpServer);
     
    //只要是tcp/ip就启动三个tcpserver
    gTcpServerMutex = xSemaphoreCreateMutex();
    start_tcp_server();
    start_tcp_server2();
    start_tcp_server3();

    kalyke_start_MicroLink_client();
*/
    vTaskDelete(NULL);
}


void re_init_ENET_WAN(void)
{
    LOGV("WAN", "Enter %s()", __func__);
    err_t ret1 = netifapi_netif_set_down(&fsl_netif0);
    LOGV("WAN", "ret1 = %d");    
    err_t ret2 = netifapi_netif_remove(&fsl_netif0);
    LOGI("WAN", "ret2 = %d");    
    BaseType_t ret = xTaskCreate((TaskFunction_t)re_init_ENET_WAN_task,
                                 (const char *)"re_init",
                                 1024,
                                 NULL,
                                 12,
                                 NULL);
    if (ret != pdPASS)
    {
        LOGE("ENET2", "Create re_init_ENET_WAN_task ERROR!");
    }
    LOGD("ENET2", "Leave %s()", __func__);
}

void init_internet_ENET(void)
{
    err_t ret;
    ip4_addr_t fsl_netif0_ipaddr, fsl_netif0_netmask, fsl_netif0_gw;
    ethernetif_config_t fsl_enet_config0;
    fsl_enet_config0.phyHandle = &phyHandle1;
    get_Mac(fsl_enet_config0.macAddress, 0);

    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
    //LOGV("ENET1", "Enter %s(), &gpio_config = 0x%08X, &fsl_netif0 = 0x%08X", __func__, &gpio_config, &fsl_netif0);
    LOGV("ENET1", "Enter %s()", __func__);

    //BOARD_InitModuleClock();

    IOMUXC_EnableMode(IOMUXC_GPR, kIOMUXC_GPR_ENET1TxClkOutputDir, true);
    /* ENET1 RST */
    GPIO_PinInit(ENET1_RST_GPIO, ENET1_RST_PIN, &gpio_config);
    GPIO_PinWrite(ENET1_RST_GPIO, ENET1_RST_PIN, 0);
    delay();
    GPIO_PinWrite(ENET1_RST_GPIO, ENET1_RST_PIN, 1);

    mdioHandle1.resource.csrClock_Hz = EXAMPLE_CLOCK_FREQ;
    LOGD("ENET1", "mdioHandle1.resource.csrClock_Hz = %u", mdioHandle1.resource.csrClock_Hz);

    LOGI("ENET1", "g_plc_netcfg.wan.ioExp = %u", g_plc_netcfg.wan.ioExp);
    if (g_plc_netcfg.wan.ioExp == WAN_CONFIG_IO_EXP_FEXLINK) //智能设备模式 wan和lan都用于Fexlink总线
    {
        //copy in slave
#if (LWIP_IPV4 == 1)
        IP4_ADDR(&fsl_netif0_ipaddr, 192U, 168U, 0U, 10U);
        IP4_ADDR(&fsl_netif0_netmask, 255U, 255U, 255U, 0U);
        IP4_ADDR(&fsl_netif0_gw, 192U, 168U, 0U, 1U);

        ret = netifapi_netif_add(&fsl_netif0, &fsl_netif0_ipaddr, &fsl_netif0_netmask, &fsl_netif0_gw, &fsl_enet_config0,
                                ethernetif0_init, ethernet_input);
#else
        ret = netifapi_netif_add(&fsl_netif0, &fsl_enet_config0, ethernetif0_init, ethernet_input);
#endif
        LOGI("ENET1", "netifapi_netif_add return : %d", ret);

        ret = netifapi_netif_set_default(&fsl_netif0);
        LOGI("ENET1", "netifapi_netif_set_default return : %d", ret);

        ret = netifapi_netif_set_up(&fsl_netif0);
        LOGD("ENET1", "Leave %s(), &fsl_netif0 = 0x%08X", __func__, &fsl_netif0);

        return;
    }

    if (g_plc_netcfg.wan.isParsed != 1)
    {
        IP4_ADDR(&fsl_netif0_ipaddr, 192U, 168U, 0U, 10U);
        IP4_ADDR(&fsl_netif0_netmask, 255U, 255U, 255U, 0U);
        IP4_ADDR(&fsl_netif0_gw, 192U, 168U, 0U, 1U);
    }
    else if (g_plc_netcfg.wan.ioExp == WAN_CONFIG_IO_EXP_STATIC_IP)
    {
        fsl_netif0_ipaddr.addr = g_plc_netcfg.wan.ip.addr;
        fsl_netif0_netmask.addr = g_plc_netcfg.wan.mask.addr;
        fsl_netif0_gw.addr = g_plc_netcfg.wan.gate.addr;
    }
    else
    {
        IP4_ADDR(&fsl_netif0_ipaddr, 0U, 0U, 0U, 0U);
        IP4_ADDR(&fsl_netif0_netmask, 0U, 0U, 0U, 0U);
        IP4_ADDR(&fsl_netif0_gw, 0U, 0U, 0U, 0U);
    }
    LOGV("ENET1", "gKalykeTCPIPHandle = 0x%08X", gKalykeTCPIPHandle);
    if (gKalykeTCPIPHandle == NULL)
    {
        tcpip_init(NULL, NULL);
        gKalykeTCPIPHandle = (void*)0x01U;
    }

    ret = netifapi_netif_add(&fsl_netif0, &fsl_netif0_ipaddr, &fsl_netif0_netmask, &fsl_netif0_gw, &fsl_enet_config0,
                             ethernetif0_init, tcpip_input);
    LOGV("ENET1", "ret = %d............000", ret);
    //LOGD("ENET1", "ret = %d............001", ret);
    
    if (link_is_up(&phyHandle1))
    {
        gWanOK = true;
        plc_set_bit_element_value(SM_ELEMENT, SM269, 1);
        ret = netifapi_netif_set_up(&fsl_netif0);
        LOGI("ENET1", "ret = %d............002", ret);
        LOGV("ENET1", "WAN is up, so set it to default");
        ret = netifapi_netif_set_default(&fsl_netif0);
    }
    else
    {
        plc_set_bit_element_value(SM_ELEMENT, SM270, 1);
        LOGV("ENET1", "LAN is up, so set it to default");
        ret = netifapi_netif_set_default(&fsl_netif1);
    }
    if (g_plc_netcfg.wan.isParsed == 1)//已解析了系统块了
    {
        if (g_plc_netcfg.wan.ioExp == WAN_CONFIG_IO_EXP_DHCP_IP)
        {
            ret = netifapi_dhcp_start(&fsl_netif0);
            LOGW("ENET1", "ret = %d............003", ret);
        }
    }
    PRINTF("\r\n************************************************\r\n");
    PRINTF(" MQTT client use ENET1\r\n");
    PRINTF("************************************************\r\n");
    if (sys_thread_new("mqtt_task", mqtt_thread, &fsl_netif0, MQTT_TASK_STACK_SIZE, MQTT_TASK_PRIO) == NULL)
    {
        LOGE("kalyke_internet_task", "Mqtt Task creation failed.");
    }
    LOGD("ENET1", "g_plc_netcfg.wan.mobbustcpServer = %u", g_plc_netcfg.wan.mobbustcpServer);
      
    //只要是tcp/ip就启动三个tcpserver
    gTcpServerMutex = xSemaphoreCreateMutex();
    start_tcp_server();
    start_tcp_server2();
    start_tcp_server3();

    kalyke_start_MicroLink_client();
}

#if (KALYKE_TOUCHUAN_WAN_LAN == 1)
static void log_pbuf(struct pbuf *p)
{
    LOGD("TOUCHUAN", "Enter %s(), p = 0x%08X", __func__, p);
    LOGV("TOUCHUAN", "p->next = 0x%08X", p->next);
    LOGD("TOUCHUAN", "p->payload = 0x%08X", p->payload);
    LOGV("TOUCHUAN", "p->tot_len = %u, p->len = %u", p->tot_len, p->len);
    hexdump(p->payload, p->len);
    LOGD("TOUCHUAN", "p->type_internal = 0x%X", p->type_internal);
    LOGD("TOUCHUAN", "p->flags = 0x%X", p->flags);
    LOGV("TOUCHUAN", "p->ref = %u", p->ref);
    LOGD("TOUCHUAN", "p->if_idx = %u", p->if_idx);
}

bool isLanNetif(struct netif *netif)
{
    if (netif == (&fsl_netif1))//LAN
    {
        return true;
    }
    return false;
}

/**
 *
 */
err_t touchuan_ethernet_input(struct pbuf *p, struct netif *netif)
{
#if 1
    /* points to packet payload, which starts with an Ethernet header */
    struct eth_hdr *ethhdr = (struct eth_hdr *)p->payload;
    log_pbuf(p);    
    LOGW("TOUCHUAN", "ethernet_input: dest:%"X8_F":%"X8_F":%"X8_F":%"X8_F":%"X8_F":%"X8_F", src:%"X8_F":%"X8_F":%"X8_F":%"X8_F":%"X8_F":%"X8_F", type:%"X16_F"\n",
               (unsigned char)ethhdr->dest.addr[0], (unsigned char)ethhdr->dest.addr[1], (unsigned char)ethhdr->dest.addr[2],
               (unsigned char)ethhdr->dest.addr[3], (unsigned char)ethhdr->dest.addr[4], (unsigned char)ethhdr->dest.addr[5],
               (unsigned char)ethhdr->src.addr[0],  (unsigned char)ethhdr->src.addr[1],  (unsigned char)ethhdr->src.addr[2],
               (unsigned char)ethhdr->src.addr[3],  (unsigned char)ethhdr->src.addr[4],  (unsigned char)ethhdr->src.addr[5],
               lwip_htons(ethhdr->type));
#endif

#if 0
    if (netif == (&fsl_netif1)) // LAN
    {
        daisy_handle_lan_received_data((uint8_t *)p->payload + SIZEOF_ETH_HDR, p->len - SIZEOF_ETH_HDR);
    }
#endif
#if 0
    if (netif == (&fsl_netif0))//WAN
    {
        ethernet_output_daisy(&fsl_netif1, p);//Send to LAN
    }
    else if (netif == (&fsl_netif1))//LAN
    {
        ethernet_output_daisy(&fsl_netif0, p);//Send to WAN
    }
#else
    if (gTouChuan == 4) // 此时说明LAN口收到了数据，所以要原封不动地传给WAN
    {
        if (ETHTYPE_IP == lwip_htons(ethhdr->type) || 
            ETHTYPE_ARP == lwip_htons(ethhdr->type))
        {
            //goto EXIT_ME;
        }
        //ethernet_output_daisy(&fsl_netif0, p);//Send to WAN
        //MicroLink_touchuan_send_data_2_tcp((uint8_t *)p->payload + SIZEOF_ETH_HDR, p->len - SIZEOF_ETH_HDR);
        #if 0
        touchuan_msg_st msg;
        msg.msgLength = p->len - SIZEOF_ETH_HDR;
        msg.dataBuff  = pvPortMalloc(p->len);
        memcpy(msg.dataBuff, p->payload + SIZEOF_ETH_HDR, msg.msgLength);
        #elif 0
        touchuan_msg_st msg;
        msg.msgLength = p->len - SIZEOF_ETH_HDR;
        msg.dataBuff  = gTCPSendBufENET2;
        memcpy(msg.dataBuff, p->payload + SIZEOF_ETH_HDR, msg.msgLength);
        #else
        touchuan_msg_st msg;
        msg.msgLength = p->len;
        msg.dataBuff  = gTCPSendBufENET2;
        memcpy(msg.dataBuff, p->payload, msg.msgLength);
        #endif
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        
        xQueueSendFromISR(gTouchuanQueueHandle, &msg, &xHigherPriorityTaskWoken);
        if( xHigherPriorityTaskWoken )
        {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
#endif

EXIT_ME:
    pbuf_free(p);
    return ERR_OK;
}

static void re_init_ENET2_LAN_task(void *arg)
{
    ip4_addr_t fsl_netif1_ipaddr, fsl_netif1_netmask, fsl_netif1_gw;
    ethernetif_config_t fsl_enet_config1;
    fsl_enet_config1.phyHandle = &phyHandle2;
    get_Mac(fsl_enet_config1.macAddress, 1);

    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
    //LOGV("ENET2", "Enter %s(), &gpio_config = 0x%08X, &fsl_netif1 = 0x%08X", __func__, &gpio_config, &fsl_netif1);
    LOGV("ENET2", "Enter %s()", __func__);
#if 0
    BOARD_InitModuleClock();
    IOMUXC_EnableMode(IOMUXC_GPR, kIOMUXC_GPR_ENET2TxClkOutputDir, true);
#endif
    /* ENET2 RST */
    GPIO_PinInit(ENET2_RST_GPIO, ENET2_RST_PIN, &gpio_config);
    GPIO_PinWrite(ENET2_RST_GPIO, ENET2_RST_PIN, 0);
    delay();
    GPIO_PinWrite(ENET2_RST_GPIO, ENET2_RST_PIN, 1);

    mdioHandle2.resource.csrClock_Hz = EXAMPLE_CLOCK_FREQ;

    LOGI("ENET2", "g_plc_netcfg.lan.ioExp = %u", g_plc_netcfg.lan.ioExp);

    fsl_netif1_ipaddr.addr = g_plc_netcfg.lan.ip.addr;
    fsl_netif1_netmask.addr = g_plc_netcfg.lan.mask.addr;
    fsl_netif1_gw.addr = g_plc_netcfg.lan.gate.addr;
    
    LOGD("ENET2", "ENET2: IPv4 Address : %s", ipaddr_ntoa(&fsl_netif1_ipaddr));
    LOGD("ENET2", "IPv4 Subnet mask : %s", ipaddr_ntoa(&fsl_netif1_netmask));
    LOGD("ENET2", "IPv4 Gateway     : %s", ipaddr_ntoa(&fsl_netif1_gw));
    LOGV("ENET2", "gKalykeTCPIPHandle = 0x%08X", gKalykeTCPIPHandle);

    err_t ret;
    ret = netifapi_netif_add(&fsl_netif1, &fsl_netif1_ipaddr, &fsl_netif1_netmask, &fsl_netif1_gw, &fsl_enet_config1,
                             ethernetif1_init, ethernet_input);
    LOGI("ENET2", "netifapi_netif_add return = %d", ret);
    if (link_is_up(&phyHandle2))
    {
        LOGI("ENET2", "Link of LAN is up");
        gLanOK = true;
    }
    else
    {
        LOGE("ENET2", "Link of LAN is not up.");
    }
#if 0
    ret = netifapi_netif_set_default(&fsl_netif1);
    LOGD("ENET2", "ret = %d............005", ret);
#endif
    ret = netifapi_netif_set_up(&fsl_netif1);
    LOGI("ENET2", "netifapi_netif_set_up return = %d", ret);

    PRINTF("\r\n************************************************\r\n");
    PRINTF(" Kalyke ENET2 will start\r\n");
    PRINTF("************************************************\r\n");
    vTaskDelete(NULL);
}

void re_init_ENET2_LAN(void)
{
    LOGV("ENET2", "Enter %s()", __func__);
    err_t ret1 = netifapi_netif_set_down(&fsl_netif1);
    LOGV("ENET2", "ret1 = %d");    
    err_t ret2 = netifapi_netif_remove(&fsl_netif1);
    LOGI("ENET2", "ret2 = %d");    
    BaseType_t ret = xTaskCreate((TaskFunction_t)re_init_ENET2_LAN_task,
                                 (const char *)"re_init",
                                 1024,
                                 NULL,
                                 12,
                                 NULL);
    if (ret != pdPASS)
    {
        LOGE("ENET2", "Create re_init ERROR!");
    }
    LOGD("ENET2", "Leave %s()", __func__);
}
#endif

//Init LAN
void init_internet_ENET2(void)
{
    ip4_addr_t fsl_netif1_ipaddr, fsl_netif1_netmask, fsl_netif1_gw;
    ethernetif_config_t fsl_enet_config1;
    fsl_enet_config1.phyHandle = &phyHandle2;
    get_Mac(fsl_enet_config1.macAddress, 1);

    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
    //LOGV("ENET2", "Enter %s(), &gpio_config = 0x%08X, &fsl_netif1 = 0x%08X", __func__, &gpio_config, &fsl_netif1);
    LOGV("ENET2", "Enter %s()", __func__);

    BOARD_InitModuleClock();

    IOMUXC_EnableMode(IOMUXC_GPR, kIOMUXC_GPR_ENET2TxClkOutputDir, true);
    /* ENET2 RST */
    GPIO_PinInit(ENET2_RST_GPIO, ENET2_RST_PIN, &gpio_config);
    GPIO_PinWrite(ENET2_RST_GPIO, ENET2_RST_PIN, 0);
    delay();
    GPIO_PinWrite(ENET2_RST_GPIO, ENET2_RST_PIN, 1);

    mdioHandle2.resource.csrClock_Hz = EXAMPLE_CLOCK_FREQ;
    
    LOGI("ENET2", "g_plc_netcfg.lan.ioExp = %u", g_plc_netcfg.lan.ioExp);
    if (g_plc_netcfg.lan.ioExp == LAN_CONFIG_IO_EXP_MODBUSTCP)
    {
        fsl_netif1_ipaddr.addr = g_plc_netcfg.lan.ip.addr;
        fsl_netif1_netmask.addr = g_plc_netcfg.lan.mask.addr;
        fsl_netif1_gw.addr = g_plc_netcfg.lan.gate.addr;
    }
    else if (g_plc_netcfg.lan.ioExp == LAN_CONFIG_IO_EXP_FEXLINK)
    {
        IP4_ADDR(&fsl_netif1_ipaddr, 192U, 168U, 0U, 240U);
        IP4_ADDR(&fsl_netif1_netmask, 255U, 255U, 255U, 0U);
        IP4_ADDR(&fsl_netif1_gw, 192U, 168U, 0U, 1U);
    }
    else
    {
        IP4_ADDR(&fsl_netif1_ipaddr, 192U, 168U, 0U, 241U);
        IP4_ADDR(&fsl_netif1_netmask, 255U, 255U, 255U, 0U);
        IP4_ADDR(&fsl_netif1_gw, 192U, 168U, 0U, 1U);
    }
    PRINTF("ENET2: IPv4 Address     : %s\r\n", ipaddr_ntoa(&fsl_netif1_ipaddr));
    PRINTF("IPv4 Subnet mask : %s\r\n", ipaddr_ntoa(&fsl_netif1_netmask));
    PRINTF("IPv4 Gateway     : %s\r\n\r\n", ipaddr_ntoa(&fsl_netif1_gw));
    LOGV("ENET2", "gKalykeTCPIPHandle = 0x%08X", gKalykeTCPIPHandle);
    if (gKalykeTCPIPHandle == NULL)
    {
        tcpip_init(NULL, NULL);
        gKalykeTCPIPHandle = (void *)0x02U;
    }

    err_t ret;
    if (gTouChuan == 4)
    {
        ret = netifapi_netif_add(&fsl_netif1, &fsl_netif1_ipaddr, &fsl_netif1_netmask, &fsl_netif1_gw, &fsl_enet_config1,
                                 ethernetif1_init, ethernet_input);
    }
    else if ((g_plc_netcfg.lan.ioExp == LAN_CONFIG_IO_EXP_FEXLINK) || (g_plc_netcfg.lan.ioExp == LAN_CONFIG_IO_EXP_ETHERCAT))
    {
        ret = netifapi_netif_add(&fsl_netif1, &fsl_netif1_ipaddr, &fsl_netif1_netmask, &fsl_netif1_gw, &fsl_enet_config1,
                             ethernetif1_init, ethernet_input);
    }
    else
    {
        ret = netifapi_netif_add(&fsl_netif1, &fsl_netif1_ipaddr, &fsl_netif1_netmask, &fsl_netif1_gw, &fsl_enet_config1,
                             ethernetif1_init, tcpip_input);
    }
    LOGV("ENET2", "ret = %d............004", ret);
    if (link_is_up(&phyHandle2))
    {
        gLanOK = true;
    }

    ret = netifapi_netif_set_default(&fsl_netif1);
    LOGD("ENET2", "ret = %d............005", ret);
    ret = netifapi_netif_set_up(&fsl_netif1);
    LOGI("ENET2", "ret = %d............006", ret);

    PRINTF("\r\n************************************************\r\n");
    PRINTF(" Kalyke ENET2 will start\r\n");
    PRINTF("************************************************\r\n");

    if (gTouChuan == 4)
    {
        start_touchuan_task();
        return;
    }
    if (g_plc_netcfg.lan.ioExp == LAN_CONFIG_IO_EXP_ETHERCAT) //EtherCAT
    {
        kalyke_start_EtherCAT();
    }
    else if (g_plc_netcfg.lan.ioExp == LAN_CONFIG_IO_EXP_MODBUSTCP) //TCP/IP协议
    {
        LOGD("ENET2", "g_plc_netcfg.lan.mobbustcpServer = %u", g_plc_netcfg.lan.mobbustcpServer);
        gEnet2TcpServerMutex = xSemaphoreCreateMutex();
        start_enet2_tcp_server();
        start_enet2_tcp_server2();
        start_enet2_tcp_server3();
 
    }
}

/*!
 * @brief 初始化ENET、ENET2
 */
static int init_internet(void)
{
    LOGV("init_internet", "Enter %s()", __func__);
#if (USE_SECOND_ENET == 1)
    init_internet_ENET2(); // LAN
#endif

    vTaskDelay(3000 / portTICK_PERIOD_MS);

#if (USE_FIRST_ENET == 1)
    init_internet_ENET(); // WAN
#endif

#if SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_SDK
    uint8_t mac[6];
    ENET_GetMacAddr(ENET, mac);
    uint8_t mac2[6];
    ENET_GetMacAddr(ENET2, mac2);
    LOGV("init_internet", "ENET mac address is :");
    hexdump(mac, 6);
    LOGI("init_internet", "ENET2 mac address is :");
    hexdump(mac2, 6);
#endif
    LOGD("init_internet", "&fsl_netif0 = 0x%08X, &fsl_netif1 = 0x%08X", &fsl_netif0, &fsl_netif1);
    LOGV("init_internet", "wan.ip.addr = 0x%08X, lan.ip.addr = 0x%08X", g_plc_netcfg.wan.ip.addr, g_plc_netcfg.lan.ip.addr);
    //vTaskDelay(3000 / portTICK_PERIOD_MS);
    xEventGroupSetBits(g_kalyke_event_group, KALYKE_EVENT_ENET_INIT_DONE_LED);
    xEventGroupSetBits(g_kalyke_event_group, KALYKE_EVENT_ENET_INIT_DONE_PLC);

    LOGV("init_internet", "Leave %s()", __func__);
    return 0;
}
#endif

void kalyke_ethernet_suspend(void)
{

    //if (gKalykeTCPIPHandle)
    {
        //vTaskSuspend(gKalykeTCPIPHandle);
    }
#if 0
    if (gKalykeTCPClientHandle)
    {
        vTaskSuspend(gKalykeTCPClientHandle);
    }
#endif
    //vTaskSuspend(gMqttTaskHandle);
}
void kalyke_ethernet_resume(void)
{

    //if (gKalykeTCPIPHandle)
    {
        //vTaskResume(gKalykeTCPIPHandle);
    }
#if 0
    if (gKalykeTCPClientHandle)
    {
        vTaskResume(gKalykeTCPClientHandle);
    }
    //vTaskResume(gMqttTaskHandle);
#endif
}

//得到当前上网设备
uint16_t getCurrentInternetDevice(void)
{
#if (WAN_4G_SWITCH_AUTO == 0)
    if (g_plc_netcfg.surfing == 0)//WAN
    {
        if (gMqttConnected)
        {
            return 1; // See SD223
        }
        else
        {
            return 0;
        }
    }
    #if (KALYKE_FEATURE_4G_TASK == 1)
    else if (g_plc_netcfg.surfing == 1)//4G
    {
        if (gIs4GMqttConnected)
        {
            return 2;
        }
        else
        {
            return 0;
        }
    }
    #endif
    else if (g_plc_netcfg.surfing == 2)//wifi
    {
        return 0;
    }
    return 0;
#else
    if (gWanOK)
    {
        return 1;
    }
    else
    {
        if (bspIsHave4G())
        {
            return 2;
        }
        else
        {
            return 0;
        }
    }
#endif
}

void kalyke_set_xAN_default(uint8_t wanOrLan)
{
    if (wanOrLan == 0)
    {
        
    }
    else
    {
    }
}
void kalyke_set_WAN_default(void)
{
    err_t err = netifapi_netif_set_up(&fsl_netif0);
    LOGW("internet", "netifapi_netif_set_up return: %d", err);
    err = netifapi_netif_set_default(&fsl_netif0);
    LOGW("internet", "netifapi_netif_set_default return: %d", err);
}
void kalyke_set_LAN_default(void)
{
    netifapi_netif_set_default(&fsl_netif1);
}

struct netif * kalyke_ip4_route_hook(const ip4_addr_t *src, const ip4_addr_t *dest)
{    
    //LOGV("internet", "Enter %s(), src->addr = 0x%08X, dest->addr = 0x%08X", __func__, src->addr, dest->addr);
    //LOGD("internet", "wan.ip.addr = 0x%08X, lan.ip.addr = 0x%08X", g_plc_netcfg.wan.ip.addr, g_plc_netcfg.lan.ip.addr);
    //LOGV("internet", "Enter %s(), src = %s, dest = %s", __func__, ipaddr_ntoa(src), ipaddr_ntoa(dest));
    //LOGD("internet", "wan.ip = %s, lan.ip = %s", ipaddr_ntoa(&g_plc_netcfg.wan.ip), ipaddr_ntoa(&g_plc_netcfg.lan.ip));
    if (src->addr == g_plc_netcfg.wan.ip.addr)
    {
        return &fsl_netif0;
    }
    if (src->addr == g_plc_netcfg.lan.ip.addr)
    {
        return &fsl_netif1;
    }
    return NULL;
}

bool kalyke_can_arp(struct netif *netif)
{
    //LOGV("internet", "Enter %s(), netif = 0x%p, &fsl_netif1 = 0x%08X", __func__, netif, &fsl_netif1);
    if (netif == (&fsl_netif1))
    {
        if (g_plc_netcfg.lan.ioExp == LAN_CONFIG_IO_EXP_ETHERCAT || g_plc_netcfg.lan.ioExp == LAN_CONFIG_IO_EXP_FEXLINK)
        {
            return false;
        }
    }
    return true;
}

static void _log_PHY(void)
{
    uint32_t data1 = 0;
    uint32_t data2 = 0;
    uint32_t id1 = 0;
    uint32_t id2 = 0;
    PHY_Kalyke_Get_Something(&phyHandle1, &data1);
    LOGW("PHY", "data1 = 0x%08X", data1);

    PHY_Kalyke_Get_Something(&phyHandle2, &data2);
    LOGW("PHY", "data2 = 0x%08X", data2);

    PHY_Kalyke_Get_ID1(&phyHandle1, &id1);
    PHY_Kalyke_Get_ID2(&phyHandle1, &id2);
    LOGW("PHY", "id1 = 0x%08X, id2 = 0x%08X", id1, id2);

    uint32_t buf[12] = {0};
    PHY_Kalyke_Read_Others(&phyHandle1, buf);
    hexdump(buf, sizeof(buf));
}

static void kalyke_init_crash_number(void)
{
    PHY_Kalyke_Read_Crash(&phyHandle1, gPHYCrashNumber);
}

static bool kalyke_if_phy_crash(void)
{
    LOGW("PHY", "Enter %s()", __func__);
    uint32_t data[2];
    PHY_Kalyke_Read_Crash(&phyHandle1, data);
    hexdump(data, 8);
    hexdump(gPHYCrashNumber, 8);
    if (gPHYCrashNumber[0] == data[0] && gPHYCrashNumber[1] == data[1])
    {
        return false;
    }
    return true;
}

void kalyke_internet_task(void *arg)
{
    LOGV("internet_task", "kalyke_internet_task RUN. Free heap size is %d bytes.", xPortGetFreeHeapSize());
    //vTaskDelay(8000 / portTICK_PERIOD_MS);
    mqtt_client = NULL;
    for (;;)
    {
        if (g_plc_netcfg.wan.isParsed != 1)
        {
            break;
        }
        else
        {
            break;
        }
    }
    if (bsp_get_deviceID() == 0xFFFF)
    {
        LOGD("internet_task", "Device id = 0xFFFF, so set default ip");
        IP4_ADDR(&g_plc_netcfg.lan.ip, 192U, 168U, 0U, 11U);
        IP4_ADDR(&g_plc_netcfg.lan.mask, 255U, 255U, 255U, 0U);
        IP4_ADDR(&g_plc_netcfg.lan.gate, 192U, 168U, 0U, 1U);
        IP4_ADDR(&g_plc_netcfg.wan.ip, 192U, 168U, 0U, 10U);
        IP4_ADDR(&g_plc_netcfg.wan.mask, 255U, 255U, 255U, 0U);
        IP4_ADDR(&g_plc_netcfg.wan.gate, 192U, 168U, 0U, 1U);
    }
    vTaskDelay(3000);
    init_internet();
    LOGD("internet_task", "PBUF_POOL_BUFSIZE = %u, gWanOK = %d", PBUF_POOL_BUFSIZE, gWanOK);

    if(g_plc_netcfg.wan.ioExp == WAN_CONFIG_IO_EXP_FEXLINK) //智能设备
    {
        vTaskDelete(NULL); //删除自身任务
    }

    kalyke_init_crash_number();
    while (1)
    {
        if (kalyke_if_phy_crash() == true)
        {
            LOGE("PHY", "PHY crash!!!");
            #if 0
            //停止plc
            gtv_PlcRunStatus.mcv_IsOnlineProgram = 1;
            gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.poweron_auto_run = 0;
            gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.cmd_run = 0;
            /*置位停止运行标志*/
            gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.cmd_stop = 1;
            LOGE("PHY", "begin re_init_ENET_WAN()!!!");
            re_init_ENET_WAN();
            LOGE("PHY", "End re_init_ENET_WAN()!!!");
            //运行plc
            /*置位CMD运行标志*/
            gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.cmd_run = 1;
            /*清除停止运行标志*/
            gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.cmd_stop = 0;
            gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.error_status_stop = 0;
            gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.cmd_reboot = 0;	
            gtv_PlcRunStatus.mcv_IsOnlineProgram = 0;
            #endif
            SET_SD_ELEMENT_VALUE( SD7, 1 + GET_SD_ELEMENT_VALUE(SD7) );
            gtv_PlcRunStatus.mcv_IsOnlineProgram = 1;
            LOGE("PHY", "begin re_init_ENET_WAN()!!!");
            re_init_ENET_WAN();
            LOGE("PHY", "End re_init_ENET_WAN()!!!");
            gtv_PlcRunStatus.mcv_IsOnlineProgram = 0;
            vTaskDelay(9999);
            continue;
        }
        if (link_is_up(&phyHandle1))//WAN
        {
            //LOGD("internet_task", "ENET: PHY link up!");
            if (gWanOK == false)
            {
                kalyke_set_WAN_default();
                plc_set_bit_element_value(SM_ELEMENT, SM269, 1);
                gWanOK = true;
            #if (WAN_4G_SWITCH_AUTO == 1)
                if (bspIsHave4G())
                {
                    mqtt_4G_QMTDISC();
                }
                if (gMqttConnected == false)
                {
                    netifapi_netif_set_up(&fsl_netif0);
                    netifapi_netif_set_default(&fsl_netif0);
                    #if 0
                    if (mqtt_client)
                    {
                        mqtt_client_free(mqtt_client);
                        mqtt_client = NULL;
                    }
                    #endif
                    mqtt_msg_st mqttMsg;
                    mqttMsg.type = 2;
                    xQueueSend(gMQTTMsgQueueHandle, &mqttMsg, 0);
                }
            #endif
            }
        }
        else
        {
            LOGW("internet_task", "WAN: PHY link down!");
            if (gWanOK == true)
            {
                if (mqtt_client)
                {
                    //LOGW("internet", "Before calling mqtt_disconnect");
                    //mqtt_disconnect(mqtt_client);
                }
                //gMqttConnected = false;
                plc_set_bit_element_value(SM_ELEMENT, SM269, 0);
                gWanOK = false;
            }
        }

        if (link_is_up(&phyHandle2))//LAN
        {
            //LOGD("internet_task", "ENET2: PHY link up!...");
            if (gLanOK == false)
            {
                plc_set_bit_element_value(SM_ELEMENT, SM270, 1);
                gLanOK = true;
            }
        }
        else
        {
            LOGI("internet_task", "LAN: PHY link down!...");
            if (gLanOK == true)
            {
                plc_set_bit_element_value(SM_ELEMENT, SM270, 0);
                gLanOK = false;
            }
        }
        vTaskDelay(4000 / portTICK_PERIOD_MS);
    }
}

