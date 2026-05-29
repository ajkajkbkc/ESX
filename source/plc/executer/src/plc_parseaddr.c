/**
  ******************************************************************************
  * @file    plc_parseaddr.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   UCODE参数寻址解析相关函数
  ******************************************************************************
  */
#include "FreeRTOS.h"
#include "plc_element.h"
#include "plc_parseaddr.h"
#include "plc_commonfunc.h"
#include "plc_errormsg.h"
#include "plc_variable.h"

/**
  * @brief  获取位元件值
  * @param  lcp_UcodeAddr   UCODE取值开始地址
  *         lcp_Value   返回值
  *         lsv_Offset  偏移量
  *         lsv_Count   读取个数
  * @retval None
  */
unsigned char get_char(unsigned char *lcp_UcodeAddr, unsigned char *lcp_Value, unsigned short lsv_Offset, unsigned short lsv_Count)
{
    /*元件下标号*/
    unsigned short lsv_ElementIndex;
    /*变址寻址中Z元件值*/
    unsigned short lsv_ZElementValue;

    switch(GET_PU8_DATA(lcp_UcodeAddr+1)) {
        case ADDR_X:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;
            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < X_RANG) {
                    *lcp_Value = plc_get_bit_element_value(X_ELEMENT, lsv_ElementIndex);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_Y:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;
            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < Y_RANG) {
                    *lcp_Value = plc_get_bit_element_value(Y_ELEMENT, lsv_ElementIndex);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_bC:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;
            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < C_RANG) {
                    *lcp_Value = plc_get_bit_element_value(C_ELEMENT, lsv_ElementIndex);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_bT:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;
            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < T_RANG) {
                    *lcp_Value = plc_get_bit_element_value(T_ELEMENT, lsv_ElementIndex);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_SM:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;
            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < SM_RANG) {
                    *lcp_Value = plc_get_bit_element_value(SM_ELEMENT, lsv_ElementIndex);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_M:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;
            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < M_RANG) {
                    *lcp_Value = plc_get_bit_element_value(M_ELEMENT, lsv_ElementIndex);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_S:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;
            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < S_RANG) {
                    *lcp_Value = plc_get_bit_element_value(S_ELEMENT, lsv_ElementIndex);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_LM:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;
            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < LM_RANG) {
                    *lcp_Value = plc_get_lm_element_value((gtp_CallInsInfoPtr->msv_SbrNestedNum+1), lsv_ElementIndex);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_XZ:
            /*取Z元件值*/
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;
            /*求最终元件下标*/
            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < X_RANG) {
                    *lcp_Value = plc_get_bit_element_value(X_ELEMENT, lsv_ElementIndex);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_YZ:
            /*取Z元件值*/
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;
            /*求最终元件下标*/
            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < Y_RANG) {
                    *lcp_Value = plc_get_bit_element_value(Y_ELEMENT, lsv_ElementIndex);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_MZ:
            /*取Z元件值*/
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;
            /*求最终元件下标*/
            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < M_RANG) {
                    *lcp_Value = plc_get_bit_element_value(M_ELEMENT, lsv_ElementIndex);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_SZ:
            /*取Z元件值*/
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;
            /*求最终元件下标*/
            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < S_RANG) {
                    *lcp_Value = plc_get_bit_element_value(S_ELEMENT, lsv_ElementIndex);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_LMZ:
            /*取Z元件值*/
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;
            /*求最终元件下标*/
            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < LM_RANG) {
                    *lcp_Value = plc_get_lm_element_value((gtp_CallInsInfoPtr->msv_SbrNestedNum+1), lsv_ElementIndex);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_bTZ:
            /*取Z元件值*/
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;
            /*求最终元件下标*/
            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < T_RANG) {
                    *lcp_Value = plc_get_bit_element_value(T_ELEMENT, lsv_ElementIndex);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_bCZ:
            /*取Z元件值*/
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;
            /*求最终元件下标*/
            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < C_RANG) {
                    *lcp_Value = plc_get_bit_element_value(C_ELEMENT, lsv_ElementIndex);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;
        default:
            return ERR_ELEMENT_TYPE;
    }
}

unsigned char get_char_default(unsigned char *lcp_UcodeAddr, unsigned char *lcp_Value)
{
    return get_char(lcp_UcodeAddr, lcp_Value, 0, 1);
}

/**
  * @brief  设置位元件值
  * @param  lcp_UcodeAddr   UCODE取值开始地址
  *         lcp_Value   指向需要保存的值
  *         lsv_Offset  偏移量
  *         lsv_Count   保存个数
  * @retval None
  */
unsigned char save_char(unsigned char *lcp_UcodeAddr, unsigned char *lcp_Value, unsigned short lsv_Offset, unsigned short lsv_Count)
{
    /*元件下标号*/
    unsigned short lsv_ElementIndex;
    /*变址寻址中Z元件值*/
    unsigned short lsv_ZElementValue;

    switch(GET_PU8_DATA(lcp_UcodeAddr+1)) {

        case ADDR_Y:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;
            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < Y_RANG) {
                    plc_set_bit_element_value(Y_ELEMENT, lsv_ElementIndex, *lcp_Value);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_C:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;
            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < C_RANG) {
                    plc_set_bit_element_value(C_ELEMENT, lsv_ElementIndex, *lcp_Value);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_T:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;
            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < T_RANG) {
                    plc_set_bit_element_value(T_ELEMENT, lsv_ElementIndex, *lcp_Value);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_SM:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;
            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < SM_RANG) {
                    plc_set_bit_element_value(SM_ELEMENT, lsv_ElementIndex, *lcp_Value);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_M:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;
            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < M_RANG) {
                    plc_set_bit_element_value(M_ELEMENT, lsv_ElementIndex, *lcp_Value);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_S:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;
            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < S_RANG) {
                    plc_set_bit_element_value(S_ELEMENT, lsv_ElementIndex, *lcp_Value);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_LM:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;
            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < LM_RANG) {
                    plc_set_lm_element_value((gtp_CallInsInfoPtr->msv_SbrNestedNum+1), lsv_ElementIndex, *lcp_Value);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_YZ:
            /*取Z元件值*/
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;
            /*求最终元件下标*/
            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < Y_RANG) {
                    plc_set_bit_element_value(Y_ELEMENT, lsv_ElementIndex, *lcp_Value);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_MZ:
            /*取Z元件值*/
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;
            /*求最终元件下标*/
            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < M_RANG) {
                    plc_set_bit_element_value(M_ELEMENT, lsv_ElementIndex, *lcp_Value);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_SZ:
            /*取Z元件值*/
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;
            /*求最终元件下标*/
            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < S_RANG) {
                    plc_set_bit_element_value(S_ELEMENT, lsv_ElementIndex, *lcp_Value);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_LMZ:
            /*取Z元件值*/
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;
            /*求最终元件下标*/
            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < LM_RANG) {
                    plc_set_lm_element_value((gtp_CallInsInfoPtr->msv_SbrNestedNum+1), lsv_ElementIndex, *lcp_Value);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_TZ:
            /*取Z元件值*/
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;
            /*求最终元件下标*/
            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < T_RANG) {
                    plc_set_bit_element_value(T_ELEMENT, lsv_ElementIndex, *lcp_Value);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        case ADDR_CZ:
            /*取Z元件值*/
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;
            /*求最终元件下标*/
            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < C_RANG) {
                    plc_set_bit_element_value(C_ELEMENT, lsv_ElementIndex, *lcp_Value);
                    lsv_ElementIndex ++;
                    lcp_Value ++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }

            return pdPASS;

        default:
            return ERR_ELEMENT_TYPE;
    }
}

unsigned char save_char_default(unsigned char *lcp_UcodeAddr, unsigned char *lcp_Value)
{
    return save_char(lcp_UcodeAddr, lcp_Value, 0, 1);
}

/**
  * @brief  获取字元件值
  * @param  lcp_UcodeAddr   UCODE取值开始地址
  *         lsp_Value   返回值指针
  *         lsv_Offset  偏移量
  *         lsv_Count   读取个数
  * @retval None
  */
unsigned char get_word(unsigned char *lcp_UcodeAddr, unsigned short *lsp_Value, unsigned short lsv_Offset, unsigned short lsv_Count)
{
    unsigned char lcv_Kn;
    unsigned short lsv_Temp;
    /*元件下标号*/
    unsigned short lsv_ElementIndex;
    unsigned short lsv_StartIndex;

    /*变址寻址中Z元件值*/
    unsigned short lsv_ZElementValue;

    switch(GET_PU8_DATA(lcp_UcodeAddr+1)) {

        case ADDR_CONST:
            if(GET_PU8_DATA(lcp_UcodeAddr) >= 2) {
                return ERR_ELEMENT_TYPE;
            } else {
                lcp_UcodeAddr += 2;
                *lsp_Value = GET_PU16_DATA(lcp_UcodeAddr);
            }
            return pdPASS;

        case ADDR_KnX:
            lcv_Kn = *lcp_UcodeAddr;
            if((lcv_Kn < 1) || (lcv_Kn > 4)) {
                return ERR_OPERANDS;
            }

            lcp_UcodeAddr += 2;
            lsv_StartIndex = GET_PU16_DATA(lcp_UcodeAddr);

            for( ; lsv_Count > 0; lsv_Count--) {

                *lsp_Value = 0;

                for(lsv_Temp=4*lcv_Kn; lsv_Temp>0; lsv_Temp--) {

                    lsv_ElementIndex = lsv_Temp + lsv_StartIndex + lsv_Offset*4*lcv_Kn - 1;

                    if(lsv_ElementIndex < X_RANG) {
                        *lsp_Value = (*lsp_Value << 1) + plc_get_bit_element_value(X_ELEMENT, lsv_ElementIndex);
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }

                lsp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_KnY:
            lcv_Kn = *lcp_UcodeAddr;
            if((lcv_Kn < 1) || (lcv_Kn > 4)) {
                return ERR_OPERANDS;
            }

            lcp_UcodeAddr += 2;
            lsv_StartIndex = GET_PU16_DATA(lcp_UcodeAddr);

            for( ; lsv_Count > 0; lsv_Count--) {

                *lsp_Value = 0;

                for(lsv_Temp=4*lcv_Kn; lsv_Temp>0; lsv_Temp--) {

                    lsv_ElementIndex = lsv_Temp + lsv_StartIndex + lsv_Offset*4*lcv_Kn - 1;

                    if(lsv_ElementIndex < Y_RANG) {
                        *lsp_Value = (*lsp_Value << 1) + plc_get_bit_element_value(Y_ELEMENT, lsv_ElementIndex);
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }

                }

                lsp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_KnM:
            lcv_Kn = *lcp_UcodeAddr;
            if((lcv_Kn < 1) || (lcv_Kn > 4)) {
                return ERR_OPERANDS;
            }

            lcp_UcodeAddr += 2;
            lsv_StartIndex = GET_PU16_DATA(lcp_UcodeAddr);

            for( ; lsv_Count > 0; lsv_Count--) {

                *lsp_Value = 0;

                for(lsv_Temp=4*lcv_Kn; lsv_Temp>0; lsv_Temp--) {

                    lsv_ElementIndex = lsv_Temp + lsv_StartIndex + lsv_Offset*4*lcv_Kn - 1;

                    if(lsv_ElementIndex < M_RANG) {
                        *lsp_Value = (*lsp_Value << 1) + plc_get_bit_element_value(M_ELEMENT, lsv_ElementIndex);
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }

                }

                lsp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_KnS:
            lcv_Kn = *lcp_UcodeAddr;
            if((lcv_Kn < 1) || (lcv_Kn > 4)) {
                return ERR_OPERANDS;
            }

            lcp_UcodeAddr += 2;
            lsv_StartIndex = GET_PU16_DATA(lcp_UcodeAddr);

            for( ; lsv_Count > 0; lsv_Count--) {

                *lsp_Value = 0;

                for(lsv_Temp=4*lcv_Kn; lsv_Temp>0; lsv_Temp--) {

                    lsv_ElementIndex = lsv_Temp + lsv_StartIndex + lsv_Offset*4*lcv_Kn - 1;

                    if(lsv_ElementIndex < S_RANG) {
                        *lsp_Value = (*lsp_Value << 1) + plc_get_bit_element_value(S_ELEMENT, lsv_ElementIndex);
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }

                }

                lsp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_KnLM:
            lcv_Kn = *lcp_UcodeAddr;
            if((lcv_Kn < 1) || (lcv_Kn > 4)) {
                return ERR_OPERANDS;
            }

            lcp_UcodeAddr += 2;
            lsv_StartIndex = GET_PU16_DATA(lcp_UcodeAddr);

            for( ; lsv_Count > 0; lsv_Count--) {

                *lsp_Value = 0;

                for(lsv_Temp=4*lcv_Kn; lsv_Temp>0; lsv_Temp--) {

                    lsv_ElementIndex = lsv_Temp + lsv_StartIndex + lsv_Offset*4*lcv_Kn - 1;

                    if(lsv_ElementIndex < LM_RANG) {
                        *lsp_Value = (*lsp_Value << 1) + plc_get_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex);
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }

                }

                lsp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_KnSM:
            lcv_Kn = *lcp_UcodeAddr;
            if((lcv_Kn < 1) || (lcv_Kn > 4)) {
                return ERR_OPERANDS;
            }

            lcp_UcodeAddr += 2;
            lsv_StartIndex = GET_PU16_DATA(lcp_UcodeAddr);

            for( ; lsv_Count > 0; lsv_Count--) {

                *lsp_Value = 0;

                for(lsv_Temp=4*lcv_Kn; lsv_Temp>0; lsv_Temp--) {

                    lsv_ElementIndex = lsv_Temp + lsv_StartIndex + lsv_Offset*4*lcv_Kn - 1;

                    if(lsv_ElementIndex < SM_RANG) {
                        *lsp_Value = (*lsp_Value << 1) + plc_get_bit_element_value(SM_ELEMENT, lsv_ElementIndex);
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }

                }

                lsp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_D:
        case ADDR_DX_Y:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < D_RANG) {
                    *lsp_Value = GET_D_ELEMENT_VALUE(lsv_ElementIndex);
                    lsp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }

            }
            return pdPASS;

        case ADDR_R:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < R_RANG) {
                    *lsp_Value = GET_R_ELEMENT_VALUE(lsv_ElementIndex);
                    lsp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }

            }
            return pdPASS;

        case ADDR_Z:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < Z_RANG) {
                    *lsp_Value = GET_Z_ELEMENT_VALUE(lsv_ElementIndex);
                    lsp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }

            }
            return pdPASS;

        case ADDR_SD:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < SD_RANG) {
                    *lsp_Value = GET_SD_ELEMENT_VALUE(lsv_ElementIndex);
                    lsp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }

            }
            return pdPASS;

        /*16位计数器当前值*/
        case ADDR_C:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < C16_RANG) {
                    *lsp_Value = GET_C16_CURRENT_VALUE(lsv_ElementIndex);
                    lsp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }

            }
            return pdPASS;

        /*定时器当前值*/
        case ADDR_T:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < T_RANG) {
                    *lsp_Value = GET_T_CURRENT_VALUE(lsv_ElementIndex);
                    lsp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }

            }
            return pdPASS;

        case ADDR_V:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < V_RANG) {
                    *lsp_Value = GET_V_ELEMENT_VALUE(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex);
                    lsp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }

            }
            return pdPASS;

        case ADDR_DZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < D_RANG) {
                    *lsp_Value = GET_D_ELEMENT_VALUE(lsv_ElementIndex);
                    lsp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }

            }
            return pdPASS;

        case ADDR_RZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < R_RANG) {
                    *lsp_Value = GET_R_ELEMENT_VALUE(lsv_ElementIndex);
                    lsp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }

            }
            return pdPASS;

        case ADDR_CZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < C16_RANG) {
                    *lsp_Value = GET_C16_CURRENT_VALUE(lsv_ElementIndex);
                    lsp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }

            }
            return pdPASS;

        case ADDR_TZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < T_RANG) {
                    *lsp_Value = GET_T_CURRENT_VALUE(lsv_ElementIndex);
                    lsp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }

            }
            return pdPASS;

        case ADDR_VZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < V_RANG) {
                    *lsp_Value = GET_V_ELEMENT_VALUE(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex);
                    lsp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }

            }
            return pdPASS;

        case ADDR_KnXZ:
            lcv_Kn = ((*lcp_UcodeAddr)&0xF0)>>4;
            if((lcv_Kn<1)||(lcv_Kn>4)) {
                return ERR_OPERANDS;
            }

            lsv_ZElementValue = GET_Z_ELEMENT_VALUE((*lcp_UcodeAddr)&0x0F);
            lcp_UcodeAddr += 2;

            lsv_StartIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                *lsp_Value = 0;

                for(lsv_Temp=4*lcv_Kn; lsv_Temp>0; lsv_Temp--) {
                    lsv_ElementIndex = lsv_StartIndex + lsv_Temp + lsv_Offset*4*lcv_Kn -1;

                    if(lsv_ElementIndex < X_RANG) {
                        *lsp_Value = (*lsp_Value << 1) + plc_get_bit_element_value(X_ELEMENT, lsv_ElementIndex);
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                lsp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_KnYZ:
            lcv_Kn = ((*lcp_UcodeAddr)&0xF0)>>4;
            if((lcv_Kn<1)||(lcv_Kn>4)) {
                return ERR_OPERANDS;
            }

            lsv_ZElementValue = GET_Z_ELEMENT_VALUE((*lcp_UcodeAddr)&0x0F);
            lcp_UcodeAddr += 2;

            lsv_StartIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                *lsp_Value = 0;

                for(lsv_Temp=4*lcv_Kn; lsv_Temp>0; lsv_Temp--) {
                    lsv_ElementIndex = lsv_StartIndex + lsv_Temp + lsv_Offset*4*lcv_Kn -1;

                    if(lsv_ElementIndex < Y_RANG) {
                        *lsp_Value = (*lsp_Value << 1) + plc_get_bit_element_value(Y_ELEMENT, lsv_ElementIndex);
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                lsp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_KnMZ:
            lcv_Kn = ((*lcp_UcodeAddr)&0xF0)>>4;
            if((lcv_Kn<1)||(lcv_Kn>4)) {
                return ERR_OPERANDS;
            }

            lsv_ZElementValue = GET_Z_ELEMENT_VALUE((*lcp_UcodeAddr)&0x0F);
            lcp_UcodeAddr += 2;

            lsv_StartIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                *lsp_Value = 0;

                for(lsv_Temp=4*lcv_Kn; lsv_Temp>0; lsv_Temp--) {
                    lsv_ElementIndex = lsv_StartIndex + lsv_Temp + lsv_Offset*4*lcv_Kn -1;

                    if(lsv_ElementIndex < M_RANG) {
                        *lsp_Value = (*lsp_Value << 1) + plc_get_bit_element_value(M_ELEMENT, lsv_ElementIndex);
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                lsp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_KnSZ:
            lcv_Kn = ((*lcp_UcodeAddr)&0xF0)>>4;
            if((lcv_Kn<1)||(lcv_Kn>4)) {
                return ERR_OPERANDS;
            }

            lsv_ZElementValue = GET_Z_ELEMENT_VALUE((*lcp_UcodeAddr)&0x0F);
            lcp_UcodeAddr += 2;

            lsv_StartIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                *lsp_Value = 0;

                for(lsv_Temp=4*lcv_Kn; lsv_Temp>0; lsv_Temp--) {
                    lsv_ElementIndex = lsv_StartIndex + lsv_Temp + lsv_Offset*4*lcv_Kn -1;

                    if(lsv_ElementIndex < S_RANG) {
                        *lsp_Value = (*lsp_Value << 1) + plc_get_bit_element_value(S_ELEMENT, lsv_ElementIndex);
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                lsp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_KnLMZ:
            lcv_Kn = ((*lcp_UcodeAddr)&0xF0)>>4;
            if((lcv_Kn<1)||(lcv_Kn>4)) {
                return ERR_OPERANDS;
            }

            lsv_ZElementValue = GET_Z_ELEMENT_VALUE((*lcp_UcodeAddr)&0x0F);
            lcp_UcodeAddr += 2;

            lsv_StartIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                *lsp_Value = 0;

                for(lsv_Temp=4*lcv_Kn; lsv_Temp>0; lsv_Temp--) {
                    lsv_ElementIndex = lsv_StartIndex + lsv_Temp + lsv_Offset*4*lcv_Kn -1;

                    if(lsv_ElementIndex < LM_RANG) {
                        *lsp_Value = (*lsp_Value << 1) + plc_get_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex);
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                lsp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        default:
            return ERR_ELEMENT_TYPE;
    }
}

unsigned char get_word_default(unsigned char *lcp_UcodeAddr, unsigned short *lsp_Value)
{
    return get_word(lcp_UcodeAddr, lsp_Value, 0, 1);
}

/**
  * @brief  获取字元件地址
  * @param  lcp_UcodeAddr   UCODE取值开始地址
  *         lsp_Addr  返回连续lsv_Count个元件开始地址值指针
  *         lsv_Num   元件个数
  * @retval None
  */
unsigned char get_word_addr(unsigned char *lcp_UcodeAddr, unsigned short **lsp_Addr, unsigned short lsv_Num)
{
    /*元件下标号*/
    unsigned short lsv_ElementIndex;
    /*变址寻址中Z元件值*/
    unsigned short lsv_ZElementValue;

    switch(GET_PU8_DATA(lcp_UcodeAddr+1)) {

        case ADDR_D:
            lcp_UcodeAddr += 2;

            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr);

            if(lsv_ElementIndex + lsv_Num >= D_RANG) {
                return ERR_OVER_ELEMENT_RANG;
            }

            *lsp_Addr = &GET_D_ELEMENT_VALUE(lsv_ElementIndex);
            break;

        case ADDR_R:
            lcp_UcodeAddr += 2;

            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr);

            if(lsv_ElementIndex + lsv_Num >= R_RANG) {
                return ERR_OVER_ELEMENT_RANG;
            }

            *lsp_Addr = &GET_R_ELEMENT_VALUE(lsv_ElementIndex);
            break;

        case ADDR_DZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);
            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr);

            if(lsv_ElementIndex + lsv_Num >= D_RANG) {
                return ERR_OVER_ELEMENT_RANG;
            }

            *lsp_Addr = &GET_D_ELEMENT_VALUE(lsv_ElementIndex);
            break;

        case ADDR_RZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);
            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr);

            if(lsv_ElementIndex + lsv_Num >= R_RANG) {
                return ERR_OVER_ELEMENT_RANG;
            }

            *lsp_Addr = &GET_R_ELEMENT_VALUE(lsv_ElementIndex);
            break;

        default:
            return ERR_ELEMENT_TYPE;
    }

    return pdPASS;
}

/**
  * @brief  保存字元件值
  * @param  lcp_UcodeAddr   UCODE取值开始地址
  *         lsp_Value   保存值指针
  *         lsv_Offset  偏移量
  *         lsv_Count   读取个数
  * @retval None
  */
unsigned char save_word(unsigned char *lcp_UcodeAddr, unsigned short *lsp_Value, unsigned short lsv_Offset, unsigned short lsv_Count)
{
    unsigned char lcv_Kn;
    unsigned short lsv_Temp;
    /*元件下标号*/
    unsigned short lsv_ElementIndex;
    unsigned short lsv_StartIndex;

    /*变址寻址中Z元件值*/
    unsigned short lsv_ZElementValue;

    switch(GET_PU8_DATA(lcp_UcodeAddr+1)) {

        case ADDR_KnY:
            lcv_Kn = *lcp_UcodeAddr;
            if((lcv_Kn < 1) || (lcv_Kn > 4)) {
                return ERR_OPERANDS;
            }

            lcp_UcodeAddr += 2;
            lsv_StartIndex = GET_PU16_DATA(lcp_UcodeAddr);

            for( ; lsv_Count > 0; lsv_Count--) {

                for(lsv_Temp=0; lsv_Temp<4*lcv_Kn; lsv_Temp++) {

                    lsv_ElementIndex = lsv_Temp + lsv_StartIndex + lsv_Offset*4*lcv_Kn;

                    if(lsv_ElementIndex < Y_RANG) {
                        plc_set_bit_element_value(Y_ELEMENT, lsv_ElementIndex, (unsigned char)(*lsp_Value & 0x01));

                        *lsp_Value >>= 0x01;
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }

                }

                lsp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_KnM:
            lcv_Kn = *lcp_UcodeAddr;
            if((lcv_Kn < 1) || (lcv_Kn > 4)) {
                return ERR_OPERANDS;
            }

            lcp_UcodeAddr += 2;
            lsv_StartIndex = GET_PU16_DATA(lcp_UcodeAddr);

            for( ; lsv_Count > 0; lsv_Count--) {

                for(lsv_Temp=0; lsv_Temp<4*lcv_Kn; lsv_Temp++) {

                    lsv_ElementIndex = lsv_Temp + lsv_StartIndex + lsv_Offset*4*lcv_Kn;

                    if(lsv_ElementIndex < M_RANG) {
                        plc_set_bit_element_value(M_ELEMENT, lsv_ElementIndex, (unsigned char)(*lsp_Value & 0x01));

                        *lsp_Value >>= 0x01;
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }

                }

                lsp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_KnS:
            lcv_Kn = *lcp_UcodeAddr;
            if((lcv_Kn < 1) || (lcv_Kn > 4)) {
                return ERR_OPERANDS;
            }

            lcp_UcodeAddr += 2;
            lsv_StartIndex = GET_PU16_DATA(lcp_UcodeAddr);

            for( ; lsv_Count > 0; lsv_Count--) {

                for(lsv_Temp=0; lsv_Temp<4*lcv_Kn; lsv_Temp++) {

                    lsv_ElementIndex = lsv_Temp + lsv_StartIndex + lsv_Offset*4*lcv_Kn;

                    if(lsv_ElementIndex < S_RANG) {
                        plc_set_bit_element_value(S_ELEMENT, lsv_ElementIndex, (unsigned char)(*lsp_Value & 0x01));

                        *lsp_Value >>= 0x01;
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }

                }

                lsp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_KnLM:
            lcv_Kn = *lcp_UcodeAddr;
            if((lcv_Kn < 1) || (lcv_Kn > 4)) {
                return ERR_OPERANDS;
            }

            lcp_UcodeAddr += 2;
            lsv_StartIndex = GET_PU16_DATA(lcp_UcodeAddr);

            for( ; lsv_Count > 0; lsv_Count--) {

                for(lsv_Temp=0; lsv_Temp<4*lcv_Kn; lsv_Temp++) {

                    lsv_ElementIndex = lsv_Temp + lsv_StartIndex + lsv_Offset*4*lcv_Kn;

                    if(lsv_ElementIndex < LM_RANG) {
                        plc_set_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex, (unsigned char)(*lsp_Value & 0x01));

                        *lsp_Value >>= 0x01;
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }

                }

                lsp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_D:
        case ADDR_DX_Y:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < D_RANG) {
                    SET_D_ELEMENT_VALUE(lsv_ElementIndex, *lsp_Value);
                    lsp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }

            }
            return pdPASS;

        case ADDR_R:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < R_RANG) {
                    SET_R_ELEMENT_VALUE(lsv_ElementIndex, *lsp_Value);
                    lsp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }

            }
            return pdPASS;

        case ADDR_Z:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < Z_RANG) {
                    SET_Z_ELEMENT_VALUE(lsv_ElementIndex, *lsp_Value);
                    lsp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }

            }
            return pdPASS;

        case ADDR_SD:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < SD_RANG) {
                    SET_SD_ELEMENT_VALUE(lsv_ElementIndex, *lsp_Value);
                    lsp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }

            }
            return pdPASS;

        case ADDR_C:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < C16_RANG) {
                    SET_C16_CURRENT_VALUE(lsv_ElementIndex, *lsp_Value);
                    lsp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }

            }
            return pdPASS;

        case ADDR_T:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < T_RANG) {
                    SET_T_CURRENT_VALUE(lsv_ElementIndex, *lsp_Value);
                    lsp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }

            }
            return pdPASS;

        case ADDR_V:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < V_RANG) {
                    SET_V_ELEMENT_VALUE((gtp_CallInsInfoPtr->msv_SbrNestedNum+1), lsv_ElementIndex, *lsp_Value);
                    lsp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }

            }
            return pdPASS;

        case ADDR_DZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < D_RANG) {
                    SET_D_ELEMENT_VALUE(lsv_ElementIndex, *lsp_Value);
                    lsp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }

            }
            return pdPASS;

        case ADDR_RZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < D_RANG) {
                    SET_R_ELEMENT_VALUE(lsv_ElementIndex, *lsp_Value);
                    lsp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }

            }
            return pdPASS;

        case ADDR_CZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < C16_RANG) {
                    SET_C16_CURRENT_VALUE(lsv_ElementIndex, *lsp_Value);
                    lsp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }

            }
            return pdPASS;

        case ADDR_TZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < T_RANG) {
                    SET_T_CURRENT_VALUE(lsv_ElementIndex, *lsp_Value);
                    lsp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }

            }
            return pdPASS;

        case ADDR_VZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < V_RANG) {
                    SET_V_ELEMENT_VALUE(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex, *lsp_Value);
                    lsp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }

            }
            return pdPASS;

        case ADDR_KnYZ:
            lcv_Kn = ((*lcp_UcodeAddr)&0xF0)>>4;
            if((lcv_Kn<1)||(lcv_Kn>4)) {
                return ERR_OPERANDS;
            }

            lsv_ZElementValue = GET_Z_ELEMENT_VALUE((*lcp_UcodeAddr)&0x0F);
            lcp_UcodeAddr += 2;

            lsv_StartIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                for(lsv_Temp=0; lsv_Temp<4*lcv_Kn; lsv_Temp++) {
                    lsv_ElementIndex = lsv_StartIndex + lsv_Temp + lsv_Offset*4*lcv_Kn;

                    if(lsv_ElementIndex < Y_RANG) {
                        plc_set_bit_element_value(Y_ELEMENT, lsv_ElementIndex, (unsigned char)(*lsp_Value & 0x01));
                        *lsp_Value >>= 0x01;
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                lsp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_KnMZ:
            lcv_Kn = ((*lcp_UcodeAddr)&0xF0)>>4;
            if((lcv_Kn<1)||(lcv_Kn>4)) {
                return ERR_OPERANDS;
            }

            lsv_ZElementValue = GET_Z_ELEMENT_VALUE((*lcp_UcodeAddr)&0x0F);
            lcp_UcodeAddr += 2;

            lsv_StartIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                for(lsv_Temp=0; lsv_Temp<4*lcv_Kn; lsv_Temp++) {
                    lsv_ElementIndex = lsv_StartIndex + lsv_Temp + lsv_Offset*4*lcv_Kn;

                    if(lsv_ElementIndex < M_RANG) {
                        plc_set_bit_element_value(M_ELEMENT, lsv_ElementIndex, (unsigned char)(*lsp_Value & 0x01));
                        *lsp_Value >>= 0x01;
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                lsp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_KnSZ:
            lcv_Kn = ((*lcp_UcodeAddr)&0xF0)>>4;
            if((lcv_Kn<1)||(lcv_Kn>4)) {
                return ERR_OPERANDS;
            }

            lsv_ZElementValue = GET_Z_ELEMENT_VALUE((*lcp_UcodeAddr)&0x0F);
            lcp_UcodeAddr += 2;

            lsv_StartIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                for(lsv_Temp=0; lsv_Temp<4*lcv_Kn; lsv_Temp++) {
                    lsv_ElementIndex = lsv_StartIndex + lsv_Temp + lsv_Offset*4*lcv_Kn;

                    if(lsv_ElementIndex < S_RANG) {
                        plc_set_bit_element_value(S_ELEMENT, lsv_ElementIndex, (unsigned char)(*lsp_Value & 0x01));
                        *lsp_Value >>= 0x01;
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                lsp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_KnLMZ:
            lcv_Kn = ((*lcp_UcodeAddr)&0xF0)>>4;
            if((lcv_Kn<1)||(lcv_Kn>4)) {
                return ERR_OPERANDS;
            }

            lsv_ZElementValue = GET_Z_ELEMENT_VALUE((*lcp_UcodeAddr)&0x0F);
            lcp_UcodeAddr += 2;

            lsv_StartIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                for(lsv_Temp=0; lsv_Temp<4*lcv_Kn; lsv_Temp++) {
                    lsv_ElementIndex = lsv_StartIndex + lsv_Temp + lsv_Offset*4*lcv_Kn;

                    if(lsv_ElementIndex < LM_RANG) {
                        plc_set_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex, (unsigned char)(*lsp_Value & 0x01));
                        *lsp_Value >>= 0x01;
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                lsp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        default:
            return ERR_ELEMENT_TYPE;
    }
}

unsigned char save_word_default(unsigned char *lcp_UcodeAddr, unsigned short *lsp_Value)
{
    return save_word(lcp_UcodeAddr, lsp_Value, 0, 1);
}


/**
  * @brief  获取双字元件值
  * @param  lcp_UcodeAddr   UCODE取值开始地址
  *         llp_Value   返回值指针
  *         lsv_Offset  偏移量
  *         lsv_Count   读取个数
  * @retval None
  */
unsigned char get_dword(unsigned char *lcp_UcodeAddr, unsigned long *llp_Value, unsigned short lsv_Offset, unsigned short lsv_Count)
{
    unsigned char lcv_Kn;
    unsigned short lsv_Temp;
    /*元件下标号*/
    unsigned short lsv_ElementIndex;
    unsigned short lsv_StartIndex;

    /*变址寻址中Z元件值*/
    unsigned short lsv_ZElementValue;

    switch(GET_PU8_DATA(lcp_UcodeAddr+1)) {

        case ADDR_CONST:
            if( *lcp_UcodeAddr < 2 || *lcp_UcodeAddr > 3) {
                return ERR_ELEMENT_TYPE;
            } else {
                lcp_UcodeAddr += 2;
                *llp_Value = GET_PU32_DATA(lcp_UcodeAddr);
            }
            return pdPASS;

        case ADDR_KnX:
            lcv_Kn = *lcp_UcodeAddr;
            if(lcv_Kn<5 || lcv_Kn>8) {
                return ERR_OPERANDS;
            }

            lcp_UcodeAddr += 2;
            lsv_StartIndex = GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                *llp_Value = 0;

                for(lsv_Temp=4*lcv_Kn; lsv_Temp>0; lsv_Temp--) {
                    lsv_ElementIndex = lsv_StartIndex + lsv_Temp + lsv_Offset*4*lcv_Kn -1;

                    if(lsv_ElementIndex < X_RANG) {
                        *llp_Value = (*llp_Value<<0x01) + plc_get_bit_element_value(X_ELEMENT, lsv_ElementIndex);
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                llp_Value++;
                lsv_Offset++;                
            }
            return pdPASS;

        case ADDR_KnY:
            lcv_Kn = *lcp_UcodeAddr;
            if(lcv_Kn<5 || lcv_Kn>8) {
                return ERR_OPERANDS;
            }

            lcp_UcodeAddr += 2;
            lsv_StartIndex = GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                *llp_Value = 0;

                for(lsv_Temp=4*lcv_Kn; lsv_Temp>0; lsv_Temp--) {
                    lsv_ElementIndex = lsv_StartIndex + lsv_Temp + lsv_Offset*4*lcv_Kn -1;

                    if(lsv_ElementIndex < Y_RANG) {
                        *llp_Value = (*llp_Value<<0x01) + plc_get_bit_element_value(Y_ELEMENT, lsv_ElementIndex);
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                llp_Value++;
                lsv_Offset++;                
            }
            return pdPASS;

        case ADDR_KnM:
            lcv_Kn = *lcp_UcodeAddr;
            if(lcv_Kn<5 || lcv_Kn>8) {
                return ERR_OPERANDS;
            }

            lcp_UcodeAddr += 2;
            lsv_StartIndex = GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                *llp_Value = 0;

                for(lsv_Temp=4*lcv_Kn; lsv_Temp>0; lsv_Temp--) {
                    lsv_ElementIndex = lsv_StartIndex + lsv_Temp + lsv_Offset*4*lcv_Kn -1;

                    if(lsv_ElementIndex < M_RANG) {
                        *llp_Value = (*llp_Value<<0x01) + plc_get_bit_element_value(M_ELEMENT, lsv_ElementIndex);
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }

                llp_Value++;
                lsv_Offset++;                
            }
            return pdPASS;

        case ADDR_KnS:
            lcv_Kn = *lcp_UcodeAddr;
            if(lcv_Kn<5 || lcv_Kn>8) {
                return ERR_OPERANDS;
            }

            lcp_UcodeAddr += 2;
            lsv_StartIndex = GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                *llp_Value = 0;

                for(lsv_Temp=4*lcv_Kn; lsv_Temp>0; lsv_Temp--) {
                    lsv_ElementIndex = lsv_StartIndex + lsv_Temp + lsv_Offset*4*lcv_Kn -1;

                    if(lsv_ElementIndex < S_RANG) {
                        *llp_Value = (*llp_Value<<0x01) + plc_get_bit_element_value(S_ELEMENT, lsv_ElementIndex);
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                
                llp_Value++;
                lsv_Offset++;                
            }
            return pdPASS;

        case ADDR_KnLM:
            lcv_Kn = *lcp_UcodeAddr;
            if(lcv_Kn<5 || lcv_Kn>8) {
                return ERR_OPERANDS;
            }

            lcp_UcodeAddr += 2;
            lsv_StartIndex = GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                *llp_Value = 0;

                for(lsv_Temp=4*lcv_Kn; lsv_Temp>0; lsv_Temp--) {
                    lsv_ElementIndex = lsv_StartIndex + lsv_Temp + lsv_Offset*4*lcv_Kn -1;

                    if(lsv_ElementIndex < LM_RANG) {
                        *llp_Value = (*llp_Value<<0x01) + plc_get_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex);
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                llp_Value++;
                lsv_Offset++;                
            }
            return pdPASS;

        case ADDR_KnSM:
            lcv_Kn = *lcp_UcodeAddr;
            if(lcv_Kn<5 || lcv_Kn>8) {
                return ERR_OPERANDS;
            }

            lcp_UcodeAddr += 2;
            lsv_StartIndex = GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                *llp_Value = 0;

                for(lsv_Temp=4*lcv_Kn; lsv_Temp>0; lsv_Temp--) {
                    lsv_ElementIndex = lsv_StartIndex + lsv_Temp + lsv_Offset*4*lcv_Kn -1;

                    if(lsv_ElementIndex < SM_RANG) {
                        *llp_Value = (*llp_Value<<0x01) + plc_get_bit_element_value(SM_ELEMENT, lsv_ElementIndex);
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                llp_Value++;
                lsv_Offset++;                
            }
            return pdPASS;

        case ADDR_D:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for(; lsv_Count>0; lsv_Count--) {
                if(lsv_ElementIndex < (D_RANG-1)) {
                    *llp_Value = (GET_D_ELEMENT_VALUE(lsv_ElementIndex+1)<<16) + GET_D_ELEMENT_VALUE(lsv_ElementIndex);
                    llp_Value++;
                    lsv_ElementIndex+=2;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_R:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for(; lsv_Count>0; lsv_Count--) {
                if(lsv_ElementIndex < (R_RANG-1)) {
                    *llp_Value = (GET_R_ELEMENT_VALUE(lsv_ElementIndex+1)<<16) + GET_R_ELEMENT_VALUE(lsv_ElementIndex);
                    llp_Value++;
                    lsv_ElementIndex+=2;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_SD:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for(; lsv_Count>0; lsv_Count--) {
                if(lsv_ElementIndex < (SD_RANG-1)) {
                    *llp_Value = (GET_SD_ELEMENT_VALUE(lsv_ElementIndex+1)<<16) + GET_SD_ELEMENT_VALUE(lsv_ElementIndex);
                    llp_Value++;
                    lsv_ElementIndex+=2;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_C:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for(; lsv_Count>0; lsv_Count--) {
                if(lsv_ElementIndex < (C_RANG-1)) {
                    *llp_Value = GET_C32_CURRENT_VALUE(lsv_ElementIndex);
                    llp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_V:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for(; lsv_Count>0; lsv_Count--) {
                if(lsv_ElementIndex < (SD_RANG-1)) {
                    *llp_Value = (GET_V_ELEMENT_VALUE((gtp_CallInsInfoPtr->msv_SbrNestedNum+1), lsv_ElementIndex+1)<<16) + GET_V_ELEMENT_VALUE(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex);
                    llp_Value++;
                    lsv_ElementIndex+=2;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_DZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < (D_RANG-1)) {
                    *llp_Value = (GET_D_ELEMENT_VALUE(lsv_ElementIndex+1)<<16) + GET_D_ELEMENT_VALUE(lsv_ElementIndex);
                    llp_Value++;
                    lsv_ElementIndex+=2;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_RZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < (R_RANG-1)) {
                    *llp_Value = (GET_R_ELEMENT_VALUE(lsv_ElementIndex+1)<<16) + GET_R_ELEMENT_VALUE(lsv_ElementIndex);
                    llp_Value++;
                    lsv_ElementIndex+=2;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_CZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < (C_RANG-1)) {
                    *llp_Value = GET_C32_CURRENT_VALUE(lsv_ElementIndex);
                    llp_Value++;
                    lsv_ElementIndex++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_VZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < (V_RANG-1)) {
                    *llp_Value = (GET_V_ELEMENT_VALUE((gtp_CallInsInfoPtr->msv_SbrNestedNum+1), lsv_ElementIndex+1)<<16) + GET_V_ELEMENT_VALUE(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex);
                    llp_Value++;
                    lsv_ElementIndex+=2;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_KnXZ:
            lcv_Kn = ((*lcp_UcodeAddr)&0xF0)>>4;
            if((lcv_Kn<1)||(lcv_Kn>8)) {
                return ERR_OPERANDS;
            }

            lsv_ZElementValue = GET_Z_ELEMENT_VALUE((*lcp_UcodeAddr)&0x0F);
            lcp_UcodeAddr += 2;

            lsv_StartIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                *llp_Value = 0;

                for(lsv_Temp=4*lcv_Kn; lsv_Temp>0; lsv_Temp--) {
                    lsv_ElementIndex = lsv_StartIndex + lsv_Temp + lsv_Offset*4*lcv_Kn -1;

                    if(lsv_ElementIndex < X_RANG) {
                        *llp_Value = (*llp_Value << 1) + plc_get_bit_element_value(X_ELEMENT, lsv_ElementIndex);
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                llp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_KnYZ:
            lcv_Kn = ((*lcp_UcodeAddr)&0xF0)>>4;
            if((lcv_Kn<1)||(lcv_Kn>8)) {
                return ERR_OPERANDS;
            }

            lsv_ZElementValue = GET_Z_ELEMENT_VALUE((*lcp_UcodeAddr)&0x0F);
            lcp_UcodeAddr += 2;

            lsv_StartIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                *llp_Value = 0;

                for(lsv_Temp=4*lcv_Kn; lsv_Temp>0; lsv_Temp--) {
                    lsv_ElementIndex = lsv_StartIndex + lsv_Temp + lsv_Offset*4*lcv_Kn -1;

                    if(lsv_ElementIndex < Y_RANG) {
                        *llp_Value = (*llp_Value << 1) + plc_get_bit_element_value(Y_ELEMENT, lsv_ElementIndex);
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                llp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_KnMZ:
            lcv_Kn = ((*lcp_UcodeAddr)&0xF0)>>4;
            if((lcv_Kn<1)||(lcv_Kn>8)) {
                return ERR_OPERANDS;
            }

            lsv_ZElementValue = GET_Z_ELEMENT_VALUE((*lcp_UcodeAddr)&0x0F);
            lcp_UcodeAddr += 2;

            lsv_StartIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                *llp_Value = 0;

                for(lsv_Temp=4*lcv_Kn; lsv_Temp>0; lsv_Temp--) {
                    lsv_ElementIndex = lsv_StartIndex + lsv_Temp + lsv_Offset*4*lcv_Kn -1;

                    if(lsv_ElementIndex < M_RANG) {
                        *llp_Value = (*llp_Value << 1) + plc_get_bit_element_value(M_ELEMENT, lsv_ElementIndex);
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                llp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_KnSZ:
            lcv_Kn = ((*lcp_UcodeAddr)&0xF0)>>4;
            if((lcv_Kn<1)||(lcv_Kn>8)) {
                return ERR_OPERANDS;
            }

            lsv_ZElementValue = GET_Z_ELEMENT_VALUE((*lcp_UcodeAddr)&0x0F);
            lcp_UcodeAddr += 2;

            lsv_StartIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                *llp_Value = 0;

                for(lsv_Temp=4*lcv_Kn; lsv_Temp>0; lsv_Temp--) {
                    lsv_ElementIndex = lsv_StartIndex + lsv_Temp + lsv_Offset*4*lcv_Kn -1;

                    if(lsv_ElementIndex < S_RANG) {
                        *llp_Value = (*llp_Value << 1) + plc_get_bit_element_value(S_ELEMENT, lsv_ElementIndex);
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                llp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_KnLMZ:
            lcv_Kn = ((*lcp_UcodeAddr)&0xF0)>>4;
            if((lcv_Kn<1)||(lcv_Kn>8)) {
                return ERR_OPERANDS;
            }

            lsv_ZElementValue = GET_Z_ELEMENT_VALUE((*lcp_UcodeAddr)&0x0F);
            lcp_UcodeAddr += 2;

            lsv_StartIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                *llp_Value = 0;

                for(lsv_Temp=4*lcv_Kn; lsv_Temp>0; lsv_Temp--) {
                    lsv_ElementIndex = lsv_StartIndex + lsv_Temp + lsv_Offset*4*lcv_Kn -1;

                    if(lsv_ElementIndex < LM_RANG) {
                        *llp_Value = (*llp_Value << 1) + plc_get_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex);
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                llp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        default:
            return ERR_ELEMENT_TYPE;
    }
}

/**
  * @brief  获取双字元件地址
  * @param  lcp_UcodeAddr   UCODE取值开始地址
  *         llp_Addr  返回连续lsv_Count个元件开始地址值指针
  *         lsv_Num   元件个数
  * @retval None
  */
unsigned char get_dword_addr(unsigned char *lcp_UcodeAddr, unsigned long **llp_Addr, unsigned short lsv_Num)
{
    /*元件下标号*/
    unsigned short lsv_ElementIndex;
    /*变址寻址中Z元件值*/
    unsigned short lsv_ZElementValue;

    switch(GET_PU8_DATA(lcp_UcodeAddr+1)) {

        case ADDR_D:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr);

            if((lsv_ElementIndex & 0x01) || ((lsv_ElementIndex + 2*lsv_Num) > D_RANG)) {
                return ERR_OVER_ELEMENT_RANG;
            }

            *llp_Addr = (unsigned long *)&GET_D_ELEMENT_VALUE(lsv_ElementIndex);
            break;

        case ADDR_R:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr);

            if((lsv_ElementIndex & 0x01) || ((lsv_ElementIndex + 2*lsv_Num) > R_RANG)) {
                return ERR_OVER_ELEMENT_RANG;
            }

            *llp_Addr = (unsigned long *)&GET_R_ELEMENT_VALUE(lsv_ElementIndex);
            break;

        case ADDR_DZ:
            lcp_UcodeAddr ++;
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);
            lcp_UcodeAddr ++;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr);

            if((lsv_ElementIndex & 0x01) || ((lsv_ElementIndex + 2*lsv_Num) > D_RANG)) {
                return ERR_OVER_ELEMENT_RANG;
            }

            *llp_Addr = (unsigned long *)&GET_D_ELEMENT_VALUE(lsv_ElementIndex);
            break;

        case ADDR_RZ:
            lcp_UcodeAddr ++;
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);
            lcp_UcodeAddr ++;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr);

            if((lsv_ElementIndex & 0x01) || ((lsv_ElementIndex + 2*lsv_Num) > R_RANG)) {
                return ERR_OVER_ELEMENT_RANG;
            }

            *llp_Addr = (unsigned long *)&GET_R_ELEMENT_VALUE(lsv_ElementIndex);
            break;

        default:
            return ERR_ELEMENT_TYPE;

    }

    return pdPASS;
}

/**
  * @brief  保存双字元件值
  * @param  lcp_UcodeAddr   UCODE取值开始地址
  *         llp_Value   保存值指针
  *         lsv_Offset  偏移量
  *         lsv_Count   读取个数
  * @retval None
  */
unsigned char save_dword(unsigned char *lcp_UcodeAddr, unsigned long *llp_Value, unsigned short lsv_Offset, unsigned short lsv_Count)
{
    unsigned char lcv_Kn;
    unsigned short lsv_Temp;
    /*元件下标号*/
    unsigned short lsv_ElementIndex;
    unsigned short lsv_StartIndex;

    /*变址寻址中Z元件值*/
    unsigned short lsv_ZElementValue;

    switch(GET_PU8_DATA(lcp_UcodeAddr+1)) {

        case ADDR_KnY:
            lcv_Kn = *lcp_UcodeAddr;
            if(lcv_Kn<5 || lcv_Kn>8) {
                return ERR_OPERANDS;
            }

            lcp_UcodeAddr += 2;
            lsv_StartIndex = GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                for(lsv_Temp=0; lsv_Temp<4*lcv_Kn; lsv_Temp++) {
                    lsv_ElementIndex = lsv_Temp + lsv_StartIndex + lsv_Offset*4*lcv_Kn;

                    if(lsv_ElementIndex < Y_RANG) {
                        plc_set_bit_element_value(Y_ELEMENT, lsv_ElementIndex, (unsigned char)(*llp_Value & 0x01));
                        *llp_Value >>= 0x01;

                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                llp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_KnM:
            lcv_Kn = *lcp_UcodeAddr;
            if(lcv_Kn<5 || lcv_Kn>8) {
                return ERR_OPERANDS;
            }

            lcp_UcodeAddr += 2;
            lsv_StartIndex = GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                for(lsv_Temp=0; lsv_Temp<4*lcv_Kn; lsv_Temp++) {
                    lsv_ElementIndex = lsv_Temp + lsv_StartIndex + lsv_Offset*4*lcv_Kn;

                    if(lsv_ElementIndex < M_RANG) {
                        plc_set_bit_element_value(M_ELEMENT, lsv_ElementIndex, (unsigned char)(*llp_Value & 0x01));
                        *llp_Value >>= 0x01;

                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                llp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_KnS:
            lcv_Kn = *lcp_UcodeAddr;
            if(lcv_Kn<5 || lcv_Kn>8) {
                return ERR_OPERANDS;
            }

            lcp_UcodeAddr += 2;
            lsv_StartIndex = GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                for(lsv_Temp=0; lsv_Temp<4*lcv_Kn; lsv_Temp++) {
                    lsv_ElementIndex = lsv_Temp + lsv_StartIndex + lsv_Offset*4*lcv_Kn;

                    if(lsv_ElementIndex < S_RANG) {
                        plc_set_bit_element_value(S_ELEMENT, lsv_ElementIndex, (unsigned char)(*llp_Value & 0x01));
                        *llp_Value >>= 0x01;

                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                llp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_KnLM:
            lcv_Kn = *lcp_UcodeAddr;
            if(lcv_Kn<5 || lcv_Kn>8) {
                return ERR_OPERANDS;
            }

            lcp_UcodeAddr += 2;
            lsv_StartIndex = GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                for(lsv_Temp=0; lsv_Temp<4*lcv_Kn; lsv_Temp++) {
                    lsv_ElementIndex = lsv_Temp + lsv_StartIndex + lsv_Offset*4*lcv_Kn;

                    if(lsv_ElementIndex < LM_RANG) {
                        plc_set_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex, (unsigned char)(*llp_Value & 0x01));
                        *llp_Value >>= 0x01;

                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                llp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_D:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for( ; lsv_Count>0; lsv_Count--) {
                if(lsv_ElementIndex < (D_RANG -1)) {
                    SET_D_ELEMENT_VALUE(lsv_ElementIndex, (unsigned short)(*llp_Value & 0x0000FFFF));
                    SET_D_ELEMENT_VALUE(lsv_ElementIndex+1, (unsigned short)(*llp_Value >> 16));
                    lsv_ElementIndex += 2;
                    llp_Value++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_R:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for( ; lsv_Count>0; lsv_Count--) {
                if(lsv_ElementIndex < (R_RANG -1)) {
                    SET_R_ELEMENT_VALUE(lsv_ElementIndex, (unsigned short)(*llp_Value & 0x0000FFFF));
                    SET_R_ELEMENT_VALUE(lsv_ElementIndex+1, (unsigned short)(*llp_Value >> 16));
                    lsv_ElementIndex += 2;
                    llp_Value++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_SD:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for( ; lsv_Count>0; lsv_Count--) {
                if(lsv_ElementIndex < (SD_RANG -1)) {
                    SET_SD_ELEMENT_VALUE(lsv_ElementIndex, (unsigned short)(*llp_Value & 0x0000FFFF));
                    SET_SD_ELEMENT_VALUE(lsv_ElementIndex+1, (unsigned short)(*llp_Value >> 16));
                    lsv_ElementIndex += 2;
                    llp_Value++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_C:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset;

            for( ; lsv_Count>0; lsv_Count--) {
                if(lsv_ElementIndex < C_RANG) {
                    SET_C32_CURRENT_VALUE(lsv_ElementIndex, *llp_Value);
                    lsv_ElementIndex ++;
                    llp_Value++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_V:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for( ; lsv_Count>0; lsv_Count--) {
                if(lsv_ElementIndex < (V_RANG -1)) {
                    SET_V_ELEMENT_VALUE(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex, (unsigned short)(*llp_Value & 0x0000FFFF));
                    SET_V_ELEMENT_VALUE(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex+1, (unsigned short)(*llp_Value >> 16));
                    lsv_ElementIndex += 2;
                    llp_Value++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_DZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for( ; lsv_Count>0; lsv_Count--) {
                if(lsv_ElementIndex < (D_RANG -1)) {
                    SET_D_ELEMENT_VALUE(lsv_ElementIndex, (unsigned short)(*llp_Value & 0x0000FFFF));
                    SET_D_ELEMENT_VALUE(lsv_ElementIndex+1, (unsigned short)(*llp_Value >> 16));
                    lsv_ElementIndex += 2;
                    llp_Value++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_RZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for( ; lsv_Count>0; lsv_Count--) {
                if(lsv_ElementIndex < (R_RANG -1)) {
                    SET_R_ELEMENT_VALUE(lsv_ElementIndex, (unsigned short)(*llp_Value & 0x0000FFFF));
                    SET_R_ELEMENT_VALUE(lsv_ElementIndex+1, (unsigned short)(*llp_Value >> 16));
                    lsv_ElementIndex += 2;
                    llp_Value++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_CZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for( ; lsv_Count>0; lsv_Count--) {
                if(lsv_ElementIndex < C_RANG) {
                    SET_C32_CURRENT_VALUE(lsv_ElementIndex, *llp_Value);
                    lsv_ElementIndex ++;
                    llp_Value++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_VZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for( ; lsv_Count>0; lsv_Count--) {
                if(lsv_ElementIndex < (V_RANG -1)) {
                    SET_V_ELEMENT_VALUE(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex, (unsigned short)(*llp_Value & 0x0000FFFF));
                    SET_V_ELEMENT_VALUE(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex+1, (unsigned short)(*llp_Value >> 16));
                    lsv_ElementIndex += 2;
                    llp_Value++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_KnYZ:
            lcv_Kn = ((*lcp_UcodeAddr)&0xF0)>>4;
            if((lcv_Kn<5)||(lcv_Kn>8)) {
                return ERR_OPERANDS;
            }

            lsv_ZElementValue = GET_Z_ELEMENT_VALUE((*lcp_UcodeAddr)&0x0F);
            lcp_UcodeAddr += 2;

            lsv_StartIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                for(lsv_Temp=0; lsv_Temp<4*lcv_Kn; lsv_Temp++) {
                    lsv_ElementIndex = lsv_Temp + lsv_StartIndex + lsv_Offset*4*lcv_Kn;

                    if(lsv_ElementIndex < Y_RANG) {
                        plc_set_bit_element_value(Y_ELEMENT, lsv_ElementIndex, (unsigned char)(*llp_Value & 0x01));
                        *llp_Value >>= 0x01;
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                llp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_KnMZ:
            lcv_Kn = ((*lcp_UcodeAddr)&0xF0)>>4;
            if((lcv_Kn<5)||(lcv_Kn>8)) {
                return ERR_OPERANDS;
            }

            lsv_ZElementValue = GET_Z_ELEMENT_VALUE((*lcp_UcodeAddr)&0x0F);
            lcp_UcodeAddr += 2;

            lsv_StartIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                for(lsv_Temp=0; lsv_Temp<4*lcv_Kn; lsv_Temp++) {
                    lsv_ElementIndex = lsv_Temp + lsv_StartIndex + lsv_Offset*4*lcv_Kn;

                    if(lsv_ElementIndex < M_RANG) {
                        plc_set_bit_element_value(M_ELEMENT, lsv_ElementIndex, (unsigned char)(*llp_Value & 0x01));
                        *llp_Value >>= 0x01;
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                llp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_KnSZ:
            lcv_Kn = ((*lcp_UcodeAddr)&0xF0)>>4;
            if((lcv_Kn<5)||(lcv_Kn>8)) {
                return ERR_OPERANDS;
            }

            lsv_ZElementValue = GET_Z_ELEMENT_VALUE((*lcp_UcodeAddr)&0x0F);
            lcp_UcodeAddr += 2;

            lsv_StartIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                for(lsv_Temp=0; lsv_Temp<4*lcv_Kn; lsv_Temp++) {
                    lsv_ElementIndex = lsv_Temp + lsv_StartIndex + lsv_Offset*4*lcv_Kn;

                    if(lsv_ElementIndex < S_RANG) {
                        plc_set_bit_element_value(S_ELEMENT, lsv_ElementIndex, (unsigned char)(*llp_Value & 0x01));
                        *llp_Value >>= 0x01;
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                llp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        case ADDR_KnLMZ:
            lcv_Kn = ((*lcp_UcodeAddr)&0xF0)>>4;
            if((lcv_Kn<5)||(lcv_Kn>8)) {
                return ERR_OPERANDS;
            }

            lsv_ZElementValue = GET_Z_ELEMENT_VALUE((*lcp_UcodeAddr)&0x0F);
            lcp_UcodeAddr += 2;

            lsv_StartIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr);

            for(; lsv_Count>0; lsv_Count--) {

                for(lsv_Temp=0; lsv_Temp<4*lcv_Kn; lsv_Temp++) {
                    lsv_ElementIndex = lsv_Temp + lsv_StartIndex + lsv_Offset*4*lcv_Kn;

                    if(lsv_ElementIndex < LM_RANG) {
                        plc_set_lm_element_value(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex, (unsigned char)(*llp_Value & 0x01));
                        *llp_Value >>= 0x01;
                    } else {
                        return ERR_OVER_ELEMENT_RANG;
                    }
                }
                llp_Value++;
                lsv_Offset++;
            }
            return pdPASS;

        default:
            return ERR_ELEMENT_TYPE;
    }
}

/**
  * @brief  获取浮点数变量
  * @param  lcp_UcodeAddr   UCODE取值开始地址
  *         lfp_Value   返回值指针
  *         lsv_Offset  偏移量
  *         lsv_Count   读取个数
  * @retval None
  */
unsigned char get_float(unsigned char *lcp_UcodeAddr, float *lfp_Value, unsigned short lsv_Offset, unsigned short lsv_Count)
{
    /*元件下标号*/
    unsigned short lsv_ElementIndex;

    /*变址寻址中Z元件值*/
    unsigned short lsv_ZElementValue;

    switch(GET_PU8_DATA(lcp_UcodeAddr+1)) {

        case ADDR_CONST:
            if( *lcp_UcodeAddr != 4) {
                return ERR_ELEMENT_TYPE;
            } else {
                lcp_UcodeAddr += 2;
                *(unsigned long *)lfp_Value = GET_PU32_DATA(lcp_UcodeAddr);
            }
            return pdPASS;

        case ADDR_D:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for(; lsv_Count>0; lsv_Count--) {
                if(lsv_ElementIndex < (D_RANG-1)) {
                    *(unsigned long *)lfp_Value = GET_D_ELEMENT_VALUE(lsv_ElementIndex) + (GET_D_ELEMENT_VALUE(lsv_ElementIndex + 1) <<16);
                    lfp_Value++;
                    lsv_ElementIndex += 2;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_R:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for(; lsv_Count>0; lsv_Count--) {
                if(lsv_ElementIndex < (R_RANG-1)) {
                    *(unsigned long *)lfp_Value = GET_R_ELEMENT_VALUE(lsv_ElementIndex) + (GET_R_ELEMENT_VALUE(lsv_ElementIndex + 1) <<16);
                    lfp_Value++;
                    lsv_ElementIndex += 2;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_V:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for(; lsv_Count>0; lsv_Count--) {
                if(lsv_ElementIndex < (V_RANG-1)) {
                    *(unsigned long *)lfp_Value = GET_V_ELEMENT_VALUE(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex) + (GET_V_ELEMENT_VALUE(gtp_CallInsInfoPtr->msv_SbrNestedNum, (lsv_ElementIndex + 1)) <<16);
                    lfp_Value++;
                    lsv_ElementIndex += 2;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }

            }
            return pdPASS;

        case ADDR_DZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < (D_RANG-1)) {
                    *(unsigned long *)lfp_Value = (GET_D_ELEMENT_VALUE(lsv_ElementIndex+1)<<16) + GET_D_ELEMENT_VALUE(lsv_ElementIndex);
                    lfp_Value++;
                    lsv_ElementIndex+=2;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_RZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < (R_RANG-1)) {
                    *(unsigned long *)lfp_Value = (GET_R_ELEMENT_VALUE(lsv_ElementIndex+1)<<16) + GET_R_ELEMENT_VALUE(lsv_ElementIndex);
                    lfp_Value++;
                    lsv_ElementIndex+=2;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_VZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for( ; lsv_Count>0; lsv_Count--) {

                if(lsv_ElementIndex < (V_RANG-1)) {
                    *(unsigned long *)lfp_Value = (GET_V_ELEMENT_VALUE(gtp_CallInsInfoPtr->msv_SbrNestedNum, (lsv_ElementIndex+1)) <<16) + GET_V_ELEMENT_VALUE(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex);
                    lfp_Value++;
                    lsv_ElementIndex+=2;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        default:
            return ERR_ELEMENT_TYPE;
    }
}

/**
  * @brief  保存浮点数变量
  * @param  lcp_UcodeAddr   UCODE取值开始地址
  *         lfp_Value   保存值指针
  *         lsv_Offset  偏移量
  *         lsv_Count   读取个数
  * @retval None
  */
unsigned char save_float(unsigned char *lcp_UcodeAddr, float *lfp_Value, unsigned short lsv_Offset, unsigned short lsv_Count)
{
    /*元件下标号*/
    unsigned short lsv_ElementIndex;

    /*变址寻址中Z元件值*/
    unsigned short lsv_ZElementValue;

    switch(GET_PU8_DATA(lcp_UcodeAddr+1)) {

        case ADDR_D:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for(; lsv_Count>0; lsv_Count--) {
                if(lsv_ElementIndex < (D_RANG-1)) {
                    SET_D_ELEMENT_VALUE(lsv_ElementIndex, (unsigned short)(*(unsigned long *)lfp_Value & 0x0000FFFF));
                    SET_D_ELEMENT_VALUE(lsv_ElementIndex+1, (unsigned short)(*(unsigned long *)lfp_Value >> 16));

                    lfp_Value++;
                    lsv_ElementIndex += 2;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_R:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for(; lsv_Count>0; lsv_Count--) {
                if(lsv_ElementIndex < (R_RANG-1)) {
                    SET_R_ELEMENT_VALUE(lsv_ElementIndex, (unsigned short)(*(unsigned long *)lfp_Value & 0x0000FFFF));
                    SET_R_ELEMENT_VALUE(lsv_ElementIndex+1, (unsigned short)(*(unsigned long *)lfp_Value >> 16));

                    lfp_Value++;
                    lsv_ElementIndex += 2;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_V:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for(; lsv_Count>0; lsv_Count--) {
                if(lsv_ElementIndex < (V_RANG-1)) {
                    SET_V_ELEMENT_VALUE(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex, (unsigned short)(*(unsigned long *)lfp_Value & 0x0000FFFF));
                    SET_V_ELEMENT_VALUE(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex+1, (unsigned short)(*(unsigned long *)lfp_Value >> 16));

                    lfp_Value++;
                    lsv_ElementIndex += 2;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_DZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for( ; lsv_Count>0; lsv_Count--) {
                if(lsv_ElementIndex < (D_RANG -1)) {
                    SET_D_ELEMENT_VALUE(lsv_ElementIndex, (unsigned short)(*(unsigned long *)lfp_Value & 0x0000FFFF));
                    SET_D_ELEMENT_VALUE(lsv_ElementIndex+1, (unsigned short)(*(unsigned long *)lfp_Value >> 16));
                    lsv_ElementIndex += 2;
                    lfp_Value++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_RZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for( ; lsv_Count>0; lsv_Count--) {
                if(lsv_ElementIndex < (R_RANG -1)) {
                    SET_R_ELEMENT_VALUE(lsv_ElementIndex, (unsigned short)(*(unsigned long *)lfp_Value & 0x0000FFFF));
                    SET_R_ELEMENT_VALUE(lsv_ElementIndex+1, (unsigned short)(*(unsigned long *)lfp_Value >> 16));
                    lsv_ElementIndex += 2;
                    lfp_Value++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        case ADDR_VZ:
            lsv_ZElementValue = GET_Z_ELEMENT_VALUE(*lcp_UcodeAddr);

            lcp_UcodeAddr += 2;

            lsv_ElementIndex = lsv_ZElementValue + GET_PU16_DATA(lcp_UcodeAddr) + lsv_Offset*2;

            for( ; lsv_Count>0; lsv_Count--) {
                if(lsv_ElementIndex < (V_RANG -1)) {
                    SET_V_ELEMENT_VALUE(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex, (unsigned short)(*(unsigned long *)lfp_Value & 0x0000FFFF));
                    SET_V_ELEMENT_VALUE(gtp_CallInsInfoPtr->msv_SbrNestedNum, lsv_ElementIndex+1, (unsigned short)(*(unsigned long *)lfp_Value >> 16));
                    lsv_ElementIndex += 2;
                    lfp_Value++;
                } else {
                    return ERR_OVER_ELEMENT_RANG;
                }
            }
            return pdPASS;

        default:
            return ERR_ELEMENT_TYPE;
    }
}

unsigned char get_char_in_simpleIns(unsigned char *lcp_UcodeAddr, unsigned char *lcp_Value)
{
    /*元件下标号*/
    unsigned short lsv_ElementIndex;

    switch(GET_PU8_DATA(lcp_UcodeAddr)) {
        case ADDR_X:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr);
             *lcp_Value = plc_get_bit_element_value(X_ELEMENT, lsv_ElementIndex);
            return pdPASS;

        case ADDR_Y:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr);
            *lcp_Value = plc_get_bit_element_value(Y_ELEMENT, lsv_ElementIndex);
            return pdPASS;

        case ADDR_bC:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr);
            *lcp_Value = plc_get_bit_element_value(C_ELEMENT, lsv_ElementIndex);
            return pdPASS;

        case ADDR_bT:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr);
            *lcp_Value = plc_get_bit_element_value(T_ELEMENT, lsv_ElementIndex);
            return pdPASS;

        case ADDR_SM:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr);
            *lcp_Value = plc_get_bit_element_value(SM_ELEMENT, lsv_ElementIndex);
            return pdPASS;

        case ADDR_M:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr);
            *lcp_Value = plc_get_bit_element_value(M_ELEMENT, lsv_ElementIndex);
            return pdPASS;

        case ADDR_S:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr);
            *lcp_Value = plc_get_bit_element_value(S_ELEMENT, lsv_ElementIndex);
            return pdPASS;

        case ADDR_LM:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr);
            *lcp_Value = plc_get_lm_element_value((gtp_CallInsInfoPtr->msv_SbrNestedNum+1), lsv_ElementIndex);
            return pdPASS;

        default:
            return ERR_ELEMENT_TYPE;
    }
}

unsigned char save_char_in_simpleIns(unsigned char *lcp_UcodeAddr, unsigned char lcp_Value)
{
    /*元件下标号*/
    unsigned short lsv_ElementIndex;
    switch(GET_PU8_DATA(lcp_UcodeAddr)) {
        case ADDR_X:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr);
            plc_set_bit_element_value(X_ELEMENT, lsv_ElementIndex, lcp_Value);
            return pdPASS;
        case ADDR_Y:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr);
            plc_set_bit_element_value(Y_ELEMENT, lsv_ElementIndex, lcp_Value);
            return pdPASS;
        case ADDR_bC:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr);
            plc_set_bit_element_value(C_ELEMENT, lsv_ElementIndex, lcp_Value);
            return pdPASS;
        case ADDR_bT:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr);
            plc_set_bit_element_value(T_ELEMENT, lsv_ElementIndex, lcp_Value);
            return pdPASS;
        case ADDR_SM:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr);
            plc_set_bit_element_value(SM_ELEMENT, lsv_ElementIndex, lcp_Value);
            return pdPASS;
        case ADDR_M:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr);
            plc_set_bit_element_value(M_ELEMENT, lsv_ElementIndex, lcp_Value);
            return pdPASS;
        case ADDR_S:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr);
            plc_set_bit_element_value(S_ELEMENT, lsv_ElementIndex, lcp_Value);
            return pdPASS;
        case ADDR_LM:
            lcp_UcodeAddr += 2;
            lsv_ElementIndex = GET_PU16_DATA(lcp_UcodeAddr);
            plc_set_lm_element_value((gtp_CallInsInfoPtr->msv_SbrNestedNum+1), lsv_ElementIndex, lcp_Value);
            return pdPASS;
        default:
            return ERR_ELEMENT_TYPE;
    }
}
