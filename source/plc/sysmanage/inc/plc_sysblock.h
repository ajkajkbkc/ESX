/**
  ******************************************************************************
  * @file    plc_sysblock.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   PLC系统块解析
  ******************************************************************************
  */

#ifndef __PLC_SYS_BLOCK_H
#define __PLC_SYS_BLOCK_H
#include "lwip/ip_addr.h"

/*------------------------------------------------------------------------------
* 系统块配置码定义
*-----------------------------------------------------------------------------*/
enum __PLC_SYS_BLOCK_CONFIG_CODE_E
{
    /*掉电保存的元件配置*/
    SYS_BLK_CFG_BATTERY_SAVE        = 0xAA01,
    /*系统STOP状态时的输出状态*/
    SYS_BLK_CFG_OUTPUT_POINT        = 0xAA02,
    /*看门狗定时*/
    SYS_BLK_CFG_WATCH_DOG_TIME      = 0xAA03,
    /*恒定扫描时间*/
    SYS_BLK_CFG_SCAN_TIME           = 0xAA04,
    /*失电检测时间*/
    SYS_BLK_CFG_LOSE_POWER_CHECK_TIME = 0xAA05,
    /*数字滤波常数*/
    SYS_BLK_CFG_DIGIT_FILTER_TIME   = 0xAA06,
    SYS_BLK_CFG_DIGIT_FILTER_TIME1  = 0xAA16,
    /*高级设置*/
    SYS_BLK_CFG_ADVANCED_SETTING    = 0xAA07,
    /*设置系统运行输入点*/
    SYS_BLK_CFG_RUN_INPUT_POINT     = 0xAA08,
    /*串口0*/
    SYS_BLK_CFG_SERIAL_PORT0        = 0xAA09,
    /*串口1*/
    SYS_BLK_CFG_SERIAL_PORT1        = 0xAA0A,

    SYS_AddrModal                  = 0xAA0B,
    SYS_BFMDefine			        = 0xAA0C,
    /*中断优先级*/
    SYS_BLK_CFG_INT_PRIORITY        = 0xAA0D,

    SYS_CommModals		            = 0xAA0E,//通讯模块配置
    /*CAN0 自由口协议配置*/
    SYS_BLK_CFG_CAN0_FREE_PROTOCOL  = 0xAA0F,
    /*串口2*/
    SYS_BLK_CFG_SERIAL_PORT2        = 0xAA10,


    /*CAN1 自由口协议配置*/
    SYS_BLK_CFG_CAN1_FREE_PROTOCOL  = 0xAA11,
    /*CAN2 自由口协议配置*/
    SYS_BLK_CFG_CAN2_FREE_PROTOCOL  = 0xAA12,
    /*CAN3 自由口协议配置*/
    SYS_BLK_CFG_CAN3_FREE_PROTOCOL  = 0xAA13,

    SYS_MDIConfig                   = 0xAA18,  //以前是0F，重复了
    SYS_InnerADDA                   = 0xAA19,
    SYS_InnerPT                     = 0xAA1A,
    SYS_InnerTC                     = 0xAA1B,
    SYS_InnerIO                     = 0xAA1C,
    
    /*MODLINK*/
    SYS_BLK_CFG_MODLINK             = 0xAA20,
    /*轴参数*/
    SYS_BLK_CFG_AXIS_DATA           = 0xAA21,


    SYS_Center                      = 0xAA22,
    SYS_Ehernet                     = 0xAA23,
    SYS_WIFI                        = 0xAA24,
    SYS_3G                          = 0xAA25,

    //两个配置项目
    SYS_DevicdID                    = 0xAA26,
    SYS_Password                    = 0xAA27,
    SYS_MacAddr                     = 0xAA28,
    SYS_TransParent                 = 0xAA29,

    /*CAN0口 CANOPEN协议配置*/
    SYS_BLK_CFG_CAN0_CANOPEN        = 0xAA30,
    /*CAN1口 CANOPEN协议配置*/
    SYS_BLK_CFG_CAN1_CANOPEN        = 0xAA31,
    /*CAN2口 CANOPEN协议配置*/
    SYS_BLK_CFG_CAN2_CANOPEN        = 0xAA32,
    /*CAN3口 CANOPEN协议配置*/
    SYS_BLK_CFG_CAN3_CANOPEN        = 0xAA33,

    /*分站配置，Mistudio反编译用*/
    SYS_SUBSTATION                  = 0xAA50,

    /*总线配置*/
    SYS_BUS_CONFIG                  = 0xAA51,

    /*FEXLINK循环周期*/
    SYS_BLK_CFG_FEXLINK_CYCLE_TIME   = 0xAA52,
    
    /*Ethercat从站配置，Mistudio反编译用*/ 
    SYS_EtherCATArrays               = 0xAA53,

    /*ethetcat总线配置*/
    SYS_ETHERCAT_CONFIG             = 0xAA54,

    SYS_MODBUS_TCP_CLIENT           = 0xAA55,
    SYS_ETHERCAT_MASTER             = 0xAA56,
    SYS_CAMSetting                  = 0xAA57,
    SYS_PLC_ROLE                    = 0xAA58,
    SYS_RESERVE7                    = 0xAA59,
    SYS_RESERVE8                    = 0xAA5A,
    SYS_RESERVE9                    = 0xAA5B

};

/*串口系统块配置字解析结构体*/
typedef struct __UART_SYSBLK_CONFIG_ST
{
    /*端口波特率配置
    000 = 38400, 001 = 19200, 010 = 9600, 011 = 4800, 100 = 2400, 101 = 1200, 110 = 57600; 111 = 115200
    */
    unsigned short BaudRate     : 3;
    /*停止位 0：1位, 1: 2位*/
    unsigned short StopBits     : 1;
    /*校验位 0: 偶校验, 1: 奇校验*/
    unsigned short Parity       : 1;
    /*校验使能     0：不校验, 1: 校验*/
    unsigned short ParityEnable : 1;
    /*数据长度      0: 8, 1: 7*/
    unsigned short WordLength   : 1;

    /*自由口起始字节允许*/
    unsigned short StartWordEn  : 1;
    /*自由口结束字节允许*/
    unsigned short EndWordEn    : 1;
    /*自由口字符间超时时间使能*/
    unsigned short CharTimeOutEn    : 1;
    /*自由口帧间超时时间使能*/
    unsigned short FrameTimeOutEn   : 1;

    /*Modbus传输模式 0：RTU, 1: ASCII*/
    unsigned short ModbusTransMode  : 1;
    /*Modbus主从模式 0：从站, 1：主站*/
    unsigned short ModebusMode      : 1;

    /*协议类型
    001: 自由口 010：Modbus 011：KCBus
    */
    unsigned short ProtocolType     : 3;
} uart_sysblk_config_st;


#define sysDaCfgNonUse              (0) // DA 不使用
#define sysDaCfgZ00_Z20ma           (1) // DA 输出0~20ma
#define sysDaCfgZ04_Z20ma           (2) // DA 输出4~20ma
#define sysDaCfgZ00_Z24ma           (3) // DA 输出0~24ma
#define sysDaCfgZ05v                (4) // DA 输出正5V
#define sysDaCfgZ10v                (5) // DA 输出正10V
#define sysDaCfgZf10v               (6) // DA 输出正负10V

#define DA_FsmNonInit           (0) // DA FSM
#define DA_FsmNonUse            (1)
#define DA_FsmIsInit            (2)
#define DA_FsmStrart            (3)
#define DA_FsmRun               (4)
#define DA_FsmStop              (5)
struct tag_InDaState                // 8Byte
{
    unsigned short modSlc;                  // 模式选择,(0：通道关闭 1：0~20mA,          2：4~20mA   3：0~24mA  4： 0~5V  5：0~10V 6：-10~10V)
    unsigned short pOutDtaU16;              // 输出值地址(D元件或者R元件，见错误状态地址)
    unsigned short pzeroDtaU16;             // 零点值整数
    unsigned short pMaxDtaU16;              // 最大值整数
    
    //unsigned short useValue;                // 使用的值(整数Or浮点数据)
    //unsigned short pOutDtaF32;              // 输出值浮点数据
    //unsigned short pzeroDtaF32;             // 零点值浮点数据
    //unsigned short pMaxDtaF32;              // 最大值浮点数据
};

#define sysAdCfgNonUse              (0) // AD 不使用
#define sysAdCfgZf10v               (1) // AD 量程正负10V
#define sysAdCfgZf05v               (2) // AD 量程正负5V 
#define sysAdCfgZf025v              (3) // AD 量程正负2.5V
#define sysAdCfgZ10v                (4) // AD 量程正0~10V
#define sysAdCfgZ05v                (5) // AD 量程正0~5V
#define sysAdCfgZ04_Z20ma           (6) // AD 量程4~20ma (1 ~ 5V)
#define sysAdCfgZ00_Z20ma           (7) // AD 量程0~20ma (0 ~ 5V)
#define sysAdCfgF20_Z20ma           (8) // AD 量程-20~20ma (-5 ~ 5V)

#define AD_FsmNonUse            (0) // AD FSM
#define AD_FsmIDLE              (1)
#define AD_FsmIsInit            (2)
#define AD_FsmStrart            (3)
#define AD_FsmRun               (4)
#define AD_FsmStop              (5)
struct tag_InAdState   // 18Byte
{
    unsigned short modSlc;                  // 模式选择(0:通道关闭  ,1：-10~10V,    2：-5~5V   , 3：-2.5~2.5V,   4：0~10V  5：0~5V     6：4~20mA    7：0~20mA   8：-20~20mA)
    unsigned short filteringMethod;         // 滤波方式(0：递推平均滤波 1：算术平均滤波 2：限幅滤波 3：中位值滤波 4：中位值平均滤波 5：限幅平均滤波 6：一阶滞后滤波 7：加权递推平均滤波)
    unsigned short filteringParm1;          // 滤波参数1
    unsigned short filteringParm2;          // 滤波参数2
    unsigned short sampleTick;              // 平均采样次数
    signed short zeroDta;                   // 零点数字量
    signed short maxDta;                    // 最大值数字量
    unsigned short pAverageU16;             // 平均值地址,    D元件或者R元件，    最高位为0：表示D元件，    最高位为1：表示R元件，    0x7FFF表示没有
    unsigned short pCurU16;                 // 当前值地址,D元件或者R元件，    最高位为0：表示D元件，    最高位为1：表示R元件，    0x7FFF表示没有
};

struct tag_InSysCfgAdDa
{
    //unsigned short cfgid;                   // = 0xAA19
    //unsigned short cfglen;

    
    unsigned short errState;                // D元件或者R元件，    最高位为0：表示D元件，    最高位为1：表示R元件，    0x7FFF表示没有
    unsigned short adSpeed;                 // 转换速度

    unsigned short adNum;                   // AD通道数
    struct tag_InAdState ad[8];
    
    unsigned short daNum;                   // DA通道数
    struct tag_InDaState da[4];

    volatile unsigned short AdFsm;                   // 处理时候用
    unsigned short DaFsm;
};

#define MAX_TCP_CONFIG_ITEM    3
typedef struct _MODBUS_TCP_CONFIG_ITEM
{
    char name[12];
    uint8_t connectMark;
    uint8_t connectType;
    ip4_addr_t ipTarget;
    uint16_t portTarget;
    uint16_t portLocal;
}modbus_tcp_config_item_st;

typedef struct _MODBUS_TCP_CONFIG
{
    uint32_t count;
    modbus_tcp_config_item_st item[MAX_TCP_CONFIG_ITEM];
    
}modbus_tcp_config_st;


#define MAX_SUB_MODULE_ITEM     200
#define MAX_SUB_STATION_ITEM    200
typedef struct _SUB_STATION_ITEM
{
    uint16_t sNumExt;// 扩展模块号
    uint16_t sNum;// 站号， 0 = 主站
    uint16_t len;
    uint8_t *pSubStationbuf;
}sub_station_item_st;

typedef enum _DAISY_E_TYPE
{
    DAISY_E_TYPE_WKC = 0x00,
    DAISY_E_TYPE_X = 0x01,
    DAISY_E_TYPE_Y = 0x02,
    DAISY_E_TYPE_M = 0x03,
    DAISY_E_TYPE_D = 0x09,
    DAISY_E_TYPE_SD= 0x0C,
    DAISY_E_TYPE_R = 0x0F,
}daisy_element_type_e;

typedef struct _PDO_ST
{
    uint16_t sType;//存取类型
    uint8_t len;  //字节
    daisy_element_type_e eType;//元件类型
    uint16_t eAddr; //元件地址或WKC分站号
    uint16_t offset;//发给从站数据的buffer的偏移地址
}pdo_st;

typedef struct _BUS_CONFIG
{
    uint16_t   nSynBuffLen;/* 同步数据帧的字节数 */
    uint16_t   nSdoCount; // 主站SDO个数
    
    uint16_t   nBfm6084; // 本站数据在帧中的偏移
    uint16_t   nBfm6085; // 本站数据的长度（字节）
    uint16_t   nBfm6086; // 网口后面挂接的模块数
    uint16_t   nBfm6087; // SPI后面挂接的模块数
    uint16_t   nBfm608A;  //超时时间

    uint16_t   nPdoBytesExt; // 主站扩展模块PDO字节数
    uint16_t   nPdoCountExt; // 主站扩展模块PDO个数
    uint16_t   pdo1ACountExt;
    uint16_t   pdo1BCountExt;
    pdo_st *pPDO1A_Ext;//TxPDO（相对于扩展模块）
    pdo_st *pPDO1B_Ext;//RxPDO（相对于扩展模块）

    uint16_t   nPdoBytes; // 从站PDO字节数
    uint16_t   nPdoCount; // 从站PDO个数
    uint16_t   pdo1ACount;
    uint16_t   pdo1BCount;
    pdo_st *pPDO1A;//TxPDO（相对于从站）
    pdo_st *pPDO1B;//RxPDO（相对于从站）

    uint16_t   nSubStationCount; // 从站+所有扩展模块的个数
    uint16_t   nSlaveCount; // 从站个数
    uint16_t   nMasterExtCount; // 主站的扩展模块个数
    sub_station_item_st item[MAX_SUB_STATION_ITEM];    
}bus_config_st;


typedef struct _SOEM_PDO_ST
{
    uint16_t cIdx;  // 0x1600 ~ 0x1603 or 0x1A00 ~ 0x1A03
    /* 参看LXM28E User Guide_Chinese.pdf的325页
     * 6040 - 索引
     *     00   - 子索引
     *       08   - 对象长度是8位
     *       10   - 对象长度是16位
     *       20   - 对象长度是32位
    */ 
    uint32_t sIdx;

    uint8_t len;   //比特 (bit)
    daisy_element_type_e eType;//元件类型
    uint16_t eAddr; //元件地址
    uint16_t offset;//发给从站数据的buffer的偏移地址, Must exist?
} soem_pdo_st;

typedef struct _SOEM_SDO_ST
{
    uint16_t idx;
    uint8_t  subIdx;
    uint8_t  len;   //字节    
    uint32_t val;
} soem_sdo_st;

typedef struct _SOEM_SLAVE_ST
{
    uint16_t nSdoCount;
    uint16_t nPdoCount;

    soem_sdo_st *pSDO;

    /* RxPDO（相对于从站） */
    uint16_t pdoRxCount;
    soem_pdo_st *pRxPDO;//0x1600

    /* TxPDO（相对于从站） */
    uint16_t pdoTxCount;
    soem_pdo_st *pTxPDO;//0x1A00

    uint16_t   outSize;//相对于主站（即：对于从站来讲就是RxPDO）
    uint16_t   inSize; //相对于主站（即：对于从站来讲就是TxPDO）
} soem_slave_st;

typedef struct _SOEM_CONFIG
{
    uint16_t   nSynBuffLen;/* 同步数据帧的字节数 */
    uint16_t   nSubStationCount;//从站个数
    soem_slave_st *pSlaves;
}soem_config_st;

typedef union
{
    unsigned char BYTE;
    struct
    {
        unsigned char HaveDM169: 1;       //DM169 8In 8Out 1COM,
        unsigned char HaveCM010: 1;       //CM010 4G模块
        unsigned char HaveDM168: 1;       //DM168 8In 8Out 1COM,
        unsigned char Have_0003: 1;       //备用；
        unsigned char Have_0004: 1;       //备用；
        unsigned char Have_0005: 1;       //备用；
        unsigned char Have_0006: 1;       //备用；
        unsigned char Have_0007: 1;       //备用；
    } BIT;
} unnion_left_modules;


extern struct tag_InSysCfgAdDa gt_InAdDaCfg;
extern modbus_tcp_config_st gModbusTcpConfig;
extern bus_config_st gBusConfig;
extern char plc_parse_system_block(unsigned char flag);
extern soem_config_st       gSoemConfig;
extern unnion_left_modules  gLeftModules;
extern unsigned char plc_parse_sysblk_modlink_config(unsigned short *lsp_ConfigPtr);
extern unsigned char plc_parse_sysblk_uart_config(unsigned char lcv_UartPort, unsigned short *lsp_ConfigPtr);
extern unsigned char gPlcRunAs;
#endif /*__PLC_SYS_BLOCK_H*/

