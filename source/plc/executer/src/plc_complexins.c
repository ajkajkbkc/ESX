/**
  ******************************************************************************
  * @file    plc_complexins.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   复杂指令解释执行函数定义
  ******************************************************************************
  */
#include "fsl_debug_console.h"
#include "plc_instruction.h"
#include "plc_element.h"
#include "plc_variable.h"
#include "plc_commonfunc.h"
#include "plc_errormsg.h"
#include "plc_complexins.h"
#include "plc_dataprocessing.h"
#include "plc_strins.h"
#include "plc_flowctrlins.h"
#include "plc_timeins.h"
#include "plc_counterins.h"
#include "plc_externalins.h"
#include "plc_parseaddr.h"
#include "plc_verifyins.h"
#include "plc_uartins.h"
#include "plc_modbusins.h"
#include "plc_highspeedins.h"
#include "plc_executer.h"
#include "plc_modbustcpins.h"
#include "plc_soem.h"
#include "plc_pid.h"
#include "plc_tcpins.h"

/**
  * @brief  解释运行复杂指令
  * @param  None
  * @retval None
  */
unsigned char plc_exec_complex_instruction(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_RetValue = pdPASS;

EXEC_CI_INS_LOOP:
    gcp_BackupUserPc = ltp_RunEnv->mcp_PC;
    switch(GET_PU16_DATA(ltp_RunEnv->mcp_PC)) {
        case CI_LBL:
            lcv_RetValue = run_ci_lbl_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 6;
            break;

        case CI_SBR:
            lcv_RetValue = run_ci_sbr_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 4;
            break;

        case CI_INTR:
            /*20170803: 后续补充...*/
            //lcv_RetValue = ERR_ELEMENT_TYPE;
            lcv_RetValue = pdPASS;
            ltp_RunEnv->mcp_PC += 4;
            break;

        case CI_LD_LM:
            lcv_RetValue = run_ci_ld_lm_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 6;
            break;

        case CI_LDI_LM:
            lcv_RetValue = run_ci_ldi_lm_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 6;
            break;

        case CI_AND_LM:
            lcv_RetValue = run_ci_and_lm_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 6;
            break;

        case CI_ANI_LM:
            lcv_RetValue = run_ci_ani_lm_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 6;
            break;

        case CI_OR_LM:
            lcv_RetValue = run_ci_or_lm_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 6;
            break;

        case CI_ORI_LM:
            lcv_RetValue = run_ci_ori_lm_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 6;
            break;

        case CI_OUT_LM:
            lcv_RetValue = run_ci_out_lm_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 6;
            break;

        case CI_SET_LM:
            lcv_RetValue = run_ci_set_lm_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 6;
            break;

        case CI_RST_LM:
            lcv_RetValue = run_ci_rst_lm_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 6;
            break;

        case CI_STL:
            lcv_RetValue = run_ci_stl_ins(ltp_RunEnv);
            break;

        case CI_RST_S:
            lcv_RetValue = run_ci_rst_s_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 6;
            break;

        case CI_SET_S:
            lcv_RetValue = run_ci_set_s_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 6;
            break;

        case CI_OUT_S:
            lcv_RetValue = run_ci_out_s_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 6;
            break;

        case CI_RET:
            lcv_RetValue = run_ci_ret_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 2;
            break;

        case CI_TON:
            lcv_RetValue = run_ci_ton_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_TOF:
            lcv_RetValue = run_ci_tof_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_TONR:
            lcv_RetValue = run_ci_tonr_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_RST_T:
            lcv_RetValue = run_ci_rst_t_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 6;
            break;

        case CI_DCNT:
            lcv_RetValue = run_ci_dcnt_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_CTU:
            lcv_RetValue = run_ci_ctu_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_CTR:
            lcv_RetValue = run_ci_ctr_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_RST_C:
            lcv_RetValue = run_ci_rst_c_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 6;
            break;

        case CI_MC:
            lcv_RetValue = run_ci_mc_ins(ltp_RunEnv);
            break;

        case CI_MCR:
            lcv_RetValue = run_ci_mcr_ins(ltp_RunEnv);
            break;

        case CI_CJ:
            lcv_RetValue = run_ci_cj_ins(ltp_RunEnv);
            break;

        case CI_FOR:
            lcv_RetValue = run_ci_for_ins(ltp_RunEnv);
            break;

        case CI_NEXT:
            lcv_RetValue = run_ci_next_ins(ltp_RunEnv);
            break;

        case CI_CONTINUE:
            lcv_RetValue = run_ci_continue_ins(ltp_RunEnv);
            break;

        case CI_BREAK:
            lcv_RetValue = run_ci_break_ins(ltp_RunEnv);
            break;

        case CI_CALL:
            lcv_RetValue = run_ci_call_ins(ltp_RunEnv);
            break;

        case CI_CSRET:
            lcv_RetValue = run_ci_csret_ins(ltp_RunEnv);
            break;

        case CI_SRET:
            lcv_RetValue = run_ci_sret_ins(ltp_RunEnv);
            break;

        case CI_CIRET:
            lcv_RetValue = run_ci_ciret_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 2;
            break;

        case CI_IRET:
            lcv_RetValue = run_ci_iret_ins(ltp_RunEnv);
            break;

        case CI_EI:
            lcv_RetValue = run_ci_ei_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 2;
            break;

        case CI_DI:
            lcv_RetValue = run_ci_di_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 2;
            break;

        case CI_STOP:
            lcv_RetValue = run_ci_stop_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 2;
            break;

        case CI_WDT:
            lcv_RetValue = run_ci_wdt_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 2;
            break;

        case CI_CFEND:
            lcv_RetValue = run_ci_cfend_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 2;
            break;

        case CI_FEND:
            lcv_RetValue = run_ci_fend_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_MOV:
            lcv_RetValue = run_ci_mov_ins(ltp_RunEnv);
            //PRINTF("CI_MOV: lcv_RetValue = %d\r\n", lcv_RetValue);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_DMOV:
            lcv_RetValue = run_ci_dmov_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_BMOV:
            lcv_RetValue = run_ci_bmov_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_FMOV:
            lcv_RetValue = run_ci_fmov_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_DFMOV:
            lcv_RetValue = run_ci_dfmov_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;

        case CI_SWAP:
            lcv_RetValue = run_ci_swap_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 6;
            break;

        case CI_XCH:
            lcv_RetValue = run_ci_xch_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_DXCH:
            lcv_RetValue = run_ci_dxch_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_PUSH:
            lcv_RetValue = run_ci_push_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_WSFL:
            lcv_RetValue = run_ci_wsfl_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;

        /*整数比较指令*/
        case CI_LDE:
        case CI_LDG:
        case CI_LDL:
        case CI_LDNE:
        case CI_LDLE:
        case CI_LDGE:
            lcv_RetValue = run_ld_compare_ins(ltp_RunEnv);
            if(lcv_RetValue != pdPASS) {
                return lcv_RetValue;
            }

            ltp_RunEnv->mcp_PC += 10;
            break;

        /*整数比较AND指令*/
        case CI_ANDE:
        case CI_ANDG:
        case CI_ANDL:
        case CI_ANDNE:
        case CI_ANDLE:
        case CI_ANDGE:
            if(GET_POWER_FLOW(ltp_RunEnv)) {
                lcv_RetValue = run_and_compare_ins(ltp_RunEnv);
                if(lcv_RetValue != pdPASS) {
                    return lcv_RetValue;
                }
            }

            ltp_RunEnv->mcp_PC += 10;
            break;

        /*整数比较 OR 指令*/
        case CI_ORE:
        case CI_ORG:
        case CI_ORL:
        case CI_ORNE:
        case CI_ORLE:
        case CI_ORGE:
            if(!GET_POWER_FLOW(ltp_RunEnv)) {
                lcv_RetValue = run_or_compare_ins(ltp_RunEnv);
                if(lcv_RetValue != pdPASS) {
                    return lcv_RetValue;
                }
            }

            ltp_RunEnv->mcp_PC += 10;
            break;

        /*长整数比较*/
        case CI_LDDE:
        case CI_LDDG:
        case CI_LDDL:
        case CI_LDDNE:
        case CI_LDDLE:
        case CI_LDDGE:
            lcv_RetValue = run_ldd_compare_ins(ltp_RunEnv);
            if(lcv_RetValue != pdPASS) {
                return lcv_RetValue;
            }

            ltp_RunEnv->mcp_PC += 14;
            break;

        /*长整数比较AND指令*/
        case CI_ANDDE:
        case CI_ANDDG:
        case CI_ANDDL:
        case CI_ANDDNE:
        case CI_ANDDLE:
        case CI_ANDDGE:
            if(GET_POWER_FLOW(ltp_RunEnv)) {
                lcv_RetValue = run_andd_compare_ins(ltp_RunEnv);
                if(lcv_RetValue != pdPASS) {
                    return lcv_RetValue;
                }
            }

            ltp_RunEnv->mcp_PC += 14;
            break;

        /*长整数比较 OR 指令*/
        case CI_ORDE:
        case CI_ORDG:
        case CI_ORDL:
        case CI_ORDNE:
        case CI_ORDLE:
        case CI_ORDGE:
            if(!GET_POWER_FLOW(ltp_RunEnv)) {
                lcv_RetValue = run_ord_compare_ins(ltp_RunEnv);
                if(lcv_RetValue != pdPASS) {
                    return lcv_RetValue;
                }
            }

            ltp_RunEnv->mcp_PC += 14;
            break;

        /*长整数比较*/
        case CI_LDRE:
        case CI_LDRG:
        case CI_LDRL:
        case CI_LDRNE:
        case CI_LDRLE:
        case CI_LDRGE:
            lcv_RetValue = run_ldr_compare_ins(ltp_RunEnv);
            if(lcv_RetValue != pdPASS) {
                return lcv_RetValue;
            }

            ltp_RunEnv->mcp_PC += 14;
            break;

        /*长整数比较AND指令*/
        case CI_ANDRE:
        case CI_ANDRG:
        case CI_ANDRL:
        case CI_ANDRNE:
        case CI_ANDRLE:
        case CI_ANDRGE:
            if(GET_POWER_FLOW(ltp_RunEnv)) {
                lcv_RetValue = run_andr_compare_ins(ltp_RunEnv);
                if(lcv_RetValue != pdPASS) {
                    return lcv_RetValue;
                }
            }

            ltp_RunEnv->mcp_PC += 14;
            break;

        /*长整数比较 OR 指令*/
        case CI_ORRE:
        case CI_ORRG:
        case CI_ORRL:
        case CI_ORRNE:
        case CI_ORRLE:
        case CI_ORRGE:
            if(!GET_POWER_FLOW(ltp_RunEnv)) {
                lcv_RetValue = run_orr_compare_ins(ltp_RunEnv);
                if(lcv_RetValue != pdPASS) {
                    return lcv_RetValue;
                }
            }

            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_ADD:
            lcv_RetValue = run_ci_add_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_SUB:
            lcv_RetValue = run_ci_sub_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_MUL:
            lcv_RetValue = run_ci_mul_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 16;
            break;

        case CI_DIV:
            //PRINTF("CI_DIV\r\n");
            lcv_RetValue = run_ci_div_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_SQT:
            lcv_RetValue = run_ci_sqt_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_INC:
            lcv_RetValue = run_ci_inc_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 6;
            break;

        case CI_DEC:
            lcv_RetValue = run_ci_dec_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 6;
            break;

        case CI_VABS:
            lcv_RetValue = run_ci_vabs_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_NEG:
            lcv_RetValue = run_ci_neg_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_DADD:
            lcv_RetValue = run_ci_dadd_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 20;
            break;

        case CI_DSUB:
            lcv_RetValue = run_ci_dsub_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 20;
            break;

        case CI_DMUL:
            lcv_RetValue = run_ci_dmul_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 20;
            break;

        case CI_DDIV:
            lcv_RetValue = run_ci_ddiv_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 20;
            break;

        case CI_DSQT:
            lcv_RetValue = run_ci_dsqt_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_DINC:
            lcv_RetValue = run_ci_dinc_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 8;
            break;

        case CI_DDEC:
            lcv_RetValue = run_ci_ddec_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 8;
            break;

        case CI_DVABS:
            lcv_RetValue = run_ci_dvabs_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_DNEG:
            lcv_RetValue = run_ci_dneg_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_RADD:
            lcv_RetValue = run_ci_radd_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 20;
            break;

        case CI_RSUB:
            lcv_RetValue = run_ci_rsub_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 20;
            break;

        case CI_RMUL:
            lcv_RetValue = run_ci_rmul_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 20;
            break;

        case CI_RDIV:
            lcv_RetValue = run_ci_rdiv_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 20;
            break;

        case CI_RSQT:
            lcv_RetValue = run_ci_rsqt_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_SIN:
            lcv_RetValue = run_ci_sin_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_COS:
            lcv_RetValue = run_ci_cos_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_TAN:
            lcv_RetValue = run_ci_tan_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_LN:
            lcv_RetValue = run_ci_ln_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_EXP:
            lcv_RetValue = run_ci_exp_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_RNEG:
            lcv_RetValue = run_ci_rneg_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_POWER:
            lcv_RetValue = run_ci_power_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 20;
            break;

        case CI_RVABS:
            lcv_RetValue = run_ci_rvabs_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_WAND:
            lcv_RetValue = run_ci_wand_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_WOR:
            lcv_RetValue = run_ci_wor_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_WXOR:
            lcv_RetValue = run_ci_wxor_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_WINV:
            lcv_RetValue = run_ci_winv_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_DWAND:
            lcv_RetValue = run_ci_dwand_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 20;
            break;

        case CI_DWOR:
            lcv_RetValue = run_ci_dwor_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 20;
            break;

        case CI_DWXOR:
            lcv_RetValue = run_ci_dwxor_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 20;
            break;

        case CI_DWINV:
            lcv_RetValue = run_ci_dwinv_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_ROR:
            lcv_RetValue = run_ci_ror_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_ROL:
            lcv_RetValue = run_ci_rol_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_RCR:
            lcv_RetValue = run_ci_rcr_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_RCL:
            lcv_RetValue = run_ci_rcl_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_DROR:
            lcv_RetValue = run_ci_dror_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;

        case CI_DROL:
            lcv_RetValue = run_ci_drol_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;

        case CI_DRCR:
            lcv_RetValue = run_ci_drcr_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;

        case CI_DRCL:
            lcv_RetValue = run_ci_drcl_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;

        case CI_SHR:
            lcv_RetValue = run_ci_shr_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_SHL:
            lcv_RetValue = run_ci_shl_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_DSHR:
            lcv_RetValue = run_ci_dshr_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;

        case CI_DSHL:
            lcv_RetValue = run_ci_dshl_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;

        case CI_SFTR:
            lcv_RetValue = run_ci_sftr_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;

        case CI_SFTL:
            lcv_RetValue = run_ci_sftl_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;

        case CI_FLT:
            lcv_RetValue = run_ci_flt_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 12;
            break;

        case CI_DFLT:
            lcv_RetValue = run_ci_dflt_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_INT:
            lcv_RetValue = run_ci_int_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 12;
            break;

        case CI_DINT:
            lcv_RetValue = run_ci_dint_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_BCD:
            lcv_RetValue = run_ci_bcd_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_DBCD:
            lcv_RetValue = run_ci_dbcd_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_BIN:
            lcv_RetValue = run_ci_bin_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_DBIN:
            lcv_RetValue = run_ci_dbin_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_GRY:
            lcv_RetValue = run_ci_gry_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_DGRY:
            lcv_RetValue = run_ci_dgry_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_GBIN:
            lcv_RetValue = run_ci_gbin_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_DGBIN:
            lcv_RetValue = run_ci_dgbin_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_SEG:
            lcv_RetValue = run_ci_seg_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_ASC:
            lcv_RetValue = run_ci_asc_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 38;
            break;

        case CI_ITA:
            lcv_RetValue = run_ci_ita_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_ATI:
            lcv_RetValue = run_ci_ati_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_PR:
            /*20170803:后续补充...*/
            lcv_RetValue = ERR_ILLEGAL_INSTRCTION;					
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_REF:
            /*20170803:后续补充...*/
            lcv_RetValue = ERR_ILLEGAL_INSTRCTION;					
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_REFF:
            lcv_RetValue = run_ci_reff_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 6;
            break;

        case CI_DECO:
            lcv_RetValue = run_ci_deco_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_ENCO:
            lcv_RetValue = run_ci_enco_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_BITS:
            lcv_RetValue = run_ci_bits_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_ZRST:
            lcv_RetValue = run_ci_zrst_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_ZSET:
            lcv_RetValue = run_ci_zset_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_PID:
            lcv_RetValue = run_ci_pid_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;

        case CI_XMT:
            lcv_RetValue = run_ci_xmt_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_TRD:
            lcv_RetValue = run_ci_trd_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 6;
            break;

        case CI_TWR:
            lcv_RetValue = run_ci_twr_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 6;
            break;

        case CI_TADD:
            lcv_RetValue = run_ci_tadd_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_TSUB:
            lcv_RetValue = run_ci_tsub_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_FIFO:
            lcv_RetValue = run_ci_fifo_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_LIFO:
            lcv_RetValue = run_ci_lifo_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_HOUR:
            lcv_RetValue = run_ci_hour_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 16;
            break;

        case CI_BLD:
            lcv_RetValue = run_ci_bld_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_BLDI:
            lcv_RetValue = run_ci_bldi_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_BAND:
            lcv_RetValue = run_ci_band_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_BANDI:
            lcv_RetValue = run_ci_bandi_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_BOR:
            lcv_RetValue = run_ci_bor_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_BORI:
            lcv_RetValue = run_ci_bori_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_DBITS:
            lcv_RetValue = run_ci_dbits_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 12;
            break;

        case CI_BON:
            lcv_RetValue = run_ci_bon_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_BSET:
            lcv_RetValue = run_ci_bset_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_BRST:
            lcv_RetValue = run_ci_brst_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_WSFR:
            lcv_RetValue = run_ci_wsfr_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;

        case CI_ITD:
            lcv_RetValue = run_ci_itd_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 12;
            break;

        case CI_DTI:
            lcv_RetValue = run_ci_dti_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 12;
            break;

        case CI_BOUT:
            lcv_RetValue = run_ci_bout_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_RMOV:
            lcv_RetValue = run_ci_rmov_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_RCV:
            lcv_RetValue = run_ci_rcv_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_MODBUS:
            lcv_RetValue = run_ci_modbus_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 16;
            break;

        case CI_SUM:
            lcv_RetValue = run_ci_sum_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 16;
            break;

        case CI_DSUM:
            lcv_RetValue = run_ci_dsum_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;

        case CI_RSUM:
            lcv_RetValue = run_ci_rsum_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;

        case CI_DCMPE:
        case CI_DCMPG:
        case CI_DCMPL:
        case CI_DCMPNE:
        case CI_DCMPLE:
        case CI_DCMPGE:
            lcv_RetValue = run_ci_dcmp_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_TCMPE:
        case CI_TCMPG:
        case CI_TCMPL:
        case CI_TCMPNE:
        case CI_TCMPLE:
        case CI_TCMPGE:
            lcv_RetValue = run_ci_tcmp_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_RAMP:
            /*20170803:后续补充...*/
            lcv_RetValue = ERR_ILLEGAL_INSTRCTION;
            ltp_RunEnv->mcp_PC += 24;
            break;

        case CI_HACKLE:
            /*20170803:后续补充...*/
            lcv_RetValue = ERR_ILLEGAL_INSTRCTION;
            ltp_RunEnv->mcp_PC += 24;
            break;

        case CI_TRIANGLE:
            /*20170803:后续补充...*/
            lcv_RetValue = ERR_ILLEGAL_INSTRCTION;
            ltp_RunEnv->mcp_PC += 24;
            break;

        case CI_TMON:
            lcv_RetValue = run_ci_tmon_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 12;
            break;

        case CI_CCITT:
            lcv_RetValue = run_ci_ccitt_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_CRC16:
            lcv_RetValue = run_ci_crc16_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_LRC:
            lcv_RetValue = run_ci_lrc_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_CANXMT:
            /*20170803:后续补充...*/
            lcv_RetValue = ERR_ILLEGAL_INSTRCTION;
            ltp_RunEnv->mcp_PC += 26;
            break;

        case CI_CANRCV:
            /*20170803:后续补充...*/
            lcv_RetValue = ERR_ILLEGAL_INSTRCTION;
            ltp_RunEnv->mcp_PC += 26;
            break;

        case CI_CANMXMT:
            /*20170803:后续补充...*/
            lcv_RetValue = ERR_ILLEGAL_INSTRCTION;
            ltp_RunEnv->mcp_PC += 20;
            break;

        case CI_LCNV:
            lcv_RetValue = run_ci_lcnv_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;

        case CI_CMP:
            lcv_RetValue = run_ci_cmp_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_LCMP:
            lcv_RetValue = run_ci_lcmp_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;

        case CI_RCMP:
            lcv_RetValue = run_ci_rcmp_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;

        case CI_ALT:
            lcv_RetValue = run_ci_alt_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 6;
            break;

        case CI_MODRW:
            lcv_RetValue = run_ci_modrw_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 28;
            break;

        case CI_ASIN:
            lcv_RetValue = run_ci_asin_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_ACOS:
            lcv_RetValue = run_ci_acos_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_ATAN:
            lcv_RetValue = run_ci_atan_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_RLCNV:
            lcv_RetValue = run_ci_rlcnv_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 24;
            break;

#if (KALYKE_MODBUS_TCP_SHEET == 0)
        case CI_MODLINK:
            lcv_RetValue = run_ci_modlink_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 20;
            break;

        case CI_MBCCONN:
            lcv_RetValue = run_ci_mbcconn_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 28;
            break;

        case CI_MODTCPLINK:
            lcv_RetValue = run_ci_mbclink_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 28;
            break;
#endif

        case CI_TCP_CONN:
            lcv_RetValue = run_ci_tcp_conn_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 32;
            break;
        case CI_TCP_XMT:
            lcv_RetValue = run_ci_tcp_xmt_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 24;
            break;
        case CI_TCP_RCV:
            lcv_RetValue = run_ci_tcp_rcv_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 24;
            break;
        case CI_TCP_PING:
            lcv_RetValue = run_ci_tcp_ping_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 20;
            break;

        case CI_LOG:
            lcv_RetValue = run_ci_log_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_RAD:
            lcv_RetValue = run_ci_rad_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_DEG:
            lcv_RetValue = run_ci_deg_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;


        case CI_LD_DX_Y:
            lcv_RetValue = run_ci_ld_dx_y_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 8;
            break;

        case CI_LDI_DX_Y:
            lcv_RetValue = run_ci_ldi_dx_y_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 8;
            break;

        case CI_AND_DX_Y:
            lcv_RetValue = run_ci_and_dx_y_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 8;
            break;

        case CI_ANI_DX_Y:
            lcv_RetValue = run_ci_ani_dx_y_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 8;
            break;

        case CI_OR_DX_Y:
            lcv_RetValue = run_ci_or_dx_y_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 8;
            break;

        case CI_ORI_DX_Y:
            lcv_RetValue = run_ci_ori_dx_y_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 8;
            break;

        case CI_SET_DX_Y:
            lcv_RetValue = run_ci_set_dx_y_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 8;
            break;

        case CI_RST_DX_Y:
            lcv_RetValue = run_ci_rst_dx_y_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 8;
            break;

        case CI_OUT_DX_Y:
            lcv_RetValue = run_ci_out_dx_y_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 8;
            break;

        case CI_MEAN:
            lcv_RetValue = run_ci_mean_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_WTOB:
            lcv_RetValue = run_ci_wtob_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_BTOW:
            lcv_RetValue = run_ci_btow_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_UNI:
            lcv_RetValue = run_ci_uni_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_DIS:
            lcv_RetValue = run_ci_dis_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_ANS:
            lcv_RetValue = run_ci_ans_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_ANR:
            lcv_RetValue = run_ci_anr_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 2;
            break;

        case CI_BKADD:
            lcv_RetValue = run_ci_bkadd_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;

        case CI_BKSUB:
            lcv_RetValue = run_ci_bksub_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;

        case CI_BKCMPDE:
        case CI_BKCMPDG:
        case CI_BKCMPDL:
        case CI_BKCMPDNE:
        case CI_BKCMPDLE:
        case CI_BKCMPDGE:
            lcv_RetValue = run_ci_bkcmp_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;

        case CI_LIMIT:
            lcv_RetValue = run_ci_limit_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;

        case CI_DBAND:
            lcv_RetValue = run_ci_dband_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;

        case CI_ZONE:
            lcv_RetValue = run_ci_zone_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;

        case CI_SCL:
            lcv_RetValue = run_ci_scl_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_SER:
            lcv_RetValue = run_ci_ser_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;
        
        case CI_STRADD:
            lcv_RetValue = run_ci_stradd_ins(ltp_RunEnv);
            if(GET_PU8_DATA(ltp_RunEnv->mcp_PC+3) == ADDR_STR) {
                ltp_RunEnv->mcp_PC += GET_PU8_DATA(ltp_RunEnv->mcp_PC+2) + 4;
            } else {
                ltp_RunEnv->mcp_PC += 6;
            }

            if(GET_PU8_DATA(ltp_RunEnv->mcp_PC+1) == ADDR_STR) {
                ltp_RunEnv->mcp_PC += GET_PU8_DATA(ltp_RunEnv->mcp_PC) + 6;
            } else {
                ltp_RunEnv->mcp_PC += 8;
            }
            break;

        case CI_STRLEN:
            lcv_RetValue = run_ci_strlen_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_STRRIGHT:
            lcv_RetValue = run_ci_strright_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_STRLEFT:
            lcv_RetValue = run_ci_strleft_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_STRMIDR:
            lcv_RetValue = run_ci_strmidr_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_STRMIDW:
            lcv_RetValue = run_ci_strmidw_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_STRINSTR:
            lcv_RetValue = run_ci_strinstr_ins(ltp_RunEnv);
            if(GET_PU8_DATA(ltp_RunEnv->mcp_PC+3) == ADDR_STR) {
                ltp_RunEnv->mcp_PC += GET_PU8_DATA(ltp_RunEnv->mcp_PC+2) + 16;
            } else {
                ltp_RunEnv->mcp_PC += 18;
            }
            break;

        case CI_STRMOV:
            lcv_RetValue = run_ci_strmov_ins(ltp_RunEnv);
            if(GET_PU8_DATA(ltp_RunEnv->mcp_PC+3) == ADDR_STR) {
                ltp_RunEnv->mcp_PC += GET_PU8_DATA(ltp_RunEnv->mcp_PC+2) + 8;
            } else {
                ltp_RunEnv->mcp_PC += 10;
            }
            break;

        case CI_RND:
            lcv_RetValue = run_ci_rnd_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 6;
            break;

        case CI_DUTY:
            lcv_RetValue = run_ci_duty_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_HTOS:
            lcv_RetValue = run_ci_htos_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_STOH:
            lcv_RetValue = run_ci_stoh_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_SWAPWORD:
            lcv_RetValue = run_ci_swapword_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;

        case CI_ECATSDORD:
            lcv_RetValue = run_ci_ecatsdord_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 36;
            break;
        case CI_ECATSDOWR:
            lcv_RetValue = run_ci_ecatsdowr_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 36;
            break;

        case CI_PLSY:
            lcv_RetValue = run_ci_plsy_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 18;
            break;

        /*** 当前不支持指令,直接跳过 ***/
        /*扩展文件寄存器指令*/
        case CI_LOADR:
        case CI_INITR:
        case CI_INITER:
            lcv_RetValue = ERR_ILLEGAL_INSTRCTION;
            ltp_RunEnv->mcp_PC += 10;
            break;

        case CI_EVREV:
        case CI_EVFWD:
        case CI_EVDFWD:
        case CI_EVDREV:
        case CI_EROMWR:
            lcv_RetValue = ERR_ILLEGAL_INSTRCTION;
            ltp_RunEnv->mcp_PC += 12;
            break;

        case CI_PWM:
            lcv_RetValue = run_ci_pwm_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_SAVER:            
        case CI_PLS:
            lcv_RetValue = ERR_ILLEGAL_INSTRCTION;
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_SPD:
            lcv_RetValue = run_ci_spd_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;
        
        case CI_HCNT:
            lcv_RetValue = run_ci_hcnt_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 14;
            break;

        case CI_EVSTOP:
        case CI_EVFRQ:
        case CI_ABS:
            lcv_RetValue = ERR_ILLEGAL_INSTRCTION;
            ltp_RunEnv->mcp_PC += 16;
            break;

        case CI_PLSV:
            lcv_RetValue = run_ci_plsv_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 16;
            break;

        case CI_GEARBOX:
        case CI_DSZR:
        case CI_FROM:
        case CI_TO:
            lcv_RetValue = ERR_ILLEGAL_INSTRCTION;
            ltp_RunEnv->mcp_PC += 18;
            break;

        case CI_DHSCS:
            lcv_RetValue = run_ci_dhscs_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 20;
            break;
        case CI_DHSCR:
            lcv_RetValue = run_ci_dhscr_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 20;
            break;
        case CI_DHST:
            lcv_RetValue = run_ci_dhst_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 20;
            break;
        case CI_DHSP:
            lcv_RetValue = run_ci_dhsp_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 20;
            break;
        case CI_DHSCI:
            lcv_RetValue = run_ci_dhsci_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 20;
            break;
        case CI_PLSR:
            lcv_RetValue = run_ci_plsr_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 20;
            break;
        case CI_DFROM:
        case CI_DTO:
        case CI_EVWRT:
        case CI_EVRDST:
        case CI_EVRD:
        case CI_DRVC:
        case CI_DHSPI:
            lcv_RetValue = ERR_ILLEGAL_INSTRCTION;
            ltp_RunEnv->mcp_PC += 20;
            break;

        case CI_LOGR:
        case CI_CAMBOX:
        case CI_CAMTABLE:
            lcv_RetValue = ERR_ILLEGAL_INSTRCTION;
            ltp_RunEnv->mcp_PC += 22;
            break;
        case CI_ZRN:
            lcv_RetValue = run_ci_zrn_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 24;
            break;
        case CI_DRVI:
            lcv_RetValue = run_ci_drvi_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 24;
            break;
			
        case CI_DRVA:
            lcv_RetValue = run_ci_drva_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 24;
            break;

        case CI_LIN:
        case CI_CW:
        case CI_CCW:
        case CI_STOPDV:
            lcv_RetValue = ERR_ILLEGAL_INSTRCTION;
            ltp_RunEnv->mcp_PC += 24;
            break;

        case CI_PLSB:
            lcv_RetValue = run_ci_plsb_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 24;
            break;

        case CI_DHSZ:
            lcv_RetValue = run_ci_dhsz_ins(ltp_RunEnv);
            ltp_RunEnv->mcp_PC += 26;
            break;
        case CI_DRV:
            lcv_RetValue = ERR_ILLEGAL_INSTRCTION;
            ltp_RunEnv->mcp_PC += 26;
            break;

        case CI_MOVELINK:
            lcv_RetValue = ERR_ILLEGAL_INSTRCTION;
            ltp_RunEnv->mcp_PC += 34;
            break;

        default:
            lcv_RetValue = ERR_ILLEGAL_INSTRCTION;
    }

    if((lcv_RetValue == pdPASS) && (GET_PU16_DATA(ltp_RunEnv->mcp_PC) >= 0xF000)) {
        goto EXEC_CI_INS_LOOP;
    }

    return lcv_RetValue;
}

