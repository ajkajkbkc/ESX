/**
  ******************************************************************************
  * @file    plc_simpleins.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   简单指令解释执行函数定义
  ******************************************************************************
  */
#include "fsl_debug_console.h"
#include "plc_instruction.h"
#include "plc_element.h"
#include "plc_variable.h"
#include "plc_commonfunc.h"
#include "plc_errormsg.h"
#include "plc_simpleins.h"
#include "plc_executer.h"

/**
  * @brief  解释运行简单指令
  * @param  None
  * @retval None
  */
unsigned char plc_exec_simple_instruction(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned long llv_InPF;
    unsigned char *lcp_UcodePC;
    unsigned short lsv_TempValue;
    unsigned char  lsv_charValue;
    uint8_t        lcv_Ret;

    llv_InPF = ltp_RunEnv->mlv_InPF;
    lcp_UcodePC = ltp_RunEnv->mcp_PC;

    /*连续处理相连的简单指令,直到遇到复杂指令退出*/
    while(GET_PU16_DATA(lcp_UcodePC) < 0xE000) {
        gcp_BackupUserPc = lcp_UcodePC;
        switch(GET_PU8_DATA(lcp_UcodePC + 1)) {
            case SI_LD_X:
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(X_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                    llv_InPF |= 0x01L;
                } else {
                    llv_InPF &= 0xFFFFFFFEL;
                }
                lcp_UcodePC += 2;
                break;

            case SI_LD_Y:
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(Y_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                    llv_InPF |= 0x01L;
                } else {
                    llv_InPF &= 0xFFFFFFFEL;
                }
                lcp_UcodePC += 2;
                break;

            case SI_LD_C:
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(C_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                    llv_InPF |= 0x01L;
                } else {
                    llv_InPF &= 0xFFFFFFFEL;
                }
                lcp_UcodePC += 2;
                break;

            case SI_LD_T:
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(T_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                    llv_InPF |= 0x01L;
                } else {
                    llv_InPF &= 0xFFFFFFFEL;
                }
                lcp_UcodePC += 2;
                break;

            case SI_LD_SM:
                /*能留压栈*/
                //PRINTF("SI_LD_SM\r\n");
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(SM_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                    llv_InPF |= 0x01L;
                } else {
                    llv_InPF &= 0xFFFFFFFEL;
                }
                lcp_UcodePC += 2;
                break;

            case SI_LD_M:
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(M_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                    llv_InPF |= 0x01L;
                } else {
                    llv_InPF &= 0xFFFFFFFEL;
                }
                lcp_UcodePC += 2;
                break;

            case SI_LD_S:
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(S_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                    llv_InPF |= 0x01L;
                } else {
                    llv_InPF &= 0xFFFFFFFEL;
                }
                lcp_UcodePC += 2;
                break;

            case SI_LD_LM:
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, GET_PU8_DATA(lcp_UcodePC))) {
                    llv_InPF |= 0x01L;
                } else {
                    llv_InPF &= 0xFFFFFFFEL;
                }
                lcp_UcodePC += 2;
                break;

            case SI_LD_X_EXT:
                lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(X_ELEMENT, lsv_TempValue)) {
                    llv_InPF |= 0x01L;
                } else {
                    llv_InPF &= 0xFFFFFFFEL;
                }
                lcp_UcodePC += 4;
                break;

            case SI_LD_Y_EXT:
                lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(Y_ELEMENT, lsv_TempValue)) {
                    llv_InPF |= 0x01L;
                } else {
                    llv_InPF &= 0xFFFFFFFEL;
                }
                lcp_UcodePC += 4;
                break;

            case SI_LD_C_EXT:
                lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(C_ELEMENT, lsv_TempValue)) {
                    llv_InPF |= 0x01L;
                } else {
                    llv_InPF &= 0xFFFFFFFEL;
                }
                lcp_UcodePC += 4;
                break;

            case SI_LD_T_EXT:
                lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(T_ELEMENT, lsv_TempValue)) {
                    llv_InPF |= 0x01L;
                } else {
                    llv_InPF &= 0xFFFFFFFEL;
                }
                lcp_UcodePC += 4;
                break;

            case SI_LD_SM_EXT:
                lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(SM_ELEMENT, lsv_TempValue)) {
                    llv_InPF |= 0x01L;
                } else {
                    llv_InPF &= 0xFFFFFFFEL;
                }
                lcp_UcodePC += 4;
                break;

            case SI_LD_M_EXT:
                lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(M_ELEMENT, lsv_TempValue)) {
                    llv_InPF |= 0x01L;
                } else {
                    llv_InPF &= 0xFFFFFFFEL;
                }
                lcp_UcodePC += 4;
                break;

            case SI_LD_S_EXT:
                lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(S_ELEMENT, lsv_TempValue)) {
                    llv_InPF |= 0x01L;
                } else {
                    llv_InPF &= 0xFFFFFFFEL;
                }
                lcp_UcodePC += 4;
                break;

            case SI_LDI_X:
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(X_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                    llv_InPF &= 0xFFFFFFFEL;
                } else {
                    llv_InPF |= 0x01L;
                }
                lcp_UcodePC += 2;
                break;

            case SI_LDI_Y:
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(Y_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                    llv_InPF &= 0xFFFFFFFEL;
                } else {
                    llv_InPF |= 0x01L;
                }
                lcp_UcodePC += 2;
                break;

            case SI_LDI_C:
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(C_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                    llv_InPF &= 0xFFFFFFFEL;
                } else {
                    llv_InPF |= 0x01L;
                }
                lcp_UcodePC += 2;
                break;

            case SI_LDI_T:
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(T_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                    llv_InPF &= 0xFFFFFFFEL;
                } else {
                    llv_InPF |= 0x01L;
                }
                lcp_UcodePC += 2;
                break;

            case SI_LDI_SM:
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(SM_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                    llv_InPF &= 0xFFFFFFFEL;
                } else {
                    llv_InPF |= 0x01L;
                }
                lcp_UcodePC += 2;
                break;

            case SI_LDI_M:
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(M_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                    llv_InPF &= 0xFFFFFFFEL;
                } else {
                    llv_InPF |= 0x01L;
                }
                lcp_UcodePC += 2;
                break;

            case SI_LDI_S:
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(S_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                    llv_InPF &= 0xFFFFFFFEL;
                } else {
                    llv_InPF |= 0x01L;
                }
                lcp_UcodePC += 2;
                break;

            case SI_LDI_LM:
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, GET_PU8_DATA(lcp_UcodePC))) {
                    llv_InPF &= 0xFFFFFFFEL;
                } else {
                    llv_InPF |= 0x01L;
                }
                lcp_UcodePC += 2;
                break;
            case SI_LDI_X_EXT:
                lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(X_ELEMENT, lsv_TempValue)) {
                    llv_InPF &= 0xFFFFFFFEL;
                } else {
                    llv_InPF |= 0x01L;
                }
                lcp_UcodePC += 4;
                break;

            case SI_LDI_Y_EXT:
                lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(Y_ELEMENT, lsv_TempValue)) {
                    llv_InPF &= 0xFFFFFFFEL;
                } else {
                    llv_InPF |= 0x01L;
                }
                lcp_UcodePC += 4;
                break;

            case SI_LDI_C_EXT:
                lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(C_ELEMENT, lsv_TempValue)) {
                    llv_InPF &= 0xFFFFFFFEL;
                } else {
                    llv_InPF |= 0x01L;
                }
                lcp_UcodePC += 4;
                break;

            case SI_LDI_T_EXT:
                lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(T_ELEMENT, lsv_TempValue)) {
                    llv_InPF &= 0xFFFFFFFEL;
                } else {
                    llv_InPF |= 0x01L;
                }
                lcp_UcodePC += 4;
                break;

            case SI_LDI_SM_EXT:
                lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(SM_ELEMENT, lsv_TempValue)) {
                    llv_InPF &= 0xFFFFFFFEL;
                } else {
                    llv_InPF |= 0x01L;
                }
                lcp_UcodePC += 4;
                break;

            case SI_LDI_M_EXT:
                lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(M_ELEMENT, lsv_TempValue)) {
                    llv_InPF &= 0xFFFFFFFEL;
                } else {
                    llv_InPF |= 0x01L;
                }
                lcp_UcodePC += 4;
                break;

            case SI_LDI_S_EXT:
                lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);
                /*能留压栈*/
                llv_InPF <<= 1;
                if(plc_get_bit_element_value(S_ELEMENT, lsv_TempValue)) {
                    llv_InPF &= 0xFFFFFFFEL;
                } else {
                    llv_InPF |= 0x01L;
                }
                lcp_UcodePC += 4;
                break;

            case SI_AND_X:
                if(llv_InPF & 0x01) {
                    if(!plc_get_bit_element_value(X_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_AND_Y:
                if(llv_InPF & 0x01) {
                    if(!plc_get_bit_element_value(Y_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_AND_C:
                if(llv_InPF & 0x01) {
                    if(!plc_get_bit_element_value(C_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_AND_T:
                if(llv_InPF & 0x01) {
                    if(!plc_get_bit_element_value(T_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_AND_SM:
                if(llv_InPF & 0x01) {
                    if(!plc_get_bit_element_value(SM_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_AND_M:
                if(llv_InPF & 0x01) {
                    if(!plc_get_bit_element_value(M_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_AND_S:
                if(llv_InPF & 0x01) {
                    if(!plc_get_bit_element_value(S_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_AND_LM:
                if(llv_InPF & 0x01) {
                    if(!plc_get_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_AND_X_EXT:
                if(llv_InPF & 0x01) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(!plc_get_bit_element_value(X_ELEMENT, lsv_TempValue)) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_AND_Y_EXT:
                if(llv_InPF & 0x01) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(!plc_get_bit_element_value(Y_ELEMENT, lsv_TempValue)) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_AND_C_EXT:
                if(llv_InPF & 0x01) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(!plc_get_bit_element_value(C_ELEMENT, lsv_TempValue)) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_AND_T_EXT:
                if(llv_InPF & 0x01) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(!plc_get_bit_element_value(T_ELEMENT, lsv_TempValue)) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_AND_SM_EXT:
                if(llv_InPF & 0x01) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(!plc_get_bit_element_value(SM_ELEMENT, lsv_TempValue)) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_AND_M_EXT:
                if(llv_InPF & 0x01) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(!plc_get_bit_element_value(M_ELEMENT, lsv_TempValue)) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_AND_S_EXT:
                if(llv_InPF & 0x01) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(!plc_get_bit_element_value(S_ELEMENT, lsv_TempValue)) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_ANI_X:
                if(llv_InPF & 0x01) {
                    if(plc_get_bit_element_value(X_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_ANI_Y:
                if(llv_InPF & 0x01) {
                    if(plc_get_bit_element_value(Y_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_ANI_C:
                if(llv_InPF & 0x01) {
                    if(plc_get_bit_element_value(C_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_ANI_T:
                if(llv_InPF & 0x01) {
                    if(plc_get_bit_element_value(T_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_ANI_SM:
                if(llv_InPF & 0x01) {
                    if(plc_get_bit_element_value(SM_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_ANI_M:
                if(llv_InPF & 0x01) {
                    if(plc_get_bit_element_value(M_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_ANI_S:
                if(llv_InPF & 0x01) {
                    if(plc_get_bit_element_value(S_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_ANI_LM:
                if(llv_InPF & 0x01) {
                    if(plc_get_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_ANI_X_EXT:
                if(llv_InPF & 0x01) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(plc_get_bit_element_value(X_ELEMENT, lsv_TempValue)) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_ANI_Y_EXT:
                if(llv_InPF & 0x01) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(plc_get_bit_element_value(Y_ELEMENT, lsv_TempValue)) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_ANI_C_EXT:
                if(llv_InPF & 0x01) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(plc_get_bit_element_value(C_ELEMENT, lsv_TempValue)) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_ANI_T_EXT:
                if(llv_InPF & 0x01) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(plc_get_bit_element_value(T_ELEMENT, lsv_TempValue)) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_ANI_SM_EXT:
                if(llv_InPF & 0x01) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(plc_get_bit_element_value(SM_ELEMENT, lsv_TempValue)) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_ANI_M_EXT:
                if(llv_InPF & 0x01) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(plc_get_bit_element_value(M_ELEMENT, lsv_TempValue)) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_ANI_S_EXT:
                if(llv_InPF & 0x01) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(plc_get_bit_element_value(S_ELEMENT, lsv_TempValue)) {
                        llv_InPF &= 0xFFFFFFFEL;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_OR_X:
                if(!(llv_InPF & 0x01)) {
                    if(plc_get_bit_element_value(X_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_OR_Y:
                if(!(llv_InPF & 0x01)) {
                    if(plc_get_bit_element_value(Y_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_OR_C:
                if(!(llv_InPF & 0x01)) {
                    if(plc_get_bit_element_value(C_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_OR_T:
                if(!(llv_InPF & 0x01)) {
                    if(plc_get_bit_element_value(T_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_OR_SM:
                if(!(llv_InPF & 0x01)) {
                    if(plc_get_bit_element_value(SM_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_OR_M:
                if(!(llv_InPF & 0x01)) {
                    if(plc_get_bit_element_value(M_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_OR_S:
                if(!(llv_InPF & 0x01)) {
                    if(plc_get_bit_element_value(S_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_OR_LM:
                if(!(llv_InPF & 0x01)) {
                    if(plc_get_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_OR_X_EXT:
                if(!(llv_InPF & 0x01)) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(plc_get_bit_element_value(X_ELEMENT, lsv_TempValue)) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_OR_Y_EXT:
                if(!(llv_InPF & 0x01)) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(plc_get_bit_element_value(Y_ELEMENT, lsv_TempValue)) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_OR_C_EXT:
                if(!(llv_InPF & 0x01)) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(plc_get_bit_element_value(C_ELEMENT, lsv_TempValue)) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_OR_T_EXT:
                if(!(llv_InPF & 0x01)) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(plc_get_bit_element_value(T_ELEMENT, lsv_TempValue)) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_OR_SM_EXT:
                if(!(llv_InPF & 0x01)) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(plc_get_bit_element_value(SM_ELEMENT, lsv_TempValue)) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_OR_M_EXT:
                if(!(llv_InPF & 0x01)) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(plc_get_bit_element_value(M_ELEMENT, lsv_TempValue)) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_OR_S_EXT:
                if(!(llv_InPF & 0x01)) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(plc_get_bit_element_value(S_ELEMENT, lsv_TempValue)) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_ORI_X:
                if(!(llv_InPF & 0x01)) {
                    if(!plc_get_bit_element_value(X_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_ORI_Y:
                if(!(llv_InPF & 0x01)) {
                    if(!plc_get_bit_element_value(Y_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_ORI_C:
                if(!(llv_InPF & 0x01)) {
                    if(!plc_get_bit_element_value(C_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_ORI_T:
                if(!(llv_InPF & 0x01)) {
                    if(!plc_get_bit_element_value(T_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_ORI_SM:
                if(!(llv_InPF & 0x01)) {
                    if(!plc_get_bit_element_value(SM_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_ORI_M:
                if(!(llv_InPF & 0x01)) {
                    if(!plc_get_bit_element_value(M_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_ORI_S:
                if(!(llv_InPF & 0x01)) {
                    if(!plc_get_bit_element_value(S_ELEMENT, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_ORI_LM:
                if(!(llv_InPF & 0x01)) {
                    if(!plc_get_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, GET_PU8_DATA(lcp_UcodePC))) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 2;
                break;

            case SI_ORI_X_EXT:
                if(!(llv_InPF & 0x01)) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(!plc_get_bit_element_value(X_ELEMENT, lsv_TempValue)) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_ORI_Y_EXT:
                if(!(llv_InPF & 0x01)) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(!plc_get_bit_element_value(Y_ELEMENT, lsv_TempValue)) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_ORI_C_EXT:
                if(!(llv_InPF & 0x01)) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(!plc_get_bit_element_value(C_ELEMENT, lsv_TempValue)) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_ORI_T_EXT:
                if(!(llv_InPF & 0x01)) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(!plc_get_bit_element_value(T_ELEMENT, lsv_TempValue)) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_ORI_SM_EXT:
                if(!(llv_InPF & 0x01)) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(!plc_get_bit_element_value(SM_ELEMENT, lsv_TempValue)) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_ORI_M_EXT:
                if(!(llv_InPF & 0x01)) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(!plc_get_bit_element_value(M_ELEMENT, lsv_TempValue)) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_ORI_S_EXT:
                if(!(llv_InPF & 0x01)) {
                    lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);

                    if(!plc_get_bit_element_value(S_ELEMENT, lsv_TempValue)) {
                        llv_InPF |= 0x01L;
                    }
                }
                lcp_UcodePC += 4;
                break;

            case SI_OUT_Y:
                //PRINTF("SI_OUT_Y\r\n");
                if(llv_InPF & 0x01) {
                    plc_set_bit_element_value(Y_ELEMENT, GET_PU8_DATA(lcp_UcodePC), 1);
                } else {
                    plc_set_bit_element_value(Y_ELEMENT, GET_PU8_DATA(lcp_UcodePC), 0);
                }
                lcp_UcodePC += 2;
                break;

            case SI_OUT_SM:
                if(llv_InPF & 0x01) {
                    plc_set_bit_element_value(SM_ELEMENT, GET_PU8_DATA(lcp_UcodePC), 1);
                } else {
                    plc_set_bit_element_value(SM_ELEMENT, GET_PU8_DATA(lcp_UcodePC), 0);
                }
                lcp_UcodePC += 2;
                break;

            case SI_OUT_M:
                if(llv_InPF & 0x01) {
                    plc_set_bit_element_value(M_ELEMENT, GET_PU8_DATA(lcp_UcodePC), 1);
                } else {
                    plc_set_bit_element_value(M_ELEMENT, GET_PU8_DATA(lcp_UcodePC), 0);
                }
                lcp_UcodePC += 2;
                break;

            case SI_OUT_Y_EXT:
                lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);
                if(llv_InPF & 0x01) {
                    plc_set_bit_element_value(Y_ELEMENT, lsv_TempValue, 1);
                } else {
                    plc_set_bit_element_value(Y_ELEMENT, lsv_TempValue, 0);
                }
                lcp_UcodePC += 4;
                break;

            case SI_OUT_SM_EXT:
                lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);
                if(llv_InPF & 0x01) {
                    plc_set_bit_element_value(SM_ELEMENT, lsv_TempValue, 1);
                } else {
                    plc_set_bit_element_value(SM_ELEMENT, lsv_TempValue, 0);
                }
                lcp_UcodePC += 4;
                break;

            case SI_OUT_M_EXT:
                lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);
                if(llv_InPF & 0x01) {
                    plc_set_bit_element_value(M_ELEMENT, lsv_TempValue, 1);
                } else {
                    plc_set_bit_element_value(M_ELEMENT, lsv_TempValue, 0);
                }
                lcp_UcodePC += 4;
                break;

            case SI_SET_Y:
                if(llv_InPF & 0x01) {
                    plc_set_bit_element_value(Y_ELEMENT, GET_PU8_DATA(lcp_UcodePC), 1);
                }
                lcp_UcodePC += 2;
                break;

            case SI_SET_SM:
                if(llv_InPF & 0x01) {
                    plc_set_bit_element_value(SM_ELEMENT, GET_PU8_DATA(lcp_UcodePC), 1);
                }
                lcp_UcodePC += 2;
                break;

            case SI_SET_M:
                if(llv_InPF & 0x01) {
                    plc_set_bit_element_value(M_ELEMENT, GET_PU8_DATA(lcp_UcodePC), 1);
                }
                lcp_UcodePC += 2;
                break;

            case SI_SET_LM:
                if(llv_InPF & 0x01) {
                    plc_set_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, GET_PU8_DATA(lcp_UcodePC), 1);
                }
                lcp_UcodePC += 2;
                break;

            case SI_SET_Y_EXT:
                lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);
                if(llv_InPF & 0x01) {
                    plc_set_bit_element_value(Y_ELEMENT, lsv_TempValue, 1);
                }
                lcp_UcodePC += 4;
                break;

            case SI_SET_SM_EXT:
                lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);
                if(llv_InPF & 0x01) {
                    plc_set_bit_element_value(SM_ELEMENT, lsv_TempValue, 1);
                }
                lcp_UcodePC += 4;
                break;

            case SI_SET_M_EXT:
                lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);
                if(llv_InPF & 0x01) {
                    plc_set_bit_element_value(M_ELEMENT, lsv_TempValue, 1);
                }
                lcp_UcodePC += 4;
                break;

            case SI_RST_Y:
                if(llv_InPF & 0x01) {
                    plc_set_bit_element_value(Y_ELEMENT, GET_PU8_DATA(lcp_UcodePC), 0);
                }
                lcp_UcodePC += 2;
                break;

            case SI_RST_SM:
                if(llv_InPF & 0x01) {
                    plc_set_bit_element_value(SM_ELEMENT, GET_PU8_DATA(lcp_UcodePC), 0);
                }
                lcp_UcodePC += 2;
                break;

            case SI_RST_M:
                if(llv_InPF & 0x01) {
                    plc_set_bit_element_value(M_ELEMENT, GET_PU8_DATA(lcp_UcodePC), 0);
                }
                lcp_UcodePC += 2;
                break;

            case SI_RST_LM:
                if(llv_InPF & 0x01) {
                    plc_set_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, GET_PU8_DATA(lcp_UcodePC), 0);
                }
                lcp_UcodePC += 2;
                break;

            case SI_RST_Y_EXT:
                lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);
                if(llv_InPF & 0x01) {
                    plc_set_bit_element_value(Y_ELEMENT, lsv_TempValue, 0);
                }
                lcp_UcodePC += 4;
                break;

            case SI_RST_SM_EXT:
                lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);
                if(llv_InPF & 0x01) {
                    plc_set_bit_element_value(SM_ELEMENT, lsv_TempValue, 0);
                }
                lcp_UcodePC += 4;
                break;

            case SI_RST_M_EXT:
                lsv_TempValue = (GET_PU16_DATA(lcp_UcodePC + 2) <<4) + GET_PU8_DATA(lcp_UcodePC);
                if(llv_InPF & 0x01) {
                    plc_set_bit_element_value(M_ELEMENT, lsv_TempValue, 0);
                }
                lcp_UcodePC += 4;
                break;

            case SI_INV:
                llv_InPF ^= (0x01L);
                lcp_UcodePC += 2;
                break;

            case SI_ANB:
                if((llv_InPF&0x03L)!= 0x03L) {
                    llv_InPF &= 0xFFFFFFFC;
                }
                llv_InPF >>= 0x01;
                lcp_UcodePC += 2;
                break;

            case SI_ORB:
                if((llv_InPF&0x03L) != 0x00L) {
                    llv_InPF |= 0x02L;
                }
                llv_InPF >>= 0x01;
                lcp_UcodePC += 2;
                break;

            case SI_MPS:
                ltp_RunEnv->mlv_OutPF <<= 0x01;
                ltp_RunEnv->mlv_OutPF |= (llv_InPF&0x01);
                lcp_UcodePC += 2;
                break;

            case SI_MRD:
                if(ltp_RunEnv->mlv_OutPF & 0x01L) {
                    llv_InPF |= 0x01L;
                } else {
                    llv_InPF &= 0xFFFFFFFEL;
                }
                lcp_UcodePC += 2;
                break;

            case SI_MPP:
                if(ltp_RunEnv->mlv_OutPF & 0x01L) {
                    llv_InPF |= 0x01L;
                } else {
                    llv_InPF &= 0xFFFFFFFEL;
                }
                ltp_RunEnv->mlv_OutPF >>= 0x01;
                lcp_UcodePC += 2;
                break;

            case SI_NOP:
                lcp_UcodePC += 2;
                break;

            case SI_EU:
                ltp_RunEnv->mlv_InPF = llv_InPF;
                ltp_RunEnv->mcp_PC = lcp_UcodePC + 2;
                run_eu_ins(ltp_RunEnv);
                llv_InPF = ltp_RunEnv->mlv_InPF;
                lcp_UcodePC += 4;
                break;

            case SI_ED:
                ltp_RunEnv->mlv_InPF = llv_InPF;
                ltp_RunEnv->mcp_PC = lcp_UcodePC + 2;
                run_ed_ins(ltp_RunEnv);
                llv_InPF = ltp_RunEnv->mlv_InPF;
                lcp_UcodePC += 4;
                break;
              
            //ldp m4 = 05 B0 04 00 ** **(沿号)
            //LDP X5 = 00 B0 05 00 ** **(沿号)
            case SI_LDP:
                LOGV("SI_LDP", "llv_InPF = 0x%x", llv_InPF);
                lcv_Ret = get_char_in_simpleIns(lcp_UcodePC, &lsv_charValue);
                /*能留压栈*/
                llv_InPF <<= 1;

                ltp_RunEnv->mlv_InPF = llv_InPF;
                ltp_RunEnv->mcp_PC = lcp_UcodePC + 4;
                run_eu_p_ins(ltp_RunEnv,lsv_charValue);
                llv_InPF = ltp_RunEnv->mlv_InPF;
                LOGV("SI_LDP", "lsv_charValue = 0x%x, llv_InPF = 0x%x", lsv_charValue,llv_InPF);
                lcp_UcodePC += 6;
                break;
            case SI_LDF:
                LOGV("SI_LDF", "llv_InPF = 0x%x", llv_InPF);
                lcv_Ret = get_char_in_simpleIns(lcp_UcodePC, &lsv_charValue);
                /*能留压栈*/
                llv_InPF <<= 1;

                ltp_RunEnv->mlv_InPF = llv_InPF;
                ltp_RunEnv->mcp_PC = lcp_UcodePC + 4;
                run_ed_f_ins(ltp_RunEnv,lsv_charValue);
                llv_InPF = ltp_RunEnv->mlv_InPF;
                LOGV("SI_LDF", "lsv_charValue = 0x%x, llv_InPF = 0x%x", lsv_charValue,llv_InPF);									

                lcp_UcodePC += 6;
                break;
            case SI_ANDP:
                if(llv_InPF & 0x01) 
                {
                    lcv_Ret = get_char_in_simpleIns(lcp_UcodePC, &lsv_charValue);
                    ltp_RunEnv->mlv_InPF = llv_InPF;
                    ltp_RunEnv->mcp_PC = lcp_UcodePC + 4;
                    run_eu_p_ins(ltp_RunEnv,lsv_charValue);
                    llv_InPF = ltp_RunEnv->mlv_InPF;
                 }
                lcp_UcodePC += 6;
                break;
            case SI_ANDF:
                if(llv_InPF & 0x01) 
                {
                    lcv_Ret = get_char_in_simpleIns(lcp_UcodePC, &lsv_charValue);
                    LOGV("llv_InPF", " SI_ANDF = 0x%x", lsv_charValue);
                    ltp_RunEnv->mlv_InPF = llv_InPF;
                    ltp_RunEnv->mcp_PC = lcp_UcodePC + 4;
                    run_ed_f_ins(ltp_RunEnv,lsv_charValue);
                    llv_InPF = ltp_RunEnv->mlv_InPF;
                    LOGV("llv_InPF", " llv_InPF = 0x%x", llv_InPF);	
                }
                lcp_UcodePC += 6;
                break;
            case SI_ORP:
                if(!(llv_InPF & 0x01))
                {
                    lcv_Ret = get_char_in_simpleIns(lcp_UcodePC, &lsv_charValue);
                    ltp_RunEnv->mlv_InPF = llv_InPF;
                    ltp_RunEnv->mcp_PC = lcp_UcodePC + 4;
                    run_eu_p_ins(ltp_RunEnv,lsv_charValue);
                    llv_InPF = ltp_RunEnv->mlv_InPF;
                }
                lcp_UcodePC += 6;
                break;
            case SI_ORF:
                if(!(llv_InPF & 0x01)) 
                {
                    lcv_Ret = get_char_in_simpleIns(lcp_UcodePC, &lsv_charValue);
                    ltp_RunEnv->mlv_InPF = llv_InPF;
                    ltp_RunEnv->mcp_PC = lcp_UcodePC + 4;
                    run_ed_f_ins(ltp_RunEnv,lsv_charValue);
                    llv_InPF = ltp_RunEnv->mlv_InPF;
                }
                lcp_UcodePC += 6;
                break;
            case SI_PLP:
                ltp_RunEnv->mlv_InPF = llv_InPF;
                ltp_RunEnv->mcp_PC = lcp_UcodePC + 4;
                run_eu_ins(ltp_RunEnv);
                llv_InPF = ltp_RunEnv->mlv_InPF;
                if(llv_InPF & 0x01) {
                    save_char_in_simpleIns(lcp_UcodePC, 1);
                } else {
                    save_char_in_simpleIns(lcp_UcodePC, 0);
                }
                lcp_UcodePC += 6;
                break;
            case SI_PLF:
                ltp_RunEnv->mlv_InPF = llv_InPF;
                ltp_RunEnv->mcp_PC = lcp_UcodePC + 4;
                run_ed_ins(ltp_RunEnv);
                llv_InPF = ltp_RunEnv->mlv_InPF;
                if(llv_InPF & 0x01) {
                    save_char_in_simpleIns(lcp_UcodePC, 1);
                } else {
                    save_char_in_simpleIns(lcp_UcodePC, 0);
                }
                lcp_UcodePC += 6;
                break;
            default:
                ltp_RunEnv->mlv_InPF = llv_InPF;
                ltp_RunEnv->mcp_PC = lcp_UcodePC;
                /*未知指令*/
                return ERR_ILLEGAL_INSTRCTION;
        }
    }

    ltp_RunEnv->mlv_InPF = llv_InPF;
    ltp_RunEnv->mcp_PC = lcp_UcodePC;

    return pdPASS;
}



/**
  * @brief  解释运行简单指令
  * @param  None
  * @retval None
  */
void run_eu_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_EdgeNum;

    lsv_EdgeNum = GET_PU16_DATA(ltp_RunEnv->mcp_PC);

    /*UCODE第一次运行*/
    if(GET_EDGE_INIT_FLAG(lsv_EdgeNum)) {
        /*清除首次运行标志*/
        RST_EDGE_INIT_FLAG(lsv_EdgeNum);

        /*保存本周期能流值*/
        if(GET_POWER_FLOW(ltp_RunEnv)) {
            SET_EDGE_VALUE(lsv_EdgeNum);
        } else {
            RST_EDGE_VALUE(lsv_EdgeNum);
        }

        /*首次运行,本周能流无效*/
        RST_POWER_FLOW(ltp_RunEnv);
    } else {
        /*上个扫描周期能流无效,本周期能流有效为上升沿,其他清除能流标志*/
        if(GET_POWER_FLOW(ltp_RunEnv)) {
            if(GET_EDGE_VALUE(lsv_EdgeNum)) {
                RST_POWER_FLOW(ltp_RunEnv);
            } else {
                SET_POWER_FLOW(ltp_RunEnv);
            }

            SET_EDGE_VALUE(lsv_EdgeNum);
        } else {
            RST_EDGE_VALUE(lsv_EdgeNum);
            RST_POWER_FLOW(ltp_RunEnv);
        }
    }
}

/**
  * @brief  解释运行简单指令
  * @param  None
  * @retval None
  */
void run_ed_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short lsv_EdgeNum;

    lsv_EdgeNum = GET_PU16_DATA(ltp_RunEnv->mcp_PC);

    /*UCODE第一次运行*/
    if(GET_EDGE_INIT_FLAG(lsv_EdgeNum)) {
        /*清除首次运行标志*/
        RST_EDGE_INIT_FLAG(lsv_EdgeNum);

        /*保存本周期能流值*/
        if(GET_POWER_FLOW(ltp_RunEnv)) {
            SET_EDGE_VALUE(lsv_EdgeNum);
        } else {
            RST_EDGE_VALUE(lsv_EdgeNum);
        }

        /*首次运行,本周能流无效*/
        RST_POWER_FLOW(ltp_RunEnv);
    } else {
        /*上个扫描周期能流有效,本周期能流无效为下降沿,其他清除能流标志*/
        if(GET_POWER_FLOW(ltp_RunEnv)) {
            SET_EDGE_VALUE(lsv_EdgeNum);
            RST_POWER_FLOW(ltp_RunEnv);
        } else {
            if(GET_EDGE_VALUE(lsv_EdgeNum)) {
                SET_POWER_FLOW(ltp_RunEnv);
            } else {
                RST_POWER_FLOW(ltp_RunEnv);
            }

            RST_EDGE_VALUE(lsv_EdgeNum);
        }
    }
}

/**
  * @brief  解释LDP ANDP ORP等简单指令
  * @param  None
  * @retval None
  */
void run_eu_p_ins(plc_run_power_flow_st *ltp_RunEnv,unsigned char lsv_charValue)
{
    unsigned short lsv_EdgeNum;

    lsv_EdgeNum = GET_PU16_DATA(ltp_RunEnv->mcp_PC);

    /*UCODE第一次运行*/
    if(GET_EDGE_INIT_FLAG(lsv_EdgeNum)) 
    {
        RST_EDGE_INIT_FLAG(lsv_EdgeNum);
        /*首次运行,本周能流无效*/
        RST_POWER_FLOW(ltp_RunEnv);
    } 
    else 
    {
        if((!GET_EDGE_VALUE(lsv_EdgeNum)) && lsv_charValue) 
        {
            SET_POWER_FLOW(ltp_RunEnv);
        } 
        else 
        {
            RST_POWER_FLOW(ltp_RunEnv);
        }
    }

    /*保存本周期能流值*/
    if(lsv_charValue) 
    {
        SET_EDGE_VALUE(lsv_EdgeNum);
    }
    else 
    {
        RST_EDGE_VALUE(lsv_EdgeNum);
    }
}

/**
  * @brief  解释LDF ANDF ORF等简单指令
  * @param  None
  * @retval None
  */
void run_ed_f_ins(plc_run_power_flow_st *ltp_RunEnv,unsigned char lsv_charValue)
{
    unsigned short lsv_EdgeNum;

    lsv_EdgeNum = GET_PU16_DATA(ltp_RunEnv->mcp_PC);

    /*UCODE第一次运行*/
    if(GET_EDGE_INIT_FLAG(lsv_EdgeNum)) 
    {
        RST_EDGE_INIT_FLAG(lsv_EdgeNum);
        /*首次运行,本周能流无效*/
        RST_POWER_FLOW(ltp_RunEnv);
    } 
    else 
    {
        if((GET_EDGE_VALUE(lsv_EdgeNum)) && (!lsv_charValue)) 
        {
           SET_POWER_FLOW(ltp_RunEnv);
        } 
        else 
        {
           RST_POWER_FLOW(ltp_RunEnv);
        }
     }
    /*保存本周期能流值*/
    if(lsv_charValue) 
     {
        SET_EDGE_VALUE(lsv_EdgeNum);
     } 
     else 
     {
        RST_EDGE_VALUE(lsv_EdgeNum);
     }
}