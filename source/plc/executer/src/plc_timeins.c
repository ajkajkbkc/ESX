/**
  ******************************************************************************
  * @file    plc_timeins.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   时间相关指令函数
  ******************************************************************************
  */

#include "plc_variable.h"
#include "plc_commonfunc.h"
#include "plc_element.h"
#include "plc_parseaddr.h"
#include "plc_timeins.h"
#include "plc_errormsg.h"
#include "plc_instruction.h"

#include "kalyke_DS1302.h"
#include "fsl_snvs_hp.h"
#include "fsl_snvs_lp.h"

/**
  * @brief  停止所有计时器
  * @param  None
  * @retval None
  */
void plc_stop_all_timer(void)
{
    unsigned long i;
    unsigned char *lcp_Temp;

    lcp_Temp =  (unsigned char *)&gtv_PlcElement.mtv_TElement.mtp_StatusInfo[0];

    for(i=0; i<T_RANG; i++) {
        lcp_Temp[i] = 0;
    }
}

/**
  * @brief  复位指定计时器
  * @param  None
  * @retval None
  */
void plc_reset_one_timer(unsigned short lsv_TimeNum)
{
    /*清除运行标志*/
    *(unsigned char *)&gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum] = 0;
    /*清位元件*/
    plc_set_bit_element_value(T_ELEMENT, lsv_TimeNum, 0);
    /*设置计时值*/
    gtv_PlcElement.mtv_TElement.msp_CurrentValue[lsv_TimeNum] = 0;
    gtv_PlcElement.mtv_TElement.msp_DestValue[lsv_TimeNum] = 0;
    gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum] = GET_1MS_TICKS_COUNT();
}

/**
  * @brief  接通延时计时指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_ton_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned long llv_SystemTick;
    unsigned short lsv_TimeNum;
    unsigned char lcv_Ret;
    unsigned long llv_Temp;

    lsv_TimeNum = GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);
    llv_SystemTick = GET_1MS_TICKS_COUNT();

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, &gtv_PlcElement.mtv_TElement.msp_DestValue[lsv_TimeNum], 0, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        /*第一次启动*/
        if(!gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Init) {
            gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Init = 1;

            if(lsv_TimeNum < gtp_PlcElementInfo->msv_TElement.msv_100msTimerCnt) {
                gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Capacity = TIME_ACCURACY_100MS;
            } else if(lsv_TimeNum < (gtp_PlcElementInfo->msv_TElement.msv_100msTimerCnt + gtp_PlcElementInfo->msv_TElement.msv_10msTimerCnt)) {
                gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Capacity = TIME_ACCURACY_10MS;
            } else {
                gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Capacity = TIME_ACCURACY_1MS;
            }

            gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Type = TIME_TYPE_TON;

            gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum] = llv_SystemTick;
        }

        if(gtv_PlcElement.mtv_TElement.msp_CurrentValue[lsv_TimeNum] >= 0x7FFF) {
            gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Enable = 0;
            plc_set_bit_element_value(T_ELEMENT, lsv_TimeNum, 1);
        } else {
            gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Enable = 1;

            llv_Temp = llv_SystemTick - gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum];

            switch(gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Capacity) {
                case TIME_ACCURACY_100MS:
                    llv_Temp /= 100;
                    if(llv_Temp > 0) {
                        gtv_PlcElement.mtv_TElement.msp_CurrentValue[lsv_TimeNum] += llv_Temp;
                        gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum] += llv_Temp * 100;
                    }
                    break;

                case TIME_ACCURACY_10MS:
                    llv_Temp /= 10;
                    if(llv_Temp > 0) {
                        gtv_PlcElement.mtv_TElement.msp_CurrentValue[lsv_TimeNum] += llv_Temp;
                        gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum] += llv_Temp * 10;
                    }

                    break;

                case TIME_ACCURACY_1MS:
                    gtv_PlcElement.mtv_TElement.msp_CurrentValue[lsv_TimeNum] += llv_Temp;
                    gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum] += llv_Temp;
                    break;
            }

            if(gtv_PlcElement.mtv_TElement.msp_CurrentValue[lsv_TimeNum] >=
               gtv_PlcElement.mtv_TElement.msp_DestValue[lsv_TimeNum]) {
                plc_set_bit_element_value(T_ELEMENT, lsv_TimeNum, 1);
            }
        }
    } else {
        if(gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Init) {
            plc_reset_one_timer(lsv_TimeNum);
        }
    }

    return pdPASS;
}

/**
  * @brief  记忆型接通延时计时指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_tonr_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned long llv_SystemTick;
    unsigned short lsv_TimeNum;
    unsigned char lcv_Ret;
    unsigned long llv_Temp;

    lsv_TimeNum = GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);
    llv_SystemTick = GET_1MS_TICKS_COUNT();

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, &gtv_PlcElement.mtv_TElement.msp_DestValue[lsv_TimeNum], 0, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        /*第一次启动*/
        if(!gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Init) {
            gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Init = 1;

            if(lsv_TimeNum < gtp_PlcElementInfo->msv_TElement.msv_100msTimerCnt) {
                gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Capacity = TIME_ACCURACY_100MS;
            } else if(lsv_TimeNum < (gtp_PlcElementInfo->msv_TElement.msv_100msTimerCnt + gtp_PlcElementInfo->msv_TElement.msv_10msTimerCnt)) {
                gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Capacity = TIME_ACCURACY_10MS;
            } else {
                gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Capacity = TIME_ACCURACY_1MS;
            }

            gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Type = TIME_TYPE_TONR;

            gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum] = llv_SystemTick;
        }

        if(gtv_PlcElement.mtv_TElement.msp_CurrentValue[lsv_TimeNum] >= 0x7FFF) {
            plc_set_bit_element_value(T_ELEMENT, lsv_TimeNum, 1);
        } else {

            if(!gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Enable) {
                gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Enable = 1;
                gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum] = llv_SystemTick;
                return pdPASS;
            }

            llv_Temp = llv_SystemTick - gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum];

            switch(gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Capacity) {
                case TIME_ACCURACY_100MS:
                    llv_Temp /= 100;
                    if(llv_Temp > 0) {
                        gtv_PlcElement.mtv_TElement.msp_CurrentValue[lsv_TimeNum] += llv_Temp;
                        gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum] += llv_Temp * 100;
                    }
                    break;

                case TIME_ACCURACY_10MS:
                    llv_Temp /= 10;
                    if(llv_Temp > 0) {
                        gtv_PlcElement.mtv_TElement.msp_CurrentValue[lsv_TimeNum] += llv_Temp;
                        gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum] += llv_Temp * 10;
                    }

                    break;

                case TIME_ACCURACY_1MS:
                    gtv_PlcElement.mtv_TElement.msp_CurrentValue[lsv_TimeNum] += llv_Temp;
                    gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum] += llv_Temp;
                    break;
            }

            if(gtv_PlcElement.mtv_TElement.msp_CurrentValue[lsv_TimeNum] >=
               gtv_PlcElement.mtv_TElement.msp_DestValue[lsv_TimeNum]) {
                plc_set_bit_element_value(T_ELEMENT, lsv_TimeNum, 1);
            }
        }
    } else {
        if(gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Init) {
            gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Enable = 0;
            gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum] = llv_SystemTick;
        }
    }

    return pdPASS;
}

/**
  * @brief  断开延时计时指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_tof_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned long llv_SystemTick;
    unsigned short lsv_TimeNum;
    unsigned char lcv_Ret;
    unsigned long llv_Temp;

    lsv_TimeNum = GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);
    llv_SystemTick = GET_1MS_TICKS_COUNT();

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        *(unsigned char *)&gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum] = 0;
        gtv_PlcElement.mtv_TElement.msp_CurrentValue[lsv_TimeNum] = 0;
        plc_set_bit_element_value(T_ELEMENT, lsv_TimeNum, 1);
        /*记录能流有效过标志*/
        gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_PowerFlow = 1;

    } else {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, &gtv_PlcElement.mtv_TElement.msp_DestValue[lsv_TimeNum], 0, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        /*第一次启动*/
        if(!gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Init
           && gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_PowerFlow) {
            gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Init = 1;

            if(lsv_TimeNum < gtp_PlcElementInfo->msv_TElement.msv_100msTimerCnt) {
                gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Capacity = TIME_ACCURACY_100MS;
            } else if(lsv_TimeNum < (gtp_PlcElementInfo->msv_TElement.msv_100msTimerCnt + gtp_PlcElementInfo->msv_TElement.msv_10msTimerCnt)) {
                gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Capacity = TIME_ACCURACY_10MS;
            } else {
                gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Capacity = TIME_ACCURACY_1MS;
            }

            gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Type = TIME_TYPE_TOF;

            gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum] = llv_SystemTick;

            gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Enable = 1;
        }

        if(gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Enable) {
            llv_Temp = llv_SystemTick - gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum];

            switch(gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Capacity) {
                case TIME_ACCURACY_100MS:
                    llv_Temp /= 100;
                    if(llv_Temp > 0) {
                        gtv_PlcElement.mtv_TElement.msp_CurrentValue[lsv_TimeNum] += llv_Temp;
                        gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum] += llv_Temp * 100;
                    }
                    break;

                case TIME_ACCURACY_10MS:
                    llv_Temp /= 10;
                    if(llv_Temp > 0) {
                        gtv_PlcElement.mtv_TElement.msp_CurrentValue[lsv_TimeNum] += llv_Temp;
                        gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum] += llv_Temp * 10;
                    }

                    break;

                case TIME_ACCURACY_1MS:
                    gtv_PlcElement.mtv_TElement.msp_CurrentValue[lsv_TimeNum] += llv_Temp;
                    gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum] += llv_Temp;
                    break;
            }

            if(gtv_PlcElement.mtv_TElement.msp_CurrentValue[lsv_TimeNum] >=
               gtv_PlcElement.mtv_TElement.msp_DestValue[lsv_TimeNum]) {
                gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Enable = 0;
                plc_set_bit_element_value(T_ELEMENT, lsv_TimeNum, 0);
            }
        }
    }

    return pdPASS;
}

/**
  * @brief  不重触发单稳计时指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_tmon_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned long llv_SystemTick;
    unsigned short lsv_TimeNum;
    unsigned char lcv_Ret;
    unsigned long llv_Temp;

    lsv_TimeNum = GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);
    llv_SystemTick = GET_1MS_TICKS_COUNT();

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        if(!gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_PowerFlow) {
            gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_PowerFlow = 1;

            if(lsv_TimeNum < gtp_PlcElementInfo->msv_TElement.msv_100msTimerCnt) {
                gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Capacity = TIME_ACCURACY_100MS;
            } else if(lsv_TimeNum < (gtp_PlcElementInfo->msv_TElement.msv_100msTimerCnt + gtp_PlcElementInfo->msv_TElement.msv_10msTimerCnt)) {
                gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Capacity = TIME_ACCURACY_10MS;
            } else {
                gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Capacity = TIME_ACCURACY_1MS;
            }

            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, &gtv_PlcElement.mtv_TElement.msp_DestValue[lsv_TimeNum], 0, 1);
            if(lcv_Ret != pdPASS) {
                return lcv_Ret;
            }

            gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Type = TIME_TYPE_TMON;

            gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum] = llv_SystemTick;

            gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Enable = 1;

            plc_set_bit_element_value(T_ELEMENT, lsv_TimeNum, 1);
        }

    } else {
        gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_PowerFlow = 0;
    }

    /*计时已经启动*/
    if(gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Enable) {
        llv_Temp = llv_SystemTick - gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum];

        switch(gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Capacity) {
            case TIME_ACCURACY_100MS:
                llv_Temp /= 100;
                if(llv_Temp > 0) {
                    gtv_PlcElement.mtv_TElement.msp_CurrentValue[lsv_TimeNum] += llv_Temp;
                    gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum] += llv_Temp * 100;
                }
                break;

            case TIME_ACCURACY_10MS:
                llv_Temp /= 10;
                if(llv_Temp > 0) {
                    gtv_PlcElement.mtv_TElement.msp_CurrentValue[lsv_TimeNum] += llv_Temp;
                    gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum] += llv_Temp * 10;
                }

                break;

            case TIME_ACCURACY_1MS:
                gtv_PlcElement.mtv_TElement.msp_CurrentValue[lsv_TimeNum] += llv_Temp;
                gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum] += llv_Temp;
                break;
        }

        if(gtv_PlcElement.mtv_TElement.msp_CurrentValue[lsv_TimeNum] >=
           gtv_PlcElement.mtv_TElement.msp_DestValue[lsv_TimeNum]) {
            gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Enable = 0;
            plc_set_bit_element_value(T_ELEMENT, lsv_TimeNum, 0);
        }
    }

    return pdPASS;
}

/**
  * @brief  定时器reset指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_rst_t_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_TimeNum;

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        lsv_TimeNum = GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);

        plc_reset_one_timer(lsv_TimeNum);

    }

    return pdPASS;
}

/**
  * @brief  刷新所有使能定时器
  * @param  None
  * @retval None
  */
void plc_refresh_10ms_1ms_timer(void)
{
    unsigned short i;
    unsigned long llv_SystemTick, llv_Temp;

    llv_SystemTick = GET_1MS_TICKS_COUNT();

    for(i=gtp_PlcElementInfo->msv_TElement.msv_10msTimerCnt; i<gtp_PlcElementInfo->msv_TElement.msv_ElementCnt; i++) {

        if(!gtv_PlcElement.mtv_TElement.mtp_StatusInfo[i].mbv_Enable) {
            continue;
        }

        llv_Temp = llv_SystemTick - gtv_PlcElement.mtv_TElement.mlp_StartValue[i];

        switch(gtv_PlcElement.mtv_TElement.mtp_StatusInfo[i].mbv_Capacity) {
            case TIME_ACCURACY_10MS:
                llv_Temp /= 10;
                if(llv_Temp > 0) {
                    gtv_PlcElement.mtv_TElement.msp_CurrentValue[i] += llv_Temp;
                    gtv_PlcElement.mtv_TElement.mlp_StartValue[i] += llv_Temp * 10;
                }
                break;

            case TIME_ACCURACY_1MS:
                gtv_PlcElement.mtv_TElement.msp_CurrentValue[i] += llv_Temp;
                gtv_PlcElement.mtv_TElement.mlp_StartValue[i] += llv_Temp;
                break;
        }

        if(gtv_PlcElement.mtv_TElement.msp_CurrentValue[i] >= 0x7FFF) {
            gtv_PlcElement.mtv_TElement.mtp_StatusInfo[i].mbv_Enable = 0;
        }

        if(gtv_PlcElement.mtv_TElement.msp_CurrentValue[i] >=
           gtv_PlcElement.mtv_TElement.msp_DestValue[i]) {
            if(gtv_PlcElement.mtv_TElement.mtp_StatusInfo[i].mbv_Type == TIME_TYPE_TOF) {
                plc_set_bit_element_value(T_ELEMENT, i, 0);
            } else {
                plc_set_bit_element_value(T_ELEMENT, i, 1);
            }
        }
    }
}
/*------------------------------------------------------------------------------
*   信号报警指令
*-----------------------------------------------------------------------------*/
/*信号报警器软元件默认使用S900 ~ S999*/
static unsigned short ssv_MinAlarmSElement = 999;

/**
  * @brief  复位信号报警器软元件默认值
  * @param  None
  * @retval None
  */
void plc_reset_signal_alarm_element(void)
{
    ssv_MinAlarmSElement = 999;
}

/**
  * @brief  信号报警器置位指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_ans_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_TimeNum;
    unsigned short lsv_AlarmSElement;
    unsigned char lcv_Ret;
    unsigned long llv_SystemTick, llv_Temp;

    /*取计时器编号*/
    lsv_TimeNum = GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);

    if(lsv_TimeNum >= gtp_PlcElementInfo->msv_TElement.msv_10msTimerCnt) {
        return ERR_OVER_ELEMENT_RANG;
    }

    gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Capacity = TIME_ACCURACY_100MS;
    gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Type = TIME_TYPE_TON;

    /*取计时目标值*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&gtv_PlcElement.mtv_TElement.msp_DestValue[lsv_TimeNum], 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*取信号报警器软元件编号*/
    lsv_AlarmSElement = GET_PU16_DATA(ltp_RunEnv->mcp_PC+12);
    if((lsv_AlarmSElement < 900) || (lsv_AlarmSElement > 999)) {
        return ERR_OVER_ELEMENT_RANG;
    }

    llv_SystemTick = GET_1MS_TICKS_COUNT();
    if(!gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Enable) {
        gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum] = llv_SystemTick;
    }

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        if(gtv_PlcElement.mtv_TElement.msp_CurrentValue[lsv_TimeNum] >= 0x7FFF) {
            gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Enable = 0;
        } else {
            gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Enable = 1;

            llv_Temp = llv_SystemTick - gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum];

            llv_Temp /= 100;

            if(llv_Temp > 0) {
                gtv_PlcElement.mtv_TElement.msp_CurrentValue[lsv_TimeNum] += llv_Temp;
                gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum] += llv_Temp*100;
            }
        }

        if(gtv_PlcElement.mtv_TElement.msp_CurrentValue[lsv_TimeNum] >=
           gtv_PlcElement.mtv_TElement.msp_DestValue[lsv_TimeNum]) {
            plc_set_bit_element_value(S_ELEMENT, lsv_AlarmSElement, 1);

            if(plc_get_bit_element_value(SM_ELEMENT, 100)) {

                plc_set_bit_element_value(SM_ELEMENT, SM401, 1);

                if(lsv_AlarmSElement < ssv_MinAlarmSElement) {
                    ssv_MinAlarmSElement = lsv_AlarmSElement;
                    SET_SD_ELEMENT_VALUE(SD401, lsv_AlarmSElement);
                }
            }
        }
    } else {
        gtv_PlcElement.mtv_TElement.msp_CurrentValue[lsv_TimeNum] = 0;
        gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_TimeNum].mbv_Enable = 0;
        gtv_PlcElement.mtv_TElement.mlp_StartValue[lsv_TimeNum] = llv_SystemTick;
    }

    return pdPASS;
}

/**
  * @brief  信号报警器复位指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_anr_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short i;

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        for(i=900; i<=999; i++) {
            if(plc_get_bit_element_value(S_ELEMENT, i)) {
                plc_set_bit_element_value(S_ELEMENT, i, 0);
                break;
            }
        }

        if(plc_get_bit_element_value(SM_ELEMENT, SM400)) {
            /*刷新SD401中保存的最小报警元件值*/
            for(; i<=999; i++) {
                if(plc_get_bit_element_value(S_ELEMENT, i))
                    SET_SD_ELEMENT_VALUE(SD401, i);
                break;
            }

            /*无报警信号,清除SD401 SM401*/
            if(i>999) {
                plc_set_bit_element_value(SM_ELEMENT, SM401, 0);
                SET_SD_ELEMENT_VALUE(SD401, 0);
                ssv_MinAlarmSElement = 999;
            }
        }
    }

    return pdPASS;
}

/*------------------------------------------------------------------------------
*   实时时钟指令
*-----------------------------------------------------------------------------*/
/**
  * @brief  实时时钟读指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_trd_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret = pdPASS;
    if(!GET_POWER_FLOW(ltp_RunEnv))
    {
        return pdPASS;
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+2, &GET_SD_ELEMENT_VALUE(100), 0, 7);

    return lcv_Ret;
}

/**
  * @brief  实时时钟写指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_twr_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char i, buf[8];
    unsigned char lcv_Ret = pdPASS;
    unsigned short lsv_Time[7];
    snvs_lp_srtc_datetime_t srtcDate;
    if(!GET_POWER_FLOW(ltp_RunEnv))
    {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Time[0], 0, 7);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*年月日时分秒周*/
    if(lsv_Time[0] < 2000 || lsv_Time[1] > 12 || lsv_Time[2] > 31 || lsv_Time[3] > 24 || lsv_Time[4] > 60 || lsv_Time[5] > 60 || lsv_Time[6] > 7)
    {
        return ERR_OPERANDS;
    }

#if KALYKE_DS1302_FEATURE == 1
    buf[0] = 0;
    buf[1] = (uint8_t)(lsv_Time[0] - 2000);
    for(i = 1; i < 7; i++) {
        buf[i+1] = (uint8_t)lsv_Time[i];
    }
    DS1302_WriteTimeBurst(buf);
#endif

    srtcDate.year = lsv_Time[0];
    srtcDate.month = lsv_Time[1];
    srtcDate.day = lsv_Time[2];
    srtcDate.hour = lsv_Time[3];
    srtcDate.minute = lsv_Time[4];
    srtcDate.second = lsv_Time[5];
    if (SNVS_LP_SRTC_SetDatetime(SNVS, &srtcDate) == kStatus_Success)
    {
        SNVS_HP_RTC_TimeSynchronize(SNVS);
    }

    return pdPASS;
}

/**
  * @brief  时钟加指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_tadd_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret = pdPASS;
    unsigned short lsv_Time1, lsv_Time2, lsav_Result[3]={0,};

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 0);
        plc_set_bit_element_value(SM_ELEMENT, 181, 0);

        /*秒相加*/
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Time1, 2, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, &lsv_Time2, 2, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        if(lsv_Time1 > 60 || lsv_Time2 > 60) {
            return ERR_OPERANDS;
        }

        lsav_Result[2] = lsv_Time1 + lsv_Time2;

        if(lsav_Result[2] > 60) {
            lsav_Result[1] = 1;
            lsav_Result[2] -= 60;
        }

        /*分相加*/
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Time1, 1, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, &lsv_Time2, 1, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        if(lsv_Time1 > 60 || lsv_Time2 > 60) {
            return ERR_OPERANDS;
        }

        lsav_Result[1] += lsv_Time1 + lsv_Time2;
        if(lsav_Result[1] > 60) {
            lsav_Result[0] = 1;
            lsav_Result[1] -= 60;
        }

        /*小时处理*/
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Time1, 0, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, &lsv_Time2, 0, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        if(lsv_Time1 > 23 || lsv_Time2 > 23) {
            return ERR_OPERANDS;
        }

        lsav_Result[0] += lsv_Time1 + lsv_Time2;
        if(lsav_Result[0] > 23) {
            lsav_Result[0] -= 24;
            plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        }

        if(!(lsav_Result[0] && lsav_Result[1] && lsav_Result[2])) {
            plc_set_bit_element_value(SM_ELEMENT, 180, 1);
        }

        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+10, lsav_Result, 0, 3);
    }

    return lcv_Ret;
}

/**
  * @brief  时钟减指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_tsub_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret = pdPASS;
    unsigned short lsv_Time1, lsv_Time2;
    short lsav_Result[3] = {0,};

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 0);
        plc_set_bit_element_value(SM_ELEMENT, 182, 0);

        /*秒相减*/
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Time1, 2, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, &lsv_Time2, 2, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        if(lsv_Time1 > 60 || lsv_Time2 > 60) {
            return ERR_OPERANDS;
        }

        lsav_Result[2] = lsv_Time1 - lsv_Time2;

        if(lsav_Result[2] < 0) {
            lsav_Result[1] = -1;
            lsav_Result[2] += 60;
        }

        /*分相减*/
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Time1, 1, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, &lsv_Time2, 1, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        if(lsv_Time1 > 60 || lsv_Time2 > 60) {
            return ERR_OPERANDS;
        }

        lsav_Result[1] += lsv_Time1 - lsv_Time2;
        if(lsav_Result[1] < 60) {
            lsav_Result[0] = -1;
            lsav_Result[1] += 60;
        }

        /*小时处理*/
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Time1, 0, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, &lsv_Time2, 0, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        if(lsv_Time1 > 23 || lsv_Time2 > 23) {
            return ERR_OPERANDS;
        }

        lsav_Result[0] += lsv_Time1 - lsv_Time2;
        if(lsav_Result[0] < 0) {
            lsav_Result[0] += 24;
            plc_set_bit_element_value(SM_ELEMENT, 182, 1);
        }

        if(!(lsav_Result[0] && lsav_Result[1] && lsav_Result[2])) {
            plc_set_bit_element_value(SM_ELEMENT, 180, 1);
        }

        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)lsav_Result, 0, 3);
    }

    return lcv_Ret;
}

/**
  * @brief  计时表指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_hour_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_InstantiateNum;
    unsigned short lsv_DestHour, lsav_ElapsedTime[2]={0,};
    unsigned char lcv_Ret;
    unsigned long llv_SystemTick;
    unsigned char lcv_Temp;

    lsv_InstantiateNum = GET_PU16_DATA(ltp_RunEnv->mcp_PC+14);
    if(lsv_InstantiateNum > 255) {
        return ERR_OPERANDS;
    }

    llv_SystemTick = GET_1MS_TICKS_COUNT();

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        if(!gtp_HourInsSt[lsv_InstantiateNum].mcv_LastPFStatus) {
            gtp_HourInsSt[lsv_InstantiateNum].mcv_LastPFStatus = 1;
            gtp_HourInsSt[lsv_InstantiateNum].mlv_LastTime = llv_SystemTick;
            return pdPASS;
        }

        /*读取比较目标时间*/
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_DestHour, 0, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        /*取已经经过时间*/
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, &lsav_ElapsedTime[0], 0, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        if(lsav_ElapsedTime[0] > 32767) {
            lsav_ElapsedTime[0] = 0;
        }

        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, &lsav_ElapsedTime[1], 1, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        lsav_ElapsedTime[1] += (llv_SystemTick - gtp_HourInsSt[lsv_InstantiateNum].mlv_LastTime)/1000;
        if(lsav_ElapsedTime[1] > 3600) {
            lsav_ElapsedTime[0] += 1;
            lsav_ElapsedTime[1] -= 3600;
        }

        gtp_HourInsSt[lsv_InstantiateNum].mlv_LastTime = llv_SystemTick - ((llv_SystemTick - gtp_HourInsSt[lsv_InstantiateNum].mlv_LastTime)%1000);

        /*比较*/
        if(lsav_ElapsedTime[0] >= lsv_DestHour) {
            lcv_Temp = 1;
            lcv_Ret = save_char(ltp_RunEnv->mcp_PC+10, &lcv_Temp, 0, 1);
            if(lcv_Ret != pdPASS) {
                return lcv_Ret;
            }
        }

        /*刷新已经过时间*/
        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, lsav_ElapsedTime, 0, 2);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

    } else {
        gtp_HourInsSt[lsv_InstantiateNum].mcv_LastPFStatus = 0;
    }

    return pdPASS;
}

/**
  * @brief  日期比较指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dcmp_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    short lsv_Data1[3], lsv_Data2[3];
    signed char lcv_Result, lcv_Temp;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)lsv_Data1, 0, 3);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)lsv_Data2, 0, 3);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_Data1[1]< 1 || lsv_Data1[1] > 12 || lsv_Data2[1]< 1 || lsv_Data2[1] > 12) {
        return ERR_OPERANDS;
    }

    switch(lsv_Data1[1]) {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            if(lsv_Data1[2]<1 || lsv_Data1[2]>31) {
                return ERR_OPERANDS;
            }
            break;

        case 4:
        case 6:
        case 9:
        case 11:
            if(lsv_Data1[2]<1 || lsv_Data1[2]>30) {
                return ERR_OPERANDS;
            }
            break;

        case 2:
            if((lsv_Data1[0]%4==0) && (lsv_Data1[0]%100) && (lsv_Data1[0]%400==0)) {
                if(lsv_Data1[2]<1 || lsv_Data1[2]>29) {
                    return ERR_OPERANDS;
                }
            } else {
                if(lsv_Data1[2]<1 || lsv_Data1[2]>28) {
                    return ERR_OPERANDS;
                }
            }
            break;
    }

    switch(lsv_Data2[1]) {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            if(lsv_Data2[2]<1 || lsv_Data2[2]>31) {
                return ERR_OPERANDS;
            }
            break;

        case 4:
        case 6:
        case 9:
        case 11:
            if(lsv_Data2[2]<1 || lsv_Data2[2]>30) {
                return ERR_OPERANDS;
            }
            break;

        case 2:
            if((lsv_Data2[0]%4==0) && (lsv_Data2[0]%100) && (lsv_Data2[0]%400==0)) {
                if(lsv_Data2[2]<1 || lsv_Data2[2]>29) {
                    return ERR_OPERANDS;
                }
            } else {
                if(lsv_Data2[2]<1 || lsv_Data2[2]>28) {
                    return ERR_OPERANDS;
                }
            }
            break;
    }

    /*日期比较*/
    if(lsv_Data1[0] < lsv_Data2[0]) {
        lcv_Result = -1;
    } else if(lsv_Data1[0] > lsv_Data2[0]) {
        lcv_Result = 1;
    } else {
        if(lsv_Data1[1] < lsv_Data2[1]) {
            lcv_Result = -1;
        } else if(lsv_Data1[1] > lsv_Data2[1]) {
            lcv_Result = 1;
        } else {
            if(lsv_Data1[2] < lsv_Data2[2]) {
                lcv_Result = -1;
            } else if(lsv_Data1[2] > lsv_Data2[2]) {
                lcv_Result = 1;
            } else {
                lcv_Result = 0;
            }
        }
    }

    switch(GET_PU16_DATA(ltp_RunEnv->mcp_PC)) {
        case CI_DCMPE:
            if(lcv_Result == 0)
                lcv_Temp = 1;
            else
                lcv_Temp = 0;
            break;

        case CI_DCMPG:
            if(lcv_Result == 1)
                lcv_Temp = 1;
            else
                lcv_Temp = 0;
            break;

        case CI_DCMPL:
            if(lcv_Result == -1)
                lcv_Temp = 1;
            else
                lcv_Temp = 0;
            break;

        case CI_DCMPNE:
            if(lcv_Result != 0)
                lcv_Temp = 1;
            else
                lcv_Temp = 0;
            break;

        case CI_DCMPLE:
            if(lcv_Result <= 0)
                lcv_Temp = 1;
            else
                lcv_Temp = 0;
            break;

        case CI_DCMPGE:
            if(lcv_Result >= 0)
                lcv_Temp = 1;
            else
                lcv_Temp = 0;
            break;
    }

    lcv_Ret = save_char(ltp_RunEnv->mcp_PC+10, (unsigned char *)&lcv_Temp, 0, 1);

    return lcv_Ret;
}

/**
  * @brief  时间比较指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_tcmp_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    short lsv_Time1[3], lsv_Time2[3];
    signed char lcv_Result, lcv_Temp;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    /*获取时间*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)lsv_Time1, 0, 3);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)lsv_Time2, 0, 3);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_Time1[0]<0 || lsv_Time1[0]>23 || lsv_Time2[0]<0 || lsv_Time2[0]>23) {
        return ERR_OPERANDS;
    }

    if(lsv_Time1[1]<0 || lsv_Time1[1]>59 || lsv_Time1[2]<0 || lsv_Time1[2]>59
       || lsv_Time2[1]<0 || lsv_Time2[1]>59 || lsv_Time2[2]<0 || lsv_Time2[2]>59) {
        return ERR_OPERANDS;
    }

    /*时间比较*/
    if(lsv_Time1[0] < lsv_Time2[0]) {
        lcv_Result = -1;
    } else if (lsv_Time1[0] > lsv_Time2[0]) {
        lcv_Result = 1;
    } else {
        if(lsv_Time1[1] < lsv_Time2[1]) {
            lcv_Result = -1;
        } else if (lsv_Time1[1] > lsv_Time2[1]) {
            lcv_Result = 1;
        } else {
            if(lsv_Time1[2] < lsv_Time2[2]) {
                lcv_Result = -1;
            } else if (lsv_Time1[2] > lsv_Time2[2]) {
                lcv_Result = 1;
            } else {
                lcv_Result = 0;
            }
        }
    }

    switch(GET_PU16_DATA(ltp_RunEnv->mcp_PC)) {
        case CI_TCMPE:
            if(lcv_Result == 0)
                lcv_Temp = 1;
            else
                lcv_Temp = 0;
            break;

        case CI_TCMPG:
            if(lcv_Result == 1)
                lcv_Temp = 1;
            else
                lcv_Temp = 0;
            break;

        case CI_TCMPL:
            if(lcv_Result == -1)
                lcv_Temp = 1;
            else
                lcv_Temp = 0;
            break;

        case CI_TCMPNE:
            if(lcv_Result != 0)
                lcv_Temp = 1;
            else
                lcv_Temp = 0;
            break;

        case CI_TCMPLE:
            if(lcv_Result <= 0)
                lcv_Temp = 1;
            else
                lcv_Temp = 0;
            break;

        case CI_TCMPGE:
            if(lcv_Result >= 0)
                lcv_Temp = 1;
            else
                lcv_Temp = 0;
            break;
    }

    lcv_Ret = save_char(ltp_RunEnv->mcp_PC+10, (unsigned char *)&lcv_Temp, 0, 1);

    return lcv_Ret;
}

/**
  * @brief  时 分 秒数据转化为秒
  * @param  None
  * @retval None
  */
unsigned char run_ci_htos_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_Time[3];
    unsigned short lsv_Result;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, lsv_Time, 0, 3);
    if(lcv_Ret!=pdPASS) {
        return lcv_Ret;
    }

    if(lsv_Time[0]>23 || lsv_Time[1]>59 || lsv_Time[2]>59) {
        return ERR_OPERANDS;
    }

    lsv_Result = lsv_Time[0]*60*60 + lsv_Time[1]*60 + lsv_Time[2];

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, &lsv_Result, 0, 1);

    return lcv_Ret;
}

/**
  * @brief  秒数据的时分秒转换
  * @param  None
  * @retval None
  */
unsigned char run_ci_stoh_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_Result[3];
    unsigned short lsv_Time;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Time, 0, 1);
    if(lcv_Ret!=pdPASS) {
        return lcv_Ret;
    }

    lsv_Result[0] = lsv_Time / 3600;
    lsv_Result[1] = (lsv_Time % 3600)/60;
    lsv_Result[2] = lsv_Time % 60;

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, lsv_Result, 0, 3);

    return lcv_Ret;
}

/**
  * @brief  计时表指令数据初始化
  * @param  None
  * @retval None
  */
void plc_hour_ins_data_init(void)
{
    unsigned short i;

    for(i=0; i<256; i++) {
        gtp_HourInsSt[i].mcv_LastPFStatus = 0;
        gtp_HourInsSt[i].mlv_LastTime = 0;
    }
}

