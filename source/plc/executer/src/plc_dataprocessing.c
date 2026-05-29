/**
  ******************************************************************************
  * @file    plc_dataprocessing.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   数据处理相关指令,包括比较指令 数据运算指令 数值转换指令等
  ******************************************************************************
  */
#include <math.h>
#include <stdlib.h>
#include "plc_errormsg.h"
#include "plc_commonfunc.h"
#include "plc_variable.h"
#include "plc_instruction.h"
#include "plc_parseaddr.h"
#include "plc_element.h"
#include "plc_dataprocessing.h"
#include "plc_counterins.h"
#include "plc_timeins.h"
#include "fsl_debug_console.h"
#include "fsl_trng.h"

/*------------------------------------------------------------------------------
*   数据比较指令
*-----------------------------------------------------------------------------*/

/**
  * @brief  整数比较 LD（=，<，>，<>，>=，<=）
  * @param  None
  * @retval None
  */
unsigned char run_ld_compare_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short a, b;
    unsigned char lcv_Ret;

    /*取比较目标数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&a, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&b, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*能流入栈*/
    ltp_RunEnv->mlv_InPF <<= 1;

    switch(GET_PU16_DATA(ltp_RunEnv->mcp_PC)) {
        case CI_LDE:
            if(a == b) {
                SET_POWER_FLOW(ltp_RunEnv);
            } else {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_LDG:
            if(a > b) {
                SET_POWER_FLOW(ltp_RunEnv);
            } else {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_LDL:
            if(a < b) {
                SET_POWER_FLOW(ltp_RunEnv);
            } else {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_LDNE:
            if(a != b) {
                SET_POWER_FLOW(ltp_RunEnv);
            } else {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_LDLE:
            if(a <= b) {
                SET_POWER_FLOW(ltp_RunEnv);
            } else {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_LDGE:
            if(a >= b) {
                SET_POWER_FLOW(ltp_RunEnv);
            } else {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;
        default:
            ltp_RunEnv->mlv_InPF >>= 1;
            return ERR_ILLEGAL_INSTRCTION;
    }

    return pdPASS;
}

/**
  * @brief  整数比较AND（=，<，>，<>，>=，<=）
  * @param  None
  * @retval None
  */
unsigned char run_and_compare_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short a, b;
    unsigned char lcv_Ret;

    /*取比较目标数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&a, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&b, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    switch(GET_PU16_DATA(ltp_RunEnv->mcp_PC)) {
        case CI_ANDE:
            if(a != b) {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ANDG:
            if(a <= b) {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ANDL:
            if(a >= b) {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ANDNE:
            if(a == b) {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ANDLE:
            if(a > b) {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ANDGE:
            if(a < b) {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        default:
            return ERR_ILLEGAL_INSTRCTION;
    }

    return pdPASS;
}

/**
  * @brief  整数比较OR（=，<，>，<>，>=，<=）
  * @param  None
  * @retval None
  */
unsigned char run_or_compare_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short a, b;
    unsigned char lcv_Ret;

    /*取比较目标数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&a, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&b, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    switch(GET_PU16_DATA(ltp_RunEnv->mcp_PC)) {

        case CI_ORE:
            if(a == b) {
                SET_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ORG:
            if(a > b) {
                SET_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ORL:
            if(a < b) {
                SET_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ORNE:
            if(a != b) {
                SET_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ORLE:
            if(a <= b) {
                SET_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ORGE:
            if(a >= b) {
                SET_POWER_FLOW(ltp_RunEnv);
            }
            break;

        default:
            return ERR_ILLEGAL_INSTRCTION;
    }
    return pdPASS;
}

/**
  * @brief  长整数比较 LD（=，<，>，<>，>=，<=）
  * @param  None
  * @retval None
  */
unsigned char run_ldd_compare_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    long a, b;
    unsigned char lcv_Ret;

    /*取比较目标数*/
    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, (unsigned long *)&a, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+8, (unsigned long *)&b, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*能流入栈*/
    ltp_RunEnv->mlv_InPF <<= 1;

    switch(GET_PU16_DATA(ltp_RunEnv->mcp_PC)) {
        case CI_LDDE:
            if(a == b) {
                SET_POWER_FLOW(ltp_RunEnv);
            } else {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_LDDG:
            if(a > b) {
                SET_POWER_FLOW(ltp_RunEnv);
            } else {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_LDDL:
            if(a < b) {
                SET_POWER_FLOW(ltp_RunEnv);
            } else {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_LDDNE:
            if(a != b) {
                SET_POWER_FLOW(ltp_RunEnv);
            } else {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_LDDLE:
            if(a <= b) {
                SET_POWER_FLOW(ltp_RunEnv);
            } else {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_LDDGE:
            if(a >= b) {
                SET_POWER_FLOW(ltp_RunEnv);
            } else {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;
        default:
            ltp_RunEnv->mlv_InPF >>= 1;
            return ERR_ILLEGAL_INSTRCTION;
    }

    return pdPASS;
}

/**
  * @brief  长整数比较AND（=，<，>，<>，>=，<=）
  * @param  None
  * @retval None
  */
unsigned char run_andd_compare_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    long a, b;
    unsigned char lcv_Ret;

    /*取比较目标数*/
    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, (unsigned long *)&a, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+8, (unsigned long *)&b, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    switch(GET_PU16_DATA(ltp_RunEnv->mcp_PC)) {
        case CI_ANDDE:
            if(a != b) {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ANDDG:
            if(a <= b) {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ANDDL:
            if(a >= b) {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ANDDNE:
            if(a == b) {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ANDDLE:
            if(a > b) {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ANDDGE:
            if(a < b) {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        default:
            return ERR_ILLEGAL_INSTRCTION;
    }

    return pdPASS;
}

/**
  * @brief  长整数比较OR（=，<，>，<>，>=，<=）
  * @param  None
  * @retval None
  */
unsigned char run_ord_compare_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    long a, b;
    unsigned char lcv_Ret;

    /*取比较目标数*/
    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, (unsigned long *)&a, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+8, (unsigned long *)&b, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    switch(GET_PU16_DATA(ltp_RunEnv->mcp_PC)) {

        case CI_ORDE:
            if(a == b) {
                SET_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ORDG:
            if(a > b) {
                SET_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ORDL:
            if(a < b) {
                SET_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ORDNE:
            if(a != b) {
                SET_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ORDLE:
            if(a <= b) {
                SET_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ORDGE:
            if(a >= b) {
                SET_POWER_FLOW(ltp_RunEnv);
            }
            break;

        default:
            return ERR_ILLEGAL_INSTRCTION;
    }
    return pdPASS;
}

/**
  * @brief  浮点数比较 LD（=，<，>，<>，>=，<=）
  * @param  None
  * @retval None
  */
unsigned char run_ldr_compare_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float a, b;
    unsigned char lcv_Ret;

    /*取比较目标数*/
    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &a, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+8, &b, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*能流入栈*/
    ltp_RunEnv->mlv_InPF <<= 1;

    /*四舍五入，保留小数点后6位精度*/
    a = (float)((int)((a + 0.0000005f)*1000000))/1000000;
    b = (float)((int)((b + 0.0000005f)*1000000))/1000000;

    switch(GET_PU16_DATA(ltp_RunEnv->mcp_PC)) {
        case CI_LDRE:
            if(a == b) {
                SET_POWER_FLOW(ltp_RunEnv);
            } else {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_LDRG:
            if(a > b) {
                SET_POWER_FLOW(ltp_RunEnv);
            } else {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_LDRL:
            if(a < b) {
                SET_POWER_FLOW(ltp_RunEnv);
            } else {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_LDRNE:
            if(a != b) {
                SET_POWER_FLOW(ltp_RunEnv);
            } else {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_LDRLE:
            if(a <= b) {
                SET_POWER_FLOW(ltp_RunEnv);
            } else {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_LDRGE:
            if(a >= b) {
                SET_POWER_FLOW(ltp_RunEnv);
            } else {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;
        default:
            ltp_RunEnv->mlv_InPF >>= 1;
            return ERR_ILLEGAL_INSTRCTION;
    }

    return pdPASS;
}

/**
  * @brief  浮点数比较AND（=，<，>，<>，>=，<=）
  * @param  None
  * @retval None
  */
unsigned char run_andr_compare_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float a, b;
    unsigned char lcv_Ret;

    /*取比较目标数*/
    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &a, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+8, &b, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*四舍五入，保留小数点后6位精度*/
    a = (float)((int)((a + 0.0000005f)*1000000))/1000000;
    b = (float)((int)((b + 0.0000005f)*1000000))/1000000;

    switch(GET_PU16_DATA(ltp_RunEnv->mcp_PC)) {
        case CI_ANDRE:
            if(a != b) {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ANDRG:
            if(a <= b) {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ANDRL:
            if(a >= b) {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ANDRNE:
            if(a == b) {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ANDRLE:
            if(a > b) {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ANDRGE:
            if(a < b) {
                RST_POWER_FLOW(ltp_RunEnv);
            }
            break;

        default:
            return ERR_ILLEGAL_INSTRCTION;
    }

    return pdPASS;
}

/**
  * @brief  浮点数比较OR（=，<，>，<>，>=，<=）
  * @param  None
  * @retval None
  */
unsigned char run_orr_compare_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float a, b;
    unsigned char lcv_Ret;

    /*取比较目标数*/
    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &a, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+8, &b, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*四舍五入，保留小数点后6位精度*/
    a = (float)((int)((a + 0.0000005f)*1000000))/1000000;
    b = (float)((int)((b + 0.0000005f)*1000000))/1000000;

    switch(GET_PU16_DATA(ltp_RunEnv->mcp_PC)) {

        case CI_ORRE:
            if(a == b) {
                SET_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ORRG:
            if(a > b) {
                SET_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ORRL:
            if(a < b) {
                SET_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ORRNE:
            if(a != b) {
                SET_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ORRLE:
            if(a <= b) {
                SET_POWER_FLOW(ltp_RunEnv);
            }
            break;

        case CI_ORRGE:
            if(a >= b) {
                SET_POWER_FLOW(ltp_RunEnv);
            }
            break;

        default:
            return ERR_ILLEGAL_INSTRCTION;
    }
    return pdPASS;
}

/**
  * @brief  整数比较指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_cmp_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcav_Result[3] = {0,};
    short lsv_Data1, lsv_Data2;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_Data1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_Data2, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_Data1 < lsv_Data2)
        lcav_Result[0] = 1;
    else if(lsv_Data1 > lsv_Data2)
        lcav_Result[2] = 1;
    else
        lcav_Result[1] = 1;

    lcv_Ret = save_char(ltp_RunEnv->mcp_PC+10, lcav_Result, 0, 3);

    return lcv_Ret;
}

/**
  * @brief  长整数比较指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_lcmp_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcav_Result[3] = {0,};
    long llv_Data1, llv_Data2;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, (unsigned long *)&llv_Data1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+8, (unsigned long *)&llv_Data2, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(llv_Data1 < llv_Data2)
        lcav_Result[0] = 1;
    else if(llv_Data1 > llv_Data2)
        lcav_Result[2] = 1;
    else
        lcav_Result[1] = 1;

    lcv_Ret = save_char(ltp_RunEnv->mcp_PC+14, lcav_Result, 0, 3);

    return lcv_Ret;
}

/**
  * @brief  浮点数比较指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_rcmp_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcav_Result[3] = {0,};
    float lfv_Data1, lfv_Data2;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &lfv_Data1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+8, &lfv_Data2, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*四舍五入，保留小数点后6位精度*/
    lfv_Data1 = (float)((int)((lfv_Data1 + 0.0000005f)*1000000))/1000000;
    lfv_Data2 = (float)((int)((lfv_Data2 + 0.0000005f)*1000000))/1000000;

    if(lfv_Data1 < lfv_Data2)
        lcav_Result[0] = 1;
    else if(lfv_Data1 > lfv_Data2)
        lcav_Result[2] = 1;
    else
        lcav_Result[1] = 1;

    lcv_Ret = save_char(ltp_RunEnv->mcp_PC+14, lcav_Result, 0, 3);

    return lcv_Ret;
}

/*------------------------------------------------------------------------------
*   位处理指令
*-----------------------------------------------------------------------------*/

/**
  * @brief  取S1单元内容的第S2位的状态用于驱动后段运算
  * @param  None
  * @retval None
  */
unsigned char run_ci_bld_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short s1;
    short s2;
    unsigned char lcv_Ret;

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&s2, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(s2<0 || s2>15) {
        return ERR_OPERANDS;
    }

    ltp_RunEnv->mlv_InPF <<= 1;

    if(s1 & (0x01<<s2)) {
        SET_POWER_FLOW(ltp_RunEnv);
    } else {
        RST_POWER_FLOW(ltp_RunEnv);
    }

    return pdPASS;
}

/**
  * @brief  取S1单元内容的第S2位状态的逻辑非用于驱动后段运算
  * @param  None
  * @retval None
  */
unsigned char run_ci_bldi_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short s1;
    short s2;
    unsigned char lcv_Ret;

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&s2, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(s2<0 || s2>15) {
        return ERR_OPERANDS;
    }

    ltp_RunEnv->mlv_InPF <<= 1;

    if (s1 & (0x01<<s2)) {
        RST_POWER_FLOW(ltp_RunEnv);
    } else {
        SET_POWER_FLOW(ltp_RunEnv);
    }

    return pdPASS;

}

/**
  * @brief  取S1单元内容的第S2位的状态与其他节点串联用于驱动后段运算
  * @param  None
  * @retval None
  */
unsigned char run_ci_band_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short s1;
    short s2;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&s2, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(s2<0 || s2>15) {
        return ERR_OPERANDS;
    }

    if (!(s1 & (0x01<<s2))) {
        RST_POWER_FLOW(ltp_RunEnv);
    }

    return pdPASS;
}

/**
  * @brief  取S1单元内容的第S2位的状态非与其他节点串联用于驱动后段运算
  * @param  None
  * @retval None
  */
unsigned char run_ci_bandi_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short s1;
    short s2;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&s2, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(s2<0 || s2>15) {
        return ERR_OPERANDS;
    }

    if (s1 & (0x01<<s2)) {
        RST_POWER_FLOW(ltp_RunEnv);
    }

    return pdPASS;

}

/**
  * @brief  取S1单元内容的第S2位的状态与其他节点并联用于驱动后段运算
  * @param  None
  * @retval None
  */
unsigned char run_ci_bor_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short s1;
    short s2;
    unsigned char lcv_Ret;

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&s2, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(s2<0 || s2>15) {
        return ERR_OPERANDS;
    }

    if (s1 & (0x01<<s2)) {
        SET_POWER_FLOW(ltp_RunEnv);
    }

    return pdPASS;

}

/**
  * @brief  取S1单元内容的第S2位的状态非与其他节点并联用于驱动后段运算
  * @param  None
  * @retval None
  */
unsigned char run_ci_bori_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short s1;
    short s2;
    unsigned char lcv_Ret;

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&s2, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(s2<0 || s2>15) {
        return ERR_OPERANDS;
    }

    if (!(s1 & (0x01<<s2))) {
        SET_POWER_FLOW(ltp_RunEnv);
    }

    return pdPASS;
}

/**
  * @brief  字位线圈置位指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_bset_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short s1;
    short s2;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&s2, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(s2<0 || s2>15) {
        return ERR_OPERANDS;
    }

    s1 |= (1<<s2);

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+2, &s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    return pdPASS;
}

/**
  * @brief  字位线圈清除指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_brst_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short s1;
    short s2;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&s2, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(s2<0 || s2>15) {
        return ERR_OPERANDS;
    }

    s1 &= ~(1<<s2);

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+2, &s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    return pdPASS;
}

/**
  * @brief  字位线圈输出指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_bout_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short s1;
    short s2;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&s2, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(s2<0 || s2>15) {
        return ERR_OPERANDS;
    }

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        s1 |= (1<<s2);
    } else {
        s1 &= ~(1<<s2);
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+2, &s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    return pdPASS;
}

/**
  * @brief  标志着一个能流运算块的开始，并将Dx的第y位的ON/OFF状态赋给当前能流
  * @param  None
  * @retval None
  */
unsigned char run_ci_ld_dx_y_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short s1;
    short s2;
    unsigned char lcv_Ret;

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    s2 = GET_PU16_DATA(ltp_RunEnv->mcp_PC + 6);
    if(s2<0 || s2>15) {
        return ERR_OPERANDS;
    }

    ltp_RunEnv->mlv_InPF <<= 1;

    if(s1 & (1<<s2)) {
        SET_POWER_FLOW(ltp_RunEnv);
    } else {
        RST_POWER_FLOW(ltp_RunEnv);
    }

    return pdPASS;
}

/**
  * @brief  标志着一个能流运算块的开始，并将Dx的第y位的ON/OFF状态取反后赋回当前能流
  * @param  None
  * @retval None
  */
unsigned char run_ci_ldi_dx_y_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short s1;
    short s2;
    unsigned char lcv_Ret;

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    s2 = GET_PU16_DATA(ltp_RunEnv->mcp_PC + 6);
    if(s2<0 || s2>15) {
        return ERR_OPERANDS;
    }

    ltp_RunEnv->mlv_InPF <<= 1;

    if(s1 & (1<<s2)) {
        RST_POWER_FLOW(ltp_RunEnv);
    } else {
        SET_POWER_FLOW(ltp_RunEnv);
    }

    return pdPASS;

}

/**
  * @brief  将Dx的第y位的ON/OFF状态和当前能流作“与”运算后，赋给当前能流
  * @param  None
  * @retval None
  */
unsigned char run_ci_and_dx_y_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short s1;
    short s2;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    s2 = GET_PU16_DATA(ltp_RunEnv->mcp_PC + 6);
    if(s2<0 || s2>15) {
        return ERR_OPERANDS;
    }

    if(!(s1 & (1<<s2))) {
        RST_POWER_FLOW(ltp_RunEnv);
    }

    return pdPASS;
}

/**
  * @brief  将Dx的第y位的ON/OFF状态取反后，与当前能流值作“与”运算计算后，赋给当前能流。
  * @param  None
  * @retval None
  */
unsigned char run_ci_ani_dx_y_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short s1;
    short s2;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    s2 = GET_PU16_DATA(ltp_RunEnv->mcp_PC + 6);
    if(s2<0 || s2>15) {
        return ERR_OPERANDS;
    }

    if (s1 & (1<<s2)) {
        RST_POWER_FLOW(ltp_RunEnv);
    }

    return pdPASS;
}

/**
  * @brief  将Dx的第y位的ON/OFF状态和当前能流作“或”运算后，赋给当前能流
  * @param  None
  * @retval None
  */
unsigned char run_ci_or_dx_y_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short s1;
    short s2;
    unsigned char lcv_Ret;

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    s2 = GET_PU16_DATA(ltp_RunEnv->mcp_PC + 6);
    if(s2<0 || s2>15) {
        return ERR_OPERANDS;
    }

    if (s1 & (1<<s2)) {
        SET_POWER_FLOW(ltp_RunEnv);
    }

    return pdPASS;
}

/**
  * @brief  将Dx的第y位的ON/OFF状态取反后和当前能流值作“或”运算后，赋给当前能流
  * @param  None
  * @retval None
  */
unsigned char run_ci_ori_dx_y_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short s1;
    short s2;
    unsigned char lcv_Ret;

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    s2 = GET_PU16_DATA(ltp_RunEnv->mcp_PC + 6);
    if(s2<0 || s2>15) {
        return ERR_OPERANDS;
    }

    if (!(s1 & (1<<s2))) {
        SET_POWER_FLOW(ltp_RunEnv);
    }

    return pdPASS;
}

/**
  * @brief  当能流有效时，Dx的第y位的将被置位
  * @param  None
  * @retval None
  */
unsigned char run_ci_set_dx_y_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short s1;
    short s2;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    s2 = GET_PU16_DATA(ltp_RunEnv->mcp_PC + 6);
    if(s2<0 || s2>15) {
        return ERR_OPERANDS;
    }

    s1 |= (1<<s2);

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+2, &s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    return pdPASS;
}

/**
  * @brief  当能流有效时，Dx的第y位的将被清零
  * @param  None
  * @retval None
  */
unsigned char run_ci_rst_dx_y_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short s1;
    short s2;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    s2 = GET_PU16_DATA(ltp_RunEnv->mcp_PC + 6);
    if(s2<0 || s2>15) {
        return ERR_OPERANDS;
    }

    s1 &= ~(1<<s2);

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+2, &s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    return pdPASS;
}

/**
  * @brief  将当前能流值赋给Dx的第y位的
  * @param  None
  * @retval None
  */
unsigned char run_ci_out_dx_y_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short s1;
    short s2;
    unsigned char lcv_Ret;

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    s2 = GET_PU16_DATA(ltp_RunEnv->mcp_PC + 6);
    if(s2<0 || s2>15) {
        return ERR_OPERANDS;
    }

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        s1 |= (1<<s2);
    } else {
        s1 &= ~(1<<s2);
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+2, &s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    return pdPASS;
}

/*------------------------------------------------------------------------------
*   增强行位处理指令
*-----------------------------------------------------------------------------*/
/**
  * @brief  批量位清零指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_zrst_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    short lsv_Cnt;
    unsigned short *lsp_ElementAddr;
    unsigned short lsv_ElementCnt, lsv_StartElement, i;
    unsigned char lcv_AddrType = GET_PU8_DATA(ltp_RunEnv->mcp_PC+3);

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_Cnt, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }
    if(lsv_Cnt < 0) {
        return ERR_OPERANDS;
    }

    switch(lcv_AddrType) {
        case ADDR_X:
        case ADDR_XZ:
            lsp_ElementAddr = X_ELEMENT;
            lsv_ElementCnt = X_RANG;
            break;

        case ADDR_Y:
        case ADDR_YZ:
            lsp_ElementAddr = Y_ELEMENT;
            lsv_ElementCnt = Y_RANG;
            break;

        case ADDR_C:
        case ADDR_CZ:
            lsp_ElementAddr = C_ELEMENT;
            lsv_ElementCnt = C_RANG;
            break;

        case ADDR_T:
        case ADDR_TZ:
            lsp_ElementAddr = T_ELEMENT;
            lsv_ElementCnt = T_RANG;
            break;

        case ADDR_M:
        case ADDR_MZ:
            lsp_ElementAddr = M_ELEMENT;
            lsv_ElementCnt = M_RANG;
            break;

        case ADDR_S:
        case ADDR_SZ:
            lsp_ElementAddr = S_ELEMENT;
            lsv_ElementCnt = S_RANG;
            break;

        case ADDR_LM:
        case ADDR_LMZ:
            lsp_ElementAddr = LM_ELEMENT;
            lsv_ElementCnt = LM_RANG;
            break;
        default:
            return ERR_OPERANDS;
    }

    /*获取元件开始地址*/
    if(lcv_AddrType >= ADDR_XZ)
        lsv_StartElement = GET_Z_ELEMENT_VALUE(GET_PU8_DATA(ltp_RunEnv->mcp_PC+2)) + GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);
    else
        lsv_StartElement = GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);

    if(lsp_ElementAddr == C_ELEMENT) {
        for(i=0; i<lsv_Cnt; i++) {
            if(lsv_StartElement+i > lsv_ElementCnt) {
                return ERR_OVER_ELEMENT_RANG;
            }
            plc_reset_one_counter(lsv_StartElement+i);
        }
    } else if(lsp_ElementAddr == T_ELEMENT) {
        for(i=0; i<lsv_Cnt; i++) {
            if(lsv_StartElement+i > lsv_ElementCnt) {
                return ERR_OVER_ELEMENT_RANG;
            }
            plc_reset_one_timer(lsv_StartElement+i);
        }
    } else if (lsp_ElementAddr == LM_ELEMENT) {
        for(i=0; i<lsv_Cnt; i++) {
            if(lsv_StartElement+i > lsv_ElementCnt) {
                return ERR_OVER_ELEMENT_RANG;
            }
            plc_set_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_StartElement+i, 0);
        }
    } else {
        for(i=0; i<lsv_Cnt; i++) {
            if(lsv_StartElement+i > lsv_ElementCnt) {
                return ERR_OVER_ELEMENT_RANG;
            }
            plc_set_bit_element_value(lsp_ElementAddr, lsv_StartElement+i, 0);
        }
    }

    return pdPASS;
}

/**
  * @brief  批量位置位指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_zset_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    short lsv_Cnt;
    unsigned short *lsp_ElementAddr;
    unsigned short lsv_ElementCnt, lsv_StartElement, i;
    unsigned char lcv_AddrType = GET_PU8_DATA(ltp_RunEnv->mcp_PC+3);

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_Cnt, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }
    if(lsv_Cnt < 0) {
        return ERR_OPERANDS;
    }

    switch(lcv_AddrType) {
        case ADDR_X:
        case ADDR_XZ:
            lsp_ElementAddr = X_ELEMENT;
            lsv_ElementCnt = X_RANG;
            break;

        case ADDR_Y:
        case ADDR_YZ:
            lsp_ElementAddr = Y_ELEMENT;
            lsv_ElementCnt = Y_RANG;
            break;

        case ADDR_C:
        case ADDR_CZ:
            lsp_ElementAddr = C_ELEMENT;
            lsv_ElementCnt = C_RANG;
            break;

        case ADDR_T:
        case ADDR_TZ:
            lsp_ElementAddr = T_ELEMENT;
            lsv_ElementCnt = T_RANG;
            break;

        case ADDR_M:
        case ADDR_MZ:
            lsp_ElementAddr = M_ELEMENT;
            lsv_ElementCnt = M_RANG;
            break;

        case ADDR_S:
        case ADDR_SZ:
            lsp_ElementAddr = S_ELEMENT;
            lsv_ElementCnt = S_RANG;
            break;

        case ADDR_LM:
        case ADDR_LMZ:
            lsp_ElementAddr = LM_ELEMENT;
            lsv_ElementCnt = LM_RANG;
            break;
        default:
            return ERR_OPERANDS;
    }

    /*获取元件开始地址*/
    if(lcv_AddrType >= ADDR_XZ)
        lsv_StartElement = GET_Z_ELEMENT_VALUE(GET_PU8_DATA(ltp_RunEnv->mcp_PC+2)) + GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);
    else
        lsv_StartElement = GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);

    if (lsp_ElementAddr == LM_ELEMENT) {
        for(i=0; i<lsv_Cnt; i++) {
            if(lsv_StartElement+i > lsv_ElementCnt) {
                return ERR_OVER_ELEMENT_RANG;
            }
            plc_set_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_StartElement+i, 1);
        }
    } else {
        for(i=0; i<lsv_Cnt; i++) {
            if(lsv_StartElement+i > lsv_ElementCnt) {
                return ERR_OVER_ELEMENT_RANG;
            }
            plc_set_bit_element_value(lsp_ElementAddr, lsv_StartElement+i, 1);
        }
    }

    return pdPASS;
}

/**
  * @brief  解码指令
  *         当能流有效时，将字元件D中的第S位，置为1，其它位清为0
  * @param  None
  * @retval None
  */
unsigned char run_ci_deco_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    unsigned short lsv_DestData = 0;
    short lsv_Cnt;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_Cnt, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_Cnt<0 || lsv_Cnt>15) {
        return ERR_OPERANDS;
    }

    lsv_DestData = 1<<lsv_Cnt;

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, &lsv_DestData, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  编码指令
  *         当能流有效时，字元件S中为“1”位的位编号，将被写入D中。
  * @param  None
  * @retval None
  */
unsigned char run_ci_enco_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    unsigned short lsv_DestData;
    unsigned short lsv_SrcData;
    unsigned char i;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_SrcData, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_SrcData == 0) {
        return pdPASS;
    }

    for(i=0; i<16; i++) {
        if((lsv_SrcData>>i) & 0x01) {
            lsv_DestData = i;
            break;
        }
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, &lsv_DestData, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  字中ON位统计指令
  *         当能流有效时，统计操作数S中为“1”位的个数，统计结果存入操作数D中。
  * @param  None
  * @retval None
  */
unsigned char run_ci_bits_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    unsigned short lsv_DestData = 0;
    unsigned short lsv_SrcData;
    unsigned char i;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_SrcData, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_SrcData == 0) {
        return pdPASS;
    }

    for(i=0; i<16; i++) {
        if((lsv_SrcData>>i) & 0x01) {
            lsv_DestData++;
        }
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, &lsv_DestData, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  双字中ON位统计指令
  *         当能流有效时，统计操作数S中为“1”位的个数，统计结果存入操作数D中。
  * @param  None
  * @retval None
  */
unsigned char run_ci_dbits_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    unsigned short lsv_DestData = 0;
    unsigned long llv_SrcData;
    unsigned char i;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, (unsigned long *)&llv_SrcData, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(llv_SrcData == 0) {
        return pdPASS;
    }

    for(i=0; i<32; i++) {
        if((llv_SrcData>>i) & 0x01) {
            lsv_DestData++;
        }
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+8, &lsv_DestData, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  ON位判断
  *         将S1的第S2位的状态输出到D。
  * @param  None
  * @retval None
  */
unsigned char run_ci_bon_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    unsigned short lsv_Data; //, lsv_ElementCnt;
    unsigned short *lsp_ElementAddr;
    unsigned short lsv_StartElement;
    unsigned char lcv_AddrType = GET_PU8_DATA(ltp_RunEnv->mcp_PC+7);
    short lsv_BitIndex;
    unsigned char lcv_Result;

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_Data, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    switch(lcv_AddrType) {
        case ADDR_Y:
        case ADDR_YZ:
            lsp_ElementAddr = Y_ELEMENT;
            //lsv_ElementCnt = Y_RANG;
            break;

        case ADDR_M:
        case ADDR_MZ:
            lsp_ElementAddr = M_ELEMENT;
            //lsv_ElementCnt = M_RANG;
            break;

        case ADDR_S:
        case ADDR_SZ:
            lsp_ElementAddr = S_ELEMENT;
            //lsv_ElementCnt = S_RANG;
            break;
        default:
            return ERR_ELEMENT_TYPE;
    }

    /*获取元件开始地址*/
    if(lcv_AddrType >= ADDR_XZ)
        lsv_StartElement = GET_Z_ELEMENT_VALUE(GET_PU8_DATA(ltp_RunEnv->mcp_PC+6)) + GET_PU16_DATA(ltp_RunEnv->mcp_PC+8);
    else
        lsv_StartElement = GET_PU16_DATA(ltp_RunEnv->mcp_PC+8);

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsv_BitIndex, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_BitIndex < 0 || lsv_BitIndex > 15) {
        return ERR_OPERANDS;
    }

    lcv_Result = (lsv_Data >> lsv_BitIndex)&0x01;

    plc_set_bit_element_value(lsp_ElementAddr, lsv_StartElement, lcv_Result);

    return pdPASS;
}

/*------------------------------------------------------------------------------
*   整数运算指令
*-----------------------------------------------------------------------------*/

/**
  * @brief  整数加法指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_add_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short x, y;
    int z;
    unsigned lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);
    plc_set_bit_element_value(SM_ELEMENT, 181, 0);
    plc_set_bit_element_value(SM_ELEMENT, 182, 0);

    z = x + y;

    if(z > 32767) {
        plc_set_bit_element_value(SM_ELEMENT, 181, 1);
    } else if(z< -32768) {
        plc_set_bit_element_value(SM_ELEMENT, 182, 1);
    } else if(z == 0) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&z, 0, 1);

    return lcv_Ret;
}

/**
  * @brief  整数减法指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_sub_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short x, y;
    int z;
    unsigned lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);
    plc_set_bit_element_value(SM_ELEMENT, 181, 0);
    plc_set_bit_element_value(SM_ELEMENT, 182, 0);

    z = x - y;

    if(z > 32767) {
        plc_set_bit_element_value(SM_ELEMENT, 181, 1);
    } else if(z< -32768) {
        plc_set_bit_element_value(SM_ELEMENT, 182, 1);
    } else if(z == 0) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&z, 0, 1);

    return lcv_Ret;
}

/**
  * @brief  整数乘法指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_mul_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short x, y;
    int z;
    unsigned lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    z = (int)x * y;

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+10, (unsigned long *)&z, 0, 1);

    return lcv_Ret;
}

/**
  * @brief  整数除法指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_div_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short x, y, z;
    unsigned lcv_Ret;
    //PRINTF("Enter %s()\r\n", __func__);
    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(y==0) {
        return ERR_DIV_ZERO;
    }

    z = x/y;
    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&z, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    z = x%y;
    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&z, 1, 1);
    return lcv_Ret;
}

/**
  * @brief  整数算术平方根指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_sqt_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short x, y;
    float z;
    unsigned lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(x<0) {
        return ERR_OPERANDS;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);
    plc_set_bit_element_value(SM_ELEMENT, 182, 0);

    z = sqrtf(x);
    y = (short)z;

    if(y != z) {
        plc_set_bit_element_value(SM_ELEMENT, 182, 1);
    }

    if(y == 0) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&y, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  整数自加一指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_inc_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short x;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    x += 1;

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&x, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  整数自减一指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dec_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short x;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    x -= 1;

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&x, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  整数绝对值指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_vabs_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short x;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(x == -32768) {
        return ERR_OPERANDS;
    }

    if(x<0)
        x = -x;

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&x, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  整数取负指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_neg_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short x;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(x == -32768) {
        return ERR_OPERANDS;
    }

    x = -x;

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&x, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  长整数加法指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dadd_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    int x, y, z;
    unsigned lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, (unsigned long *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+8, (unsigned long *)&y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);
    plc_set_bit_element_value(SM_ELEMENT, 181, 0);
    plc_set_bit_element_value(SM_ELEMENT, 182, 0);

    z = x + y;

    if((x>0)&(y>0)&(z<0)) {
        plc_set_bit_element_value(SM_ELEMENT, 181, 1);
    } else if((x<=0)&(y<0)&(z>0)) {
        plc_set_bit_element_value(SM_ELEMENT, 182, 1);
    }

    if(z == 0) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+14, (unsigned long *)&z, 0, 1);

    return lcv_Ret;
}

/**
  * @brief  长整数减法指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dsub_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    int x, y, z;
    unsigned lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, (unsigned long *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+8, (unsigned long *)&y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);
    plc_set_bit_element_value(SM_ELEMENT, 181, 0);
    plc_set_bit_element_value(SM_ELEMENT, 182, 0);

    z = x - y;

    if((x>0)&(y<0)&(z<0)) {
        plc_set_bit_element_value(SM_ELEMENT, 181, 1);
    } else if((x<0)&(y>0)&(z>0)) {
        plc_set_bit_element_value(SM_ELEMENT, 182, 1);
    }

    if(z == 0) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+14, (unsigned long *)&z, 0, 1);

    return lcv_Ret;
}

/**
  * @brief  长整数乘法指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dmul_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    int x, y, z;
    unsigned lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, (unsigned long *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+8, (unsigned long *)&y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    z = x * y;

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+14, (unsigned long *)&z, 0, 1);

    return lcv_Ret;
}

/**
  * @brief  长整数除法指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_ddiv_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    int x, y, z;
    unsigned lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, (unsigned long *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+8, (unsigned long *)&y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(y==0) {
        return ERR_DIV_ZERO;
    }

    z = x/y;
    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+14, (unsigned long *)&z, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    z = x%y;
    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+14, (unsigned long *)&z, 1, 1);
    return lcv_Ret;
}

/**
  * @brief  长整数算术平方根指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dsqt_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    int x, y;
    float z;
    unsigned lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, (unsigned long *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(x<0) {
        return ERR_OPERANDS;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);
    plc_set_bit_element_value(SM_ELEMENT, 182, 0);

    z = sqrtf(x);
    y = (int)z;

    if(y != z) {
        plc_set_bit_element_value(SM_ELEMENT, 182, 1);
    }

    if(y == 0) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+8, (unsigned long *)&y, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  长整数自加一指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dinc_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    int x;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, (unsigned long *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    x += 1;

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+2, (unsigned long *)&x, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  长整数自减一指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_ddec_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    int x;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, (unsigned long *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    x -= 1;

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+2, (unsigned long *)&x, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  长整数绝对值指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dvabs_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    int x;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, (unsigned long *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(x == -2147483647) {
        return ERR_OPERANDS;
    }

    if(x<0)
        x = -x;

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+8, (unsigned long *)&x, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  长整数取负指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dneg_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    int x;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, (unsigned long *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(x == -2147483647) {
        return ERR_OPERANDS;
    }

    x = -x;

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+8, (unsigned long *)&x, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  整数累加指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_sum_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short lsv_Data;
    unsigned char lcv_Ret;
    short i, lsv_Cnt;
    int liv_Sum = 0;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_Cnt, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_Cnt<0 || lsv_Cnt>255) {
        return ERR_OPERANDS;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);
    plc_set_bit_element_value(SM_ELEMENT, 181, 0);
    plc_set_bit_element_value(SM_ELEMENT, 182, 0);

    for(i=0; i<lsv_Cnt; i++) {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_Data, i, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        liv_Sum += lsv_Data;
    }

    if(liv_Sum == 0) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 0);
    }

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+10, (unsigned long *)&liv_Sum, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  长整数累加指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dsum_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    int lsv_Data;
    unsigned char lcv_Ret;
    short i, lsv_Cnt;
    int liv_Temp, liv_Sum = 0;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+8, (unsigned short *)&lsv_Cnt, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_Cnt<0 || lsv_Cnt>255) {
        return ERR_OPERANDS;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);
    plc_set_bit_element_value(SM_ELEMENT, 181, 0);
    plc_set_bit_element_value(SM_ELEMENT, 182, 0);

    for(i=0; i<lsv_Cnt; i++) {
        lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, (unsigned long *)&lsv_Data, i, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }
        liv_Temp = liv_Sum;
        liv_Sum += lsv_Data;

        if((liv_Temp>0)&&(lsv_Data>0)&&(liv_Sum<0)) {
            plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        } else if((liv_Temp<=0)&&(lsv_Data<0)&&(liv_Sum>0)) {
            plc_set_bit_element_value(SM_ELEMENT, 182, 1);
        }
    }

    if(liv_Sum == 0) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+12, (unsigned long *)&liv_Sum, 0, 1);
    return lcv_Ret;
}

/*------------------------------------------------------------------------------
*   浮点数运算指令
*-----------------------------------------------------------------------------*/
/**
  * @brief  浮点数加法指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_radd_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float x, y, lfv_Sum;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+8, &y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);
    plc_set_bit_element_value(SM_ELEMENT, 181, 0);

    lfv_Sum = x + y;

    if((lfv_Sum == HUGE_VAL) || (lfv_Sum == -HUGE_VAL)) {
        plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        return pdPASS;
    }

    if(lfv_Sum == 0.0f) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_float(ltp_RunEnv->mcp_PC+14, &lfv_Sum, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  浮点数减法指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_rsub_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float x, y, z;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+8, &y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);
    plc_set_bit_element_value(SM_ELEMENT, 181, 0);

    z = x - y;

    if((z == HUGE_VAL) || (z == -HUGE_VAL)) {
        plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        return pdPASS;
    }

    if(z == 0.0f) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_float(ltp_RunEnv->mcp_PC+14, &z, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  浮点数乘法指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_rmul_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float x, y, z;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+8, &y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);
    plc_set_bit_element_value(SM_ELEMENT, 181, 0);

    z = x*y;

    if((z == HUGE_VAL) || (z == -HUGE_VAL)) {
        plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        return pdPASS;
    }

    if(z == 0.0f) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_float(ltp_RunEnv->mcp_PC+14, &z, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  浮点数除法指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_rdiv_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float x, y, z;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+8, &y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }
    if(y == 0.0f) {
        return ERR_DIV_ZERO;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);
    plc_set_bit_element_value(SM_ELEMENT, 181, 0);

    z = x/y;

    if((z == HUGE_VAL) || (z == -HUGE_VAL)) {
        plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        return pdPASS;
    }

    if(z == 0.0f) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_float(ltp_RunEnv->mcp_PC+14, &z, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  浮点数算术平方根指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_rsqt_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float x, z;
    unsigned lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(x<0.0f) {
        return ERR_OPERANDS;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);

    z = sqrtf(x);

    if(z == 0.0f) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_float(ltp_RunEnv->mcp_PC+8, &z, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  浮点数绝对值指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_rvabs_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float x;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    x = fabsf(x);

    lcv_Ret = save_float(ltp_RunEnv->mcp_PC+8, &x, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  浮点数取负指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_rneg_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float x;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    x = -x;

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+8, (unsigned long *)&x, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  浮点数SIN指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_sin_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float x, y;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);

    y = sinf(x);

    if(y==0.0f) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_float(ltp_RunEnv->mcp_PC+8, &y, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  浮点数COS指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_cos_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float x, y;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);

    y = cosf(x);

    if(y==0.0f) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_float(ltp_RunEnv->mcp_PC+8, &y, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  浮点数TAN指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_tan_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float x, y;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);
    plc_set_bit_element_value(SM_ELEMENT, 181, 0);

    y = tanf(x);

    if((y == HUGE_VAL) || (y == -HUGE_VAL)) {
        plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        return pdPASS;
    }

    if(y==0.0f) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_float(ltp_RunEnv->mcp_PC+8, &y, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  浮点数ASIN指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_asin_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float x, y;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if( x<-1 || x>1) {
        return ERR_OPERANDS;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);

    y = asinf(x);

    if(y==0.0f) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_float(ltp_RunEnv->mcp_PC+8, &y, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  浮点数ACOS指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_acos_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float x, y;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }
    if( x<-1 || x>1) {
        return ERR_OPERANDS;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);

    y = acosf(x);

    if(y==0.0f) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_float(ltp_RunEnv->mcp_PC+8, &y, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  浮点数ATAN指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_atan_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float x, y;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);

    y = atanf(x);

    if(y==0.0f) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_float(ltp_RunEnv->mcp_PC+8, &y, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  浮点数求幂指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_power_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float x, y, z;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+8, &y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if((x==0 && y<=0) || (x<0 && y!=(short)y)) {
        return ERR_OPERANDS;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);
    plc_set_bit_element_value(SM_ELEMENT, 181, 0);

    if(x<1 && x>-1 && x!=0) {
        x = 1.0f/x;
        y= -y;
    }

    z = powf(x, y);

    if((z == HUGE_VAL) || (z == -HUGE_VAL)) {
        plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        return pdPASS;
    }

    if(z == 0.0f) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_float(ltp_RunEnv->mcp_PC+14, &z, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  浮点数自然对数指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_ln_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float x, y;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(x <= 0) {
        return ERR_OPERANDS;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);
    plc_set_bit_element_value(SM_ELEMENT, 181, 0);

    y = logf(x);

    if((y == HUGE_VAL) || (y == -HUGE_VAL)) {
        plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        return pdPASS;
    }

    if(y == 0.0f) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_float(ltp_RunEnv->mcp_PC+8, &y, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  浮点数自然数幂指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_exp_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float x, y;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);
    plc_set_bit_element_value(SM_ELEMENT, 181, 0);

    y = expf(x);

    if((y == HUGE_VAL) || (y == -HUGE_VAL)) {
        plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        return pdPASS;
    }

    if(y == 0.0f) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_float(ltp_RunEnv->mcp_PC+8, &y, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  浮点数累加指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_rsum_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float lfv_Data;
    unsigned char lcv_Ret;
    short i, lsv_Cnt;
    float lfv_Sum = 0;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+8, (unsigned short *)&lsv_Cnt, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_Cnt<0 || lsv_Cnt>255) {
        return ERR_OPERANDS;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);
    plc_set_bit_element_value(SM_ELEMENT, 181, 0);

    for(i=0; i<lsv_Cnt; i++) {
        lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &lfv_Data, i, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }
        lfv_Sum += lfv_Data;

        if((lfv_Sum == HUGE_VAL) || (lfv_Sum == -HUGE_VAL)) {
            plc_set_bit_element_value(SM_ELEMENT, 181, 1);
            return pdPASS;
        }
    }

    if(lfv_Sum == 0.0f) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_float(ltp_RunEnv->mcp_PC+12, &lfv_Sum, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  浮点数对数指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_log_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float x, y;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(x <= 0) {
        return ERR_OPERANDS;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);
    plc_set_bit_element_value(SM_ELEMENT, 181, 0);

    y = log10f(x);

    if((y == HUGE_VAL) || (y == -HUGE_VAL)) {
        plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        return pdPASS;
    }

    if(y == 0.0f) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_float(ltp_RunEnv->mcp_PC+8, &y, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  浮点数角度->弧度转化
  * @param  None
  * @retval None
  */
unsigned char run_ci_rad_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float x, y;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);

    y = x*PI/180;

    if(y == 0.0f) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_float(ltp_RunEnv->mcp_PC+8, &y, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  浮点数弧度->角度转化
  * @param  None
  * @retval None
  */
unsigned char run_ci_deg_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float x, y;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);
    plc_set_bit_element_value(SM_ELEMENT, 181, 0);

    y = x*180/PI;

    if((y == HUGE_VAL) || (y == -HUGE_VAL)) {
        plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        return pdPASS;
    }

    if(y == 0.0f) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_float(ltp_RunEnv->mcp_PC+8, &y, 0, 1);
    return lcv_Ret;
}

/*------------------------------------------------------------------------------
*   数值转化指令
*-----------------------------------------------------------------------------*/
/**
  * @brief  长整数转化整数指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dti_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    int liv_Data;
    short lsv_Result;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, (unsigned long *)&liv_Data, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if((liv_Data > 32767) || (liv_Data <-32768)) {
        return ERR_OPERANDS;
    }

    lsv_Result = (short)liv_Data;

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+8, (unsigned short *)&lsv_Result, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  整数转化长整数指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_itd_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short lsv_Data;
    int liv_Result;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_Data, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }


    liv_Result = (long)lsv_Data;

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+6, (unsigned long *)&liv_Result, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  整数转化浮点数指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_flt_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short lsv_Data;
    float lfv_Result;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_Data, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }


    lfv_Result = (float)lsv_Data;

    lcv_Ret = save_float(ltp_RunEnv->mcp_PC+6, &lfv_Result, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  长整数转化浮点数指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dflt_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    long llv_Data;
    float lfv_Result;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, (unsigned long *)&llv_Data, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }


    lfv_Result = (float)llv_Data;

    lcv_Ret = save_float(ltp_RunEnv->mcp_PC+8, &lfv_Result, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  浮点数转化整数指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_int_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float lfv_Data;
    short lsv_Result;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &lfv_Data, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);
    plc_set_bit_element_value(SM_ELEMENT, 181, 0);
    plc_set_bit_element_value(SM_ELEMENT, 182, 0);

    if(lfv_Data > 32767.0f) {
        plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        lsv_Result = 32767;
    } else if(lfv_Data < -32768.0) {
        plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        lsv_Result = -32768;
    } else {
        lsv_Result = (short)lfv_Data;
        if(lsv_Result == 0) {
            plc_set_bit_element_value(SM_ELEMENT, 180, 1);
        }

        if(lsv_Result != lfv_Data) {
            plc_set_bit_element_value(SM_ELEMENT, 182, 1);
        }
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+8, (unsigned short *)&lsv_Result, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  浮点数转化长整数指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dint_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float lfv_Data;
    long llv_Result;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &lfv_Data, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);
    plc_set_bit_element_value(SM_ELEMENT, 181, 0);
    plc_set_bit_element_value(SM_ELEMENT, 182, 0);

    if(lfv_Data > 2147483647.0f) {
        plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        llv_Result = 2147483647;
    } else if(lfv_Data < -2147483648.0) {
        plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        llv_Result = -2147483648;
    } else {
        llv_Result = (short)lfv_Data;
        if(llv_Result == 0) {
            plc_set_bit_element_value(SM_ELEMENT, 180, 1);
        }

        if(llv_Result != lfv_Data) {
            plc_set_bit_element_value(SM_ELEMENT, 182, 1);
        }
    }

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+8, (unsigned long *)&llv_Result, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  字转换16位BCD码指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_bcd_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_Data, lsv_Result = 0;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Data, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_Data > 9999) {
        return ERR_OPERANDS;
    }

    lsv_Result += 0x1000 *(lsv_Data/1000);
    lsv_Result += 0x100*((lsv_Data%1000)/100);
    lsv_Result += 0x10*((lsv_Data%100)/10);
    lsv_Result += lsv_Data%10;

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, &lsv_Result, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  双字转换32位BCD码指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dbcd_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned long llv_Data, llv_Result = 0;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, &llv_Data, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(llv_Data > 99999999) {
        return ERR_OPERANDS;
    }

    llv_Result += 0x10000000 *(llv_Data/100000000);
    llv_Result += 0x1000000 *((llv_Data%10000000)/1000000);
    llv_Result += 0x100000 *((llv_Data%1000000)/100000);
    llv_Result += 0x10000 *((llv_Data%100000)/10000);
    llv_Result += 0x1000 *((llv_Data%10000)/1000);
    llv_Result += 0x100*((llv_Data%1000)/100);
    llv_Result += 0x10*((llv_Data%100)/10);
    llv_Result += llv_Data%10;

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+8, &llv_Result, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  16位BCD码转换字指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_bin_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_Data, lsv_Result = 0;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Data, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if ((((lsv_Data&0xF000)>>12)>9) ||  (((lsv_Data&0x0F00)>>8)>9) || (((lsv_Data&0x00F0)>>4)>9) || ((lsv_Data&0x000F)>9)) {
        return ERR_OPERANDS;
    }

    lsv_Result += ((lsv_Data&0xF000)>>12)*1000;
    lsv_Result += ((lsv_Data&0x0F00)>>8)*100;
    lsv_Result += ((lsv_Data&0x00F0)>>4)*10;
    lsv_Result += (lsv_Data&0x000F);

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, &lsv_Result, 0, 1);
    return lcv_Ret;

}

/**
  * @brief  32位BCD码转换双字指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dbin_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned long llv_Data, llv_Result = 0;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, &llv_Data, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if ((((llv_Data&0xF0000000)>>28)>9) || (((llv_Data&0x0F000000)>>24)>9) || (((llv_Data&0x00F00000)>>20)>9) || (((llv_Data&0x000F0000)>>16)>9) ||
        (((llv_Data&0x0000F000)>>12)>9) ||  (((llv_Data&0x00000F00)>>8)>9) || (((llv_Data&0x000000F0)>>4)>9) || ((llv_Data&0x0000000F)>9)) {
        return ERR_OPERANDS;
    }

    llv_Result += ((llv_Data&0xF0000000)>>28)*10000000;
    llv_Result += ((llv_Data&0x0F000000)>>24)*1000000;
    llv_Result += ((llv_Data&0x00F00000)>>20)*100000;
    llv_Result += ((llv_Data&0x000F0000)>>16)*10000;

    llv_Result += ((llv_Data&0x0000F000)>>12)*1000;
    llv_Result += ((llv_Data&0x00000F00)>>8)*100;
    llv_Result += ((llv_Data&0x000000F0)>>4)*10;
    llv_Result += (llv_Data&0x0000000F);

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+8, &llv_Result, 0, 1);
    return lcv_Ret;

}

/**
  * @brief  字转换16位格雷码指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_gry_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_Data;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Data, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lsv_Data ^= lsv_Data >> 1;

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, &lsv_Data, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  字转换32位格雷码指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dgry_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned long llv_Data;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, &llv_Data, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    llv_Data ^= llv_Data >> 1;

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+8, &llv_Data, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  16位格雷转换字码指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_gbin_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_Data;
    unsigned char lcv_Ret;
    unsigned char i;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Data, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    for(i=0; (1<<i)<16; i++) {
        lsv_Data ^= lsv_Data >> (1<<i);
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, &lsv_Data, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  32位格雷转换双字码指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dgbin_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned long llv_Data;
    unsigned char lcv_Ret;
    unsigned char i;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, &llv_Data, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    for(i=0; (1<<i)<32; i++) {
        llv_Data ^= llv_Data >> (1<<i);
    }

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+8, &llv_Data, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  字转换7段码指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_seg_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_Data, lsv_Result;
    unsigned char lcv_Ret;
    unsigned char lcv_SegArray[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
                                    0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71,
                                    0x73, 0x3E, 0x31, 0x6E, 0x76, 0x38, 0x00
                                   };

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Data, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_Data > 15) {
        return ERR_OPERANDS;
    }

    lsv_Result = lcv_SegArray[lsv_Data];

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, &lsv_Result, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  ASCII码转换指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_asc_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_Data, lsv_Temp=0, lsv_Result[8];
    unsigned char lcv_Ret;
    unsigned char i;
    unsigned char *lcp_Ucode;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcp_Ucode = ltp_RunEnv->mcp_PC + 2;

    if(plc_get_bit_element_value(SM_ELEMENT, 186)) {
        for(i=0; i<8; i++) {
            lcv_Ret = get_word(lcp_Ucode, &lsv_Result[i], 0, 1);
            if(lcv_Ret != pdPASS) {
                return lcv_Ret;
            }
            lcp_Ucode += 4;
        }

        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+34, lsv_Result, 0, 8);
        return lcv_Ret;
    } else {
        for(i=0; i<4; i++) {
            lcv_Ret = get_word(lcp_Ucode, &lsv_Data, 0, 1);
            if(lcv_Ret != pdPASS) {
                return lcv_Ret;
            }

            lsv_Temp = lsv_Data&0xFF;

            lcp_Ucode += 4;

            lcv_Ret = get_word(lcp_Ucode, &lsv_Data, 0, 1);
            if(lcv_Ret != pdPASS) {
                return lcv_Ret;
            }

            lsv_Temp += (lsv_Data&0xFF)<<8;

            lcp_Ucode += 4;

            lsv_Result[i] = lsv_Temp;
            lsv_Temp = 0;
        }

        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+34, lsv_Result, 0, 4);
        return lcv_Ret;
    }
}

/**
  * @brief  16位16进制整数转换ASCII码指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_ita_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_Data, lsv_Result[4], lsv_Cnt;
    unsigned char lcv_Ret;
    unsigned short i;
    unsigned char lca_ASCIICode[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
                                     0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46
                                    };

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, &lsv_Cnt, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_Cnt<1 || lsv_Cnt>256) {
        return ERR_OPERANDS;
    }

    if((GET_PU8_DATA(ltp_RunEnv->mcp_PC+2) == 0x00) && (lsv_Cnt > 4)) {
        lsv_Cnt = 4;
    }


    if(plc_get_bit_element_value(SM_ELEMENT, 186)) {
        for(i=0; i+4<=lsv_Cnt; i+=4) {
            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Data, i>>2, 1);
            if(lcv_Ret != pdPASS) {
                return lcv_Ret;
            }

            lsv_Result[0] = lca_ASCIICode[(lsv_Data >> 12)&0xFF];
            lsv_Result[1] = lca_ASCIICode[(lsv_Data >> 8)&0xFF];
            lsv_Result[2] = lca_ASCIICode[(lsv_Data >> 4)&0xFF];
            lsv_Result[3] = lca_ASCIICode[lsv_Data & 0xFF];

            lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, lsv_Result, i, 4);
        }

        if(i < lsv_Cnt) {
            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Data, i>>2, 1);
            if(lcv_Ret != pdPASS) {
                return lcv_Ret;
            }

            lsv_Result[0] = lca_ASCIICode[(lsv_Data >> 12)&0xFF];
            lsv_Result[1] = lca_ASCIICode[(lsv_Data >> 8)&0xFF];
            lsv_Result[2] = lca_ASCIICode[(lsv_Data >> 4)&0xFF];
            lsv_Result[3] = lca_ASCIICode[lsv_Data & 0xFF];

            lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, lsv_Result, i, lsv_Cnt-i);
        }

        return lcv_Ret;
    } else {
        for(i=0; i+4<=lsv_Cnt; i+=4) {
            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Data, i>>2, 1);
            if(lcv_Ret != pdPASS) {
                return lcv_Ret;
            }

            lsv_Result[0] = lca_ASCIICode[(lsv_Data >> 12)&0xFF];
            lsv_Result[0] += lca_ASCIICode[(lsv_Data >> 8)&0xFF]<<8;
            lsv_Result[1] = lca_ASCIICode[(lsv_Data >> 4)&0xFF];
            lsv_Result[1] += lca_ASCIICode[lsv_Data & 0xFF]<<8;

            lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, lsv_Result, i/2, 2);
        }

        if(i < lsv_Cnt) {
            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Data, i>>2, 1);
            if(lcv_Ret != pdPASS) {
                return lcv_Ret;
            }

            switch(lsv_Cnt - i) {
                case 1:
                    lsv_Result[0] = lca_ASCIICode[(lsv_Data >> 12)&0xFF];
                    break;
                case 2:
                    lsv_Result[0] = lca_ASCIICode[(lsv_Data >> 12)&0xFF];
                    lsv_Result[0] += lca_ASCIICode[(lsv_Data >> 8)&0xFF]<<8;
                    break;
                case 3:
                    lsv_Result[0] = lca_ASCIICode[(lsv_Data >> 12)&0xFF];
                    lsv_Result[0] += lca_ASCIICode[(lsv_Data >> 8)&0xFF]<<8;
                    lsv_Result[1] = lca_ASCIICode[(lsv_Data >> 4)&0xFF];
                    break;
            }

            lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, lsv_Result, i/2, (lsv_Cnt-i)>2?2:1);
        }

        return lcv_Ret;
    }
}

/**
  * @brief  ASCII码转16位16进制数指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_ati_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_Data[4], lsv_Result, lsv_Cnt;
    unsigned char lcv_Ret;
    unsigned i, j;
    unsigned char lca_DeASCIICode[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
                                      };

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, &lsv_Cnt, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_Cnt<1 || lsv_Cnt>256) {
        return ERR_OPERANDS;
    }

    if(GET_PU8_DATA(ltp_RunEnv->mcp_PC+2) == 0x00) {
        if(plc_get_bit_element_value(SM_ELEMENT, 186)) {
            if(lsv_Cnt > 1)
                lsv_Cnt = 1;
        } else {
            if(lsv_Cnt > 2)
                lsv_Cnt = 2;
        }
    }

    if(plc_get_bit_element_value(SM_ELEMENT, 186)) {
        for(i=0; i+4<=lsv_Cnt; i+=4) {
            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, lsv_Data, i, 4);
            if(lcv_Ret != pdPASS) {
                return lcv_Ret;
            }

            lsv_Result = 0;

            for(j=0; j<4; j++) {
                if(IS_HEX_NUM_ASCII(lsv_Data[j])) {
                    lsv_Result = (lsv_Result<<4) + lca_DeASCIICode[lsv_Data[j]-0x30];
                } else {
                    return ERR_OPERANDS;
                }
            }

            lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, &lsv_Result, i/4, 1);
        }

        if(i < lsv_Cnt) {
            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, lsv_Data, i, 4);
            if(lcv_Ret != pdPASS) {
                return lcv_Ret;
            }

            lsv_Result = 0;

            switch(lsv_Cnt - i) {
                case 1:
                    if(IS_HEX_NUM_ASCII(lsv_Data[0])) {
                        lsv_Result = (lsv_Result<<4) + lca_DeASCIICode[lsv_Data[j]-0x30];
                    } else {
                        return ERR_OPERANDS;
                    }
                    break;
                case 2:
                    for(j=0; j<2; j++) {
                        if(IS_HEX_NUM_ASCII(lsv_Data[j])) {
                            lsv_Result = (lsv_Result<<4) + lca_DeASCIICode[lsv_Data[j]-0x30];
                        } else {
                            return ERR_OPERANDS;
                        }
                    }

                    break;
                case 3:
                    for(j=0; j<3; j++) {
                        if(IS_HEX_NUM_ASCII(lsv_Data[j])) {
                            lsv_Result = (lsv_Result<<4) + lca_DeASCIICode[lsv_Data[j]-0x30];
                        } else {
                            return ERR_OPERANDS;
                        }
                    }
                    break;
            }

            lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, &lsv_Result, i/4, 1);
        }

        return lcv_Ret;
    } else {
        for(i=0; i+4<=lsv_Cnt; i+=4) {
            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, lsv_Data, i/2, 2);
            if(lcv_Ret != pdPASS) {
                return lcv_Ret;
            }

            lsv_Result = 0;

            for(j=0; j<2; j++) {
                if(IS_HEX_NUM_ASCII(lsv_Data[j]&0xFF) && IS_HEX_NUM_ASCII((lsv_Data[j]>>8)&0xFF)) {
                    lsv_Result = (lsv_Result<<8) + (lca_DeASCIICode[lsv_Data[j]&0xFF-0x30]<<4) + lca_DeASCIICode[(lsv_Data[j]>>8)&0xFF-0x30];
                } else {
                    return ERR_OPERANDS;
                }
            }

            lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, &lsv_Result, i/4, 1);
        }

        if(i<lsv_Cnt) {
            if(lsv_Cnt - i > 2)
                lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, lsv_Data, i/2, 2);
            else
                lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, lsv_Data, i/2, 1);

            if(lcv_Ret != pdPASS) {
                return lcv_Ret;
            }

            lsv_Result = 0;
            switch(lsv_Cnt -i) {
                case 1:
                    if(IS_HEX_NUM_ASCII(lsv_Data[0]&0xFF))
                        lsv_Result = lca_DeASCIICode[lsv_Data[0]&0xFF-0x30];
                    break;
                case 2:
                    if(IS_HEX_NUM_ASCII(lsv_Data[0]&0xFF) && IS_HEX_NUM_ASCII((lsv_Data[0]>>8)&0xFF)) {
                        lsv_Result = (lca_DeASCIICode[lsv_Data[0]&0xFF-0x30]<<4) + lca_DeASCIICode[(lsv_Data[0]>>8)&0xFF-0x30];
                    }
                    break;
                case 3:
                    if(IS_HEX_NUM_ASCII(lsv_Data[0]&0xFF) && IS_HEX_NUM_ASCII((lsv_Data[0]>>8)&0xFF) && IS_HEX_NUM_ASCII(lsv_Data[1]&0xFF)) {
                        lsv_Result = (lca_DeASCIICode[lsv_Data[0]&0xFF-0x30]<<8) + (lca_DeASCIICode[(lsv_Data[0]>>8)&0xFF-0x30]<<4) + lca_DeASCIICode[lsv_Data[1]&0xFF-0x30];
                    }
                    break;
            }

            lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, &lsv_Result, i/4, 1);
        }

        return lcv_Ret;
    }
}

/*------------------------------------------------------------------------------
*   字逻辑运算指令
*-----------------------------------------------------------------------------*/
/**
  * @brief  字与指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_wand_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short x, y, z;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, &y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    z = x & y;

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+10, &z, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  字或指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_wor_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short x, y, z;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, &y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    z = x | y;

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+10, &z, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  字异或指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_wxor_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short x, y, z;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, &y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    z = x ^ y;

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+10, &z, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  字位取反指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_winv_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short x, z;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    z = ~x;

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, &z, 0, 1);
    return lcv_Ret;

}

/**
  * @brief  双字与指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dwand_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned long x, y, z;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, (unsigned long *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+8, (unsigned long *)&y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    z = x & y;

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+14, (unsigned long *)&z, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  双字或指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dwor_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned long x, y, z;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, (unsigned long *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+8, (unsigned long *)&y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    z = x | y;

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+14, (unsigned long *)&z, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  双字异或指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dwxor_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned long x, y, z;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, (unsigned long *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+8, (unsigned long *)&y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    z = x ^ y;

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+14, (unsigned long *)&z, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  双字位取反指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dwinv_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned long x, z;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, (unsigned long *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    z = ~x;

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+8, (unsigned long *)&z, 0, 1);
    return lcv_Ret;

}

/*------------------------------------------------------------------------------
*   位移动旋转指令
*-----------------------------------------------------------------------------*/
/**
  * @brief  16位循环右移指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_ror_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short x, z;
    short y;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }
    /*取移动位数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(y<0) {
        return ERR_OPERANDS;
    }

    if(y == 0) {
        z = x;
    } else {
        y = y%16;
        z = x>>y | x<<(16-y);
        plc_set_bit_element_value(SM_ELEMENT, 181, (z>>15));
    }
    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&z, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  16位循环左移指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_rol_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short x, z;
    short y;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }
    /*取移动位数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(y<0) {
        return ERR_OPERANDS;
    }

    if(y == 0) {
        z = x;
    } else {
        y = y%16;
        z = x<<y | x>>(16-y);
        plc_set_bit_element_value(SM_ELEMENT, 181, (z&0x01));
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&z, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  16位带标志位循环右移指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_rcr_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short x, z;
    short y;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }
    /*取移动位数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(y<0) {
        return ERR_OPERANDS;
    }

    y = y%17;
    if(y == 0) {
        z = x;
    } else {
        z = x>>y | (x<<(17-y)) + (plc_get_bit_element_value(SM_ELEMENT, 181)<<(16-y));
        plc_set_bit_element_value(SM_ELEMENT, 181, (x>>(y-1)&0x01));
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&z, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  16位带标志位循环左移指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_rcl_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short x, z;
    short y;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }
    /*取移动位数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(y<0) {
        return ERR_OPERANDS;
    }

    y = y%17;
    if(y == 0) {
        z = x;
    } else {
        z = x<<y | (x>>(17-y)) + (plc_get_bit_element_value(SM_ELEMENT, 181)<<(y-1));
        plc_set_bit_element_value(SM_ELEMENT, 181, (x>>(16-y)&0x01));
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&z, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  32位循环右移指令
  * @param  None
  * @retval None
  */

unsigned char run_ci_dror_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned long x, z;
    short y;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }
    /*取移动位数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+14, (unsigned short *)&y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(y<0) {
        return ERR_OPERANDS;
    }

    if(y == 0) {
        z = x;
    } else {
        y = y%32;
        z = x>>y | x<<(32-y);
        plc_set_bit_element_value(SM_ELEMENT, 181, (z>>31));
    }

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+8, &z, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  32位循环左移指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_drol_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned long x, z;
    short y;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }
    /*取移动位数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+14, (unsigned short *)&y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(y<0) {
        return ERR_OPERANDS;
    }

    if(y == 0) {
        z = x;
    } else {
        y = y%32;
        z = x<<y | x>>(32-y);
        plc_set_bit_element_value(SM_ELEMENT, 181, (z&0x01));
    }

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+8, &z, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  32位带标志位循环右移指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_drcr_ins(plc_run_power_flow_st *ltp_RunEnv)
{
#if SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_SDK
        static uint32_t counts = 0;
#endif

    unsigned long x, z;
    short y;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }
#if SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_SDK
        counts++;
#endif

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }
    /*取移动位数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+14, (unsigned short *)&y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }
#if SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_SDK
    if (counts % 3000 == 0)
    {
        LOGD("dp", "x = 0x%08X, y = %d", x, y);
    }
#endif

    if(y<0) {
        return ERR_OPERANDS;
    }

    y = y%33;
    if(y == 0) {
        z = x;
    } else {
        z = x>>y | (x<<(33-y)) + (plc_get_bit_element_value(SM_ELEMENT, 181)<<(32-y));
        plc_set_bit_element_value(SM_ELEMENT, 181, (x>>(y-1)&0x01));
    }

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+8, &z, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  32位带标志位循环左移指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_drcl_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned long x, z;
    short y;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }
    /*取移动位数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+14, (unsigned short *)&y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(y<0) {
        return ERR_OPERANDS;
    }

    y = y%33;
    if(y == 0) {
        z = x;
    } else {
        z = x<<y | (x>>(33-y)) + (plc_get_bit_element_value(SM_ELEMENT, 181)<<(y-1));
        plc_set_bit_element_value(SM_ELEMENT, 181, (x>>(32-y)&0x01));
    }

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+8, &z, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  16位右移指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_shr_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short x, z;
    short y;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }
    /*取移动位数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(y<0) {
        return ERR_OPERANDS;
    }

    if(y >= 16) {
        z = 0;
    } else {
        z = x>>y;
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&z, 0, 1);
    return lcv_Ret;

}

/**
  * @brief  16位左移指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_shl_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short x, z;
    short y;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }
    /*取移动位数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(y<0) {
        return ERR_OPERANDS;
    }

    if(y >= 16) {
        z = 0;
    } else {
        z = x<<y;
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&z, 0, 1);
    return lcv_Ret;

}

/**
  * @brief  32位右移指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dshr_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned long x, z;
    short y;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }
    /*取移动位数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+14, (unsigned short *)&y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(y<0) {
        return ERR_OPERANDS;
    }

    if(y >= 32) {
        z = 0;
    } else {
        z = x>>y;
    }

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+8, &z, 0, 1);
    return lcv_Ret;

}

/**
  * @brief  32位左移指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dshl_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned long x, z;
    short y;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, &x, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }
    /*取移动位数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&y, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(y<0) {
        return ERR_OPERANDS;
    }

    if(y >= 32) {
        z = 0;
    } else {
        z = x<<y;
    }

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+6, &z, 0, 1);
    return lcv_Ret;

}

/**
  * @brief  位串右移指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_sftr_ins(plc_run_power_flow_st *ltp_RunEnv)
{
#if SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_SDK
    static uint32_t counts = 0;
#endif
    short s1, s2, i;
    unsigned short *lsp_Ucode = (unsigned short *)ltp_RunEnv->mcp_PC;
    unsigned char lcv_Ret;
    char x;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }
#if SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_SDK
    counts++;
#endif
    /*取位串长度S1*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*取右移位数S2*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+14, (unsigned short *)&s2, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*参数合法性判断*/
    if(s1<0 || s2<0 || s2>s1 || s1>1024) {
        return ERR_OPERANDS;
    }

    /*S元件与D元件地址重叠判断*/
    if(GET_PU16_DATA(lsp_Ucode+1) == GET_PU16_DATA(lsp_Ucode+3)) {
        if((GET_PU16_DATA(lsp_Ucode+4) <= GET_PU16_DATA(lsp_Ucode+2)) &&
           (GET_PU16_DATA(lsp_Ucode+2) < GET_PU16_DATA(lsp_Ucode+4)+s1)) {
            return ERR_OPERANDS;
        }

        if((GET_PU16_DATA(lsp_Ucode+4) <= GET_PU16_DATA(lsp_Ucode+2)+s2) &&
           (GET_PU16_DATA(lsp_Ucode+2)+s2 < GET_PU16_DATA(lsp_Ucode+4)+s1)) {
            return ERR_OPERANDS;
        }
    }
#if SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_SDK
    if (counts % 3000 == 0)
    {
        LOGD("dp", "s1 = %d, s2 = %d", s1, s2); // s1 - s2 = 7
    }
#endif
    /*目的数右移*/
    for(i=0; i<s1-s2; i++) {
        lcv_Ret = get_char(ltp_RunEnv->mcp_PC+6, (unsigned char *)&x, s2 + i, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        lcv_Ret = save_char(ltp_RunEnv->mcp_PC+6, (unsigned char *)&x, i, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }
    }

    /*写入源位串内容*/
    for(i=0; i<s2; i++) {
        lcv_Ret = get_char(ltp_RunEnv->mcp_PC+2, (unsigned char *)&x, i, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        lcv_Ret = save_char(ltp_RunEnv->mcp_PC+6, (unsigned char *)&x, s1-s2+i, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }
    }

    return pdPASS;
}

/**
  * @brief  位串左移指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_sftl_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short s1, s2, i;
    unsigned short *lsp_Ucode = (unsigned short *)ltp_RunEnv->mcp_PC;
    unsigned char lcv_Ret;
    char x;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    /*取位串长度S1*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&s1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*取右移位数S2*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+14, (unsigned short *)&s2, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*参数合法性判断*/
    if(s1<0 || s2<0 || s2>s1 || s1>1024) {
        return ERR_OPERANDS;
    }

    /*S元件与D元件地址重叠判断*/
    if(GET_PU16_DATA(lsp_Ucode+1) == GET_PU16_DATA(lsp_Ucode+3)) {
        if((GET_PU16_DATA(lsp_Ucode+4) <= GET_PU16_DATA(lsp_Ucode+2)) &&
           (GET_PU16_DATA(lsp_Ucode+2) < GET_PU16_DATA(lsp_Ucode+4)+s1)) {
            return ERR_OPERANDS;
        }

        if((GET_PU16_DATA(lsp_Ucode+4) <= GET_PU16_DATA(lsp_Ucode+2)+s2) &&
           (GET_PU16_DATA(lsp_Ucode+2)+s2 < GET_PU16_DATA(lsp_Ucode+4)+s1)) {
            return ERR_OPERANDS;
        }
    }

    /*目的数左移*/
    for(i=0; i<s1-s2; i++) {
        lcv_Ret = get_char(ltp_RunEnv->mcp_PC+6, (unsigned char *)&x, s1-s2-1-i, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        lcv_Ret = save_char(ltp_RunEnv->mcp_PC+6, (unsigned char *)&x, s1-1-i, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }
    }

    /*写入源位串内容*/
    for(i=0; i<s2; i++) {
        lcv_Ret = get_char(ltp_RunEnv->mcp_PC+2, (unsigned char *)&x, i, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        lcv_Ret = save_char(ltp_RunEnv->mcp_PC+6, (unsigned char *)&x, i, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }
    }

    return pdPASS;
}

/*------------------------------------------------------------------------------
*   数据块处理指令
*-----------------------------------------------------------------------------*/
/**
  * @brief  数据块加法
  * @param  None
  * @retval None
  */
unsigned char run_ci_bkadd_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    short lsv_BlockSize;
    short *lsp_DestBuff;
    short lsv_ConstData, lsv_TempData;
    unsigned short i;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    /*取运算数据数量*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+14, (unsigned short *)&lsv_BlockSize, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_BlockSize <= 0) {
        return ERR_OPERANDS;
    }

    /*申请运算结果缓存*/
    lsp_DestBuff = (short *)pvPortMalloc(sizeof(short)*lsv_BlockSize);
    configASSERT(lsp_DestBuff != NULL);

    /*S2 为常数*/
    if(((GET_PU16_DATA(ltp_RunEnv->mcp_PC+6) >> 8)&0xFF) == 0x00FF) {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_ConstData, 0, 1);
        if(lcv_Ret != pdPASS) {
            vPortFree(lsp_DestBuff);
            return lcv_Ret;
        }

        for(i=0; i<lsv_BlockSize; i++) {
            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_TempData, i, 1);
            if(lcv_Ret != pdPASS) {
                vPortFree(lsp_DestBuff);
                return lcv_Ret;
            }

            lsp_DestBuff[i] = lsv_TempData + lsv_ConstData;
        }
    } else {
        for(i=0; i<lsv_BlockSize; i++) {
            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_TempData, i, 1);
            if(lcv_Ret != pdPASS) {
                vPortFree(lsp_DestBuff);
                return lcv_Ret;
            }

            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_ConstData, i, 1);
            if(lcv_Ret != pdPASS) {
                vPortFree(lsp_DestBuff);
                return lcv_Ret;
            }

            lsp_DestBuff[i] = lsv_TempData + lsv_ConstData;
        }
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsp_DestBuff[0], 0, lsv_BlockSize);

    vPortFree(lsp_DestBuff);

    return lcv_Ret;
}

/**
  * @brief  数据块减法
  * @param  None
  * @retval None
  */
unsigned char run_ci_bksub_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    short lsv_BlockSize;
    short *lsp_DestBuff;
    short lsv_ConstData, lsv_TempData;
    unsigned short i;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    /*取运算数据数量*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+14, (unsigned short *)&lsv_BlockSize, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_BlockSize <= 0) {
        return ERR_OPERANDS;
    }

    /*申请运算结果缓存*/
    lsp_DestBuff = (short *)pvPortMalloc(sizeof(short)*lsv_BlockSize);
    configASSERT(lsp_DestBuff != NULL);

    /*S2 为常数*/
    if(((GET_PU16_DATA(ltp_RunEnv->mcp_PC+6) >> 8)&0xFF) == 0x00FF) {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_ConstData, 0, 1);
        if(lcv_Ret != pdPASS) {
            vPortFree(lsp_DestBuff);
            return lcv_Ret;
        }

        for(i=0; i<lsv_BlockSize; i++) {
            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_TempData, i, 1);
            if(lcv_Ret != pdPASS) {
                vPortFree(lsp_DestBuff);
                return lcv_Ret;
            }

            lsp_DestBuff[i] = lsv_TempData - lsv_ConstData;
        }
    } else {
        for(i=0; i<lsv_BlockSize; i++) {
            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_TempData, i, 1);
            if(lcv_Ret != pdPASS) {
                vPortFree(lsp_DestBuff);
                return lcv_Ret;
            }

            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_ConstData, i, 1);
            if(lcv_Ret != pdPASS) {
                vPortFree(lsp_DestBuff);
                return lcv_Ret;
            }

            lsp_DestBuff[i] = lsv_TempData - lsv_ConstData;
        }
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsp_DestBuff[0], 0, lsv_BlockSize);

    vPortFree(lsp_DestBuff);

    return lcv_Ret;
}

#define BKCMP_SM_NUM    188
/**
  * @brief  数据块比较
  * @param  None
  * @retval None
  */
unsigned char run_ci_bkcmp_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    short lsv_BlockSize;
    unsigned char *lcp_DestBuff;
    short lsv_Op1Data, lsv_Op2Data;
    unsigned short i;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    /*取运算数据数量*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+14, (unsigned short *)&lsv_BlockSize, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_BlockSize <= 0) {
        return ERR_OPERANDS;
    }

    /*申请运算结果缓存*/
    lcp_DestBuff = (unsigned char *)pvPortMalloc(lsv_BlockSize);
    configASSERT(lcp_DestBuff != NULL);

    plc_set_bit_element_value(SM_ELEMENT, BKCMP_SM_NUM, 1);

    /*S1 为常数*/
    if(((GET_PU16_DATA(ltp_RunEnv->mcp_PC+2) >> 8)&0xFF) == 0x00FF) {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_Op1Data, 0, 1);
        if(lcv_Ret != pdPASS) {
            vPortFree(lcp_DestBuff);
            plc_set_bit_element_value(SM_ELEMENT, BKCMP_SM_NUM, 0);
            return lcv_Ret;
        }

        for(i=0; i<lsv_BlockSize; i++) {
            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_Op2Data, i, 1);
            if(lcv_Ret != pdPASS) {
                vPortFree(lcp_DestBuff);
                plc_set_bit_element_value(SM_ELEMENT, BKCMP_SM_NUM, 0);
                return lcv_Ret;
            }

            switch(GET_PU16_DATA(ltp_RunEnv->mcp_PC)) {

                case CI_BKCMPDE:
                    if(lsv_Op1Data == lsv_Op2Data) {
                        lcp_DestBuff[i] = 1;
                    } else {
                        lcp_DestBuff[i] = 0;
                        plc_set_bit_element_value(SM_ELEMENT, BKCMP_SM_NUM, 0);
                    }
                    break;

                case CI_BKCMPDG:
                    if(lsv_Op1Data > lsv_Op2Data) {
                        lcp_DestBuff[i] = 1;
                    } else {
                        lcp_DestBuff[i] = 0;
                        plc_set_bit_element_value(SM_ELEMENT, BKCMP_SM_NUM, 0);
                    }
                    break;

                case CI_BKCMPDL:
                    if(lsv_Op1Data < lsv_Op2Data) {
                        lcp_DestBuff[i] = 1;
                    } else {
                        lcp_DestBuff[i] = 0;
                        plc_set_bit_element_value(SM_ELEMENT, BKCMP_SM_NUM, 0);
                    }
                    break;

                case CI_BKCMPDNE:
                    if(lsv_Op1Data != lsv_Op2Data) {
                        lcp_DestBuff[i] = 1;
                    } else {
                        lcp_DestBuff[i] = 0;
                        plc_set_bit_element_value(SM_ELEMENT, BKCMP_SM_NUM, 0);
                    }
                    break;

                case CI_BKCMPDLE:
                    if(lsv_Op1Data <= lsv_Op2Data) {
                        lcp_DestBuff[i] = 1;
                    } else {
                        lcp_DestBuff[i] = 0;
                        plc_set_bit_element_value(SM_ELEMENT, BKCMP_SM_NUM, 0);
                    }
                    break;

                case CI_BKCMPDGE:
                    if(lsv_Op1Data >= lsv_Op2Data) {
                        lcp_DestBuff[i] = 1;
                    } else {
                        lcp_DestBuff[i] = 0;
                        plc_set_bit_element_value(SM_ELEMENT, BKCMP_SM_NUM, 0);
                    }
                    break;

            }
        }
    } else {
        for(i=0; i<lsv_BlockSize; i++) {
            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_Op1Data, i, 1);
            if(lcv_Ret != pdPASS) {
                vPortFree(lcp_DestBuff);
                plc_set_bit_element_value(SM_ELEMENT, BKCMP_SM_NUM, 0);
                return lcv_Ret;
            }

            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_Op2Data, i, 1);
            if(lcv_Ret != pdPASS) {
                vPortFree(lcp_DestBuff);
                plc_set_bit_element_value(SM_ELEMENT, BKCMP_SM_NUM, 0);
                return lcv_Ret;
            }

            switch(GET_PU16_DATA(ltp_RunEnv->mcp_PC)) {

                case CI_BKCMPDE:
                    if(lsv_Op1Data == lsv_Op2Data) {
                        lcp_DestBuff[i] = 1;
                    } else {
                        lcp_DestBuff[i] = 0;
                        plc_set_bit_element_value(SM_ELEMENT, BKCMP_SM_NUM, 0);
                    }
                    break;

                case CI_BKCMPDG:
                    if(lsv_Op1Data > lsv_Op2Data) {
                        lcp_DestBuff[i] = 1;
                    } else {
                        lcp_DestBuff[i] = 0;
                        plc_set_bit_element_value(SM_ELEMENT, BKCMP_SM_NUM, 0);
                    }
                    break;

                case CI_BKCMPDL:
                    if(lsv_Op1Data < lsv_Op2Data) {
                        lcp_DestBuff[i] = 1;
                    } else {
                        lcp_DestBuff[i] = 0;
                        plc_set_bit_element_value(SM_ELEMENT, BKCMP_SM_NUM, 0);
                    }
                    break;

                case CI_BKCMPDNE:
                    if(lsv_Op1Data != lsv_Op2Data) {
                        lcp_DestBuff[i] = 1;
                    } else {
                        lcp_DestBuff[i] = 0;
                        plc_set_bit_element_value(SM_ELEMENT, BKCMP_SM_NUM, 0);
                    }
                    break;

                case CI_BKCMPDLE:
                    if(lsv_Op1Data <= lsv_Op2Data) {
                        lcp_DestBuff[i] = 1;
                    } else {
                        lcp_DestBuff[i] = 0;
                        plc_set_bit_element_value(SM_ELEMENT, BKCMP_SM_NUM, 0);
                    }
                    break;

                case CI_BKCMPDGE:
                    if(lsv_Op1Data >= lsv_Op2Data) {
                        lcp_DestBuff[i] = 1;
                    } else {
                        lcp_DestBuff[i] = 0;
                        plc_set_bit_element_value(SM_ELEMENT, BKCMP_SM_NUM, 0);
                    }
                    break;
            }
        }
    }

    lcv_Ret = save_char(ltp_RunEnv->mcp_PC+10, lcp_DestBuff, 0, lsv_BlockSize);

    vPortFree(lcp_DestBuff);

    return lcv_Ret;
}

/*------------------------------------------------------------------------------
*   LM 元件相关指令
*-----------------------------------------------------------------------------*/

/**
  * @brief  LM常开触点指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_ld_lm_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_ElementIndex;

    /*能流入栈*/
    PUSH_POWER_FLOW(ltp_RunEnv);

    lsv_ElementIndex = GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);

    if(plc_get_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex)) {
        SET_POWER_FLOW(ltp_RunEnv);
    } else {
        RST_POWER_FLOW(ltp_RunEnv);
    }

    return pdPASS;
}

/**
  * @brief  LM常闭触点指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_ldi_lm_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_ElementIndex;

    /*能流入栈*/
    PUSH_POWER_FLOW(ltp_RunEnv);

    lsv_ElementIndex = GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);

    if(plc_get_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex)) {
        RST_POWER_FLOW(ltp_RunEnv);
    } else {
        SET_POWER_FLOW(ltp_RunEnv);
    }

    return pdPASS;
}

/**
  * @brief  LM常触点与指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_and_lm_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_ElementIndex;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lsv_ElementIndex = GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);

    if(!plc_get_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex)) {
        RST_POWER_FLOW(ltp_RunEnv);
    }

    return pdPASS;
}

/**
  * @brief  LM触点与非指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_ani_lm_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_ElementIndex;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lsv_ElementIndex = GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);

    if(plc_get_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex)) {
        RST_POWER_FLOW(ltp_RunEnv);
    }

    return pdPASS;
}

/**
  * @brief  LM触点或指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_or_lm_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_ElementIndex;

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lsv_ElementIndex = GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);

    if(plc_get_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex)) {
        SET_POWER_FLOW(ltp_RunEnv);
    }

    return pdPASS;
}

/**
  * @brief  LM触点或非指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_ori_lm_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_ElementIndex;

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lsv_ElementIndex = GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);

    if(!plc_get_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex)) {
        SET_POWER_FLOW(ltp_RunEnv);
    }

    return pdPASS;
}

/**
  * @brief  LM触点置位指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_set_lm_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_ElementIndex;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lsv_ElementIndex = GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);

    plc_set_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex, 1);

    return pdPASS;
}

/**
  * @brief  LM触点输出指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_out_lm_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_ElementIndex;
    lsv_ElementIndex = GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        plc_set_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex, 1);
    } else {
        plc_set_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex, 0);
    }

    return pdPASS;
}

/**
  * @brief  LM触点复位指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_rst_lm_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_ElementIndex;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lsv_ElementIndex = GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);

    plc_set_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex, 0);

    return pdPASS;
}

/*------------------------------------------------------------------------------
*   数据传输指令
*-----------------------------------------------------------------------------*/
/**
  * @brief  子数据传输指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_mov_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_Data;
    unsigned char lcv_Ret;

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Data, 0, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, &lsv_Data, 0, 1);
        return lcv_Ret;
    }
    return pdPASS;
}

/**
  * @brief  浮点数传输指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_rmov_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float lfv_Data;
    unsigned char lcv_Ret;

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, &lfv_Data, 0, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        lcv_Ret = save_float(ltp_RunEnv->mcp_PC+8, &lfv_Data, 0, 1);
        return lcv_Ret;
    }
    return pdPASS;
}

/**
  * @brief  双字传输指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dmov_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned long llv_Data;
    unsigned char lcv_Ret;

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, &llv_Data, 0, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+8, &llv_Data, 0, 1);
        return lcv_Ret;
    }
    return pdPASS;
}

/**
  * @brief  块数据传输指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_bmov_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_SrcData;
    short lsv_Cnt;
    unsigned char lcv_Ret;
    unsigned short i;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsv_Cnt, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_Cnt <= 0) {
        return ERR_OPERANDS;
    }

    for(i=0; i<lsv_Cnt; i++) {

        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_SrcData, i, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, &lsv_SrcData, i, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }
    }
    return pdPASS;
}

/**
  * @brief  块数据填充指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_fmov_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_SrcData, lsv_Temp;
    short lsv_Cnt;
    unsigned char lcv_Ret;
    unsigned short i;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsv_Cnt, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_Cnt <= 0) {
        return ERR_OPERANDS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_SrcData, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    for(i=0; i<lsv_Cnt; i++) {
        lsv_Temp = lsv_SrcData;
        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, &lsv_Temp, i, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }
    }
    return pdPASS;
}

/**
  * @brief  块数据双字填充指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dfmov_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned long llv_SrcData, llv_Temp;
    short lsv_Cnt;
    unsigned char lcv_Ret;
    unsigned short i;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+14, (unsigned short *)&lsv_Cnt, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_Cnt <= 0) {
        return ERR_OPERANDS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, &llv_SrcData, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    for(i=0; i<lsv_Cnt; i++) {
        llv_Temp = llv_SrcData;
        lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+8, &llv_Temp, i, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }
    }
    return pdPASS;
}

/**
  * @brief  高低字节交换指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_swap_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    unsigned short lsv_Data;
    unsigned short lsv_Temp;


    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Data, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lsv_Temp = ((lsv_Data & 0xFF)<<8) | ((lsv_Data >> 8)&0xFF);

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+2, &lsv_Temp, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  块数据双字字交换指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_swapword_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    unsigned short *lsp_SrcPtr, *lsp_DestPtr;
    unsigned short lsv_Cnt;
    unsigned short i;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+14, &lsv_Cnt, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_Cnt == 0) {
        return pdPASS;
    }

    lsv_Cnt <<= 1;

    lcv_Ret = get_word_addr(ltp_RunEnv->mcp_PC+2, &lsp_SrcPtr, lsv_Cnt);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word_addr(ltp_RunEnv->mcp_PC+8, &lsp_DestPtr, lsv_Cnt);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lsv_Cnt >>= 1;

    for(i=0; i<lsv_Cnt; i++) {
        lsp_DestPtr[2*i] = lsp_SrcPtr[2*i+1];
        lsp_DestPtr[2*i+1] = lsp_SrcPtr[2*i];
    }

    return pdPASS;
}

/**
  * @brief  字交换指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_xch_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    unsigned short lsv_Ch1, lsv_Ch2;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Ch1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, &lsv_Ch2, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+2, &lsv_Ch2, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, &lsv_Ch1, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  双字交换指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dxch_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    unsigned long llv_Ch1, llv_Ch2;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+2, &llv_Ch1, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_dword(ltp_RunEnv->mcp_PC+8, &llv_Ch2, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+2, &llv_Ch2, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = save_dword(ltp_RunEnv->mcp_PC+8, &llv_Ch1, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  数据入栈指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_push_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short lsv_StackSize, lsv_Data, lsv_StackIndex;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    /*取栈长度*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsv_StackSize, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_StackSize <= 0) {
        return ERR_STACK_DEFINE;
    }

    /*取栈顶位置*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_StackIndex, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_StackIndex < 0) {
        return ERR_STACK_DEFINE;
    }

    if(lsv_StackIndex > lsv_StackSize) {
        plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        return ERR_STACK_DEFINE;
    }

    /*清溢出标志*/
    plc_set_bit_element_value(SM_ELEMENT, 181, 0);

    if(lsv_StackIndex == lsv_StackSize) {
        plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        return pdPASS;
    }

    /*取入栈值*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_Data, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*栈顶后移*/
    lsv_StackIndex ++;

    /*数据入栈*/
    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_Data, lsv_StackIndex, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*保存栈顶值*/
    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_StackIndex, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  FIFO队列指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_fifo_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short lsv_QueueSize, lsv_OutData, lsv_QueueLen;
    unsigned char lcv_Ret;
    unsigned short i;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    /*取队列长度*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsv_QueueSize, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_QueueSize <= 0) {
        return ERR_STACK_DEFINE;
    }

    /*取队列中元素个数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_QueueLen, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_QueueLen <0 || lsv_QueueLen > lsv_QueueSize) {
        return ERR_STACK_DEFINE;
    }

    /*清零标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);

    /*队列为空*/
    if(lsv_QueueLen == 0) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
        return pdPASS;
    }

    /*取队列头元素值*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_OutData, 1, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_OutData, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*队列剩余元素移动*/
    for(i=1; i<lsv_QueueLen; i++) {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_OutData, i+1, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_OutData, i, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }
    }

    /*保存队列中元素个数*/
    lsv_QueueLen--;
    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_QueueLen, 0, 1);

    return lcv_Ret;
}

/**
  * @brief  LIFO栈指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_lifo_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short lsv_StackSize, lsv_OutData, lsv_StackLen;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    /*取栈长度*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsv_StackSize, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_StackSize <= 0) {
        return ERR_STACK_DEFINE;
    }

    /*取栈中元素个数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_StackLen, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_StackLen <0 || lsv_StackLen > lsv_StackSize) {
        return ERR_STACK_DEFINE;
    }

    /*清零标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);

    /*栈为空*/
    if(lsv_StackLen == 0) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
        return pdPASS;
    }

    /*取栈顶素值*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_OutData, lsv_StackLen, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_OutData, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*保存栈中元素个数*/
    lsv_StackLen--;
    if(lsv_StackSize == 0) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }
    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_StackLen, 0, 1);

    return lcv_Ret;
}

/**
  * @brief  字串右移指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_wsfr_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short lsv_SrcLen, lsv_DestLen;
    unsigned short lsv_Temp;
    unsigned char lcv_Ret;
    unsigned short i;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    /*取目标字串长度*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsv_DestLen, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*取源字符串长度*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+14, (unsigned short *)&lsv_SrcLen, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_DestLen<0 || lsv_SrcLen<0 || lsv_SrcLen>lsv_DestLen) {
        return ERR_OPERANDS;
    }

    /*目标字串右移*/
    for(i=0; i<(lsv_DestLen - lsv_SrcLen); i++) {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, &lsv_Temp, i+lsv_SrcLen, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, &lsv_Temp, i, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }
    }

    /*移入源字串*/
    for(i=0; i < lsv_SrcLen; i++) {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Temp, i, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, &lsv_Temp, i+(lsv_DestLen - lsv_SrcLen), 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }
    }

    return pdPASS;
}

/**
  * @brief  字串左移指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_wsfl_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short lsv_SrcLen, lsv_DestLen;
    unsigned short lsv_Temp;
    unsigned char lcv_Ret;
    unsigned short i;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    /*取目标字串长度*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsv_DestLen, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*取源字符串长度*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+14, (unsigned short *)&lsv_SrcLen, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_DestLen<0 || lsv_SrcLen<0 || lsv_SrcLen>lsv_DestLen) {
        return ERR_OPERANDS;
    }

    /*目标字串右移*/
    for(i=lsv_DestLen; i>lsv_SrcLen; i++) {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, &lsv_Temp, (i-lsv_SrcLen-1), 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, &lsv_Temp, i-1, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }
    }

    /*移入源字串*/
    for(i=0; i < lsv_SrcLen; i++) {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Temp, i, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, &lsv_Temp, i, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }
    }

    return pdPASS;
}

/*------------------------------------------------------------------------------
*   数据处理指令
*-----------------------------------------------------------------------------*/
/**
  * @brief  平均值指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_mean_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    short lsv_Avg=0, lsv_DataCnt;
    long  llv_sum = 0;
    short *lsp_Data;

    unsigned char i;
    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    /*取数据个数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsv_DataCnt, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_DataCnt<1 || lsv_DataCnt >64) {
        return ERR_OPERANDS;
    }

    /*清除相关标志*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);
    plc_set_bit_element_value(SM_ELEMENT, 181, 0);
    plc_set_bit_element_value(SM_ELEMENT, 182, 0);

    lsp_Data = (short *)pvPortMalloc(sizeof(short)*lsv_DataCnt);
    configASSERT(lsp_Data != NULL);

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)lsp_Data, 0, lsv_DataCnt);
    if(lcv_Ret != pdPASS) {
        vPortFree(lsp_Data);
        return lcv_Ret;
    }

    for(i=0; i<lsv_DataCnt; i++) {
        llv_sum += lsp_Data[i];
    }

    lsv_Avg = llv_sum / lsv_DataCnt;

    if(!lsv_Avg)
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_Avg, 0, 1);

    vPortFree(lsp_Data);

    return lcv_Ret;
}

/**
  * @brief  字节单位的数据分离指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_wtob_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    short lsv_WordData, lsav_Byte[2], lsv_DataCnt;
    unsigned short i;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    /*获取转换数据个数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsv_DataCnt, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_DataCnt < 0) {
        return ERR_OPERANDS;
    }

    for(i=0; i<lsv_DataCnt/2; i++) {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_WordData, i, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        lsav_Byte[0] = (lsv_WordData >> 8) & 0xFF;
        lsav_Byte[1] = lsv_WordData & 0xFF;

        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)lsav_Byte, i*2, 2);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }
    }

    if(lsv_DataCnt%2) {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_WordData, i, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        lsav_Byte[0] = (lsv_WordData >> 8) & 0xFF;

        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)lsav_Byte, i*2, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }
    }

    return pdPASS;
}

/**
  * @brief  字节单位的数据组合指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_btow_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    short lsv_WordData, lsav_Byte[2], lsv_DataCnt;
    unsigned short i;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    /*获取转换数据个数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsv_DataCnt, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_DataCnt < 0) {
        return ERR_OPERANDS;
    }

    for(i=0; i<lsv_DataCnt/2; i++) {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)lsav_Byte, i*2, 2);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        lsv_WordData = (lsav_Byte[0]&0xFF)<<8 | (lsav_Byte[1]&0xFF);

        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_WordData, i, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }
    }

    if(lsv_DataCnt%2) {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)lsav_Byte, 2*i, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        lsv_WordData = (lsav_Byte[0]&0xFF)<<8;

        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_WordData, i, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }
    }

    return pdPASS;

}

/**
  * @brief  16位数据的低4位组合指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_uni_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short lsv_WordData[4] = {0,}, lsv_DataCnt, lsv_DestData;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    /*取操作数个数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsv_DataCnt, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_DataCnt < 1 || lsv_DataCnt > 4) {
        return  ERR_OPERANDS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)lsv_WordData, 0, lsv_DataCnt);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lsv_DestData = (lsv_WordData[0]&0x0F)<<12 | (lsv_WordData[1]&0x0F)<<8 | (lsv_WordData[2]&0x0F)<<4 | (lsv_WordData[3]&0x0F);

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_DestData, 0, 1);

    return lcv_Ret;
}

/**
  * @brief  16位数据的低4位分离指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dis_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short lsv_WordData, lsv_DataCnt, lsv_DestData[4]= {0,};
    unsigned char lcv_Ret, i;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    /*取操作数个数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsv_DataCnt, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_DataCnt < 1 || lsv_DataCnt > 4) {
        return  ERR_OPERANDS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_WordData, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    for(i=0; i<lsv_DataCnt; i++) {
        lsv_DestData[i] = (lsv_WordData >> (3-i)*4)&0x0F;
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)lsv_DestData, 0, lsv_DataCnt);

    return lcv_Ret;
}

/**
  * @brief  随机数生成指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_rnd_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short lsv_RandomNum;
    unsigned char lcv_Ret;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    /*清除标志位*/
    plc_set_bit_element_value(SM_ELEMENT, 180, 0);
#if 1
    //srand(GET_1MS_TICKS_COUNT());
    lsv_RandomNum = rand();
#else
    /* Get Random data*/
    int32_t status = TRNG_GetRandomData(TRNG, &lsv_RandomNum, 2);
    if (status != kStatus_Success)
    {
        srand(GET_1MS_TICKS_COUNT());
        lsv_RandomNum = rand();
    }
#endif
    if(lsv_RandomNum == 0) {
        plc_set_bit_element_value(SM_ELEMENT, 180, 1);
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_RandomNum, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  线性转换指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_lcnv_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short *lsp_SrcData;
    short lsv_ConvertTable[4];
    short lsv_DataNum;
    short *lsp_DestData;
    unsigned char i;
    unsigned char lcv_Ret;
    float A, B, lfv_Dest;

    if(!GET_PU16_DATA(ltp_RunEnv)) {
        return pdPASS;
    }

    /*取待转化数据数量*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+14, (unsigned short *)&lsv_DataNum, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_DataNum<1 || lsv_DataNum>64) {
        return ERR_OPERANDS;
    }

    /*分配内存*/
    lsp_SrcData = (short *)pvPortMalloc(sizeof(short)*lsv_DataNum);
    configASSERT(lsp_SrcData != NULL);

    lsp_DestData = (short *)pvPortMalloc(sizeof(short)*lsv_DataNum);
    configASSERT(lsp_DestData != NULL);

    /*取待转换数据*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)lsp_SrcData, 0, lsv_DataNum);
    if(lcv_Ret != pdPASS) {
        vPortFree(lsp_SrcData);
        vPortFree(lsp_DestData);
        return lcv_Ret;
    }

    /*取转化表*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)lsv_ConvertTable, 0, 4);
    if(lcv_Ret != pdPASS) {
        vPortFree(lsp_SrcData);
        vPortFree(lsp_DestData);
        return lcv_Ret;
    }

    if(lsv_ConvertTable[0] > lsv_ConvertTable[1] || lsv_ConvertTable[2] > lsv_ConvertTable[3]) {
        vPortFree(lsp_SrcData);
        vPortFree(lsp_DestData);
        return ERR_OPERANDS;
    }

    A = (float)((lsv_ConvertTable[2] - lsv_ConvertTable[3])*10000)/(lsv_ConvertTable[0] - lsv_ConvertTable[1]);
    B = (float)(lsv_ConvertTable[2] - (lsv_ConvertTable[0]*A)/10000);

    for(i=0; i<lsv_DataNum; i++) {
        lfv_Dest =  (lsp_SrcData[i]*A)/10000 + B;

        if(lfv_Dest > 32767)
            lfv_Dest = 32767;
        else if(lfv_Dest < -32768)
            lfv_Dest = -32768;

        lsp_DestData[i] = (short)lfv_Dest;
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsp_DestData, 0, lsv_DataNum);

    vPortFree(lsp_SrcData);
    vPortFree(lsp_DestData);

    return lcv_Ret;
}

/**
  * @brief  浮点数线性转换指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_rlcnv_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    float *lfp_SrcData;
    float lfv_ConvertTable[4];
    short lsv_DataNum;
    float *lfp_DestData;
    unsigned char i;
    unsigned char lcv_Ret;
    float A, B;

    if(!GET_PU16_DATA(ltp_RunEnv)) {
        return pdPASS;
    }

    /*取待转化数据数量*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+20, (unsigned short *)&lsv_DataNum, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_DataNum<1 || lsv_DataNum>64) {
        return ERR_OPERANDS;
    }

    /*分配内存*/
    lfp_SrcData = (float *)pvPortMalloc(sizeof(float)*lsv_DataNum);
    configASSERT(lfp_SrcData != NULL);

    lfp_DestData = (float *)pvPortMalloc(sizeof(float)*lsv_DataNum);
    configASSERT(lfp_DestData != NULL);

    /*取待转换数据*/
    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+2, lfp_SrcData, 0, lsv_DataNum);
    if(lcv_Ret != pdPASS) {
        vPortFree(lfp_SrcData);
        vPortFree(lfp_DestData);
        return lcv_Ret;
    }

    /*取转化表*/
    lcv_Ret = get_float(ltp_RunEnv->mcp_PC+6, lfv_ConvertTable, 0, 4);
    if(lcv_Ret != pdPASS) {
        vPortFree(lfp_SrcData);
        vPortFree(lfp_DestData);
        return lcv_Ret;
    }

    if(lfv_ConvertTable[0] > lfv_ConvertTable[1] || lfv_ConvertTable[2] > lfv_ConvertTable[3]) {
        vPortFree(lfp_SrcData);
        vPortFree(lfp_DestData);
        return ERR_OPERANDS;
    }

    A = (float)((lfv_ConvertTable[2] - lfv_ConvertTable[3])*10000)/(lfv_ConvertTable[0] - lfv_ConvertTable[1]);
    B = (float)(lfv_ConvertTable[2] - (lfv_ConvertTable[0]*A)/10000);

    for(i=0; i<lsv_DataNum; i++) {
        lfp_DestData[i] =  (lfp_SrcData[i]*A)/10000 + B;
    }

    lcv_Ret = save_float(ltp_RunEnv->mcp_PC+14, lfp_DestData, 0, lsv_DataNum);

    vPortFree(lfp_SrcData);
    vPortFree(lfp_DestData);

    return lcv_Ret;
}

/**
  * @brief  交替输出指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_alt_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret, lcv_ElementValue;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_char(ltp_RunEnv->mcp_PC+2, &lcv_ElementValue, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_ElementValue &= 0x01;
    lcv_ElementValue = 1 - lcv_ElementValue;

    lcv_Ret = save_char(ltp_RunEnv->mcp_PC+2, &lcv_ElementValue, 0, 1);
    return lcv_Ret;
}

/*------------------------------------------------------------------------------
*   数据表指令
*-----------------------------------------------------------------------------*/
/**
  * @brief  死区控制指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_dband_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    short lsv_DownLimit, lsv_UpLimit, lsv_Data, lsv_Result;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_DownLimit, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_UpLimit, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_DownLimit > lsv_UpLimit) {
        return ERR_OPERANDS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsv_Data, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_Data < lsv_DownLimit) {
        lsv_Result = lsv_Data - lsv_DownLimit;
    } else if(lsv_Data > lsv_UpLimit) {
        lsv_Result = lsv_Data - lsv_UpLimit;
    } else {
        lsv_Result = 0;
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+14, (unsigned short *)&lsv_Result, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  死区控制指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_limit_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    short lsv_DownLimit, lsv_UpLimit, lsv_Data, lsv_Result;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_DownLimit, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_UpLimit, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_DownLimit > lsv_UpLimit) {
        return ERR_OPERANDS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsv_Data, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_Data < lsv_DownLimit) {
        lsv_Result = lsv_DownLimit;
    } else if(lsv_Data > lsv_UpLimit) {
        lsv_Result = lsv_UpLimit;
    } else {
        lsv_Result = lsv_Data;
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+14, (unsigned short *)&lsv_Result, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  区域控制指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_zone_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    short lsv_DownLimit, lsv_UpLimit, lsv_Data, lsv_Result;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_DownLimit, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_UpLimit, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_DownLimit > lsv_UpLimit) {
        return ERR_OPERANDS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsv_Data, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_Data < 0) {
        lsv_Result = lsv_Data + lsv_DownLimit;
    } else if(lsv_Data > 0) {
        lsv_Result = lsv_Data + lsv_UpLimit;
    } else {
        lsv_Result = 0;
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+14, (unsigned short *)&lsv_Result, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  定坐标指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_scl_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short lsv_InputX, lsv_InputY;
    short lsv_PointNum, *lsp_PointX, *lsp_PointY;
    unsigned char lcv_Ret;
    unsigned short i;
    double lfv_Temp;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    /*取坐标点数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_PointNum, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_PointNum <= 1) {
        return ERR_OPERANDS;
    }

    /*取定坐标输入值*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lsv_InputX, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lsp_PointX = (short *)pvPortMalloc(sizeof(short)*lsv_PointNum);
    configASSERT(lsp_PointX != NULL);

    lsp_PointY = (short *)pvPortMalloc(sizeof(short)*lsv_PointNum);
    configASSERT(lsp_PointY != NULL);

    /*取第一个点坐标*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsp_PointX[0], 1, 1);
    if(lcv_Ret != pdPASS) {
        vPortFree(lsp_PointX);
        vPortFree(lsp_PointY);
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsp_PointY[0], 2, 1);
    if(lcv_Ret != pdPASS) {
        vPortFree(lsp_PointX);
        vPortFree(lsp_PointY);
        return lcv_Ret;
    }

    for(i=1; i<lsv_PointNum; i++) {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsp_PointX[i], i*2+1, 1);
        if(lcv_Ret != pdPASS) {
            vPortFree(lsp_PointX);
            vPortFree(lsp_PointY);
            return lcv_Ret;
        }

        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsp_PointY[i], i*2+2, 1);
        if(lcv_Ret != pdPASS) {
            vPortFree(lsp_PointX);
            vPortFree(lsp_PointY);
            return lcv_Ret;
        }

        if((lsv_InputX >= lsp_PointX[i-1]) && (lsv_InputX <= lsp_PointX[i])) {
            break;
        }
    }

    if(i >= lsv_PointNum) {
        vPortFree(lsp_PointX);
        vPortFree(lsp_PointY);
        return ERR_OPERANDS;
    }

    /*X 坐标相等,直接返回对应Y坐标*/
    if(lsv_InputX == lsp_PointX[i-1]) {
        lsv_InputY = lsp_PointY[i-1];
    } else if(lsv_InputX == lsp_PointX[i]) {
        lsv_InputY = lsp_PointY[i];
    } else {
        /*按比列计算Y值*/
        lfv_Temp = ((double)(lsv_InputX)*(lsp_PointY[i] - lsp_PointY[i-1]))/(lsp_PointX[i] - lsp_PointY[i-1]);
        lsv_InputY = (short)(lfv_Temp + 0.5);
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsv_InputY, 0, 1);

    vPortFree(lsp_PointX);
    vPortFree(lsp_PointY);
    return lcv_Ret;
}

/**
  * @brief  数据检索指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_ser_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    short lsv_DataNum, *lsp_DataBuff, lsv_CmpData;
    /*数组中依次存储检索结果：保存相同数据的个数,初值/终值的位置,最小值,最大值的位置*/
    short lsav_Result[5] = {0,};
    short lsv_Min, lsv_Max;
    unsigned char lcv_Ret;
    unsigned short i;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    /*取被检索数据个数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+14, (unsigned short *)&lsv_DataNum, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_DataNum <1 || lsv_DataNum > 256) {
        return ERR_OPERANDS;
    }

    /*取待检索数据*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_CmpData, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*分配被检索数据缓存*/
    lsp_DataBuff = (short *)pvPortMalloc(sizeof(short)*lsv_DataNum);
    configASSERT(lsp_DataBuff != NULL);

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)lsp_DataBuff, 0, lsv_DataNum);
    if(lcv_Ret != pdPASS) {
        vPortFree(lsp_DataBuff);
        return lcv_Ret;
    }

    /*检索*/
    for(i=0; i<lsv_DataNum; i++) {
        if(lsp_DataBuff[i] == lsv_CmpData) {
            lsav_Result[0]++;

            if(lsav_Result[0] == 1) {
                lsav_Result[1] = i;
                lsv_Min = lsp_DataBuff[i];
                lsv_Max = lsp_DataBuff[i];
            } else {
                lsav_Result[2] = i;
            }
        }

        if(lsp_DataBuff[i] < lsv_Min) {
            lsv_Min = lsp_DataBuff[i];
            lsav_Result[3] = i;
        }

        if(lsp_DataBuff[i] > lsv_Max) {
            lsv_Max = lsp_DataBuff[i];
            lsav_Result[4] = i;
        }
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)lsav_Result, 0, 5);

    vPortFree(lsp_DataBuff);

    return lcv_Ret;
}

/**
  * @brief  产生定时脉冲指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_duty_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    unsigned short lsv_OnTime, lsv_OffTime, lsv_ElementNum;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_OnTime, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, &lsv_OffTime, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lsv_ElementNum = GET_PU16_DATA(ltp_RunEnv->mcp_PC+12);
    if((lsv_ElementNum < DUTY_INS_BASE_ELEMENT) || lsv_ElementNum > (DUTY_INS_BASE_ELEMENT + MAX_DUTY_SUPPORT_NUM -1)) {
        return ERR_OPERANDS;
    }

    lsv_ElementNum -= DUTY_INS_BASE_ELEMENT;

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        gtp_DutyInsInfo->mcv_IsEnable[lsv_ElementNum] = 1;
    }

    if(gtp_DutyInsInfo->mcv_IsEnable[lsv_ElementNum]) {
        if(lsv_OnTime == 0) {
            plc_set_bit_element_value(SM_ELEMENT, lsv_ElementNum+DUTY_INS_BASE_ELEMENT, 0);
            return pdPASS;
        }

        if(lsv_OffTime == 0) {
            plc_set_bit_element_value(SM_ELEMENT, lsv_ElementNum+DUTY_INS_BASE_ELEMENT, 1);
            return pdPASS;
        }

        if(gtp_DutyInsInfo->msv_OnTime[lsv_ElementNum] < lsv_OnTime) {
            plc_set_bit_element_value(SM_ELEMENT, lsv_ElementNum+DUTY_INS_BASE_ELEMENT, 1);
            gtp_DutyInsInfo->msv_OnTime[lsv_ElementNum] ++;
        } else {
            plc_set_bit_element_value(SM_ELEMENT, lsv_ElementNum+DUTY_INS_BASE_ELEMENT, 0);
            gtp_DutyInsInfo->msv_OffTime[lsv_ElementNum] ++;
        }

        /*设置对应SD元件*/
        SET_SD_ELEMENT_VALUE(lsv_ElementNum+DUTY_INS_BASE_ELEMENT, (gtp_DutyInsInfo->msv_OnTime[lsv_ElementNum] + gtp_DutyInsInfo->msv_OffTime[lsv_ElementNum]));

        if(GET_SD_ELEMENT_VALUE(lsv_ElementNum+DUTY_INS_BASE_ELEMENT) >= (lsv_OnTime + lsv_OffTime)) {
            gtp_DutyInsInfo->msv_OnTime[lsv_ElementNum] = 0;
            gtp_DutyInsInfo->msv_OffTime[lsv_ElementNum] = 0;
            SET_SD_ELEMENT_VALUE(lsv_ElementNum+DUTY_INS_BASE_ELEMENT, 0);
        }
    }

    return pdPASS;
}



unsigned char run_ci_ramp_ins(plc_run_power_flow_st *ltp_RunEnv)
{
/*
    unsigned char *pc_ucode;
    short da1,da2,da,step,step_leav;
    int delt,temp,val_leav;
    unsigned char s,sf;
    short sn;
    pc_ucode =pEnv->pc;
    sn=*(short*)(pc_ucode+22);
    if( _yMarcoGetPf(pEnv) ==0)
    {
        g_wave_rec.g_pf[sn]=0;  //save pf
        sf=0;
        s=_ySaveChar(pc_ucode+18,&sf,0,1);  //save output flag
        if(s)
        {
            return s;
        }
        return yRIGHT;
    }
    s=_yGetWord(pc_ucode+2,(unsigned short *)&da1,0,1);
    if(s)
    {
        return s;
    }
    s=_yGetWord(pc_ucode+6,(unsigned short *)&da2,0,1);
    if(s)
    {
        return s;
    }
    if(da1==da2)
    {
        da=da2;
        sf=1;
        s=_ySaveWord(pc_ucode+10,(unsigned short *)&da,0,1);    //save output data
        if(s)
        {
            return s;
        }
        s=_ySaveChar(pc_ucode+18,&sf,0,1);  //save output flag
        if(s)
        {
            return s;
        }
        return yRIGHT;
    }
    if(g_wave_rec.g_pf[sn]==0)
    {
        //up edge
        g_wave_rec.g_pf[sn]=1;  //save pf
        g_wave_rec.g_step[sn]=0;        //save step
        da=da1; //initial output data
        sf=0;
        s=_ySaveWord(pc_ucode+10,(unsigned short *)&da,0,1);    //save output data
        if(s)
        {
            return s;
        }
        s=_ySaveChar(pc_ucode+18,&sf,0,1);  //save output flag
        if(s)
        {
            return s;
        }
        return yRIGHT;
    }
    
    // pf valid & not up edge dea
    s=_yGetWord(pc_ucode+10,(unsigned short *)&da,0,1);
    if(s)
    {
        return s;
    }
    if(da==da2)
    {
        return yRIGHT;
    }
    s=_yGetWord(pc_ucode+14,(unsigned short *)&step,0,1);
    if(s)
    {
        return s;
    }
    if(step<=0)
    {
        return ERR_OPERAND_VAL;
    }
    if(g_wave_rec.g_step[sn]<0)
    {
        return ERR_USR_INST;
    }
    if(g_wave_rec.g_step[sn]<32767)
    {
        g_wave_rec.g_step[sn]++;
    }
    if(g_wave_rec.g_step[sn]>=step)
    {
        da=da2;
        sf=1;
    }
    else
    {
        val_leav=da2;
        val_leav=val_leav-da;
        step_leav=step-g_wave_rec.g_step[sn];
        delt=val_leav/step_leav;
        temp=val_leav%step_leav;
        if(temp!=0)
        {
            delt=delt+(temp<<1)/step_leav;
        }
        da=delt+da;
        if( (da2>da1)&&(da>da2) )
        {
            da=da2;
        }
        if( (da2<da1)&&(da<da2) )
        {
            da=da2;
        }
        if(da==da2)
        {
            sf=1;
        }
        else
        {
            sf=0;
        }
    }
    s=_ySaveWord(pc_ucode+10,(unsigned short *)&da,0,1);        //save output data
    if(s)
    {
        return s;
    }
    s=_ySaveChar(pc_ucode+18,&sf,0,1);  //save output falg
    if(s)
    {
        return s;
    }
    */

    return pdPASS;
}



unsigned char run_ci_hackle_ins(plc_run_power_flow_st *ltp_RunEnv)
{

/*
    unsigned char *pc_ucode;
    short da1,da2,da,step,step_leav;
    int delt,temp,val_leav;
    unsigned char s,sf;
    short sn;
    pc_ucode =pEnv->pc;
    sn=*(short*)(pc_ucode+22);
    if( _yMarcoGetPf(pEnv) ==0)
    {
        g_wave_rec.g_pf[sn]=0;  //save pf
        sf=0;
        s=_ySaveChar(pc_ucode+18,&sf,0,1);  //save output flag
        if(s)
        {
            return s;
        }
        return yRIGHT;
    }
    s=_yGetWord(pc_ucode+2,(unsigned short *)&da1,0,1);
    if(s)
    {
        return s;
    }
    s=_yGetWord(pc_ucode+6,(unsigned short *)&da2,0,1);
    if(s)
    {
        return s;
    }
    if(da1==da2)
    {
        da=da2;
        sf=1;
        s=_ySaveWord(pc_ucode+10,(unsigned short *)&da,0,1);    //save output data
        if(s)
        {
            return s;
        }
        s=_ySaveChar(pc_ucode+18,&sf,0,1);  //save output flag
        if(s)
        {
            return s;
        }
        return yRIGHT;
    }
    if(g_wave_rec.g_pf[sn]==0)
    {
        //up edge
        g_wave_rec.g_pf[sn]=1;  //save pf
        g_wave_rec.g_step[sn]=0;        //save step
        da=da1; //initial output data
        sf=0;
        s=_ySaveWord(pc_ucode+10,(unsigned short *)&da,0,1);    //save output data
        if(s)
        {
            return s;
        }
        s=_ySaveChar(pc_ucode+18,&sf,0,1);  //save output flag
        if(s)
        {
            return s;
        }
        return yRIGHT;
    }
    
    // pf valid & not up edge deal
    s=_yGetWord(pc_ucode+14,(unsigned short *)&step,0,1);
    if(s)
    {
        return s;
    }
    if(step<=0)
    {
        return ERR_OPERAND_VAL;
    }
    if(g_wave_rec.g_step[sn]<0)
    {
        return ERR_USR_INST;
    }
    if(g_wave_rec.g_step[sn]<32767)
    {
        g_wave_rec.g_step[sn]++;
    }
    s=_yGetWord(pc_ucode+10,(unsigned short *)&da,0,1);
    if(s)
    {
        return s;
    }
    if(da==da2)
    {
        da=da1;
        sf=0;
        g_wave_rec.g_step[sn]=0;
    }
    else if(g_wave_rec.g_step[sn]>=step)
    {
        da=da2;
        sf=1;
    }
    else
    {
        val_leav=da2;
        val_leav=val_leav-da;
        step_leav=step-g_wave_rec.g_step[sn];
        delt=val_leav/step_leav;
        temp=val_leav%step_leav;
        if(temp!=0)
        {
            delt=delt+(temp<<1)/step_leav;
        }
        da=delt+da;
        if( (da2>da1)&&(da>da2) )
        {
            da=da2;
        }
        if( (da2<da1)&&(da<da2) )
        {
            da=da2;
        }
        if(da==da2)
        {
            sf=1;
        }
        else
        {
            sf=0;
        }
    }
    s=_ySaveWord(pc_ucode+10,(unsigned short *)&da,0,1);        //save output data
    if(s)
    {
        return s;
    }
    s=_ySaveChar(pc_ucode+18,&sf,0,1);  //save output falg
    if(s)
    {
        return s;
    }
    */



    return pdPASS;
}
	

	
unsigned char run_ci_triangle_ins(plc_run_power_flow_st *ltp_RunEnv)
{
/*
    unsigned char *pc_ucode;
    short da1,da2,da,step,step_leav;
    int delt,temp,val_leav;
    unsigned char s,sf=0;
    short sn;
    pc_ucode =pEnv->pc;
    sn=*(short*)(pc_ucode+22);
    if( _yMarcoGetPf(pEnv) ==0)
    {
        g_wave_rec.g_pf[sn]=0;  //save pf
        sf=0;
        s=_ySaveChar(pc_ucode+18,&sf,0,1);  //save output flag
        if(s)
        {
            return s;
        }
        return yRIGHT;
    }
    s=_yGetWord(pc_ucode+2,(unsigned short *)&da1,0,1);
    if(s)
    {
        return s;
    }
    s=_yGetWord(pc_ucode+6,(unsigned short *)&da2,0,1);
    if(s)
    {
        return s;
    }
    if(da1==da2)
    {
        da=da2;
        sf=1;
        s=_ySaveWord(pc_ucode+10,(unsigned short *)&da,0,1);    //save output data
        if(s)
        {
            return s;
        }
        s=_ySaveChar(pc_ucode+18,&sf,0,1);  //save output flag
        if(s)
        {
            return s;
        }
        return yRIGHT;
    }
    if(g_wave_rec.g_pf[sn]==0)
    {
        //up edge
        g_wave_rec.g_pf[sn]=1;  //save pf
        g_wave_rec.g_step[sn]=0;        //save step
        da=da1; //initial output data
        sf=0;
        s=_ySaveWord(pc_ucode+10,(unsigned short *)&da,0,1);    //save output data
        if(s)
        {
            return s;
        }
        s=_ySaveChar(pc_ucode+18,&sf,0,1);  //save output flag
        if(s)
        {
            return s;
        }
        return yRIGHT;
    }
    // pf valid & not up edge deal
    s=_yGetWord(pc_ucode+14,(unsigned short *)&step,0,1);
    if(s)
    {
        return s;
    }
    if(step<=0)
    {
        return ERR_OPERAND_VAL;
    }
    if(g_wave_rec.g_step[sn]<0)
    {
        return ERR_USR_INST;
    }
    if(g_wave_rec.g_step[sn]<32767)
    {
        g_wave_rec.g_step[sn]++;
    }
    s=_yGetWord(pc_ucode+10,(unsigned short *)&da,0,1);
    if(s)
    {
        return s;
    }
    if(g_wave_rec.g_pf[sn]==0x11)   //retral ramp
    {
        if(g_wave_rec.g_step[sn]>=step)
        {
            da=da1;
        }
        else
        {
            val_leav=da1;
            val_leav=val_leav-da;
            step_leav=step-g_wave_rec.g_step[sn];
            delt=val_leav/step_leav;
            temp=val_leav%step_leav;
            if(temp!=0)
            {
                delt=delt+(temp<<1)/step_leav;
            }
            da=delt+da;
            if( (da2>da1)&&(da<da1) )
            {
                da=da1;
            }
            if( (da2<da1)&&(da>da1) )
            {
                da=da1;
            }
        }
    }
    else    //frontal ramp
    {
        if(g_wave_rec.g_step[sn]>=step)
        {
            da=da2;
        }
        else
        {
            val_leav=da2;
            val_leav=val_leav-da;
            step_leav=step-g_wave_rec.g_step[sn];
            delt=val_leav/step_leav;
            temp=val_leav%step_leav;
            if(temp!=0)
            {
                delt=delt+(temp<<1)/step_leav;
            }
            da=delt+da;
            if( (da2>da1)&&(da>da2) )
            {
                da=da2;
            }
            if( (da2<da1)&&(da<da2) )
            {
                da=da2;
            }
        }
    }
    if(da==da2) //reached aim poshort
    {
        g_wave_rec.g_pf[sn]=0x11;   //save pf & ramp direction
        g_wave_rec.g_step[sn]=0;
    }
    if(da==da1)
    {
        g_wave_rec.g_pf[sn]=0x01;   //save pf & ramp direction
        sf=1;
        g_wave_rec.g_step[sn]=0;
    }
    s=_ySaveWord(pc_ucode+10,(unsigned short *)&da,0,1);        //save output data
    if(s)
    {
        return s;
    }
    s=_ySaveChar(pc_ucode+18,&sf,0,1);  //save output falg
    if(s)
    {
        return s;
    }

    */
    return pdPASS;
}



