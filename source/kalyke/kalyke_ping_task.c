
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdio.h>
#include "lwip/opt.h"

#include "lwip/mem.h"
#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/netif.h"
#include "lwip/sys.h"
#include "lwip/timeouts.h"
#include "lwip/inet_chksum.h"
#include "lwip/prot/ip4.h"

#include "lwip/sockets.h"
#include "lwip/inet.h"
#include <string.h>

#include "netif/ethernet.h"
#include "enet_ethernetif.h"

#include "kalyke_opts.h"
#include "plc_modbustcpins.h"
#include "plc_parseaddr.h"
#include "plc_netcfg.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/** ping receive timeout - in milliseconds */
#ifndef PING_RCV_TIMEO
#define PING_RCV_TIMEO 4000
#endif

/** ping delay - in milliseconds */
#ifndef PING_DELAY
#define PING_DELAY     1000
#endif

/** ping identifier - must fit on a u16_t */
#ifndef PING_ID
#define PING_ID        0xAFAF
#endif

/** ping additional data size to include in the packet */
#ifndef PING_DATA_SIZE
#define PING_DATA_SIZE 32
#endif

/** ping result action - no default action */
#ifndef PING_RESULT
#define PING_RESULT(ping_ok)
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static const char *TAG = "PING";
TaskHandle_t gPingHandle = NULL;
static uint8_t *gPing_PC;
static uint16_t gPingWanOrLan;

/* ping variables */
static ip4_addr_t g_ping_target;
static u16_t ping_seq_num = 0;
static u32_t ping_time;

/*******************************************************************************
 * Code
 ******************************************************************************/

static void save_ping_result(int16_t result)
{
    save_word_default(gPing_PC + 14, (uint16_t*)&result);
}

/** Prepare a echo ICMP request */
static void ping_prepare_echo( struct icmp_echo_hdr *iecho, u16_t len)
{
    size_t i;
    size_t data_len = len - sizeof(struct icmp_echo_hdr);

    ICMPH_TYPE_SET(iecho, ICMP_ECHO);
    ICMPH_CODE_SET(iecho, 0);
    iecho->chksum = 0;
    iecho->id     = PING_ID;
    iecho->seqno  = lwip_htons(++ping_seq_num);

    /* fill the additional data buffer with some data */
    for (i = 0; i < data_len; i++)
    {
        ((char*)iecho)[sizeof(struct icmp_echo_hdr) + i] = (char)i;
    }

    iecho->chksum = inet_chksum(iecho, len);
}

/* Ping using the socket ip */
static err_t kalyke_ping_send(int s, const ip_addr_t *addr)
{
    int err;
    struct icmp_echo_hdr *iecho;
    struct sockaddr_storage to;
    size_t ping_size = sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE;

    iecho = (struct icmp_echo_hdr *)mem_malloc((mem_size_t)ping_size);
    if (!iecho)
    {
        return ERR_MEM;
    }

    ping_prepare_echo(iecho, (u16_t)ping_size);

    if(IP_IS_V4(addr))
    {
        struct sockaddr_in *to4 = (struct sockaddr_in*)&to;
        to4->sin_len    = sizeof(to4);
        to4->sin_family = AF_INET;
        inet_addr_from_ip4addr(&to4->sin_addr, ip_2_ip4(addr));
    }

    err = lwip_sendto(s, iecho, ping_size, 0, (struct sockaddr*)&to, sizeof(to));

    mem_free(iecho);
    return (err ? ERR_OK : ERR_VAL);
}

static void kalyke_ping_recv(int s)
{
    char buf[64];
    int len = 0;
    struct sockaddr_storage from;
    int fromlen = sizeof(from);

    while((len = lwip_recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*)&from, (socklen_t*)&fromlen)) > 0)
    {
        if (len >= (int)(sizeof(struct ip_hdr)+sizeof(struct icmp_echo_hdr)))
        {
            ip_addr_t fromaddr;
            memset(&fromaddr, 0, sizeof(fromaddr));

            if(from.ss_family == AF_INET)
            {
                struct sockaddr_in *from4 = (struct sockaddr_in*)&from;
                inet_addr_to_ip4addr(ip_2_ip4(&fromaddr), &from4->sin_addr);
                IP_SET_TYPE_VAL(fromaddr, IPADDR_TYPE_V4);
            }

            LOGV(TAG, "ping: recv %s %ums", ipaddr_ntoa(&fromaddr), (xTaskGetTickCount() - ping_time));

            if (IP_IS_V4_VAL(fromaddr))
            {
                struct ip_hdr *iphdr;
                struct icmp_echo_hdr *iecho;

                iphdr = (struct ip_hdr *)buf;
                iecho = (struct icmp_echo_hdr *)(buf + (IPH_HL(iphdr) * 4));
                if ((iecho->id == PING_ID) && (iecho->seqno == lwip_htons(ping_seq_num)))
                {
                    /* Do some ping success result processing */
                    //PING_RESULT((ICMPH_TYPE(iecho) == ICMP_ER));
                    save_ping_result(PING_SUCCESS);
                    return;
                }
                else
                {
                    LOGW(TAG, "ping: drop");
                    save_ping_result(PING_DROP);
                }
            }
        }
        fromlen = sizeof(from);
    }

    LOGW(TAG, "len = %d", len);
    if (len == 0 || len == -1 )
    {
        save_ping_result(PING_TIME_OUT);
        LOGE(TAG, "ping: recv - %ums - timeout.", (xTaskGetTickCount() - ping_time));
    }
    else
    {
        save_ping_result(PING_ERR_UNKNOW);
        LOGE(TAG, "ping: recv - %ums - unknow..", (xTaskGetTickCount() - ping_time));
    }
}

static void kalyke_ping_task(void *arg)
{
    int s;
    int ret;

#if LWIP_SO_SNDRCVTIMEO_NONSTANDARD
    int timeout = PING_RCV_TIMEO;
#else
    struct timeval timeout;
    timeout.tv_sec = PING_RCV_TIMEO/1000;
    timeout.tv_usec = (PING_RCV_TIMEO%1000)*1000;
#endif
    LWIP_UNUSED_ARG(arg);

    s = lwip_socket(AF_INET,  SOCK_RAW, IP_PROTO_ICMP);
    if (s < 0)
    {
        return;
    }

    ret = lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    if (ret != 0)
    {
        LOGE(TAG, "setting receive timeout failed");
    }
    
    //ipaddr_ntoa(&fromaddr);
    struct sockaddr_in saddr;
    if (gPingWanOrLan == 0)
    {
        //IP4_ADDR((ip4_addr_t *)&saddr.sin_addr, 192U, 168U, 0U, 10U);
        saddr.sin_addr.s_addr = g_plc_netcfg.wan.ip.addr;
    }
    else
    {
        saddr.sin_addr.s_addr = g_plc_netcfg.lan.ip.addr;
    }
    saddr.sin_family = AF_INET;
    saddr.sin_port = 65252;
    lwip_bind(s, (struct sockaddr *)&saddr, sizeof(saddr));

    if (kalyke_ping_send(s, &g_ping_target) == ERR_OK)
    {
        LOGV(TAG, "ping: send %s", ipaddr_ntoa(&g_ping_target));
        #if (LOG_OPEN == 1)
        ping_time = xTaskGetTickCount();
        #endif
        kalyke_ping_recv(s);
    }
    else
    {
        save_ping_result(PING_SEND_ERR);
        LOGE(TAG, "ping: send %s - error", ipaddr_ntoa(&g_ping_target));
    }
    lwip_close(s);

    // So we can next ping.
    uint8_t mD1 = 0;
    save_char_default(gPing_PC + 10, &mD1);

    gPingHandle = NULL;
    vTaskDelete(NULL);

}

#if (KALYKE_PING_FEATURE == 1)
void kalyke_start_ping(uint8_t *pc, ip4_addr_t pingTarget, uint16_t wanOrLan)
{
    static uint32_t pingTimes = 0;
    LOGV(TAG, "Enter %s(), pc = 0x%08X, pingTarget = 0x%08X, gPingHandle = 0x%08X", __func__, pc, pingTarget.addr, gPingHandle);
    if (gPingHandle != NULL)
    {
        LOGW(TAG, "Pinging, just return.");
        return;
    }
    
    gPing_PC = pc;
    gPingWanOrLan = wanOrLan;
    g_ping_target.addr = pingTarget.addr;

    char taskName[configMAX_TASK_NAME_LEN] = {0};
    sprintf(taskName, "ping_%u", ++pingTimes);
    BaseType_t ret = xTaskCreate((TaskFunction_t)kalyke_ping_task,
                          (const char *)taskName,
                          PING_TASK_STACK_SIZE,
                          (void *)NULL,
                          PING_TASK_PRIO,
                          &gPingHandle);
    if (ret != pdPASS)
    {
        LOGE(TAG, "Create kalyke_ping_task ERROR!  pingTimes = %u", pingTimes);
    }
}
#else
void kalyke_start_ping(uint8_t *pc, ip4_addr_t pingTarget, uint16_t wanOrLan)
{
}
#endif
