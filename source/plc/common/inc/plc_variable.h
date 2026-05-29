/**
  ******************************************************************************
  * @file    plc_variable.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   PLC 系统运行过程中用到的各种变量申明,结构体定义,宏定义
  ******************************************************************************
  */
#ifndef __PLC_VARIABLE_H
#define __PLC_VARIABLE_H

#include "list_func.h"
#include "FreeRTOS.h"
#include "task.h"
#include "mb.h"
#include "plc_element.h"
#include "plc_commonfunc.h"
#include "bsp_dct.h"

/*------------------------------------------------------------------------------
* 默认用户程序,全局变量表,数据块,系统块定义
*-----------------------------------------------------------------------------*/
extern unsigned char ucode_default[];
extern const unsigned char sysblock_default[];
extern unsigned char datablock_default[];
extern unsigned char pouinfo_default[];
extern unsigned char gvt_default[];
extern const unsigned char netcfg_default[];

/*------------------------------------------------------------------------------
* 用户程序文件指针结构体定义
*-----------------------------------------------------------------------------*/
typedef struct __PLC_USER_FILE_PTR_ST {
    /*Ucode 文件指针*/
    unsigned char * UCodePtr;
    /*系统块文件指针*/
    unsigned char * SysBlockPtr;
    /*网络参数块文件指针*/
    unsigned char * NetcfgBlockPtr;
    /*数据块文件指针*/
    unsigned char * DataBlockPtr;
    /*POU info文件指针*/
    unsigned char * PouInfoPtr;
    /*全局变量表文件指针*/
    unsigned char * GvtPtr;
    /*云数据表指针*/
    unsigned char * CloudDataTablePtr;
    /*PID1文件指针*/
    unsigned char * PID1Ptr;
    /*PID2文件指针*/
    unsigned char * PID2Ptr;
} plc_user_file_ptr_st;

extern plc_user_file_ptr_st gtv_UserFilePtrSt;
/*------------------------------------------------------------------------------
* 运行错误标志定义 申明
*-----------------------------------------------------------------------------*/
/*停机错误标志*/
typedef union __PLC_STOP_ERROR_FLAG_U {
    unsigned short msv_Error;
    struct {
        /*SRAM错误*/
        unsigned short ram_err      :1;
        /*FALSH错误*/
        unsigned short flash_err    :1;
        /*通讯口错误*/
        unsigned short comm_err     :1;
        /*实时时钟错误*/
        unsigned short rtc_err      :1;
        /*用户程序错误*/
        unsigned short ucode_err    :1;
        /*系统块错误*/
        unsigned short sysblock_err :1;
        /*数据块错误*/
        unsigned short datablock_err:1;
        /*严重执行错误*/
        unsigned short exec_err     :1;
        /*本地IO错误*/
        unsigned short local_io_err :1;
        /*网络参数配置错误*/
        unsigned short netcfg_err   :1;
        /*保留位*/
        unsigned short reserved     :6;
    } bit;
} plc_stop_error_flag;

extern plc_stop_error_flag guv_StopError;

/*非停机错误标志定义*/
typedef union __PLC_NON_STOP_ERROR_FLASH_U {
    unsigned short msv_Error;
    struct {
        /*实时时钟错误*/
        unsigned short rtc_err      :1;
        /*元件保存错误*/
        unsigned short element_save_err :1;
        /*POU信息错误*/
        unsigned short pou_info_err :1;
        /*gvt表错误*/
        unsigned short gvt_err      :1;
        /*操作数错误*/
        unsigned short operands_err :1;
        
        /*扩展I/O错误*/
        unsigned short extend_io_num_err :1;
        /*扩展总线错误*/
        unsigned short extend_cfg_err    :1;
        /*扩展总线错误*/
        unsigned short extend_bus_err    :1;
        
        /*EEPROM 写错误*/
        unsigned short eeprom_wr_err    :1;
        /*netcfg 错误*/
        unsigned short netconfig_err    :1;
        /*用户程序错误*/
        unsigned short ucode_err        :1;
        /*数据块错误*/
        unsigned short datablock_err    :1;
        /*系统块错误*/
        unsigned short sysblock_err     :1;
        /* 系统块配置的从站数与实际不符或未进入op模式 */
        unsigned short ecat_err     :1;
        /*保留*/
        unsigned short reserved         :2;
    } bit;
} plc_non_stop_error_flag;

extern plc_non_stop_error_flag guv_NonStopError;

/*编译,运行错误记录数据结构,供KS软件定位具体错误地址*/
/*最多记录5条运行错误*/
#define MAX_EXEC_ERROR_RECORD_CNT   5

typedef struct __PLC_EXEC_ERROR_RECORD_ST {
    /*已记录错误次数*/
    unsigned char mcv_ErrorCnt;
    /**/
    unsigned short msv_ErrorMsg[MAX_EXEC_ERROR_RECORD_CNT];
    /*错误指令偏移地址*/
    unsigned long msv_ErrorAddr[MAX_EXEC_ERROR_RECORD_CNT];
} plc_exec_error_record_st;

extern plc_exec_error_record_st gtv_PlcExecErrorRecord;
/*------------------------------------------------------------------------------
* PLC运行状态及控制启停标志定义
*-----------------------------------------------------------------------------*/
/*PLC运行*/
#define PLC_RUN         1
/*PLC停止*/
#define PLC_STOP        0

/*PLC 运行状态定义*/
enum __PLC_RUN_STOP_STATUS {
    /*停止运行*/
    PLC_STOP_STATUS = 0x0,
    /*停止到运行*/
    PLC_STOP_TO_RUN_STATUS,
    /*运行*/
    PLC_RUN_STATUS,
    /*运行到停止*/
    PLC_RUN_TO_STOP_STATUS
};

/*PLC启停开关标志*/
typedef union __PLC_STATUS_SWITCH_U {
    unsigned char byte;
    struct {
        /*上电自运行*/
        unsigned char poweron_auto_run      :1;
        /*通信RUN命令*/
        unsigned char cmd_run               :1;
        /*输入点运行*/
        unsigned char input_point_run       :1;
        /*保留位*/
        unsigned char reserved_run          :1;

        /*运行错误停止*/
        unsigned char error_status_stop     :1;
        /*通信STOP命令*/
        unsigned char cmd_stop              :1;
        /*复位重启命令*/
        unsigned char cmd_reboot            :1;
        /*保留位*/
        unsigned char reserved_stop         :1;
    } bit;
} plc_status_switch_u;

/*PLC运行态结构体*/
typedef struct __PLC_RUN_STATUS_ST {
    /*plc运行,停止开关*/
    unsigned char mcv_PlcRunSwitch;
    /*运行,停止原因*/
    plc_status_switch_u mtv_PlcRunStopFlag;
    /*PLC当前状态,在__PLC_RUN_STOP_STATUS选择*/
    unsigned char mcv_PlcCurrentStatus;
    /*在线编程模式标志*/
    unsigned char mcv_IsOnlineProgram;
    /*系统扫描速率*/
    unsigned long mlv_SysScanTime;
    /*运行中看门狗计数器*/
    unsigned long mlv_WatchDogTime;
} plc_run_status_st;

//extern volatile plc_run_status_st gtv_PlcRunStatus;
extern plc_run_status_st gtv_PlcRunStatus;

/*plc运行态能留保存结构体定义*/
typedef struct __PLC_RUN_POWER_FLOW_INFO_ST {
    /*能留输入栈*/
    unsigned long mlv_InPF;
    /*能留输出栈*/
    unsigned long mlv_OutPF;
    /*用户程序PC*/
    unsigned char *mcp_PC;
} plc_run_power_flow_st;

#define GET_POWER_FLOW(pf)  (pf->mlv_InPF & 0x01L)
#define SET_POWER_FLOW(pf)  (pf->mlv_InPF |= 0x01L)
#define RST_POWER_FLOW(pf)  (pf->mlv_InPF &= 0xFFFFFFFEL)
#define PUSH_POWER_FLOW(pf) (pf->mlv_InPF <<= 1)
#define POP_POWER_FLOG(pf)  (pf->mlv_InPF >>= 1)
/*------------------------------------------------------------------------------
* 停止运行时输出点状态结构体定义
*-----------------------------------------------------------------------------*/
enum __PLC_STOP_OUTPUT_POINT_STATUS {
    /*PLC由RUN切换到STOP时,输出点保持最后状态不变*/
    OUTPUT_POINT_KEEP = 0x00,
    /*PLC由RUN切换到STOP时,输出点关闭输出*/
    OUTPUT_POINT_CLOSE,
    /*PLC由RUN切换到STOP时,输出点按配置组态输出*/
    OUTPUT_POINT_CONFIG
};

typedef struct __PLC_RUN_TO_STOP_OUTPUT_STATUS_ST {
    unsigned char mcv_Status;
    unsigned short msv_OutValue[8];
} plc_run_to_stop_output_status;

extern plc_run_to_stop_output_status gtv_PlcRunToStopOutputStatus;

/*------------------------------------------------------------------------------
* 系统块高级设置
*-----------------------------------------------------------------------------*/
typedef union __PLC_SYS_BLOCK_ADVANCED_SETTING {
    unsigned short msv_Setting;
    struct {
        /*无电池模式*/
        unsigned short no_battery_mode      :1;
        /*保留*/
        unsigned short reserved0            :1;
        /*元件值保持*/
        unsigned short keep_element_value   :1;
        /*数据块有效*/
        unsigned short data_block_valid     :1;
        /*禁止格式化*/
        unsigned short forbidden_format     :1;
        /*其余位保留*/
        unsigned short reserved1            :11;
    } bit;
} plc_sys_block_advanced_setting;

extern plc_sys_block_advanced_setting guv_PlcSysBlkAdSetting;


/*存储CALL指令调用信息结构体*/
typedef struct __PLC_CALL_INS_INFO_ST {
    /*子程序嵌套调用层数*/
    unsigned long msv_SbrNestedNum;
    /*CALL指令总计调用次数*/
    unsigned long  msv_UseNum;
    /*保存CALL指令UCODE开始地址*/
    unsigned long mlv_UCodeAddr[MAX_CALL_INS_USE_NUMBER];
    /*CALL指令是否执行标志*/
    unsigned char mcv_IsExec[MAX_CALL_INS_USE_NUMBER];
    /*嵌套调用返回时PC地址*/
    unsigned char *mcp_RetPc[MAX_SBR_NESTED_LAYER];
    /*子程序入口PC地址*/
    unsigned char *mcp_SbrPc[MAX_SBR_COUNT];
} plc_call_ins_info_st;

extern plc_call_ins_info_st *gtp_CallInsInfoPtr;

/*------------------------------------------------------------------------------
* STL/MC-MCR/子程序 在能留未导通时,需要清除特殊指令输出
* UCODE 编译时,针对以上指令代码,使用链表分别保存程序块中的简单指令及复杂指令UCODE
* PC值, PLC程序运行时, 程序快前面能留无效,直接从链表中取指令代码PC值,按照规则处理
* 相关指令即可.
* STL/子程序 使用单层双向链表
* MC-MCR 可以定义的标号区间为 0~7,将MC-MCR主控语句块串成一个队列.
*-----------------------------------------------------------------------------*/
/*指令链表结构体*/
typedef struct __PLC_INS_LIST_ST {
    /*指向对应指令UCODE地址*/
    void * mlp_InsPc;
    /*链表指针*/
    struct list_head mtv_InsList;
} plc_ins_list_st;

/*STL/子程序 指令链表HEAD 结构体定义*/
typedef struct __PLC_INS_LIST_HEAD_ST {
    /*简单指令列表头*/
    struct list_head mtv_SimpInsHead;
    /*复杂指令列表头*/
    struct list_head mtv_CompInsHead;
} plc_ins_list_head_st;

/*独立一个MC-MCR指令块链表HEAD 结构体定义*/
typedef struct __PLC_MC_MCR_INS_LIST_ST {
    /*MC-MCR指令块MC指令 UCODE地址*/
    unsigned char *mcp_McUCodePc;
    /*简单指令列表头*/
    struct list_head mtv_SimpInsHead;
    /*复杂指令列表头*/
    struct list_head mtv_CompInsHead;
    /*MC-MCR语句块队列指针*/
    struct list_head mtv_List;
} plc_mc_mcr_ins_list_st;

typedef struct __PLC_MC_MCR_INS_INFO_ST {
    /*MC-MCR主控指令嵌套深度*/
    unsigned char mcv_NestedLevel;
    /*指令列表HEAD*/
    struct list_head mtv_McMcrHead;
} plc_mc_mcr_ins_info_st;

/*子程序指令列表指针*/
extern plc_ins_list_head_st *gtp_SbrListPtr;
/*STL指令列表指针*/
extern plc_ins_list_head_st *gtp_StlListPtr;
/*MC-MCR 主控指令 列表指针*/
extern plc_mc_mcr_ins_info_st gtv_McMcrBlockInfo;

/*------------------------------------------------------------------------------
* 全局1ms定时器定义,与OS相关
*-----------------------------------------------------------------------------*/
#define GET_1MS_TICKS_COUNT()     xTaskGetTickCount()

/*------------------------------------------------------------------------------
* EU/ED 上升沿/下降沿 数据存储结构体
* 使用一个short型变量标识16个Edge,每一位对应一个特定的EU/ED
*-----------------------------------------------------------------------------*/
typedef struct __PLC_EU_ED_ST {
    /*标识特定位是否第一次运行*/
    unsigned short msv_IsInit;
    /*本此扫描周期输入能留值, 1:能留有效, 0:能留无效*/
    unsigned short msv_Value;
} plc_eu_ed_st;

extern plc_eu_ed_st *gtp_EuEd;
/*获取or清除特定沿初始化标志*/
#define GET_EDGE_INIT_FLAG(edge_num)    (gtp_EuEd[edge_num>>4].msv_IsInit & (0x01 << (edge_num & 0x0F)))
#define RST_EDGE_INIT_FLAG(edge_num)    (gtp_EuEd[edge_num>>4].msv_IsInit &= ~(0x01 << (edge_num & 0x0F)))
/*获取or清除特定沿输入能流值*/
#define GET_EDGE_VALUE(edge_num)    (gtp_EuEd[edge_num>>4].msv_Value & (0x01 << (edge_num & 0x0F)))
#define SET_EDGE_VALUE(edge_num)    (gtp_EuEd[edge_num>>4].msv_Value |= (0x01 << (edge_num & 0x0F)))
#define RST_EDGE_VALUE(edge_num)    (gtp_EuEd[edge_num>>4].msv_Value &= ~(0x01 << (edge_num & 0x0F)))

/*------------------------------------------------------------------------------
* FOR-NEXT 循环指令结构体定义
*-----------------------------------------------------------------------------*/
typedef struct __PLC_FOR_NEXT_INS_ST {
    /*循环次数*/
    unsigned short msa_LoopCnt[MAX_FOR_NEXT_NESTED_NUM];
    /*FOR-NEXT指令开始UCODE地址*/
    unsigned char *msp_StartUcodeAddr[MAX_FOR_NEXT_NESTED_NUM];
    /*FOR-NEXT指令结束UCODE地址*/
    unsigned char *msp_EndUcodeAddr[MAX_FOR_NEXT_NESTED_NUM];
    /*嵌套层数*/
    unsigned short msv_NestedNum;
} plc_for_next_ins_st;

extern plc_for_next_ins_st *gtp_ForNextIns;

/*------------------------------------------------------------------------------
* 步进控制状态结构体定义
*-----------------------------------------------------------------------------*/
typedef struct __PLC_SFC_RUN_STATUS_INFO_ST {
    unsigned char mcv_sfcEnable;
    /*STL 元件串联数量*/
    unsigned char mcv_SeriesSTLCnt;
    /*串联STL S编号记录*/
    unsigned short msv_StlNum[MAX_SFC_SERIES_STL_NUM];
} plc_sfc_run_status_info_st;

extern plc_sfc_run_status_info_st *gtp_SfcStatus;

/*------------------------------------------------------------------------------
* 时间表指令
*-----------------------------------------------------------------------------*/
typedef struct __PLC_HOUR_INS_ST {
    /*记录上次能流状态*/
    unsigned char mcv_LastPFStatus;

    /*记录上次能流有效开始时间*/
    unsigned long   mlv_LastTime;
} plc_hour_ins_st;

extern plc_hour_ins_st *gtp_HourInsSt;

/*------------------------------------------------------------------------------
* 定时脉冲指令DUTY数据结构
*-----------------------------------------------------------------------------*/
#define MAX_DUTY_SUPPORT_NUM        (5)
#define DUTY_INS_BASE_ELEMENT       (430)
typedef struct __PLC_DUTY_INS_INFO_ST {
    /*记录对应脉冲是否开启*/
    unsigned char mcv_IsEnable[MAX_DUTY_SUPPORT_NUM];
    /*脉冲ON周期*/
    unsigned short msv_OnTime[MAX_DUTY_SUPPORT_NUM];
    /*脉冲OFF周期*/
    unsigned short msv_OffTime[MAX_DUTY_SUPPORT_NUM];
} plc_duty_ins_info_st;

extern plc_duty_ins_info_st *gtp_DutyInsInfo;

/*------------------------------------------------------------------------------
* 监控元件链表结构定义
*-----------------------------------------------------------------------------*/

typedef struct __MB_MONITOR_ELEMENT_T {
    struct list_head list;
    /*元件类型*/
    unsigned char mcv_ElementType;
    /*转化后元件真实地址*/
    unsigned short msv_ElementAddr;
    /*Modbus协议地址*/
    unsigned short msv_ModbusAddr;
    /*元件值*/
    unsigned short msv_ElementValue;
} mb_monitor_element_t;

typedef struct __MB_MONITOR_ELEMENT_HEAD_T {
    /*链表头指针*/
    struct list_head head;
    /*链表长度*/
    unsigned short lsv_ListLen;
} mb_monitor_element_head_t;

extern mb_monitor_element_head_t gtv_MbMonitorBits;
extern mb_monitor_element_head_t gtv_MbMonitorWords;
extern mb_monitor_element_head_t gtv_MbMonitorBitsWords;
/*------------------------------------------------------------------------------
* 强制元件相关结构体定义
*-----------------------------------------------------------------------------*/
typedef mb_monitor_element_t mb_force_element_t;
typedef mb_monitor_element_head_t mb_force_element_head_t;

extern mb_force_element_head_t gtv_ForceBits;
extern mb_force_element_head_t gtv_ForceWords;

/*------------------------------------------------------------------------------
* 串口相关定义
*-----------------------------------------------------------------------------*/
/*端口模式定义*/
enum __UART_PORT_TYPE {
    /*自由口*/
    UART_TYPE_FREE_PORT = 0x00,
    /*Modbus 从站*/
    UART_TYPE_MB_SLAVE,
    /*Modbus 主站*/
    UART_TYPE_MB_MASTER,
    UART_TYPE_MAX
};

/*Modbus端口模式定义*/
enum __MB_UART_MODE {
    MB_RTU  = 0x00,
    MB_ASCII
};

/*UART 校验类型*/
enum __UART_PARITY_TYPE {
    UART_PARITY_NO  = 0x00,
    UART_PARITY_EVEN,
    UART_PARITY_ODD,
};

/*UART停止位定义*/
enum __UART_STOP_BITS {
    UART_STB_1  = 0x01,
    UART_STB_2,
};

/*串口SD标志位偏移量定义*/
enum __UART_PORT_SD_FLAG_E {
		/*保存系统块模式状态字*/
    UART_SD_MODE_CONFIG = 0x0,	
    /*本设备站号*/
    UART_SD_MODBUS_ID = 0x01,
    /*接收完成信息*/
    UART_SD_FINISH_RX_CHAR = 0x05,
    /*当前接收到字符*/
    UART_SD_CURRENT_RX_CHAR = 0x06,	
    /*当前接收长度*/
    UART_SD_CURRENT_RX_LEN = 0x07,	
	  /*当前发送字符*/
    UART_SD_CURRENT_TX_CHAR = 0x08,	
    /*主站错误代码*/
    UART_SD_MASTER_ERROR_CODE =0x09,	
};


#define SET_UART_SD_VALUE(port, index, value)      (gtv_PlcElement.msp_SDElement[gtp_UartPort[port].msv_SdFlagStartELement + index] = value)
#define GET_UART_SD_VALUE(port, index)             (gtv_PlcElement.msp_SDElement[gtp_UartPort[port].msv_SdFlagStartELement + index])

/*串口SM标志*/
enum __UART_PORT_SM_FLAG_E {
    /*自由口发送使能*/
    UART_SM_FREE_PORT_TX_EN = 0,
    /*自由口接收使能*/
    UART_SM_FREE_PORT_RX_EN,	
	  /*发送完成标志*/
    UART_SM_TX_FINISH,
    /*接收完成标志*/
    UART_SM_RX_FINISH,
    /*串口空闲标志*/
    UART_SM_IDLE ,
    /*MODBUS通讯完成*/
    UART_SM_MODBUS_FINISH,
    /*MODBUS通讯错误*/
    UART_SM_MODBUS_ERROR,
};

#define SET_UART_SM_FLAG(port, flag)    (plc_set_bit_element_value(SM_ELEMENT, (gtp_UartPort[port].msv_SmFlagStartELement + flag), 1))
#define RST_UART_SM_FLAG(port, flag)    (plc_set_bit_element_value(SM_ELEMENT, (gtp_UartPort[port].msv_SmFlagStartELement + flag), 0))
#define GET_UART_SM_FLAG(port, flag)    (plc_get_bit_element_value(SM_ELEMENT, (gtp_UartPort[port].msv_SmFlagStartELement + flag)))

/*uart端口参数*/
typedef struct __UART_PORT_PARA_ST {
    /*数据位*/
    unsigned char mcv_WordLength;
    /*停止位*/
    unsigned char mcv_StopBits;
    /*奇偶校验*/
    unsigned char mcv_Parity;
    /*波特率*/
    unsigned long mlv_BaudRate;
} uart_port_para_st;
/*自由口结构体定义*/
typedef struct __FREE_UART_PORT_ST {
    /*开始字符*/
    unsigned short msv_StartChar;
    /*结束字符*/
    unsigned short msv_EndChar;
    /*字符间超时时间*/
    unsigned short msv_WordTimeout;
    /*帧超时时间*/
    unsigned short msv_FrameTimeout;
    /*标志位*/
    unsigned short msv_Flag;
} free_uart_port_info_st;

/*Modbus 从站结构体定义*/
typedef struct __MODBUS_SLAVE_PORT_ST {
    /*传送模式：RTU/ASCII*/
    unsigned char mcv_TransMode;
    /*超时时间*/
    unsigned short msv_RxT35Time;
    /*发送超时时间*/
    unsigned short msv_TxTimeOut;
    /*从站信息处理包*/
    md_slave_msg_pack mtv_SlaveMsg;
} modbus_slave_port_info_st;

/*Modbus 主站结构体定义*/
typedef struct __MODBUS_MASTER_PORT_ST {
    /*传送模式：RTU/ASCII*/
    unsigned char mcv_TransMode;
    /*重试次数*/
    unsigned char mcv_RetryNum;
    /*接收超时时间*/
    unsigned short msv_RxT35Time;
    /*发送超时时间*/
    unsigned short msv_TxTimeOut;
} modbus_master_port_info_st;

/*串口信息联合体定义，用户串口模式三选一*/
typedef union __UART_PORT_MODE_INFO_ST {
    free_uart_port_info_st FreePort;
    modbus_slave_port_info_st SlavePort;
    modbus_master_port_info_st MasterPort;
} uart_port_mode_info_st;

/*串口结构体定义*/
typedef struct __UART_PORT_INFO_ST {
    /*串口模式：自由口, Modbus Slave, Modbus Msater*/
    unsigned char mcv_Mode;
    /*端口配置参数*/
    uart_port_para_st mtv_PortPara;
    /*对应模式具体信息*/
    uart_port_mode_info_st mtv_ModeInfo;

    /*SD标志元件起始地址*/
    unsigned short msv_SdFlagStartELement;
    /*SM标志元件起始地址*/
    unsigned short msv_SmFlagStartELement;

    /*发送缓冲区*/
    ring_buffer_st mtv_SendBuff;

    /*配置函数指针定义*/
    void (*pConfigFunc)(unsigned long llv_BaudRate, unsigned char lcv_Parity, unsigned char lcv_WordLength, unsigned char lcv_StopBits);
    /*字符发送函数*/
    void (*pSendFunc)(unsigned char *lcp_SendBuff, unsigned short lsv_Length);
    /*定时器初始化函数指针*/
    void (*pTimerInitFunc)(unsigned short lsv_RxPeriod, unsigned short lsv_TxPeriod);
} uart_port_info_st;

extern uart_port_info_st  gtp_UartPort[MAX_SUPPORT_UART_PORT];

/*自由口配置标志位定义*/
#define FREE_COM_START_CHAR_EN    0x01
#define FREE_COM_END_CHAR_EN      0x02
#define FREE_COM_WORD_TIMEOUT     0x04
#define FREE_COM_FRAME_TIMEOUT    0x08

/*自由口接收完成状态字定义*/
/*用户终止接收*/
#define FREE_COM_FINISH_RX_USER_STOP    0x01
/*收到结束字符*/
#define FREE_COM_FINISH_RX_RCV_END_CHAR 0x02
/*收到最大字符数*/
#define FREE_COM_FINISH_RX_MAX_RCV_CNT  0x04
/*字符间超时*/
#define FREE_COM_FINISH_RX_WORD_TIMEOUT 0x08
/*帧超时*/
#define FREE_COM_FINISH_RX_FRAME_TIMEOUT    0x10
/*奇偶校验错误*/
#define FREE_COM_FINISH_RX_PARITY_ERR       0x20

/*------------------------------------------------------------------------------
* Modlink指令相关定义
*-----------------------------------------------------------------------------*/
#define MAX_MODLINK_SHEET_NUM       32
#define MAX_MODLINK_SHEET_NAME_LEN  8

/*Modlink便携通讯指令表列表项定义*/
typedef struct __PLC_MODLINK_INS_ITEM_ST {
    /*停止标志位地址(M元件)*/
    unsigned short msv_StopFlagIndex;
    /*从站地址*/
    unsigned char mcv_SlaveAddr;
    /*modbus功能码*/
    unsigned char mcv_MbFunc;
    /*起始地址*/
    unsigned short msv_StartAddr;
    /*读写元件个数*/
    unsigned short msv_ElementCnt;
    /*Modbus指令读、执行结果写地址*/
    unsigned short *msp_ResultData;
    /*读写buffer边界*/
    unsigned short *msp_MaxResultData;
    /*modbus指令执行结果,保存在D元件中,此参数为对应D元件下标
     * 0：未执行     1：未执行参数错误 2：正在执行中 3：执行过程出错 4：执行成功
    */
    unsigned short *msp_ExecResult;
    /*运行标志*/
    unsigned char mcv_IsExec;
    /*超时时间*/
    unsigned long mlv_TimeOut;
} plc_modlink_ins_item_st;

/*Modlink便携通讯指令表表头定义*/
typedef struct __PLC_MODLINK_HEAD_ST {
    /*表名称*/
    unsigned char mcv_TableName[MAX_MODLINK_SHEET_NAME_LEN];
    /*表类型*/
    unsigned char mcv_TableType;
    /*指令列表项数量*/
    unsigned char mcv_InsListNum;
    /*指令列表数组指针*/
    plc_modlink_ins_item_st *mtp_InsListPtr;
    /*当前执行指令列表下标*/
    unsigned short msv_ListIndex;
    /*缓存区，暂存从站返回数据*/
    unsigned short *msp_RecvBuff;
} plc_modlink_head_st;


/*便携通讯指令表类型定义*/
enum __PLC_MODLINK_INS_TYPE_E {
    MDL_TYPE_NOT_USE = 0x00,
    MDL_TYPE_MODLINK,
    MDL_TYPE_MBCLINK,
    MDL_TYPE_S7CLINK,
};

/*便携通讯指令执行结果定义*/
enum __PLC_MODLINK_RESULT_E {
    /*指令未执行*/
    MODLINK_NOT_RUN = 0x00,
    /*运行成功*/
    MODLINK_PASS,  
    /*参数错误，未执行*/
    MODLINK_PARA_ERR,
    /*运行中*/
    MODLINK_RUNNING,
     /*运行错误*/
    MODLINK_RUN_ERR,
    /*串口配置错误*/
    MODLINK_COMCFG_ERR,
    /*串口号太大*/
    MODLINK_COM_ERR,
    /*SM标志位错误*/
    MODLINK_COM_IDLE_ERR,
    /*超时*/
    MODLINK_RUN_TIMEOUT = 0xff,    
};

extern plc_modlink_head_st gtv_ModlinkSheetHead[MAX_MODLINK_SHEET_NUM];

/*------------------------------------------------------------------------------
* 掉电保存相关结构体定义
*-----------------------------------------------------------------------------*/
/*掉电保持元件顺序*/
enum __PLC_PLSD_SEQ_E {
    PLSD_M  = 0x0,
    PLSD_S,
    PLSD_C,
    PLSD_T,
    PLSD_D,
    PLSD_MAX,
};

/*掉电保持数据组定义*/
#define PLC_PLSD_GROUP_NUM   PLSD_MAX

/*掉电保持数据项定义*/
typedef struct __PLC_PLSD_ITEM_ST {
    /*保存元件起始位置*/
    unsigned short msv_StartElement;
    /*保存元件数目*/
    unsigned short msv_Length;
} plc_plsd_item_st;

/** 掉电保持数据段定义
 * 每个group保存的元件按顺序为：M、S、C、T、D
 * 目前有2个group
*/
typedef struct __PLC_POWER_LOSE_SAVE_DATA_SEG_ST
{
    unsigned long flashHead; // Should be 0x44455649
    unsigned long lowPowerReset; // 如果这次系统复位之前，发生了低电则为1
    plc_plsd_item_st group1[PLC_PLSD_GROUP_NUM];
    plc_plsd_item_st group2[PLC_PLSD_GROUP_NUM];
    unsigned long dummy1;
    unsigned long dummy2;
    unsigned long dummy3;
    unsigned long dummy4;
} plc_plsd_st;

extern plc_plsd_st gtp_PowerLoseSaveDataInfo;

#endif /*__PLC_VARIABLE_H*/

