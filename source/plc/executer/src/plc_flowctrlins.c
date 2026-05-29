/**
  ******************************************************************************
  * @file    plc_flowctrlins.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   程序流控制指令
  ******************************************************************************
  */
#include "plc_variable.h"
#include "plc_executer.h"
#include "plc_instruction.h"
#include "plc_flowctrlins.h"
#include "plc_errormsg.h"
#include "plc_commonfunc.h"
#include "plc_parseaddr.h"
#include "plc_element.h"
#include "plc_counterins.h"
#include "plc_timeins.h"
#include "plc_dataprocessing.h"
#include "plc_interrupt.h"
#include "bsp_iwdg.h"

/**
  * @brief  循环指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_for_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    plc_for_next_ins_st *ltp_ForNextIns;
    unsigned char lcv_Ret;
    unsigned short lsv_LoopCnt;
    unsigned char *lcp_NextInsUcode;
    unsigned long llv_InsLen;

    /*取当前函数for结构信息*/
    ltp_ForNextIns = &gtp_ForNextIns[gtp_CallInsInfoPtr->msv_SbrNestedNum];

    /*取循环参数*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_LoopCnt, 0, 1);
    if(lcv_Ret != pdPASS) {
        ltp_RunEnv->mcp_PC += 10;
        return lcv_Ret;
    }

    /*获取指令长度*/
    llv_InsLen = GET_PU16_DATA(ltp_RunEnv->mcp_PC+6) + (GET_PU16_DATA(ltp_RunEnv->mcp_PC+8)<<16);
    lcp_NextInsUcode = ltp_RunEnv->mcp_PC + llv_InsLen;

    ltp_ForNextIns->msp_StartUcodeAddr[ltp_ForNextIns->msv_NestedNum] = ltp_RunEnv->mcp_PC;
    ltp_ForNextIns->msp_EndUcodeAddr[ltp_ForNextIns->msv_NestedNum] = lcp_NextInsUcode;

    if(GET_POWER_FLOW(ltp_RunEnv) && (lsv_LoopCnt > 0)) {
        ltp_RunEnv->mcp_PC += 10;
        ltp_ForNextIns->msa_LoopCnt[ltp_ForNextIns->msv_NestedNum] = lsv_LoopCnt;

        /*循环嵌套值加一*/
        ltp_ForNextIns->msv_NestedNum ++;

    } else {
        ltp_RunEnv->mcp_PC = lcp_NextInsUcode;
    }

    return pdPASS;
}

/**
  * @brief  循环指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_next_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    plc_for_next_ins_st *ltp_ForNextIns;

    /*取当前函数for结构信息*/
    ltp_ForNextIns = &gtp_ForNextIns[gtp_CallInsInfoPtr->msv_SbrNestedNum];

    /*msv_NestedNum > 0,表示For-Next循环已经执行，否则直接退出*/

    if(ltp_ForNextIns->msv_NestedNum > 0) {

        switch(ltp_ForNextIns->msa_LoopCnt[ltp_ForNextIns->msv_NestedNum -1]) {
            case 0:
                ltp_ForNextIns->msv_NestedNum--;
                if(ltp_ForNextIns->msv_NestedNum != 0) {
                    switch(ltp_ForNextIns->msa_LoopCnt[ltp_ForNextIns->msv_NestedNum -1]) {
                        case 1:
                            ltp_ForNextIns->msa_LoopCnt[ltp_ForNextIns->msv_NestedNum -1] = 0;
                            ltp_RunEnv->mcp_PC += 2;
                            break;
                        default:
                            ltp_ForNextIns->msa_LoopCnt[ltp_ForNextIns->msv_NestedNum -1]--;
                            ltp_RunEnv->mcp_PC = ltp_ForNextIns->msp_StartUcodeAddr[ltp_ForNextIns->msv_NestedNum -1] + 10;
                            break;
                    }
                } else {
                    ltp_RunEnv->mcp_PC += 2;
                }
                break;

            case 1:
                ltp_ForNextIns->msa_LoopCnt[ltp_ForNextIns->msv_NestedNum -1] = 0;
                ltp_RunEnv->mcp_PC += 2;
                break;
            default:
                ltp_ForNextIns->msa_LoopCnt[ltp_ForNextIns->msv_NestedNum -1]--;
                ltp_RunEnv->mcp_PC = ltp_ForNextIns->msp_StartUcodeAddr[ltp_ForNextIns->msv_NestedNum -1] + 10;
                break;
        }

    } else {
        ltp_RunEnv->mcp_PC += 2;
    }

    return pdPASS;
}

unsigned char run_ci_continue_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    return pdPASS;
}

unsigned char run_ci_break_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    return pdPASS;
}


/**
  * @brief  跳转标号定义指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_lbl_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    return pdPASS;
}

/**
  * @brief  跳转指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_cj_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char *lcp_DestAddr, *lcp_InsStartAddr;
    long i;
    plc_for_next_ins_st *ltp_ForNextIns;



    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        ltp_RunEnv->mcp_PC += 10;
        return pdPASS;
    }

    lcp_InsStartAddr = ltp_RunEnv->mcp_PC;
    /*跳转目标*/
    ltp_RunEnv->mcp_PC += GET_PS32_DATA(ltp_RunEnv->mcp_PC+6);

    lcp_DestAddr = ltp_RunEnv->mcp_PC;

    /*取当前函数for结构信息*/
    ltp_ForNextIns = &gtp_ForNextIns[gtp_CallInsInfoPtr->msv_SbrNestedNum];

    /*判断是否跳出FOR-NEXT结构*/
    for(i=ltp_ForNextIns->msv_NestedNum-1; i>=0; i--) {
        if((GET_POINT_ADDR(lcp_InsStartAddr) > GET_POINT_ADDR(ltp_ForNextIns->msp_StartUcodeAddr[i]))
           && (GET_POINT_ADDR(lcp_InsStartAddr) < GET_POINT_ADDR(ltp_ForNextIns->msp_EndUcodeAddr[i]))) {
            /*跳转指令在FOR-NEXT结构中*/
            if((GET_POINT_ADDR(lcp_DestAddr) < GET_POINT_ADDR(ltp_ForNextIns->msp_StartUcodeAddr[i]))
               || (GET_POINT_ADDR(lcp_DestAddr) > GET_POINT_ADDR(ltp_ForNextIns->msp_EndUcodeAddr[i]))) {
                /*跳出FOR-NEXT结构, FOR-NEXT嵌套层数减一*/
                ltp_ForNextIns->msv_NestedNum --;
            } else {
                break;
            }
        } else {
            break;
        }
    }

    return pdPASS;
}

/**
  * @brief  主程序返回指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_fend_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    return EXEC_FLAG_FEND;
}

/**
  * @brief  主程序条件返回指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_cfend_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    if(GET_POWER_FLOW(ltp_RunEnv)) {
        return EXEC_FLAG_FEND;
    }
    return pdPASS;
}

/**
  * @brief  看门狗指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_wdt_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    if(GET_POWER_FLOW(ltp_RunEnv)) {
        gtv_PlcRunStatus.mlv_WatchDogTime = GET_1MS_TICKS_COUNT();
        bsp_feed_watch_dog();
    }

    return pdPASS;
}

/**
  * @brief  中断使能指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_ei_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    if(GET_POWER_FLOW(ltp_RunEnv)) {
        plc_user_interrupt_enable(1);
    }
    return pdPASS;
}

/**
  * @brief  中断禁止指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_di_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    if(GET_POWER_FLOW(ltp_RunEnv)) {
        plc_user_interrupt_enable(0);
    }
    return pdPASS;
}

/**
  * @brief  用户中断返回指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_iret_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    return EXEC_FLAG_IRET;
}

/**
  * @brief  用户中断程序条件返回指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_ciret_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    if(GET_POWER_FLOW(ltp_RunEnv)) {
        return EXEC_FLAG_IRET;
    }
    return pdPASS;
}

/**
  * @brief  用户程序停止指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_stop_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    if(GET_POWER_FLOW(ltp_RunEnv)) {
        gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.cmd_stop = 1;
        /*20170726: 需要增加停止输出函数...*/

        return EXEC_FLAG_STOP;
    }
    return pdPASS;
}

/**
  * @brief  计算CALL指令长度
  * @param  None
  * @retval None
  */
unsigned short plc_call_instruction_length(unsigned char *lcp_CallInsPtr)
{
    return (10 + GET_PU16_DATA(lcp_CallInsPtr +4) + GET_PU16_DATA(lcp_CallInsPtr +6)
            + GET_PU16_DATA(lcp_CallInsPtr +8));
}

/**
  * @brief  获取CALL指令调用ID
  * @param  None
  * @retval None
  */
unsigned char plc_get_call_ins_id(unsigned char  *lcp_Ucode, unsigned short *lsp_CallId)
{
    unsigned short i;

    for(i=0; i<gtp_CallInsInfoPtr->msv_UseNum; i++) {
        if(gtp_CallInsInfoPtr->mlv_UCodeAddr[i] == GET_POINT_ADDR(lcp_Ucode)) {
            *lsp_CallId = i;
            return pdPASS;
        }
    }

    return ERR_NO_DEFINE_SBR_INTR;
}

/**
  * @brief  CALL指令In参数传递
  * @param  lcp_BitCnt: IN类型参数中LM元件参数个数
  *         lcp_WordCnt: IN 类型参数中V元件参数个数
  * @retval None
  */
unsigned char plc_call_ins_in_parameter_parse(unsigned char *lcp_BitCnt, unsigned char *lcp_WordCnt, unsigned char *lcp_CallInsPtr)
{
    unsigned short lsv_InCnt, lsv_InOutCnt, lsv_TempCnt;
    unsigned char *lcp_ParaPtr;
    unsigned char lcv_BitCnt=0, lcv_WordCnt=0;
    unsigned char lcv_Ret, lcv_Temp;
    unsigned short i;
    unsigned short lsv_ParaType;

    /*清除LM & LV参数区域*/
    for(i=0; i<LM_RANG; i++) {
        plc_set_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum+1, i, 0);
        SET_V_ELEMENT_VALUE((gtp_CallInsInfoPtr->msv_SbrNestedNum+1), i, 0);
    }

    /*IN类型参数长度*/
    lsv_InCnt = GET_PS16_DATA(lcp_CallInsPtr + 4);

    /*IN OUT 类型参数长度, 包含IN*/
    lsv_InOutCnt = lsv_InCnt + GET_PS16_DATA(lcp_CallInsPtr + 6);

    lsv_TempCnt = 0;
    /*指向In参数开始地址*/
    lcp_ParaPtr = lcp_CallInsPtr + 10;

    while(lsv_TempCnt < lsv_InOutCnt) {
        lsv_ParaType = GET_PU16_DATA(lcp_ParaPtr);

        switch(lsv_ParaType) {
            case PARA_BOOL:
                lcv_Ret = get_char(lcp_ParaPtr+2, &lcv_Temp, 0, 1);
                plc_set_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum+1, lcv_BitCnt, lcv_Temp);
                lcp_ParaPtr += 6;
                lsv_TempCnt += 6;
                lcv_BitCnt ++;
                break;

            case PARA_WORD:
            case PARA_INT:

                lcv_Ret = get_word(lcp_ParaPtr+2,GET_V_ELEMENT_ADDR((gtp_CallInsInfoPtr->msv_SbrNestedNum+1), lcv_WordCnt), 0, 1);
                lcp_ParaPtr += 6;
                lsv_TempCnt += 6;
                lcv_WordCnt ++;
                break;

            case PARA_DWORD:
            case PARA_DINT:
                lcv_Ret = get_dword(lcp_ParaPtr+2,(unsigned long *)GET_V_ELEMENT_ADDR((gtp_CallInsInfoPtr->msv_SbrNestedNum+1), lcv_WordCnt), 0, 1);
                lcp_ParaPtr += 8;
                lsv_TempCnt += 8;
                lcv_WordCnt += 2;
                break;

            case PARA_REAL:
                lcv_Ret = get_float(lcp_ParaPtr+2,(float *)GET_V_ELEMENT_ADDR((gtp_CallInsInfoPtr->msv_SbrNestedNum+1), lcv_WordCnt), 0, 1);
                lcp_ParaPtr += 8;
                lsv_TempCnt += 8;
                lcv_WordCnt += 2;
                break;
            default:
                lcv_Ret = ERR_OPERANDS;
        }

        if(lsv_TempCnt == lsv_InCnt) {
            *lcp_BitCnt = lcv_BitCnt;
            *lcp_WordCnt = lcv_WordCnt;
        }

        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }
    }

    return pdPASS;
}

/**
  * @brief  CALL指令OUT参数传递
  * @param  lcv_BitCnt: OUT类型参数LM元件起始位置
  *         lcv_WordCnt: OUT类型参数V元件起始位置
  * @retval None
  */
unsigned char plc_call_ins_out_parameter_parse(unsigned char lcv_BitCnt, unsigned char lcv_WordCnt, unsigned char *lcp_CallInsPtr)
{
    unsigned short lsv_InCnt, lsv_OutCnt, lsv_TempCnt;
    unsigned char *lcp_ParaPtr;
    unsigned char lcv_Ret, lcv_Temp;
    unsigned short lsv_ParaType;

    /*IN类型参数长度*/
    lsv_InCnt = GET_PS16_DATA(lcp_CallInsPtr + 4);

    /*OUT 类型参数长度, 包含IN-OUT, OUT*/
    lsv_OutCnt = GET_PS16_DATA(lcp_CallInsPtr + 6) + GET_PS16_DATA(lcp_CallInsPtr + 8);

    lsv_TempCnt = 0;
    /*指向IN-OUT参数开始地址*/
    lcp_ParaPtr = lcp_CallInsPtr + 10 + lsv_InCnt;

    while(lsv_TempCnt < lsv_OutCnt) {
        lsv_ParaType = GET_PU16_DATA(lcp_ParaPtr);

        switch(lsv_ParaType) {
            case PARA_BOOL:
                lcv_Temp = plc_get_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum+1, lcv_BitCnt);
                lcv_Ret = save_char(lcp_ParaPtr+2, &lcv_Temp, 0, 1);
                lcp_ParaPtr += 6;
                lsv_TempCnt += 6;
                lcv_BitCnt ++;
                break;

            case PARA_WORD:
            case PARA_INT:
                lcv_Ret = save_word(lcp_ParaPtr+2,GET_V_ELEMENT_ADDR((gtp_CallInsInfoPtr->msv_SbrNestedNum+1), lcv_WordCnt), 0, 1);
                lcp_ParaPtr += 6;
                lsv_TempCnt += 6;
                lcv_WordCnt ++;
                break;

            case PARA_DWORD:
            case PARA_DINT:
                lcv_Ret = save_dword(lcp_ParaPtr+2,(unsigned long *)GET_V_ELEMENT_ADDR((gtp_CallInsInfoPtr->msv_SbrNestedNum+1), lcv_WordCnt), 0, 1);
                lcp_ParaPtr += 8;
                lsv_TempCnt += 8;
                lcv_WordCnt += 2;
                break;

            case PARA_REAL:
                lcv_Ret = save_float(lcp_ParaPtr+2,(float *)GET_V_ELEMENT_ADDR((gtp_CallInsInfoPtr->msv_SbrNestedNum+1), lcv_WordCnt), 0, 1);
                lcp_ParaPtr += 8;
                lsv_TempCnt += 8;
                lcv_WordCnt += 2;
                break;
            default:
                lcv_Ret = ERR_OPERANDS;
        }

        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }
    }

    return pdPASS;
}

/**
  * @brief  CALL指令在能流无效时,清除子程序中指定指令输出
  * @param  None
  * @retval None
  */
unsigned char plc_call_ins_clean_element(unsigned short lsv_SbrId)
{
    struct list_head *ltp_ForCur;
    struct list_head *ltp_ForNext;
    struct list_head *ltp_Head;
    plc_ins_list_st *ltp_PlcIns;
    unsigned char *lcp_Ins;
    unsigned short lsv_Temp;
    plc_run_power_flow_st ltv_RunEnv;
    unsigned short lsv_SbrNum, lsv_CallId;
    unsigned char lcv_Ret;

    if(lsv_SbrId > MAX_SBR_COUNT) {
        return ERR_OVER_SBR_STACK;
    }

    /*搜索清除简单指令*/
    ltp_Head = &gtp_SbrListPtr[lsv_SbrId].mtv_SimpInsHead;
    list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
        ltp_PlcIns = list_entry(ltp_ForCur, plc_ins_list_st, mtv_InsList);
        lcp_Ins = (unsigned char *)ltp_PlcIns->mlp_InsPc;
        switch(*(lcp_Ins +1)) {
            case SI_EU:
            case SI_ED:
                lsv_Temp = GET_PU16_DATA(lcp_Ins+2);
                RST_EDGE_VALUE(lsv_Temp);
                RST_EDGE_INIT_FLAG(lsv_Temp);
                break;
            case SI_LDP:
            case SI_LDF:
            case SI_ANDP:
            case SI_ANDF:
            case SI_ORP:
            case SI_ORF:
            case SI_PLP:
            case SI_PLF:
                lsv_Temp = GET_PU16_DATA(lcp_Ins+4);
                RST_EDGE_VALUE(lsv_Temp);
                RST_EDGE_INIT_FLAG(lsv_Temp);
                break;

        }
    }

    /*搜索清除复杂指令*/
    ltv_RunEnv.mlv_InPF = 0;

    ltp_Head = &gtp_SbrListPtr[lsv_SbrId].mtv_CompInsHead;
    list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
        ltp_PlcIns = list_entry(ltp_ForCur, plc_ins_list_st, mtv_InsList);
        lcp_Ins = (unsigned char *)ltp_PlcIns->mlp_InsPc;

        ltv_RunEnv.mcp_PC = lcp_Ins;

        switch(GET_PU16_DATA(lcp_Ins)) {
            case CI_TON:
                lsv_Temp = GET_PU16_DATA(lcp_Ins+4);
                plc_reset_one_timer(lsv_Temp);
                break;

            case CI_TONR:
                lsv_Temp = GET_PU16_DATA(lcp_Ins+4);
                gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_Temp].mbv_Enable = 0;
                break;

            case CI_TOF:
                lsv_Temp = GET_PU16_DATA(lcp_Ins+4);
                plc_reset_one_timer(lsv_Temp);
                break;

            case CI_OUT_LM:
                lsv_Temp = GET_PU16_DATA(lcp_Ins+4);
                plc_set_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_Temp, 0);
                break;

            case CI_BOUT:
                run_ci_bout_ins(&ltv_RunEnv);
                break;

            case CI_CALL:
                /*子程序中调用子程序,获取调用的子程序编号,CALL指令编号*/
                lsv_SbrNum = GET_PU16_DATA(lcp_Ins+2);
                lsv_SbrNum &= 0xFF;

                lcv_Ret = plc_get_call_ins_id(lcp_Ins, &lsv_CallId);
                if(lcv_Ret != pdPASS) {
                    return lcv_Ret;
                }

                if(gtp_CallInsInfoPtr->msv_SbrNestedNum >= MAX_SBR_NESTED_LAYER) {
                    return ERR_OVER_SBR_STACK;
                }

                if(gtp_CallInsInfoPtr->mcv_IsExec[lsv_CallId]) {
                    gtp_CallInsInfoPtr->mcv_IsExec[lsv_CallId] = 0;

                    gtp_CallInsInfoPtr->msv_SbrNestedNum ++;
                    lcv_Ret = plc_call_ins_clean_element(lsv_SbrNum);
                    gtp_CallInsInfoPtr->msv_SbrNestedNum--;

                    if(lcv_Ret != pdPASS) {
                        return lcv_Ret;
                    }
                }
                break;

            /*当前不支持指令,直接跳过*/
            case CI_DRVI:
            case CI_DRVC:
            case CI_DRVA:
            case CI_ZRN:
            case CI_PLS:
            case CI_PLSB:
            case CI_PWM:
            case CI_PLSY:
            case CI_PLSR:
            case CI_PLSV:
            case CI_DHSCS:
            case CI_HCNT:
            case CI_SPD:
            case CI_GEARBOX:
            case CI_LIN:
            case CI_CW:
            case CI_CCW:
            case CI_CAMBOX:
                break;
        }
    }

    return pdPASS;
}

/**
  * @brief  用户子程序调用指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_call_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    unsigned short lsv_CallId;
    unsigned short lsv_CallInsLen;
    unsigned short lsv_SbrId;
    unsigned char lcv_BitCnt, lcv_WordCnt;

    lcv_Ret = plc_get_call_ins_id(ltp_RunEnv->mcp_PC, &lsv_CallId);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lsv_CallInsLen = plc_call_instruction_length(ltp_RunEnv->mcp_PC);

    if(gtp_CallInsInfoPtr->msv_SbrNestedNum >= MAX_SBR_NESTED_LAYER) {
        ltp_RunEnv->mcp_PC += lsv_CallInsLen;
        return ERR_OVER_SBR_STACK;
    }

    /*获取调用的子程序编号*/
    lsv_SbrId = GET_PU16_DATA(ltp_RunEnv->mcp_PC+2);
    lsv_SbrId &= 0xFF;

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        /*设置程序运行标志*/
        gtp_CallInsInfoPtr->mcv_IsExec[lsv_CallId] = 1;

        /*判断所调用的子程序是否合法*/
        if(GET_PU16_DATA(gtp_CallInsInfoPtr->mcp_SbrPc[lsv_SbrId]) != CI_SBR) {
            ltp_RunEnv->mcp_PC += lsv_CallInsLen;
            return ERR_NO_DEFINE_SBR_INTR;
        }

        /*IN & IN-OUT 参数装载*/
        lcv_BitCnt = 0;
        lcv_WordCnt = 0;

        lcv_Ret = plc_call_ins_in_parameter_parse(&lcv_BitCnt, &lcv_WordCnt, ltp_RunEnv->mcp_PC);
        if(lcv_Ret != pdPASS) {
            ltp_RunEnv->mcp_PC += lsv_CallInsLen;
            return lcv_Ret;
        }

        /*保留CALL指令入口地址, 调用层数加一*/
        gtp_CallInsInfoPtr->mcp_RetPc[gtp_CallInsInfoPtr->msv_SbrNestedNum] = ltp_RunEnv->mcp_PC;
        gtp_CallInsInfoPtr->msv_SbrNestedNum++;

        /*执行子函数*/
        lcv_Ret = plc_run_user_program(gtp_CallInsInfoPtr->mcp_SbrPc[lsv_SbrId], 1);
        if(lcv_Ret != EXEC_FLAG_SRET) {
            return lcv_Ret;
        }

        gtp_CallInsInfoPtr->msv_SbrNestedNum--;

        /*OUT参数处理*/
        lcv_Ret = plc_call_ins_out_parameter_parse(lcv_BitCnt, lcv_WordCnt, ltp_RunEnv->mcp_PC);
        if(lcv_Ret != pdPASS) {
            ltp_RunEnv->mcp_PC += lsv_CallInsLen;
            return lcv_Ret;
        }

        ltp_RunEnv->mcp_PC += lsv_CallInsLen;

    } else if(gtp_CallInsInfoPtr->mcv_IsExec[lsv_CallId]) {
        /*已执行过,能流无效,清除特殊指令*/
        gtp_CallInsInfoPtr->mcv_IsExec[lsv_CallId] = 0;
        ltp_RunEnv->mcp_PC += lsv_CallInsLen;

        lcv_Ret = plc_call_ins_clean_element(lsv_SbrId);
        if(lcv_Ret != pdPASS) {
            return ERR_NO_DEFINE_SBR_INTR;
        }
    } else {
        ltp_RunEnv->mcp_PC += lsv_CallInsLen;
    }

    return pdPASS;
}

/**
  * @brief  用户子程序条件返回指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_csret_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    if(GET_POWER_FLOW(ltp_RunEnv)) {
        return EXEC_FLAG_SRET;
    } else {
        ltp_RunEnv->mcp_PC += 2;
    }

    return pdPASS;
}

/**
  * @brief  用户子程序条件返回指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_sret_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    return EXEC_FLAG_SRET;
}

/**
  * @brief  用户子程序定义指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_sbr_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    return pdPASS;
}

/*------------------------------------------------------------------------------
*   主控指令
*-----------------------------------------------------------------------------*/

/**
  * @brief  主控指令在能流无效时,清除指令中指定指令输出
  * @param  None
  * @retval None
  */
unsigned char plc_mc_ins_clean_element(plc_run_power_flow_st *ltp_RunEnv)
{
    struct list_head *ltp_ForCur;
    struct list_head *ltp_ForNext;
    struct list_head *ltp_McHead;
    struct list_head *ltp_Head;
    struct list_head *ltp_ListCur;
    struct list_head *ltp_ListNext;
    plc_mc_mcr_ins_list_st *ltp_McMcrIns;
    plc_ins_list_st *ltp_PlcIns;
    unsigned char *lcp_Ins;
    unsigned short lsv_Temp;
    unsigned char lcv_Ret;
    unsigned char lcv_IsFind;
    plc_run_power_flow_st ltv_RunEnv;
    unsigned short lsv_SbrNum, lsv_CallId;

    lcv_IsFind = 0;

    ltp_McHead = &gtv_McMcrBlockInfo.mtv_McMcrHead;
    list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_McHead) {
        ltp_McMcrIns = list_entry(ltp_ForCur, plc_mc_mcr_ins_list_st, mtv_List);
        if(GET_POINT_ADDR(ltp_McMcrIns->mcp_McUCodePc) == GET_POINT_ADDR(ltp_RunEnv->mcp_PC)) {
            lcv_IsFind = 1;
            break;
        }
    }

    if(lcv_IsFind) {
        ltp_Head = &ltp_McMcrIns->mtv_SimpInsHead;
        list_for_each_safe(ltp_ListCur, ltp_ListNext, ltp_Head) {
            ltp_PlcIns = list_entry(ltp_ListCur, plc_ins_list_st, mtv_InsList);
            lcp_Ins = (unsigned char *)ltp_PlcIns->mlp_InsPc;

            switch(*(lcp_Ins + 1)) {
                case SI_OUT_Y:
                    plc_set_bit_element_value(Y_ELEMENT, *lcp_Ins, 0);
                    break;

                case SI_OUT_SM:
                    plc_set_bit_element_value(SM_ELEMENT, *lcp_Ins, 0);
                    break;

                case SI_OUT_M:
                    plc_set_bit_element_value(M_ELEMENT, *lcp_Ins, 0);
                    break;

                case SI_OUT_LM:
                    plc_set_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, *lcp_Ins, 0);
                    break;

                case SI_OUT_Y_EXT:
                    lsv_Temp = (GET_PU16_DATA(lcp_Ins + 2) <<4) + GET_PU8_DATA(lcp_Ins);
                    plc_set_bit_element_value(Y_ELEMENT, lsv_Temp, 0);
                    break;

                case SI_OUT_SM_EXT:
                    lsv_Temp = (GET_PU16_DATA(lcp_Ins + 2) <<4) + GET_PU8_DATA(lcp_Ins);
                    plc_set_bit_element_value(SM_ELEMENT, lsv_Temp, 0);
                    break;

                case SI_OUT_M_EXT:
                    lsv_Temp = (GET_PU16_DATA(lcp_Ins + 2) <<4) + GET_PU8_DATA(lcp_Ins);
                    plc_set_bit_element_value(M_ELEMENT, lsv_Temp, 0);
                    break;

                case SI_EU:
                case SI_ED:
                    lsv_Temp = GET_PU16_DATA(lcp_Ins+2);
                    RST_EDGE_VALUE(lsv_Temp);
                    RST_EDGE_INIT_FLAG(lsv_Temp);
                    break;
                case SI_LDP:
                case SI_LDF:
                case SI_ANDP:
                case SI_ANDF:
                case SI_ORP:
                case SI_ORF:
                case SI_PLP:
                case SI_PLF:
                    lsv_Temp = GET_PU16_DATA(lcp_Ins+4);
                    RST_EDGE_VALUE(lsv_Temp);
                    RST_EDGE_INIT_FLAG(lsv_Temp);
                    break;
            }
        }

        ltv_RunEnv.mlv_InPF = 0;

        ltp_Head = &ltp_McMcrIns->mtv_CompInsHead;
        list_for_each_safe(ltp_ListCur, ltp_ListNext, ltp_Head) {
            ltp_PlcIns = list_entry(ltp_ListCur, plc_ins_list_st, mtv_InsList);
            lcp_Ins = (unsigned char *)ltp_PlcIns->mlp_InsPc;

            ltv_RunEnv.mcp_PC = lcp_Ins;

            switch(GET_PU16_DATA(lcp_Ins)) {
                case CI_TON:
                    lsv_Temp = GET_PU16_DATA(lcp_Ins+4);
                    plc_reset_one_timer(lsv_Temp);
                    break;

                case CI_TONR:
                    lsv_Temp = GET_PU16_DATA(lcp_Ins+4);
                    gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_Temp].mbv_Enable = 0;
                    break;

                case CI_TOF:
                    lsv_Temp = GET_PU16_DATA(lcp_Ins+4);
                    plc_reset_one_timer(lsv_Temp);
                    break;

                case CI_OUT_LM:
                    lsv_Temp = GET_PU16_DATA(lcp_Ins+4);
                    plc_set_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_Temp, 0);
                    break;

                case CI_BOUT:
                    run_ci_bout_ins(&ltv_RunEnv);
                    break;

                case CI_CALL:
                    /*子程序中调用子程序,获取调用的子程序编号,CALL指令编号*/
                    lsv_SbrNum = GET_PU16_DATA(lcp_Ins+2);
                    lsv_SbrNum &= 0xFF;

                    lcv_Ret = plc_get_call_ins_id(lcp_Ins, &lsv_CallId);
                    if(lcv_Ret != pdPASS) {
                        return lcv_Ret;
                    }

                    if(gtp_CallInsInfoPtr->msv_SbrNestedNum >= MAX_SBR_NESTED_LAYER) {
                        return ERR_OVER_SBR_STACK;
                    }

                    if(gtp_CallInsInfoPtr->mcv_IsExec[lsv_CallId]) {
                        gtp_CallInsInfoPtr->mcv_IsExec[lsv_CallId] = 0;

                        gtp_CallInsInfoPtr->msv_SbrNestedNum ++;
                        lcv_Ret = plc_call_ins_clean_element(lsv_SbrNum);
                        gtp_CallInsInfoPtr->msv_SbrNestedNum--;

                        if(lcv_Ret != pdPASS) {
                            return lcv_Ret;
                        }
                    }
                    break;

                /*当前不支持指令,直接跳过*/
                case CI_DRVI:
                case CI_DRVC:
                case CI_DRVA:
                case CI_ZRN:
                case CI_PLS:
                case CI_PLSB:
                case CI_PWM:
                case CI_PLSY:
                case CI_PLSR:
                case CI_PLSV:
                case CI_DHSCS:
                case CI_HCNT:
                case CI_SPD:
                case CI_GEARBOX:
                case CI_LIN:
                case CI_CW:
                case CI_CCW:
                case CI_CAMBOX:
                    break;

            }
        }
    }

    return pdPASS;
}

/**
  * @brief  主控指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_mc_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_EdgeNum;
    unsigned char lcv_Ret;

    lsv_EdgeNum = GET_PU16_DATA(ltp_RunEnv->mcp_PC+6);

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        /*能流有效,执行主控指令*/
        SET_EDGE_VALUE(lsv_EdgeNum);
        ltp_RunEnv->mcp_PC += 10;
    } else if(GET_EDGE_VALUE(lsv_EdgeNum)) {
        /*能流无效,之前执行过,清特定指令*/
        RST_EDGE_VALUE(lsv_EdgeNum);
        lcv_Ret = plc_mc_ins_clean_element(ltp_RunEnv);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }
        ltp_RunEnv->mcp_PC += GET_PU16_DATA(ltp_RunEnv->mcp_PC+8);
    } else {
        RST_EDGE_VALUE(lsv_EdgeNum);
        ltp_RunEnv->mcp_PC += GET_PU16_DATA(ltp_RunEnv->mcp_PC+8);
    }

    return pdPASS;
}

/**
  * @brief  主控清除指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_mcr_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    ltp_RunEnv->mcp_PC += 6;
    return pdPASS;
}

/*------------------------------------------------------------------------------
*   SFC指令
*-----------------------------------------------------------------------------*/
/**
  * @brief  母线分析
  * @param  None
  * @retval None
  */
unsigned char plc_stl_bus_meet_analyse(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char i;
    unsigned char *lcp_Ucode = ltp_RunEnv->mcp_PC;

    gtp_SfcStatus->msv_StlNum[0] = GET_PU16_DATA(lcp_Ucode+4);

    if(GET_PU16_DATA(lcp_Ucode+10) != CI_STL) {
        gtp_SfcStatus->mcv_SeriesSTLCnt = 1;
        return 0;
    }

    for(i=1; i<MAX_SFC_SERIES_STL_NUM; i++) {
        lcp_Ucode += 10;

        if(GET_PU16_DATA(lcp_Ucode) == CI_STL) {
            gtp_SfcStatus->msv_StlNum[i] = GET_PU16_DATA(lcp_Ucode+4);
            gtp_SfcStatus->mcv_SeriesSTLCnt = 2;
        } else {
            break;
        }
    }

    return 1;
}

/**
  * @brief  判断并行汇流母线中的S是否全有效
  * @param  None
  * @retval None
  */
unsigned char plc_stl_is_all_on()
{
    unsigned char i;

    for(i=0; i<gtp_SfcStatus->mcv_SeriesSTLCnt; i++) {
        if(plc_get_bit_element_value(S_ELEMENT, gtp_SfcStatus->msv_StlNum[i]) == 0) {
            return 0;
        }
    }

    return 1;
}

/**
  * @brief  SFC指令在步进状态能流无效时,清除指令中指定指令输出
  * @param  None
  * @retval None
  */
unsigned char plc_stl_ins_clean_element(unsigned long llv_SNum)
{
    struct list_head *ltp_ForCur;
    struct list_head *ltp_ForNext;
    struct list_head *ltp_Head;
    plc_ins_list_st *ltp_PlcIns;
    unsigned char *lcp_Ins;
    unsigned short lsv_Temp;
    unsigned char lcv_Ret;
    plc_run_power_flow_st ltv_RunEnv;
    unsigned short lsv_SbrNum, lsv_CallId;

    if(llv_SNum > S_RANG) {
        return ERR_OVER_ELEMENT_RANG;
    }



    ltp_Head = &gtp_StlListPtr[llv_SNum].mtv_SimpInsHead;
    list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
        ltp_PlcIns = list_entry(ltp_ForCur, plc_ins_list_st, mtv_InsList);
        lcp_Ins = (unsigned char *)ltp_PlcIns->mlp_InsPc;

        switch(*(lcp_Ins + 1)) {
            case SI_OUT_Y:
                plc_set_bit_element_value(Y_ELEMENT, *lcp_Ins, 0);
                break;

            case SI_OUT_SM:
                plc_set_bit_element_value(SM_ELEMENT, *lcp_Ins, 0);
                break;

            case SI_OUT_M:
                plc_set_bit_element_value(M_ELEMENT, *lcp_Ins, 0);
                break;

            case SI_OUT_LM:
                plc_set_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, *lcp_Ins, 0);
                break;

            case SI_OUT_Y_EXT:
                lsv_Temp = (GET_PU16_DATA(lcp_Ins + 2) <<4) + GET_PU8_DATA(lcp_Ins);
                plc_set_bit_element_value(Y_ELEMENT, lsv_Temp, 0);
                break;

            case SI_OUT_SM_EXT:
                lsv_Temp = (GET_PU16_DATA(lcp_Ins + 2) <<4) + GET_PU8_DATA(lcp_Ins);
                plc_set_bit_element_value(SM_ELEMENT, lsv_Temp, 0);
                break;

            case SI_OUT_M_EXT:
                lsv_Temp = (GET_PU16_DATA(lcp_Ins + 2) <<4) + GET_PU8_DATA(lcp_Ins);
                plc_set_bit_element_value(M_ELEMENT, lsv_Temp, 0);
                break;

            case SI_EU:
            case SI_ED:
                lsv_Temp = GET_PU16_DATA(lcp_Ins+2);
                RST_EDGE_VALUE(lsv_Temp);
                RST_EDGE_INIT_FLAG(lsv_Temp);
                break;
            case SI_LDP:
            case SI_LDF:
            case SI_ANDP:
            case SI_ANDF:
            case SI_ORP:
            case SI_ORF:
            case SI_PLP:
            case SI_PLF:
                lsv_Temp = GET_PU16_DATA(lcp_Ins+4);
                RST_EDGE_VALUE(lsv_Temp);
                RST_EDGE_INIT_FLAG(lsv_Temp);
        }
    }

    ltv_RunEnv.mlv_InPF = 0;

    ltp_Head = &gtp_StlListPtr[llv_SNum].mtv_CompInsHead;
    list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
        ltp_PlcIns = list_entry(ltp_ForCur, plc_ins_list_st, mtv_InsList);
        lcp_Ins = (unsigned char *)ltp_PlcIns->mlp_InsPc;

        ltv_RunEnv.mcp_PC = lcp_Ins;

        switch(GET_PU16_DATA(lcp_Ins)) {
            case CI_TON:
                lsv_Temp = GET_PU16_DATA(lcp_Ins+4);
                plc_reset_one_timer(lsv_Temp);
                break;

            case CI_TONR:
                lsv_Temp = GET_PU16_DATA(lcp_Ins+4);
                gtv_PlcElement.mtv_TElement.mtp_StatusInfo[lsv_Temp].mbv_Enable = 0;
                break;

            case CI_TOF:
                lsv_Temp = GET_PU16_DATA(lcp_Ins+4);
                plc_reset_one_timer(lsv_Temp);
                break;

            case CI_OUT_LM:
                lsv_Temp = GET_PU16_DATA(lcp_Ins+4);
                plc_set_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_Temp, 0);
                break;

            case CI_BOUT:
                run_ci_bout_ins(&ltv_RunEnv);
                break;

            case CI_CALL:
                /*子程序中调用子程序,获取调用的子程序编号,CALL指令编号*/
                lsv_SbrNum = GET_PU16_DATA(lcp_Ins+2);
                lsv_SbrNum &= 0xFF;

                lcv_Ret = plc_get_call_ins_id(lcp_Ins, &lsv_CallId);
                if(lcv_Ret != pdPASS) {
                    return lcv_Ret;
                }

                if(gtp_CallInsInfoPtr->msv_SbrNestedNum >= MAX_SBR_NESTED_LAYER) {
                    return ERR_OVER_SBR_STACK;
                }

                if(gtp_CallInsInfoPtr->mcv_IsExec[lsv_CallId]) {
                    gtp_CallInsInfoPtr->mcv_IsExec[lsv_CallId] = 0;

                    gtp_CallInsInfoPtr->msv_SbrNestedNum ++;
                    lcv_Ret = plc_call_ins_clean_element(lsv_SbrNum);
                    gtp_CallInsInfoPtr->msv_SbrNestedNum--;

                    if(lcv_Ret != pdPASS) {
                        return lcv_Ret;
                    }
                }
                break;

            /*当前不支持指令,直接跳过*/
            case CI_DRVI:
            case CI_DRVC:
            case CI_DRVA:
            case CI_ZRN:
            case CI_PLS:
            case CI_PLSB:
            case CI_PWM:
            case CI_PLSY:
            case CI_PLSR:
            case CI_PLSV:
            case CI_DHSCS:
            case CI_HCNT:
            case CI_SPD:
            case CI_GEARBOX:
            case CI_LIN:
            case CI_CW:
            case CI_CCW:
            case CI_CAMBOX:
                break;

        }
    }
    return pdPASS;
}

/**
  * @brief  SFC状态装载指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_stl_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    unsigned char lcv_StlNum;
    unsigned short lsv_EdgeNum;

    lcv_StlNum = GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);

    gtp_SfcStatus->mcv_SeriesSTLCnt = 0;
    gtp_SfcStatus->mcv_sfcEnable = 1;

    lcv_Ret = plc_stl_bus_meet_analyse(ltp_RunEnv);
    if(!lcv_Ret) {
        lsv_EdgeNum = GET_PU16_DATA(ltp_RunEnv->mcp_PC + 6);
        /*步进状态是否有效*/
        if(plc_get_bit_element_value(S_ELEMENT, lcv_StlNum)) {
            /*第一次运行*/
            if(GET_EDGE_INIT_FLAG(lsv_EdgeNum)) {
                /*清除首次运行标志*/
                RST_EDGE_INIT_FLAG(lsv_EdgeNum);

                /*清除*/
                plc_stl_ins_clean_element(lcv_StlNum);
            }

            SET_EDGE_VALUE(lsv_EdgeNum);
            SET_POWER_FLOW(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;

            return pdPASS;
        } else {
            /*步进状态无效,且位执行过*/
            if(GET_EDGE_VALUE(lsv_EdgeNum) == 0) {
                ltp_RunEnv->mcp_PC += GET_PU16_DATA(ltp_RunEnv->mcp_PC+8);
                return pdPASS;
            }

            /*清除已执行过标志*/
            RST_EDGE_VALUE(lsv_EdgeNum);
            /**/
            plc_stl_ins_clean_element(lcv_StlNum);
            ltp_RunEnv->mcp_PC += GET_PU16_DATA(ltp_RunEnv->mcp_PC+8);
            return pdPASS;
        }
    }


    if(plc_stl_is_all_on()) {
        /*汇流母线中的状态都有效,设置能流ON,跳走执行*/
        SET_POWER_FLOW(ltp_RunEnv);
        ltp_RunEnv->mcp_PC += 10*(gtp_SfcStatus->mcv_SeriesSTLCnt);
    } else {
        /*汇流母线状态无效,计算跳转位置*/
        ltp_RunEnv->mcp_PC += 10*(gtp_SfcStatus->mcv_SeriesSTLCnt);

        ltp_RunEnv->mcp_PC += GET_PU16_DATA(ltp_RunEnv->mcp_PC+8);
    }

    return pdPASS;
}


/**
  * @brief  SFC状态转移指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_set_s_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_SNum;
    unsigned char i;

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        lsv_SNum = GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);
        if(gtp_SfcStatus->mcv_sfcEnable) {
            for(i=0; i<gtp_SfcStatus->mcv_SeriesSTLCnt; i++) {
                plc_set_bit_element_value(S_ELEMENT, gtp_SfcStatus->msv_StlNum[i], 0);
            }
        }

        plc_set_bit_element_value(S_ELEMENT, lsv_SNum, 1);
    }

    return pdPASS;
}

/**
  * @brief  SFC状态跳转指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_out_s_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_SNum;
    unsigned char i;

    lsv_SNum = GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        if(gtp_SfcStatus->mcv_sfcEnable) {
            for(i=0; i<gtp_SfcStatus->mcv_SeriesSTLCnt; i++) {
                plc_set_bit_element_value(S_ELEMENT, gtp_SfcStatus->msv_StlNum[i], 0);
            }
        }

        plc_set_bit_element_value(S_ELEMENT, lsv_SNum, 1);
    } else {
        if(!gtp_SfcStatus->mcv_sfcEnable) {
            plc_set_bit_element_value(S_ELEMENT, lsv_SNum, 0);
        }
    }

    return pdPASS;
}

/**
  * @brief  SFC状态清除指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_rst_s_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_SNum;
    unsigned char i;

    if(GET_POWER_FLOW(ltp_RunEnv)) {
        lsv_SNum = GET_PU16_DATA(ltp_RunEnv->mcp_PC+4);

        if(gtp_SfcStatus->mcv_sfcEnable) {
            for(i=0; i<gtp_SfcStatus->mcv_SeriesSTLCnt; i++) {
                plc_set_bit_element_value(S_ELEMENT, gtp_SfcStatus->msv_StlNum[i], 0);
            }
        }

        plc_set_bit_element_value(S_ELEMENT, lsv_SNum, 0);
    }

    return pdPASS;
}

/**
  * @brief  SFC程序段结束
  * @param  None
  * @retval None
  */
unsigned char run_ci_ret_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    gtp_SfcStatus->mcv_sfcEnable = 0;
    return pdPASS;
}

