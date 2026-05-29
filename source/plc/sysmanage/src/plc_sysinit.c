/**
  ******************************************************************************
  * @file    plc_sysinit.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   PLC 系统初始化
  ******************************************************************************
  */
#include "FreeRTOS.h"
#include "fsl_debug_console.h"
#include "plc_element.h"
#include "plc_sysinit.h"
#include "bsp_flash.h"
#include "bsp_dct.h"
#include "plc_variable.h"
#include "plc_commonfunc.h"
#include "verify_func.h"
#include "task.h"
#include "list_func.h"
#include "kalyke_tool.h"
#include "plc_password.h"
#include "mb.h"
#include "kalyke_version.h"
#include "mb_download.h"
#include "kalyke_internet_task.h"
#include "kalyke_monitor_task.h"
#include "plc_errormsg.h"
#include "bsp_tim.h"
#include "bsp_iwdg.h"

unsigned long gUcodeLen = 0;

/**
  * @brief  plc 系统资源初始化
  * @param  None
  * @retval None
  */
void plc_sys_init(void)
{
    /*PLC系统各种元件内存申请,初始化*/
    plc_element_init();

    /*全局变量初始化*/
    plc_system_global_var_init();

    /*密码结构体初始化*/
    plc_password_init();
}

/**
  * @brief  系统全局变量初始化
  * @param  None
  * @retval None
  */
void plc_system_global_var_init(void)
{
    LOGV("plc_sysinit", "Enter %s\r\n", __func__);
    unsigned long i;

    /*设置为停止状态, 上电完成后自启动运行*/
    gtv_PlcRunStatus.mcv_PlcRunSwitch = PLC_STOP;
    gtv_PlcRunStatus.mtv_PlcRunStopFlag.byte = 0x00;
    gtv_PlcRunStatus.mcv_PlcCurrentStatus = PLC_STOP_STATUS;

    gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.poweron_auto_run = 1;
    LOGD("plc_sysinit", "mtv_PlcRunStopFlag byte : 0x%X\r\n", gtv_PlcRunStatus.mtv_PlcRunStopFlag.byte);

    /*清除错误标志*/
    guv_StopError.msv_Error = 0x00;
    guv_NonStopError.msv_Error = 0x00;

    /*子程序CALL调用信息结构体内存申请*/
    gtp_CallInsInfoPtr = (plc_call_ins_info_st *)pvPortMalloc(sizeof(plc_call_ins_info_st));
    configASSERT(gtp_CallInsInfoPtr != NULL);

    /*子程序指令列表结构体数组内存申请*/
    gtp_SbrListPtr = (plc_ins_list_head_st *)pvPortMalloc(sizeof(plc_ins_list_head_st) * MAX_SBR_COUNT);
    configASSERT(gtp_SbrListPtr != NULL);

    for(i = 0; i < MAX_SBR_COUNT; i++)
    {
        INIT_LIST_HEAD(&(gtp_SbrListPtr[i].mtv_SimpInsHead));
        INIT_LIST_HEAD(&(gtp_SbrListPtr[i].mtv_CompInsHead));
    }

    /*STL 指令列表结构体数组内存申请*/
    gtp_StlListPtr = (plc_ins_list_head_st *)pvPortMalloc(sizeof(plc_ins_list_head_st) * S_RANG); // * 4096
    configASSERT(gtp_StlListPtr != NULL);

    for(i = 0; i < S_RANG; i++)
    {
        INIT_LIST_HEAD(&gtp_StlListPtr[i].mtv_SimpInsHead);
        INIT_LIST_HEAD(&gtp_StlListPtr[i].mtv_CompInsHead);
    }

    /*MC-MCR主控指令列表结构体初始化*/
    gtv_McMcrBlockInfo.mcv_NestedLevel = 0;
    INIT_LIST_HEAD(&gtv_McMcrBlockInfo.mtv_McMcrHead);


    /*EU/ED 结构体内存申请*/
    gtp_EuEd = (plc_eu_ed_st *)pvPortMalloc(sizeof(unsigned short) * (gtv_DeviceConfigTable.msv_EuEdNumber / 16));//1K
    configASSERT(gtp_EuEd != NULL);

    /*For-Next 结构体内存申请*/
    gtp_ForNextIns = (plc_for_next_ins_st *)pvPortMalloc(sizeof(plc_for_next_ins_st) * MAX_SBR_NESTED_LAYER);
    configASSERT(gtp_ForNextIns != NULL);

    /*SFC 状态结构体内存申请*/
    gtp_SfcStatus = (plc_sfc_run_status_info_st *)pvPortMalloc(sizeof(plc_sfc_run_status_info_st));
    configASSERT(gtp_SfcStatus != NULL);

    /*HOUR 指令结构体*/
    gtp_HourInsSt = (plc_hour_ins_st *)pvPortMalloc(sizeof(plc_hour_ins_st) * 256);
    configASSERT(gtp_HourInsSt != NULL);
    for(i = 0; i < 256; i++)
    {
        gtp_HourInsSt[i].mcv_LastPFStatus = 0;
        gtp_HourInsSt[i].mlv_LastTime = 0;
    }

    /*Duty 指令结构体*/
    gtp_DutyInsInfo = (plc_duty_ins_info_st *)pvPortMalloc(sizeof(plc_duty_ins_info_st));
    configASSERT(gtp_DutyInsInfo);

    /*位监控元件头指针初始化*/
    gtv_MbMonitorBits.lsv_ListLen = 0;
    INIT_LIST_HEAD(&gtv_MbMonitorBits.head);

    /*字监控元件头指针初始化*/
    gtv_MbMonitorWords.lsv_ListLen = 0;
    INIT_LIST_HEAD(&gtv_MbMonitorWords.head);

    /*字位监控元件头指针初始化*/
    gtv_MbMonitorBitsWords.lsv_ListLen = 0;
    INIT_LIST_HEAD(&gtv_MbMonitorBitsWords.head);

    /*位元件强制链表头初始化*/
    gtv_ForceBits.lsv_ListLen = 0;
    INIT_LIST_HEAD(&gtv_ForceBits.head);

    /*字元件强制链表头初始化*/
    gtv_ForceWords.lsv_ListLen = 0;
    INIT_LIST_HEAD(&gtv_ForceWords.head);

    /*Modbus诊断校验信息结构体指针初始化*/
    //gtp_ModbusSlaveDiagInfo = (mb_slave_diagnositic_info_st *)pvPortMalloc(sizeof(mb_slave_diagnositic_info_st) * MB_SENDER_MAX);
    LOGD("plc_sysinit", "gtp_ModbusSlaveDiagInfo = 0x%08X, &gtp_ModbusSlaveDiagInfo[0] = 0x%08X\r\n", gtp_ModbusSlaveDiagInfo, gtp_ModbusSlaveDiagInfo +1);
    //configASSERT(gtp_ModbusSlaveDiagInfo != NULL);
    mb_slave_diag_clear_ctrl_diag_reg();

#if 0
    if(gtv_DeviceConfigTable.mcv_IsSupportPlsd)
    {
        uint32_t address = 0x60110000; // 参看flash_page_info变量
        plc_plsd_st *pPLSD = (plc_plsd_st *)address;
        LOGV("plc_sysinit", "pPLSD->flashHead = 0x%08X", pPLSD->flashHead);
        if (pPLSD->flashHead == 0x44455649)
        {
            memcpy(&gtp_PowerLoseSaveDataInfo, pPLSD, sizeof(gtp_PowerLoseSaveDataInfo));
        }
        else
        {
            memset(&gtp_PowerLoseSaveDataInfo, 0, sizeof(gtp_PowerLoseSaveDataInfo));
        }
        LOGD("plc_sysinit", "sizeof(pPLSD->group1) = %u", sizeof(pPLSD->group1));
    }
#endif
    LOGV("plc_sysinit", "sizeof(gtp_PowerLoseSaveDataInfo) = %u", sizeof(plc_plsd_st));
    LOGV("plc_sysinit", "sizeof(plc_t_element_st) = %u", sizeof(plc_t_element_st));
    LOGV("plc_sysinit", "sizeof(plc_c_element_st) = %u", sizeof(plc_c_element_st));
    LOGV("plc_sysinit", "sizeof(msp_MElement) = %u", sizeof(gtv_PlcElement.msp_MElement));
    LOGV("plc_sysinit", "sizeof(msp_SElement) = %u", sizeof(gtv_PlcElement.msp_SElement));
    LOGV("plc_sysinit", "sizeof(msp_DElement) = %u", sizeof(gtv_PlcElement.msp_DElement));
}

/**
  * @brief  EU/ED 初始化
  * @param  None
  * @retval None
  */
void plc_eu_ed_init(void)
{
    unsigned short i;

    for(i = 0; i < (gtv_DeviceConfigTable.msv_EuEdNumber / 16); i++)
    {
        gtp_EuEd[i].msv_Value = 0;
        gtp_EuEd[i].msv_IsInit = 0xFFFF;
    }
}

void plc_HSP_init(void)
{
    unsigned short i;
    for (i = 0; i < MAX_DRVI_OUTPUT_CHANNEL_NUM; i++)
    {
        memset(&gHspChannelInfo[i], 0, sizeof(bsp_hsp_ins_channel_info_st));
        gHspChannelInfo[i].edgeID = 0xFFFF;
    }
}



/**
  * @brief  特殊SM&SD元件初始化
  * @param  None
  * @retval None
  */
void plc_sm_sd_init(void)
{
    LOGV("plc_sysinit", "Enter %s()\r\n", __func__);
    unsigned short i;
    /*初始化SM元件*/
    for(i = 0; i < SM_RANG / 16; i++)
    {
        gtv_PlcElement.msp_SMElement[i] = 0;
    }

    SET_UART_SM_FLAG(0, UART_SM_IDLE);
    SET_UART_SM_FLAG(1, UART_SM_IDLE);

    /*特殊SM设置*/
    if (gWanOK)
    {
        plc_set_bit_element_value(SM_ELEMENT, SM269, 1);
    }
    if (gLanOK)
    {
        plc_set_bit_element_value(SM_ELEMENT, SM270, 1);
    }

    /*特殊SD设置*/

    /*PLC 类型*/
    SET_SD_ELEMENT_VALUE(SD0, gtv_DeviceConfigTable.mtv_DevInfo.mlv_DeviceTypeId);
    /*软件版本号*/
    i = FIRMWARE_IMAGE_ID;
    SET_SD_ELEMENT_VALUE(SD1, i);
    /*用户程序容量 K步*/
    SET_SD_ELEMENT_VALUE(SD2, 128);
    LOGD("plc_sysinit", "Version:%u, Rongliang:%u\r\n", GET_SD_ELEMENT_VALUE(1), GET_SD_ELEMENT_VALUE(2));

    /*特殊SD设置*/
    SET_SD_ELEMENT_VALUE(SD6,gResetReason);
    SET_SD_ELEMENT_VALUE(SD7,0x00);
    SET_SD_ELEMENT_VALUE(SD30, 0x00);
    SET_SD_ELEMENT_VALUE(SD31, 0x00);
    SET_SD_ELEMENT_VALUE(SD32, 0x00);

    SET_SD_ELEMENT_VALUE(SD258, g_plc_netcfg.wan.ioExp);
    SET_SD_ELEMENT_VALUE(SD259, g_plc_netcfg.lan.ioExp);

    uint32_t *pSD = (uint32_t *)&gtv_PlcElement.msp_SDElement[SD213];
    *pSD = bsp_get_kalyke_boot_time();
    pSD = (uint32_t *)&gtv_PlcElement.msp_SDElement[SD207];
    *pSD = FIRMWARE_IMAGE_ID;
    init_SD_SW_version();
}

/**
  * @brief  Ucode校验
  * @param  isSetSysFlag 是否设置UCODE错误标志
  * @retval None
  */
void plc_ucode_verify(unsigned char isSetUcodeErrFlag)
{
    LOGV("plc_sysinit", "Enter %s()\r\n", __func__);
    unsigned char *mcp_UcodePtr = NULL;
    flash_part_info_t *ltp_UCodeInfo;

    ltp_UCodeInfo = bsp_get_flash_info(UCODE_FILE_START_PAGE);
    LOGD("plc_sysinit", "ltp_UCodeInfo = 0x%08x, &ltp_UCodeInfo = 0x%08x\r\n", ltp_UCodeInfo, &ltp_UCodeInfo);
    if(ltp_UCodeInfo == (flash_part_info_t *)0)
    {
        LOGE("plc_sysinit", "%s: Error 1\r\n", __func__);
        /*置用户程序错误标志*/
        if(isSetUcodeErrFlag)
        {
            guv_StopError.bit.ucode_err = 1;
            plc_refresh_error_msg(ERR_USER_PROGRAM);
        }
        gtv_UserFilePtrSt.UCodePtr = (unsigned char *)ucode_default;
    }

    mcp_UcodePtr = (unsigned char *)ltp_UCodeInfo->startAddr;
    LOGD("plc_sysinit", "mcp_UcodePtr = 0x%x\r\n", mcp_UcodePtr);
    
    gUcodeLen = plc_get_file_length(mcp_UcodePtr + FILE_LEN_INFO_START_INDEX, 4);
    LOGD("plc_sysinit", "len = %u\r\n", gUcodeLen);
    if (gUcodeLen < 1024)
    {
        hexdump(mcp_UcodePtr, gUcodeLen);
    }
    else
    {
        hexdump(mcp_UcodePtr, 1024);
    }

    if(gUcodeLen == 0xFFFFFFFF)  //first on
    {
        LOGE("plc_sysinit", "Ucode empty...");
        /*置UCODE错误标志*/
        if(isSetUcodeErrFlag)
        {
            //guv_StopError.bit.ucode_err = 1;
            //plc_refresh_error_msg(ERR_USER_PROGRAM);
            plc_refresh_error_msg(ERR_NULL_UCODE);
        }
        gtv_UserFilePtrSt.UCodePtr = (unsigned char *)ucode_default;
    }
    else if(GET_PU16_DATA(mcp_UcodePtr) != FILE_START_CHARACTER ||
            GET_PU16_DATA(mcp_UcodePtr + gUcodeLen - 2) != FILE_TAIL_CHARACTER)
    {
        LOGE("plc_sysinit", "%s: Error 2\r\n", __func__);
        /*置UCODE错误标志*/
        if(isSetUcodeErrFlag)
        {
            guv_StopError.bit.ucode_err = 1;
            plc_refresh_error_msg(ERR_USER_PROGRAM);
        }
        gtv_UserFilePtrSt.UCodePtr = (unsigned char *)ucode_default;

    }
    else if(calc_ccitt(mcp_UcodePtr, gUcodeLen - 4) != GET_BIGPU16_DATA(mcp_UcodePtr + gUcodeLen - 4))
    {
        LOGE("plc_sysinit", "%s: Error 3\r\n", __func__);
        /*置UCODE错误标志*/
        if(isSetUcodeErrFlag)
        {
            guv_StopError.bit.ucode_err = 1;
            plc_refresh_error_msg(ERR_USER_PROGRAM);
        }
        gtv_UserFilePtrSt.UCodePtr = (unsigned char *)ucode_default;

    }
    else
    {
        LOGV("plc_sysinit", "%s: Success!", __func__);
        /*清除UCODE错误标志*/
        guv_StopError.bit.ucode_err = 0;
        #if 1
        gtv_UserFilePtrSt.UCodePtr = (unsigned char *)ltp_UCodeInfo->startAddr;
        #else
        if ((uint32_t)gtv_UserFilePtrSt.UCodePtr > 0x20200000u && (uint32_t)gtv_UserFilePtrSt.UCodePtr < 0x202C0000u)
        {
            vPortFree(gtv_UserFilePtrSt.UCodePtr);
        }
        gtv_UserFilePtrSt.UCodePtr = pvPortMalloc(len + 512);
        memcpy(gtv_UserFilePtrSt.UCodePtr, (void*)ltp_UCodeInfo->startAddr, len);
        //hexdump(gtv_UserFilePtrSt.UCodePtr, len);
        #endif
    }
    LOGV("plc_sysinit", "gtv_UserFilePtrSt.UCodePtr = 0x%08X\r\n", gtv_UserFilePtrSt.UCodePtr);
}

/**
  * @brief  数据块校验
  * @param  flag 是否设置用户程序错误标志
  * @retval None
  */
void plc_data_block_verify(unsigned char flag)
{
    flash_part_info_t *ltp_DataBlock;
    unsigned char *mcp_DataBlockPtr;
    unsigned long len;

    ltp_DataBlock = bsp_get_flash_info(DATA_BLOCK_START_PAGE);
    LOGV("plc_sysinit", "Enter %s(), ltp_DataBlock = 0x%x\r\n", __func__, ltp_DataBlock);
    if(ltp_DataBlock == (flash_part_info_t *)0)
    {

        /*置用户程序错误标志*/
        if(flag)
        {
            guv_StopError.bit.datablock_err = 1;
            plc_refresh_error_msg(ERR_DATA_BLOCK);
        }
        gtv_UserFilePtrSt.DataBlockPtr = (unsigned char *)datablock_default;
    }

    mcp_DataBlockPtr = (unsigned char *)ltp_DataBlock->startAddr;
    hexdump(mcp_DataBlockPtr, 32);
    len = plc_get_file_length(mcp_DataBlockPtr + FILE_LEN_INFO_START_INDEX, 4);
    LOGD("plc_sysinit", "mcp_DataBlockPtr = 0x%x, len = %d\r\n", mcp_DataBlockPtr, len);
    hexdump(mcp_DataBlockPtr + len - 16, 18);
    unsigned short fileStart = GET_PU16_DATA(mcp_DataBlockPtr);
    unsigned short fileEnd = GET_PU16_DATA(mcp_DataBlockPtr + len - 2);
    LOGD("plc_sysinit", "fileStart = 0x%X, fileEnd = 0x%X\r\n", fileStart, fileEnd);

    if(len == 0xFFFFFFFF)  //first on
    {
        LOGE("plc_sysinit", "Data block empty...");
        /*置用户程序错误标志*/
        if(flag)
        {
            //guv_StopError.bit.datablock_err = 1;
            //plc_refresh_error_msg(ERR_DATA_BLOCK);
            plc_refresh_error_msg(ERR_NULL_DATA_BLOCK);
        }
        gtv_UserFilePtrSt.DataBlockPtr = (unsigned char *)datablock_default;
    }
    else if(fileStart != FILE_START_CHARACTER || fileEnd != FILE_TAIL_CHARACTER)
    {
        LOGE("plc_sysinit", "Error 1\r\n");
        /*置用户程序错误标志*/
        if(flag)
        {
            guv_StopError.bit.datablock_err = 1;
            plc_refresh_error_msg(ERR_DATA_BLOCK);
        }
        gtv_UserFilePtrSt.DataBlockPtr = (unsigned char *)datablock_default;

    }
    else if(calc_ccitt(mcp_DataBlockPtr, len - 4) !=  GET_BIGPU16_DATA(mcp_DataBlockPtr + len - 4))
    {
        LOGE("plc_sysinit", "Error 2\r\n");
        /*置用户程序错误标志*/
        if(flag)
        {
            guv_StopError.bit.datablock_err = 1;
            plc_refresh_error_msg(ERR_DATA_BLOCK);
        }
        gtv_UserFilePtrSt.DataBlockPtr = (unsigned char *)datablock_default;

    }
    else
    {
        LOGD("plc_sysinit", "Success 1! \r\n");
        /*清除用户程序错误标志*/
        guv_StopError.bit.datablock_err = 0;
        gtv_UserFilePtrSt.DataBlockPtr = (unsigned char *)ltp_DataBlock->startAddr;
    }
    LOGD("plc_sysinit", "plc_data_block_verify SUCCESS!! DataBlockPtr = 0x%08X\r\n", gtv_UserFilePtrSt.DataBlockPtr);
}

/**
  * @brief  POU INFO校验
  * @param  flag 是否设置POU INFO错误标志
  * @retval None
  */
void plc_pou_info_verify(unsigned char flag)
{
    flash_part_info_t *ltp_PouInfo;
    unsigned char *mcp_PouInfoPtr;
    unsigned long len;

    ltp_PouInfo = bsp_get_flash_info(POU_FILE_START_PAGE);

    if(ltp_PouInfo == (flash_part_info_t *)0)
    {

        /*置POU INFO错误标志*/
        if(flag)
        {
            guv_NonStopError.bit.pou_info_err = 1;
        }
        gtv_UserFilePtrSt.PouInfoPtr = (unsigned char *)pouinfo_default;
    }

    mcp_PouInfoPtr = (unsigned char *)ltp_PouInfo->startAddr;
    len = plc_get_file_length(mcp_PouInfoPtr + FILE_LEN_INFO_START_INDEX, 4);

    if(len == 0xFFFFFFFF)  //first on
    {
        LOGE("plc_sysinit", "POU info empty...");
        /*置POU INFO错误标志*/
        if(flag)
        {
            //guv_NonStopError.bit.pou_info_err = 1;
            plc_refresh_error_msg(ERR_NULL_POU_INFO);
        }
        gtv_UserFilePtrSt.PouInfoPtr = (unsigned char *)pouinfo_default;
    }
    else if(GET_PU16_DATA(mcp_PouInfoPtr) != FILE_START_CHARACTER ||
            GET_PU16_DATA(mcp_PouInfoPtr + len - 2) != FILE_TAIL_CHARACTER)
    {

        /*置POU INFO错误标志*/
        if(flag)
        {
            guv_NonStopError.bit.pou_info_err = 1;
        }
        gtv_UserFilePtrSt.PouInfoPtr = (unsigned char *)pouinfo_default;

    }
    else if(calc_ccitt(mcp_PouInfoPtr, len - 4) !=  GET_BIGPU16_DATA(mcp_PouInfoPtr + len - 4))
    {

        /*置POU INFO错误标志*/
        if(flag)
        {
            guv_NonStopError.bit.pou_info_err = 1;
        }
        gtv_UserFilePtrSt.PouInfoPtr = (unsigned char *)pouinfo_default;

    }
    else
    {

        /*清除POU INFO错误标志*/
        guv_NonStopError.bit.pou_info_err = 0;
        gtv_UserFilePtrSt.PouInfoPtr = (unsigned char *)ltp_PouInfo->startAddr;
    }

}

/**
  * @brief  系统块校验
  * @param  flag 是否设置系统块错误标志
  * @retval None
  */
void plc_system_block_verify(unsigned char flag)
{
    flash_part_info_t *ltp_SystemBlock;
    unsigned char *mcp_SystemBlockPtr;
    unsigned long len;

    ltp_SystemBlock = bsp_get_flash_info(SYS_BLOCK_START_PAGE);

    if(ltp_SystemBlock == (flash_part_info_t *)0)
    {
        LOGE("plc_sysinit", "System block Error......00");
        /*置系统块错误标志*/
        if(flag)
        {
            guv_StopError.bit.sysblock_err = 1;
            plc_refresh_error_msg(ERR_SYSTEM_BLOCK);
        }
        gtv_UserFilePtrSt.SysBlockPtr = (unsigned char *)sysblock_default;
    }

    mcp_SystemBlockPtr = (unsigned char *)ltp_SystemBlock->startAddr;
    len = plc_get_file_length(mcp_SystemBlockPtr + FILE_LEN_INFO_START_INDEX, 4);
    LOGD("plc_sysinit", "mcp_SystemBlockPtr = 0x%08X, len = %u\r\n", mcp_SystemBlockPtr, len);

    if(len == 0xFFFFFFFF)  //first on
    {
        LOGE("plc_sysinit", "System block empty...");
        /*置系统块错误标志*/
        if(flag)
        {
            //guv_StopError.bit.sysblock_err = 1;
            //plc_refresh_error_msg(ERR_SYSTEM_BLOCK);
            plc_refresh_error_msg(ERR_NULL_SYSTEM_BLOCK);
        }
        gtv_UserFilePtrSt.SysBlockPtr = (unsigned char *)sysblock_default;
    }
    else if(GET_PU16_DATA(mcp_SystemBlockPtr) != FILE_START_CHARACTER ||
            GET_PU16_DATA(mcp_SystemBlockPtr + len - 2) != FILE_TAIL_CHARACTER)
    {
        LOGE("plc_sysinit", "System block Error......01");
        /*置系统块错误标志*/
        if(flag)
        {
            guv_StopError.bit.sysblock_err = 1;
            plc_refresh_error_msg(ERR_SYSTEM_BLOCK);
        }
        gtv_UserFilePtrSt.SysBlockPtr = (unsigned char *)sysblock_default;

    }
    else if(calc_ccitt(mcp_SystemBlockPtr, len - 4) !=  GET_BIGPU16_DATA(mcp_SystemBlockPtr + len - 4))
    {
        LOGE("plc_sysinit", "System block Error......02");
        /*置系统块错误标志*/
        if(flag)
        {
            guv_StopError.bit.sysblock_err = 1;
            plc_refresh_error_msg(ERR_SYSTEM_BLOCK);
        }
        gtv_UserFilePtrSt.SysBlockPtr = (unsigned char *)sysblock_default;
    }
    else
    {
        /*清除系统块错误标志*/
        guv_StopError.bit.sysblock_err = 0;
        gtv_UserFilePtrSt.SysBlockPtr = (unsigned char *)ltp_SystemBlock->startAddr;
        //hexdump(mcp_SystemBlockPtr, len);
    }
    LOGV("plc_sysinit", "Leave %s(), guv_StopError.bit.sysblock_err = %u", __func__, guv_StopError.bit.sysblock_err);
}

/**
  * @brief  网络参数块校验
  * @param  flag 是否设置网络参数块错误标志
  * @retval None
  */
void plc_netcfg_block_verify(unsigned char flag)
{
    LOGV("plc_sysinit", "Enter %s()\r\n", __func__);
    flash_part_info_t *ltp_NetcfgBlock;
    unsigned char *mcp_NetcfgBlockPtr;
    unsigned long len;

    LOGD("plc_sysinit", "sizeof(MB_DL_NETCFG) = %u\r\n", sizeof(MB_DL_NETCFG));
    ltp_NetcfgBlock = bsp_get_flash_info(DEV_NETWORK_INFO_START_FLASH_PAGE);

    if(ltp_NetcfgBlock == (flash_part_info_t *)0)
    {
        LOGE("plc_sysinit", "%s(): Error...001\r\n", __func__);
        /*置网络块错误标志*/
        if(flag)
        {
            guv_StopError.bit.netcfg_err = 1;
            plc_refresh_error_msg(ERR_NET_CONFIG);
        }
        gtv_UserFilePtrSt.NetcfgBlockPtr = (unsigned char*)netcfg_default;
        return;
    }

    mcp_NetcfgBlockPtr = (unsigned char *)ltp_NetcfgBlock->startAddr;
    len = plc_get_file_length(mcp_NetcfgBlockPtr + FILE_LEN_INFO_START_INDEX, 4);
    LOGD("plc_sysinit", "mcp_NetcfgBlockPtr = 0x%08X, len = %u\r\n", mcp_NetcfgBlockPtr, len);

    if(len == 0xFFFFFFFF)  //first on
    {
        LOGE("plc_sysinit", "NetCfg empty...");
        /*置网络块错误标志*/
        if(flag)
        {
            //guv_StopError.bit.netcfg_err = 1;
            //plc_refresh_error_msg(ERR_NET_CONFIG);
            plc_refresh_error_msg(ERR_NULL_NET_CONFIG);
        }
        gtv_UserFilePtrSt.NetcfgBlockPtr = (unsigned char *)netcfg_default;
        return;
    }
    else if (GET_PU16_DATA(mcp_NetcfgBlockPtr) != FILE_START_CHARACTER ||
            GET_PU16_DATA(mcp_NetcfgBlockPtr + len - 2) != FILE_TAIL_CHARACTER)
    {
        LOGE("plc_sysinit", "%s(): Error...002\r\n", __func__);
        /*置网络块错误标志*/
        if(flag)
        {
            guv_StopError.bit.netcfg_err = 1;
            plc_refresh_error_msg(ERR_NET_CONFIG);
        }
        gtv_UserFilePtrSt.NetcfgBlockPtr = (unsigned char *)netcfg_default;
        return;
    }
    //hexdump1(mcp_NetcfgBlockPtr, len);
    uint32_t crc32 = calc_crc32(mcp_NetcfgBlockPtr, len - 6);
    uint32_t recvCrc32 = GET_BIGPU32_DATA(mcp_NetcfgBlockPtr + len - 6);
    LOGD("plc_sysinit", "crc32 = 0x%08X, recvCrc32 = 0x%08X\r\n", crc32, recvCrc32);
    if(crc32 != recvCrc32)
    {
        LOGE("plc_sysinit", "%s(): CRC32 check ERROR...003\r\n", __func__);
        /*置网络块错误标志*/
        if(flag)
        {
            guv_StopError.bit.netcfg_err = 1;
            plc_refresh_error_msg(ERR_NET_CONFIG);
        }
        gtv_UserFilePtrSt.NetcfgBlockPtr = (unsigned char *)netcfg_default;
        return;
    }

    /*清除网络块错误标志*/
    guv_StopError.bit.netcfg_err = 0;
    guv_NonStopError.bit.netconfig_err = 0;
    gtv_UserFilePtrSt.NetcfgBlockPtr = (unsigned char *)ltp_NetcfgBlock->startAddr;
    LOGD("plc_sysinit", "plc_netcfg_block_verify SUCCESS!\r\n");
}

/**
  * @brief  全局变量表校验
  * @param  flag 是否设置GVT错误标志
  * @retval None
  */
void plc_gvt_verify(unsigned char flag)
{
    flash_part_info_t *ltp_Gvt;
    unsigned char *mcp_GvtPtr;
    unsigned long len;

    ltp_Gvt = bsp_get_flash_info(GVT_FILE_START_PAGE);

    if(ltp_Gvt == (flash_part_info_t *)0)
    {

        /*置GVT错误标志*/
        if(flag)
        {
            guv_NonStopError.bit.gvt_err = 1;
        }
        gtv_UserFilePtrSt.GvtPtr = (unsigned char *)gvt_default;
    }

    mcp_GvtPtr = (unsigned char *)ltp_Gvt->startAddr;
    len = plc_get_file_length(mcp_GvtPtr + FILE_LEN_INFO_START_INDEX, 4);

    if(len == 0xFFFFFFFF)  //first on
    {
        LOGE("plc_sysinit", "GVT block empty...");
        /*置GVT错误标志*/
        if(flag)
        {
            //guv_NonStopError.bit.gvt_err = 1;
            plc_refresh_error_msg(ERR_NULL_GVT);
        }
        gtv_UserFilePtrSt.GvtPtr = (unsigned char *)gvt_default;
    }
    else if(GET_PU16_DATA(mcp_GvtPtr) != FILE_START_CHARACTER ||
            GET_PU16_DATA(mcp_GvtPtr + len - 2) != FILE_TAIL_CHARACTER)
    {

        /*置GVT错误标志*/
        if(flag)
        {
            guv_NonStopError.bit.gvt_err = 1;
        }
        gtv_UserFilePtrSt.GvtPtr = (unsigned char *)gvt_default;

    }
    else if(calc_ccitt(mcp_GvtPtr, len - 4) !=  GET_BIGPU16_DATA(mcp_GvtPtr + len - 4))
    {

        /*置GVT错误标志*/
        if(flag)
        {
            guv_NonStopError.bit.gvt_err = 1;
        }
        gtv_UserFilePtrSt.GvtPtr = (unsigned char *)gvt_default;

    }
    else
    {

        /*清除GVT错误标志*/
        guv_NonStopError.bit.gvt_err = 0;
        gtv_UserFilePtrSt.GvtPtr = (unsigned char *)ltp_Gvt->startAddr;
    }

}

/**
  * @brief  定时脉冲指令DUTY数据结构初始化
  * @param  None
  * @retval None
  */
void plc_duty_ins_init(void)
{
    unsigned short i;

    configASSERT(gtp_DutyInsInfo != NULL);

    for(i = 0; i < MAX_DUTY_SUPPORT_NUM; i++)
    {
        gtp_DutyInsInfo->mcv_IsEnable[i] = 0;
        gtp_DutyInsInfo->msv_OnTime[i] = 0;
        gtp_DutyInsInfo->msv_OffTime[i] = 0;
    }
}

