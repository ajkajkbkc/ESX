/**
  ******************************************************************************
  * @file    plc_internalmanage.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   内务处理相关函数
  ******************************************************************************
  */
#include "FreeRTOS.h"
#include "fsl_debug_console.h"
#include "plc_internalmanage.h"
#include "plc_variable.h"
#include "plc_element.h"
#include "bsp_dct.h"
#include "bsp_gpio.h"
#include "list_func.h"
#include "mb_maptable.h"
#include "kalyke_opts.h"
#include "plc_sysblock.h"

plc_clock_cycle_record_st gtv_ClockCycleRecord = { 0, };

/**
  * @brief  振荡时钟源初始化
  * @param  None
  * @retval None
  */
void plc_clock_cycle_init(void)
{
    //vPortEnterCritical();
    gtv_ClockCycleRecord.mlv_Hour = 0;
    gtv_ClockCycleRecord.mlv_Minute = 0;
    gtv_ClockCycleRecord.mlv_Second = 0;
    gtv_ClockCycleRecord.mlv_HundredMs = 0;
    gtv_ClockCycleRecord.mcv_Status = 0;

    plc_set_bit_element_value(SM_ELEMENT, 10, 0);
    plc_set_bit_element_value(SM_ELEMENT, 11, 0);
    plc_set_bit_element_value(SM_ELEMENT, 12, 0);
    plc_set_bit_element_value(SM_ELEMENT, 13, 0);
    plc_set_bit_element_value(SM_ELEMENT, 14, 0);

    //vPortExitCritical();
}

#if 0
/**
  * @brief  PLC运行期间，刷新周期振荡时钟
  * @param  None
  * @retval None
  */
void plc_refresh_cycle_clock(void)
{
    gtv_ClockCycleRecord.mlv_Hour += 5;
    gtv_ClockCycleRecord.mlv_Minute += 5;
    gtv_ClockCycleRecord.mlv_Second += 5;
    gtv_ClockCycleRecord.mlv_HundredMs += 5;

    /*10ms反转*/
    if(gtv_ClockCycleRecord.mcv_Status & 0x01) {
        plc_set_bit_element_value(SM_ELEMENT, 10, 0);
        gtv_ClockCycleRecord.mcv_Status &= ~0x01;
    } else {
        plc_set_bit_element_value(SM_ELEMENT, 10, 1);
        gtv_ClockCycleRecord.mcv_Status |= 0x01;
    }

    /*100ms反转*/
    if(gtv_ClockCycleRecord.mlv_HundredMs >= 50) {
        if(gtv_ClockCycleRecord.mcv_Status & 0x02) {
            plc_set_bit_element_value(SM_ELEMENT, 11, 0);
            gtv_ClockCycleRecord.mcv_Status &= ~0x02;
        } else {
            plc_set_bit_element_value(SM_ELEMENT, 11, 1);
            gtv_ClockCycleRecord.mcv_Status |= 0x02;
        }
        gtv_ClockCycleRecord.mlv_HundredMs = 0;
    }

    /*1秒反转*/
    if(gtv_ClockCycleRecord.mlv_Second >= 500) {
        if(gtv_ClockCycleRecord.mcv_Status & 0x04) {
            plc_set_bit_element_value(SM_ELEMENT, 12, 0);
            gtv_ClockCycleRecord.mcv_Status &= ~0x04;
        } else {
            plc_set_bit_element_value(SM_ELEMENT, 12, 1);
            gtv_ClockCycleRecord.mcv_Status |= 0x04;
        }
        gtv_ClockCycleRecord.mlv_Second = 0;
    }

    /*1分钟反转*/
    if(gtv_ClockCycleRecord.mlv_Minute >= 30*1000) {
        if(gtv_ClockCycleRecord.mcv_Status & 0x08) {
            plc_set_bit_element_value(SM_ELEMENT, 13, 0);
            gtv_ClockCycleRecord.mcv_Status &= ~0x08;
        } else {
            plc_set_bit_element_value(SM_ELEMENT, 13, 1);
            gtv_ClockCycleRecord.mcv_Status |= 0x08;
        }
        gtv_ClockCycleRecord.mlv_Minute = 0;
    }

    /*1小时反转*/
    if(gtv_ClockCycleRecord.mlv_Hour >= 30*60*1000) {
        if(gtv_ClockCycleRecord.mcv_Status & 0x10) {
            plc_set_bit_element_value(SM_ELEMENT, 14, 0);
            gtv_ClockCycleRecord.mcv_Status &= ~0x10;
        } else {
            plc_set_bit_element_value(SM_ELEMENT, 14, 1);
            gtv_ClockCycleRecord.mcv_Status |= 0x10;
        }
        gtv_ClockCycleRecord.mlv_Hour = 0;
    }
}
#endif

/**
  * @brief  刷新PLC扫描周期
  * @param  None
  * @retval 1：恒定扫描周期未完成
  *         0:
  */
unsigned char plc_refresh_sys_scan_time(void)
{
    int32_t mlv_ScanTime;

    mlv_ScanTime = GET_1MS_TICKS_COUNT() - gtv_PlcRunStatus.mlv_SysScanTime;

    /*计时器溢出*/
    if(mlv_ScanTime < 0) {
        mlv_ScanTime = GET_1MS_TICKS_COUNT() + (0xFFFFFFFFUL - gtv_PlcRunStatus.mlv_SysScanTime);
    }

    if(gtv_PlcRunStatus.mcv_PlcCurrentStatus == PLC_RUN_STATUS) 
    {
        if(plc_get_bit_element_value(SM_ELEMENT, SM8) && (mlv_ScanTime < GET_SD_ELEMENT_VALUE(33)))
        {
            /*恒定扫描周期未完成*/
            return 1;
        } 
        else 
        {
            //LOGD("scan", "SD31 = %u", GET_SD_ELEMENT_VALUE(31));
            SET_SD_ELEMENT_VALUE(SD30, mlv_ScanTime);
            /*刷新最大扫描速率*/
            if(mlv_ScanTime > GET_SD_ELEMENT_VALUE(SD32)) {
                SET_SD_ELEMENT_VALUE(SD32, mlv_ScanTime);
            }
            /*刷新最小扫描速率*/
            if(mlv_ScanTime < GET_SD_ELEMENT_VALUE(SD31)) {
                SET_SD_ELEMENT_VALUE(SD31, mlv_ScanTime);
            }
        }
    }

    gtv_PlcRunStatus.mlv_SysScanTime = GET_1MS_TICKS_COUNT();

    /*更新看门狗时间*/
    gtv_PlcRunStatus.mlv_WatchDogTime = gtv_PlcRunStatus.mlv_SysScanTime;
    
    return 0;
}

#if (PLC_SCAN_TIME_LOGIC_2 == 1)
void plc_refresh_scan_time(uint32_t lastTick)
{
    uint32_t curTick = GET_1MS_TICKS_COUNT();
    int32_t mlv_ScanTime = curTick - lastTick;

    /*计时器溢出*/
    if(mlv_ScanTime < 0)
    {
        mlv_ScanTime = curTick + (0xFFFFFFFFUL - gtv_PlcRunStatus.mlv_SysScanTime);
    }
    else if (mlv_ScanTime == 0)
    {
        mlv_ScanTime = 1;
    }

    SET_SD_ELEMENT_VALUE(SD30, mlv_ScanTime);
    /*刷新最大扫描速率*/
    if(mlv_ScanTime > GET_SD_ELEMENT_VALUE(SD32))
    {
        SET_SD_ELEMENT_VALUE(SD32, mlv_ScanTime);
    }
    /*刷新最小扫描速率*/
    if(mlv_ScanTime < GET_SD_ELEMENT_VALUE(SD31))
    {
        SET_SD_ELEMENT_VALUE(SD31, mlv_ScanTime);
    }

    gtv_PlcRunStatus.mlv_SysScanTime = GET_1MS_TICKS_COUNT();

    /*更新看门狗时间*/
    gtv_PlcRunStatus.mlv_WatchDogTime = gtv_PlcRunStatus.mlv_SysScanTime;
}
#endif

/**
  * @brief  刷新输入输出端口
  * @param  None
  * @retval None
  */
#if 0
void plc_refresh_io_port(void)
{
    if(gtv_DeviceConfigTable.mcv_IsHaveIOPort)
    {
        if(gtv_DeviceConfigTable.mcv_SelfInputNum > 0)
        {
            //bsp_refresh_input_port(X_ELEMENT, gtv_DeviceConfigTable.mcv_SelfInputNum);
            gGPIORefreshInput(X_ELEMENT, gtv_DeviceConfigTable.mcv_SelfInputNum);
        }

        if(gtv_DeviceConfigTable.mcv_SelfOutputNum > 0)
        {
            //bsp_refresh_output_port(Y_ELEMENT, gtv_DeviceConfigTable.mcv_SelfOutputNum);
            gGPIORefreshOutput(Y_ELEMENT, gtv_DeviceConfigTable.mcv_SelfOutputNum);
        }
    }
}
#else
void plc_refresh_io_port(void)
{
    if (gLeftModules.BIT.HaveDM169 == 1)
    {
        //gGPIORefreshIO(gtv_DeviceConfigTable.mcv_SelfInputNum, gtv_DeviceConfigTable.mcv_SelfOutputNum); //2022.3.3
        bsp_refresh_input_output_port(gtv_DeviceConfigTable.mcv_SelfInputNum, gtv_DeviceConfigTable.mcv_SelfOutputNum);
    }
}

#endif

/**
  * @brief  刷新强制元件
  * @param  None
  * @retval None
  */

void plc_refresh_force_element_value(void)
{
    struct list_head *ltp_Head;
    struct list_head *ltp_ForCur;
    struct list_head *ltp_ForNext;
    mb_force_element_t *ltp_ForceData;
    mb_force_element_t *ltp_ForceData2;
    unsigned short lsv_32BitCAddr;
    unsigned long   llv_32BitCValue;

    /*刷新强制位元件*/
    if(gtv_ForceBits.lsv_ListLen > 0) {
        //PRINTF("gtv_ForceBits.lsv_ListLen = %d\r\n", gtv_ForceBits.lsv_ListLen);
        ltp_Head = &gtv_ForceBits.head;
        list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
            ltp_ForceData = list_entry(ltp_ForCur, mb_force_element_t, list);
            switch(ltp_ForceData->mcv_ElementType) {
                case MB_BIT_Y:
                    plc_set_bit_element_value(Y_ELEMENT, ltp_ForceData->msv_ElementAddr, ltp_ForceData->msv_ElementValue);
                    break;
                case MB_BIT_X:
                    plc_set_bit_element_value(X_ELEMENT, ltp_ForceData->msv_ElementAddr, ltp_ForceData->msv_ElementValue);
                    break;                
                case MB_BIT_M:
                    plc_set_bit_element_value(M_ELEMENT, ltp_ForceData->msv_ElementAddr, ltp_ForceData->msv_ElementValue);
                    break;
                case MB_BIT_SM:
                    plc_set_bit_element_value(SM_ELEMENT, ltp_ForceData->msv_ElementAddr, ltp_ForceData->msv_ElementValue);
                    break;
                case MB_BIT_S:
                    plc_set_bit_element_value(S_ELEMENT, ltp_ForceData->msv_ElementAddr, ltp_ForceData->msv_ElementValue);
                    break;
                case MB_BIT_T:
                    plc_set_bit_element_value(T_ELEMENT, ltp_ForceData->msv_ElementAddr, ltp_ForceData->msv_ElementValue);
                    break;
                case MB_BIT_C:
                    plc_set_bit_element_value(C_ELEMENT, ltp_ForceData->msv_ElementAddr, ltp_ForceData->msv_ElementValue);
                    break;
            }
        }
    }

    if(gtv_ForceWords.lsv_ListLen > 0) {
        //PRINTF("gtv_ForceWords.lsv_ListLen = %d\r\n", gtv_ForceWords.lsv_ListLen);
        ltp_Head = &gtv_ForceWords.head;
        list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
            ltp_ForceData = list_entry(ltp_ForCur, mb_force_element_t, list);
            switch(ltp_ForceData->mcv_ElementType) {
                case MB_WORD_D:
                    SET_D_ELEMENT_VALUE(ltp_ForceData->msv_ElementAddr, ltp_ForceData->msv_ElementValue);
                    break;
                case MB_WORD_SD:
                    SET_SD_ELEMENT_VALUE(ltp_ForceData->msv_ElementAddr, ltp_ForceData->msv_ElementValue);
                    break;
                case MB_WORD_Z:
                    SET_Z_ELEMENT_VALUE(ltp_ForceData->msv_ElementAddr, ltp_ForceData->msv_ElementValue);
                    break;
                case MB_WORD_T:
                    SET_T_CURRENT_VALUE(ltp_ForceData->msv_ElementAddr, ltp_ForceData->msv_ElementValue);
                    break;
                case MB_WORD_C:
                    if(ltp_ForceData->msv_ElementAddr < C16_RANG) {
                        SET_C16_CURRENT_VALUE(ltp_ForceData->msv_ElementAddr, ltp_ForceData->msv_ElementValue);
                    } else {
                        /*32bit C元件需要取下一个组合*/
                        ltp_ForCur = ltp_ForNext;
                        ltp_ForNext = ltp_ForCur->next;

                        ltp_ForceData2 = list_entry(ltp_ForCur, mb_force_element_t, list);
                        if(ltp_ForceData2->mcv_ElementType != MB_WORD_C) {
                            /*强制元件列表错误，回退指针，忽略一个32bit C元件*/
                            ltp_ForNext = ltp_ForCur;
                            ltp_ForCur = ltp_ForNext->prev;
                            break;
                        }

                        lsv_32BitCAddr = (ltp_ForceData->msv_ElementAddr - C16_RANG)/2 + gtp_PlcElementInfo->msv_CElement.msv_16bitCnt;
                        llv_32BitCValue = (ltp_ForceData->msv_ElementValue) + (unsigned long)(ltp_ForceData2->msv_ElementValue << 16);

                        SET_C32_CURRENT_VALUE(lsv_32BitCAddr, llv_32BitCValue);
                    }
                    break;
                case MB_WORD_R:
                    SET_R_ELEMENT_VALUE(ltp_ForceData->msv_ElementAddr, ltp_ForceData->msv_ElementValue);
                    break;
            }
        }
    }
}

/**
  * @brief  刷新强制元件
  * @param  None
  * @retval None
  */
void plc_set_stop_status_output_port(void)
{
#if 0
    static uint32_t tick = 0;
    if ((xTaskGetTickCount() - tick) > 2000)
    {
        tick = xTaskGetTickCount();
        LOGD("plc_internal", "Enter %s(), gtv_PlcRunToStopOutputStatus.mcv_Status=%u", __func__, gtv_PlcRunToStopOutputStatus.mcv_Status);
    }
#endif
    switch(gtv_PlcRunToStopOutputStatus.mcv_Status)
    {
        case OUTPUT_POINT_KEEP:
            break;

        case OUTPUT_POINT_CLOSE:
            for(uint32_t i = 0; i < (Y_RANG>>4); i++)
            {
                gtv_PlcElement.msp_YElement[i] = 0;
            }
            break;

        case OUTPUT_POINT_CONFIG:
            for(uint32_t i = 0; i < 8; i++)
            {
                gtv_PlcElement.msp_YElement[i] = gtv_PlcRunToStopOutputStatus.msv_OutValue[i];
            }
            break;

        default:
            break;
    }
}

