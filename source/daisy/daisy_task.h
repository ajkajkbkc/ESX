/**
  ******************************************************************************
  * @file    daisy_task.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2020-04-18
  * @brief
  ******************************************************************************
  */
#ifndef _DAISY_TASK_H
#define _DAISY_TASK_H
#include "FreeRTOS.h"
#include "task.h"
#include "mb.h"
#include "plc_sysblock.h"


/**
  @brief 从站和扩展最大数量
  */
#define DAISY_MAX_EXTEND_NUM  33
#define DAISY_MAX_SLAVE_NUM   128 //MAX_SUB_STATION_ITEM


/**
  @brief 状态机
  */
typedef enum __DAISY_STATUS_E
{
    MASTER_IDLE                =   0, /* 空闲 */
    SLAVE_IDLE                 =   0, /* 空闲 */
    EXT_IDLE                   =   0, /* 空闲 */
    SMART_IDLE                 =   0, /* 空闲 */

    /* master */
    MASTER_SCAN_EXT            =   1, /* 扫描扩展 */
    MASTER_IN_SCAN_EXT         =   2, /* 正在扫描扩展 */
    MASTER_IN_SCAN_NUM_EXT     =   3, /* 扫描后编址扩展 */
    MASTER_SCAN_EXT_OVER       =   4, /* 扫描扩展结束 */

    MASTER_NUM_EXT             =  11, /* 编址扩展 */
    MASTER_IN_NUM_EXT          =  12, /* 正在编址扩展 */
    MASTER_NUM_EXT_OVER        =  13, /* 编址扩展结束 */

    MASTER_CONFIG_EXT          =  21, /* 收到配置扩展帧 */
    MASTER_IN_CONFIG_EXT       =  22, /* 开始配置扩展 */
    MASTER_CONFIG_EXT_OVER     =  23, /* 配置扩展结束 */

    MASTER_PRELOOP_EXT         =  31, /* 网口收到启动帧 */
    MASTER_IN_PRELOOP_EXT      =  32, /* 串口发送启动帧 */
    MASTER_PRELOOP_EXT_OVER    =  33, /* 串口启动帧结束 */

    MASTER_LOOP_EXT            =  41, /* 串口收到集数帧 */

    MASTER_SCAN_SLAVE          =  51, /* 扫描从站 */
    MASTER_IN_SCAN_SLAVE       =  52, /* 正在扫描从站 */
    MASTER_IN_SCAN_NUM_SLAVE   =  53, /* 扫描后编址从站 */
    MASTER_SCAN_SLAVE_OVER     =  54, /* 扫描从站结束 */

    MASTER_NUM_SLAVE           =  61, /* 编址从站 */
    MASTER_IN_NUM_SLAVE        =  62, /* 正在编址从站 */
    MASTER_NUM_SLAVE_OVER      =  63, /* 编址从站结束 */

    MASTER_CONFIG_SLAVE        =  71, /* 收到配置从站帧 */
    MASTER_IN_CONFIG_SLAVE     =  72, /* 开始配置从站 */
    MASTER_CONFIG_SLAVE_OVER   =  73, /* 配置从站结束 */

    MASTER_PRELOOP_SLAVE       =  81, /* 网口收到启动帧 */
    MASTER_IN_PRELOOP_SLAVE    =  82, /* 串口发送启动帧 */
    MASTER_PRELOOP_SLAVE_OVER  =  83, /* 串口启动帧结束 */

    MASTER_LOOP_SLAVE          =  91, /* 网口收到集数帧 */


    /* slave */
    SLAVE_SCAN_EXT             = 101, /* 扫描扩展 */
    SLAVE_IN_SCAN_EXT          = 102, /* 正在扫描扩展 */
    SLAVE_IN_SCAN_NUM_EXT      = 103, /* 扫描后编址扩展 */
    SLAVE_SCAN_EXT_OVER        = 104, /* 扫描扩展结束 */

    SLAVE_NUM_OK               = 111, /* 从站编址完成 */

    SLAVE_NUM_EXT              = 121, /* 编址扩展 */
    SLAVE_IN_NUM_EXT           = 122, /* 正在编址扩展 */
    SLAVE_NUM_EXT_OVER         = 123, /* 编址扩展结束 */

    SLAVE_CONFIG_EXT           = 131, /* 收到配置扩展帧 */
    SLAVE_IN_CONFIG_EXT        = 132, /* 开始配置扩展 */
    SLAVE_CONFIG_EXT_OVER      = 133, /* 配置扩展结束 */

    SLAVE_PRELOOP_EXT          = 141, /* 网口收到启动帧 */
    SLAVE_IN_PRELOOP_EXT       = 142, /* 串口发送启动帧 */
    SLAVE_PRELOOP_EXT_OVER     = 143, /* 串口启动帧结束 */

    SLAVE_LOOP_EXT             = 151, /* 网口收到集数帧 */


    /* extend */
    EXT_IN_NUM                 = 201, /* 扩展编址 */
    EXT_NUM_OVER               = 202, /* 扩展编址结束 */

    EXT_IN_CONFIG              = 211, /* 扩展配置 */
    EXT_CONFIG_OVER            = 212, /* 扩展配置结束 */

    EXT_IN_LOOP                = 221, /* 数据交互 */
    EXT_LOOP_OVER              = 222, /* 数据交互超过bfm608A时间 */


    /* smart */
    SMART_IN_NUM               = 231, /* 智能设备编址 */
    SMART_NUM_OVER             = 232, /* 智能设备编址结束 */

    SMART_IN_CONFIG            = 241, /* 智能设备配置 */
    SMART_CONFIG_OVER          = 242, /* 智能设备配置结束 */

    SMART_IN_LOOP              = 251, /* 数据交互 */
    SMART_LOOP_OVER            = 252, /* 数据交互超过bfm608A时间 */

} daisy_status_e;


/**
  @brief 错误码
  */
typedef enum _DAISY_ERR
{
    DAISY_COMMON_NO_ERR = 0,



    //主站发现错误，在向扩展通讯时
    DAISY_STATE_ERR_MASTER_RECV_EXT             = 0x0001, //接收扩展时，主站状态异常
    DAISY_STATE_LEN_ERR_MASTER_RECV_EXT         = 0x0002, //接收扩展时，接收长度和主站状态都异常

    DAISY_LEN_ERR_MASTER_NUM_EXT                = 0x0011, //编址扩展时，帧长度错误
    DAISY_CRC_ERR_MASTER_NUM_EXT                = 0x0012, //编址扩展时，CRC错误
    DAISY_CMD_ERR_MASTER_NUM_EXT                = 0x0013, //编址扩展时，返回其他指令
    DAISY_NODEADDR_ERR_MASTER_NUM_EXT           = 0x0014, //编址扩展时，返回模块数不对
    DAISY_TIMEOUT_ERR_MASTER_NUM_EXT            = 0x0015, //编址扩展超时

    DAISY_LEN_ERR_MASTER_SCAN_EXT               = 0x0021, //扫描扩展时，帧长度错误
    DAISY_CRC_ERR_MASTER_SCAN_EXT               = 0x0022, //扫描扩展时，CRC错误
    DAISY_CMD_ERR_MASTER_SCAN_EXT               = 0x0023, //扫描扩展时，返回其他指令
    DAISY_NODEADDR_ERR_MASTER_SCAN_EXT          = 0x0024, //扫描扩展时，地址错误

    DAISY_LEN_ERR_MASTER_CONFIG_EXT             = 0x0031, //配置扩展时，帧长度错误
    DAISY_CRC_ERR_MASTER_CONFIG_EXT             = 0x0032, //配置扩展时，CRC错误
    DAISY_CMD_ERR_MASTER_CONFIG_EXT             = 0x0033, //配置扩展时，返回其他指令
    DAISY_OTHER_ERR_MASTER_CONFIG_EXT           = 0x0034, //配置扩展时，返回正确码错误
    DAISY_TIMEOUT_ERR_MASTER_CONFIG_EXT         = 0x0035, //配置扩展超时

    DAISY_LEN_ERR_MASTER_PRELOOP_EXT            = 0x0041, //获取就绪扩展数时，帧长度错误
    DAISY_CRC_ERR_MASTER_PRELOOP_EXT            = 0x0042, //获取就绪扩展数时，CRC错误
    DAISY_CMD_ERR_MASTER_PRELOOP_EXT            = 0x0043, //获取就绪扩展数时，返回其他指令
    DAISY_NODEADDR_ERR_MASTER_PRELOOP_EXT       = 0x0044, //获取就绪扩展数时，返回数量错误
    DAISY_TIMEOUT_ERR_MASTER_PRELOOP_EXT        = 0x0045, //获取就绪扩展数超时

    DAISY_LEN_ERR_MASTER_LOOP_EXT               = 0x0051, //集数帧交互时，帧长度错误
    DAISY_CRC_ERR_MASTER_LOOP_EXT               = 0x0052, //集数帧交互时，CRC错误
    DAISY_CMD_ERR_MASTER_LOOP_EXT               = 0x0053, //集数帧交互时，返回其他指令
    DAISY_TIMEOUT_ERR_MASTER_LOOP_EXT           = 0x0054, //集数帧交互超时



    //主站发现错误，在向从站通讯时
    DAISY_STATE_ERR_MASTER_RECV_SLAVE           = 0x0101, //接收从站时，主站状态异常
    DAISY_STATE_LEN_ERR_MASTER_RECV_SLAVE       = 0x0102, //接收从站时，接收长度和主站状态都异常

    DAISY_LEN_ERR_MASTER_NUM_SLAVE              = 0x0111, //编址从站时，帧长度错误
    DAISY_CMD_ERR_MASTER_NUM_SLAVE              = 0x0112, //编址从站时，返回其他指令
    DAISY_NODEADDR_ERR_MASTER_NUM_SLAVE         = 0x0113, //编址从站时，返回模块数不对
    DAISY_TIMEOUT_ERR_MASTER_NUM_SLAVE          = 0x0114, //编址从站超时

    DAISY_LEN_ERR_MASTER_SCAN_SLAVE             = 0x0121, //扫描从站时，帧长度错误
    DAISY_CMD_ERR_MASTER_SCAN_SLAVE             = 0x0122, //扫描从站时，返回其他指令
    DAISY_NODEADDR_ERR_MASTER_SCAN_SLAVE        = 0x0123, //扫描从站时，返回模块地址不对
    DAISY_TIMEOUT_ERR_MASTER_SCAN_SLAVE         = 0x0124, //扫描从站时超时

    DAISY_LEN_ERR_MASTER_CONFIG_SLAVE           = 0x0131, //配置从站时，帧长度错误
    DAISY_CMD_ERR_MASTER_CONFIG_SLAVE           = 0x0132, //配置从站时，返回其他指令
    DAISY_OTHER_ERR_MASTER_CONFIG_SLAVE         = 0x0133, //配置从站时，返回正确码错误
    DAISY_TIMEOUT_ERR_MASTER_CONFIG_SLAVE       = 0x0134, //配置从站超时

    DAISY_LEN_ERR_MASTER_PRELOOP_SLAVE          = 0x0141, //获取就绪从站数时，帧长度错误
    DAISY_CMD_ERR_MASTER_PRELOOP_SLAVE          = 0x0142, //获取就绪从站数时，返回其他指令
    DAISY_NODEADDR_ERR_MASTER_PRELOOP_SLAVE     = 0x0143, //获取就绪从站数时，返回数量错误
    DAISY_TIMEOUT_ERR_MASTER_PRELOOP_SLAVE      = 0x0144, //获取就绪从站数超时

    DAISY_LEN_ERR_MASTER_LOOP_SLAVE             = 0x0151, //集数帧交互时，帧长度错误
    DAISY_CMD_ERR_MASTER_LOOP_SLAVE             = 0x0152, //集数帧交互时，返回其他指令
    DAISY_TIMEOUT_ERR_MASTER_LOOP_SLAVE         = 0x0153, //集数帧交互超时
    DAISY_WKC_LOST_ERR_MASTER_LOOP_SLAVE        = 0x0154, //集数帧交互，WKC丢失



    //从站发现错误，在向扩展通讯时
    DAISY_STATE_ERR_SLAVE_RECV_EXT              = 0x0201, //接收扩展时，从站状态异常
    DAISY_STATE_LEN_ERR_SLAVE_RECV_EXT          = 0x0202, //接收扩展时，接收长度和从站状态都异常

    DAISY_LEN_ERR_SLAVE_NUM_EXT                 = 0x0211, //编址扩展时，帧长度错误
    DAISY_CRC_ERR_SLAVE_NUM_EXT                 = 0x0212, //编址扩展时，CRC错误
    DAISY_CMD_ERR_SLAVE_NUM_EXT                 = 0x0213, //编址扩展时，返回其他指令
    DAISY_NODEADDR_ERR_SLAVE_NUM_EXT            = 0x0214, //编址扩展时，返回模块数不对
    DAISY_TIMEOUT_ERR_SLAVE_NUM_EXT             = 0x0215, //编址扩展超时

    DAISY_LEN_ERR_SLAVE_SCAN_EXT                = 0x0221, //扫描扩展时，帧长度错误
    DAISY_CRC_ERR_SLAVE_SCAN_EXT                = 0x0222, //扫描扩展时，CRC错误
    DAISY_CMD_ERR_SLAVE_SCAN_EXT                = 0x0223, //扫描扩展时，返回其他指令
    DAISY_NODEADDR_ERR_SLAVE_SCAN_EXT           = 0x0224, //扫描扩展时，地址错误

    DAISY_LEN_ERR_SLAVE_CONFIG_EXT              = 0x0231, //配置扩展时，帧长度错误
    DAISY_CRC_ERR_SLAVE_CONFIG_EXT              = 0x0232, //配置扩展时，CRC错误
    DAISY_CMD_ERR_SLAVE_CONFIG_EXT              = 0x0233, //配置扩展时，返回其他指令
    DAISY_OTHER_ERR_SLAVE_CONFIG_EXT            = 0x0234, //配置扩展时，返回正确码错误
    DAISY_TIMEOUT_ERR_SLAVE_CONFIG_EXT          = 0x0235, //配置扩展超时

    DAISY_LEN_ERR_SLAVE_PRELOOP_EXT             = 0x0241, //获取就绪扩展数时，帧长度错误
    DAISY_CRC_ERR_SLAVE_PRELOOP_EXT             = 0x0242, //获取就绪扩展数时，CRC错误
    DAISY_CMD_ERR_SLAVE_PRELOOP_EXT             = 0x0243, //获取就绪扩展数时，返回其他指令
    DAISY_NODEADDR_ERR_SLAVE_PRELOOP_EXT        = 0x0244, //获取就绪扩展数时，返回数量错误
    DAISY_TIMEOUT_ERR_SLAVE_PRELOOP_EXT         = 0x0245, //获取就绪扩展数超时

    DAISY_LEN_ERR_SLAVE_LOOP_EXT                = 0x0251, //集数帧交互时，帧长度错误
    DAISY_CRC_ERR_SLAVE_LOOP_EXT                = 0x0252, //集数帧交互时，CRC错误
    DAISY_CMD_ERR_SLAVE_LOOP_EXT                = 0x0253, //集数帧交互时，返回其他指令
    DAISY_TIMEOUT_ERR_SLAVE_LOOP_EXT            = 0x0254, //集数帧交互超时



    //从站发现错误，在收主站数据时
    DAISY_LEN_ERR_IN_SLAVE                      = 0x0301, //从站收到帧长错误
    DAISY_CMD_ERR_IN_SLAVE                      = 0x0302, //从站收到指令错误

    DAISY_NODEADDR_ERR_IN_NUM_SLAVE             = 0x0311, //编址从站时，没有对应的模块地址

    DAISY_SLAVE_NOT_ADDR_ERR_IN_SCAN_SLAVE      = 0x0321, //扫描从站时，从站未编址
    DAISY_NODEADDR_ERR_IN_SCAN_SLAVE            = 0x0322, //扫描从站时，没有对应的模块地址

    DAISY_SLAVE_NOT_ADDR_ERR_IN_CONFIG_SLAVE    = 0x0331, //配置从站时，从站未编址
    DAISY_NODEADDR_ERR_IN_CONFIG_SLAVE          = 0x0332, //配置从站时，没有对应的模块地址
    DAISY_SLAVE_ID_ERR_IN_CONFIG_SLAVE          = 0x0333, //配置从站时，配置错误的从站编号，如CR100是0x1A06
    DAISY_BFM_IDX_SDO_ERR_IN_CONFIG_SLAVE       = 0x0334, //配置从站时，配置SDO index 错误
    DAISY_BFM_IDX_PDO_ERR_IN_CONFIG_SLAVE       = 0x0335, //配置从站时，配置PDD index 错误
    DAISY_BFM_LEN_ERR_IN_CONFIG_SLAVE           = 0x0336, //配置从站时，配置BFM长度错误
    DAISY_EXT_NUM_ERR_IN_CONFIG_SLAVE           = 0x0337, //配置从站时，扩展数量大于扫描出的扩展数

    DAISY_SLAVE_NOT_ADDR_ERR_IN_PRELOOP_SLAVE   = 0x0341, //获取就绪从站数时，从站未编址
    DAISY_SLAVE_NOT_CONFIG_ERR_IN_PRELOOP_SLAVE = 0x0342, //获取就绪从站数时，从站未配置
    DAISY_NODEADDR_ERR_IN_PRELOOP_SLAVE         = 0x0343, //获取就绪从站数时，没有对应的模块地址

    DAISY_SLAVE_NOT_ADDR_ERR_IN_LOOP_SLAVE      = 0x0351, //集数帧从站时，从站未编址
    DAISY_SLAVE_NOT_CONFIG_ERR_IN_LOOP_SLAVE    = 0x0352, //集数帧从站时，从站未配置
    DAISY_SLAVE_NOT_PRELOOP_ERR_IN_LOOP_SLAVE   = 0x0353, //集数帧从站时，从站未准备就绪
    DAISY_NODEADDR_ERR_IN_LOOP_SLAVE            = 0x0354, //集数帧从站时，没有对应的模块地址



    //扩展发现错误，在收主站/从站数据时
    DAISY_LEN_ERR_IN_EXTEND                     = 0x0401, //扩展收到帧长错误
    DAISY_CRC_ERR_IN_EXTEND                     = 0x0402, //扩展收到CRC错误
    DAISY_CMD_ERR_IN_EXTEND                     = 0x0403, //扩展收到指令错误

    DAISY_EXT_NOT_ADDR_ERR_IN_SCAN_EXT          = 0x0421, //扫描扩展时，扩展未编址

    DAISY_EXT_NOT_ADDR_ERR_IN_CONFIG_EXT        = 0x0431, //配置扩展时，扩展未编址
    DAISY_EXT_ID_ERR_IN_CONFIG_EXT              = 0x0432, //配置扩展时，配置错误的扩展编号，如TC060是0x0021
    DAISY_BFM_IDX_SDO_ERR_IN_CONFIG_EXT         = 0x0433, //配置扩展时，配置SDO index 错误
    DAISY_BFM_IDX_PDO_ERR_IN_CONFIG_EXT         = 0x0434, //配置扩展时，配置PDD index 错误
    DAISY_BFM_LEN_ERR_IN_CONFIG_EXT             = 0x0435, //配置扩展时，配置BFM长度错误

    DAISY_EXT_NOT_ADDR_ERR_IN_PRELOOP_EXT       = 0x0441, //预启动扩展时，扩展未编址

    DAISY_EXT_NOT_ADDR_ERR_IN_LOOP_EXT          = 0x0451, //集数帧扩展时，扩展未编址
    DAISY_EXT_NOT_CONFIG_ERR_IN_LOOP_EXT        = 0x0452, //集数帧扩展时，扩展未配置



    //智能设备发现错误，在收主站数据时
    DAISY_LEN_ERR_IN_SMART                      = 0x0501, //智能设备收到帧长错误
    DAISY_CMD_ERR_IN_SMART                      = 0x0502, //智能设备收到指令错误

    DAISY_NODEADDR_ERR_IN_NUM_SMART             = 0x0511, //编址智能设备时，没有对应的模块地址

    DAISY_SMART_NOT_ADDR_ERR_IN_SCAN_SMART      = 0x0521, //扫描智能设备时，智能设备未编址
    DAISY_NODEADDR_ERR_IN_SCAN_SMART            = 0x0522, //扫描智能设备时，没有对应的模块地址

    DAISY_SMART_NOT_ADDR_ERR_IN_CONFIG_SMART    = 0x0531, //配置智能设备时，智能设备未编址
    DAISY_NODEADDR_ERR_IN_CONFIG_SMART          = 0x0532, //配置智能设备时，没有对应的模块地址
    DAISY_SMART_ID_ERR_IN_CONFIG_SMART          = 0x0533, //配置智能设备时，配置错误的智能设备编号
    DAISY_BFM_IDX_SDO_ERR_IN_CONFIG_SMART       = 0x0534, //配置智能设备时，配置SDO index 错误
    DAISY_BFM_PDO_STYPE_ERR_IN_CONFIG_SMART     = 0x0535, //配置智能设备时，PDO属性错误（不是1A、1B）
    DAISY_BFM_LEN_ERR_IN_CONFIG_SMART           = 0x0536, //配置智能设备时，配置BFM长度错误
    DAISY_HAVE_EXT_ERR_IN_CONFIG_SMART          = 0x0537, //配置智能设备时，扩展地址不为0

    DAISY_SMART_NOT_ADDR_ERR_IN_PRELOOP_SMART   = 0x0541, //获取就绪智能设备时，智能设备未编址
    DAISY_SMART_NOT_CONFIG_ERR_IN_PRELOOP_SMART = 0x0542, //获取就绪智能设备时，智能设备未配置
    DAISY_NODEADDR_ERR_IN_PRELOOP_SMART         = 0x0543, //获取就绪智能设备时，没有对应的模块地址

    DAISY_SMART_NOT_ADDR_ERR_IN_LOOP_SMART      = 0x0551, //集数帧智能设备时，智能设备未编址
    DAISY_SMART_NOT_CONFIG_ERR_IN_LOOP_SMART    = 0x0552, //集数帧智能设备时，智能设备未配置
    DAISY_NODEADDR_ERR_IN_LOOP_SMART            = 0x0553, //集数帧智能设备时，没有对应的模块地址


} daisy_err_e;


typedef uint32_t id_version_t;

typedef struct _SCAN_SLAVE_EXT_ID
{
    uint32_t num;
    id_version_t *pIdList;
} scan_slave_ext_id_st;


typedef struct _MASTER_RUN_INFO
{
    uint16_t flag; //标志
    uint16_t node_addr; //本站号 当智能设备时才会用到

    uint8_t daisy_uart_err_flag; //背板总线错误标志
    uint8_t daisy_err_flag; //以太网总线错误标志

    uint8_t daisy_status; //网口总线状态机
    uint8_t daisy_uart_status; //背板总线状态机
    uint8_t daisy_err_times; //网口总线错误次数
    uint8_t daisy_uart_err_times; //背板总线错误次数
    uint16_t daisy_totalerr_times; //网口总线错误总次数
    uint16_t daisy_uart_totalerr_times; //背板总线错误总次数

    uint16_t daisy_index; //网口总线下标
    uint16_t daisy_uart_index; //背板总线下标

    uint16_t scan_ext_num; //扫描主站下扩展数量
    uint32_t scan_ext_id_version[DAISY_MAX_EXTEND_NUM]; //扫描主站下扩展ID和版本表

    uint16_t scan_slave_num; //扫描从站数量
    id_version_t scan_slave_ext_id_version_temp[DAISY_MAX_EXTEND_NUM + 1];
    scan_slave_ext_id_st scan_slave_ext_id_version[DAISY_MAX_SLAVE_NUM]; //每个从站的模块ID和版本表
} master_run_info_st;


typedef struct _MAP_ERR_NODE
{
    uint16_t val;
    uint16_t offset;
} err_st;


typedef struct _SMART_BFM_ST
{
    uint16_t bfm6081; // SLAVE_NUMBER;//Define who am I
    uint16_t bfm6082; // SLAVE_VERSION;
    //err_st   bfm6083; //智能设备不需要
    uint16_t bfm6084; //本站数据在帧中的偏移
    uint16_t bfm6085; //本站数据的长度（字节）
    uint16_t bfm6086; //网口后面挂接的模块数
    uint16_t bfm6087; //UART后面挂接的模块数
    uint16_t bfm6088; //本从站的数据长度

    uint16_t bfm608A; // 超时时间

    uint16_t nPdoBytesSmart; // 智能设备PDO字节数
    uint16_t nPdoCountSmart; // 智能设备PDO个数
    uint16_t pdo1ACountSmart;
    uint16_t pdo1BCountSmart;
    pdo_st *pPDO1A_Smart;//TxPDO（相对于智能设备）
    pdo_st *pPDO1B_Smart;//RxPDO（相对于智能设备）

} smart_bfm_st;


/** @addtogroup Exported_macros
  * @{
  */
#define SET_BIT(REG, BIT)     ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT)   ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT)    ((REG) & (BIT))
#define CLEAR_REG(REG)        ((REG) = (0x0))
#define WRITE_REG(REG, VAL)   ((REG) = (VAL))
#define READ_REG(REG)         ((REG))
#define MODIFY_REG(REG, CLEARMASK, SETMASK)  WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))


/**
  @brief 将从站和扩展地址组合成U16的地址
  */
#define DAISY_NODE_ADDRESS(slave_addr, ext_addr)    ((ext_addr) | ((slave_addr) << 8))


/**
  @brief 变量flag为U16型，存储各个标志
  */
#define SMART_RUN_FLAG_LAST_NODE_Msk        0x0001  //我是菊花链的最后一个Node吗？

#define MASTER_RUN_FLAG_EXT_NUM_Msk         0x0002  //扩展已编址
#define MASTER_RUN_FLAG_EXT_CONFIG_Msk      0x0004  //下发扩展配置完成
#define MASTER_RUN_FLAG_EXT_PRELOOP_Msk     0x0008  //扩展可启动

#define MASTER_RUN_FLAG_SLAVE_NUM_Msk       0x0010  //从站已编址
#define MASTER_RUN_FLAG_SLAVE_CONFIG_Msk    0x0020  //从站已配置并初始化完成
#define MASTER_RUN_FLAG_SLAVE_PRELOOP_Msk   0x0040  //从站可启动

#define MASTER_RUN_FLAG_EXT_CAN_LOOP_Msk    (MASTER_RUN_FLAG_EXT_NUM_Msk | MASTER_RUN_FLAG_EXT_CONFIG_Msk | MASTER_RUN_FLAG_EXT_PRELOOP_Msk)
#define MASTER_RUN_FLAG_SLAVE_CAN_LOOP_Msk  (MASTER_RUN_FLAG_SLAVE_NUM_Msk | MASTER_RUN_FLAG_SLAVE_CONFIG_Msk | MASTER_RUN_FLAG_SLAVE_PRELOOP_Msk)

#define SMART_RUN_FLAG_SMART_NUM_Msk        0x0100  //智能设备已编址
#define SMART_RUN_FLAG_SMART_CONFIG_Msk     0x0200  //智能设备已配置并初始化完成
#define SMART_RUN_FLAG_SMART_RUNNING_Msk    0x0400  //正在数据交互3333

#define SMART_RUN_FLAG_SMART_CAN_LOOP_Msk   (SMART_RUN_FLAG_SMART_NUM_Msk | SMART_RUN_FLAG_SMART_CONFIG_Msk)

#define MASTER_RUN_FLAG_EXT_RUNNING_Msk     0x1000  //正在数据交互3333
#define MASTER_RUN_FLAG_SLAVE_RUNNING_Msk   0x2000  //正在数据交互3333


/**
  @brief 等待时间(ms)
  */
#define DAISY_WAIT_EXT_0101_MAX_TIME    500
#define DAISY_WAIT_EXT_0303_MAX_TIME    500
#define DAISY_WAIT_EXT_1C1C_MAX_TIME    500
#define DAISY_WAIT_EXT_2222_MAX_TIME    500
#define DAISY_WAIT_EXT_3333_MAX_TIME    10

#define DAISY_WAIT_SLAVE_0101_MAX_TIME  500
#define DAISY_WAIT_SLAVE_0707_MAX_TIME  500
#define DAISY_WAIT_SLAVE_1C1C_MAX_TIME  500
#define DAISY_WAIT_SLAVE_2222_MAX_TIME  500
#define DAISY_WAIT_SLAVE_3333_MAX_TIME  10

/**
  @brief 操作失败后重试时间间隔（ms）
  */
#define DAISY_SCAN_EXT_ERR_DELAY_TIME      10000
#define DAISY_NUM_EXT_ERR_DELAY_TIME       2000
#define DAISY_CONFIG_EXT_ERR_DELAY_TIME    2000
#define DAISY_PRELOOP_EXT_ERR_DELAY_TIME   2000

#define DAISY_SCAN_SLAVE_ERR_DELAY_TIME    10000
#define DAISY_NUM_SLAVE_ERR_DELAY_TIME     2000
#define DAISY_CONFIG_SLAVE_ERR_DELAY_TIME  2000
#define DAISY_PRELOOP_SLAVE_ERR_DELAY_TIME 2000


/**
  @brief 总线开始延时时间（ms） 避免因智能设备网口没启动导致该设备扫描不上来
  */
#define DAISY_READY_START_DELAY_TIME       1500
#define DAISY_UART_READY_START_DELAY_TIME  1500


/**
  @brief 错误最大次数
  */
#define DAISY_CONFIG_ERR_MAX_TIMES            10 //下发配置最大错误次数
#define DAISY_PRELOOP_ERR_MAX_TIMES           10 //预启动最大错误次数
#define DAISY_LOOP_ERR_MAX_TIMES              10 //集数帧最大错误次数

#define DAISY_UART_CONFIG_ERR_MAX_TIMES       10 //下发配置最大错误次数
#define DAISY_UART_PRELOOP_ERR_MAX_TIMES      10 //预启动最大错误次数
#define DAISY_UART_LOOP_ERR_MAX_TIMES         10 //集数帧最大错误次数



/**
  @brief 总线命令
  */
#define DAISY_CMD_0101    0x0101 // 获取从站个数
#define DAISY_CMD_0303    0x0303 // 获取单个模块ID
#define DAISY_CMD_0707    0x0707 // 获取从站下的扩展信息（数量和类型）
#define DAISY_CMD_1C1C    0x1C1C // 逐个配置从站属性
#define DAISY_CMD_2222    0x2222 // 收到该帧后，从站变为数据交换工作模式
#define DAISY_CMD_3333    0x3333 // 主循环
#define DAISY_CMD_5555    0x5555 // 错误帧
#define DAISY_CMD_8888    0x8888 // 升级


/**
  @brief BFM index
  */
#define BFM_IDX_0x6081    0x6081 //存放模块识别码
#define BFM_IDX_0x6082    0x6082 //存放版本号
#define BFM_IDX_0x6083    0x6083 //存放错误码
#define BFM_IDX_0x6084    0x6084 //本站数据在帧中的偏移
#define BFM_IDX_0x6085    0x6085 //本站数据的长度（字节）
#define BFM_IDX_0x6086    0x6086 //网口后面挂接的模块数
#define BFM_IDX_0x6087    0x6087 //UART后面挂接的模块数
#define BFM_IDX_0x6088    0x6088 //所有Pdo字节数
#define BFM_IDX_0x608A    0x608A //UART后面挂接的模块数




extern smart_bfm_st gSmartBFM;
extern volatile master_run_info_st gMasterRunInfo;

extern TaskHandle_t gDaisyTaskHandle;
extern uint16_t gSlaveIDUpgrade;
extern uint8_t gDaisyLANRecvBuffer[512];




extern void start_daisy_task(void);
extern void daisy_get_info(md_slave_msg_pack *pMsg);
extern void kalyke_daisy_init(void);
extern void kalyke_daisy_stop(void);
extern void daisy_LAN_send_bin(uint8_t *pBuf, uint16_t len);

extern uint16_t daisy_handle_config(uint8_t *pBuf, uint16_t *pErrContent);
extern void daisy_handle_run_led(void);
extern void saveErrorCodeFIFO(uint16_t nErrorCode);

static inline uint8_t daisy_get_8bit_element_value(uint16_t *baseAddr, uint16_t element)
{
    uint8_t lcv_ElementValue;

    lcv_ElementValue = (*(baseAddr + (element >> 4)) >> (element & 0x0F));

    return lcv_ElementValue;
}

static inline void daisy_set_8bit_element_value(unsigned short *baseAddr, unsigned short bitElement, unsigned char value)
{
    baseAddr += (bitElement >> 4);
    if ((bitElement & 0x0F) == 0)
    {
        *baseAddr &= 0xFF00;
        *baseAddr |= value;
    }
    else
    {
        *baseAddr &= 0x00FF;
        *baseAddr |= (value << 8);
    }
}

#endif /* _DAISY_TASK_H */

