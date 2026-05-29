/**
  ******************************************************************************
  * @file    plc_tcpins.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2022-07-24
  * @brief   TCP_CONN, TCP_XMT, TCP_RCV 相关指令实现
  ******************************************************************************
  */
#include "plc_tcpins.h"
//#include "plc_variable.h"
#include "plc_commonfunc.h"
#include "plc_element.h"
#include "plc_parseaddr.h"
#include "plc_errormsg.h"
#include "plc_instruction.h"

#include "bsp_uart.h"
#include "bsp_dct.h"
#include "plc_sysblock.h"
#include "kalyke_internet_task.h"

#include "verify_func.h"
#include "fsl_debug_console.h"
#include "kalyke_tool.h"
#include "plc_modbustcpins.h"
#include "kalyke_tcp_task.h"
#include "kalyke_ping_task.h"


static const char *TAG = "TCP_INS";
//0 = Use WAN , 1 = Use LAN
static uint16_t gWanOrLan[MAX_TCP_CONFIG_ITEM];

#if 0
static uint32_t gCounts = 0;
#define MODLINK_DEBUGF(message) do { \
                                if (gCounts++ % 2000 == 0) { \
                                    do {PRINTF message;} while(0); \
                                } \
                             } while(0)
static bool gDebug = true;
#else
static bool gDebug = false;
#define MODLINK_DEBUGF(message)
#endif
static uint8_t gTcpSendBuf[128];
static uint8_t gTcpRecvBuf1[128];
static uint8_t gTcpRecvBuf2[128];
static uint8_t gTcpRecvBuf3[128];
#if 0
static void tcpConnCB(uint8_t connState)
{
}
#endif

static void get_ip_port(uint8_t *ucode_pc, ip4_addr_t *pip, uint16_t *pPort)
{
    LOGV(TAG, "Enter %s()", __func__);
    hexdump(ucode_pc, 22);
    uint16_t elementIndex = GET_PU16_DATA(ucode_pc + 2);
    LOGD(TAG, "elementIndex = %u", elementIndex);
    uint16_t *ptr = &gtv_PlcElement.msp_DElement[elementIndex];
    hexdump(ptr, 16);
    IP4_ADDR(pip, *ptr, *(ptr + 1), *(ptr + 2), *(ptr + 3));
    *pPort = *(ptr + 4);
}

/* CI_TCP_CONN, 0xF180
80 F1 -- CI_TCP_CONN
00 FF 01 00 -- 连接模式 0 = 断开；1 = 连接
00 FF 53 00 -- 连接标识码
00 11 64 00 -- D100，连续5个D元件最后一个是端口号，192.168.0.125:502
00 FF 01 00 -- 0 = WAN, 1 = LAN
00 05 02 00 -- M2, 1 = TCP正在连接中
00 05 03 00 -- M3, 1 = TCP连接已成功建立,  0 = 尚未连接 
00 11 C8 00 -- D200, 存放错误码
00 00 
*/
unsigned char run_ci_tcp_conn_ins(plc_run_power_flow_st *ltp_RunEnv)
{
#if 1
    static uint32_t mTick = 0;
    uint32_t curTick = xTaskGetTickCount();
    if (curTick - mTick > 3000)
    {
        gDebug = true;
        mTick = curTick;
    }
    else
    {
        gDebug = false;
    }
    if (gDebug) LOGI(TAG, "Enter %s(), ltp_RunEnv = 0x%08X", __func__, ltp_RunEnv);
    uint16_t mS1;// 连接模式，0：断开连接，  1：进行连接
    uint16_t mS2;// 网络连接标识码 clientID
    uint8_t mD1; // 1: TCP正在连接中
    uint8_t mD2; // 1: TCP连接已成功建立,  0: 尚未连接 
    int16_t mD3; // 错误码
    uint8_t lcv_Ret;

    //网络连接标识码
    lcv_Ret = get_word_default(ltp_RunEnv->mcp_PC + 6, &mS2);
    if(lcv_Ret != pdPASS)
    {
        if (gDebug) LOGE(TAG, "Leave %s()...001\r\n", __func__);
        return lcv_Ret;
    }
    if (mS2 > MAX_TCP_CONFIG_ITEM || mS2 == 0)
    {
        mD3 = MBCCONNECT_IDENTIFICATION_CODE_ERROR;
        save_word_default(ltp_RunEnv->mcp_PC + 26, (uint16_t*)&mD3);
        if (gDebug) LOGE(TAG, "Leave %s()...002\r\n", __func__);
        return ERR_MDTCP_EXCEEDMAXCONN;
    }
    mS2--;

    // 使用哪个网口连接，0：WAN, 1：LAN
    lcv_Ret = get_word_default(ltp_RunEnv->mcp_PC + 14, &gWanOrLan[mS2]);
    if(lcv_Ret != pdPASS)
    {
        if (gDebug) LOGE(TAG, "Leave %s()...003\r\n", __func__);
        return lcv_Ret;
    }   

    if (gWanOrLan[mS2] == 0) // Use WAN
    {
        if (!plc_get_bit_element_value(SM_ELEMENT, SM269))
        {
            mD3 = MBCCONNECT_NO_LINK;
            save_word_default(ltp_RunEnv->mcp_PC + 26, (uint16_t*)&mD3);
            if (gDebug) LOGE(TAG, "Leave %s(), because WAN not ready now...", __func__);
            return pdPASS;
        }
        else
        {
            mD3 = MBCCONNECT_NO_ERROR;
        }
    }
    else // Use LAN
    {
        if (!plc_get_bit_element_value(SM_ELEMENT, SM270))
        {
            mD3 = MBCCONNECT_NO_LINK;
            save_word_default(ltp_RunEnv->mcp_PC + 26, (uint16_t*)&mD3);
            if (gDebug) LOGE(TAG, "Leave %s(), because LAN not ready now...", __func__);
            return pdPASS;
        }
        else
        {
            mD3 = MBCCONNECT_NO_ERROR;
        }
    }
    
    //连接模式，0：断开连接，  1：进行连接
    lcv_Ret = get_word_default(ltp_RunEnv->mcp_PC + 2, &mS1);
    if(lcv_Ret != pdPASS)
    {
        if (gDebug) LOGE(TAG, "Leave %s()...004\r\n", __func__);
        return lcv_Ret;
    }

    //正在连接
    lcv_Ret = get_char_default(ltp_RunEnv->mcp_PC + 18, &mD1);
    if(lcv_Ret != pdPASS)
    {
        if (gDebug) LOGE(TAG, "Leave %s()...005\r\n", __func__);
        return lcv_Ret;
    }

    //TCP连接已成功建立
    lcv_Ret = get_char_default(ltp_RunEnv->mcp_PC + 22, &mD2);
    if(lcv_Ret != pdPASS)
    {
        if (gDebug) LOGE(TAG, "Leave %s()...006\r\n", __func__);
        return lcv_Ret;
    }

    
    if (gDebug) LOGV(TAG, "mS1 = %u, mS2 = %u, gWanOrLan = %u, mD1 = %u, mD2 = %u, mD3 = %u,", mS1, mS2, gWanOrLan[mS2], mD1, mD2, mD3);

    if(!GET_POWER_FLOW(ltp_RunEnv)) // Power flow not effective, just return.
    {
        if (gDebug) LOGE(TAG, "Power flow not effective, just return.");
        return pdPASS;
    }
    else
    {
        if (gDebug) LOGI(TAG, "Power flow effective!!!");
        save_word_default(ltp_RunEnv->mcp_PC + 26, (uint16_t*)&mD3);
    }

    if (mS1 == 0) // 断开连接
    {
        if (!tcp_client_get_connected_bit(mS2))//如果已是断开状态，直接返回
        {
            if (gDebug) LOGE(TAG, "Leave %s()...007\r\n", __func__);
            return pdPASS;
        }

        if (tcp_client_get_connectting_bit(mS2)) // 如果正在断开，直接返回
        {
            if (gDebug) LOGE(TAG, "Leave %s()...008\r\n", __func__);
            return pdPASS;
        }
        
        stop_tcp_client(mS2);
    }
    else //进行连接
    {
        if (tcp_client_get_connected_bit(mS2)) // Already connected, just return.
        {
            if (gDebug) LOGE(TAG, "Leave %s()...009\r\n", __func__);
            return pdPASS;
        }
        if (tcp_client_get_connectting_bit(mS2)) // Connecting , just return.
        {
            if (gDebug) LOGE(TAG, "Leave %s()...010\r\n", __func__);
            return pdPASS;
        }
        get_ip_port(ltp_RunEnv->mcp_PC + 10, &gModbusTcpConfig.item[mS2].ipTarget, &gModbusTcpConfig.item[mS2].portTarget);
        start_tcp_client(mS2, ltp_RunEnv->mcp_PC);
    }
#endif
    return pdPASS;
}

static void get_send_buffer(uint8_t *ucode_pc, uint8_t *buf, uint16_t sendBytes)
{
    uint16_t i = 0;

    uint16_t elementIndex = GET_PU16_DATA(ucode_pc + 2);

    LOGV(TAG, "Enter %s(), elementIndex = %u", __func__, elementIndex);
    hexdump(ucode_pc, 18);
    uint16_t *dPtr = &gtv_PlcElement.msp_DElement[elementIndex];
    for (i = 0; i < sendBytes; i++)
    {
        buf[i] = *dPtr;
        dPtr++;
    }
}

/* CI_TCP_XMT, 0xF181
81 F1 -- CI_TCP_XMT
00 FF 53 00 -- 连接标识码
00 11 2C 01 -- D300, 发送的起始地址
00 FF 10 00 -- 发送的字节数
00 05 04 00 -- M4, 1 = Sending data
00 11 2E 01 -- D302, 错误码
01 00 
*/
unsigned char run_ci_tcp_xmt_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    uint8_t lcv_Ret;
    uint16_t mS1;// 网络连接标识码 clientID
    uint8_t mD1 = 0; // 1 = Sending data
    int16_t mD2 = 0; // 错误码
    
    if(!GET_POWER_FLOW(ltp_RunEnv)) // Power flow not effective, just return.
    {
        if (gDebug) LOGE(TAG, "XMT: Power flow not effective, just return.");
        tcp_client_set_sending_bit(mS1, 0);
        return pdPASS;
    }
    else
    {
        if (gDebug) LOGI(TAG, "XMT: Power flow effective!!!");
        save_word_default(ltp_RunEnv->mcp_PC + 18, (uint16_t*)&mD2);
    }

    /* 发送标志, 1 = Sending data
     * 一般来讲，在发送完后，会等待接收数据，在这期间，
     * 这个标志一直为1
     */
    lcv_Ret = get_char_default(ltp_RunEnv->mcp_PC + 14, &mD1);
    if(lcv_Ret != pdPASS)
    {
        if (gDebug) LOGE(TAG, "Leave %s()...005\r\n", __func__);
        return lcv_Ret;
    }
#if 0
    if (mD1 == 1)
    {
        if (gDebug) LOGI(TAG, "Sending data, just return.");
        return pdPASS;
    }
#endif
    //网络连接标识码
    lcv_Ret = get_word_default(ltp_RunEnv->mcp_PC + 2, &mS1);
    if(lcv_Ret != pdPASS)
    {
        if (gDebug) LOGE(TAG, "Leave %s()...001\r\n", __func__);
        return lcv_Ret;
    }
    if (mS1 > MAX_TCP_CONFIG_ITEM || mS1 == 0)
    {
        mD2 = MBCCONNECT_IDENTIFICATION_CODE_ERROR;
        save_word_default(ltp_RunEnv->mcp_PC + 18, (uint16_t*)&mD2);
        if (gDebug) LOGE(TAG, "Leave %s()...002\r\n", __func__);
        return ERR_MDTCP_EXCEEDMAXCONN;
    }
    mS1--;//数组下标从0开始

    if (1 == tcp_client_get_sending_bit(mS1))
    {
        if (gDebug) LOGI(TAG, "Have send data, waiting receive...");
        return pdPASS;
    }

    //发送的字节数
    uint16_t sendBytes = 0;
    lcv_Ret = get_word_default(ltp_RunEnv->mcp_PC + 10, &sendBytes);
    if(lcv_Ret != pdPASS)
    {
        if (gDebug) LOGE(TAG, "Leave %s()...003\r\n", __func__);
        return lcv_Ret;
    }
    if (sendBytes == 0)
    {
        return pdPASS;
    }

    static uint8_t sendBuf[64] = {0};
    get_send_buffer(ltp_RunEnv->mcp_PC + 6, sendBuf, sendBytes);
    uint8_t *recvBuf = NULL;
    switch (mS1)
    {
        case 0:
            recvBuf = gTcpRecvBuf1;
            break;
        case 1:
            recvBuf = gTcpRecvBuf2;
            break;
        case 2:
            recvBuf = gTcpRecvBuf3;
            break;
    }

    mD1 = 1;
    save_char_default(ltp_RunEnv->mcp_PC + 14, &mD1);
    tcp_client_set_sending_bit(mS1, 1);
    joanna_tcp_send(sendBuf, sendBytes, mS1, ltp_RunEnv->mcp_PC, 1, recvBuf);
    return pdPASS;
}

static void get_recv_buffer(uint8_t *ucode_pc, uint16_t id, uint16_t recvBytes)
{
	uint8_t *recvBuf = NULL;
	uint16_t elementIndex = GET_PU16_DATA(ucode_pc + 2);
	switch (id)
	{
		case 0:
			recvBuf = gTcpRecvBuf1;
			break;
		case 1:
			recvBuf = gTcpRecvBuf2;
			break;
		case 2:
			recvBuf = gTcpRecvBuf3;
			break;
	}
	uint16_t *dPtr = &gtv_PlcElement.msp_DElement[elementIndex];
	for (uint16_t i = 0; i < recvBytes; i++)
	{
		*dPtr = recvBuf[i];
		dPtr++;
		
	}
}

/* CI_TCP_RCV, 0xF182
82 F1 -- CI_TCP_RCV
00 FF 53 00 -- 连接标识码
00 11 90 01 -- D400, 接收数据的起始地址
00 FF 10 00 -- 接收的最大字节数
00 05 05 00 -- M5, 完成标志 , 1 = 收到数据了
00 11 C2 01 -- D450, 错误码
02 00 
*/
unsigned char run_ci_tcp_rcv_ins(plc_run_power_flow_st *ltp_RunEnv)
{
	uint8_t lcv_Ret;
	uint16_t mS1;    // 网络连接标识码 clientID
	uint16_t mS2;    // 接收数据的起始地址
	uint8_t mD1 = 0; // 完成标志
	int16_t mD2 = 0; // 错误码
	
	if (!GET_POWER_FLOW(ltp_RunEnv)) // Power flow not effective, just return.
    {
        if (gDebug) LOGE(TAG, "RCV: Power flow not effective, just return.");
        return pdPASS;
    }
    else
    {
        if (gDebug) LOGI(TAG, "RCV: Power flow effective!!!");
    }

	//完成标志
    lcv_Ret = get_char_default(ltp_RunEnv->mcp_PC + 14, &mD1);
    if(lcv_Ret != pdPASS)
    {
        if (gDebug) LOGE(TAG, "Leave %s()...001\r\n", __func__);
        return lcv_Ret;
    }

	//网络连接标识码
    lcv_Ret = get_word_default(ltp_RunEnv->mcp_PC + 2, &mS1);
    if(lcv_Ret != pdPASS)
    {
        if (gDebug) LOGE(TAG, "Leave %s()...002\r\n", __func__);
        return lcv_Ret;
    }
    if (mS1 > MAX_TCP_CONFIG_ITEM || mS1 == 0)
    {
        mD2 = MBCCONNECT_IDENTIFICATION_CODE_ERROR;
        save_word_default(ltp_RunEnv->mcp_PC + 18, (uint16_t*)&mD2);
        if (gDebug) LOGE(TAG, "Leave %s()...003\r\n", __func__);
        return ERR_MDTCP_EXCEEDMAXCONN;
    }
    mS1--; //数组下标从0开始

    mD2 = 0;
    save_word_default(ltp_RunEnv->mcp_PC + 18, (uint16_t*)&mD2);

    // There is no data received, so just return.
    if (0 == tcp_client_get_received_bit(mS1))
    {
        mD1 = 0;
        save_char_default(ltp_RunEnv->mcp_PC + 14, &mD1);
        if (gDebug) LOGV(TAG, "no data received, just return");
        return pdPASS;
    }
    tcp_client_set_received_bit(mS1, 0);
    mD1 = 1;
    save_char_default(ltp_RunEnv->mcp_PC + 14, &mD1);

	//接收的最大字节数
    uint16_t recvBytes = 0;
    lcv_Ret = get_word_default(ltp_RunEnv->mcp_PC + 10, &recvBytes);
    if(lcv_Ret != pdPASS)
    {
        if (gDebug) LOGE(TAG, "Leave %s()...004\r\n", __func__);
        return lcv_Ret;
    }
    if (recvBytes == 0)
    {
        if (gDebug) LOGE(TAG, "Leave %s()...005\r\n", __func__);
        return pdPASS;
    }

    get_recv_buffer(ltp_RunEnv->mcp_PC + 6, mS1, recvBytes);
    //收完数据了，可以发下一个数据了
    tcp_client_set_sending_bit(mS1, 0);
    return pdPASS;
}

/* CI_TCP_PING, 0xF183
83 F1 -- CI_TCP_PING
00 11 64 00 -- D100，连续4个D元件存放IP，Such as : 45.78.7.91
00 FF 01 00 -- 0 = WAN, 1 = LAN
00 05 02 00 -- M2, 1 = 正在ping
00 11 C8 00 -- D200, 存放错误码
00 00 
*/
unsigned char run_ci_tcp_ping_ins(plc_run_power_flow_st *ltp_RunEnv)
{
#if 1
    static const char *TAG = "PING_INS";
    static bool pingDebug = false;
    static uint32_t mTick = 0;
    uint32_t curTick = xTaskGetTickCount();
    if (curTick - mTick > 3000)
    {
        pingDebug = true;
        mTick = curTick;
    }
    else
    {
        pingDebug = false;
    }
#else
    static bool pingDebug = false;
#endif

    if (pingDebug) LOGI(TAG, "Enter %s(), ltp_RunEnv = 0x%08X", __func__, ltp_RunEnv);
    uint16_t wanOrLan = 0;
    uint8_t mD1 = 0; // 1: 正在ping
    int16_t mD3 = 0; // 错误码
    uint8_t lcv_Ret;

    // 使用哪个网口连接，0：WAN, 1：LAN
    lcv_Ret = get_word_default(ltp_RunEnv->mcp_PC + 6, &wanOrLan);
    if(lcv_Ret != pdPASS)
    {
        if (pingDebug) LOGE(TAG, "Leave %s()...000", __func__);
        return lcv_Ret;
    }   

    if (wanOrLan == 0) // Use WAN
    {
        if (!plc_get_bit_element_value(SM_ELEMENT, SM269))
        {
            mD3 = MBCCONNECT_NO_LINK;
            save_word_default(ltp_RunEnv->mcp_PC + 14, (uint16_t*)&mD3);
            if (pingDebug) LOGE(TAG, "Leave %s(), because WAN not ready now...", __func__);
            return pdPASS;
        }
        else
        {
            mD3 = MBCCONNECT_NO_ERROR;
        }
    }
    else // Use LAN
    {
        if (!plc_get_bit_element_value(SM_ELEMENT, SM270))
        {
            mD3 = MBCCONNECT_NO_LINK;
            save_word_default(ltp_RunEnv->mcp_PC + 14, (uint16_t*)&mD3);
            if (pingDebug) LOGE(TAG, "Leave %s(), because LAN not ready now...", __func__);
            return pdPASS;
        }
        else
        {
            mD3 = MBCCONNECT_NO_ERROR;
        }
    }

    //正在ping
    lcv_Ret = get_char_default(ltp_RunEnv->mcp_PC + 10, &mD1);
    if(lcv_Ret != pdPASS)
    {
        if (pingDebug) LOGE(TAG, "Leave %s()...001", __func__);
        return lcv_Ret;
    }
    if (pingDebug) LOGV(TAG, "wanOrLan = %u, mD1 = %u, mD3 = %u,", wanOrLan, mD1, mD3);
    if (mD1 != 0) // PINGing
    {
        if (pingDebug) LOGW(TAG, "PINGing, just return.");
        return pdPASS;
    }

    if(!GET_POWER_FLOW(ltp_RunEnv)) // Power flow not effective, just return.
    {
        if (pingDebug) LOGE(TAG, "Power flow not effective, just return.");
        return pdPASS;
    }
    else
    {
        if (pingDebug) LOGI(TAG, "Power flow effective!!!");
        save_word_default(ltp_RunEnv->mcp_PC + 14, (uint16_t*)&mD3);
    }

    mD1 = 1;
    save_char_default(ltp_RunEnv->mcp_PC + 10, &mD1);

    // Let us do ping ...
    ip4_addr_t pingTarget;
    uint16_t port;
    get_ip_port(ltp_RunEnv->mcp_PC + 2, &pingTarget, &port);
    kalyke_start_ping(ltp_RunEnv->mcp_PC, pingTarget, wanOrLan);
    return pdPASS;
}

