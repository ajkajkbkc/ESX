/**
  ******************************************************************************
  * @file    plc_externalins.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   外设指令
  ******************************************************************************
  */

#include "plc_variable.h"
#include "plc_commonfunc.h"
#include "plc_element.h"
#include "plc_parseaddr.h"
#include "plc_timeins.h"
#include "plc_errormsg.h"
#include "plc_instruction.h"

/**
  * @brief  设置输入滤波常数指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_reff_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    unsigned short lsv_Data;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Data, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_Data > 60) {
        lsv_Data = 60;
    }

    if(GET_D_ELEMENT_VALUE(35) != lsv_Data) {
        SET_D_ELEMENT_VALUE(35, lsv_Data);
    }

    return pdPASS;
}


