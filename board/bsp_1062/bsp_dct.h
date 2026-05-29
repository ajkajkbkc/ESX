/**
  ******************************************************************************
  * @file    bsp_dct.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   设备配置表相关函数
  ******************************************************************************
  */

#ifndef __BSP_DCT_H
#define __BSP_DCT_H
#include <stdbool.h>
#include "kalyke_opts.h"
#include "plc_element.h"

/*
FX：微物联工业手环
FW：微物联工业网关
FC：微物联控制器
FT: 微物联温度控制器
*/
#define FX_08R06AI08_C1    0x1001 // 8路输入 6路继电器输出 8路模拟量输入 2个串口2个网口
#define FX_08R06AI08_C2    0x1101 // 8路输入 6路继电器输出 8路模拟量输入 2个串口2个网口 4G模块
#define FX_16R14_C1        0x1002 // 16路输入 14路继电器输出2个串口2个网口 
#define FX_16R14_C2        0x1102 // 16路输入 14路继电器输出2个串口2个网口 4G模块
#define FX_08R06_C1        0x1000 // 8路输入 6路继电器输出2个串口2个网口
#define FX_08R06_C2        0x1100 // 8路输入 6路继电器输出2个串口2个网口 4G模块

#define ESX_C2             0x1201 // 电气安全网关：2个RS485串口、4G模块

#define FW_C1              0x2000 // 微物联工业网关: 1个RS232串口、2个RS485串口、2个网口
#define FW_C2              0x2100 // 微物联工业网关: 1个RS232串口、2个RS485串口、2个网口、4G模块
#define FW28E_C1           0x2200 // 微物联工业网关: 1个RS232串口、2个RS485串口、2个网口、Ehercat

#define CC_100             0x3000 
#define CC_120             0x3001 
#define CC_130             0x3002 


typedef struct __BSP_DEVICE_INFO_ST {
    /*固定为 0x44455649*/
    unsigned long mlv_Head;
    /*以太网口MAC地址*/
    unsigned char mcv_EthernetMac[6];
    /*WIFI MAC地址*/
    unsigned char mcv_WifiMac[6];
    /*云登陆设备ID*/
    unsigned char mcv_DeviceId[12];
    /*云登陆密码*/
    unsigned char mcv_DevicePasswd[12];
    /*模块类型*/
    unsigned short mlv_DeviceTypeId;
    /*CRC校验值*/
    unsigned long mlv_Crc32;
} bsp_device_info_st;

/*扩展串口定义*/
#define MAX_SUPPORT_UART_PORT   3

typedef struct __BSP_UART_PORT_INFO_ST {
    /*设备支持串口数量*/
    unsigned char mcv_SupportUartNum;
    struct {
        /*RS485 模式*/
        unsigned char mcv_RS485Mode;
        /*在设备上串口编号*/
        unsigned char mcv_OnDeviceNum;
    }mta_PortInfo[MAX_SUPPORT_UART_PORT];
} bsp_uart_port_info;

/*设备描述表结构体定义*/
typedef struct __BSP_DEVICE_CONFIG_TABLE_ST {
    /*设备信息*/
    bsp_device_info_st mtv_DevInfo;
    /*是否有电池,支持掉电保持*/
    unsigned char mcv_IsSupportBattery;
    /*是否支持SDRAM*/
    unsigned char isSupportSdram;
    unsigned long   mlv_SdramSize;
    /*PLC元件信息定义*/
    plc_element_info_st mtv_PlcElementInfo;
    /*EU/ED 数量*/
    unsigned short msv_EuEdNumber;
    /*本体是否有I/O口*/
    unsigned char mcv_IsHaveIOPort;
    /*本体输入口数量*/
    unsigned char mcv_SelfInputNum;
    /*本体输出口数量*/
    unsigned char mcv_SelfOutputNum;
    /*是否支持扩展板*/
    unsigned char mcv_IsSupportExtendBaord;
    /*串口定义*/
    bsp_uart_port_info mtv_UartPort;
    /*是否支持掉电保持*/
    unsigned char mcv_IsSupportPlsd;
} bsp_device_config_table_st;


extern bsp_device_config_table_st  gtv_DeviceConfigTable;

extern void bsp_get_device_config_table(void);
extern void bsp_write_device_info_to_flash(void);
extern bool bspIsHaveAD(void);
extern bool bspIsHave4G(void);
extern unsigned short bsp_get_deviceID(void);
extern bool bspIsGateway(void);
#endif /*__BSP_DCT_H*/

