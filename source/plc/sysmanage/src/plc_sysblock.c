/**
  ******************************************************************************
  * @file    plc_sysblock.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   PLC系统块解析
  ******************************************************************************
  */

#include "FreeRTOS.h"
#include "fsl_debug_console.h"

#include "plc_variable.h"
#include "plc_commonfunc.h"
#include "plc_executer.h"
#include "plc_element.h"
#include "plc_sysinit.h"
#include "plc_sysblock.h"
#include "bsp_dct.h"
#include "mb.h"
#include "mb_maptable.h"
#include "plc_interrupt.h"
#include "kalyke_tool.h"
#include "daisy_task.h"
#include "ether_cat_task.h"
#include "bsp_iwdg.h"

static const char *TAG = "plc_sysblock";

struct tag_InSysCfgAdDa gt_InAdDaCfg;
modbus_tcp_config_st gModbusTcpConfig;
bus_config_st        gBusConfig;
soem_config_st       gSoemConfig;
unsigned char        gPlcRunAs;  //1:master,2:smartdevice
unnion_left_modules  gLeftModules;

/**
  * @brief  Modlink指令表表头初始化
  * @param  None
  * @retval None
  */
void plc_parse_sysblk_modlink_head_init()
{
    LOGV("plc_sysblock", "Enter %s()", __func__);
    //hexdump(gtv_ModlinkSheetHead, sizeof(gtv_ModlinkSheetHead));
    LOGD("plc_sysblock", "gtv_ModlinkSheetHead[0].mtp_InsListPtr = 0x%08X", gtv_ModlinkSheetHead[0].mtp_InsListPtr);

    unsigned char i, j;

    for(i = 0; i < MAX_MODLINK_SHEET_NUM; i++)
    {
        for(j = 0; j < MAX_MODLINK_SHEET_NAME_LEN; j++)
        {
            gtv_ModlinkSheetHead[i].mcv_TableName[j] = 0;
        }

        gtv_ModlinkSheetHead[i].mcv_TableType = MDL_TYPE_NOT_USE;

        if(gtv_ModlinkSheetHead[i].mtp_InsListPtr != NULL)
        {
            vPortFree(gtv_ModlinkSheetHead[i].mtp_InsListPtr);
            gtv_ModlinkSheetHead[i].mtp_InsListPtr = NULL;
        }

        gtv_ModlinkSheetHead[i].mcv_InsListNum = 0;

        gtv_ModlinkSheetHead[i].msv_ListIndex = 0;

        if(!gtv_ModlinkSheetHead[i].msp_RecvBuff)
        {
            gtv_ModlinkSheetHead[i].msp_RecvBuff = (unsigned short *)pvPortMalloc(sizeof(unsigned short) * 32);
            configASSERT(gtv_ModlinkSheetHead[i].msp_RecvBuff != NULL);
        }
    }
}

/**
  * @brief  Modlink 系统块配置解析
  * @param  None
  * @retval None
  */
unsigned char plc_parse_sysblk_modlink_config(unsigned short *lsp_ConfigPtr)
{
    LOGV("plc_sysblock", "Enter %s()", __func__);
    plc_modlink_head_st *ltp_Head;
    unsigned char lcv_SheetNum;
    unsigned int i, j;
    unsigned char *lcp_Temp;
    unsigned char lcv_Ret;
    unsigned short lsv_MbRwAddr;
    unsigned char lcv_ElementType;
    unsigned short lsv_ElementAddr;

    /*链表头数据初始化*/
    plc_parse_sysblk_modlink_head_init();

    lcv_SheetNum = GET_PU16_DATA(lsp_ConfigPtr + 2);

    lcp_Temp = (unsigned char *)(lsp_ConfigPtr + 3);
    LOGD("plc_sysblock", "lcv_SheetNum = %u, lcp_Temp = 0x%08X", lcv_SheetNum, lcp_Temp);
    for(i = 0; i < lcv_SheetNum; i++)
    {

        ltp_Head = &gtv_ModlinkSheetHead[i];

        /*指令列表名称解析*/
        for(j = 0; j < MAX_MODLINK_SHEET_NAME_LEN; j++)
        {
            ltp_Head->mcv_TableName[j] = *lcp_Temp;
            lcp_Temp++;
        }

        /*列表类型解析*/
        ltp_Head->mcv_TableType = GET_BIGPU16_DATA(lcp_Temp);
        lcp_Temp += 2;

        /*指令列表项数目*/
        ltp_Head->mcv_InsListNum = GET_PU16_DATA(lcp_Temp);
        lcp_Temp += 2;

        /*分配指令列表内存*/
        ltp_Head->mtp_InsListPtr = (plc_modlink_ins_item_st *)pvPortMalloc(sizeof(plc_modlink_ins_item_st) * ltp_Head->mcv_InsListNum);
        if (ltp_Head->mtp_InsListPtr == NULL)
        {
            return pdFAIL;
        }
        configASSERT(ltp_Head->mtp_InsListPtr != NULL);

        for(j = 0; j < ltp_Head->mcv_InsListNum; j++)
        {
            /*跳过序号*/
            lcp_Temp += 2;

            ltp_Head->mtp_InsListPtr[j].msv_StopFlagIndex = GET_PU16_DATA(lcp_Temp);
            lcp_Temp += 2;

            ltp_Head->mtp_InsListPtr[j].mcv_SlaveAddr = GET_PU8_DATA(lcp_Temp);
            lcp_Temp ++;

            ltp_Head->mtp_InsListPtr[j].mcv_MbFunc = GET_PU8_DATA(lcp_Temp);
            lcp_Temp ++;

            ltp_Head->mtp_InsListPtr[j].msv_StartAddr = GET_PU16_DATA(lcp_Temp);
            lcp_Temp += 2;

            ltp_Head->mtp_InsListPtr[j].msv_ElementCnt = GET_PU16_DATA(lcp_Temp);
            lcp_Temp += 2;

            /*解析读、写地址*/
            lsv_MbRwAddr = GET_PU16_DATA(lcp_Temp);
            lcp_Temp += 2;

            lcv_Ret = mb_slave_convert_element_info(MB_WORD_ELEMENT, lsv_MbRwAddr, &lcv_ElementType, &lsv_ElementAddr);
            if(lcv_Ret != pdPASS)
            {
                return lcv_Ret;
            }

            switch(lcv_ElementType)
            {
            case MB_WORD_D:
                ltp_Head->mtp_InsListPtr[j].msp_ResultData = &GET_D_ELEMENT_VALUE(lsv_ElementAddr);
                ltp_Head->mtp_InsListPtr[j].msp_MaxResultData = &GET_D_ELEMENT_VALUE(D_RANG - 1);
                break;

            case MB_WORD_R:
                ltp_Head->mtp_InsListPtr[j].msp_ResultData = &GET_R_ELEMENT_VALUE(lsv_ElementAddr);
                ltp_Head->mtp_InsListPtr[j].msp_MaxResultData = &GET_R_ELEMENT_VALUE(R_RANG - 1);
                break;

            default:
                return pdFAIL;
            }

            /*解析执行结果存储地址*/
            lsv_MbRwAddr = GET_PU16_DATA(lcp_Temp);
            lcp_Temp += 2;

            lcv_Ret = mb_slave_convert_element_info(MB_WORD_ELEMENT, lsv_MbRwAddr, &lcv_ElementType, &lsv_ElementAddr);
            if(lcv_Ret != pdPASS)
            {
                return lcv_Ret;
            }

            switch(lcv_ElementType)
            {
            case MB_WORD_D:
                ltp_Head->mtp_InsListPtr[j].msp_ExecResult = &GET_D_ELEMENT_VALUE(lsv_ElementAddr);
                break;

            case MB_WORD_R:
                ltp_Head->mtp_InsListPtr[j].msp_ExecResult = &GET_R_ELEMENT_VALUE(lsv_ElementAddr);
                break;

            default:
                return pdFAIL;
            }

            ltp_Head->mtp_InsListPtr[j].mcv_IsExec = 0;
        }

    }

    LOGV(TAG, "Let us dump 'gtv_ModlinkSheetHead':");
    hexdump(gtv_ModlinkSheetHead, sizeof(gtv_ModlinkSheetHead));

    return pdPASS;
}

/**
  * @brief  串口配置信息解析
  * @param  None
  * @retval None
  */
unsigned char plc_parse_sysblk_uart_config(unsigned char lcv_UartPort, unsigned short *lsp_ConfigPtr)
{
    PRINTF("Enter %s, lcv_UartPort = %d, mcv_SupportUartNum = %u\r\n", __func__, lcv_UartPort, gtv_DeviceConfigTable.mtv_UartPort.mcv_SupportUartNum);
    uart_port_info_st *ltp_UartPort;
    uart_sysblk_config_st *ltp_UartConfig;
    const unsigned long cllv_BaudRate[]      = {38400, 19200, 9600, 4800, 2400, 1200, 57600, 115200};
    const unsigned short clsv_RxT35TimeOut[] = {3,     4,     6,    12,   24,   48,   2,     2};
    //const unsigned short clsv_RxT35TimeOut[] = {100,     100,  100,  100,  100,  100,   100,    10};
    const unsigned short clsv_TxTimeOut[] = {1000, 1000, 1000, 1400, 2000, 3000, 1000, 1000};
    //const unsigned short clsv_TxTimeOut[]    = {2000,   2000, 2000, 2400, 2400, 3000,  2000,   2000};
    if(lcv_UartPort >= gtv_DeviceConfigTable.mtv_UartPort.mcv_SupportUartNum)
    {
        return pdFAIL;
    }

    ltp_UartPort = &gtp_UartPort[lcv_UartPort];

    ltp_UartConfig = (uart_sysblk_config_st *)(lsp_ConfigPtr + 2);

    PRINTF("ltp_UartConfig->ProtocolType = %d\r\n", ltp_UartConfig->ProtocolType);
    PRINTF("ltp_UartConfig->ModebusMode = %d\r\n", ltp_UartConfig->ModebusMode);
    switch(ltp_UartConfig->ProtocolType)
    {

    case 1:
        ltp_UartPort->mcv_Mode = UART_TYPE_FREE_PORT;
        ltp_UartPort->mtv_ModeInfo.FreePort.msv_Flag = (*(unsigned short *)ltp_UartConfig & 0x0780) >> 7;

        ltp_UartPort->mtv_ModeInfo.FreePort.msv_StartChar = GET_PU16_DATA(lsp_ConfigPtr + 6);
        ltp_UartPort->mtv_ModeInfo.FreePort.msv_EndChar = GET_PU16_DATA(lsp_ConfigPtr + 7);
        ltp_UartPort->mtv_ModeInfo.FreePort.msv_WordTimeout = GET_PU16_DATA(lsp_ConfigPtr + 8);
        ltp_UartPort->mtv_ModeInfo.FreePort.msv_FrameTimeout = GET_PU16_DATA(lsp_ConfigPtr + 9);

        if(ltp_UartPort->pTimerInitFunc != NULL)
        {
            ltp_UartPort->pTimerInitFunc(ltp_UartPort->mtv_ModeInfo.FreePort.msv_WordTimeout,
                                         ltp_UartPort->mtv_ModeInfo.FreePort.msv_FrameTimeout);
        }
        break;

    case 2:
        if(ltp_UartConfig->ModebusMode)
        {
            ltp_UartPort->mcv_Mode = UART_TYPE_MB_MASTER;
            PRINTF("ltp_UartConfig->ModbusTransMode = %d\r\n", ltp_UartConfig->ModbusTransMode);
            if(ltp_UartConfig->ModbusTransMode)
                ltp_UartPort->mtv_ModeInfo.MasterPort.mcv_TransMode = MB_ASCII;
            else
                ltp_UartPort->mtv_ModeInfo.MasterPort.mcv_TransMode = MB_RTU;

            ltp_UartPort->mtv_ModeInfo.MasterPort.msv_RxT35Time = clsv_RxT35TimeOut[ltp_UartConfig->BaudRate];
            ltp_UartPort->mtv_ModeInfo.MasterPort.msv_TxTimeOut = clsv_TxTimeOut[ltp_UartConfig->BaudRate];
            ltp_UartPort->mtv_ModeInfo.MasterPort.mcv_RetryNum = GET_PU16_DATA(lsp_ConfigPtr + 5);

            if(ltp_UartPort->pTimerInitFunc != NULL)
            {
                ltp_UartPort->pTimerInitFunc(ltp_UartPort->mtv_ModeInfo.MasterPort.msv_RxT35Time,
                                             ltp_UartPort->mtv_ModeInfo.MasterPort.msv_TxTimeOut);
            }
        }
        else
        {
            ltp_UartPort->mcv_Mode = UART_TYPE_MB_SLAVE;

            PRINTF("ltp_UartConfig->ModbusTransMode = %d\r\n", ltp_UartConfig->ModbusTransMode);
            if(ltp_UartConfig->ModbusTransMode)
                ltp_UartPort->mtv_ModeInfo.SlavePort.mcv_TransMode = MB_ASCII;
            else
                ltp_UartPort->mtv_ModeInfo.SlavePort.mcv_TransMode = MB_RTU;

            ltp_UartPort->mtv_ModeInfo.SlavePort.msv_RxT35Time = clsv_RxT35TimeOut[ltp_UartConfig->BaudRate];
            ltp_UartPort->mtv_ModeInfo.SlavePort.msv_TxTimeOut = clsv_TxTimeOut[ltp_UartConfig->BaudRate];

            if(ltp_UartPort->pTimerInitFunc != NULL)
            {
                ltp_UartPort->pTimerInitFunc(ltp_UartPort->mtv_ModeInfo.SlavePort.msv_RxT35Time,
                                             ltp_UartPort->mtv_ModeInfo.SlavePort.msv_TxTimeOut);
            }
        }
        break;

    }

    /*波特率*/
    ltp_UartPort->mtv_PortPara.mlv_BaudRate = cllv_BaudRate[ltp_UartConfig->BaudRate];

    /*奇偶校验位*/
    if(ltp_UartConfig->ParityEnable)
    {
        if(ltp_UartConfig->Parity)
            ltp_UartPort->mtv_PortPara.mcv_Parity = UART_PARITY_EVEN;
        else
            ltp_UartPort->mtv_PortPara.mcv_Parity = UART_PARITY_ODD;
    }
    else
    {
        ltp_UartPort->mtv_PortPara.mcv_Parity = UART_PARITY_NO;
    }

    /*停止位*/
    if(ltp_UartConfig->StopBits)
        ltp_UartPort->mtv_PortPara.mcv_StopBits = UART_STB_2;
    else
        ltp_UartPort->mtv_PortPara.mcv_StopBits = UART_STB_1;

    /*数据长度*/
    if(ltp_UartConfig->WordLength)
        ltp_UartPort->mtv_PortPara.mcv_WordLength = 7;
    else
        ltp_UartPort->mtv_PortPara.mcv_WordLength = 8;

    /*保存串口配置字*/
    //SET_UART_SD_VALUE(lcv_UartPort, UART_SD_MODE_CONFIG, *(unsigned short *)(ltp_UartConfig) && 0x7FF);
    SET_UART_SD_VALUE(lcv_UartPort, UART_SD_MODE_CONFIG, *(unsigned short *)(ltp_UartConfig));
    /*保存本设备站号*/
    SET_UART_SD_VALUE(lcv_UartPort, UART_SD_MODBUS_ID, GET_PU16_DATA(lsp_ConfigPtr + 3));
    gtp_ModbusSlaveDiagInfo[lcv_UartPort].mcv_SlaveId = GET_PU16_DATA(lsp_ConfigPtr + 3);
    PRINTF("%s: slaveId = %d\r\n", __func__, gtp_ModbusSlaveDiagInfo[lcv_UartPort].mcv_SlaveId);
    if (lcv_UartPort == 0)
    {
        gtp_ModbusSlaveDiagInfo[MB_SENDER_TCP].mcv_SlaveId = gtp_ModbusSlaveDiagInfo[lcv_UartPort].mcv_SlaveId;
        gtp_ModbusSlaveDiagInfo[MB_SENDER_USB].mcv_SlaveId = gtp_ModbusSlaveDiagInfo[lcv_UartPort].mcv_SlaveId;
    }
    /*设置标志位*/
    SET_UART_SM_FLAG(lcv_UartPort, UART_SM_IDLE);
    /*接收、发送完成标志*/
    RST_UART_SM_FLAG(lcv_UartPort, UART_SM_RX_FINISH);
    RST_UART_SM_FLAG(lcv_UartPort, UART_SM_TX_FINISH);
    /*自由口发送，接收使能标志*/
    RST_UART_SM_FLAG(lcv_UartPort, UART_SM_FREE_PORT_RX_EN);
    RST_UART_SM_FLAG(lcv_UartPort, UART_SM_FREE_PORT_TX_EN);
    /*MODBUS通讯完成、错误标志*/
    RST_UART_SM_FLAG(lcv_UartPort, UART_SM_MODBUS_FINISH);
    RST_UART_SM_FLAG(lcv_UartPort, UART_SM_MODBUS_ERROR);

    if (lcv_UartPort == 0 || lcv_UartPort == 1 || lcv_UartPort == 2) // 暂时先只初始化串口0,1,2
    {
        /*串口初始化*/
        if(gtp_UartPort[lcv_UartPort].pConfigFunc != NULL)
        {
            gtp_UartPort[lcv_UartPort].pConfigFunc(ltp_UartPort->mtv_PortPara.mlv_BaudRate,
                                                   ltp_UartPort->mtv_PortPara.mcv_Parity,
                                                   ltp_UartPort->mtv_PortPara.mcv_WordLength,
                                                   ltp_UartPort->mtv_PortPara.mcv_StopBits);
        }
    }

    return pdPASS;
}

/**
  * @brief  掉电保持两组数据去重叠
  * @param  None
  * @retval None
  */
void plc_parse_sysblk_plsd_data_processing(unsigned char *lcp_Data, unsigned char lcv_Element)
{
    unsigned short lsv_Start1, lsv_Start2, lsv_Len1, lsv_Len2;

    lsv_Start1 = GET_PU16_DATA(lcp_Data);
    lsv_Len1   = GET_PU16_DATA(lcp_Data + 2);
    lsv_Start2 = GET_PU16_DATA(lcp_Data + 4);
    lsv_Len2   = GET_PU16_DATA(lcp_Data + 6);
    LOGD("sysblock", "lsv_Start1 = %u, lsv_Len1 = %u, lsv_Start2 = %u, lsv_Len2 = %u", lsv_Start1, lsv_Len1, lsv_Start2, lsv_Len2);
#if 0
    if(lsv_Start1 < lsv_Start2)
    {
        if((lsv_Start1 + lsv_Len1 - 1) < lsv_Start2)
        {
            gtp_PowerLoseSaveDataInfo[0].mtv_Element[lcv_Element].msv_StartElement = lsv_Start1;
            gtp_PowerLoseSaveDataInfo[0].mtv_Element[lcv_Element].msv_Length = lsv_Len1;
            gtp_PowerLoseSaveDataInfo[1].mtv_Element[lcv_Element].msv_StartElement = lsv_Start2;
            gtp_PowerLoseSaveDataInfo[1].mtv_Element[lcv_Element].msv_Length = lsv_Len2;
        }
        else
        {
            /*>=*/
            if((lsv_Start1 + lsv_Len1 - 1) < (lsv_Start2 + lsv_Len2 - 1))
            {
                gtp_PowerLoseSaveDataInfo[0].mtv_Element[lcv_Element].msv_StartElement = lsv_Start1;
                gtp_PowerLoseSaveDataInfo[0].mtv_Element[lcv_Element].msv_Length = (lsv_Start2 + lsv_Len2 - 1) - lsv_Start1 + 1;
                gtp_PowerLoseSaveDataInfo[1].mtv_Element[lcv_Element].msv_StartElement = 0;
                gtp_PowerLoseSaveDataInfo[1].mtv_Element[lcv_Element].msv_Length = 0;
            }
            else
            {
                gtp_PowerLoseSaveDataInfo[0].mtv_Element[lcv_Element].msv_StartElement = lsv_Start1;
                gtp_PowerLoseSaveDataInfo[0].mtv_Element[lcv_Element].msv_Length = lsv_Len1;
                gtp_PowerLoseSaveDataInfo[1].mtv_Element[lcv_Element].msv_StartElement = 0;
                gtp_PowerLoseSaveDataInfo[1].mtv_Element[lcv_Element].msv_Length = 0;
            }
        }
    }
    else
    {
        if((lsv_Start2 + lsv_Len2 - 1) < lsv_Start1)
        {
            gtp_PowerLoseSaveDataInfo[1].mtv_Element[lcv_Element].msv_StartElement = lsv_Start1;
            gtp_PowerLoseSaveDataInfo[1].mtv_Element[lcv_Element].msv_Length = lsv_Len1;
            gtp_PowerLoseSaveDataInfo[0].mtv_Element[lcv_Element].msv_StartElement = lsv_Start2;
            gtp_PowerLoseSaveDataInfo[0].mtv_Element[lcv_Element].msv_Length = lsv_Len2;
        }
        else
        {
            if((lsv_Start2 + lsv_Len2 - 1) < (lsv_Start1 + lsv_Len1 - 1))
            {
                gtp_PowerLoseSaveDataInfo[0].mtv_Element[lcv_Element].msv_StartElement = lsv_Start2;
                gtp_PowerLoseSaveDataInfo[0].mtv_Element[lcv_Element].msv_Length = (lsv_Start1 + lsv_Len1 - 1) - lsv_Start2 + 1;
                gtp_PowerLoseSaveDataInfo[1].mtv_Element[lcv_Element].msv_StartElement = 0;
                gtp_PowerLoseSaveDataInfo[1].mtv_Element[lcv_Element].msv_Length = 0;
            }
            else
            {
                gtp_PowerLoseSaveDataInfo[0].mtv_Element[lcv_Element].msv_StartElement = lsv_Start2;
                gtp_PowerLoseSaveDataInfo[0].mtv_Element[lcv_Element].msv_Length = lsv_Len2;
                gtp_PowerLoseSaveDataInfo[1].mtv_Element[lcv_Element].msv_StartElement = 0;
                gtp_PowerLoseSaveDataInfo[1].mtv_Element[lcv_Element].msv_Length = 0;
            }
        }
    }
#endif
    gtp_PowerLoseSaveDataInfo.group1[lcv_Element].msv_StartElement = lsv_Start1;
    gtp_PowerLoseSaveDataInfo.group1[lcv_Element].msv_Length = lsv_Len1;
    gtp_PowerLoseSaveDataInfo.group2[lcv_Element].msv_StartElement = lsv_Start2;
    gtp_PowerLoseSaveDataInfo.group2[lcv_Element].msv_Length = lsv_Len2;
}

/**
  * @brief  掉电保持数据解析
  * @param  None
  * @retval None
01 AA 2C 00
01 00 02 00  M 1  2
0A 00 06 00  M 10 6
2D 00 03 00  S 45 3
32 00 07 00  S 50 7
0C 00 04 00  C element
C8 00 08 00
65 00 05 00  T element
6E 00 0A 00
7B 00 01 00  D element
00 00 09 00
*/
void plc_parse_sysblk_plsd_config(unsigned char *lcp_sysConfig)
{
    unsigned char *lcp_Config = lcp_sysConfig;
    unsigned char i;

    if(!gtv_DeviceConfigTable.mcv_IsSupportPlsd)
    {
        return;
    }

    lcp_Config += 4;
    /*系统块配置解析*/
    for(i = PLSD_M; i < PLSD_MAX; i++)
    {
        plc_parse_sysblk_plsd_data_processing(lcp_Config, i);
        lcp_Config += 8;
    }
}

/*
19 AA 9C 00
FF 7F //错误状态地址。2个字节(D元件或者R元件，最高位为0：表示D元件，最高位为1：表示R元件，0x7FFF表示没有。)

64 00 // 转换速度
08 00 // AD通道数
00 00 // DA通道数

// AD第1通道配置(共18个字节)
01 00 // 模式选择（0:通道关闭  1：-10~10V    2：-5~5V   3：-2.5~2.5V   4：0~10V  5：0~5V     6：4~20mA    7：0~20mA   8：-20~20mA）
00 00 // 滤波方式(0：递推平均滤波 1：算术平均滤波 2：限幅滤波 3：中位值滤波 4：中位值平均滤波 5：限幅平均滤波 6：一阶滞后滤波 7：加权递推平均滤波)
08 00 // 滤波参数1
00 00 // 滤波参数2
08 00 // 平均采样次数
00 08 // 零点数字量
00 10 // 数字量最大值
58 1B // 平均值地址（D元件或者R元件，见错误状态地址）
5A 1B // 当前值地址（D元件或者R元件，见错误状态地址）

// AD第2通道配置(共18个字节)
00 00 00 00 08 00 00 00 08 00 00 00 D0 07 FF 7F
FF 7F
// AD第3通道配置(共18个字节)
00 00 00 00 08 00 00 00 08 00 00 00 D0 07 FF 7F
FF 7F
// AD第4通道配置(共18个字节)
00 00 00 00 08 00 00 00 08 00 00 00 D0 07 FF 7F
FF 7F
// AD第5通道配置(共18个字节)
01 00 00 00 08 00 00 00 08 00 00 08 00 10 70 17
72 17
// AD第6通道配置(共18个字节)
00 00 00 00 08 00 00 00 08 00 00 00 D0 07 FF 7F
FF 7F
// AD第7通道配置(共18个字节)
00 00 00 00 08 00 00 00 08 00 00 00 D0 07 FF 7F
FF 7F
// AD第8通道配置(共18个字节)
00 00 00 00 08 00 00 00 08 00 00 00 D0 07 FF 7F
FF 7F

7C 0D 55 55
*/
static uint16_t parse_AD(unsigned char *ptr)
{
    LOGV(TAG, "Enter %s()", __func__);
    uint16_t length = GET_PU16_DATA(ptr + 2);

    ptr += 4;
    gt_InAdDaCfg.errState = GET_PU16_DATA(ptr);
    ptr += 2;
    gt_InAdDaCfg.adSpeed = GET_PU16_DATA(ptr);
    ptr += 2;
    gt_InAdDaCfg.adNum = GET_PU16_DATA(ptr);
    ptr += 2;
    gt_InAdDaCfg.daNum = GET_PU16_DATA(ptr);

    gt_InAdDaCfg.AdFsm = AD_FsmNonUse;
    for (int i = 0; i < gt_InAdDaCfg.adNum; i++)
    {
        ptr += 2;
        gt_InAdDaCfg.ad[i].modSlc = GET_PU16_DATA(ptr);
        ptr += 2;
        gt_InAdDaCfg.ad[i].filteringMethod = GET_PU16_DATA(ptr);
        ptr += 2;
        gt_InAdDaCfg.ad[i].filteringParm1 = GET_PU16_DATA(ptr);
        ptr += 2;
        gt_InAdDaCfg.ad[i].filteringParm2 = GET_PU16_DATA(ptr);
        ptr += 2;
        gt_InAdDaCfg.ad[i].sampleTick = GET_PU16_DATA(ptr);
        ptr += 2;
        gt_InAdDaCfg.ad[i].zeroDta = GET_PU16_DATA(ptr);
        ptr += 2;
        gt_InAdDaCfg.ad[i].maxDta = GET_PU16_DATA(ptr);
        ptr += 2;
        gt_InAdDaCfg.ad[i].pAverageU16 = GET_PU16_DATA(ptr);
        ptr += 2;
        gt_InAdDaCfg.ad[i].pCurU16 = GET_PU16_DATA(ptr);
        if(gt_InAdDaCfg.ad[i].modSlc != sysAdCfgNonUse)
        {
            gt_InAdDaCfg.AdFsm = AD_FsmIsInit;
        }
    }

    gt_InAdDaCfg.DaFsm = AD_FsmNonUse;
    for (int i = 0; i < gt_InAdDaCfg.daNum; i++)
    {
        ptr += 2;
        gt_InAdDaCfg.da[i].modSlc = GET_PU16_DATA(ptr);
        ptr += 2;
        gt_InAdDaCfg.da[i].pOutDtaU16 = GET_PU16_DATA(ptr);
        ptr += 2;
        gt_InAdDaCfg.da[i].pzeroDtaU16 = GET_PU16_DATA(ptr);
        ptr += 2;
        gt_InAdDaCfg.da[i].pMaxDtaU16 = GET_PU16_DATA(ptr);
        if(gt_InAdDaCfg.da[i].modSlc != sysDaCfgNonUse)
        {
            gt_InAdDaCfg.DaFsm = DA_FsmIsInit;
        }
    }

    return length;
}

static uint16_t parse_modbus_tcp_client(unsigned char *ptr)
{
    uint16_t length = GET_PU16_DATA(ptr + 2);

    memset(&gModbusTcpConfig, 0, sizeof(gModbusTcpConfig));

    ptr += 4;
    gModbusTcpConfig.count = GET_PU16_DATA(ptr);

    for (uint16_t i = 0; i < gModbusTcpConfig.count; i++)
    {
        ptr += 2;
        memcpy(gModbusTcpConfig.item[i].name, ptr, 8);
        ptr += 8;
        gModbusTcpConfig.item[i].connectMark = *ptr++;
        gModbusTcpConfig.item[i].connectType = *ptr++;
        IP4_ADDR(&gModbusTcpConfig.item[i].ipTarget, *ptr, *(ptr + 1), *(ptr + 2), *(ptr + 3));
        ptr += 4;
        gModbusTcpConfig.item[i].portTarget = GET_PU16_DATA(ptr);
        ptr += 2;
        gModbusTcpConfig.item[i].portLocal = GET_PU16_DATA(ptr);
    }
    return length;
}

/*
 1B  01020008 0000   (Maping to 1060）
 1A  01000001 0001   (Maping to 104f）
 1A  01010008 0002   (Maping to 1050）
*/
static uint8_t * parse_ExtPDO(uint8_t *ptr)
{
    LOGV(TAG, "Enter %s()", __func__);
    gBusConfig.pPDO1B_Ext = pvPortMalloc(sizeof(pdo_st) * gBusConfig.nPdoCountExt);
    gBusConfig.pPDO1A_Ext = pvPortMalloc(sizeof(pdo_st) * gBusConfig.nPdoCountExt);
    LOGV(TAG, "pPDO1B_Ext = 0x%08X, pPDO1A_Ext = 0x%08X", gBusConfig.pPDO1B_Ext, gBusConfig.pPDO1A_Ext);
    pdo_st *pdo = NULL;
    uint16_t ia = 0;
    uint16_t ib = 0;
    for (uint16_t i = 0; i < gBusConfig.nPdoCountExt; i++)
    {
        if (*ptr == 0x1B)
        {
            pdo = gBusConfig.pPDO1B_Ext + ib++;
            gBusConfig.pdo1BCountExt++;
        }
        else
        {
            pdo = gBusConfig.pPDO1A_Ext + ia++;
            gBusConfig.pdo1ACountExt++;
        }
        pdo->sType = *ptr;
        ptr++; // 08000201 0000 -- PDO
        pdo->eAddr = *(ptr + 1) << 8 | *ptr;
        ptr += 2; //  0201 0000 -- PDO
        pdo->eType = (daisy_element_type_e)*ptr;
        ptr++; //       01 0000 -- PDO
        pdo->len = *ptr;
        ptr++; //          0000 -- PDO
        pdo->offset = *(ptr + 1) << 8 | *ptr;
        ptr += 2; // 指向了下一个pdo开始处
        LOGI(TAG, "pdo[%u].sType = 0x%02X", i, pdo->sType);
        LOGI(TAG, "pdo[%u].len = %u", i, pdo->len);
        LOGI(TAG, "pdo[%u].eType = %u", i, pdo->eType);
        LOGI(TAG, "pdo[%u].eAddr = 0x%04X", i, pdo->eAddr);
        LOGD(TAG, "pdo[%u].offset = 0x%04X", i, pdo->offset);
    }
    LOGI(TAG, "pdo1BCountExt = %u, pdo1ACountExt = %u", gBusConfig.pdo1BCountExt, gBusConfig.pdo1ACountExt);
#if (LOG_OPEN == 1)
    for (uint16_t i = 0; i < gBusConfig.pdo1ACountExt; i++)
    {
        LOGV(TAG, "gBusConfig.pPDO1A_Ext[%u].sType = 0x%02X", i, gBusConfig.pPDO1A_Ext[i].sType);
        LOGV(TAG, "gBusConfig.pPDO1A_Ext[%u].len = %u", i, gBusConfig.pPDO1A_Ext[i].len);
        LOGV(TAG, "gBusConfig.pPDO1A_Ext[%u].eType = %u", i, gBusConfig.pPDO1A_Ext[i].eType);
        LOGV(TAG, "gBusConfig.pPDO1A_Ext[%u].eAddr = 0x%04X", i, gBusConfig.pPDO1A_Ext[i].eAddr);
        LOGD(TAG, "gBusConfig.pPDO1A_Ext[%u].offset = 0x%04X", i, gBusConfig.pPDO1A_Ext[i].offset);
    }
    for (uint16_t i = 0; i < gBusConfig.pdo1BCountExt; i++)
    {
        LOGV(TAG, "gBusConfig.pPDO1B_Ext[%u].sType = 0x%02X", i, gBusConfig.pPDO1B_Ext[i].sType);
        LOGV(TAG, "gBusConfig.pPDO1B_Ext[%u].len = %u", i, gBusConfig.pPDO1B_Ext[i].len);
        LOGV(TAG, "gBusConfig.pPDO1B_Ext[%u].eType = %u", i, gBusConfig.pPDO1B_Ext[i].eType);
        LOGV(TAG, "gBusConfig.pPDO1B_Ext[%u].eAddr = 0x%04X", i, gBusConfig.pPDO1B_Ext[i].eAddr);
        LOGD(TAG, "gBusConfig.pPDO1B_Ext[%u].offset = 0x%04X", i, gBusConfig.pPDO1B_Ext[i].offset);
    }
#endif
    return ptr;
}

static uint8_t * parse_SlavePDO(uint8_t *ptr)
{
    LOGV(TAG, "Enter %s()", __func__);
    gBusConfig.pPDO1B = pvPortMalloc(sizeof(pdo_st) * gBusConfig.nPdoCount);
    gBusConfig.pPDO1A = pvPortMalloc(sizeof(pdo_st) * gBusConfig.nPdoCount);
    LOGV(TAG, "pPDO1B = 0x%08X, pPDO1A = 0x%08X", gBusConfig.pPDO1B, gBusConfig.pPDO1A);
    pdo_st *pdo = NULL;//gBusConfig.pPDO;
    uint16_t ia = 0;
    uint16_t ib = 0;
    for (uint16_t i = 0; i < gBusConfig.nPdoCount; i++)
    {
        if (*ptr == 0x1B)
        {
            pdo = gBusConfig.pPDO1B + ib++;
            gBusConfig.pdo1BCount++;
        }
        else
        {
            pdo = gBusConfig.pPDO1A + ia++;
            gBusConfig.pdo1ACount++;
        }
        pdo->sType = *ptr;
        ptr++; // 08000201 0000 -- PDO
        pdo->eAddr = *(ptr + 1) << 8 | *ptr;
        ptr += 2; //  0201 0000 -- PDO
        pdo->eType = (daisy_element_type_e)*ptr;
        ptr++; //       01 0000 -- PDO
        pdo->len = *ptr;
        ptr++; //          0000 -- PDO
        pdo->offset = *(ptr + 1) << 8 | *ptr;
        ptr += 2; // 指向了下一个pdo开始处
        LOGV(TAG, "pdo[%u].sType = 0x%02X", i, pdo->sType);
        LOGV(TAG, "pdo[%u].len = %u", i, pdo->len);
        LOGV(TAG, "pdo[%u].eType = %u", i, pdo->eType);
        LOGV(TAG, "pdo[%u].eAddr = 0x%04X", i, pdo->eAddr);
        LOGD(TAG, "pdo[%u].offset = 0x%04X", i, pdo->offset);
    }
    LOGI(TAG, "pdo1BCount = %u, pdo1ACount = %u", gBusConfig.pdo1BCount, gBusConfig.pdo1ACount);
    for (uint16_t i = 0; i < gBusConfig.pdo1ACount; i++)
    {
        LOGV(TAG, "gBusConfig.pPDO1A[%u].sType = 0x%02X", i, gBusConfig.pPDO1A[i].sType);
        LOGV(TAG, "gBusConfig.pPDO1A[%u].len = %u", i, gBusConfig.pPDO1A[i].len);
        LOGV(TAG, "gBusConfig.pPDO1A[%u].eType = %u", i, gBusConfig.pPDO1A[i].eType);
        LOGV(TAG, "gBusConfig.pPDO1A[%u].eAddr = 0x%04X", i, gBusConfig.pPDO1A[i].eAddr);
        LOGD(TAG, "gBusConfig.pPDO1A[%u].offset = 0x%04X", i, gBusConfig.pPDO1A[i].offset);
    }
    return ptr;
}

/*
aa51  //配置码
0094  //该配置所有内容的长度（从配置码开始计算包含配置码）
0004  //同步数据帧的字节数
0002 0004  //SDO的个数和PDO的个数
1C 1005     02    0002   //将BFM1005赋值为0002，02表示字节长度，1005表示网口后面挂接的模块数
1C 1006     02    0000   //将BFM1006赋值为0000，02表示字节长度，1006表示SPI后面挂接的模块数，以后用
//以下1B和1A是解释同步数据帧每个字节的含义的，主站和从站都需要
1B  01020000 0000   (Maping to 1060）
1A  01010000 0001   (Maping to 1050）
1B  01020008 0002   (Maping to 1060）
1A  01010008 0003   (Maping to 1050）
0002
// 分站_1_数据配置
0001 //1号站
002C //1号站的长度（从下面的字节开始计算）
0005 0002 //SDO的个数和PDO的个数
1E 1000     02    1A00 //0x1E表示比较指令，BFM1000是扩展模块的ID，用0x1A00和本机的BFM1000的值比较，如果不相等，表示配置错误，直接报错
1C 1003     02    0000 //将BFM1003赋值为0000，02表示字节长度，1003表示本站的数据在帧中的偏移
1C 1004     02    0002 //将BFM1004赋值为0002，02表示字节长度  1004表示本站数据的长度
1C 1005     02    0001 //网口后面挂接的模块数
1C 1006     02    0000 //SPI后面挂接的模块数，以后用
1B  1060  0000   (Maping to 020000）
1A  1050  0001   (Maping to 010000）
// 分站_2_数据配置
0002 //2号站
002C //1号站的长度（从下面的字节开始计算）
0005 0002 //SDO的个数和PDO的个数
1E 1000     02    1A02 //0x1E表示比较指令，BFM1000是扩展模块的ID，用0x1A02和本机的BFM1000的值比较，如果不相等，表示配置错误，直接报错，
1C 1003     02    0002 //1003表示本站的数据在帧中的偏移
1C 1004     02    0002 //1004表示本站数据的长度
1C 1005     02    0000 //1005表示网口后面挂接的模块数
1C 1006     02    0000 //1006表示SPI后面挂接的模块数，以后用
1B  1060  0002   (Maping to 020008）
1A  1050  0003   (Maping to 010008）
*/
static uint16_t parse_bus_config(unsigned char *ptr)
{
    LOGV(TAG, "Enter %s(), ptr = 0x%08X", __func__, ptr);
    //设置FEXLINK总线的默认超时时间为20，如果为0，会红灯闪烁报203错误
    SET_SD_ELEMENT_VALUE(SD233, 20);
    
    uint16_t length = GET_PU16_DATA(ptr + 2);
    hexdump(ptr, length);

    uint16_t bfmIdx;
    memset(&gBusConfig, 0, sizeof(gBusConfig));
    /*
        aa51 
        012c 
        0004 -- SDO个数
        1C 1003     02    0000
        1C 1004     02    0006
        1C 1005     02    0001
        1C 1006     02    0000 
    */
#if 0
    /* 同步数据帧的字节数 */
    ptr += 4;
    gBusConfig.nSynBuffLen = *(ptr + 1) << 8 | *ptr;
    LOGI(TAG, "gBusConfig.nSynBuffLen = 0x%X", gBusConfig.nSynBuffLen);
#endif
    /* SDO的个数 */
    ptr += 4; //Point to : 0004 -- SDO个数
    gBusConfig.nSdoCount = *(ptr + 1) << 8 | *ptr;
    LOGI(TAG, "gBusConfig.nSdoCount = %u", gBusConfig.nSdoCount);

    ptr += 2; //Point to : 1C 1003     02    0000 -- SDO
    ptr++;    //Point to :    1003     02    0000 -- SDO
    /* 解析SDO */
    for (uint16_t i = 0; i < gBusConfig.nSdoCount; i++)
    {
        bfmIdx = *(ptr + 1) << 8 | *ptr;
        LOGV(TAG, "bfmIdx = 0x%X", bfmIdx);
        switch(bfmIdx)
        {
        case BFM_IDX_0x6084:
            ptr += 3;
            gBusConfig.nBfm6084 = *(ptr + 1) << 8 | *ptr;
            break;
        case BFM_IDX_0x6085:
            ptr += 3;
            gBusConfig.nBfm6085 = *(ptr + 1) << 8 | *ptr;
            break;
        case BFM_IDX_0x6086:
            ptr += 3;
            gBusConfig.nBfm6086 = *(ptr + 1) << 8 | *ptr;
            break;
        case BFM_IDX_0x6087:
            ptr += 3;
            gBusConfig.nBfm6087 = *(ptr + 1) << 8 | *ptr;
            break;
        case BFM_IDX_0x608A:
            ptr += 3;
            gBusConfig.nBfm608A = *(ptr + 1) << 8 | *ptr;
            if(gBusConfig.nBfm608A < 1)
            {
                gBusConfig.nBfm608A = 1;
            }
            SET_SD_ELEMENT_VALUE(SD233, gBusConfig.nBfm608A);
            LOGI(TAG, "SD233 = %u,", GET_SD_ELEMENT_VALUE(SD233));
            break;
        default:
            LOGE(TAG, "Error SDO BFM index,  0x%X", bfmIdx);
            return pdFAIL;
        }
        ptr += 3;
    }

    ptr--; // Point to : 0003 0003 

    /* 解析PDO，PDO就是同步数据帧的元素 */
    /* 同步数据帧pdo的字节数 */
    gBusConfig.nPdoBytesExt = *(ptr + 1) << 8 | *ptr;
    ptr += 2; // Point to : 0003
    /* 同步数据帧pdo的个数 */
    gBusConfig.nPdoCountExt = *(ptr + 1) << 8 | *ptr;
    LOGI(TAG, "gBusConfig.nPdoCountExt = %u, gBusConfig.nPdoBytesExt = %u", gBusConfig.nPdoCountExt, gBusConfig.nPdoBytesExt);
    ptr += 2; // Point to : 1B  01020008 0000   (Maping to 1060)
    if (gBusConfig.nPdoCountExt == 0)
    {
        goto PARSE_IT1;
    }
    ptr = parse_ExtPDO(ptr);


PARSE_IT1:
    gBusConfig.nPdoBytes = *(ptr + 1) << 8 | *ptr;
    gBusConfig.nSynBuffLen = gBusConfig.nPdoBytes;
    LOGV(TAG, "gBusConfig.nSynBuffLen = %u", gBusConfig.nSynBuffLen);
    ptr += 2;
    gBusConfig.nPdoCount = *(ptr + 1) << 8 | *ptr;
    ptr += 2;
    /* 此时ptr已指向: 1B      01020010 0000   (Maping to 1060)
       或者，指向了： 0001 （从站+扩展的个数）
    */
    LOGI(TAG, "gBusConfig.nPdoBytes = %u, gBusConfig.nPdoCount = %u", gBusConfig.nPdoBytes, gBusConfig.nPdoCount);
    if (gBusConfig.nPdoCount == 0) // 说明没有从站
    {
        goto PARSE_IT2;
    }
    ptr = parse_SlavePDO(ptr);

PARSE_IT2:
    // 此时ptr指向： 01 00 -- 有多少从站+模块
    /* 从站的个数，从站的内容不用解析，弄出来直接发给从站，从站解析即可 */
    gBusConfig.nSubStationCount = *(ptr + 1) << 8 | *ptr;
    ptr += 2; //此时指向了分站或扩展模块的起始处了
    LOGI(TAG, "nSubStationCount = %u",  gBusConfig.nSubStationCount);

/* 分站_1_数据配置
    01 00 -- 站号
    2C 00 -- 长度
    05 00 -- SDO个数
    02 00 -- PDO个数

    //以下是SDO
    1E  00 10  02  00 1A 
    1C  03 10  02  00 00 
    1C  04 10  02  02 00 
    1C  05 10  02  00 00 
    1C  06 10  02  00 00

    //以下是PDO
    1B  60 10  00 00 
    1A  50 10  01 00  
*/
    //此时ptr指向了分站1（或模块1）的起始处
    for (uint16_t i = 0; i < gBusConfig.nSubStationCount; i++)
    {
        gBusConfig.item[i].sNumExt = *ptr;
        gBusConfig.item[i].sNum = *(ptr + 1);
        ptr += 2;// Point to : 2C 00 -- 长度
        gBusConfig.item[i].len  = *(ptr + 1) << 8 | *ptr;
        gBusConfig.item[i].len += 4;

        gBusConfig.item[i].pSubStationbuf = pvPortMalloc(gBusConfig.item[i].len + 8);
        memcpy(gBusConfig.item[i].pSubStationbuf, ptr - 2 , gBusConfig.item[i].len);

        ptr -= 2;
        ptr += gBusConfig.item[i].len; //指向了下一个分站数据起始处

        if ((gBusConfig.item[i].sNum == 0) && (gBusConfig.item[i].sNumExt != 0))
        {
            gBusConfig.nMasterExtCount++;
        }
        if ((gBusConfig.item[i].sNum != 0) && (gBusConfig.item[i].sNumExt == 0))
        {
            gBusConfig.nSlaveCount++;
        }
        LOGD(TAG, "item[%u].sNumExt = %u", i, gBusConfig.item[i].sNumExt);
        LOGV(TAG, "item[%u].sNum = %u", i, gBusConfig.item[i].sNum);
        LOGI(TAG, "item[%u].len = %u", i, gBusConfig.item[i].len);
        LOGV(TAG, "item[%u].pSubStationbuf = 0x%08X", i, gBusConfig.item[i].pSubStationbuf);
        hexdump(gBusConfig.item[i].pSubStationbuf, gBusConfig.item[i].len);
    }
    LOGW(TAG, "nSlaveCount = %u, nMasterExtCount = %u", gBusConfig.nSlaveCount, gBusConfig.nMasterExtCount);

    return length;
}

bool isRxPDO(uint16_t idx)
{
    bool ret = false;

  //  LOGV(TAG, "Enter %s(), idx = 0x%04X", __func__, idx);	
    /*
    switch (idx)
    {
        case KOD_INDEX_1600:
        case KOD_INDEX_1601:
        case KOD_INDEX_1602:
        case KOD_INDEX_1603:
            ret = true;
            break;
        case KOD_INDEX_1A00:
        case KOD_INDEX_1A01:
        case KOD_INDEX_1A02:
        case KOD_INDEX_1A03:
            ret = false;
            break;
    }
    */

    if (idx >= 0x1600 && idx <= 0x16FF)
    {
         ret = true;
    }
    else if (idx >= 0x1A00 && idx <= 0x1AFF)
    {
         ret = false;
    }

    return ret;
}

#if 0
void set_sIdx(uint32_t *pSidx, uint8_t len)
{
    uint32_t val = *pSidx;

    switch (len)
    {
        case 1:
            val = (val << 8) | 0x08U;
            break;
        case 2:
            val = (val << 8) | 0x10U;
            break;
        case 4:
            val = (val << 8) | 0x20U;
            break;
        default:
            val = (val << 8) | 0x10U;
            break;
    }

    *pSidx = val;
}
#else
void set_sIdx(uint32_t *pSidx, uint8_t bitLen)
{
    uint32_t val = *pSidx;
    val = (val << 8) | bitLen;
    *pSidx = val;
}
#endif

/* From MiLink.txt
aa54 
00b9 
001f 
0001 
// 从站_1_数据配置
0001
00AD
0009 000a 
6060    00    01    0A
60C2    01    01    03
60C2    02    01    FE
431F    00    02    FFFE
4107    00    02    0102
6092    01    04    00000202
6092    02    04    00000204
6091    01    04    01000006
6091    02    04    01000008
1600  604000  020907D0  0000
1600  60FF00  040907D1  0002
1600  607A00  040907D3  0006
1600  60FE01  040907D5  000A
1600  606000  010907D7  000E
1A00  604100  02091388  000F
1A00  606400  04091389  0011
1A00  606C00  0409138B  0015
1A00  603F00  0209138D  0019
1A00  60FD00  0409138E  001B
*/
/* 内存直接情况：
54AA 
B900 -- 总长度（字节）
1F00 -- 帧数据总长度（字节）
0100 -- 从站个数
// 从站_1_数据配置
01 00
AD 00
09 00 0A 00
6060 00 01 0A
C260 01 01 03
C260 02 01 FE
1F43 00 02 FEFF
0741 00 02 0201
9260 01 04 02020000
9260 02 04 04020000
9160 01 04 06000001
9160 02 04 08000001
0016 406000 D0070902 0000
0016 FF6000 D1070904 0200
0016 7A6000 D3070904 0600
0016 FE6001 D5070904 0A00
0016 606000 D7070901 0E00
001A 416000 88130902 0F00
001A 646000 89130904 1100
001A 6C6000 8B130904 1500
001A 3F6000 8D130902 1900
001A FD6000 8E130904 1B00
// 从站_2_数据配置
02 00
AD 00
09 00 0A 00
...
*/
static uint16_t parse_ethercat_config(unsigned char *ptr)
{
    LOGV(TAG, "Enter %s(), ptr = 0x%08X, free heap = %u", __func__, ptr, xPortGetFreeHeapSize());
    uint16_t length = GET_PU16_DATA(ptr + 2);
    hexdump(ptr, length);
    uint16_t nTemp;
    unsigned char *ucharPtr;
    uint16_t bfmIdx;
    uint16_t cIdx;
    uint8_t *ptrBak;

    memset(&gSoemConfig, 0, sizeof(gSoemConfig));

    ptr += 4; // 1F00 -- 帧数据总长度（字节）
    gSoemConfig.nSynBuffLen = *(ptr + 1) << 8 | *ptr;
    LOGI(TAG, "gSoemConfig.nSynBuffLen = %u", gSoemConfig.nSynBuffLen);

    ptr += 2;//0100 -- 从站个数
    gSoemConfig.nSubStationCount = *(ptr + 1) << 8 | *ptr;;
    LOGD(TAG, "gSoemConfig.nSubStationCount = %u", gSoemConfig.nSubStationCount);
    if (gSoemConfig.nSubStationCount == 0)
    {
        return length;
    }
    gSoemConfig.pSlaves = pvPortMalloc(sizeof(soem_slave_st) * gSoemConfig.nSubStationCount);
    memset(gSoemConfig.pSlaves, 0, sizeof(soem_slave_st) * gSoemConfig.nSubStationCount);

    ptr += 2;//0100 -- 从站号
    for (int i = 0; i < gSoemConfig.nSubStationCount; i++)
    {
        ptr += 4; //0900 0A00 -- SDO的个数和PDO的个数
        gSoemConfig.pSlaves[i].nSdoCount = *(ptr + 1) << 8 | *ptr;
        LOGV(TAG, "gSoemConfig.pSlaves[%d].nSdoCount = %u", i, gSoemConfig.pSlaves[i].nSdoCount);
        if (gSoemConfig.pSlaves[i].nSdoCount != 0)
        {
            gSoemConfig.pSlaves[i].pSDO = pvPortMalloc(sizeof(soem_sdo_st) * gSoemConfig.pSlaves[i].nSdoCount);
        }
        ptr += 2; //0A00 -- PDO的个数
        gSoemConfig.pSlaves[i].nPdoCount = *(ptr + 1) << 8 | *ptr;
        LOGW(TAG, "gSoemConfig.pSlaves[%d].nPdoCount = %u", i, gSoemConfig.pSlaves[i].nPdoCount);

        ptr += 2; // 6060 00 01 0A (如果nSdoCount等于0: 0016 406000 D0070902 0000)
        for (int k = 0; k < gSoemConfig.pSlaves[i].nSdoCount; k++)
        {
            gSoemConfig.pSlaves[i].pSDO[k].idx = *(ptr + 1) << 8 | *ptr;
            ptr += 2; //00 01 0A
            gSoemConfig.pSlaves[i].pSDO[k].subIdx = *ptr;
            ptr++; //01 0A
            gSoemConfig.pSlaves[i].pSDO[k].len = *ptr;
            ptr++; //0A
            if (gSoemConfig.pSlaves[i].pSDO[k].len == 0x08)
            {
                gSoemConfig.pSlaves[i].pSDO[k].val = (uint32_t)*ptr;
                ptr++;
            }
            else if (gSoemConfig.pSlaves[i].pSDO[k].len == 0x10)
            {
                gSoemConfig.pSlaves[i].pSDO[k].val = (uint32_t)(*(ptr + 1) << 8 | *ptr);
                ptr += 2;
            }
            else // 0x20
            {
                gSoemConfig.pSlaves[i].pSDO[k].val = (*(ptr + 3) << 24) |
                            (*(ptr + 2) << 16) |
                            (*(ptr + 1) << 8)  |
                            (*ptr);
                ptr += 4;
            }
        }

        //此时ptr已指向：0016 406000 D0070902 0000
        ptrBak = ptr;
        for (int j = 0; j < gSoemConfig.pSlaves[i].nPdoCount; j++)
        {
            cIdx = *(ptr + 1) << 8 | *ptr;
            if (isRxPDO(cIdx))
            {
                gSoemConfig.pSlaves[i].pdoRxCount++;
            }
            else
            {
                gSoemConfig.pSlaves[i].pdoTxCount++;
            }
            ptr += 11;
        }
        LOGI(TAG, "gSoemConfig.pSlaves[%d].pdoRxCount = %u, gSoemConfig.pSlaves[%d].pdoTxCount = %u", i, gSoemConfig.pSlaves[i].pdoRxCount, i, gSoemConfig.pSlaves[i].pdoTxCount);

        gSoemConfig.pSlaves[i].pRxPDO = pvPortMalloc(sizeof(soem_pdo_st) * gSoemConfig.pSlaves[i].pdoRxCount);
        memset(gSoemConfig.pSlaves[i].pRxPDO, 0, sizeof(soem_pdo_st) * gSoemConfig.pSlaves[i].pdoRxCount);
        gSoemConfig.pSlaves[i].pTxPDO = pvPortMalloc(sizeof(soem_pdo_st) * gSoemConfig.pSlaves[i].pdoTxCount);
        memset(gSoemConfig.pSlaves[i].pTxPDO, 0, sizeof(soem_pdo_st) * gSoemConfig.pSlaves[i].pdoTxCount);

        ptr = ptrBak; //0016 406000 D0070902 0000
        int r = 0; // rxPDO count
        int t = 0; // txPDO count
        uint16_t rAddr, tAddr;
        soem_pdo_st *pTempPDO;
        for (int k = 0; k < gSoemConfig.pSlaves[i].nPdoCount; k++)
        {
            cIdx = *(ptr + 1) << 8 | *ptr;
            if (isRxPDO(cIdx))
            {
                pTempPDO = &gSoemConfig.pSlaves[i].pRxPDO[r];
                r++;
            }
            else
            {
                pTempPDO = &gSoemConfig.pSlaves[i].pTxPDO[t];
                t++;
            }
            pTempPDO->cIdx = cIdx;
            ptr += 2; // 406000 D0070902 0000
            pTempPDO->sIdx = *(ptr + 1) << 8 | *ptr;
            ptr += 2; // 00 D0070902 0000
            pTempPDO->sIdx = (pTempPDO->sIdx<<8) | *ptr;
            ptr++; // D0070902 0000
            pTempPDO->eAddr = *(ptr + 1) << 8 | *ptr;
            ptr += 2; // 0902 0000
            pTempPDO->eType = (daisy_element_type_e)*ptr;
            ptr++; // 02 0000
            pTempPDO->len = *ptr;
            set_sIdx(&pTempPDO->sIdx, pTempPDO->len);
            ptr++; // 0000
            pTempPDO->offset = *(ptr + 1) << 8 | *ptr;
            ptr += 2; // Point to next PDO : 0316 FF6000 D1070904 0200

            if (r == 1)
            {
                rAddr = pTempPDO->offset;
            }
            if (r == gSoemConfig.pSlaves[i].pdoRxCount)
            {
                if (r == 0)
                {
                    gSoemConfig.pSlaves[i].outSize = 0;
                }
                else
                {
                    gSoemConfig.pSlaves[i].outSize = pTempPDO->offset + pTempPDO->len - rAddr;
                }
                r = 0xffff;
            }

            if (t == 1)
            {
                tAddr = pTempPDO->offset;
            }
            if (t == gSoemConfig.pSlaves[i].pdoTxCount)
            {
                if (t == 0)
                {
                    gSoemConfig.pSlaves[i].inSize = 0;
                }
                else
                {
                    gSoemConfig.pSlaves[i].inSize = pTempPDO->offset + pTempPDO->len - tAddr;
                }
                t = 0xffff;
            }
        }
        LOGI(TAG, "outSize = %u, inSize = %u", gSoemConfig.pSlaves[i].outSize, gSoemConfig.pSlaves[i].inSize);
    }

    LOGD(TAG, "Leave %s(), free heap = %u", __func__, xPortGetFreeHeapSize());
    return length;
}

static uint16_t parse_ethercat_master(unsigned char *ptr)
{
    LOGV(TAG, "Enter %s(), ptr = 0x%08X", __func__, ptr);
    
    uint16_t length = GET_PU16_DATA(ptr + 2);
    hexdump(ptr, length);

    ptr += 4;
    uint16_t slaveStartWait = GET_PU16_DATA(ptr);
    SET_SD_ELEMENT_VALUE(SD301, slaveStartWait);
    
    ptr += 2;
    uint16_t loopTime = GET_PU16_DATA(ptr);
    if (loopTime == 0)
    {
        loopTime = 1;
    }
    SET_SD_ELEMENT_VALUE(SD300, loopTime);

    ptr += 2;
    uint8_t stop = *ptr;
    SET_SD_ELEMENT_VALUE(SD302, stop);

    ptr++;
    uint8_t tryTimes = *ptr;
    SET_SD_ELEMENT_VALUE(SD303, tryTimes);

    LOGD(TAG, "SD300 = %u, SD301 = %u, SD302 = %u, SD303 = %u", loopTime, slaveStartWait, stop, tryTimes);
    return length;
}

/**
  * @brief  系统块解析
  * @param  flag == 1,表示不解析Ethercat和总线的相关内容
  * @retval None
  */
char plc_parse_system_block(unsigned char flag)
{
    unsigned long i;
    unsigned long llv_Len;
    unsigned char *lcp_SysBlockPtr;
    unsigned char *lcp_SysBlockEndPtr;
    unsigned short *lsp_TempPtr;
    unsigned char *lcp_TempPtr;

    /*系统块错误,清系统块配置,返回*/
    if(guv_StopError.bit.sysblock_err)
    {
        return pdFAIL;
    }

    lcp_SysBlockPtr = gtv_UserFilePtrSt.SysBlockPtr;

    /*读系统块长度*/
    llv_Len = plc_get_file_length(lcp_SysBlockPtr + FILE_LEN_INFO_START_INDEX, 4);
    PRINTF("Enter %s(), lcp_SysBlockPtr = 0x%x, llv_Len = %d\r\n", __func__, lcp_SysBlockPtr, llv_Len);
    hexdump(lcp_SysBlockPtr, llv_Len);
    /*取系统块结束位置*/
    lcp_SysBlockEndPtr = lcp_SysBlockPtr + llv_Len - 4;
    /*取第一条配置码位置*/
    lcp_SysBlockPtr += FILE_INFO_START_INDEX + 2 ;

    PRINTF("lcp_SysBlockPtr = 0x%x, lcp_SysBlockEndPtr = 0x%x\r\n", lcp_SysBlockPtr, lcp_SysBlockEndPtr);
    while ( lcp_SysBlockPtr < lcp_SysBlockEndPtr)
    {
        lsp_TempPtr = (unsigned short *)lcp_SysBlockPtr;

        LOGD(TAG, "*lsp_TempPtr = 0x%X", *lsp_TempPtr);
        switch(*lsp_TempPtr)
        {

        /*掉电保存的元件配置*/
        case SYS_BLK_CFG_BATTERY_SAVE:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            if(llv_Len != 44)
            {
                guv_StopError.bit.sysblock_err = 1;
                gtv_UserFilePtrSt.SysBlockPtr = (unsigned char *)sysblock_default;
                return pdFAIL;
            }

            plc_parse_sysblk_plsd_config(lcp_SysBlockPtr);

            lcp_SysBlockPtr += llv_Len;
            break;

        /* 系统STOP状态时的输出状态 */
        // 02 AA 16 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
        case SYS_BLK_CFG_OUTPUT_POINT:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            if(llv_Len != 22)
            {
                guv_StopError.bit.sysblock_err = 1;
                gtv_UserFilePtrSt.SysBlockPtr = (unsigned char *)sysblock_default;
                return pdFAIL;
            }
            lcp_SysBlockPtr += llv_Len;

            gtv_PlcRunToStopOutputStatus.mcv_Status = *(lsp_TempPtr + 2);
            LOGI("plc_sysblock", "mcv_Status = 0x%X", gtv_PlcRunToStopOutputStatus.mcv_Status);
            lsp_TempPtr += 3;
            for(i = 0; i < 8; i++)
            {
                gtv_PlcRunToStopOutputStatus.msv_OutValue[i] = *(lsp_TempPtr + i);
            }
            hexdump(gtv_PlcRunToStopOutputStatus.msv_OutValue, sizeof(gtv_PlcRunToStopOutputStatus.msv_OutValue));
            break;

        /*看门狗定时*/
        case SYS_BLK_CFG_WATCH_DOG_TIME:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            if(llv_Len != 6)
            {
                guv_StopError.bit.sysblock_err = 1;
                gtv_UserFilePtrSt.SysBlockPtr = (unsigned char *)sysblock_default;
                return pdFAIL;
            }
            lcp_SysBlockPtr += llv_Len;

            if(*(lsp_TempPtr + 2) < 100)
            {
                SET_SD_ELEMENT_VALUE(34, 100);
            }
            else if(*(lsp_TempPtr + 2) > 1000)
            {
                SET_SD_ELEMENT_VALUE(34, 1000);
            }
            else
            {
                SET_SD_ELEMENT_VALUE(34, *(lsp_TempPtr + 2));
            }
            LOGI("plc_sysblock", "wathdog = %u", GET_SD_ELEMENT_VALUE(SD34));
            break;

        /*恒定扫描时间*/
        case SYS_BLK_CFG_SCAN_TIME:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            if(llv_Len != 6)
            {
                guv_StopError.bit.sysblock_err = 1;
                gtv_UserFilePtrSt.SysBlockPtr = (unsigned char *)sysblock_default;
                return pdFAIL;
            }
            lcp_SysBlockPtr += llv_Len;

            SET_SD_ELEMENT_VALUE(33, *(lsp_TempPtr + 2));

            /*恒定扫描时间不为零,置为SM8 标志位*/
            if(GET_SD_ELEMENT_VALUE(33))
            {
                plc_set_bit_element_value(SM_ELEMENT, SM8, 1);
            }
            break;

        /*失电检测时间*/
        case SYS_BLK_CFG_LOSE_POWER_CHECK_TIME:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            if(llv_Len != 6)
            {
                guv_StopError.bit.sysblock_err = 1;
                gtv_UserFilePtrSt.SysBlockPtr = (unsigned char *)sysblock_default;
                return pdFAIL;
            }
            lcp_SysBlockPtr += llv_Len;

            if(*(lsp_TempPtr + 2) < 10)
            {
                SET_SD_ELEMENT_VALUE(5, 10);
            }
            else if(*(lsp_TempPtr + 2) > 100)
            {
                SET_SD_ELEMENT_VALUE(5, 100);
            }
            else
            {
                SET_SD_ELEMENT_VALUE(5, *(lsp_TempPtr + 2));
            }
            break;


        /*FEXLINK循环周期*/ 
        case SYS_BLK_CFG_FEXLINK_CYCLE_TIME:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            if(llv_Len != 6)
            {
                guv_StopError.bit.sysblock_err = 1;
                gtv_UserFilePtrSt.SysBlockPtr = (unsigned char *)sysblock_default;
                return pdFAIL;
            }
            lcp_SysBlockPtr += llv_Len;

            if(*(lsp_TempPtr + 2) <= 0)
            {
                SET_SD_ELEMENT_VALUE(SD234, 0);
            }
            else if(*(lsp_TempPtr + 2) > 2000)
            {
                SET_SD_ELEMENT_VALUE(SD234, 2000);
            }
            else
            {
                SET_SD_ELEMENT_VALUE(SD234, *(lsp_TempPtr + 2));
            }
            break;

        /*数字滤波常数*/
        case SYS_BLK_CFG_DIGIT_FILTER_TIME:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            if(llv_Len != 8)
            {
                guv_StopError.bit.sysblock_err = 1;
                gtv_UserFilePtrSt.SysBlockPtr = (unsigned char *)sysblock_default;
                return pdFAIL;
            }
            lcp_SysBlockPtr += llv_Len;

            SET_SD_ELEMENT_VALUE(35, *(lsp_TempPtr + 2));
            SET_SD_ELEMENT_VALUE(36, *(lsp_TempPtr + 3));

            /*20170713:设置滤波常数, 待增加...*/
            break;
        case SYS_BLK_CFG_DIGIT_FILTER_TIME1:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            if(llv_Len != 20)
            {
                guv_StopError.bit.sysblock_err = 1;
                gtv_UserFilePtrSt.SysBlockPtr = (unsigned char *)sysblock_default;
                return pdFAIL;
            }
            lcp_SysBlockPtr += llv_Len;
            break;

        /*高级设置*/
        case SYS_BLK_CFG_ADVANCED_SETTING: //07 AA 06 00 08 80
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            PRINTF("llv_Len = %d\r\n", llv_Len);
            if(llv_Len != 6)
            {
                guv_StopError.bit.sysblock_err = 1;
                gtv_UserFilePtrSt.SysBlockPtr = (unsigned char *)sysblock_default;
                return pdFAIL;
            }
            lcp_SysBlockPtr += llv_Len;

            guv_PlcSysBlkAdSetting.msv_Setting = *(lsp_TempPtr + 2);
            PRINTF("guv_PlcSysBlkAdSetting.msv_Setting1 = 0x%x\r\n", guv_PlcSysBlkAdSetting.msv_Setting);
            LOGV("plc_sysblock", "no_battery_mode = %u", guv_PlcSysBlkAdSetting.bit.no_battery_mode);
            LOGD("plc_sysblock", "keep_element_value = %u", guv_PlcSysBlkAdSetting.bit.keep_element_value);
            LOGI("plc_sysblock", "data_block_valid = %u", guv_PlcSysBlkAdSetting.bit.data_block_valid);
            LOGW("plc_sysblock", "forbidden_format = %u", guv_PlcSysBlkAdSetting.bit.forbidden_format);
            if(guv_PlcSysBlkAdSetting.bit.no_battery_mode)
            {
                plc_set_bit_element_value(SM_ELEMENT, 7, 1);
            }
            else
            {
                plc_set_bit_element_value(SM_ELEMENT, 7, 0);
            }
            break;

        /*设置系统运行输入点*/
        case SYS_BLK_CFG_RUN_INPUT_POINT: //08 AA 06 00 00 00
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            if(llv_Len != 6)
            {
                guv_StopError.bit.sysblock_err = 1;
                gtv_UserFilePtrSt.SysBlockPtr = (unsigned char *)sysblock_default;
                return pdFAIL;
            }
            lcp_SysBlockPtr += llv_Len;

            if(*(lsp_TempPtr + 2) == 0)
            {
                plc_set_bit_element_value(SM_ELEMENT, 9, 0);
                SET_SD_ELEMENT_VALUE(9, 0);
            }
            else
            {
                plc_set_bit_element_value(SM_ELEMENT, 9, 1);

                i = 0;
                while(((*(lsp_TempPtr + 2)) & (0x01 << i)) == 0)
                {
                    i++;
                }

                SET_SD_ELEMENT_VALUE(9, i);
            }
            break;
#if 0
        /*串口0*/
        case SYS_BLK_CFG_SERIAL_PORT0:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            PRINTF("llv_Len = %d\r\n", llv_Len);
            if(llv_Len != 20)
            {
                guv_StopError.bit.sysblock_err = 1;
                gtv_UserFilePtrSt.SysBlockPtr = (unsigned char *)sysblock_default;
                return pdFAIL;
            }
            lcp_SysBlockPtr += llv_Len;

            plc_parse_sysblk_uart_config(0, lsp_TempPtr);
            break;

        /*串口1*/
        case SYS_BLK_CFG_SERIAL_PORT1:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            PRINTF("llv_Len PORT1 = %d\r\n", llv_Len);
            if(llv_Len != 20)
            {
                guv_StopError.bit.sysblock_err = 1;
                gtv_UserFilePtrSt.SysBlockPtr = (unsigned char *)sysblock_default;
                return pdFAIL;
            }
            lcp_SysBlockPtr += llv_Len;

            plc_parse_sysblk_uart_config(1, lsp_TempPtr);
            break;

        /*串口2*/
        case SYS_BLK_CFG_SERIAL_PORT2:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            if(llv_Len != 20)
            {
                guv_StopError.bit.sysblock_err = 1;
                gtv_UserFilePtrSt.SysBlockPtr = (unsigned char *)sysblock_default;
                return pdFAIL;
            }
            lcp_SysBlockPtr += llv_Len;

            plc_parse_sysblk_uart_config(2, lsp_TempPtr);
            break;
#endif
        /*中断优先级*/
        case SYS_BLK_CFG_INT_PRIORITY:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            PRINTF("llv_Len 1 = %d\r\n", llv_Len);
            lcp_SysBlockPtr += llv_Len;

            /*配置中断源数量*/
            llv_Len = GET_PU16_DATA(lsp_TempPtr + 2);
            PRINTF("llv_Len 2 = %d\r\n", llv_Len);
            if(llv_Len > MAX_SYS_INT_NUM)
            {
                guv_StopError.bit.sysblock_err = 1;
                gtv_UserFilePtrSt.SysBlockPtr = (unsigned char *)sysblock_default;
                return pdFAIL;
            }

            lcp_TempPtr = (unsigned char *)(lsp_TempPtr + 3);
            for(i = 0; i < llv_Len; i++)
            {
                if((GET_PU8_DATA(lcp_TempPtr) > 1) && (GET_PU8_DATA(lcp_TempPtr) != 0xFF))
                {
                    guv_StopError.bit.sysblock_err = 1;
                    gtv_UserFilePtrSt.SysBlockPtr = (unsigned char *)sysblock_default;
                    return pdFAIL;
                }

                gtp_InterruptInfo->mtv_IntSource[i].mcv_Priority = GET_PU8_DATA(lcp_TempPtr);
                lcp_TempPtr ++;
            }
            break;

        case SYS_CommModals:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            LOGI("sysblock", "SYS_CommModals llv_Len = %u", llv_Len);				
            lcp_SysBlockPtr += llv_Len;
            break;

        /*CAN0 自由口协议配置*/
        case SYS_BLK_CFG_CAN0_FREE_PROTOCOL:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            LOGI("sysblock", "SYS_BLK_CFG_CAN0_FREE_PROTOCOL llv_Len = %u", llv_Len);		
            lcp_SysBlockPtr += llv_Len;
            /*20170713:CAN配置解析,待增加...*/

            break;

        /*CAN1 自由口协议配置*/
        case SYS_BLK_CFG_CAN1_FREE_PROTOCOL:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            LOGI("sysblock", "SYS_BLK_CFG_CAN1_FREE_PROTOCOL llv_Len = %u", llv_Len);	
            lcp_SysBlockPtr += llv_Len;
            /*20170713:CAN配置解析,待增加...*/

            break;

        /*CAN2 自由口协议配置*/
        case SYS_BLK_CFG_CAN2_FREE_PROTOCOL:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            LOGI("sysblock", "SYS_BLK_CFG_CAN2_FREE_PROTOCOL llv_Len = %u", llv_Len);	
            lcp_SysBlockPtr += llv_Len;
            /*20170713:CAN配置解析,待增加...*/

            break;

        /*CAN3 自由口协议配置*/
        case SYS_BLK_CFG_CAN3_FREE_PROTOCOL:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            LOGI("sysblock", "SYS_BLK_CFG_CAN3_FREE_PROTOCOL llv_Len = %u", llv_Len);	
            lcp_SysBlockPtr += llv_Len;
            /*20170713:CAN配置解析,待增加...*/

            break;
        /*串口0*/
        case SYS_BLK_CFG_SERIAL_PORT0:
        /*串口1*/
        case SYS_BLK_CFG_SERIAL_PORT1:
        /*串口2*/
        case SYS_BLK_CFG_SERIAL_PORT2:
        case SYS_InnerADDA:
//            llv_Len = parse_AD(lcp_SysBlockPtr);
//            LOGI("sysblock", "llv_Len = %u", llv_Len);
//            lcp_SysBlockPtr += llv_Len;
//            break;
        case SYS_InnerPT:
        case SYS_InnerTC:
        case SYS_InnerIO:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            LOGI("sysblock", "SYS_InnerPT TC IO llv_Len = %u", llv_Len);
            lcp_SysBlockPtr += llv_Len;
            break;

#if (KALYKE_MODBUS_TCP_SHEET == 0)
        /*MODLINK*/
        case SYS_BLK_CFG_MODLINK:
        {
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            lcp_SysBlockPtr += llv_Len;
            LOGD("sysblock", "llv_Len = %u", llv_Len);
            if (llv_Len != 6)
            {
                unsigned char lcv_Ret = plc_parse_sysblk_modlink_config(lsp_TempPtr);
                if(lcv_Ret != pdPASS)
                {
                    LOGE("sysblock", "plc_parse_sysblk_modlink_config return ERROR!");
                    guv_StopError.bit.sysblock_err = 1;
                    gtv_UserFilePtrSt.SysBlockPtr = (unsigned char *)sysblock_default;
                    return pdFAIL;
                }
            }
            break;
        }

        case SYS_MODBUS_TCP_CLIENT:
            llv_Len = parse_modbus_tcp_client(lcp_SysBlockPtr);
            LOGI("sysblock", "llv_Len = %u", llv_Len);
            lcp_SysBlockPtr += llv_Len;
            break;
#endif
        /*轴参数*/
        case SYS_BLK_CFG_AXIS_DATA:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            LOGI("sysblock", "SYS_BLK_CFG_AXIS_DATA llv_Len = %u", llv_Len);
            lcp_SysBlockPtr += llv_Len;
            break;

        /*CAN0口 CANOPEN协议配置*/
        case SYS_BLK_CFG_CAN0_CANOPEN:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            LOGI("sysblock", "SYS_BLK_CFG_CAN0_CANOPEN llv_Len = %u", llv_Len);
            lcp_SysBlockPtr += llv_Len;
            /*20170713:CAN配置解析,待增加...*/

            break;

        /*CAN1口 CANOPEN协议配置*/
        case SYS_BLK_CFG_CAN1_CANOPEN:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            LOGI("sysblock", "SYS_BLK_CFG_CAN1_CANOPEN llv_Len = %u", llv_Len);
            lcp_SysBlockPtr += llv_Len;
            /*20170713:CAN配置解析,待增加...*/

            break;

        /*CAN2口 CANOPEN协议配置*/
        case SYS_BLK_CFG_CAN2_CANOPEN:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            LOGI("sysblock", "SYS_BLK_CFG_CAN2_CANOPEN llv_Len = %u", llv_Len);
            lcp_SysBlockPtr += llv_Len;
            /*20170713:CAN配置解析,待增加...*/

            break;

        /*CAN3口 CANOPEN协议配置*/
        case SYS_BLK_CFG_CAN3_CANOPEN:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            LOGI("sysblock", "SYS_BLK_CFG_CAN3_CANOPEN llv_Len = %u", llv_Len);
            lcp_SysBlockPtr += llv_Len;
            /*20170713:CAN配置解析,待增加...*/

            break;

        /*分站配置，Mistudio反编译用*/
        case SYS_SUBSTATION:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            LOGI("sysblock", "SYS_SUBSTATION llv_Len = %u", llv_Len);
            lcp_SysBlockPtr += llv_Len;
            break;

        /*扩展总线配置码*/
        case SYS_BUS_CONFIG:
            if (flag == 1)
            {
               llv_Len = parse_bus_config(lcp_SysBlockPtr);
            }
            else
            {
                llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            }
            LOGI("sysblock", "SYS_BUS_CONFIG llv_Len = %u", llv_Len);
            lcp_SysBlockPtr += llv_Len;
            break;
        /*Mistudio反编译用*/
        case SYS_EtherCATArrays:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            LOGI("sysblock", "SYS_EtherCATArrays llv_Len = %u", llv_Len);
            lcp_SysBlockPtr += llv_Len;
            break;

        /* EtherCAT总线配置码 */
        case SYS_ETHERCAT_CONFIG:
            if (flag == 1)
            {
                llv_Len = parse_ethercat_config(lcp_SysBlockPtr);
            }
            else
            {
                 llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            }
            LOGI("sysblock", "SYS_ETHERCAT_CONFIG llv_Len = %u", llv_Len);
            hexdump(lcp_SysBlockPtr, llv_Len);
            lcp_SysBlockPtr += llv_Len;
            break;

        case SYS_ETHERCAT_MASTER:
            llv_Len = parse_ethercat_master(lcp_SysBlockPtr);
            LOGI("sysblock", "SYS_ETHERCAT_MASTER llv_Len = %u", llv_Len);
            lcp_SysBlockPtr += llv_Len;
            break;
        case SYS_PLC_ROLE:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            if(llv_Len != 10)
            {
                guv_StopError.bit.sysblock_err = 1;
                gtv_UserFilePtrSt.SysBlockPtr = (unsigned char *)sysblock_default;
                return pdFAIL;
            }
            gPlcRunAs = *(lcp_SysBlockPtr + 4);
            gLeftModules.BYTE = *(lcp_SysBlockPtr + 5);
           //后面4个字节备用，以防还有其他配置
            LOGI("plc_sysblock", "gPldRunAs(1:master,2:smartdevice) = %u,gLeftModules.BYTE = %u", gPlcRunAs,gLeftModules.BYTE);
            lcp_SysBlockPtr += llv_Len;
            break;
        case SYS_MODBUS_TCP_CLIENT:
        case SYS_BLK_CFG_MODLINK:
        case SYS_CAMSetting:
        case SYS_RESERVE7:
        case SYS_RESERVE8:
        case SYS_RESERVE9:
            llv_Len = GET_PU16_DATA(lcp_SysBlockPtr + 2);
            LOGI("sysblock", "RESERVER7-8 llv_Len = %u", llv_Len);
            lcp_SysBlockPtr += llv_Len;
            break;

        default:
            /*不支持系统块配置码,系统块错误*/
            LOGI("sysblock", "my god! sysblock is error = %u", *lsp_TempPtr);
            guv_StopError.bit.sysblock_err = 1;
            gtv_UserFilePtrSt.SysBlockPtr = (unsigned char *)sysblock_default;
            return pdFAIL;
        }
    }

    LOGV("sysblock", "Leave %s()", __func__);
    return pdPASS;
}

