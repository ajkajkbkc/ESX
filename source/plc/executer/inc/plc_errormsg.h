/**
  ******************************************************************************
  * @file    plc_errormsg.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   PLC运行过程中错误信息定义
  ******************************************************************************
  */

#ifndef __PLC_ERROR_MSG_H
#define __PLC_ERROR_MSG_H

enum __PLC_EXEC_ERROR_MSG{
    /*EEPROM操作失败*/
    ERR_EEPROM_OPERATE      = 1,
    /*RAM测试失败*/
    ERR_RAM_TEST            = 10,
    ERR_FLASH_TEST,
    ERR_COM_TEST,
    ERR_RTC_TEST,
    ERR_I2C_TEST,
    ERR_PHY_TEST,   
    ERR_SER_IO              = 20,
    ERR_EXT_COM,
    ERR_EXT_SPEC,
    ERR_REAL_TIME,
    ERR_EEPROM_WR,
    /*电源失电*/
    ERR_LOSE_POWER          = 25,
    /*24V失电*/
    ERR_24V_POWER_LOSE,
    /*用户程序错误*/
    ERR_USER_PROGRAM        = 40,
    /*系统块错误*/
    ERR_SYSTEM_BLOCK,
    /*数据块错误*/
    ERR_DATA_BLOCK,
    /*数据保存错误*/
    ERR_SAVE_DATA,
    /*强制表错误*/
    ERR_FORCE_LIST_TABLE,
    /*用户信息文件错误*/
    ERR_USER_INFO,
    /*编译错误*/
    ERR_COMPILER            = 60,
    /*执行超时*/
    ERR_EXEC_OVER_TIME,
    /*非法指令*/
    ERR_ILLEGAL_INSTRCTION,
    /*非法元件类型*/
    ERR_ELEMENT_TYPE,
    /*非法操作数*/
    ERR_OPERANDS,
    /*元件超范围*/
    ERR_OVER_ELEMENT_RANG,
    /*子程序栈溢出*/
    ERR_OVER_SBR_STACK      = 66,
    /* */
    ERR_OVER_SERIAL_NUM,
    /*除0错误*/
    ERR_DIV_ZERO            = 69,
    /*栈定义错误*/
    ERR_STACK_DEFINE,
    /*未定义用户子程序或中断子程序*/
    ERR_NO_DEFINE_SBR_INTR  = 72,
    /*TRD/TWR读写RTC错误*/
    ERR_RTC_INSTRUCTION     = 76,
    /*网络参数配置错误*/
    ERR_NET_CONFIG          = 77,
    /*MODBUS表格执行错误*/
    ERR_MODBUS_TBL, 
    /*MODBUSTCP表格执行错误*/
    ERR_MODBUSTCP_TBL,

    /*MQTT脚本错误*/
    ERR_MQTT_SCRIPT         = 84,

    /*文件块为空*/
    ERR_NULL_UCODE          = 90,
    ERR_NULL_DATA_BLOCK,
    ERR_NULL_POU_INFO,
    ERR_NULL_SYSTEM_BLOCK,
    ERR_NULL_NET_CONFIG,
    ERR_NULL_GVT,

    /*无空闲内存*/
    ERR_NO_FREE_MEMORY      = 113,

    ERR_EXTEND_ERR         = 200,
    ERR_SLAVE_ERR          = 201,

    ERR_ECAT_SLAVE_NUM_ERR  = 204,
    ERR_ECAT_WKC_ERR        = 205,
    ERR_EXPANSION_MODULE    = 206,//华太从站的扩展模块突然断电
    ERR_ECAT_NOT_OPMODE     = 207,

    ERR_MDTCP_EXCEEDMAXCONN = 230,
};

#endif /*__PLC_ERROR_MSG_H*/

