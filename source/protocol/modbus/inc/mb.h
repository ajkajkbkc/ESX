/**
  ******************************************************************************
  * @file    mb.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   Modbus slave 主程序
  ******************************************************************************
  */
#ifndef __PROTOCOL_MB_H
#define __PROTOCOL_MB_H
/*Modbus从站接收到信息发送端定义*/
enum __MB_MSG_SENDER_E {
    MB_SENDER_COM0  = 0x00,
    MB_SENDER_COM1,
    MB_SENDER_COM2,
    MB_SENDER_COM3,
    MB_SENDER_USB,
    MB_SENDER_TCP,
    MB_SENDER_TCP_4G,
    MB_SENDER_MAX
};

/*Modbus 数据传输结构体定义*/
typedef struct __MD_SLAVE_MSG_PACK_ST {
    /*信息发送源*/
    unsigned char mcv_Sender;
    /*接收数据长度*/
    unsigned short msv_ReceiveLen;
    /*发送数据长度*/
    unsigned short msv_RespLen;
    /*是否为广播数据包*/
    unsigned char mcv_IsBroadcastInfo;
    /*错误码*/
    unsigned char mcv_ErrorCode;

    /*接收数据缓存*/
    unsigned char *mcp_ReceiveBuff;
    /*响应数据缓存指针*/
    unsigned char *mcp_RespBuff;

    unsigned char isTcpClient; // 1: This is TCP client.
    unsigned char clientID;
    /*MODBUS从站站号*/
    unsigned char slaveID;
    void (*tcp_resp_func)(unsigned char *, unsigned short, unsigned char);
    
    /*响应回掉函数指针*/
    void (*resp_func)(unsigned char *, unsigned short);
} md_slave_msg_pack;

/*modbus 诊断校验信息结构体*/
typedef struct __MB_DIAGNOSITIC_INFO_ST {
    /*站号*/
    unsigned char mcv_SlaveId;
    /*只听模式标志*/
    unsigned char mcv_ListenOnlyMode;
    /*总线报文计数*/
    unsigned short msv_BusPackageCnt;
    /*总线CRC错误计数*/
    unsigned short msv_BusCrcErrCnt;
    /*从站异常差错计数*/
    unsigned short msv_SlaveErrCnt;
    /*从站报文计数*/
    unsigned short msv_SlavePackageCnt;
    /*从站无响应计数*/
    unsigned short msv_SlaveNoRespCnt;
    /*总线字符超限计数*/
    unsigned short msv_BusCharOverrunCnt;
} mb_slave_diagnositic_info_st;

extern mb_slave_diagnositic_info_st gtp_ModbusSlaveDiagInfo[MB_SENDER_MAX];
/*------------------------------------------------------------------------------
*   MODBUS 功能码定义
*-----------------------------------------------------------------------------*/
/*读线圈状态*/
#define MB_READ_COILS_STATUS            0x01
/*读离散输入量状态*/
#define MB_READ_DESCRETE_INPUT_STATUS   0x02
/*读寄存器值*/
#define MB_READ_HOLDING_REGISTER        0x03
/*读多个输入寄存器*/
#define MB_READ_MULTIPLE_INPUT_REGISTER 0x04
/*写单线圈*/
#define MB_WRITE_SINGLE_COIL            0x05
/*写寄存器*/
#define MB_WRITE_REGISTER               0x06
/*回送诊断校验*/
#define MB_DIAG_DIAGNOSTIC              0x08
/*写多个线圈*/
#define MB_WRITE_MULTIPLE_COILS         0x0F
/*写多个寄存器*/
#define MB_WRITE_MULTIPLE_REGISTERS     0x10
/*批量读位元件*/
#define MB_READ_MULTIPLE_BIT_ELEMENT    0x64
/*批量读字元件*/
#define MB_READ_MULTIPLE_WORD_ELEMENT   0x65
/*批量写位元件*/
#define MB_WRITE_MULTIPLE_BIT_ELEMENT   0x66
/*批量写字元件*/
#define MB_WRITE_MULTIPLE_WORD_ELEMENT  0x67
/*下载*/
#define MB_DOWNLOWD_FUNC                0x68
/*上载*/
#define MB_UPLOAD_FUNC                  0x69
/*控制*/
#define MB_CTRL_FUNC                    0x6A

/*------------------------------------------------------------------------------
*   MODBUS 下载 子功能码定义
*-----------------------------------------------------------------------------*/
/*监控位元件*/
#define MB_DOWNLOAD_MONITOR_BITS          0x06
/*监控字元件*/
#define MB_DOWNLOAD_MONITOR_WORDS         0x07
/*监控位、字元件*/
#define MB_DOWNLOAD_MONITOR_BITS_WORDS    0x0A

/*下载UCODE*/
#define MB_DOWNLOAD_UCODE               0x5D
/*下载系统块*/
#define MB_DOWNLOAD_SYS_BLOCK           0x5E
/*下载数据块*/
#define MB_DOWNLOAD_DATA_BLOCK          0x5F
/*下载POU INFO*/
#define MB_DOWNLOAD_POU_INFO            0x60
/*下载全局变量表*/
#define MB_DOWNLOAD_GVT                 0x61
/*下载网络参数  */
#define MB_DOWNLOAD_NETCFG              0x66
/*下载PID参数1  */
#define MB_DOWNLOAD_PID1                0x6C
/*下载PID参数2  */
#define MB_DOWNLOAD_PID2                0x6D


/*系统升级*/
#define MB_DOWNLOAD_SYS_UPGRADE         0x68
/*从站升级*/
#define MB_DOWNLOAD_SLAVE               0x69

/*PLC应用程序CBIN文件升级*/
#define MB_DOWNLOAD_PLC_PROGRAME_CBIN   0x6A

/*------------------------------------------------------------------------------
*   MODBUS 上载 子功能码定义
*-----------------------------------------------------------------------------*/
/*监控位元件*/
#define MB_UPLOAD_MONITOR_BITS          0x06
/*监控字元件*/
#define MB_UPLOAD_MONITOR_WORDS         0x07
/*强制位元件*/
#define MB_UPLOAD_FORCE_BITS            0x08
/*强制字元件*/
#define MB_UPLOAD_FORCE_WORDS           0x09
/*监控位、字元件*/
#define MB_UPLOAD_MONITOR_BITS_WORDS    0x0A

/*从内存中生成数据块*/
#define MB_UPLOAD_GEN_DATA_BLOCK        0x46
/*上载UCODE*/
#define MB_UPLOAD_UCODE                 0x63
/*上载系统块*/
#define MB_UPLOAD_SYS_BLOCK             0x64
/*上载数据块*/
#define MB_UPLOAD_DATA_BLOCK            0x65
/*上载POU INFO*/
#define MB_UPLOAD_POU_INFO              0x66
/*上载全局变量表*/
#define MB_UPLOAD_GVT                   0x67
/*上载网络参数配置*/
#define MB_UPLOAD_NETCFG                0x69 
/*上载PID参数1配置*/
#define MB_UPLOAD_PID1                  0x6C
/*上载PID参数2配置*/
#define MB_UPLOAD_PID2                  0x6D

/*------------------------------------------------------------------------------
*   MODBUS CTRL 子功能码定义
*-----------------------------------------------------------------------------*/
/*读取时钟信息*/
#define MB_CTRL_READ_RTC                0x01
/*写入时钟信息*/
#define MB_CTRL_WRITE_RTC               0x02
/*运行PLC程序*/
#define MB_CTRL_RUN_CMD                 0x03
/*停止运行PLC*/
#define MB_CTRL_STOP_CMD                0x04
/*批量强制位元件*/
#define MB_CTRL_FORCE_BITS              0x05
/*批量字元件强制*/
#define MB_CTRL_FORCE_WORDS             0x06
/*取消位元件强制*/
#define MB_CTRL_UNFORCE_BITS            0x07
/*取消字元件强制*/
#define MB_CTRL_UNFORCE_WORDS           0x08
/*取消全部强制*/
#define MB_CTRL_UNFORCE_ALL             0x09
/*读取PLC信息*/
#define MB_CTRL_READ_PLC_INFO           0x10
/*读PLC状态*/
#define MB_CTRL_READ_PLC_STATUS         0x11
/*固化指令*/
#define MB_CTRL_PROGRAMME_CMD           0x12
/*清除用户程序*/
#define MB_CTRL_CLEAN_USER_CODE         0x13
/*清除系统块*/
#define MB_CTRL_CLEAN_SYS_BLOCK         0x14
/*清除数据块*/
#define MB_CTRL_CLEAN_DATA_BLOCK        0x15
/*格式化*/
#define MB_CTRL_CLEAN_ALL               0x16
/*UCODE校验*/
#define MB_CTRL_VERIFY_UCODE            0x17
/*复位*/
#define MB_CTRL_REBOOT                  0x18
/*清除错误信息*/
#define MB_CTRL_CLEAN_ERROR_INFO        0x19
/*清除元件值*/
#define MB_CTRL_CLEAN_ELEMENT           0x20
/*清除网络配置*/
#define MB_CTRL_CLEAN_NETCFG            0x21
/*PID参数1校验*/
#define MB_CTRL_VERIFY_PID1             0x23
/*禁止上载*/
#define MB_CTRL_SET_UPLOAD_FORBID       0x30
/*解除禁止上载*/
#define MB_CTRL_CLEAN_UPLOAD_FORBID     0x31
/* 菊花链扫描从站信息 */
#define MB_CTRL_SEARCH_SLAVE            0x38
/*读取错误信息*/
#define MB_CTRL_READ_ERROR_INFO         0x39
#define MB_CTRL_SEARCH_ETH_SLAVE        0x3A

/*设置上载密码*/
#define MB_CTRL_SET_UPLOAD_PWD          0x57
/*验证上载密码*/
#define MB_CTRL_CHECK_UPLOAD_PWD        0x58
/*设置下载密码*/
#define MB_CTRL_SET_DOWNLOAD_PWD        0x59
/*验证下载密码*/
#define MB_CTRL_CHECK_DOWNLOAD_PWD      0x5A
/*设置监控密码*/
#define MB_CTRL_SET_MONITOR_PWD         0x5B
/*验证监控密码*/
#define MB_CTRL_CHECK_MONITOR_PWD       0x5C
/*设置时钟密码*/
#define MB_CTRL_SET_TIMER_PWD           0x5D
/*验证时钟密码*/
#define MB_CTRL_CHECK_TIMER_PWD         0x5E

/*进入在线编程模式*/
#define MB_CTRL_ENTERY_ONLINE_PROGRAM   0x6E

#define MB_CTRL_TOU_CHUAN_COM0          0x70
#define MB_CTRL_TOU_CHUAN_COM1          0x71
#define MB_CTRL_TOU_CHUAN_COM2          0x72

#define MB_CTRL_TOU_CHUAN_WAN           0x77
#define MB_CTRL_TOU_CHUAN_LAN           0x78

#define MB_CTRL_TOU_CHUAN_CLOSE         0x7E


/*退出在线编程模式*/
#define MB_CTRL_EXIT_ONLINE_PROGRAM     0x83
/*写设备ID MAC地址等信息*/
#define MB_CTRL_WRITE_DEVICE_INFO       0x84
#define MB_CTRL_BLINK_NET_LED           0x88


#define MB_CTRL_SLAVE_ID                0x90

/*------------------------------------------------------------------------------
*   MODBUS 回送诊断校验功能码
*-----------------------------------------------------------------------------*/
/*请求帧返回*/
#define MB_DIAG_RETURN_QUERY_DATA       0x00
/*重启通讯选项*/
#define MB_DIAG_RESTART_COMMUNICATION   0x01
/*进入只听模式*/
#define MB_DIAG_FORCE_LISTEN_ONLY_MODE  0x04
/*清除计数器及诊断寄存器*/
#define MB_DIAG_CLEAR_CTRL_DIAG_REG     0x0A
/*返回总线报文计数*/
#define MB_DIAG_RETURN_BUS_PACKAGE_CNT  0x0B
/*返回总线CRC报文计数*/
#define MB_DIAG_RETURN_CRC_ERR_CNT      0x0C
/*返回从站异常差错计数*/
#define MB_DIAG_SLAVE_ERR_CNT           0x0D
/*返回从站报文计数*/
#define MB_DIAG_SLAVE_BUS_MSG_CNT       0x0E
/*返回从站无响应计数*/
#define MB_DIAG_SLAVE_NO_RESP_CNT       0x0F
/*返回总线字符超限计数*/
#define MB_DIAG_BUS_CHAR_OVERRUN_CNT    0x12
/*广播请求帧返回，用于波特率匹配*/
#define MB_DIAG_BROAD_RETURN_QUERY_DATA 0xAA

/*------------------------------------------------------------------------------
*   MODBUS 错误码定义
*-----------------------------------------------------------------------------*/
/*非法功能码*/
#define MB_ILIEGAL_CODE                 0x01
/*非法寄存器地址*/
#define MB_ILIEGAL_ADDR                 0x02
/*非法数据*/
#define MB_ILIEGAL_DATA                 0x03
/*非法从模式操作*/
#define MB_ERROR_SLAVE_OP               0x04
/*从站忙*/
#define MB_ERROR_SLAVE_BUSY             0x06
/*非法文件类型*/
#define MB_ILIEGAL_FILE_TYPE            0x07
/*密码校验失败*/
#define MB_PASSWORD_CHECK_FAIL          0x09
/*参数错误，设定主从模式错误*/
#define MB_SERVER_SLAVE_MODE_ERR        0x12
/*指令设置站号和本身站号相同错误*/
#define MB_SAME_SLAVE_ID_ERR            0x13
/*信息帧错误,包括信息长度错误,校验错误*/
#define MB_ERROR_FRAME                  0x18
/*无上载权限*/
#define MB_UPLOAD_FORBID                0x22

/*------------------------------------------------------------------------------
*   宏定义
*-----------------------------------------------------------------------------*/
/*单次最大读位元件数量*/
#define MB_MAX_R_BIT_NUM           0x7D0
/*单次最大写位元件数量*/
#define MB_MAX_W_BIT_NUM           0x7B0
/*单次最大读字元件数量*/
#define MB_MAX_R_WORD_NUM          0x7D
/*单次最大写字元件数量*/
#define MB_MAX_W_WORD_NUM          0x78

/*------------------------------------------------------------------------------
*   函数定义
*-----------------------------------------------------------------------------*/
void mb_slave_msg_handler(md_slave_msg_pack *pMsg);
void mb_slave_error_resp(md_slave_msg_pack *pMsg);
void mb_slave_verify_resp_msg(md_slave_msg_pack *pMsg);
void mb_slave_diag_clear_ctrl_diag_reg(void);
#endif /*__PROTOCOL_MB_H*/
