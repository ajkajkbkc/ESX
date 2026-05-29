/**
  ******************************************************************************
  * @file    plc_element.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   PLC中各种原件相关操作
  ******************************************************************************
  */

#ifndef __PLC_ELEMENT_H
#define __PLC_ELEMENT_H

#include "kalyke_opts.h"

//#define PLC_ELEMENT_USE_DYNAMIC_MALLOC

/*------------------------------------------------------------------------------
* 子程序常量定义
*-----------------------------------------------------------------------------*/
/*最大子程序个数*/
#define MAX_SBR_COUNT       (64)
/*子程序最大可嵌套调用层数*/
#define MAX_SBR_NESTED_LAYER      (8)
/*CALL指令最大调用次数*/
#define MAX_CALL_INS_USE_NUMBER     (256)
/*最大MC-MCR指令块数量*/
#define MAX_MC_MCR_BLOCK_NUMBER     (8)
/*最大循环嵌套次数*/
#define MAX_FOR_NEXT_NESTED_NUM     (8)
/*SFC 并行分支汇合结构最大分支数*/
#define MAX_SFC_SERIES_STL_NUM      (16)

#define SD_ELEMENT_CNT  1024
#define SM_ELEMENT_CNT  1024
#define X_ELEMENT_CNT   2048
#define Y_ELEMENT_CNT   2048
#define M_ELEMENT_CNT   16384 //16*1024
#define LM_ELEMENT_CNT  64
#define S_ELEMENT_CNT   4096
#define D_ELEMENT_CNT   8000
#define R_ELEMENT_CNT   32768 // 32*1024
#define V_ELEMENT_CNT   64
#define Z_ELEMENT_CNT   16

#define C_ELEMENT_CNT          512
#define C_ELEMENT_16_BIT_CNT   200
#define C_ELEMENT_32_BIT_CNT   312 //(C_ELEMENT_CNT - C_ELEMENT_16_BIT_CNT)

#define T_ELEMENT_CNT               512
#define T_ELEMENT_100MS_TIMER_CNT   256
#define T_ELEMENT_10MS_TIMER_CNT    128
#define T_ELEMENT_1MS_TIMER_CNT     128



/*PLC中计数器C元件定义*/
typedef struct __PLC_C_ELEMENT_INFO_ST {
    /*元件数量*/
    unsigned short  msv_ElementCnt;
    /*16位计数器数量*/
    unsigned short  msv_16bitCnt;
    /*32位计数器数量*/
    unsigned short  msv_32bitCnt;
} plc_c_element_info_st;

/*PLC C计数器元件状态信息*/
typedef struct __PLC_C_ELEMENT_STATUS_ST{
    /*能流存储位*/
    unsigned char mbv_PowerFlow :1;
    /* 高速计数器已运行 */
    unsigned char mbv_Started   :1;
    /*保留位*/
    unsigned char mbv_Reserved  :6;
}plc_c_element_status_st;

/*C 元件运行态结构体*/
#ifdef PLC_ELEMENT_USE_DYNAMIC_MALLOC
typedef struct __PLC_C_ELEMENT_ST{
    /*C 位元件指针*/
    unsigned short *msp_BitElement;
    /*16位计数器当前值*/
    unsigned short *msp_16BitValue;
    /*32位计数器当前值*/
    long *msp_32BitValue;
    /*C 元件状态信息*/
    plc_c_element_status_st *mtp_StatusInfo;
}plc_c_element_st;
#else
typedef struct __PLC_C_ELEMENT_ST{
    /*C 位元件指针*/
    unsigned short msp_BitElement[C_ELEMENT_CNT/16];
    /*16位计数器当前值*/
    unsigned short msp_16BitValue[C_ELEMENT_16_BIT_CNT];
    /*32位计数器当前值*/
    long msp_32BitValue[C_ELEMENT_32_BIT_CNT];
    /*C 元件状态信息*/
    plc_c_element_status_st mtp_StatusInfo[C_ELEMENT_CNT];
}plc_c_element_st;
#endif

/*PLC中定时器T元件定义*/
typedef struct __PLC_T_ELEMENT_INFO_ST {
    /*元件数量*/
    unsigned short msv_ElementCnt;
    /*100ms计时精度定时器数量*/
    unsigned short msv_100msTimerCnt;
    /*10ms计时精度定时器数量*/
    unsigned short msv_10msTimerCnt;
    /*1ms计时精度定时器数量*/
    unsigned short msv_1msTimerCnt;
} plc_t_element_info_st;

/*PLC中定时器元件状态结构体定义*/
typedef struct __PLC_T_ELEMENT_STATUS_ST{
    /*定时器使能标志*/
    unsigned char mbv_Enable    :1;
    /*能流存储位*/
    unsigned char mbv_PowerFlow :1;
    /*能流需要初始化标志*/
    unsigned char mbv_Init      :1;
    /*定时器类型*/
    unsigned char mbv_Type      :2;
    /*定时器精度*/
    unsigned char mbv_Capacity  :3;
}plc_t_element_status_st;

/*T 元件运行态结构体*/
#ifdef PLC_ELEMENT_USE_DYNAMIC_MALLOC
typedef struct __PLC_T_ELEMENT_ST{
    /*T 位元件指针*/
    unsigned short *msp_BitElement;
    /*T 元件当前值数组*/
    unsigned short *msp_CurrentValue;
    /*T 元件定时目标值*/
    unsigned short *msp_DestValue;
    /*T 元件开始值*/
    unsigned long *mlp_StartValue;
    /*T 元件状态信息*/
    plc_t_element_status_st *mtp_StatusInfo;
}plc_t_element_st;
#else
typedef struct __PLC_T_ELEMENT_ST{
    /*T 位元件指针*/
    unsigned short msp_BitElement[T_ELEMENT_CNT/16];
    /*T 元件当前值数组*/
    unsigned short msp_CurrentValue[T_ELEMENT_CNT];
    /*T 元件定时目标值*/
    unsigned short msp_DestValue[T_ELEMENT_CNT];
    /*T 元件开始值*/
    unsigned long mlp_StartValue[T_ELEMENT_CNT];
    /*T 元件状态信息*/
    plc_t_element_status_st mtp_StatusInfo[T_ELEMENT_CNT];
}plc_t_element_st;
#endif

/*PLC中各种元件信息定义*/
typedef struct __PLC_ELEMENT_INFO_ST {
    /*X元件数量*/
    unsigned short  msv_XElementCnt;
    /*Y元件数量*/
    unsigned short  msv_YElementCnt;
    /*M元件数量*/
    unsigned short  msv_MElementCnt;
    /*LM元件数量*/
    unsigned short  msv_LMElementCnt;
    /*SM元件数量*/
    unsigned short  msv_SMElementCnt;
    /*S元件数量*/
    unsigned short  msv_SElementCnt;
    /*D元件数量*/
    unsigned short  msv_DElementCnt;
    /*R元件数量*/
    unsigned short  msv_RElementCnt;
    /*SD元件数量*/
    unsigned short  msv_SDElementCnt;
    /*V元件数量*/
    unsigned short  msv_VElementCnt;
    /*Z元件数量*/
    unsigned short  msv_ZElementCnt;
    /*C元件定义*/
    plc_c_element_info_st msv_CElement;
    /*T元件定义*/
    plc_t_element_info_st msv_TElement;
} plc_element_info_st;


/*实际存储各元件值结构体定义*/
#ifdef PLC_ELEMENT_USE_DYNAMIC_MALLOC
typedef struct __PLC_ELEMENT_ST {
    /*X元件存储空间*/
    unsigned short  *msp_XElement;
    /*Y元件存储空间*/
    unsigned short  *msp_YElement;
    /*M元件存储空间*/
    unsigned short  *msp_MElement;
    /*LM元件存储空间*/
    unsigned short  *msp_LMElement;
    /*SM元件存储空间*/
    unsigned short  *msp_SMElement;
    /*S元件存储空间*/
    unsigned short  *msp_SElement;
    /*D元件存储空间*/
    unsigned short  *msp_DElement;
    /*R元件存储空间*/
    unsigned short  *msp_RElement;
    /*SD元件存储空间*/
    unsigned short  *msp_SDElement;
    /*V元件存储空间*/
    unsigned short  *msp_VElement;
    /*Z元件存储空间*/
    unsigned short  *msp_ZElement;
    /*C元件定义*/
    plc_c_element_st mtv_CElement;
    /*T元件定义*/
    plc_t_element_st mtv_TElement;
} plc_element_st;
#else
typedef struct __PLC_ELEMENT_ST {
    /*X元件存储空间*/
    unsigned short  msp_XElement[X_ELEMENT_CNT/16];
    /*Y元件存储空间*/
    unsigned short  msp_YElement[Y_ELEMENT_CNT/16];
    /*M元件存储空间*/
    unsigned short  msp_MElement[M_ELEMENT_CNT/16*MAX_SBR_NESTED_LAYER];//16K bytes
    /*LM元件存储空间*/
    unsigned short  msp_LMElement[LM_ELEMENT_CNT/16*MAX_SBR_NESTED_LAYER];
    /*SM元件存储空间*/
    unsigned short  msp_SMElement[SM_ELEMENT_CNT/16];//512 bytes
    /*S元件存储空间*/
    unsigned short  msp_SElement[S_ELEMENT_CNT/16];//512 bytes
    /*D元件存储空间*/
    unsigned short  msp_DElement[D_ELEMENT_CNT];//16000 bytes
    /*R元件存储空间*/
    unsigned short  *msp_RElement;
    /*SD元件存储空间*/
    unsigned short  msp_SDElement[SD_ELEMENT_CNT];
    /*V元件存储空间*/
    unsigned short  msp_VElement[V_ELEMENT_CNT*MAX_SBR_NESTED_LAYER];
    /*Z元件存储空间*/
    unsigned short  msp_ZElement[Z_ELEMENT_CNT];
    /*C元件定义*/
    plc_c_element_st mtv_CElement; //2224 bytes
    /*T元件定义*/
    plc_t_element_st mtv_TElement; //4672 bytes
} plc_element_st;
#endif

/*------------------------------------------------------------------------------
*   元件读取/设置宏定义
*-----------------------------------------------------------------------------*/
/*读SD元件*/
#define GET_SD_ELEMENT_VALUE(element)   (gtv_PlcElement.msp_SDElement[element])
/*设置第element个SD元件值为value*/
#define SET_SD_ELEMENT_VALUE(element, value)    (gtv_PlcElement.msp_SDElement[element] = value)

/*取第layer层第element个V元件值*/
#define GET_V_ELEMENT_VALUE(layer, element)  (*(unsigned short *)(gtv_PlcElement.msp_VElement + gtp_PlcElementInfo->msv_VElementCnt*layer + element))
/*设置第layer层第element个V元件值为value*/
#define SET_V_ELEMENT_VALUE(layer, element, value) (*(unsigned short *)(gtv_PlcElement.msp_VElement + gtp_PlcElementInfo->msv_VElementCnt*layer + element) = value)
/*取layer层第element个V元件地址*/
#define GET_V_ELEMENT_ADDR(layer, element)  ((unsigned short *)(gtv_PlcElement.msp_VElement + gtp_PlcElementInfo->msv_VElementCnt*layer + element))

/*读Z元件*/
#define GET_Z_ELEMENT_VALUE(element)   (gtv_PlcElement.msp_ZElement[element])
/*设置第element个Z元件值为value*/
#define SET_Z_ELEMENT_VALUE(element, value)    (gtv_PlcElement.msp_ZElement[element] = value)

/*读D元件*/
#define GET_D_ELEMENT_VALUE(element)   (gtv_PlcElement.msp_DElement[element])
/*设置第element个D元件值为value*/
#define SET_D_ELEMENT_VALUE(element, value)    (gtv_PlcElement.msp_DElement[element] = value)

/*读R元件*/
#define GET_R_ELEMENT_VALUE(element)   (gtv_PlcElement.msp_RElement[element])
/*设置第element个R元件值为value*/
#define SET_R_ELEMENT_VALUE(element, value)    (gtv_PlcElement.msp_RElement[element] = value)

/*读16位计数器当前值*/
#define GET_C16_CURRENT_VALUE(element)  (gtv_PlcElement.mtv_CElement.msp_16BitValue[element])
/*设置16位计数器当前值*/
#define SET_C16_CURRENT_VALUE(element, value)   (gtv_PlcElement.mtv_CElement.msp_16BitValue[element] = value)

/*读32位计数器当前值*/
#define GET_C32_CURRENT_VALUE(element)  (gtv_PlcElement.mtv_CElement.msp_32BitValue[element - gtp_PlcElementInfo->msv_CElement.msv_16bitCnt])
/*设置32位计数器当前值*/
#define SET_C32_CURRENT_VALUE(element, value)  (gtv_PlcElement.mtv_CElement.msp_32BitValue[element - gtp_PlcElementInfo->msv_CElement.msv_16bitCnt] = value)
#define GET_C32_ADDRESS(element)  (&gtv_PlcElement.mtv_CElement.msp_32BitValue[element - gtp_PlcElementInfo->msv_CElement.msv_16bitCnt])
#define PLUS_C32_CURRENT_VALUE(element)  (gtv_PlcElement.mtv_CElement.msp_32BitValue[element - gtp_PlcElementInfo->msv_CElement.msv_16bitCnt]++)
#define MINUS_C32_CURRENT_VALUE(element)  (gtv_PlcElement.mtv_CElement.msp_32BitValue[element - gtp_PlcElementInfo->msv_CElement.msv_16bitCnt]--)

#define IS_C32_STARTED(element) (gtv_PlcElement.mtv_CElement.mtp_StatusInfo[element].mbv_Started == 1)
#define SET_C32_STARTED(element) (gtv_PlcElement.mtv_CElement.mtp_StatusInfo[element].mbv_Started = 1)
#define SET_C32_STOP(element) (gtv_PlcElement.mtv_CElement.mtp_StatusInfo[element].mbv_Started = 0)

/*读T元件*/
#define GET_T_CURRENT_VALUE(element)   (gtv_PlcElement.mtv_TElement.msp_CurrentValue[element])
/*设置第element个R元件值为value*/
#define SET_T_CURRENT_VALUE(element, value)    (gtv_PlcElement.mtv_TElement.msp_CurrentValue[element] = value)

/*------------------------------------------------------------------------------
*   元件基地址定义
*-----------------------------------------------------------------------------*/
#define X_ELEMENT           (gtv_PlcElement.msp_XElement)
#define Y_ELEMENT           (gtv_PlcElement.msp_YElement)
#define M_ELEMENT           (gtv_PlcElement.msp_MElement)
#define SM_ELEMENT          (gtv_PlcElement.msp_SMElement)
#define S_ELEMENT           (gtv_PlcElement.msp_SElement)
#define LM_ELEMENT          (gtv_PlcElement.msp_LMElement)
/*C位元件*/
#define C_ELEMENT           (gtv_PlcElement.mtv_CElement.msp_BitElement)
/*T位元件*/
#define T_ELEMENT           (gtv_PlcElement.mtv_TElement.msp_BitElement)

/*------------------------------------------------------------------------------
*   元件范围宏定义
*-----------------------------------------------------------------------------*/
#define X_RANG              (gtp_PlcElementInfo->msv_XElementCnt)
#define Y_RANG              (gtp_PlcElementInfo->msv_YElementCnt)
#define M_RANG              (gtp_PlcElementInfo->msv_MElementCnt)
#define LM_RANG             (gtp_PlcElementInfo->msv_LMElementCnt)
#define SM_RANG             (gtp_PlcElementInfo->msv_SMElementCnt)
#define S_RANG              (gtp_PlcElementInfo->msv_SElementCnt)
#define D_RANG              (gtp_PlcElementInfo->msv_DElementCnt)
#define R_RANG              (gtp_PlcElementInfo->msv_RElementCnt)
#define SD_RANG             (gtp_PlcElementInfo->msv_SDElementCnt)
#define V_RANG              (gtp_PlcElementInfo->msv_VElementCnt)
#define Z_RANG              (gtp_PlcElementInfo->msv_ZElementCnt)
#define C_RANG              (gtp_PlcElementInfo->msv_CElement.msv_ElementCnt)
#define C16_RANG            (gtp_PlcElementInfo->msv_CElement.msv_16bitCnt)
#define C32_RANG            (gtp_PlcElementInfo->msv_CElement.msv_32bitCnt)
#define T_RANG              (gtp_PlcElementInfo->msv_TElement.msv_ElementCnt)
/*------------------------------------------------------------------------------
*   变量申明
*-----------------------------------------------------------------------------*/
/*PLC元件信息*/
extern  plc_element_info_st *gtp_PlcElementInfo;

/*PLC各种元件值指针结构体*/
extern  plc_element_st gtv_PlcElement;

/*------------------------------------------------------------------------------
*   函数定义
*-----------------------------------------------------------------------------*/
extern void plc_element_value_init(void);
extern void plc_element_init(void);


#if (INLINE_BIT_ELEMENT == 1)
// Inline functions
/**
  * @brief  设置位元件值
  * @param  BaseAddr 位元件基地址
  *         Element  位元件
  *         Value   设置值
  * @retval None
  */
static inline void plc_set_bit_element_value(unsigned short * BaseAddr, unsigned short Element, unsigned char Value)
{
    unsigned short * lsp_BaseAddr = BaseAddr;

    lsp_BaseAddr += (Element >> 4);

    if(Value) {
        *lsp_BaseAddr |= (0x01 << (Element & 0x0F));
    } else {
        *lsp_BaseAddr &= (~(0x01 << (Element & 0x0F)));
    }
}

/**
  * @brief  获取位元件值
  * @param  BaseAddr 位元件基地址
  *         Element  位元件
  * @retval None
  */
static inline unsigned char plc_get_bit_element_value(unsigned short * BaseAddr, unsigned short Element)
{
    unsigned char lcv_ElementValue;

    lcv_ElementValue = (*(BaseAddr + (Element >> 4)) >> (Element & 0x0F)) & 0x01;

    return lcv_ElementValue;
}

/**
  * @brief  获取指定layer层LM位元件值
  * @param  layer 嵌套层数,小于MAX_SBR_NEST_LAYER
  *         Element  位元件
  * @retval None
  */
static inline unsigned char plc_get_lm_element_value(unsigned char layer, unsigned short Element)
{
    unsigned char lcv_ElementValue;
    unsigned short *lsp_BaseAddr = gtv_PlcElement.msp_LMElement + ((LM_RANG)/16)*layer;

    lcv_ElementValue = (*(lsp_BaseAddr + (Element >> 4)) >> (Element & 0x0F)) & 0x01;

    return lcv_ElementValue;
}

/**
  * @brief  设置指定layer层LM位元件值
  * @param  layer 嵌套层数,小于MAX_SBR_NEST_LAYER
  *         Element  位元件
  * @retval None
  */
static inline void plc_set_lm_element_value(unsigned char layer, unsigned short Element, unsigned char Value)
{
    unsigned short *lsp_BaseAddr = gtv_PlcElement.msp_LMElement + ((LM_RANG)/16)*layer;

    lsp_BaseAddr += (Element >> 4);

    if(Value) {
        *lsp_BaseAddr |= (0x01 << (Element & 0x0F));
    } else {
        *lsp_BaseAddr &= (~(0x01 << (Element & 0x0F)));
    }
}

/**
  * @brief  清除指定layer层LM位元件值
  * @param  layer 嵌套层数,小于MAX_SBR_NEST_LAYER
  * @retval None
  */

static inline void plc_clean_layer_lm_element(unsigned char layer)
{
    unsigned char i;
    unsigned short *lsp_BaseAddr = gtv_PlcElement.msp_LMElement + ((LM_RANG)/16)*layer;

    for(i=0; i<((LM_RANG)/16); i++) {
        lsp_BaseAddr[i] = 0x0;
    }
}
#else
extern void plc_set_bit_element_value(unsigned short * BaseAddr, unsigned short Element, unsigned char Value);
extern unsigned char plc_get_bit_element_value(unsigned short * BaseAddr, unsigned short Element);
extern unsigned char plc_get_lm_element_value(unsigned char layer, unsigned short Element);
extern void plc_set_lm_element_value(unsigned char layer, unsigned short Element, unsigned char Value);
extern void plc_clean_layer_lm_element(unsigned char layer);
#endif
#define SD0      0
#define SD1      1
#define SD2      2
#define SD6      6   //启动原因
#define SD7      7   //PHY失效次数
#define SD30     30  //当前扫描值 (ms)
#define SD31     31  //最小扫描值 (ms)
#define SD32     32  //最大扫描值 (ms)
#define SD34     34  //用户程序超时设定(ms)

#define SD84     84  //基底速度
#define SD87     87  //加减速时间
#define SD90     90  //SD90-97高速脉冲输出当前位置

#define SD180    180 //DHSP，表格比较输出数据的高位
#define SD181    181 //DHSP，表格比较输出数据的低位
#define SD182    182 //DHST，要比较数据的高位
#define SD183    183 //DHST，要比较数据的低位
#define SD184    184 //DHST，当前正在执行的表格记录号


#define SD200    200 //DWORD, 系统从第一次启动开始，总的运行秒数
#define SD202    202 //DWORD, 此次系统开机运行的秒数

#define SD204    204 //WORD,  MCU温度，15秒测量一次；该值除以10得到实际浮点温度值。例如该值是417，则温度值为41.7摄氏度
#define SD205    205 //DWORD, 系统剩余动态内存，每15秒更新一次
#define SD207    207 //DWORD, 固件唯一ID(ota_get_image_id)
#define SD209    209 //WORD,  SW_VERSION 1
#define SD210    210 //WORD,  SW_VERSION 2
#define SD211    211 //WORD,  SW_VERSION 3
#define SD212    212 //WORD,  SW_VERSION 4
#define SD213    213 //DWORD, 系统重启次数

#define SD215    215 //WORD, GPS标志位: 0=未定位，1=已定位
#define SD216    216 //WORD, GPS搜星数
#define SD217    217 //REAL，海拔高度（-9999.9~99999.9米） 
#define SD219    219 //REAL，经度值，(-)ddd.ddddd
#define SD221    221 //REAL，纬度值，(-)dd.ddddd

#define SD223    223 //WORD, 当前联网设备：0=未联网 , 1=Ethernet, 2=4G, 3=Wifi
#define SD224    224 //WORD, 远程维护标志位：0=未连接远程维护服务器，1=已连接远程维护服务器
#define SD225    225 //WORD, MQTT通讯标志位：0=未连接MQTT服务器，    1=已连接MQTT服务器
#define SD226    226 //INT,  4G信号强度
#define SD227    227 //WORD, SIM联网状态
#define SD228    228 //INT, MQTT脚本协议的错误
#define SD229    229 //INT,  MQTT通讯（WAN或4G）错误值或状态
#define SD230    230 //INT,  4G模块当前错误值或状态值

/*
 * 0 = 透传至COM0
 * 1 = 透传至COM1
 * 2 = 透传至COM2
 * 0xFF = 关闭透传
*/
#define SD231    231 //端口透传状态
#define SD232    232 //INT,  4G模块被复位的次数

#define SD233    233 // FEXLINK总线的超时时间
#define SD234    234 // 恒定刷新时间设定值（通过系统块设置）
#define SD235    235 // 刷新一次所有扩展模块（背板总线）所用的时间（ms）
#define SD236    236 // 刷新一次所有从站（网口FEXLINK总线）所用的时间（ms）
#define SD237    237 // 刷新一次所有扩展模块（背板总线）所用的最大时间（ms）
#define SD238    238 // 刷新一次所有从站（网口FEXLINK总线）所用的最大时间（ms）

#define SD240    240 // 错误码（背板总线）
#define SD241    241 // 错误模块地址（背板总线）
#define SD242    242 // 错误内容（背板总线）
#define SD243    243 // 错误码（以太网总线）
#define SD244    244 // 错误模块地址（以太网总线）
#define SD245    245 // 错误内容（以太网总线）

#define SD246    246 // 出现错误帧的累计次数的值(背板总线)
#define SD247    247 // 出现错误帧的累计次数的值(以太网总线)
#define SD248    248 // 258-247 总线错误代码fifo区
#define SD257    257

#define SD258    258 // WAN口协议类型
#define SD259    259 // LAN口协议类型

#define SD260    260 // WAN Mobbus Tcp Server 1
#define SD261    261 // WAN Mobbus Tcp Server 2
#define SD262    262 // WAN Mobbus Tcp Server 3
#define SD263    263 // LAN Mobbus Tcp Server 1
#define SD264    264 // LAN Mobbus Tcp Server 2
#define SD265    265 // LAN Mobbus Tcp Server 3


#define SD280    280

#define SD300    300 // SOEM主站扫描时间
#define SD301    301 // 从站启动等待时间
#define SD302    302 // 从站故障是否停机
#define SD303    303 // PDO重试次数

#define SD305    305 // 出现错误帧的累计次数的值
#define SD306    306 // 出现错误帧的counter的值

#define SD320    320
//SD321-SD384，保存从轴的EtherCAT通信状态
#define SD321    321
#define SD384    384




/* 因COM1使用了MAX14783EESA+这个485芯片，发送、接收
   都得先使能，所以发送完后得有个延时 */
#define SD400    400
#define SD401    401 //信号报警 On状态最小编号



/*
 SD940 ~ SD947对应通道0 ~ 7的AD值
 此AD值是从ADS8368直接得到的值，范围是0 ~ 4096
*/
#define SD940    940
#define SD941    941
#define SD942    942
#define SD943    943
#define SD944    944
#define SD945    945
#define SD946    946
#define SD947    947


/***************************************************************************************/
#define SM8      8   //恒定扫描

#define SM70     70  //Y0输出使能
#define SM80     80  //Y0输出标志
#define SM84     84  //0 = 1 ~ 32767(ms), 1 = 10 ~ 32757(us)

#define SM90     90  //X0上升沿脉冲捕捉
#define SM91     91  //X1上升沿脉冲捕捉
#define SM92     92  //X2上升沿脉冲捕捉
#define SM93     93  //X3上升沿脉冲捕捉

#define SM185    185 //当整个表格记录完成比较时置位（DHST、DHSP）

#define SM236    236 //单相单端X0，1=减计数，0=增计数
#define SM237    237 //单相单端X1，1=减计数，0=增计数
#define SM238    238 //单相单端X2，1=减计数，0=增计数
#define SM239    239 //单相单端X3，1=减计数，0=增计数
#define SM240    240
#define SM241    241
#define SM242    242
#define SM243    243
#define SM244    244
#define SM245    245

#define SM246    246
#define SM247    247
#define SM248    248
#define SM249    249
#define SM250    250
#define SM251    251
#define SM252    252
#define SM253    253
#define SM254    254
#define SM255    255
#define SM256    256
#define SM257    257
#define SM258    258
#define SM259    259
#define SM260    260
#define SM261    261
#define SM262    262
#define SM263    263


/* 以下注释均是在SM为ON时的解释 */
#define SM269    269 // WAN口（ENET）可用
#define SM270    270 // LAN口（ENET2）可用

#define SM271    271 // TCP client1 had connected
#define SM272    272 // TCP client1 is connectting
#define SM273    273 // TCP client1 received some data
#define SM274    274 // TCP client1 sending data

#define SM275    275 // TCP client2 had connected
#define SM276    276 // TCP client2 is connectting
#define SM277    277 // TCP client2 received some data
#define SM278    278 // TCP client2 sending data

#define SM279    279 // TCP client3 had connected
#define SM280    280 // TCP client3 is connectting
#define SM281    281 // TCP client3 received some data
#define SM282    282 // TCP client3 sending data

#if 0
#define SM277    277 // TCP client4 had connected
#define SM278    278 // TCP client4 is connectting

#define SM279    279 // TCP client5 had connected
#define SM280    280 // TCP client5 is connectting

#define SM281    281 // TCP client6 had connected
#define SM282    282 // TCP client6 is connectting

#define SM283    283 // TCP client7 had connected
#define SM284    284 // TCP client7 is connectting

#define SM285    285 // TCP client8 had connected
#define SM286    286 // TCP client8 is connectting

#define SM287    287 // TCP client9 had connected
#define SM288    288 // TCP client9 is connectting

#define SM289    289 // TCP client10 had connected
#define SM290    290 // TCP client10 is connectting
#endif
#define SM400    400 //信号报警器有效
#define SM401    401 //信号报警器动作


/***************************************************************************************/
#define HCOUNTER236     236 // X0增减计数
#define HCOUNTER237     237 // X1增减计数
#define HCOUNTER238     238 // X2增减计数
#define HCOUNTER239     239 // X3增减计数
#define HCOUNTER240     240 // X4增减计数
#define HCOUNTER241     241 // X5增减计数

#define HCOUNTER244     244 // X0增减计数，X2复位
#define HCOUNTER245     245 // X3增减计数，X5复位
#define HCOUNTER246     246 // X0增减计数，X2复位，X6启动
#define HCOUNTER247     247 // X3增减计数，X5复位，X7启动

#define HCOUNTER248     248 // X0增计数，X1减计数
#define HCOUNTER249     249 // X2增计数，X3减计数
#define HCOUNTER250     250 // X4增计数，X5减计数

#define HCOUNTER252     252 // X0增计数，X1减计数，X2复位
#define HCOUNTER253     253 // X3增计数，X4减计数，X5复位
#define HCOUNTER254     254 // X0增计数，X1减计数，X2复位，X6启动
#define HCOUNTER255     255 // X3增计数，X4减计数，X5复位，X7启动

#define HCOUNTER256     256 // X0：A相，X1：B相
#define HCOUNTER257     257 // X2：A相，X3：B相
#define HCOUNTER258     258 // X4：A相，X5：B相

#define HCOUNTER260     260 // X0：A相，X1：B相，X2复位
#define HCOUNTER261     261 // X3：A相，X4：B相，X5复位
#define HCOUNTER262     262 // X0：A相，X1：B相，X2复位，X6启动
#define HCOUNTER263     263 // X3：A相，X4：B相，X5复位，X7启动


#endif /*__PLC_ELEMENT_H*/

