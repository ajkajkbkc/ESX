/**
  ******************************************************************************
  * @file    bsp_dct.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   设备配置表相关函数
  ******************************************************************************
  */

#include "fsl_debug_console.h"
#include "fsl_cache.h"
#include "bsp.h"
#include "bsp_dct.h"
#include "bsp_gpio.h"
#include "FreeRTOS.h"
#include "bsp_flash.h"
#include "verify_func.h"
#include "kalyke_opts.h"
#include "plc_sysblock.h"


static bool bspIsSupportLowPowerDetect(void);

/*
*   全局变量定义
*/
bsp_device_config_table_st  gtv_DeviceConfigTable;


/**
  * @brief  初始化设备配置表信息
  * @param  None
  * @retval None
  */
static void bsp_get_default_device_info(void)
{
    unsigned char i;

    gtv_DeviceConfigTable.mtv_DevInfo.mlv_Head = 0x44455649;

    for(i=0; i<6; i++) {
        gtv_DeviceConfigTable.mtv_DevInfo.mcv_EthernetMac[i] = 0xFF;
    }

    for(i=0; i<6; i++) {
        gtv_DeviceConfigTable.mtv_DevInfo.mcv_WifiMac[i] = 0xFF;
    }

    gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[0] = 'd';
    gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[1] = 'e';
    gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[2] = 'v';
    gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[3] = 'i';
    gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[4] = 'c';
    gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[5] = 'e';
    gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[6] = 'I';
    gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[7] = 'd';
    gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[8] = '0';
    gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[9] = '0';
    gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[10] = '0';
    gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[11] = '1';

    for(i=0; i<12; i++) {
        gtv_DeviceConfigTable.mtv_DevInfo.mcv_DevicePasswd[i] = gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[i];
    }

    gtv_DeviceConfigTable.mtv_DevInfo.mlv_DeviceTypeId = 0xFFFF;

    gtv_DeviceConfigTable.mtv_DevInfo.mlv_Crc32 = calc_crc32((unsigned char *)&gtv_DeviceConfigTable.mtv_DevInfo, (sizeof(bsp_device_info_st)-4));

}

/**
  * @brief  获取设备配置表信息
  * @param  None
  * @retval None
  */
void bsp_get_device_config_table(void)
{
    //PRINTF("Enter bsp_get_device_config_table()\r\n");
    mem_part_info_t ltv_MemInfo =
    {
        //0x0800C000,
        0x6010C000,
        4 * 1024,
    };

    unsigned short i;
    bsp_device_info_st *ltp_DeviceInfo;
    unsigned long llv_Crc32;
    unsigned char *lcp_CharBuff;
#if 1
    /*从flash读取设备配置信息表*/
    ltp_DeviceInfo = (bsp_device_info_st *)ltv_MemInfo.startAddr;
    llv_Crc32 = calc_crc32((unsigned char *)ltp_DeviceInfo, (sizeof(bsp_device_info_st) - 4));
#elif 0

    bsp_device_info_st devInfo;
    DCACHE_CleanInvalidateByRange(ltv_MemInfo.startAddr, FLASH_PAGE_SIZE);
    memcpy(&devInfo, (void*)ltv_MemInfo.startAddr, sizeof(bsp_device_info_st));
    llv_Crc32 = calc_crc32((unsigned char *)&devInfo, (sizeof(bsp_device_info_st) - 4));
    PRINTF("ltp_DeviceInfo->mlv_Head = 0x%x, llv_Crc32=0x%x, ltp_DeviceInfo->mlv_Crc32=0x%x\r\n", devInfo.mlv_Head, llv_Crc32, devInfo.mlv_Crc32);
#endif
    if((ltp_DeviceInfo->mlv_Head != 0x44455649) || (llv_Crc32 != ltp_DeviceInfo->mlv_Crc32))
    //if((devInfo.mlv_Head != 0x44455649) || (llv_Crc32 != devInfo.mlv_Crc32))
    //if (pdTRUE)
    {
        /*数据头或者校验不匹配，使用默认数据*/
#if 1
        bsp_get_default_device_info();
        //bsp_write_device_info_to_flash();  /* 不可以在任务启动前调用该函数 */
#else
        gtv_DeviceConfigTable.mtv_DevInfo.mlv_DeviceTypeId = 0xFFFF;
        
        gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[0] = 'd';
        gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[1] = 'e';
        gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[2] = 'v';
        gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[3] = 'i';
        gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[4] = 'c';
        gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[5] = 'e';
        gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[6] = 'I';
        gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[7] = 'd';
        gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[8] = '0';
        gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[9] = '0';
        gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[10] = '0';
        gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId[11] = '1';
#endif
    }
    else
    {
        lcp_CharBuff = (unsigned char *)&gtv_DeviceConfigTable.mtv_DevInfo;

        for(i = 0; i < sizeof(bsp_device_info_st) - 4; i++)
        {
            lcp_CharBuff[i] = ((unsigned char *)ltp_DeviceInfo)[i];
        }
    }

    gtv_DeviceConfigTable.isSupportSdram = pdFALSE;
    gtv_DeviceConfigTable.mlv_SdramSize = 16 * 1024 * 1024;

    /*软元件数量定义*/
    gtv_DeviceConfigTable.mtv_PlcElementInfo.msv_XElementCnt =  X_ELEMENT_CNT;
    gtv_DeviceConfigTable.mtv_PlcElementInfo.msv_YElementCnt =  Y_ELEMENT_CNT;
    gtv_DeviceConfigTable.mtv_PlcElementInfo.msv_MElementCnt =  M_ELEMENT_CNT;
    gtv_DeviceConfigTable.mtv_PlcElementInfo.msv_LMElementCnt = LM_ELEMENT_CNT;
    gtv_DeviceConfigTable.mtv_PlcElementInfo.msv_SMElementCnt = SM_ELEMENT_CNT;
    gtv_DeviceConfigTable.mtv_PlcElementInfo.msv_SElementCnt =  S_ELEMENT_CNT;

    gtv_DeviceConfigTable.mtv_PlcElementInfo.msv_DElementCnt =  D_ELEMENT_CNT;
    gtv_DeviceConfigTable.mtv_PlcElementInfo.msv_RElementCnt =  R_ELEMENT_CNT;
    gtv_DeviceConfigTable.mtv_PlcElementInfo.msv_SDElementCnt = SD_ELEMENT_CNT;
    gtv_DeviceConfigTable.mtv_PlcElementInfo.msv_VElementCnt =  V_ELEMENT_CNT;
    gtv_DeviceConfigTable.mtv_PlcElementInfo.msv_ZElementCnt =  Z_ELEMENT_CNT;

    gtv_DeviceConfigTable.mtv_PlcElementInfo.msv_CElement.msv_ElementCnt = C_ELEMENT_CNT;
    gtv_DeviceConfigTable.mtv_PlcElementInfo.msv_CElement.msv_16bitCnt = C_ELEMENT_16_BIT_CNT;
    gtv_DeviceConfigTable.mtv_PlcElementInfo.msv_CElement.msv_32bitCnt = C_ELEMENT_32_BIT_CNT;

    gtv_DeviceConfigTable.mtv_PlcElementInfo.msv_TElement.msv_ElementCnt = T_ELEMENT_CNT;
    gtv_DeviceConfigTable.mtv_PlcElementInfo.msv_TElement.msv_100msTimerCnt = T_ELEMENT_100MS_TIMER_CNT;
    gtv_DeviceConfigTable.mtv_PlcElementInfo.msv_TElement.msv_10msTimerCnt = T_ELEMENT_10MS_TIMER_CNT;
    gtv_DeviceConfigTable.mtv_PlcElementInfo.msv_TElement.msv_1msTimerCnt = T_ELEMENT_1MS_TIMER_CNT;

    /*EU/ED 数量定义*/
    gtv_DeviceConfigTable.msv_EuEdNumber = 8 * 1024;

    /*本体是否有I/O口*/
    gtv_DeviceConfigTable.mcv_IsHaveIOPort = 1;
    //gGPIORefreshIO = bsp_refresh_input_output_port;  //2022.3.3

    /*是否支持扩展板*/
    gtv_DeviceConfigTable.mcv_IsSupportExtendBaord = 0;

    /* 串口数量 */
    uint8_t uartNum = 3;
    if (gtv_DeviceConfigTable.mcv_IsHaveIOPort == 1)
    {
       /* 本体输入口数量 */
       gtv_DeviceConfigTable.mcv_SelfInputNum = 8;
       /* 本体输出口数量 */
       gtv_DeviceConfigTable.mcv_SelfOutputNum = 8;
    }

    gtv_DeviceConfigTable.mtv_UartPort.mcv_SupportUartNum = uartNum;
    for (i = 0; i < uartNum; i++)
    {
        gtv_DeviceConfigTable.mtv_UartPort.mta_PortInfo[i].mcv_RS485Mode = 0;
        gtv_DeviceConfigTable.mtv_UartPort.mta_PortInfo[i].mcv_OnDeviceNum = i;
    }

    /*串口0*/
    gtv_DeviceConfigTable.mtv_UartPort.mta_PortInfo[0].mcv_RS485Mode = 0;
    /*串口1*/
    gtv_DeviceConfigTable.mtv_UartPort.mta_PortInfo[1].mcv_RS485Mode = 0;

    /*是否支持掉电保持*/
    if (bspIsSupportLowPowerDetect())
    {
        gtv_DeviceConfigTable.mcv_IsSupportPlsd = 1;
    }
    else
    {
        gtv_DeviceConfigTable.mcv_IsSupportPlsd = 0;
    }
    //PRINTF("Leave bsp_get_device_config_table()\r\n");
}

/**
  * @brief  写设备信息到FLASH
  * @param  None
  * @retval None
  */
void bsp_write_device_info_to_flash(void)
{
    mem_part_info_t ltv_MemInfo =
    {
        0x6010C000,
        4 * 1024,
    };

    bsp_flash_erase_partition(&ltv_MemInfo);

    /*写数据到目标区*/
    bsp_flash_write_buffer(ltv_MemInfo.startAddr, (unsigned char *)&gtv_DeviceConfigTable.mtv_DevInfo, sizeof(bsp_device_info_st));
}

unsigned short bsp_get_deviceID(void)
{
    bsp_device_info_st *ltp_DeviceInfo = (bsp_device_info_st *)0x6010C000;
    return ltp_DeviceInfo->mlv_DeviceTypeId;
}

bool bspIsHaveAD(void)
{
    return false;
}

bool bspIsHave4G(void)
{
#if (KALYKE_FEATURE_4G_TASK == 1)
    #if 0  //用网关配置工具下载时不能下载左扩展标志
        LOGV("bspIsHave4G", "gLeftModules.BIT.HaveCM010 = %u", gLeftModules.BIT.HaveCM010);
        //当调用该函数时，gLeftModules没被赋值
        return (gLeftModules.BIT.HaveCM010 == 1);
    #else
        return true;
    #endif
#else
    return false;
#endif
}

static bool bspIsSupportLowPowerDetect(void)
{
    return true;
}

bool bspIsGateway(void)
{
    return true;
}

