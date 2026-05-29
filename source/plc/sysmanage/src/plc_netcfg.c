/**
  ******************************************************************************
  * @file    plc_netcfg.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-04-30
  * @brief   网络配置
  ******************************************************************************
  */

#include "FreeRTOS.h"
#include "fsl_debug_console.h"
#include "plc_netcfg.h"
#include "kalyke_json.h"
#include "kalyke_cJSON.h"
#include "plc_variable.h"
#include "kalyke_tool.h"
#include "kalyke_opts.h"
#include "plc_errormsg.h"
#include "plc_sysblock.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
enum _NetConfig_E
{
    NET_WEI_HU_CENTER =  0xAA4A,
    NET_CLOUD_CENTER  =  0xAA40,
    NET_WAN_CONFIG    =  0xAA41,
    NET_LAN_CONFIG    =  0xAA42,
    NET_MQTT_CONFIG   =  0xAA43,
    NET_WIFI_CONFIG   =  0xAA44,
    NET_3G_CONFIG     =  0xAA45,
    NET_TT_CONFIG     =  0xAA49,
    NET_SURF_CONFIG   =  0xAA4B,
   /*串口0*/
    NET_CFG_SERIAL_PORT0 = 0xAA09,
    /*串口1*/
    NET_CFG_SERIAL_PORT1  = 0xAA0A,  
   /*串口2*/
    NET_CFG_SERIAL_PORT2  = 0xAA10, 
 
    NET_MODBUS_CMD     =  0xAA4C,
    NET_MODBUSTCP_CMF  =  0xAA4D,
    
    NET_RESERVE_3     =  0xAA4E,  //MyStudio use
    NET_RESERVE_4     =  0xAA4F,  //MyStudio use

    NET_MQTT_LIST_CONFIG  = 0xAA60,

    NET_RESERVE6            = 0xAA61,
    NET_RESERVE7            = 0xAA62,
    NET_RESERVE8            = 0xAA63,
    NET_RESERVE9            = 0xAA64,

};
/*******************************************************************************
 * Variables
 ******************************************************************************/
plc_netcfg_st g_plc_netcfg = {.mqtt.reportingCycle=3};
modbus_cfg_st g_modbus_cfg;
modbus_tcp_cfg_st g_modbusTCP_cfg;

/*******************************************************************************
 * Code
 ******************************************************************************/

/** 这个在MiStudio上是隐藏的
4A AA 
2E 00 
4D 69 63 72 6F 4C 69 6E 6B 2E 63 6F 6D 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
87 EA
*/
static uint16_t parse_wei_hu_center(unsigned char * ptr)
{
    LOGV("netcfg", "Enter %s()", __func__);
    uint16_t length = GET_PU16_DATA(ptr + 2);
    ptr += 4;
    strncpy((char *)g_plc_netcfg.weihu.domain, (char*)ptr, sizeof(g_plc_netcfg.weihu.domain)-2);
    LOGD("netcfg", "g_plc_netcfg.weihu.domain = %s", g_plc_netcfg.weihu.domain);
    
    ptr += 40;
    g_plc_netcfg.weihu.port = GET_PU16_DATA(ptr);
    LOGD("netcfg", "g_plc_netcfg.weihu.port = %d", g_plc_netcfg.weihu.port);
    return length;
}

/*
40 AA 
3B 00 
01 
0A C7 C6 46 
85 EA 
0A C7 C6 C8 
85 EA 
4D 69 63 72 6F 4C 69 6E 6B 2E 63 6F D 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
85 EA 
*/
static uint16_t parse_cloud_center(unsigned char *ptr)
{
    LOGV("netcfg", "Enter %s()", __func__);
    uint16_t length = GET_PU16_DATA(ptr + 2);
    hexdump(ptr, length);
    
    ptr += 4;
    g_plc_netcfg.cloud.ipOrDomain = *ptr;
    LOGD("netcfg", "g_plc_netcfg.cloud.ipOrDomain = %d", g_plc_netcfg.cloud.ipOrDomain);

    ptr++;
    IP4_ADDR(&g_plc_netcfg.cloud.ip, *ptr, *(ptr+1), *(ptr+2), *(ptr+3));
    LOGD("netcfg", "g_plc_netcfg.cloud.ip = %s", ipaddr_ntoa(&g_plc_netcfg.cloud.ip));

    ptr += 4;
    g_plc_netcfg.cloud.port = GET_PU16_DATA(ptr);
    LOGD("netcfg", "g_plc_netcfg.cloud.port = %d", g_plc_netcfg.cloud.port);

    ptr += 2;
    IP4_ADDR(&g_plc_netcfg.cloud.ip2, *ptr, *(ptr+1), *(ptr+2), *(ptr+3));
    LOGD("netcfg", "g_plc_netcfg.cloud.ip2 = %s", ipaddr_ntoa(&g_plc_netcfg.cloud.ip2));

    ptr += 4;
    g_plc_netcfg.cloud.port2 = GET_PU16_DATA(ptr);
    LOGD("netcfg", "g_plc_netcfg.cloud.port2 = %d", g_plc_netcfg.cloud.port2);

    ptr += 2;
    strncpy((char *)g_plc_netcfg.cloud.domain, (char *)ptr, sizeof(g_plc_netcfg.cloud.domain));
    LOGD("netcfg", "g_plc_netcfg.cloud.domain = %s", g_plc_netcfg.cloud.domain);

    ptr += 40;
    g_plc_netcfg.cloud.portDomain = GET_PU16_DATA(ptr);
    LOGD("netcfg", "g_plc_netcfg.cloud.portCloud = %d", g_plc_netcfg.cloud.portDomain);

    ptr += 2;
    g_plc_netcfg.cloud.ifConnect = *ptr;
    LOGI("netcfg", "g_plc_netcfg.cloud.ifConnect = %d", g_plc_netcfg.cloud.ifConnect);
    return length;
}

/**
 41 AA 
 18 00 
 C0 A8 01 FE -> IP
 FF FF FF 00 -> mask
 C0 A8 0B 01 -> gateway
 C0 A8 01 01 -> DNS
 01 -> Whether use DHCP to get ip address
 00 -> MODBUSTCP Server，0:不启用，1：启用1个， 2：启用2个
 00 00 -> Reserved
*/
static uint16_t parse_wan(unsigned char *ptr)
{
    LOGV("netcfg", "Enter %s()", __func__);
    uint16_t length = GET_PU16_DATA(ptr + 2);

    ptr += 4;
    IP4_ADDR(&g_plc_netcfg.wan.ip, *ptr, *(ptr+1), *(ptr+2), *(ptr+3));
    ptr += 4;
    IP4_ADDR(&g_plc_netcfg.wan.mask, *ptr, *(ptr+1), *(ptr+2), *(ptr+3));
    ptr += 4;
    IP4_ADDR(&g_plc_netcfg.wan.gate, *ptr, *(ptr+1), *(ptr+2), *(ptr+3));
    ptr += 4;
    IP4_ADDR(&g_plc_netcfg.wan.dns, *ptr, *(ptr+1), *(ptr+2), *(ptr+3));
    ptr += 4;
    g_plc_netcfg.wan.ioExp = *ptr++;
    g_plc_netcfg.wan.mobbustcpServer = *ptr;
    g_plc_netcfg.wan.isParsed = 1;
    return length;
}

/**
 42 AA 
 18 00 
 C0 A8 01 FE -> IP
 FF FF FF 00 -> mask
 C0 A8 0B 01 -> gateway
 C0 A8 01 01 -> DNS
 01 -> If use MiLink
 00 -> MODBUSTCP Server，0:不启用，1：启用1个， 2：启用2个
 00 00 -> Reserved
*/
static uint16_t parse_lan(unsigned char *ptr)
{
    LOGV("netcfg", "Enter %s()", __func__);
    uint16_t length = GET_PU16_DATA(ptr + 2);
    ptr += 4;
    IP4_ADDR(&g_plc_netcfg.lan.ip, *ptr, *(ptr+1), *(ptr+2), *(ptr+3));
    ptr += 4;
    IP4_ADDR(&g_plc_netcfg.lan.mask, *ptr, *(ptr+1), *(ptr+2), *(ptr+3));
    ptr += 4;
    IP4_ADDR(&g_plc_netcfg.lan.gate, *ptr, *(ptr+1), *(ptr+2), *(ptr+3));
    ptr += 4;
    IP4_ADDR(&g_plc_netcfg.lan.dns, *ptr, *(ptr+1), *(ptr+2), *(ptr+3));
    ptr += 4;
    g_plc_netcfg.lan.ioExp = *ptr++;
    g_plc_netcfg.lan.mobbustcpServer = *ptr;

    g_plc_netcfg.lan.isParsed = 1;
    return length;
}
/**
4C AA 
24 00 -- 长度
00 00 00 00 00 00 00 00 00 00 00 00 -- 备用
01 00 -- 记录条数

00 00 -- 序号
09 -- 从站号
01 -- 执行模式（0表示循环，1表示触发）
03 -- 功能码（03 16 01 15）
01 -- 触发器条件种类（1表示M元件，0表示S元件）
68 00 -- 触发器条件地址
D0 07 -- 从站寄存器地址
01 00 -- 从站寄存器个数
CC 0B -- 主站缓冲区地址
A4 0F -- 执行结果
64 00 -- 延时(ms)
01 00 -- 源端口
*/
static uint16_t parse_modbus_com(unsigned char *ptr)
{
    LOGV("netcfg", "Enter %s()", __func__);
    uint16_t length = GET_PU16_DATA(ptr + 2);

    memset(&g_modbus_cfg, 0, sizeof(g_modbus_cfg));

    ptr += 16;//Point to 记录条数
    g_modbus_cfg.listNum = (*(ptr + 1) << 8) | (*ptr);
    LOGV("netcfg", "g_modbus_cfg.listNum = %u", g_modbus_cfg.listNum);
    if (g_modbus_cfg.listNum == 0)
    {
        return length;
    }
    g_modbus_cfg.listPtr = (modbus_item_st *)pvPortMalloc(sizeof(modbus_item_st) * g_modbus_cfg.listNum);
    memset(g_modbus_cfg.listPtr, 0, sizeof(modbus_item_st) * g_modbus_cfg.listNum);

    for (uint32_t i = 0; i < g_modbus_cfg.listNum; i++)
    {
        ptr += 2;//Point to 序号
    #if 0
        uint16_t snNum = (*(ptr + 1) << 8) | (*ptr);
        g_modbus_cfg.listPtr[snNum].sNum = snNum;
    #else
        uint16_t snNum = i;
        g_modbus_cfg.listPtr[snNum].sNum = snNum;
    #endif
        modbus_item_st *pItem = &g_modbus_cfg.listPtr[snNum];

        ptr += 2; // Point to 从站号
        pItem->slaveAddr = *ptr;
        LOGD("netcfg", "slaveAddr = %u", pItem->slaveAddr);

        ptr++; // Point to 执行模式（0表示循环，1表示触发）
        pItem->commType = *ptr;
        LOGD("netcfg", "commType = %u", pItem->commType);

        ptr++; // Point to 功能码（03 16 01 15）
        pItem->funcCode = *ptr;
        LOGD("netcfg", "funcCode = %u", pItem->funcCode);

        ptr++; // Point to 触发器条件种类（1表示M元件，0表示S元件）
        pItem->triggerType = *ptr;
        LOGD("netcfg", "triggerType = %u", pItem->triggerType);

        ptr++; // Point to 触发器条件地址
        pItem->triggerAddr = (*(ptr + 1) << 8) | (*ptr);
        LOGD("netcfg", "triggerAddr = %u", pItem->triggerAddr);

        ptr += 2; // Point to 从站寄存器地址
        pItem->slaveRegAddr = (*(ptr + 1) << 8) | (*ptr);
        LOGD("netcfg", "slaveRegAddr = %u(0x%X)", pItem->slaveRegAddr, pItem->slaveRegAddr);

        ptr += 2; // Point to 从站寄存器个数
        pItem->elementCnt = (*(ptr + 1) << 8) | (*ptr);
        LOGD("netcfg", "elementCnt = %u", pItem->elementCnt);

        ptr += 2; // Point to 主站缓冲区地址
        uint16_t elementAddr = (*(ptr + 1) << 8) | (*ptr);
        LOGV("netcfg", "elementAddr = %u", elementAddr);
        if (elementAddr & 0x8000)
        {
            pItem->masterBuf = &GET_R_ELEMENT_VALUE(elementAddr & 0x7FFF);
            pItem->masterBufBoundary = &GET_R_ELEMENT_VALUE(R_RANG - 1);
        }
        else
        {
            pItem->masterBuf = &GET_D_ELEMENT_VALUE(elementAddr);
            pItem->masterBufBoundary = &GET_D_ELEMENT_VALUE(D_RANG - 1);
        }

        ptr += 2; // Point to 执行结果
        elementAddr = (*(ptr + 1) << 8) | (*ptr);
        if (elementAddr & 0x8000)
        {
            pItem->execResult = &GET_R_ELEMENT_VALUE(elementAddr& 0x7FFF);
        }
        else
        {
            pItem->execResult = &GET_D_ELEMENT_VALUE(elementAddr);
        }

        ptr += 2; // Point to 延时（ms）
        pItem->delay = (*(ptr + 1) << 8) | (*ptr);
        if (pItem->delay == 0)
        {
            pItem->delay = 90;
        }
        LOGI("netcfg", "delay = %u", pItem->delay);

        ptr += 2; // Point to 端口号
        pItem->port = (*(ptr + 1) << 8) | (*ptr);
        LOGD("netcfg", "port = %u", pItem->port);
    }
    return length;
}

/**
 4D AA -- 配置码
 29 00 -- 长度
 01 00 00 00 00 00 00 00 00 00 00 00 -- 备用
 01 00 -- 记录条数

 00 00 -- 记录序号（clitent id）
 C0 A8 00 7E -- IP
 F6 01 -- 端口号
 01 -- 执行模式：1个字节（0表示循环，1表示触发）
 03 -- 功能码：1个字节（03 16 01 15）
 01 -- 触发器条件种类：1个字节（1表示M元件，0表示S元件）
 64 00 -- 触发器条件地址：2个字节
 D0 07 -- 从站寄存器地址
 01 00 -- 从站寄存器个数
 B8 0B -- 主站缓冲区地址
 A0 0F -- 执行结果
 01 00 -- 1=LAN , 2=WAN
 */
static uint16_t parse_modbus_tcp(unsigned char *ptr)
{
    LOGV("netcfg", "Enter %s()", __func__);
    uint16_t length = GET_PU16_DATA(ptr + 2);

    memset(&g_modbusTCP_cfg, 0, sizeof(g_modbusTCP_cfg));

    ptr += 16; //Point to 记录条数
    g_modbusTCP_cfg.listNum = (*(ptr + 1) << 8) | (*ptr);
    LOGV("netcfg", "g_modbusTCP_cfg.listNum = %u", g_modbusTCP_cfg.listNum);
    if (g_modbusTCP_cfg.listNum == 0)
    {
        return length;
    }

    g_modbusTCP_cfg.listPtr = (modbus_tcp_item_st *)pvPortMalloc(sizeof(modbus_tcp_item_st) * g_modbusTCP_cfg.listNum);
    memset(g_modbusTCP_cfg.listPtr, 0, sizeof(modbus_tcp_item_st) * g_modbusTCP_cfg.listNum);

    for (uint32_t i = 0; i < g_modbusTCP_cfg.listNum; i++)
    {
        ptr += 2; //Point to 记录序号（clitent id）

        uint16_t clientID = (*(ptr + 1) << 8) | (*ptr);
        LOGV("netcfg", "clientID = %u", clientID);
        g_modbusTCP_cfg.listPtr[i].client_id = clientID;

        modbus_tcp_item_st *pModbusItem = &g_modbusTCP_cfg.listPtr[i];
        
        ptr += 2; //Point to IP
        IP4_ADDR(&pModbusItem->slaveIP, *ptr, *(ptr+1), *(ptr+2), *(ptr+3));

        ptr += 4; //Point to 端口号
        pModbusItem->slavePort = (*(ptr + 1) << 8) | (*ptr);
        LOGD("netcfg", "Slave IP and Port : %s:%u", ipaddr_ntoa(&pModbusItem->slaveIP), pModbusItem->slavePort);
        
        ptr += 2; //Point to 执行模式：1个字节（0表示循环，1表示触发）
        pModbusItem->commType = *ptr;
        LOGD("netcfg", "commType = %u", pModbusItem->commType);

        ptr++; //Point to 功能码：1个字节（03 16 01 15）
        pModbusItem->funcCode = *ptr;
        LOGD("netcfg", "funcCode = %u", pModbusItem->funcCode);

        ptr++; //Point to 触发器条件种类：1个字节（1表示M元件，0表示S元件）
        pModbusItem->triggerType = *ptr;
        LOGD("netcfg", "triggerType = %u", pModbusItem->triggerType);

        ptr++; //Point to 触发器条件地址
        pModbusItem->triggerAddr = (*(ptr + 1) << 8) | (*ptr);
        LOGD("netcfg", "triggerAddr = %u", pModbusItem->triggerAddr);

        ptr += 2; //Point to 从站寄存器地址
        pModbusItem->slaveRegAddr = (*(ptr + 1) << 8) | (*ptr);
        LOGD("netcfg", "slaveRegAddr = %u(0x%X)", pModbusItem->slaveRegAddr, pModbusItem->slaveRegAddr);

        ptr += 2; //Point to 从站寄存器个数
        pModbusItem->elementCnt = (*(ptr + 1) << 8) | (*ptr);
        LOGV("netcfg", "elementCnt = %u", pModbusItem->elementCnt);

        ptr += 2; //Point to 主站缓冲区地址
        uint16_t elementAddr = (*(ptr + 1) << 8) | (*ptr);
        LOGV("netcfg", "elementAddr = %u", elementAddr);
        if (elementAddr & 0x8000)
        {
            pModbusItem->masterBuf = &GET_R_ELEMENT_VALUE(elementAddr& 0x7FFF);
            pModbusItem->masterBufBoundary = &GET_R_ELEMENT_VALUE(R_RANG - 1);			
        }
        else
        {
            pModbusItem->masterBuf = &GET_D_ELEMENT_VALUE(elementAddr);
            pModbusItem->masterBufBoundary = &GET_D_ELEMENT_VALUE(D_RANG - 1);
        }

        ptr += 2; //Point to 执行结果
        elementAddr = (*(ptr + 1) << 8) | (*ptr);
        if (elementAddr & 0x8000)
        {
            pModbusItem->execResult = &GET_R_ELEMENT_VALUE(elementAddr& 0x7FFF);
        }
        else
        {
            pModbusItem->execResult = &GET_D_ELEMENT_VALUE(elementAddr);
        }
        ptr += 2; //Point to: 1=LAN , 2=WAN
        pModbusItem->wanOrLan = (*(ptr + 1) << 8) | (*ptr);
        LOGV("netcfg", "wanOrLan = %u", pModbusItem->wanOrLan);
    }
    return length;
}

static void MQTT_config_log(mqtt_config_st *pMqtt)
{
    PRINTF("MQTT: vender -> %s\r\n", pMqtt->vender);
    PRINTF("MQTT: reportingCycle -> %u\r\n", pMqtt->reportingCycle);

    PRINTF("MQTT: ProductKey -> %s\r\n", pMqtt->ProductKey);
    PRINTF("MQTT: ProductSecret -> %s\r\n", pMqtt->ProductSecret);
    PRINTF("MQTT: DeviceName -> %s\r\n", pMqtt->DeviceName);
    PRINTF("MQTT: DeviceSecret -> %s\r\n", pMqtt->DeviceSecret);
    
    PRINTF("MQTT: host -> %s\r\n", pMqtt->host);
    PRINTF("MQTT: port -> %u\r\n", pMqtt->port);
    PRINTF("MQTT: client_id -> %s\r\n", pMqtt->client_id);
    PRINTF("MQTT: username -> %s\r\n", pMqtt->username);
    PRINTF("MQTT: password -> %s\r\n", pMqtt->password);
    PRINTF("MQTT: keepalive -> %u\r\n", pMqtt->keepalive);
    PRINTF("MQTT: lwt_topic -> %s\r\n", pMqtt->lwt_topic);
    PRINTF("MQTT: lwt_msg -> %s\r\n", pMqtt->lwt_msg);
    PRINTF("MQTT: lwt_qos -> %u\r\n", pMqtt->lwt_qos);
    PRINTF("MQTT: lwt_retain -> %u\r\n", pMqtt->lwt_retain);

    PRINTF("MQTT: publish_topic -> %s\r\n", pMqtt->publish_topic);
    PRINTF("MQTT: publish_topic_alarm -> %s\r\n", pMqtt->publish_topic_alarm);

#if SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_SDK
    LOGW("plc_netcfg", "configLength : %d", pMqtt->configLength);
    
    if (pMqtt->pConfigsHANYU != NULL)
    {
        mqtt_config_array_st *pCfg;
        for (int i = 0; i < pMqtt->configLength; i++)
        {
            LOGI("plc_netcfg", "pConfigsHANYU[%d].report_cycle = %u", i, pMqtt->pConfigsHANYU[i].report_cycle);
            LOGI("plc_netcfg", "pConfigsHANYU[%d].slave_id = %u", i, pMqtt->pConfigsHANYU[i].slave_id);	
            LOGI("plc_netcfg", "pConfigsHANYU[%d].slave_name = %s", i, pMqtt->pConfigsHANYU[i].slave_name);	
            LOGW("plc_netcfg", "pConfigsHANYU[%d].reportContentLen = %u", i, pMqtt->pConfigsHANYU[i].reportContentLen);
            pCfg = pMqtt->pConfigsHANYU[i].pReportContent;
            for (int j = 0; j < pMqtt->pConfigsHANYU[i].reportContentLen; j++)
            {
                LOGV("plc_netcfg", "pCfg[%d].name : %s", j, pCfg[j].name);
                LOGV("plc_netcfg", "pCfg[%d].element : %u", j, pCfg[j].element);
                LOGV("plc_netcfg", "pCfg[%d].dataType : %u", j, pCfg[j].dataType);
                LOGV("plc_netcfg", "pCfg[%d].address : %u", j, pCfg[j].address);
                LOGD("plc_netcfg", "pCfg[%d].sym : %u", j, pCfg[j].sym);
                LOGD("plc_netcfg", "pCfg[%d].alarmVal : %.3f", j, pCfg[j].alarmVal);
                LOGI("plc_netcfg", "pCfg[%d].alarmContent : %s", j, pCfg[j].alarmContent);
            }
        }
    }
    else if (pMqtt->pConfigs != NULL)
    {
        for (int i = 0; i < pMqtt->configLength; i++)
        {
            LOGV("plc_netcfg", "pConfigs[%d].name : %s", i, pMqtt->pConfigs[i].name);
            LOGV("plc_netcfg", "pConfigs[%d].element : %u", i, pMqtt->pConfigs[i].element);
            LOGV("plc_netcfg", "pConfigs[%d].dataType : %u", i, pMqtt->pConfigs[i].dataType);
            LOGV("plc_netcfg", "pConfigs[%d].address : %u", i, pMqtt->pConfigs[i].address);
            LOGD("plc_netcfg", "pConfigs[%d].sym : %u", i, pMqtt->pConfigs[i].sym);
            LOGD("plc_netcfg", "pConfigs[%d].alarmVal : %.3f", i, pMqtt->pConfigs[i].alarmVal);
            LOGI("plc_netcfg", "pConfigs[%d].alarmContent : %s", i, pMqtt->pConfigs[i].alarmContent);
        }
    }
#endif
}

/**
{
    "host":"broker.hivemq.com",
    "port":1883,
    "client_id":"kalyke001",
    "username":"",
    "password":"",
    "keep_alive":120,
    "will_topic":"",
    "will_msg":"",
    "will_qos":0,
    "will_retain":true,
    "publish_topic":"sys/teiobCPILLff/device/plctest/post",
    "subscribe_topic":"sys/teiobCPILLff/device/plctest/set",
    "response_topic":"sys/teiobCPILLff/device/plctest/setback",
    "config":[
        {
            "name":"temperature",
            "element":"D",
            "address":1000,
            "dataType":"float32"
        },
        {
            "name":"speed",
            "element":"D",
            "address":1002,
            "dataType":"int16"
        }
    ]
}
*/
void strReplace(char *str,char *oldstr,char *newstr)
{
     char strbuff[512];//转换缓冲区
     memset(strbuff,0,512);

     for(int i = 0;i < strlen(str);i++)
     {
         //查找目标字符串
         if(0 == strncmp(str+i,oldstr,strlen(oldstr)))
         {
            strcat(strbuff,newstr);
            i += strlen(oldstr) - 1;
         }
         else
         {
            strncat(strbuff,str + i,1);//保存一字节进缓冲区
         }
     }
     strcpy(str,strbuff);
}

#if (KALYKE_CJSON == 0)
static uint16_t parse_mqtt(unsigned char *ptr)
{
    int8_t ret;
    uint16_t length = GET_PU16_DATA(ptr + 2);
    LOGV("netcfg", "Enter %s(), length = 0x%X\r\n", __func__, length);
    ptr += 4; // Point to json
    char *pJson = pvPortMalloc(length + 32);
    memset(pJson, 0, length+32);
    memcpy(pJson, ptr, length-4);
    LOGV("netcfg", "pJson = %s\r\n", pJson);
    int tokenCount = 0;
    if (Kalyke_isJsonValidAndParse(pJson, &tokenCount) == false)
    {
        vPortFree(pJson);
        return 0;
    }
    LOGV("netcfg", "pJson = 0x%08X, tokenCount = %u\r\n", pJson, tokenCount);
    hexdump(&g_plc_netcfg.mqtt, sizeof(g_plc_netcfg.mqtt));
    ret = Kalyke_extractMqttConfig(pJson, tokenCount, &g_plc_netcfg.mqtt);
    if (ret != 0)
    {
        SET_SD_ELEMENT_VALUE(SD229, ret);
        g_plc_netcfg.mqtt.isParsed = 0;
        LOGE("plc_netcfg", "Leave %s()...%d", __func__, ret);
        vPortFree(pJson);
        return 0;
    }
    g_plc_netcfg.mqtt.isParsed = 1;
    LOGV("plc_netcfg", "Leave %s()...001", __func__);
    vPortFree(pJson);
    return length;
}
#else

static uint16_t parse_mqtt(unsigned char *ptr)
{
    char *pstr;
    int tokenCount = 0;
    int8_t ret;
    memset(&g_plc_netcfg.mqtt, 0, sizeof(g_plc_netcfg.mqtt));
    uint16_t length = GET_PU16_DATA(ptr + 2);
    LOGD("netcfg", "Enter %s(), length = %u", __func__, length);
    if (length == 4)
    {
        LOGE("plc_netcfg", "Mqtt Script is Empty!");
        return 0;
    }
    ptr += 4; // Point to json
    char *pJson = pvPortMalloc(length + 32);
    memset(pJson, 0, length + 32);
    memcpy(pJson, ptr, length - 4);
    LOGD("netcfg", "pJson = %s\r\n", pJson);

    Kalyke_cJSON_init();
    ret = Kalyke_extractMqttConfig(pJson, tokenCount, &g_plc_netcfg.mqtt);
    if (ret != 0)
    {
        g_plc_netcfg.mqtt.isParsed = 0;
        /*如果脚本不为空时，需要报错红灯闪烁*/
        if (ret != ERR_MQTT_JSON_NOT_OBJECT)
        {
            SET_SD_ELEMENT_VALUE(SD228, ret);
            guv_StopError.bit.netcfg_err = 1;
            plc_refresh_error_msg(ERR_MQTT_SCRIPT);
        }
        LOGE("plc_netcfg", "Leave %s()...%d", __func__, ret);
        vPortFree(pJson);
        return length;

    }

#if 0 
    pstr = strchr(g_plc_netcfg.mqtt.client_id, '%');
    if(pstr != NULL)
    {
        LOGW("netcfg", "client_id have %s", pstr);
        if(memcmp(pstr, "%DEVID", 6) == 0)
        {
            memcpy(pstr, gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId, 12);
            LOGD("netcfg", "client_id change %s", g_plc_netcfg.mqtt.client_id);
        }
    }
    pstr = strchr(g_plc_netcfg.mqtt.publish_topic, '%');
    if(pstr != NULL)
    {
        LOGW("netcfg", "publish_topic have %s", pstr);
        if(memcmp(pstr, "%DEVID", 6) == 0)
        {
            memcpy(pstr, gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId, 12);
            LOGD("netcfg", "publish_topic change %s", g_plc_netcfg.mqtt.publish_topic);
        }
    }
    pstr = strchr(g_plc_netcfg.mqtt.publish_topic_alarm, '%');
    if(pstr != NULL)
    {
        LOGW("netcfg", "publish_topic_alarm have %s", pstr);
        if(memcmp(pstr, "%DEVID", 6) == 0)
        {
            memcpy(pstr, gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId, 12);
            LOGD("netcfg", "publish_topic_alarm change %s", g_plc_netcfg.mqtt.publish_topic_alarm);
        }
    }
    pstr = strchr(g_plc_netcfg.mqtt.subscribe_topic, '%');
    if(pstr != NULL)
    {
        LOGW("netcfg", "subscribe_topic have %s", pstr);
        if(memcmp(pstr, "%DEVID", 6) == 0)
        {
            memcpy(pstr, gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId, 12);
            LOGD("netcfg", "subscribe_topic change %s", g_plc_netcfg.mqtt.subscribe_topic);
        }
    }
#else
    char strdeviceid[13] = {0};
    memcpy(strdeviceid, gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId, 12);
    LOGD("netcfg", "client_id change %s", strdeviceid);

    strReplace(g_plc_netcfg.mqtt.client_id, "%DEVID",strdeviceid);
    LOGD("netcfg", "client_id change %s", g_plc_netcfg.mqtt.client_id);	

    strReplace(g_plc_netcfg.mqtt.publish_topic, "%DEVID",strdeviceid);
    LOGD("netcfg", "publish_topic change %s", g_plc_netcfg.mqtt.publish_topic);

    strReplace(g_plc_netcfg.mqtt.publish_topic_alarm, "%DEVID",strdeviceid);
    LOGD("netcfg", "publish_topic_alarm change %s", g_plc_netcfg.mqtt.publish_topic_alarm);	

    strReplace(g_plc_netcfg.mqtt.subscribe_topic, "%DEVID",strdeviceid);
    LOGD("netcfg", "subscribe_topic change %s", g_plc_netcfg.mqtt.subscribe_topic);

    strReplace(g_plc_netcfg.mqtt.subscribe_topic_reboot, "%DEVID",strdeviceid);
    LOGD("netcfg", "subscribe_topic_reboot change %s", g_plc_netcfg.mqtt.subscribe_topic_reboot);

    strReplace(g_plc_netcfg.mqtt.subscribe_topic_pub_cycle, "%DEVID",strdeviceid);
    LOGD("netcfg", "subscribe_topic_pub_cycle change %s", g_plc_netcfg.mqtt.subscribe_topic_pub_cycle);

    strReplace(g_plc_netcfg.mqtt.subscribe_topic_pause, "%DEVID",strdeviceid);
    LOGD("netcfg", "subscribe_topic_pause change %s", g_plc_netcfg.mqtt.subscribe_topic_pause);	

//  strReplace(g_plc_netcfg.mqtt.subscribe_topic_pub_now, "%DEVID",strdeviceid);
//  LOGD("netcfg", "subscribe_topic_pub_now change %s", g_plc_netcfg.mqtt.subscribe_topic_pub_now);	

#endif

    g_plc_netcfg.mqtt.isParsed = 1;

    vPortFree(pJson);
    LOGD("plc_netcfg", "Leave %s()...001", __func__);
    return length;
}

#endif

static uint32_t parse_mqtt_list(unsigned char *ptr)
{
    uint32_t length;
    uint32_t i, j, temp;
    mqtt_config_array_st *pcfg;

    memset(&g_plc_netcfg.mqtt, 0, sizeof(g_plc_netcfg.mqtt));
    length = GET_SMLPU32_DATA(ptr + 2);
    LOGD("netcfg", "Enter %s(), length = %u", __func__, length);
    if (length == 4)
    {
        LOGE("plc_netcfg", "Mqtt list Script is Empty!");
        return 0;
    }
    //hexdump(ptr, length);
    ptr += 6; // Point to list

    strncpy(g_plc_netcfg.mqtt.vender, (char*)ptr, sizeof(g_plc_netcfg.mqtt.vender) - 1);
    LOGD("netcfg", "vender: %s", g_plc_netcfg.mqtt.vender);
    ptr += 32;

    strncpy(g_plc_netcfg.mqtt.host, (char*)ptr, sizeof(g_plc_netcfg.mqtt.host) - 1);
    LOGD("netcfg", "host: %s", g_plc_netcfg.mqtt.host);
    ptr += 64;

    g_plc_netcfg.mqtt.port = GET_SMLPU16_DATA(ptr);
    LOGD("netcfg", "port: %d", g_plc_netcfg.mqtt.port);
    ptr += 2;

    strncpy(g_plc_netcfg.mqtt.client_id, (char*)ptr, sizeof(g_plc_netcfg.mqtt.client_id) - 1);
    LOGD("netcfg", "client_id: %s", g_plc_netcfg.mqtt.client_id);
    ptr += 200;

    strncpy(g_plc_netcfg.mqtt.username, (char*)ptr, sizeof(g_plc_netcfg.mqtt.username) - 1);
    LOGD("netcfg", "username: %s", g_plc_netcfg.mqtt.username);
    ptr += 64;

    strncpy(g_plc_netcfg.mqtt.password, (char*)ptr, sizeof(g_plc_netcfg.mqtt.password) - 1);
    LOGD("netcfg", "password: %s", g_plc_netcfg.mqtt.password);
    ptr += 64;

    g_plc_netcfg.mqtt.keepalive = GET_SMLPU16_DATA(ptr);
    LOGD("netcfg", "keepalive: %d", g_plc_netcfg.mqtt.keepalive);
    ptr += 2;

    strncpy(g_plc_netcfg.mqtt.publish_topic, (char*)ptr, sizeof(g_plc_netcfg.mqtt.publish_topic) - 1);
    LOGD("netcfg", "publish_topic: %s", g_plc_netcfg.mqtt.publish_topic);
    ptr += 64;

    strncpy(g_plc_netcfg.mqtt.publish_topic_alarm, (char*)ptr, sizeof(g_plc_netcfg.mqtt.publish_topic_alarm) - 1);
    LOGD("netcfg", "publish_topic_alarm: %s", g_plc_netcfg.mqtt.publish_topic_alarm);
    ptr += 64;

    strncpy(g_plc_netcfg.mqtt.subscribe_topic, (char*)ptr, sizeof(g_plc_netcfg.mqtt.subscribe_topic) - 1);
    LOGD("netcfg", "subscribe_topic: %s", g_plc_netcfg.mqtt.subscribe_topic);
    ptr += 64;

    g_plc_netcfg.mqtt.configLength = GET_SMLPU16_DATA(ptr);
    LOGD("netcfg", "configLength: %d", g_plc_netcfg.mqtt.configLength);
    ptr += 2;

    temp = sizeof(mqtt_config_array_hanyu_st) * g_plc_netcfg.mqtt.configLength;
    g_plc_netcfg.mqtt.pConfigsHANYU = pvPortMalloc(temp);
    LOGW("netcfg", "After pCfgHY: %d(bytes), free heap: %d(bytes)", temp, xPortGetFreeHeapSize());
    memset(g_plc_netcfg.mqtt.pConfigsHANYU, 0, temp);
    mqtt_config_array_hanyu_st *pCfgHY = g_plc_netcfg.mqtt.pConfigsHANYU;
    for(i = 0; i < g_plc_netcfg.mqtt.configLength; i++)
    {
        pCfgHY[i].report_cycle = GET_SMLPU32_DATA(ptr);
        if (pCfgHY[i].report_cycle < 10)
        {
            pCfgHY[i].report_cycle = 10;
        }
        LOGI("netcfg", "[%d]report_cycle: %d", i, pCfgHY[i].report_cycle);
        ptr += 4;

        pCfgHY[i].slave_id = GET_SMLPU32_DATA(ptr);
        LOGI("netcfg", "[%d]slave_id: %d", i, pCfgHY[i].slave_id);
        ptr += 4;

        strncpy(pCfgHY[i].slave_name, (char*)ptr, sizeof(pCfgHY[i].slave_name) - 1);
        LOGI("netcfg", "[%d]slave_name: %s", i, pCfgHY[i].slave_name);
        ptr += 16;

        pCfgHY[i].reportContentLen = GET_SMLPU16_DATA(ptr);
        LOGI("netcfg", "[%d]reportContentLen: %d", i, pCfgHY[i].reportContentLen);
        ptr += 2;

        temp = sizeof(mqtt_config_array_st) * pCfgHY[i].reportContentLen;
        pCfgHY[i].pReportContent = pvPortMalloc(temp);
        LOGW("netcfg", "After pCfgHY[%d].reportContentLen: %d(bytes), free heap: %d(bytes)", i, temp, xPortGetFreeHeapSize());
        memset(pCfgHY[i].pReportContent, 0, temp);
        pcfg = pCfgHY[i].pReportContent;
        for(j = 0; j < pCfgHY[i].reportContentLen; j++)
        {
            strncpy(pcfg[j].name, (char*)ptr, sizeof(pcfg[j].name) - 1);
            //LOGD("netcfg", "[%d][%d].name: %s", i, j, pcfg[j].name);
            ptr += 16;

            pcfg[j].element = *ptr;
            //LOGD("netcfg", "[%d][%d].element: %d", i, j, pcfg[j].element);
            ptr += 1;

            pcfg[j].dataType = *ptr;
            //LOGD("netcfg", "[%d][%d].dataType: %d", i, j, pcfg[j].dataType);
            ptr += 1;

            pcfg[j].address = GET_SMLPU16_DATA(ptr);
            //LOGD("netcfg", "[%d][%d].address: %d", i, j, pcfg[j].address);
            ptr += 2;

            //null
            ptr += 1;

            pcfg[j].sym = *ptr;
            //LOGD("netcfg", "[%d][%d].sym: %d", i, j, pcfg[j].sym);
            ptr += 1;

            temp = GET_SMLPU32_DATA(ptr);
            pcfg[j].alarmVal = *(float*)&temp;
            //LOGD("netcfg", "[%d][%d].alarmVal: %f", i, j, pcfg[j].alarmVal);
            ptr += 4;

            strncpy(pcfg[j].alarmContent, (char*)ptr, sizeof(pcfg[j].alarmContent) - 1);
            //LOGD("netcfg", "[%d][%d].alarmContent: %s", i, j, pcfg[j].alarmContent);
            ptr += 16;
        }
    }

#if 0 //自己计算文件长度
    uint32_t calLen = 0;
    for(i = 0; i < g_plc_netcfg.mqtt.configLength; i++)
    {
        calLen += pCfgHY[i].reportContentLen;
    }
    calLen *= 42;
    calLen += 26 * g_plc_netcfg.mqtt.configLength;
    calLen += 628;
    LOGI("netcfg", "calLen = %d(0x%08X)", calLen, calLen);
    length = calLen;
#endif

    char strdeviceid[13] = {0};
    memcpy(strdeviceid, gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId, 12);
    LOGD("netcfg", "client_id change %s", strdeviceid);

    strReplace(g_plc_netcfg.mqtt.client_id, "%DEVID",strdeviceid);
    LOGD("netcfg", "client_id change %s", g_plc_netcfg.mqtt.client_id);	

    strReplace(g_plc_netcfg.mqtt.publish_topic, "%DEVID",strdeviceid);
    LOGD("netcfg", "publish_topic change %s", g_plc_netcfg.mqtt.publish_topic);

    strReplace(g_plc_netcfg.mqtt.publish_topic_alarm, "%DEVID",strdeviceid);
    LOGD("netcfg", "publish_topic_alarm change %s", g_plc_netcfg.mqtt.publish_topic_alarm);	

    strReplace(g_plc_netcfg.mqtt.subscribe_topic, "%DEVID",strdeviceid);
    LOGD("netcfg", "subscribe_topic change %s", g_plc_netcfg.mqtt.subscribe_topic);

    strReplace(g_plc_netcfg.mqtt.subscribe_topic_reboot, "%DEVID",strdeviceid);
    LOGD("netcfg", "subscribe_topic_reboot change %s", g_plc_netcfg.mqtt.subscribe_topic_reboot);

    strReplace(g_plc_netcfg.mqtt.subscribe_topic_pub_cycle, "%DEVID",strdeviceid);
    LOGD("netcfg", "subscribe_topic_pub_cycle change %s", g_plc_netcfg.mqtt.subscribe_topic_pub_cycle);

    strReplace(g_plc_netcfg.mqtt.subscribe_topic_pause, "%DEVID",strdeviceid);
    LOGD("netcfg", "subscribe_topic_pause change %s", g_plc_netcfg.mqtt.subscribe_topic_pause);	

//  strReplace(g_plc_netcfg.mqtt.subscribe_topic_pub_now, "%DEVID",strdeviceid);
//  LOGD("netcfg", "subscribe_topic_pub_now change %s", g_plc_netcfg.mqtt.subscribe_topic_pub_now);
    g_plc_netcfg.mqtt.isParsed = 1;

    LOGD("plc_netcfg", "Leave %s()...001", __func__);
    return length;
}

/** Wifi解析
 44 AA 
 4A 00 
 C0 A8 0B FE -> IP
 FF FF FF 00 -> mask
 C0 A8 0B 01 -> gate
 C0 A8 0B 01 -> DNS
 61 62 63 64 65 66 67 68 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ->ssid
 01 04 -> 是否加密与加密方式
 31 32 33 34 35 36 37 38 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ->password
 01 -> 是否DHCP
 00 62 00 -> 保留
*/
static uint16_t parse_wifi(unsigned char *ptr)
{
    LOGV("netcfg", "Enter %s()", __func__);
    uint16_t length = GET_PU16_DATA(ptr + 2);
    ptr += 4;
    IP4_ADDR(&g_plc_netcfg.wifi.ip, *ptr, *(ptr+1), *(ptr+2), *(ptr+3));
    ptr += 4;
    IP4_ADDR(&g_plc_netcfg.wifi.mask, *ptr, *(ptr+1), *(ptr+2), *(ptr+3));
    ptr += 4;
    IP4_ADDR(&g_plc_netcfg.wifi.gate, *ptr, *(ptr+1), *(ptr+2), *(ptr+3));
    ptr += 4;
    IP4_ADDR(&g_plc_netcfg.wifi.dns, *ptr, *(ptr+1), *(ptr+2), *(ptr+3));
    ptr += 4;
    strncpy((char *)g_plc_netcfg.wifi.ssid, (char *)ptr, sizeof(g_plc_netcfg.wifi.ssid));
    ptr += 24;
    g_plc_netcfg.wifi.isCrpy = *ptr;
    ptr += 1;
    g_plc_netcfg.wifi.crpyType = *ptr;
    ptr += 1;
    strncpy((char *)g_plc_netcfg.wifi.password, (char *)ptr, sizeof(g_plc_netcfg.wifi.password));
    ptr += 24;
    g_plc_netcfg.wifi.notUseDHCP = *ptr;
    return length;
}

static uint16_t parse_3G(unsigned char *ptr)
{
    LOGV("netcfg", "Enter %s()", __func__);
    uint16_t length = GET_PU16_DATA(ptr + 2);

    return length;
}

/** 透传解析
 49 AA 
 1D 00 
 00 -> 是否启用透传
 00 -> 以太云socket透传配置
 00 -> Wifi云socket透传配置
 00 -> 3G云socket透传配置
 00 -> 以太socket1透传配置
 00 -> 以太socket2透传配置
 00 -> 以太socket3透传配置
 00 00 00 00 00 00 -> 以太Sock1的IP与端口号
 00 00 00 00 00 00 -> 以太Sock2的IP与端口号
 00 -> 是否禁用
 00 -> 是否禁用
 00 00 -> 保留
 00 00 -> 以太Sock3（Server）端口号
 00 01 08 00 00 00 00 00 ---> ????
*/
static uint16_t parse_tt(unsigned char *ptr)
{
    LOGV("netcfg", "Enter %s()", __func__);
    uint16_t length = GET_PU16_DATA(ptr + 2);
    ptr += 4;
    g_plc_netcfg.touchuan.openTT = *ptr++;
    g_plc_netcfg.touchuan.yiTaiCloudSocketCfg = *ptr++;
    g_plc_netcfg.touchuan.wifiCloudSocketCfg = *ptr++;
    g_plc_netcfg.touchuan.threeGCloudSocketCfg = *ptr++;

    g_plc_netcfg.touchuan.yiTaiSocket1Cfg = *ptr++;
    g_plc_netcfg.touchuan.yiTaiSocket2Cfg = *ptr++;
    g_plc_netcfg.touchuan.yiTaiSocket3Cfg = *ptr++;

    IP4_ADDR(&g_plc_netcfg.touchuan.yiTaiSocket1IP, *ptr, *(ptr+1), *(ptr+2), *(ptr+3));
    ptr += 4;
    g_plc_netcfg.touchuan.yiTaiSocket1Port = GET_PU16_DATA(ptr);
    ptr += 2;
    IP4_ADDR(&g_plc_netcfg.touchuan.yiTaiSocket2IP, *ptr, *(ptr+1), *(ptr+2), *(ptr+3));
    ptr += 4;
    g_plc_netcfg.touchuan.yiTaiSocket2Port = GET_PU16_DATA(ptr);
    ptr += 2;
    g_plc_netcfg.touchuan.isForbidden_yiTaiCloudSocket = *ptr;
    g_plc_netcfg.touchuan.isForbidden_wifiCloudSocket = *ptr;
    g_plc_netcfg.touchuan.isForbidden_threeGCloudSocket = *ptr;
    ptr += 1;
    g_plc_netcfg.touchuan.isForbidden_yiTaiSocket1 = *ptr;
    g_plc_netcfg.touchuan.isForbidden_yiTaiSocket2 = *ptr;
    g_plc_netcfg.touchuan.isForbidden_yiTaiSocket3 = *ptr;
    ptr += 3;
    g_plc_netcfg.touchuan.yiTaiSocket3Port = GET_PU16_DATA(ptr);
    
    return length;
}

static uint16_t parse_surf(unsigned char *ptr)
{
    LOGV("netcfg", "Enter %s()", __func__);
    uint16_t length = GET_PU16_DATA(ptr + 2);
    hexdump(ptr, length);
    ptr += 4;

    g_plc_netcfg.surfing = *ptr;
    LOGD("netcfg", "g_plc_netcfg.surfing = %d", g_plc_netcfg.surfing);
    return length;
}

static void set_error(void)
{
    LOGV("netcfg", "Enter %s()", __func__);
    guv_StopError.bit.netcfg_err = 1;
    guv_NonStopError.bit.netconfig_err = 1;
    gtv_UserFilePtrSt.NetcfgBlockPtr = (unsigned char *)netcfg_default;
}

char plc_parse_netcfg_block_NoUart(void) // 可参考 plc_parse_system_block 函数
{
    LOGV("netcfg", "Enter %s(), guv_StopError.bit.netcfg_err = %d", __func__, guv_StopError.bit.netcfg_err);
    unsigned long llv_Len;
    unsigned char *lcp_NetcfgPtr;
    unsigned char *lcp_NetcfgEndPtr;
    unsigned short *lsp_TempPtr;
    //unsigned char *lcp_TempPtr;

    /*网络块错误，返回*/
    if(guv_StopError.bit.netcfg_err)
    {
        g_plc_netcfg.lan.ioExp = LAN_CONFIG_IO_EXP_ERR;
        return pdFAIL;
    }

    lcp_NetcfgPtr = gtv_UserFilePtrSt.NetcfgBlockPtr;

    /*读网络块长度*/
    llv_Len = plc_get_file_length(lcp_NetcfgPtr + FILE_LEN_INFO_START_INDEX, 4);
    LOGV("netcfg", "lcp_NetcfgPtr = 0x%x, llv_Len = %d", lcp_NetcfgPtr, llv_Len);
    //hexdump1(lcp_NetcfgPtr, llv_Len);
    /*取系统块结束位置*/
    lcp_NetcfgEndPtr = lcp_NetcfgPtr + llv_Len - 6;
    /*取第一条配置码位置*/
    lcp_NetcfgPtr += FILE_INFO_START_INDEX + 2 ;

    LOGV("netcfg", "lcp_NetcfgPtr = 0x%x, lcp_NetcfgEndPtr = 0x%x", lcp_NetcfgPtr, lcp_NetcfgEndPtr);
    while ( lcp_NetcfgPtr < lcp_NetcfgEndPtr)
    {
        lsp_TempPtr = (unsigned short *)lcp_NetcfgPtr;

        LOGV("netcfg", "*lsp_TempPtr = 0x%X", *lsp_TempPtr);
        switch(*lsp_TempPtr)
        {
        case NET_WEI_HU_CENTER:
            llv_Len = parse_wei_hu_center(lcp_NetcfgPtr);
            LOGV("netcfg", "llv_Len = %d", llv_Len);
            if(llv_Len != 46)
            {
                set_error();
                return pdFAIL;
            }
            lcp_NetcfgPtr += llv_Len;
            break;

        case NET_CLOUD_CENTER:
            llv_Len = parse_cloud_center(lcp_NetcfgPtr);
            LOGV("netcfg", "llv_Len = %d", llv_Len);
            if(llv_Len != 61)
            {
                set_error();
                return pdFAIL;
            }
            lcp_NetcfgPtr += llv_Len;
            break;

        case NET_WAN_CONFIG:
            llv_Len = parse_wan(lcp_NetcfgPtr);
            LOGV("netcfg", "llv_Len = %d", llv_Len);
            if(llv_Len != 24)
            {
                set_error();
                return pdFAIL;
            }
            lcp_NetcfgPtr += llv_Len;
            break;

        case NET_LAN_CONFIG:
            llv_Len = parse_lan(lcp_NetcfgPtr);
            LOGV("netcfg", "llv_Len = %d", llv_Len);
            if(llv_Len != 24)
            {
                //set_error();
                //return pdFAIL;
            }
            lcp_NetcfgPtr += 24;
            break;

        case NET_MQTT_CONFIG:
            llv_Len = parse_mqtt(lcp_NetcfgPtr);// 不定长
            LOGV("netcfg", "llv_Len = %d", llv_Len);
            MQTT_config_log(&g_plc_netcfg.mqtt);
            if (llv_Len == 0)
            {
                g_plc_netcfg.mqtt.reportingCycle = 3600;
                llv_Len = 4;
            }
            lcp_NetcfgPtr += llv_Len;
            break;

        case NET_MQTT_LIST_CONFIG:
            llv_Len = parse_mqtt_list(lcp_NetcfgPtr);// 不定长
            LOGV("netcfg", "llv_Len = %d", llv_Len);
            //MQTT_config_log(&g_plc_netcfg.mqtt);
            if (llv_Len == 0)
            {
                g_plc_netcfg.mqtt.reportingCycle = 3600;
                llv_Len = 6;
            }
            lcp_NetcfgPtr += llv_Len;
            break;

        case NET_WIFI_CONFIG:
            llv_Len = parse_wifi(lcp_NetcfgPtr);
            LOGV("netcfg", "llv_Len = %d", llv_Len);
            lcp_NetcfgPtr += llv_Len;
            break;

        case NET_3G_CONFIG:
            llv_Len = parse_3G(lcp_NetcfgPtr);
            LOGV("netcfg", "llv_Len = %d", llv_Len);
            lcp_NetcfgPtr += llv_Len;
            break;

        /*透传配置*/
        case NET_TT_CONFIG:
            llv_Len = parse_tt(lcp_NetcfgPtr);
            LOGV("netcfg", "llv_Len = %d", llv_Len);
            if(llv_Len != 29)
            {
                set_error();
                return pdFAIL;
            }
            lcp_NetcfgPtr += llv_Len;
            break;

        case NET_SURF_CONFIG:
            llv_Len = parse_surf(lcp_NetcfgPtr);
            lcp_NetcfgPtr += llv_Len;
            break;

         /*串口0*/
        case NET_CFG_SERIAL_PORT0:
        case NET_CFG_SERIAL_PORT1:
        case NET_CFG_SERIAL_PORT2:
            llv_Len = GET_PU16_DATA(lcp_NetcfgPtr + 2);
            lcp_NetcfgPtr += llv_Len;
            break;

#if (KALYKE_MODBUS_TCP_SHEET == 1)
        case NET_MODBUSTCP_CMF:
            llv_Len = parse_modbus_tcp(lcp_NetcfgPtr);
            LOGW("netcfg", "llv_Len = %d", llv_Len);
            lcp_NetcfgPtr += llv_Len;
            hexdump(lsp_TempPtr, llv_Len);
            break;
#else
        case NET_MODBUSTCP_CMF:
            llv_Len = GET_PU16_DATA(lcp_NetcfgPtr + 2);
            LOGW("netcfg", "llv_Len = %d", llv_Len);
            lcp_NetcfgPtr += llv_Len;
            hexdump(lsp_TempPtr, llv_Len);
            break;
#endif

#if (KALYKE_MODBUS_TCP_SHEET == 1)
        case NET_MODBUS_CMD:
            llv_Len = parse_modbus_com(lcp_NetcfgPtr);
            LOGW("netcfg", "llv_Len = %d", llv_Len);
            lcp_NetcfgPtr += llv_Len;
            hexdump(lsp_TempPtr, llv_Len);
            #if 0
            if (llv_Len != 18)
            {
                unsigned char lcv_Ret = plc_parse_sysblk_modlink_config(lsp_TempPtr);
                if(lcv_Ret != pdPASS)
                {
                    set_error();
                }
            }
            #endif
            break;
#else
        case NET_MODBUS_CMD:
            llv_Len = GET_PU16_DATA(lcp_NetcfgPtr + 2);
            LOGW("netcfg", "llv_Len = %d", llv_Len);
            lcp_NetcfgPtr += llv_Len;
            hexdump(lsp_TempPtr, llv_Len);
            break;
#endif

        case NET_RESERVE_3:
        case NET_RESERVE_4:
        case NET_RESERVE6:
        case NET_RESERVE7:
        case NET_RESERVE8:
        case NET_RESERVE9:
            llv_Len = GET_PU16_DATA(lcp_NetcfgPtr + 2);
            lcp_NetcfgPtr += llv_Len;
            break;
        
        case 0x100:
            return pdPASS;

        default:
            set_error();
            return pdFAIL;
        }
        LOGV("netcfg", "lcp_NetcfgPtr = 0x%x, lcp_NetcfgEndPtr = 0x%X\r\n", lcp_NetcfgPtr, lcp_NetcfgEndPtr);
    }

    return pdPASS;
}

char plc_parse_netcfg_block_OnlyUart(void)
{
    LOGV("netcfg", "Enter %s(), guv_StopError.bit.netcfg_err = %d", __func__, guv_StopError.bit.netcfg_err);
    unsigned long llv_Len;
    unsigned char *lcp_NetcfgPtr;
    unsigned char *lcp_NetcfgEndPtr;
    unsigned short *lsp_TempPtr;
    //unsigned char *lcp_TempPtr;

    /*网络块错误，返回*/
    if(guv_StopError.bit.netcfg_err)
    {
        g_plc_netcfg.lan.ioExp = LAN_CONFIG_IO_EXP_ERR;
        return pdFAIL;
    }

    lcp_NetcfgPtr = gtv_UserFilePtrSt.NetcfgBlockPtr;

    /*读网络块长度*/
    llv_Len = plc_get_file_length(lcp_NetcfgPtr + FILE_LEN_INFO_START_INDEX, 4);
    LOGV("netcfg", "lcp_NetcfgPtr = 0x%x, llv_Len = %d", lcp_NetcfgPtr, llv_Len);
    //hexdump1(lcp_NetcfgPtr, llv_Len);
    /*取系统块结束位置*/
    lcp_NetcfgEndPtr = lcp_NetcfgPtr + llv_Len - 6;
    /*取第一条配置码位置*/
    lcp_NetcfgPtr += FILE_INFO_START_INDEX + 2 ;

    LOGV("netcfg", "lcp_NetcfgPtr = 0x%x, lcp_NetcfgEndPtr = 0x%x", lcp_NetcfgPtr, lcp_NetcfgEndPtr);
    while ( lcp_NetcfgPtr < lcp_NetcfgEndPtr)
    {
        lsp_TempPtr = (unsigned short *)lcp_NetcfgPtr;

        LOGV("netcfg", "*lsp_TempPtr = 0x%X", *lsp_TempPtr);
        switch(*lsp_TempPtr)
        {
        case NET_WEI_HU_CENTER:
        case NET_CLOUD_CENTER:
        case NET_WAN_CONFIG:
        case NET_LAN_CONFIG:
        case NET_MQTT_CONFIG:
        case NET_WIFI_CONFIG:
        case NET_3G_CONFIG:
        case NET_TT_CONFIG:
        case NET_SURF_CONFIG:
        case NET_MODBUSTCP_CMF:
        case NET_MODBUS_CMD:
        case NET_RESERVE_3:
        case NET_RESERVE_4:
        case NET_RESERVE6:
        case NET_RESERVE7:
        case NET_RESERVE8:
        case NET_RESERVE9:
            llv_Len = GET_PU16_DATA(lcp_NetcfgPtr + 2);
            lcp_NetcfgPtr += llv_Len;
            break;

        case NET_MQTT_LIST_CONFIG:
            llv_Len = GET_SMLPU32_DATA(lcp_NetcfgPtr + 2);
            lcp_NetcfgPtr += llv_Len;
            break;

         /*串口0*/
        case NET_CFG_SERIAL_PORT0:
            llv_Len = GET_PU16_DATA(lcp_NetcfgPtr + 2);
            PRINTF("NET_CFG_SERIAL_PORT0 llv_Len = %d\r\n", llv_Len);
            if(llv_Len != 20)
            {
                set_error();
                return pdFAIL;
            }
            lcp_NetcfgPtr += llv_Len;
            plc_parse_sysblk_uart_config(0, lsp_TempPtr);
            break;

        /*串口1*/
        case NET_CFG_SERIAL_PORT1:
            llv_Len = GET_PU16_DATA(lcp_NetcfgPtr + 2);
            PRINTF("NET_CFG_SERIAL_PORT1 llv_Len PORT1 = %d\r\n", llv_Len);
            if(llv_Len != 20)
            {
                set_error();
                return pdFAIL;
            }
            lcp_NetcfgPtr += llv_Len;
            plc_parse_sysblk_uart_config(1, lsp_TempPtr);
            break;

        /*串口2*/
        case NET_CFG_SERIAL_PORT2:
            llv_Len = GET_PU16_DATA(lcp_NetcfgPtr + 2);
            PRINTF("NET_CFG_SERIAL_PORT2 llv_Len PORT1 = %d\r\n", llv_Len);
            if(llv_Len != 20)
            {
                set_error();
                return pdFAIL;
            }
            lcp_NetcfgPtr += llv_Len;
            plc_parse_sysblk_uart_config(2, lsp_TempPtr);
            break;
        
        case 0x100:
            return pdPASS;

        default:
            set_error();
            return pdFAIL;
        }
        LOGV("netcfg", "lcp_NetcfgPtr = 0x%x, lcp_NetcfgEndPtr = 0x%X\r\n", lcp_NetcfgPtr, lcp_NetcfgEndPtr);
    }

    return pdPASS;
}

