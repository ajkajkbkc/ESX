/**
  ******************************************************************************
  * @file    plc_compiler.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   PLC由STOP切换到RUN状态时,编译UCODE代码
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

#include "plc_sysblock.h"
#include "list_func.h"
#include "plc_instruction.h"
#include "plc_errormsg.h"
#include "plc_compiler.h"
#include "plc_complexins.h"
#include "plc_simpleins.h"
#include "plc_flowctrlins.h"
#include "plc_interrupt.h"
#include "kalyke_tool.h"
#include "bsp_iwdg.h"

/*正在编译指令类型*/
static unsigned char scv_CompilerInsFlag;
/*当前STL链表头指针*/
static plc_ins_list_head_st *stp_StlListPtr;
/*当前子程序链表头指针*/
static plc_ins_list_head_st *stp_SbrListPtr;
/*MC-MCR主控指令临时链表头指针*/
static plc_mc_mcr_ins_list_st *stap_McListPtr[MAX_MC_MCR_BLOCK_NUMBER] = {NULL, };

/**
  * @brief  移动UCODE指针位置
  * @param  None
  * @retval None
  */
static unsigned char plc_move_ucode_ptr_from_simple_ins(unsigned char **ppUcode)
{
    unsigned char *lcp_Ucode = *ppUcode;
    uint8_t ins = *(*ppUcode + 1);

    //LOGI("compiler", "%s: ins = 0x%X, lcp_Ucode = 0x%08X", __func__, ins, lcp_Ucode);
    //switch(*(lcp_Ucode+1)) {
    switch(ins) {
        /*  LD xxx  2Byte   */
        case SI_LD_X:
        case SI_LD_Y:
        case SI_LD_C:
        case SI_LD_T:
        case SI_LD_SM:
        case SI_LD_M:
        case SI_LD_S:
        case SI_LD_LM:
        /*  LDI xxx  2Byte   */
        case SI_LDI_X:
        case SI_LDI_Y:
        case SI_LDI_C:
        case SI_LDI_T:
        case SI_LDI_SM:
        case SI_LDI_M:
        case SI_LDI_S:
        case SI_LDI_LM:
        /*  AND xxx  2Byte   */
        case SI_AND_X:
        case SI_AND_Y:
        case SI_AND_C:
        case SI_AND_T:
        case SI_AND_SM:
        case SI_AND_M:
        case SI_AND_S:
        case SI_AND_LM:
        /*  ANI xxx  2Byte   */
        case SI_ANI_X:
        case SI_ANI_Y:
        case SI_ANI_C:
        case SI_ANI_T:
        case SI_ANI_SM:
        case SI_ANI_M:
        case SI_ANI_S:
        case SI_ANI_LM:
        /*  OR xxx  2Byte   */
        case SI_OR_X:
        case SI_OR_Y:
        case SI_OR_C:
        case SI_OR_T:
        case SI_OR_SM:
        case SI_OR_M:
        case SI_OR_S:
        case SI_OR_LM:
        /*  ORI xxx  2Byte   */
        case SI_ORI_X:
        case SI_ORI_Y:
        case SI_ORI_C:
        case SI_ORI_T:
        case SI_ORI_SM:
        case SI_ORI_M:
        case SI_ORI_S:
        case SI_ORI_LM:
        /* OUT xxx  2Byte   */
        case SI_OUT_Y:
        case SI_OUT_SM:
        case SI_OUT_M:
        case SI_OUT_LM:
        /* SET xxx  2Byte   */
        case SI_SET_Y:
        case SI_SET_SM:
        case SI_SET_M:
        case SI_SET_LM:
        /* RST xxx  2Byte   */
        case SI_RST_Y:
        case SI_RST_SM:
        case SI_RST_M:
        case SI_RST_LM:
        /* 不带需要带变量   2Byte   */
        case SI_INV:
        case SI_ANB:
        case SI_ORB:
        case SI_MPS:
        case SI_MRD:
        case SI_MPP:
        case SI_NOP:
            lcp_Ucode += 2;
            break;
        /*  LD xxx  4Byte   */
        case SI_LD_X_EXT:
        case SI_LD_Y_EXT:
        case SI_LD_C_EXT:
        case SI_LD_T_EXT:
        case SI_LD_SM_EXT:
        case SI_LD_M_EXT:
        case SI_LD_S_EXT:
        /*  LDI xxx  4Byte   */
        case SI_LDI_X_EXT:
        case SI_LDI_Y_EXT:
        case SI_LDI_C_EXT:
        case SI_LDI_T_EXT:
        case SI_LDI_SM_EXT:
        case SI_LDI_M_EXT:
        case SI_LDI_S_EXT:
        /*  AND xxx  4Byte   */
        case SI_AND_X_EXT:
        case SI_AND_Y_EXT:
        case SI_AND_C_EXT:
        case SI_AND_T_EXT:
        case SI_AND_SM_EXT:
        case SI_AND_M_EXT:
        case SI_AND_S_EXT:
        /*  ANI xxx  4Byte   */
        case SI_ANI_X_EXT:
        case SI_ANI_Y_EXT:
        case SI_ANI_C_EXT:
        case SI_ANI_T_EXT:
        case SI_ANI_SM_EXT:
        case SI_ANI_M_EXT:
        case SI_ANI_S_EXT:
        /*  OR xxx  4Byte   */
        case SI_OR_X_EXT:
        case SI_OR_Y_EXT:
        case SI_OR_C_EXT:
        case SI_OR_T_EXT:
        case SI_OR_SM_EXT:
        case SI_OR_M_EXT:
        case SI_OR_S_EXT:
        /*  ORI xxx  4Byte   */
        case SI_ORI_X_EXT:
        case SI_ORI_Y_EXT:
        case SI_ORI_C_EXT:
        case SI_ORI_T_EXT:
        case SI_ORI_SM_EXT:
        case SI_ORI_M_EXT:
        case SI_ORI_S_EXT:
        /*  OUT xxx  4Byte   */
        case SI_OUT_Y_EXT:
        case SI_OUT_SM_EXT:
        case SI_OUT_M_EXT:
        /*  SET xxx  4Byte   */
        case SI_SET_Y_EXT:
        case SI_SET_SM_EXT:
        case SI_SET_M_EXT:
        /*  RST xxx  4Byte   */
        case SI_RST_Y_EXT:
        case SI_RST_SM_EXT:
        case SI_RST_M_EXT:
        /* 不带需要带变量   4Byte   */
        case SI_EU:
        case SI_ED:
            lcp_Ucode += 4;
            break;
       /*  LDP xxx  6Byte   */
        case SI_LDP:
        case SI_LDF:
        case SI_ANDP:
        case SI_ANDF:
        case SI_ORP:
        case SI_ORF:
        case SI_PLP:
        case SI_PLF:
            lcp_Ucode += 6;
            break;

        default:
            return ERR_COMPILER;

    }

    *ppUcode = lcp_Ucode;
    return pdPASS;
}

/**
  * @brief  移动UCODE指针位置
  * @param  None
  * @retval None
  */
static unsigned char plc_move_ucode_ptr_from_complex_ins(unsigned char **ppUcode)
{
    unsigned char *lcp_Ucode;

    lcp_Ucode = *ppUcode;

    switch(GET_PU16_DATA(lcp_Ucode)) {
        case CI_FEND:
            lcp_Ucode +=2;
            break;
        case CI_CFEND:
            lcp_Ucode +=2;
            break;
        case CI_CJ:
            lcp_Ucode +=10;
            break;
        case CI_LBL:
            lcp_Ucode +=6;
            break;
        case CI_FOR:
            lcp_Ucode+=10;
            break;
        case CI_NEXT:
            lcp_Ucode +=2;
            break;
        case CI_TON:
            lcp_Ucode +=10;
            break;
        case CI_TOF:
            lcp_Ucode +=10;
            break;
        case CI_TONR:
            lcp_Ucode +=10;
            break;
        case CI_RST_T:
            lcp_Ucode +=6;
            break;
        case CI_RST_C:
            lcp_Ucode +=6;
            break;
        case CI_CTU:
            lcp_Ucode +=10;
            break;
        case CI_CTR:
            lcp_Ucode +=10;
            break;
        case CI_DCNT:
            lcp_Ucode +=14;
            break;
        case CI_IRET:
            lcp_Ucode +=2;
            break;
        case CI_CIRET:
            lcp_Ucode +=2;
            break;
        case CI_LD_LM:
            lcp_Ucode +=6;
            break;
        case CI_LDI_LM:
            lcp_Ucode +=6;
            break;
        case CI_AND_LM:
            lcp_Ucode +=6;
            break;
        case CI_ANI_LM:
            lcp_Ucode +=6;
            break;
        case CI_OR_LM:
            lcp_Ucode +=6;
            break;
        case CI_ORI_LM:
            lcp_Ucode +=6;
            break;
        case CI_SET_LM:
            lcp_Ucode +=6;
            break;
        case CI_RST_LM:
            lcp_Ucode +=6;
            break;
        case CI_OUT_LM:
            lcp_Ucode +=6;
            break;
        case CI_RST_S:
            lcp_Ucode +=6;
            break;
        case CI_SET_S:
            lcp_Ucode +=6;
            break;
        case CI_OUT_S:
            lcp_Ucode +=6;
            break;
        case CI_STOP:
            lcp_Ucode +=2;
            break;
        case CI_WDT:
            lcp_Ucode +=2;
            break;
        case CI_BITS:
            lcp_Ucode +=10;
            break;
        case CI_DBITS:
            lcp_Ucode +=12;
            break;
        case CI_BON:
            lcp_Ucode +=14;
            break;
        case CI_ZRST:
            lcp_Ucode +=10;
            break;
        case CI_ZSET:
            lcp_Ucode +=10;
            break;
        case CI_ENCO:
            lcp_Ucode +=10;
            break;
        case CI_DECO:
            lcp_Ucode +=10;
            break;
        case CI_EI:
            lcp_Ucode +=2;
            break;
        case CI_DI:
            lcp_Ucode +=2;
            break;
        case CI_MOV:
            lcp_Ucode +=10;
            break;
        case CI_DMOV:
            lcp_Ucode +=14;
            break;
        case CI_RMOV:
            lcp_Ucode +=14;
            break;
        case CI_BMOV:
            lcp_Ucode +=14;
            break;
        case CI_FMOV:
            lcp_Ucode +=14;
            break;
        case CI_DFMOV:
            lcp_Ucode +=18;
            break;
        case CI_SWAP:
            lcp_Ucode +=6;
            break;
        
        case CI_SWAPWORD:
            lcp_Ucode +=18;
            break;
        
        case CI_ECATSDORD:
            lcp_Ucode += 36;
            break;

        case CI_ECATSDOWR:
            lcp_Ucode += 36;
            break;

        case CI_XCH:
            lcp_Ucode +=10;
            break;
        case CI_DXCH:
            lcp_Ucode +=14;
            break;
        case CI_WSFR:
            lcp_Ucode +=18;
            break;
        case CI_WSFL:
            lcp_Ucode +=18;
            break;
        case CI_PUSH:
            lcp_Ucode +=14;
            break;
        case CI_FIFO:
            lcp_Ucode +=14;
            break;
        case CI_LIFO:
            lcp_Ucode +=14;
            break;
        case CI_ADD:
            lcp_Ucode +=14;
            break;
        case CI_SUB:
            lcp_Ucode +=14;
            break;
        case CI_MUL:
            lcp_Ucode +=16;
            break;
        case CI_DIV:
            lcp_Ucode +=14;
            break;
        case CI_SQT:
            lcp_Ucode +=10;
            break;
        case CI_INC:
            lcp_Ucode +=6;
            break;
        case CI_DEC:
            lcp_Ucode +=6;
            break;
        case CI_VABS:
            lcp_Ucode +=10;
            break;
        case CI_NEG:
            lcp_Ucode +=10;
            break;
        case CI_DADD:
            lcp_Ucode +=20;
            break;
        case CI_DSUB:
            lcp_Ucode +=20;
            break;
        case CI_DMUL:
            lcp_Ucode +=20;
            break;
        case CI_DDIV:
            lcp_Ucode +=20;
            break;
        case CI_DSQT:
            lcp_Ucode +=14;
            break;
        case CI_DINC:
            lcp_Ucode +=8;
            break;
        case CI_DDEC:
            lcp_Ucode +=8;
            break;
        case CI_DVABS:
            lcp_Ucode +=14;
            break;
        case CI_DNEG:
            lcp_Ucode +=14;
            break;
        case CI_RADD:
            lcp_Ucode +=20;
            break;
        case CI_RSUB:
            lcp_Ucode +=20;
            break;
        case CI_RMUL:
            lcp_Ucode +=20;
            break;
        case CI_RDIV:
            lcp_Ucode +=20;
            break;
        case CI_RSQT:
            lcp_Ucode +=14;
            break;
        case CI_RNEG:
            lcp_Ucode +=14;
            break;
        case CI_POWER:
            lcp_Ucode +=20;
            break;
        case CI_RVABS:
            lcp_Ucode +=14;
            break;
        case CI_SIN:
            lcp_Ucode +=14;
            break;
        case CI_COS:
            lcp_Ucode +=14;
            break;
        case CI_TAN:
            lcp_Ucode +=14;
            break;
        case CI_ASIN:
            lcp_Ucode +=14;
            break;
        case CI_ACOS:
            lcp_Ucode +=14;
            break;
        case CI_ATAN:
            lcp_Ucode +=14;
            break;
        case CI_LN:
            lcp_Ucode +=14;
            break;
        case CI_EXP:
            lcp_Ucode +=14;
            break;
        case CI_LOG:
            lcp_Ucode +=14;
            break;
        case CI_RAD:
            lcp_Ucode +=14;
            break;
        case CI_DEG:
            lcp_Ucode +=14;
            break;
        case CI_WAND:
            lcp_Ucode +=14;
            break;
        case CI_WOR:
            lcp_Ucode +=14;
            break;
        case CI_WXOR:
            lcp_Ucode +=14;
            break;
        case CI_WINV:
            lcp_Ucode +=10;
            break;
        case CI_DWAND:
            lcp_Ucode +=20;
            break;
        case CI_DWOR:
            lcp_Ucode +=20;
            break;
        case CI_DWXOR:
            lcp_Ucode +=20;
            break;
        case CI_DWINV:
            lcp_Ucode +=14;
            break;
        case CI_ROR:
            lcp_Ucode +=14;
            break;
        case CI_ROL:
            lcp_Ucode +=14;
            break;
        case CI_RCR:
            lcp_Ucode+=14;
            break;
        case CI_RCL:
            lcp_Ucode +=14;
            break;
        case CI_DROR:
            lcp_Ucode +=18;
            break;
        case CI_DROL:
            lcp_Ucode +=18;
            break;
        case CI_DRCR:
            lcp_Ucode +=18;
            break;
        case CI_DRCL:
            lcp_Ucode +=18;
            break;
        case CI_SHR:
            lcp_Ucode +=14;
            break;
        case CI_SHL:
            lcp_Ucode +=14;
            break;
        case CI_DSHR:
            lcp_Ucode +=18;
            break;
        case CI_DSHL:
            lcp_Ucode +=18;
            break;
        case CI_SFTR:
            lcp_Ucode +=18;
            break;
        case CI_SFTL:
            lcp_Ucode +=18;
            break;
        case CI_XMT:
            lcp_Ucode +=14;
            break;
        case CI_RCV:
            lcp_Ucode +=14;
            break;
        case CI_MODBUS:
            lcp_Ucode +=16;
            break;
        case CI_MODRW:
            lcp_Ucode +=28;
            break;
        case CI_MODLINK:
            lcp_Ucode +=20;
            break;
        case CI_MBCCONN:
            lcp_Ucode +=28;
            break;

        case CI_TCP_CONN:
            lcp_Ucode += 32;
            break;
        case CI_TCP_XMT:
            lcp_Ucode += 24;
            break;
        case CI_TCP_RCV:
            lcp_Ucode += 24;
            break;
        case CI_TCP_PING:
            lcp_Ucode += 20;
            break;

        case CI_MODTCPLINK:
            lcp_Ucode +=28;
            break;
        case CI_S7TCPLINK:
            lcp_Ucode +=28;
            break;
        case CI_EVFWD:
            lcp_Ucode +=12;
            break;
        case CI_EVREV:
            lcp_Ucode +=12;
            break;
        case CI_EVDFWD:
            lcp_Ucode +=12;
            break;
        case CI_EVDREV:
            lcp_Ucode +=12;
            break;
        case CI_EVSTOP:
            lcp_Ucode +=16;
            break;
        case CI_EVFRQ:
            lcp_Ucode +=16;
            break;
        case CI_EVWRT:
            lcp_Ucode +=20;
            break;
        case CI_EVRDST:
            lcp_Ucode +=20;
            break;
        case CI_EVRD:
            lcp_Ucode +=20;
            break;
        case CI_CANXMT:
            lcp_Ucode += 26;
            break;
        case CI_CANRCV:
            lcp_Ucode += 26;
            break;
        case CI_CANMXMT:
            lcp_Ucode += 20;
            break;
        case CI_LDE:
            lcp_Ucode +=10;
            break;
        case CI_LDG:
            lcp_Ucode +=10;
            break;
        case CI_LDL:
            lcp_Ucode +=10;
            break;
        case CI_LDNE:
            lcp_Ucode +=10;
            break;
        case CI_LDLE:
            lcp_Ucode +=10;
            break;
        case CI_LDGE:
            lcp_Ucode +=10;
            break;
        case CI_ANDG:
            lcp_Ucode +=10;
            break;
        case CI_ANDL:
            lcp_Ucode +=10;
            break;
        case CI_ANDNE:
            lcp_Ucode +=10;
            break;
        case CI_ANDLE:
            lcp_Ucode +=10;
            break;
        case CI_ANDE:
            lcp_Ucode +=10;
            break;
        case CI_ANDGE:
            lcp_Ucode +=10;
            break;
        case CI_ORE:
            lcp_Ucode +=10;
            break;
        case CI_ORL:
            lcp_Ucode +=10;
            break;
        case CI_ORG:
            lcp_Ucode +=10;
            break;
        case CI_ORNE:
            lcp_Ucode +=10;
            break;
        case CI_ORLE:
            lcp_Ucode +=10;
            break;
        case CI_ORGE:
            lcp_Ucode +=10;
            break;
        case CI_LDDE:
            lcp_Ucode +=14;
            break;
        case CI_LDDG:
            lcp_Ucode +=14;
            break;
        case CI_LDDL:
            lcp_Ucode +=14;
            break;
        case CI_LDDNE:
            lcp_Ucode +=14;
            break;
        case CI_LDDLE:
            lcp_Ucode +=14;
            break;
        case CI_LDDGE:
            lcp_Ucode +=14;
            break;
        case CI_ANDDE:
            lcp_Ucode +=14;
            break;
        case CI_ANDDG:
            lcp_Ucode +=14;
            break;
        case CI_ANDDL:
            lcp_Ucode +=14;
            break;
        case CI_ANDDNE:
            lcp_Ucode +=14;
            break;
        case CI_ANDDLE:
            lcp_Ucode +=14;
            break;
        case CI_ANDDGE:
            lcp_Ucode +=14;
            break;
        case CI_ORDE:
            lcp_Ucode +=14;
            break;
        case CI_ORDG:
            lcp_Ucode +=14;
            break;
        case CI_ORDL:
            lcp_Ucode +=14;
            break;
        case CI_ORDNE:
            lcp_Ucode +=14;
            break;
        case CI_ORDLE:
            lcp_Ucode +=14;
            break;
        case CI_ORDGE:
            lcp_Ucode +=14;
            break;
        case CI_LDRE:
            lcp_Ucode +=14;
            break;
        case CI_LDRG:
            lcp_Ucode +=14;
            break;
        case CI_LDRL:
            lcp_Ucode +=14;
            break;
        case CI_LDRNE:
            lcp_Ucode +=14;
            break;
        case CI_LDRLE:
            lcp_Ucode +=14;
            break;
        case CI_LDRGE:
            lcp_Ucode +=14;
            break;
        case CI_ANDRE:
            lcp_Ucode +=14;
            break;
        case CI_ANDRG:
            lcp_Ucode +=14;
            break;
        case CI_ANDRL:
            lcp_Ucode +=14;
            break;
        case CI_ANDRNE:
            lcp_Ucode +=14;
            break;
        case CI_ANDRLE:
            lcp_Ucode +=14;
            break;
        case CI_ANDRGE:
            lcp_Ucode +=14;
            break;
        case CI_ORRE:
            lcp_Ucode +=14;
            break;
        case CI_ORRG:
            lcp_Ucode +=14;
            break;
        case CI_ORRL:
            lcp_Ucode +=14;
            break;
        case CI_ORRNE:
            lcp_Ucode +=14;
            break;
        case CI_ORRLE:
            lcp_Ucode +=14;
            break;
        case CI_ORRGE:
            lcp_Ucode +=14;
            break;
        case CI_BCD:
            lcp_Ucode +=10;
            break;
        case CI_DBCD:
            lcp_Ucode +=14;
            break;
        case CI_BIN:
            lcp_Ucode +=10;
            break;
        case CI_DBIN:
            lcp_Ucode +=14;
            break;
        case CI_GRY:
            lcp_Ucode +=10;
            break;
        case CI_DGRY:
            lcp_Ucode +=14;
            break;
        case CI_GBIN:
            lcp_Ucode +=10;
            break;
        case CI_DGBIN:
            lcp_Ucode +=14;
            break;
        case CI_FLT:
            lcp_Ucode +=12;
            break;
        case CI_DFLT:
            lcp_Ucode +=14;
            break;
        case CI_INT:
            lcp_Ucode +=12;
            break;
        case CI_DINT:
            lcp_Ucode +=14;
            break;
        case CI_SEG:
            lcp_Ucode +=10;
            break;
        case CI_BLD:
            lcp_Ucode +=10;
            break;
        case CI_BLDI:
            lcp_Ucode +=10;
            break;
        case CI_BAND:
            lcp_Ucode +=10;
            break;
        case CI_BANDI:
            lcp_Ucode +=10;
            break;
        case CI_BOR:
            lcp_Ucode +=10;
            break;
        case CI_BORI:
            lcp_Ucode +=10;
            break;
        case CI_BSET:
            lcp_Ucode +=10;
            break;
        case CI_BRST:
            lcp_Ucode +=10;
            break;
        case CI_BOUT:
            lcp_Ucode +=10;
            break;
        case CI_ITD:
            lcp_Ucode +=12;
            break;
        case CI_DTI:
            lcp_Ucode +=12;
            break;
        case CI_PID:
            lcp_Ucode +=18;
            break;
        case CI_SUM:
            lcp_Ucode +=16;
            break;
        case CI_DSUM:
            lcp_Ucode +=18;
            break;
        case CI_RSUM:
            lcp_Ucode +=18;
            break;
        case CI_DCMPE:
            lcp_Ucode +=14;
            break;
        case CI_DCMPG:
            lcp_Ucode +=14;
            break;
        case CI_DCMPL:
            lcp_Ucode +=14;
            break;
        case CI_DCMPNE:
            lcp_Ucode +=14;
            break;
        case CI_DCMPLE:
            lcp_Ucode +=14;
            break;
        case CI_DCMPGE:
            lcp_Ucode +=14;
            break;
        case CI_TCMPE:
            lcp_Ucode +=14;
            break;
        case CI_TCMPG:
            lcp_Ucode +=14;
            break;
        case CI_TCMPL:
            lcp_Ucode +=14;
            break;
        case CI_TCMPNE:
            lcp_Ucode +=14;
            break;
        case CI_TCMPLE:
            lcp_Ucode +=14;
            break;
        case CI_TCMPGE:
            lcp_Ucode +=14;
            break;
        case CI_RAMP:
            lcp_Ucode +=24;
            break;
        case CI_HACKLE:
            lcp_Ucode +=24;
            break;
        case CI_TRIANGLE:
            lcp_Ucode +=24;
            break;
        case CI_TMON:
            lcp_Ucode +=12;
            break;
        case CI_CCITT:
            lcp_Ucode +=14;
            break;
        case CI_CRC16:
            lcp_Ucode +=14;
            break;
        case CI_LRC:
            lcp_Ucode +=14;
            break;
        case CI_FROM:
            lcp_Ucode +=18;
            break;
        case CI_TO:
            lcp_Ucode +=18;
            break;
        case CI_DFROM:
            lcp_Ucode +=20;
            break;
        case CI_DTO:
            lcp_Ucode +=20;
            break;
        case CI_EROMWR:
            lcp_Ucode +=12;
            break;
        case CI_REF:
            lcp_Ucode +=10;
            break;
        case CI_REFF:
            lcp_Ucode +=6;
            break;
        case CI_TADD:
            lcp_Ucode +=14;
            break;
        case CI_TSUB:
            lcp_Ucode +=14;
            break;
        case CI_TRD:
            lcp_Ucode +=6;
            break;
        case CI_TWR:
            lcp_Ucode +=6;
            break;
        case CI_DHSCS:
            lcp_Ucode +=20;
            break;
        case CI_DHSPI:
            lcp_Ucode +=20;
            break;
        case CI_DHSCI:
            lcp_Ucode +=20;
            break;
        case CI_DHSCR:
            lcp_Ucode +=20;
            break;
        case CI_DHSZ:
            lcp_Ucode +=26;
            break;
        case CI_DHST:
            lcp_Ucode +=20;
            break;
        case CI_DHSP:
            lcp_Ucode +=20;
            break;
        case CI_PLSY:
            /*只有在使用此指令时，才初始化相关配置*/
            bsp_tim3_init();
            lcp_Ucode +=18;
            break;
        case CI_PLSR:
            lcp_Ucode +=20;
            break;
        case CI_PWM:
            lcp_Ucode +=14;
            break;
        case CI_SPD:
            lcp_Ucode +=14;
            break;
        case CI_HCNT:
            lcp_Ucode +=14;
            break;
        case CI_VRRD:
            lcp_Ucode +=10;
            break;
        case CI_HOUR:
            lcp_Ucode +=16;
            break;
        case CI_ATI:
            lcp_Ucode +=14;
            break;
        case CI_ITA:
            lcp_Ucode +=14;
            break;
        case CI_LCNV:
            lcp_Ucode +=18;
            break;
        case CI_RLCNV:
            lcp_Ucode +=24;
            break;
        case CI_ASC:
            lcp_Ucode +=38;
            break;
        case CI_ABS:
            lcp_Ucode +=16;
            break;
        case CI_ZRN:
            lcp_Ucode +=24;
            break;
        case CI_PLSV:
            lcp_Ucode +=16;
            break;
        case CI_DRVI:
            lcp_Ucode +=24;
            break;
        case CI_DRVC:
            lcp_Ucode +=20;
            break;
        case CI_DRVA:
            lcp_Ucode +=24;
            break;
        case CI_PLS:
            lcp_Ucode +=14;
            break;
        case CI_PLSB:
            lcp_Ucode +=24;
            break;
        case CI_TKY:
            lcp_Ucode +=14;
            break;
        case CI_PR:
            lcp_Ucode +=10;
            break;
        case CI_LD_DX_Y:
            lcp_Ucode +=8;
            break;
        case CI_LDI_DX_Y:
            lcp_Ucode +=8;
            break;
        case CI_AND_DX_Y:
            lcp_Ucode +=8;
            break;
        case CI_ANI_DX_Y:
            lcp_Ucode +=8;
            break;
        case CI_OR_DX_Y:
            lcp_Ucode +=8;
            break;
        case CI_ORI_DX_Y:
            lcp_Ucode +=8;
            break;
        case CI_SET_DX_Y:
            lcp_Ucode +=8;
            break;
        case CI_RST_DX_Y:
            lcp_Ucode +=8;
            break;
        case CI_OUT_DX_Y:
            lcp_Ucode +=8;
            break;
        case CI_MEAN:
            lcp_Ucode +=14;
            break;
        case CI_WTOB:
            lcp_Ucode +=14;
            break;
        case CI_BTOW:
            lcp_Ucode +=14;
            break;
        case CI_DIS:
            lcp_Ucode +=14;
            break;
        case CI_UNI:
            lcp_Ucode +=14;
            break;
        case CI_ANS:
            lcp_Ucode +=14;
            break;
        case CI_ANR:
            lcp_Ucode +=2;
            break;
        case CI_BKADD:
            lcp_Ucode +=18;
            break;
        case CI_BKSUB:
            lcp_Ucode +=18;
            break;
        case CI_BKCMPDE:
            lcp_Ucode +=18;
            break;
        case CI_BKCMPDG:
            lcp_Ucode +=18;
            break;
        case CI_BKCMPDL:
            lcp_Ucode +=18;
            break;
        case CI_BKCMPDNE:
            lcp_Ucode +=18;
            break;
        case CI_BKCMPDLE:
            lcp_Ucode +=18;
            break;
        case CI_BKCMPDGE:
            lcp_Ucode +=18;
            break;
        case CI_RND:
            lcp_Ucode +=6;
            break;
        case CI_DUTY:
            lcp_Ucode +=14;
            break;
        case CI_HTOS:
            lcp_Ucode +=10;
            break;
        case CI_STOH:
            lcp_Ucode +=10;
            break;
        case CI_LIMIT:
            lcp_Ucode +=18;
            break;
        case CI_DBAND:
            lcp_Ucode +=18;
            break;
        case CI_ZONE:
            lcp_Ucode +=18;
            break;
        case CI_SCL:
            lcp_Ucode +=14;
            break;
        case CI_SER:
            lcp_Ucode +=18;
            break;
        case CI_STRLEN:
            lcp_Ucode +=10;
            break;
        case CI_STRRIGHT:
            lcp_Ucode +=14;
            break;
        case CI_STRLEFT:
            lcp_Ucode +=14;
            break;
        case CI_STRMIDR:
            lcp_Ucode +=14;
            break;
        case CI_STRMIDW:
            lcp_Ucode +=14;
            break;
        case CI_CMP:
            lcp_Ucode +=14;
            break;
        case CI_LCMP:
            lcp_Ucode +=18;
            break;
        case CI_RCMP:
            lcp_Ucode +=18;
            break;
        case CI_ALT:
            lcp_Ucode +=6;
            break;
        case CI_ABSD:
            lcp_Ucode +=18;
            break;
        case CI_DABSD:
            lcp_Ucode +=22;
            break;
        case CI_LOADR:
            lcp_Ucode +=10;
            break;
        case CI_SAVER:
            lcp_Ucode +=14;
            break;
        case CI_INITR:
            lcp_Ucode +=10;
            break;
        case CI_LOGR:
            lcp_Ucode +=22;
            break;
        case CI_INITER:
            lcp_Ucode +=10;
            break;
        case CI_LIN:
            lcp_Ucode +=24;
            break;
        case CI_CW:
            lcp_Ucode +=24;
            break;
        case CI_CCW:
            lcp_Ucode +=24;
            break;
        case CI_DRV:
            lcp_Ucode +=26;
            break;
        case CI_DSZR:
            lcp_Ucode +=18;
            break;
        case CI_MOVELINK:
            lcp_Ucode +=34;
            break;
        case CI_STOPDV:
            lcp_Ucode +=24;
            break;
        case CI_GEARBOX:
            lcp_Ucode +=18;
            break;
        case CI_CAMBOX:
            lcp_Ucode +=22;
            break;
        case CI_CAMTABLE:
            lcp_Ucode +=22;
            break;

        /*编译时需要特殊处理指令*/
        case CI_STRADD:
            if( (*(lcp_Ucode +2)) ==0x82) {
                lcp_Ucode +=((*(lcp_Ucode+3)) +4);
            } else {
                lcp_Ucode +=6;
            }
            if((*(lcp_Ucode)) ==0x82) {
                lcp_Ucode +=(*(lcp_Ucode+1)) +6;
            } else {
                lcp_Ucode +=8;
            }
            break;

        case CI_STRINSTR:
            if(GET_PU8_DATA(lcp_Ucode+2) == 0x82) {
                lcp_Ucode += (GET_PU8_DATA(lcp_Ucode+3) + 16);

            } else {
                lcp_Ucode += 18;

            }
            break;

        case CI_STRMOV:
            if(GET_PU8_DATA(lcp_Ucode+3) == 0x82) {
                lcp_Ucode += (GET_PU8_DATA(lcp_Ucode+2) + 8);
            } else {
                lcp_Ucode += 10;
            }
            break;

        case CI_CALL:
            gtp_CallInsInfoPtr->mlv_UCodeAddr[gtp_CallInsInfoPtr->msv_UseNum] = GET_POINT_ADDR(lcp_Ucode);
            gtp_CallInsInfoPtr->msv_UseNum ++;
            lcp_Ucode += plc_call_instruction_length(lcp_Ucode);
            break;

        case CI_SBR: {
            unsigned short lsv_Temp;
            lsv_Temp = GET_PU16_DATA(lcp_Ucode+2);
            if(((lsv_Temp >= 0x100)&&(lsv_Temp <= 0x1FF)) ||
               ((lsv_Temp >= 0x400)&&(lsv_Temp <= 0x4FF))) {
                lsv_Temp &= 0xFF;
            } else {
                return ERR_USER_PROGRAM;
            }

            /*保存子程序入口地址*/
            gtp_CallInsInfoPtr->mcp_SbrPc[lsv_Temp] = lcp_Ucode;
            /*设置进入子程序标志*/
            scv_CompilerInsFlag = COMPILER_SUB_INS;
            /*取对应子程序清除链表头指针*/
            stp_SbrListPtr = &gtp_SbrListPtr[lsv_Temp];
            INIT_LIST_HEAD(&stp_SbrListPtr->mtv_SimpInsHead);
            INIT_LIST_HEAD(&stp_SbrListPtr->mtv_CompInsHead);
            lcp_Ucode += 4;
        }
        break;

        case CI_SRET:
            /*设置进入普通程序标志*/
            scv_CompilerInsFlag = COMPILER_NORMAL_INS;
            stp_SbrListPtr = NULL;
            lcp_Ucode += 2;
            break;

        case CI_CSRET:
            lcp_Ucode += 2;
            break;
        
        case CI_INTR: {
            unsigned short lsv_Temp;
            lsv_Temp = GET_PU16_DATA(lcp_Ucode+2);
            if((lsv_Temp >= 0x200)&&(lsv_Temp <= 0x2FF)) {
                lsv_Temp &= 0xFF;
            } else {
                return ERR_USER_PROGRAM;
            }
            gtp_InterruptInfo->mtv_IntSource[lsv_Temp].mcp_IntPara = lcp_Ucode;
            gtp_InterruptInfo->mtv_IntSource[lsv_Temp].mcv_IntFlag |= 0x02;
        }
        lcp_Ucode += 4;
        break;

        case CI_MC: {
            struct list_head *ltp_Head;
            /*设置进入MC-MCR语句块标志*/
            scv_CompilerInsFlag = COMPILER_MC_MCR_INS;

            ltp_Head = &gtv_McMcrBlockInfo.mtv_McMcrHead;

            stap_McListPtr[gtv_McMcrBlockInfo.mcv_NestedLevel] = (plc_mc_mcr_ins_list_st *)pvPortMalloc(sizeof(plc_mc_mcr_ins_list_st));
            if(!stap_McListPtr[gtv_McMcrBlockInfo.mcv_NestedLevel]) {
                return ERR_NO_FREE_MEMORY;
            }

            /*保存MC-MCR入口地址*/
            stap_McListPtr[gtv_McMcrBlockInfo.mcv_NestedLevel]->mcp_McUCodePc = lcp_Ucode;

            list_add_tail(&stap_McListPtr[gtv_McMcrBlockInfo.mcv_NestedLevel]->mtv_List, ltp_Head);

            INIT_LIST_HEAD(&stap_McListPtr[gtv_McMcrBlockInfo.mcv_NestedLevel]->mtv_SimpInsHead);
            INIT_LIST_HEAD(&stap_McListPtr[gtv_McMcrBlockInfo.mcv_NestedLevel]->mtv_CompInsHead);
            gtv_McMcrBlockInfo.mcv_NestedLevel++;
            lcp_Ucode += 10;
        }
        break;

        case CI_MCR:
            if(gtv_McMcrBlockInfo.mcv_NestedLevel > 0) {
                gtv_McMcrBlockInfo.mcv_NestedLevel--;

            }
            if(gtv_McMcrBlockInfo.mcv_NestedLevel == 0) {
                /*设置进入普通程序标志*/
                scv_CompilerInsFlag = COMPILER_NORMAL_INS;
            }

            lcp_Ucode += 6;
            break;

        case CI_STL: {
            unsigned char lcv_Flag = 0;
            unsigned short lsv_Temp;
            /*搜索合并分支*/
            while(1) {
                lcp_Ucode += 10;
                if(GET_PU16_DATA(lcp_Ucode) != CI_STL) {
                    break;
                }
                lcv_Flag = 1;
            }

            /*合并分支*/
            if(lcv_Flag) {
                /*设置进入普通程序标志*/
                scv_CompilerInsFlag = COMPILER_NORMAL_INS;

                stp_StlListPtr = NULL;
            } else {
                /*设置进入STL标志*/
                scv_CompilerInsFlag = COMPILER_STL_INS;

                /*获取S元件号*/
                lsv_Temp = GET_PU16_DATA((*ppUcode)+4);

                stp_StlListPtr = &gtp_StlListPtr[lsv_Temp];
                INIT_LIST_HEAD(&stp_StlListPtr->mtv_SimpInsHead);
                INIT_LIST_HEAD(&stp_StlListPtr->mtv_CompInsHead);
            }
        }
        break;

        case CI_RET:
            /*设置进入普通程序标志*/
            scv_CompilerInsFlag = COMPILER_NORMAL_INS;
            stp_StlListPtr = NULL;
            lcp_Ucode += 2;
            break;

        /*编译到未定义的指令，终止编译退出*/
        default:
            return ERR_COMPILER;

    }

    *ppUcode = lcp_Ucode;
    return pdPASS;
}

/**
  * @brief  添加指令到简单指令列表
  * @param  None
  * @retval None
  */
static unsigned char plc_add_ins_to_simple_list(unsigned char *pUcode)
{
    plc_ins_list_st *ltp_PlcIns = NULL;
    unsigned char i;

    switch(scv_CompilerInsFlag) {
        case COMPILER_STL_INS:
            switch(*(pUcode+1)) {
                case SI_OUT_Y:
                case SI_OUT_SM:
                case SI_OUT_M:
                case SI_OUT_Y_EXT:
                case SI_OUT_SM_EXT:
                case SI_OUT_M_EXT:
                case SI_EU:
                case SI_ED:
                case SI_LDP:
                case SI_LDF:
                case SI_ANDP:
                case SI_ANDF:
                case SI_ORP:
                case SI_ORF:
                case SI_PLP:
                case SI_PLF:

                    if(stp_StlListPtr) {
                        ltp_PlcIns = (plc_ins_list_st *)pvPortMalloc(sizeof(plc_ins_list_st));
                        if(!ltp_PlcIns) {
                            return ERR_NO_FREE_MEMORY;
                        }

                        ltp_PlcIns->mlp_InsPc = pUcode;
                        /*插入链表*/
                        list_add_tail(&ltp_PlcIns->mtv_InsList, &stp_StlListPtr->mtv_SimpInsHead);
                    }
                    break;
            }
            break;

        case COMPILER_MC_MCR_INS:
            switch(*(pUcode+1)) {
                case SI_OUT_Y:
                case SI_OUT_SM:
                case SI_OUT_M:
                case SI_OUT_Y_EXT:
                case SI_OUT_SM_EXT:
                case SI_OUT_M_EXT:
                case SI_EU:
                case SI_ED:
                case SI_LDP:
                case SI_LDF:
                case SI_ANDP:
                case SI_ANDF:
                case SI_ORP:
                case SI_ORF:
                case SI_PLP:
                case SI_PLF:
                    if(gtv_McMcrBlockInfo.mcv_NestedLevel > 0) {
                        for(i=0; i<gtv_McMcrBlockInfo.mcv_NestedLevel; i++) {
                            ltp_PlcIns = (plc_ins_list_st *)pvPortMalloc(sizeof(plc_ins_list_st));
                            if(!ltp_PlcIns) {
                                return ERR_NO_FREE_MEMORY;
                            }

                            ltp_PlcIns->mlp_InsPc = pUcode;
                            /*插入链表*/
                            list_add_tail(&ltp_PlcIns->mtv_InsList, &stap_McListPtr[i]->mtv_SimpInsHead);
                        }
                    }
                    break;
            }

            break;

        case COMPILER_SUB_INS:
            switch(*(pUcode+1)) {
                case SI_EU:
                case SI_ED:
                case SI_LDP:
                case SI_LDF:
                case SI_ANDP:
                case SI_ANDF:
                case SI_ORP:
                case SI_ORF:
                case SI_PLP:
                case SI_PLF:
                    if(stp_SbrListPtr) {
                        ltp_PlcIns = (plc_ins_list_st *)pvPortMalloc(sizeof(plc_ins_list_st));
                        if(!ltp_PlcIns) {
                            return ERR_NO_FREE_MEMORY;
                        }

                        ltp_PlcIns->mlp_InsPc = pUcode;
                        /*插入链表*/
                        list_add_tail(&ltp_PlcIns->mtv_InsList, &stp_SbrListPtr->mtv_SimpInsHead);
                    }
                    break;
            }
            break;

        default:
            break;
    }

    return pdPASS;
}

/**
  * @brief  添加指令到复杂指令列表
  * @param  None
  * @retval None
  */
static unsigned char plc_add_ins_to_complex_list(unsigned char *pUcode)
{
    unsigned char i;
    plc_ins_list_st *ltp_PlcIns = NULL;

    switch(scv_CompilerInsFlag) {
        case COMPILER_STL_INS:
            switch(GET_PU16_DATA(pUcode)) {
                case CI_TON:
                case CI_TONR:
                case CI_TOF:
                case CI_OUT_LM:
                case CI_BOUT:
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
                case CI_CALL:
                    if(stp_StlListPtr) {
                        ltp_PlcIns = (plc_ins_list_st *)pvPortMalloc(sizeof(plc_ins_list_st));
                        if(!ltp_PlcIns) {
                            return ERR_NO_FREE_MEMORY;
                        }

                        ltp_PlcIns->mlp_InsPc = pUcode;
                        /*插入链表*/
                        list_add_tail(&ltp_PlcIns->mtv_InsList, &stp_StlListPtr->mtv_CompInsHead);
                    }

                    break;
            }
            break;

        case COMPILER_MC_MCR_INS:
            switch(GET_PU16_DATA(pUcode)) {
                case CI_TON:
                case CI_TONR:
                case CI_TOF:
                case CI_OUT_LM:
                case CI_BOUT:
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
                case CI_CALL:
                    if(gtv_McMcrBlockInfo.mcv_NestedLevel > 0) {
                        for(i=0; i<gtv_McMcrBlockInfo.mcv_NestedLevel; i++) {
                            ltp_PlcIns = (plc_ins_list_st *)pvPortMalloc(sizeof(plc_ins_list_st));
                            if(!ltp_PlcIns) {
                                return ERR_NO_FREE_MEMORY;
                            }

                            ltp_PlcIns->mlp_InsPc = pUcode;
                            /*插入链表*/
                            list_add_tail(&ltp_PlcIns->mtv_InsList, &stap_McListPtr[i]->mtv_CompInsHead);
                        }
                    }
                    break;
            }
            break;

        case COMPILER_SUB_INS:
            switch(GET_PU16_DATA(pUcode)) {
                case CI_TON:
                case CI_TONR:
                case CI_TOF:
                case CI_OUT_LM:
                case CI_BOUT:
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
                case CI_CALL:
                    if(stp_SbrListPtr) {
                        ltp_PlcIns = (plc_ins_list_st *)pvPortMalloc(sizeof(plc_ins_list_st));
                        if(!ltp_PlcIns) {
                            return ERR_NO_FREE_MEMORY;
                        }

                        ltp_PlcIns->mlp_InsPc = pUcode;
                        /*插入链表*/
                        list_add_tail(&ltp_PlcIns->mtv_InsList, &stp_SbrListPtr->mtv_CompInsHead);
                    }
                    break;
            }
            break;

        default:
            break;
    }
    return pdPASS;
}

/**
  * @brief  编译UCODE代码
  * @param  None
  * @retval None
  */
unsigned char plc_compiler_ucode(unsigned char *lcv_UCodePtr)
{
    PRINTF("Enter %s(), lcv_UCodePtr = 0x%08X\r\n", __func__, lcv_UCodePtr);
    unsigned short i;
    struct list_head *ltp_ForCur;
    struct list_head *ltp_ForNext;
    struct list_head *ltp_Head;
    struct list_head *ltp_McHead;
    struct list_head *ltp_ListCur;
    struct list_head *ltp_ListNext;
    plc_ins_list_st *ltp_PlcIns;
    plc_mc_mcr_ins_list_st *ltp_McMcrIns;

    unsigned char *lcp_UCodePtr;
    unsigned char *lcp_UCodeEndPtr;
    unsigned long llv_UCodeLen;

    unsigned char lcv_RetValue = pdPASS;

    /*CALL 指令信息结构体初始化*/
    gtp_CallInsInfoPtr->msv_UseNum = 0;

    for(i=0; i<MAX_CALL_INS_USE_NUMBER; i++) {
        gtp_CallInsInfoPtr->mlv_UCodeAddr[i] = 0;
        gtp_CallInsInfoPtr->mcv_IsExec[i] = 0;
    }

    for(i=0; i<MAX_SBR_COUNT; i++) {
        gtp_CallInsInfoPtr->mcp_SbrPc[i] = 0;
    }

    for(i=0; i<MAX_SBR_NESTED_LAYER; i++) {
        gtp_CallInsInfoPtr->mcp_RetPc[i] = 0;
    }

    /*删除子程序的清除指令列表项,释放内存*/
    for(i=0; i<MAX_SBR_COUNT; i++) {

        ltp_Head = &gtp_SbrListPtr[i].mtv_SimpInsHead;
        list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
            ltp_PlcIns = list_entry(ltp_ForCur, plc_ins_list_st, mtv_InsList);
            list_del(&ltp_PlcIns->mtv_InsList);
            vPortFree(ltp_PlcIns);
        }

        ltp_Head = &gtp_SbrListPtr[i].mtv_CompInsHead;
        list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
            ltp_PlcIns = list_entry(ltp_ForCur, plc_ins_list_st, mtv_InsList);
            list_del(&ltp_PlcIns->mtv_InsList);
            vPortFree(ltp_PlcIns);
        }
    }

    /*删除STL指令列表,并释放内存*/
    for(i=0; i<S_RANG; i++) {

        ltp_Head = &gtp_StlListPtr[i].mtv_SimpInsHead;
        list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
            ltp_PlcIns = list_entry(ltp_ForCur, plc_ins_list_st, mtv_InsList);
            list_del(&ltp_PlcIns->mtv_InsList);
            vPortFree(ltp_PlcIns);
        }

        ltp_Head = &gtp_StlListPtr[i].mtv_CompInsHead;
        list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
            ltp_PlcIns = list_entry(ltp_ForCur, plc_ins_list_st, mtv_InsList);
            list_del(&ltp_PlcIns->mtv_InsList);
            vPortFree(ltp_PlcIns);
        }
    }

    /*删除MC-MCR指令列表项,并释放内存*/
    gtv_McMcrBlockInfo.mcv_NestedLevel = 0;

    ltp_McHead = &gtv_McMcrBlockInfo.mtv_McMcrHead;
    list_for_each_safe(ltp_ListCur, ltp_ListNext, ltp_McHead) {
        ltp_McMcrIns = list_entry(ltp_ListCur, plc_mc_mcr_ins_list_st, mtv_List);

        ltp_Head = &ltp_McMcrIns->mtv_SimpInsHead;
        list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
            ltp_PlcIns = list_entry(ltp_ForCur, plc_ins_list_st, mtv_InsList);
            list_del(&ltp_PlcIns->mtv_InsList);
            vPortFree(ltp_PlcIns);
        }

        ltp_Head = &ltp_McMcrIns->mtv_CompInsHead;
        list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
            ltp_PlcIns = list_entry(ltp_ForCur, plc_ins_list_st, mtv_InsList);
            list_del(&ltp_PlcIns->mtv_InsList);
            vPortFree(ltp_PlcIns);
        }

        list_del(&ltp_McMcrIns->mtv_List);
        vPortFree(ltp_McMcrIns);
    }

    /*UCODE有效指令开始地址*/
    lcp_UCodePtr = lcv_UCodePtr + 26 + 2;
    llv_UCodeLen =  plc_get_file_length(lcv_UCodePtr + FILE_LEN_INFO_START_INDEX, 4);
#if (LOG_OPEN == 1)
    if (llv_UCodeLen < 1024)
    {
        hexdump(lcv_UCodePtr, llv_UCodeLen);
    }
    else
    {
        hexdump(lcv_UCodePtr, 1024);
    }
#endif
    /*UCODE最大指令地址,用于判断越界*/
    lcp_UCodeEndPtr = lcv_UCodePtr + llv_UCodeLen - 4;
    LOGV("plc_compiler", "lcp_UCodePtr = 0x%08X, lcp_UCodeEndPtr = 0x%08X, llv_UCodeLen = %u", lcp_UCodePtr, lcp_UCodeEndPtr, llv_UCodeLen);

#if (LOG_OPEN == 1)
    uint32_t theLen = lcp_UCodeEndPtr - lcp_UCodePtr;
    LOGD("plc_compiler", "theLen = %u, The pure ucode is :", theLen);
    if (theLen < 1024)
    {
        hexdump(lcp_UCodePtr, theLen);
    }
    else
    {
        hexdump(lcp_UCodePtr, 1024);
    }
#endif

    /*默认普通指令*/
    scv_CompilerInsFlag = COMPILER_NORMAL_INS;    
    while(lcp_UCodePtr < lcp_UCodeEndPtr) {
        bsp_feed_watch_dog();
        if(*(lcp_UCodePtr + 1) < SI_MAX_INS) {
            /* LOGI("compiler", "Current simple instructions is 0x%X", *(lcp_UCodePtr + 1)); */
            /*简单指令*/
            lcv_RetValue = plc_add_ins_to_simple_list(lcp_UCodePtr);
            if(lcv_RetValue != pdPASS) {
                LOGE("compiler", "ERROR : 000, lcv_RetValue=%d, instructions: 0x%X", lcv_RetValue, *(lcp_UCodePtr + 1));
                goto COMPILER_UCODE_ERR;
            }

            lcv_RetValue = plc_move_ucode_ptr_from_simple_ins(&lcp_UCodePtr);
            if(lcv_RetValue != pdPASS) {
                LOGE("compiler", "ERROR : 001, lcv_RetValue=%d, instructions: 0x%X", lcv_RetValue, *(lcp_UCodePtr + 1));
                goto COMPILER_UCODE_ERR;
            }


        } else {
            /* LOGI("compiler", "Current complex instructions is 0x%X", *((unsigned short*)lcp_UCodePtr)); */
            /*复杂指令*/
            lcv_RetValue = plc_add_ins_to_complex_list(lcp_UCodePtr);
            if(lcv_RetValue != pdPASS) {
                LOGE("compiler", "ERROR : 002, lcv_RetValue=%d\r\n", lcv_RetValue);
                goto COMPILER_UCODE_ERR;
            }

            lcv_RetValue = plc_move_ucode_ptr_from_complex_ins(&lcp_UCodePtr);
            if(lcv_RetValue != pdPASS) {
                /*UCODE文件结束*/
                if(GET_PU16_DATA(lcp_UCodePtr) == CI_END) {
                    break;
                }
                LOGE("compiler", "ERROR : 003, lcv_RetValue=%d\r\n", lcv_RetValue);
                goto COMPILER_UCODE_ERR;
            }

        }

        if(lcp_UCodePtr > lcp_UCodeEndPtr) {
            LOGE("compiler", "ERROR : 004, lcv_RetValue=%d\r\n", lcv_RetValue);
            lcv_RetValue = ERR_USER_PROGRAM;
            goto COMPILER_UCODE_ERR;
        }
    }

    return pdPASS;

COMPILER_UCODE_ERR:
    guv_StopError.bit.ucode_err = 1;
    /*刷新运行态错误信息,提供给MISTUDIO软件定位*/
    plc_refresh_exec_error_record(lcv_RetValue, lcp_UCodePtr);
    return pdFAIL;
}

void plc_free_ins(void)
{
    PRINTF("Enter %s()\r\n", __func__);
    struct list_head *ltp_McHead;
    struct list_head *ltp_Head;
    struct list_head *ltp_ForCur;
    struct list_head *ltp_ForNext;
    plc_ins_list_st *ltp_PlcIns;
    struct list_head *ltp_ListCur;
    struct list_head *ltp_ListNext;
    plc_mc_mcr_ins_list_st *ltp_McMcrIns;
    unsigned short i;
    /*删除子程序的清除指令列表项,释放内存*/
    for(i=0; i<MAX_SBR_COUNT; i++) {

        ltp_Head = &gtp_SbrListPtr[i].mtv_SimpInsHead;
        list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
            ltp_PlcIns = list_entry(ltp_ForCur, plc_ins_list_st, mtv_InsList);
            list_del(&ltp_PlcIns->mtv_InsList);
            vPortFree(ltp_PlcIns);
        }

        ltp_Head = &gtp_SbrListPtr[i].mtv_CompInsHead;
        list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
            ltp_PlcIns = list_entry(ltp_ForCur, plc_ins_list_st, mtv_InsList);
            list_del(&ltp_PlcIns->mtv_InsList);
            vPortFree(ltp_PlcIns);
        }
    }

    /*删除STL指令列表,并释放内存*/
    for(i=0; i<S_RANG; i++) {

        ltp_Head = &gtp_StlListPtr[i].mtv_SimpInsHead;
        list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
            ltp_PlcIns = list_entry(ltp_ForCur, plc_ins_list_st, mtv_InsList);
            list_del(&ltp_PlcIns->mtv_InsList);
            vPortFree(ltp_PlcIns);
        }

        ltp_Head = &gtp_StlListPtr[i].mtv_CompInsHead;
        list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
            ltp_PlcIns = list_entry(ltp_ForCur, plc_ins_list_st, mtv_InsList);
            list_del(&ltp_PlcIns->mtv_InsList);
            vPortFree(ltp_PlcIns);
        }
    }

    /*删除MC-MCR指令列表项,并释放内存*/
    gtv_McMcrBlockInfo.mcv_NestedLevel = 0;

    ltp_McHead = &gtv_McMcrBlockInfo.mtv_McMcrHead;
    list_for_each_safe(ltp_ListCur, ltp_ListNext, ltp_McHead) {
        ltp_McMcrIns = list_entry(ltp_ListCur, plc_mc_mcr_ins_list_st, mtv_List);

        ltp_Head = &ltp_McMcrIns->mtv_SimpInsHead;
        list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
            ltp_PlcIns = list_entry(ltp_ForCur, plc_ins_list_st, mtv_InsList);
            list_del(&ltp_PlcIns->mtv_InsList);
            vPortFree(ltp_PlcIns);
        }

        ltp_Head = &ltp_McMcrIns->mtv_CompInsHead;
        list_for_each_safe(ltp_ForCur, ltp_ForNext, ltp_Head) {
            ltp_PlcIns = list_entry(ltp_ForCur, plc_ins_list_st, mtv_InsList);
            list_del(&ltp_PlcIns->mtv_InsList);
            vPortFree(ltp_PlcIns);
        }

        list_del(&ltp_McMcrIns->mtv_List);
        vPortFree(ltp_McMcrIns);
    }
}
