/**
  ******************************************************************************
  * @file    plc_executer.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   PLC运行中相关函数定义
  ******************************************************************************
  */
#include "FreeRTOS.h"
#include "fsl_debug_console.h"
#include "plc_variable.h"
#include "plc_commonfunc.h"
#include "plc_executer.h"
#include "plc_element.h"
#include "plc_sysinit.h"
#include "bsp_dct.h"
#include "bsp_tim.h"
#include "bsp_gpio.h"
#include "bsp.h"
#include "bsp_flash.h"
#include "bsp_led.h"

#include "plc_sysblock.h"
#include "plc_compiler.h"
#include "plc_timeins.h"
#include "plc_errormsg.h"
#include "plc_internalmanage.h"
#include "plc_simpleins.h"
#include "plc_complexins.h"
#include "plc_instruction.h"

#include "plc_password.h"
#include "plc_interrupt.h"
#include "plsd_task.h"
#include "pid_task.h"
#include "plc_netcfg.h"
#include "kalyke_ad_task.h"
#include "kalyke_internet_task.h"
#include "kalyke_tool.h"
#include "kalyke_monitor_task.h"
#include "high_count.h"
#include "plc_spd.h"
#include "plc_io_interrupt.h"
#include "daisy_task.h"
#include "kalyke_event.h"
#include "kalyke_4G_task.h"
#if (KALYKE_MODBUS_TCP_SHEET == 1)
#include "kalyke_modbus_tcp.h"
#include "kalyke_modbus_com.h"
#endif
#include "plc_highspeedins.h"


static const char *TAG = "plc_executer";

unsigned char *gcp_BackupUserPc;
#define PLC_EXECUTER_DEBUG    0

/**
  * @brief  切换plc系统运行装态
  * @param  None
  * @retval None
  */
void plc_sys_status_switch(void)
{
    unsigned char lcv_RunInputPointX;

#if (PLC_EXECUTER_DEBUG == 1)
    static uint32_t cont = 0;
    if (cont++ % 2000 == 0)
    {
        LOGD(TAG, "byte = 0x%X", gtv_PlcRunStatus.mtv_PlcRunStopFlag.byte);
    }
#endif
    if (gtv_PlcRunStatus.mtv_PlcRunStopFlag.byte)
    {
        if(gtv_PlcRunStatus.mtv_PlcRunStopFlag.byte & 0x0F) {
            gtv_PlcRunStatus.mcv_PlcRunSwitch = PLC_RUN;
        }

        if(gtv_PlcRunStatus.mtv_PlcRunStopFlag.byte & 0xF0) {
            gtv_PlcRunStatus.mcv_PlcRunSwitch = PLC_STOP;
        }
    }

    /*输入点启动运行*/
    if (gtv_PlcRunStatus.mcv_PlcCurrentStatus == PLC_STOP_STATUS) 
    {
        unsigned char temp = plc_get_bit_element_value(SM_ELEMENT, 9);
        #if (PLC_EXECUTER_DEBUG == 1)
        if (cont % 2001 == 0)
        {
            LOGD(TAG, "temp = 0x%X", temp);
        }
        #endif
        if (temp)
        {
            lcv_RunInputPointX = plc_get_bit_element_value(X_ELEMENT, GET_SD_ELEMENT_VALUE(9));
            #if (PLC_EXECUTER_DEBUG == 1)
            if (cont % 2001 == 0)
            {
                LOGV(TAG, "lcv_RunInputPointX = 0x%X", lcv_RunInputPointX);
            }
            #endif
            if(lcv_RunInputPointX) 
            {
                gtv_PlcRunStatus.mcv_PlcRunSwitch = PLC_RUN;
                gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.input_point_run = 1;
                gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.cmd_stop = 0;
                gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.error_status_stop = 0;
                gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.cmd_reboot = 0;
            }
        }
    }

    switch(gtv_PlcRunStatus.mcv_PlcCurrentStatus) {
        case PLC_STOP_STATUS:
            if(gtv_PlcRunStatus.mcv_PlcRunSwitch == PLC_RUN) {
                gtv_PlcRunStatus.mcv_PlcCurrentStatus = PLC_STOP_TO_RUN_STATUS;
            }
            break;

        case PLC_STOP_TO_RUN_STATUS:
            if(gtv_PlcRunStatus.mcv_PlcRunSwitch == PLC_STOP) {
                gtv_PlcRunStatus.mcv_PlcCurrentStatus = PLC_RUN_TO_STOP_STATUS;
            }
            break;

        case PLC_RUN_STATUS:
            if(gtv_PlcRunStatus.mcv_PlcRunSwitch == PLC_STOP) {
                gtv_PlcRunStatus.mcv_PlcCurrentStatus = PLC_RUN_TO_STOP_STATUS;
            }
            break;

        case PLC_RUN_TO_STOP_STATUS:
            LOGI(TAG, "OMG!!!PLC WILL GO TO STOP");
            if(gtv_PlcRunStatus.mcv_PlcRunSwitch == PLC_RUN) {
                gtv_PlcRunStatus.mcv_PlcCurrentStatus = PLC_STOP_TO_RUN_STATUS;
            } else {
                gtv_PlcRunStatus.mcv_PlcCurrentStatus = PLC_STOP_STATUS;
            }
            break;

        default:
            /*未知状态停机*/
            gtv_PlcRunStatus.mcv_PlcCurrentStatus = PLC_STOP_STATUS;
    }
}

/**
  * @brief  PLC状态由停止到运行
  * @param  None
  * @retval None
  */

void plc_status_stop_to_run(void)
{
    LOGV(TAG, "Enter %s", __func__);
    unsigned char lcv_SM2Backup;
    unsigned char lcv_ComplierRet;

    LOGV(TAG, "&lcv_SM2Backup = 0x%08x, &lcv_ComplierRet = 0x%08x", &lcv_SM2Backup, &lcv_ComplierRet);

    LOGV(TAG, "Dump some D element:");
    hexdump(gtv_PlcElement.msp_DElement, 128);
    
    bsp_close_err_led();
    /*备份SM2*/
    lcv_SM2Backup = plc_get_bit_element_value(SM_ELEMENT, 2);

    /*未设置元件值保持,初始化所有元件*/
    if(guv_PlcSysBlkAdSetting.bit.keep_element_value == 0)
    {
        LOGV(TAG, "%s: 010. heap:%d", __func__, xPortGetFreeHeapSize());
        plc_element_value_init();
        /*特殊SM & SD 元件初始化*/
        plc_sm_sd_init();
    }
    LOGI(TAG, "Dump some D element:");
    hexdump(gtv_PlcElement.msp_DElement, 128);

    /*清除用户程序运行错误*/
    plc_clean_user_run_error_flag();
    LOGV(TAG, "%s: 000. heap:%d", __func__, xPortGetFreeHeapSize());
    /*在线模式不需要重新校验UCODE & POUINFO*/
    if(!gtv_PlcRunStatus.mcv_IsOnlineProgram) {
        LOGV(TAG, "%s: 001. heap:%d", __func__, xPortGetFreeHeapSize());
        /*UCODE校验*/
        plc_ucode_verify(1);

        /*POU INFO 校验*/
        plc_pou_info_verify(1);
    }

    LOGV(TAG, "%s: 002. heap:%d", __func__, xPortGetFreeHeapSize());
    /*数据块文件校验*/
    plc_data_block_verify(1);

    LOGV(TAG, "%s: 003. heap:%d", __func__, xPortGetFreeHeapSize());
    /*系统块校验*/
    plc_system_block_verify(1);
    /*网络配置块校验*/

    plc_netcfg_block_verify(1);
    LOGV(TAG, "%s: 004. heap:%d", __func__, xPortGetFreeHeapSize());
    /*GVT校验*/
    plc_gvt_verify(1);


    LOGV(TAG, "%s: 005. heap:%d", __func__, xPortGetFreeHeapSize());
    /*读密码数据*/
    plc_read_password_info();

    LOGV(TAG, "%s: 006. heap:%d", __func__, xPortGetFreeHeapSize());
    /*用户中断初始化*/
    plc_user_interrupt_init();

    LOGV(TAG, "%s: 007. heap:%d", __func__, xPortGetFreeHeapSize());
    #if 0
    /*无电池模式,或者电池电压过低*/
    if(gtv_DeviceConfigTable.mcv_IsSupportBattery == 0) {
        LOGV(TAG, "%s: 008. heap:%d", __func__, xPortGetFreeHeapSize());
        guv_PlcSysBlkAdSetting.bit.no_battery_mode = 1;
        plc_element_value_init();
        /*特殊SM & SD 元件初始化*/
        plc_sm_sd_init();
    }
    #endif
    LOGV(TAG, "%s: 009. heap:%d", __func__, xPortGetFreeHeapSize());
    
    LOGV(TAG, "%s: 011. heap:%d", __func__, xPortGetFreeHeapSize());
    /*清除系统块错误,解析系统块*/
    //guv_StopError.bit.sysblock_err = 0;
    char ret = plc_parse_system_block(0);
    if (ret == pdPASS)
    {
        LOGV(TAG, "plc_parse_system_block SUCCESS!!!");
    }
    else
    {
        LOGE(TAG, "plc_parse_system_block FAIL !!!");
        guv_StopError.bit.sysblock_err = 1;
        plc_refresh_error_msg(ERR_SYSTEM_BLOCK);
    }

    //解析通讯配置串口配置
    ret = plc_parse_netcfg_block_OnlyUart();
    if (ret == pdPASS)
    {
        LOGV(TAG, "plc_parse_netcfg_block_OnlyUart SUCCESS!");
    }
    else
    {
        LOGE(TAG, "plc_parse_netcfg_block_OnlyUart FAIL!");
        guv_NonStopError.bit.netconfig_err = 1;
        plc_refresh_error_msg(ERR_NET_CONFIG);
    }
    LOGV(TAG, "%s: 012. heap:%d", __func__, xPortGetFreeHeapSize());

    LOGV(TAG, "Dump some M element:");
    hexdump(gtv_PlcElement.msp_MElement, 128);
    if (pid_open_check())
    {
        pid_read_parameter_2();       //PID参数2从Flash读到 D元件
    }
    if (guv_PlcSysBlkAdSetting.bit.no_battery_mode == 0)
    {
        /*掉电保持数据恢复*/
        plsd_restore_data();
    }
    LOGV(TAG, "%s: 013. heap:%d", __func__, xPortGetFreeHeapSize());
    /*数据块有效,写入数据块数据*/
    if(guv_PlcSysBlkAdSetting.bit.data_block_valid) {
        LOGV(TAG, "%s: 014. heap:%d", __func__, xPortGetFreeHeapSize());
        plc_restore_data_block();
    }
    LOGW(TAG, "Dump some M element:");
    hexdump(gtv_PlcElement.msp_MElement, 128);
    LOGV(TAG, "%s: 015", __func__);
    /*清除第0层LM元件*/
    plc_clean_layer_lm_element(0);

    LOGV(TAG, "%s: 017", __func__);
    /*编译 UCODE*/
    lcv_ComplierRet = plc_compiler_ucode(gtv_UserFilePtrSt.UCodePtr);

    LOGV(TAG, "%s: 018", __func__);
    if(lcv_ComplierRet != pdPASS) {
        LOGE(TAG, "%s: 019", __func__);
        gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.error_status_stop = 1;
        /*错误信息处理*/
        plc_refresh_error_msg(ERR_COMPILER);

    } else {
        LOGV(TAG, "%s: 020", __func__);
        plc_set_bit_element_value(SM_ELEMENT, 0, 1);
        plc_set_bit_element_value(SM_ELEMENT, 1, 1);
        plc_set_bit_element_value(SM_ELEMENT, 2, lcv_SM2Backup);

        /*处理扫描时间相关SD元件*/
        SET_SD_ELEMENT_VALUE(SD31, 1000);
        SET_SD_ELEMENT_VALUE(SD32, 0);

        /*初始化定时器*/
        plc_stop_all_timer();
        /*时钟振荡源初始化*/
        plc_clock_cycle_init();

    }

    LOGV(TAG, "%s: 021", __func__);
    plc_reset_signal_alarm_element();

    LOGV(TAG, "%s: 022", __func__);
    /*Duty指令信息结构体初始化*/
    plc_duty_ins_init();

    LOGV(TAG, "%s: 023", __func__);
    /*计时表指令数据初始化*/
    plc_hour_ins_data_init();

    LOGV(TAG, "%s: 024", __func__);
    /*EU/ED沿初始化*/
    plc_eu_ed_init();
    /*高速指令变量初始化*/
    plc_HSP_init();

    LOGV(TAG, "%s: 025", __func__);
    /*使能时钟振荡源*/
    bsp_cycle_clock_enable(1);

    spd_init();
    highcount_poweron_init();
    kalyke_highspped_init();
//  kalyke_monitor_init();
    SET_SD_ELEMENT_VALUE(SD231, gTouChuan);
    //kalyke_daisy_init();

#if (KALYKE_FEATURE_AD_TASK == 1)
    if (lcv_ComplierRet == pdPASS)
    {
        kalyke_start_AD();
    }
#endif
    LOGV(TAG, "%s: 026", __func__);

    start_modbus_com();

    /*系统扫描速率初始化*/
    gtv_PlcRunStatus.mlv_SysScanTime = GET_1MS_TICKS_COUNT();

    LOGV(TAG, "%s: 027. heap:%d", __func__, xPortGetFreeHeapSize());
    /*运行中看门狗时间初始化*/
    gtv_PlcRunStatus.mlv_WatchDogTime = GET_1MS_TICKS_COUNT();

    //hexdump(gtv_PlcElement.msp_DElement, 128);
    LOGV(TAG, "Leave %s", __func__);
}


/**
  * @brief  清除用户程序运行中错误标志
  * @param  None
  * @retval None
  */
void plc_clean_user_run_error_flag(void)
{
    LOGV(TAG, "Enter %s", __func__);
    plc_set_bit_element_value(SM_ELEMENT, 20, 0);

    SET_SD_ELEMENT_VALUE(20, 0);
    SET_SD_ELEMENT_VALUE(21, 0);
    SET_SD_ELEMENT_VALUE(22, 0);
    SET_SD_ELEMENT_VALUE(23, 0);
    SET_SD_ELEMENT_VALUE(24, 0);

    guv_NonStopError.bit.operands_err = 0;
    guv_StopError.bit.exec_err = 0;

    gtv_PlcExecErrorRecord.mcv_ErrorCnt = 0;
}

/**
  * @brief  PLC运行态处理函数
  * @param  None
  * @retval None
  */
void plc_run_deamon(void)
{
    uint32_t curTick = 0;
    /*开始运行,清零嵌套调用层数计数器*/
    gtp_CallInsInfoPtr->msv_SbrNestedNum = 0;
#if 0
    if (xTaskGetTickCount() % 3000 == 0)
    {
        LOGV(TAG, "gtv_UserFilePtrSt.UCodePtr = 0x%08X", gtv_UserFilePtrSt.UCodePtr);
    }
#endif
    /*解释运行用户UCODE程序*/
#if (PLC_SCAN_TIME_LOGIC_2 == 1)
    if (!plc_get_bit_element_value(SM_ELEMENT, SM8))
    {
        curTick = GET_1MS_TICKS_COUNT();
    }
#endif
    plc_run_user_program(gtv_UserFilePtrSt.UCodePtr + 28, 0);
#if (PLC_SCAN_TIME_LOGIC_2 == 1)
    if (!plc_get_bit_element_value(SM_ELEMENT, SM8))
    {
        plc_refresh_scan_time(curTick);
    }
#endif
    /*置位相关SM标志*/
    plc_set_bit_element_value(SM_ELEMENT, 0, 1);
    plc_set_bit_element_value(SM_ELEMENT, 1, 0);
    plc_set_bit_element_value(SM_ELEMENT, 2, 0);

    if(plc_get_bit_element_value(SM_ELEMENT, 15))
        plc_set_bit_element_value(SM_ELEMENT, 15, 0);
    else
        plc_set_bit_element_value(SM_ELEMENT, 15, 1);
}

/**
  * @brief  PLC运行用户程序
  * @param  lcp_UcodeAddr : UCODE开始执行指令地址
  * @retval None
  */
unsigned char plc_run_user_program(unsigned char *lcp_UcodeAddr, uint8_t flag)
{
    unsigned char lcv_RetValue;
    plc_run_power_flow_st ltv_RunEnv;
    ltv_RunEnv.mlv_InPF = 0;
    ltv_RunEnv.mlv_OutPF = 0;
    ltv_RunEnv.mcp_PC = lcp_UcodeAddr;

    //LOGV(TAG, "Enter %s(), lcp_UcodeAddr = 0x%x", __func__, lcp_UcodeAddr);
    /*清除本函数中FOR-NEXT嵌套层数*/
    gtp_ForNextIns[gtp_CallInsInfoPtr->msv_SbrNestedNum].msv_NestedNum = 0;

EXEC_LOOP_FLAG:

    gcp_BackupUserPc = ltv_RunEnv.mcp_PC;
    //LOGV(TAG, "lcp_BackupUserPc = 0x%08X", gcp_BackupUserPc);

    if(GET_PU16_DATA(ltv_RunEnv.mcp_PC) < 0xE000) {
        /*简单指令*/
        lcv_RetValue = plc_exec_simple_instruction(&ltv_RunEnv);

        if(lcv_RetValue != pdPASS) {
            /*刷新错误标志元件值*/
            plc_refresh_error_msg(lcv_RetValue);
            /*刷新运行态错误信息,提供给KS软件定位*/
            plc_refresh_exec_error_record(lcv_RetValue, gcp_BackupUserPc);
        }
    } else {
        /*复杂指令*/
        lcv_RetValue = plc_exec_complex_instruction(&ltv_RunEnv);

        /*复杂指令返回值处理*/
        switch(lcv_RetValue) {
            case pdPASS:
                plc_set_bit_element_value(SM_ELEMENT, 20, 0);
                plc_set_bit_element_value(SM_ELEMENT, 21, 0);
                plc_set_bit_element_value(SM_ELEMENT, 22, 0);
                break;

            case EXEC_FLAG_FEND:
                return EXEC_FLAG_FEND;

            case EXEC_FLAG_IRET:
                return EXEC_FLAG_IRET;

            case EXEC_FLAG_SRET:
                return EXEC_FLAG_SRET;

            case EXEC_FLAG_STOP:
                gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.cmd_stop = 1;
                return EXEC_FLAG_STOP;

            default:
                /*刷新错误标志元件值*/
                plc_refresh_error_msg(lcv_RetValue);
                /*刷新运行态错误信息,提供给KS软件定位*/
                plc_refresh_exec_error_record(lcv_RetValue, gcp_BackupUserPc);
        }
    }

    /*非在线模式，运行周期超时判断*/
    if(!gtv_PlcRunStatus.mcv_IsOnlineProgram) {
        plc_judge_run_over_time();
    }

    /*运行错误,停止运行*/
    if(gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.error_status_stop) {

        LOGD(TAG, "%s(): Run error, we are going to stop!!!, flag = %u", __func__, flag);
        /*禁止用户中断*/
        plc_user_interrupt_enable(0);

        return EXEC_FLAG_STOP;
    }

    goto EXEC_LOOP_FLAG;
}


/**
  * @brief  PLC状态由运行到停止
  * @param  None
  * @retval None
  */
void plc_status_run_to_stop()
{
    //kalyke_stop_4G_for_PLC();
    //停止运行时fexlink正常通讯	
    //kalyke_daisy_stop();
    spd_deinit();
    highcount_poweron_deinit();
    uninit_high_speed();
#if (KALYKE_FEATURE_AD_TASK == 1)
    kalyke_stop_AD();
#endif
    plc_set_bit_element_value(SM_ELEMENT, 0, 0);
    /*根据系统块设置输出状态*/
    plc_set_stop_status_output_port();
    /*停止所有Time*/
    plc_stop_all_timer();
    /*禁止时钟振荡源*/
    bsp_cycle_clock_enable(0);

    /*禁止用户中断*/

    /*掉电保持数据保存*/

#if 0
    if (gtv_UserFilePtrSt.UCodePtr)
    {
        if ((uint32_t)gtv_UserFilePtrSt.UCodePtr > 0x20200000u && (uint32_t)gtv_UserFilePtrSt.UCodePtr < 0x202C0000u)
        {
            vPortFree(gtv_UserFilePtrSt.UCodePtr);
            gtv_UserFilePtrSt.UCodePtr = NULL;
        }
    }
#endif
    //plc_free_ins();
    stop_modbus_com();
}

/**
  * @brief  PLC STOP 状态
  * @param  None
  * @retval None
  */
void plc_status_stop()
{
    //LOGD(TAG, "Enter %s()", __func__);
    /*根据系统块设置输出状态*/
    plc_set_stop_status_output_port();

    /*系统重启*/
    if (gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.cmd_reboot)
    {
        plc_reboot();
    }
}
void plc_reboot()
{
    kalyke_stop_4G_for_PLC();
    GPIO_PinWrite(ENET1_RST_GPIO, ENET1_RST_PIN, 0);
    GPIO_PinWrite(ENET2_RST_GPIO, ENET2_RST_PIN, 0);
    plsd_save_data();
    bsp_save_KalykeSecondTick(gKalykeSecondTick);
    vTaskDelay(100);
    LOGD(TAG, "OMG!mtv_PlcRunStopFlag = 0x%X", gtv_PlcRunStatus.mtv_PlcRunStopFlag);
    bsp_reboot_system();
}

/**
  * @brief  PLC 运行超时判断
  * @param  None
  * @retval None
  */
void plc_judge_run_over_time()
{
    if(GET_1MS_TICKS_COUNT() - gtv_PlcRunStatus.mlv_WatchDogTime > GET_SD_ELEMENT_VALUE(SD34))
    //if(GET_1MS_TICKS_COUNT() - gtv_PlcRunStatus.mlv_WatchDogTime > 4500)
    {
        LOGE(TAG, "mlv_WatchDogTime = %u", gtv_PlcRunStatus.mlv_WatchDogTime);
        /*刷新错误标志元件值*/
        plc_refresh_error_msg(ERR_EXEC_OVER_TIME);
    }
}

void plc_run_do(void)
{
    LOGV(TAG, "Enter %s()", __func__);
    gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.cmd_run = 1;
}

/**
  * @brief  恢复数据块
  * @param  None
  * @retval None
  */
void plc_restore_data_block(void)
{
    LOGV(TAG, "Enter %s()", __func__);

    unsigned short *lsp_ElementIndex, *lsp_Vaule;
    unsigned short lsv_Length;
    unsigned short lsv_MinLen;
    unsigned long i, j, k, m;

    if(GET_BIGPU16_DATA(gtv_UserFilePtrSt.DataBlockPtr + 2) != 0x02) {
        return;
    }

    /*数据块文件最小长度，不设定任何值情况下*/
    lsv_MinLen = (D_RANG + R_RANG)/8 + 28;

    /*数据块文件真实长度*/
    lsv_Length = plc_get_file_length(gtv_UserFilePtrSt.DataBlockPtr + FILE_LEN_INFO_START_INDEX, 4);

    if(lsv_MinLen > lsv_Length) {
        return;
    }

    lsp_ElementIndex = (unsigned short *)(gtv_UserFilePtrSt.DataBlockPtr+24);
    lsp_Vaule = (unsigned short *)(gtv_UserFilePtrSt.DataBlockPtr+lsv_MinLen-4);

    j = 0;
    k = 0;
    m = 0;

    /*D元件*/
    for(i=0; i<D_RANG; i++) {
        if(lsp_ElementIndex[j] & (0x01<<k)) {
            SET_D_ELEMENT_VALUE(i, lsp_Vaule[m]);
            m++;
        }

        k++;

        if(k>=16) {
            j++;
            k = 0;
        }
    }

    /*R元件*/
    k = 0;
    if(D_RANG%8) {
        j++;
    }

    for(i=0; i<R_RANG; i++) {
        if(lsp_ElementIndex[j] & (0x01<<k)) {
            SET_R_ELEMENT_VALUE(i, lsp_Vaule[m]);
            m++;
        }

        k++;

        if(k>=16) {
            j++;
            k = 0;
        }
    }
}

