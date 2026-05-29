/**
  ******************************************************************************
  * @file    plc_counterins.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   计数器相关指令函数
  ******************************************************************************
  */
#include "plc_element.h"
#include "plc_variable.h"
#include "plc_parseaddr.h"
#include "plc_commonfunc.h"

/**
  * @brief  重置计数器
  * @param  None
  * @retval None
  */
void plc_reset_one_counter(unsigned short Element)
{
    if(Element < C16_RANG)
        gtv_PlcElement.mtv_CElement.msp_16BitValue[Element] = 0;
    else
        gtv_PlcElement.mtv_CElement.msp_32BitValue[Element - C16_RANG] = 0;

    plc_set_bit_element_value(C_ELEMENT, Element, 0);
}

/**
  * @brief  16位增计数指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_ctu_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_CounterNum;
    unsigned char lcv_Ret;
    unsigned short lsv_DestValue;

    lsv_CounterNum = GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        if(gtv_PlcElement.mtv_CElement.mtp_StatusInfo[lsv_CounterNum].mbv_PowerFlow != 1) {
            gtv_PlcElement.mtv_CElement.mtp_StatusInfo[lsv_CounterNum].mbv_PowerFlow = 1;

            if(gtv_PlcElement.mtv_CElement.msp_16BitValue[lsv_CounterNum] != 0x7FFF) {
                gtv_PlcElement.mtv_CElement.msp_16BitValue[lsv_CounterNum]++;
            }

            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, &lsv_DestValue, 0, 1);
            if(lcv_Ret != pdPASS) {
                return lcv_Ret;
            }

            if(gtv_PlcElement.mtv_CElement.msp_16BitValue[lsv_CounterNum] >= lsv_DestValue) {
                plc_set_bit_element_value(C_ELEMENT, lsv_CounterNum, 1);
            }
        }
    } else {
        gtv_PlcElement.mtv_CElement.mtp_StatusInfo[lsv_CounterNum].mbv_PowerFlow = 0;
    }

    return pdPASS;
}

/**
  * @brief  16位循环计数指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_ctr_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_CounterNum;
    unsigned char lcv_Ret;
    unsigned short lsv_DestValue;

    lsv_CounterNum = GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        if(gtv_PlcElement.mtv_CElement.mtp_StatusInfo[lsv_CounterNum].mbv_PowerFlow != 1) {
            gtv_PlcElement.mtv_CElement.mtp_StatusInfo[lsv_CounterNum].mbv_PowerFlow = 1;

            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, &lsv_DestValue, 0, 1);
            if(lcv_Ret != pdPASS) {
                return lcv_Ret;
            }

            if(gtv_PlcElement.mtv_CElement.msp_16BitValue[lsv_CounterNum] == lsv_DestValue) {
                gtv_PlcElement.mtv_CElement.msp_16BitValue[lsv_CounterNum] = 1;
                plc_set_bit_element_value(C_ELEMENT, lsv_CounterNum, 0);
                return pdPASS;
            }

            gtv_PlcElement.mtv_CElement.msp_16BitValue[lsv_CounterNum]++;

            if(gtv_PlcElement.mtv_CElement.msp_16BitValue[lsv_CounterNum] >= lsv_DestValue) {
                plc_set_bit_element_value(C_ELEMENT, lsv_CounterNum, 1);
            }
        }
    } else {
        gtv_PlcElement.mtv_CElement.mtp_StatusInfo[lsv_CounterNum].mbv_PowerFlow = 0;
    }

    return pdPASS;
}

/**
  * @brief  32位增减计数指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dcnt_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_CounterNum;
    unsigned char lcv_Ret;
    long llv_DestValue;

    lsv_CounterNum = GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        if(gtv_PlcElement.mtv_CElement.mtp_StatusInfo[lsv_CounterNum].mbv_PowerFlow != 1) {
            gtv_PlcElement.mtv_CElement.mtp_StatusInfo[lsv_CounterNum].mbv_PowerFlow = 1;

            lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+8, (unsigned long *)&llv_DestValue, 0, 1);
            if(lcv_Ret != pdPASS) {
                return lcv_Ret;
            }

            if(plc_get_bit_element_value(SM_ELEMENT, lsv_CounterNum) == 0) {
                /*增计数*/
                gtv_PlcElement.mtv_CElement.msp_32BitValue[lsv_CounterNum - C16_RANG]++;

                if(gtv_PlcElement.mtv_CElement.msp_32BitValue[lsv_CounterNum - C16_RANG] >= llv_DestValue) {
                    plc_set_bit_element_value(C_ELEMENT, lsv_CounterNum, 1);
                } else {
                    plc_set_bit_element_value(C_ELEMENT, lsv_CounterNum,0);
                }
            } else {
                /*减计数*/
                gtv_PlcElement.mtv_CElement.msp_32BitValue[lsv_CounterNum - C16_RANG]--;

                if(gtv_PlcElement.mtv_CElement.msp_32BitValue[lsv_CounterNum - C16_RANG] <= llv_DestValue) {
                    plc_set_bit_element_value(C_ELEMENT, lsv_CounterNum, 1);
                } else {
                    plc_set_bit_element_value(C_ELEMENT, lsv_CounterNum,0);
                }
            }
        }
    } else {
        gtv_PlcElement.mtv_CElement.mtp_StatusInfo[lsv_CounterNum].mbv_PowerFlow = 0;
    }

    return pdPASS;

}

/**
  * @brief  计数器复位指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_rst_c_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_CounterNum;

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        lsv_CounterNum = GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);
        plc_reset_one_counter(lsv_CounterNum);
    }
    return 1;
}

