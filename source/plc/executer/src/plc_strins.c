/**
  ******************************************************************************
  * @file    plc_strins.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   字符串相关指令函数
  ******************************************************************************
  */
#include "plc_errormsg.h"
#include "plc_commonfunc.h"
#include "plc_variable.h"
#include "plc_instruction.h"
#include "plc_parseaddr.h"
#include "plc_element.h"
#include "plc_dataprocessing.h"
#include "plc_counterins.h"
#include "plc_timeins.h"

/**
  * @brief  字符串结合
  * @param  None
  * @retval None
  */
unsigned char run_ci_stradd_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    short lsv_Str1Len=0, lsv_Str2Len=0;
    unsigned char *lcp_DestStrPtr, lcv_Str1UcodeLen, lcv_Str2UcodeLen;
    unsigned short lsv_Temp;
    unsigned char i;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    /*获取第一个字符串长度*/
    if(GET_PU8_DATA(ltp_RunEnv->mcp_PC+3) == ADDR_STR) {
        lsv_Str1Len = GET_PU8_DATA(ltp_RunEnv->mcp_PC+2);
        lcv_Str1UcodeLen = lsv_Str1Len;
        /*减去字符串结尾0x00*/
        lsv_Str1Len --;
    } else {
        do {
            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Temp, lsv_Str1Len/2, 1);
            if(lcv_Ret != pdPASS) {
                return lcv_Ret;
            }

            if((lsv_Temp & 0x00FF) == 0x00) {
                break;
            }

            lsv_Str1Len ++;

            if(((lsv_Temp>>8) & 0x00FF) == 0x00) {
                break;
            }

            lsv_Str1Len ++;
        } while(1);

        lcv_Str1UcodeLen = 2;
    }

    if(lsv_Str1Len > 32) {
        return ERR_OPERANDS;
    }

    /*获取第一个字符串长度*/
    if(GET_PU8_DATA(ltp_RunEnv->mcp_PC+4+lcv_Str1UcodeLen+1) == ADDR_STR) {
        lsv_Str2Len = GET_PU8_DATA(ltp_RunEnv->mcp_PC+4+lcv_Str1UcodeLen);
        lcv_Str2UcodeLen = lsv_Str2Len;
        /*减去字符串结尾0x00*/
        lsv_Str2Len --;
    } else {
        do {
            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+4+lcv_Str1UcodeLen, &lsv_Temp, lsv_Str2Len/2, 1);
            if(lcv_Ret != pdPASS) {
                return lcv_Ret;
            }

            if((lsv_Temp & 0x00FF) == 0x00) {
                break;
            }

            lsv_Str2Len ++;

            if(((lsv_Temp>>8) & 0x00FF) == 0x00) {
                break;
            }

            lsv_Str2Len ++;
        } while(1);

        lcv_Str2UcodeLen = 2;
    }

    if(lsv_Str2Len > 32) {
        return ERR_OPERANDS;
    }

    /*分配内存*/
    lcp_DestStrPtr = (unsigned char *)pvPortMalloc(lsv_Str1Len + lsv_Str2Len +2);
    configASSERT(lcp_DestStrPtr != NULL);

    /*复制第一个字符串*/
    if(GET_PU8_DATA(ltp_RunEnv->mcp_PC+3) == ADDR_STR) {
        for(i=0; i<lsv_Str1Len; i++) {
            lcp_DestStrPtr[i] = GET_PU8_DATA(ltp_RunEnv->mcp_PC+4+i);
        }
    } else {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lcp_DestStrPtr[0], 0, (lsv_Str1Len+1)/2);
        if(lcv_Ret != pdPASS) {
            vPortFree(lcp_DestStrPtr);
            return lcv_Ret;
        }
    }

    /*复制第二个字符串*/
    if(GET_PU8_DATA(ltp_RunEnv->mcp_PC+4+lcv_Str1UcodeLen+1) == ADDR_STR) {
        for(i=0; i< lsv_Str2Len; i++) {
            lcp_DestStrPtr[lsv_Str1Len + i] = GET_PU8_DATA(ltp_RunEnv->mcp_PC+6+lcv_Str1UcodeLen+i);
        }
    } else {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+4+lcv_Str1UcodeLen, (unsigned short *)&lcp_DestStrPtr[lsv_Str1Len], 0, (lsv_Str2Len+1)/2);
        if(lcv_Ret != pdPASS) {
            vPortFree(lcp_DestStrPtr);
            return lcv_Ret;
        }
    }

    if((lsv_Str1Len + lsv_Str2Len)%2) {
        lcp_DestStrPtr[lsv_Str1Len + lsv_Str2Len] = 0x00;
        lcv_Ret = save_word(ltp_RunEnv->mcp_PC + 6 + lcv_Str2UcodeLen + lcv_Str1UcodeLen, (unsigned short *)&lcp_DestStrPtr[0], 0, (lsv_Str1Len + lsv_Str2Len + 1)/2);
    } else {
        *(unsigned short *)&lcp_DestStrPtr[(lsv_Str1Len + lsv_Str2Len)/2] = 0x0000;
        lcv_Ret = save_word(ltp_RunEnv->mcp_PC + 6 + lcv_Str2UcodeLen + lcv_Str1UcodeLen, (unsigned short *)&lcp_DestStrPtr[0], 0, (lsv_Str1Len + lsv_Str2Len + 2)/2);
    }


    vPortFree(lcp_DestStrPtr);

    return lcv_Ret;
}

/**
  * @brief  字符串长度
  * @param  None
  * @retval None
  */
unsigned char run_ci_strlen_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    unsigned short lsv_Strlen = 0;
    unsigned short lsv_Temp;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    do {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Temp, lsv_Strlen/2, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        if((lsv_Temp&0xFF) == 0x00) {
            break;
        }
        lsv_Strlen += 1;

        if(((lsv_Temp>>8)&0xFF) == 0x00) {
            break;
        }
        lsv_Strlen += 1;

    } while(1);

    if(lsv_Strlen >= 32767) {
        return ERR_OPERANDS;
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, &lsv_Strlen, 0, 1);
    return lcv_Ret;
}

/**
  * @brief  从字符串右侧读取字符串
  * @param  None
  * @retval None
  */
unsigned char run_ci_strright_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    unsigned short lsv_DestStrLen, lsv_SrcStrLen;
    unsigned short lsv_Temp, lsv_Temp2;
    unsigned short i;
    unsigned char *lcp_TempBuff;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, &lsv_DestStrLen, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_DestStrLen > 32767) {
        return ERR_OPERANDS;
    }

    if(lsv_DestStrLen == 0) {
        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, &lsv_DestStrLen, 0, 1);
        return lcv_Ret;
    }

    /*取源字符串长度*/
    do {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+1, &lsv_Temp, lsv_SrcStrLen/2, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        if((lsv_Temp&0xFF) == 0x00) {
            break;
        }
        lsv_SrcStrLen ++;

        if(((lsv_Temp>>8)&0xFF) == 0x00) {
            break;
        }
        lsv_SrcStrLen ++;

    } while(1);

    /*目标取出长度大于源长度*/
    if(lsv_DestStrLen > lsv_SrcStrLen) {
        return ERR_OPERANDS;
    }

    /*申请临时buff存放需要复制的字符串*/
    lcp_TempBuff = (unsigned char *)pvPortMalloc(lsv_DestStrLen+2);
    configASSERT(lcp_TempBuff != NULL);

    /*读字符串*/
    /*取出的第一个字符在双字的高字节*/
    if((lsv_SrcStrLen-lsv_DestStrLen)%2) {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Temp2, (lsv_SrcStrLen-lsv_DestStrLen)/2, 1);
        if(lcv_Ret != pdPASS) {
            vPortFree(lcp_TempBuff);
            return lcv_Ret;
        }

        lcp_TempBuff[0] = (lsv_Temp2 >> 8)&0xFF;

        for(i=1; i< lsv_DestStrLen; i+=2) {
            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Temp, (lsv_SrcStrLen-lsv_DestStrLen + i)/2, 1);
            if(lcv_Ret != pdPASS) {
                vPortFree(lcp_TempBuff);
                return lcv_Ret;
            }

            *(unsigned short *)&lcp_TempBuff[i] = lsv_Temp;
        }

    } else {
        for(i=0; i< lsv_DestStrLen; i+=2) {
            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Temp, (lsv_SrcStrLen-lsv_DestStrLen + i)/2, 1);
            if(lcv_Ret != pdPASS) {
                vPortFree(lcp_TempBuff);
                return lcv_Ret;
            }

            *(unsigned short *)&lcp_TempBuff[i] = lsv_Temp;
        }
    }

    if(lsv_DestStrLen % 2) {
        lcp_TempBuff[lsv_DestStrLen] = 0x00;
    } else {
        *(unsigned short *)&lcp_TempBuff[lsv_DestStrLen/2] = 0x00;
    }

    /*保存*/
    if(lsv_DestStrLen % 2) {
        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lcp_TempBuff[0], 0, (lsv_DestStrLen+1)/2);
    } else {
        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lcp_TempBuff[0], 0, (lsv_DestStrLen+2)/2);
    }

    /*释放内存*/
    vPortFree(lcp_TempBuff);

    return lcv_Ret;
}

/**
  * @brief  从字符串左侧读取字符串
  * @param  None
  * @retval None
  */
unsigned char run_ci_strleft_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    unsigned short lsv_DestStrLen, lsv_SrcStrLen;
    unsigned short lsv_Temp;
    unsigned short i;
    unsigned char *lcp_TempBuff;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, &lsv_DestStrLen, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_DestStrLen > 32767) {
        return ERR_OPERANDS;
    }

    if(lsv_DestStrLen == 0) {
        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, &lsv_DestStrLen, 0, 1);
        return lcv_Ret;
    }

    /*取源字符串长度*/
    do {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+1, &lsv_Temp, lsv_SrcStrLen/2, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        if((lsv_Temp&0xFF) == 0x00) {
            break;
        }
        lsv_SrcStrLen ++;

        if(((lsv_Temp>>8)&0xFF) == 0x00) {
            break;
        }
        lsv_SrcStrLen ++;

    } while(1);

    /*目标取出长度大于源长度*/
    if(lsv_DestStrLen > lsv_SrcStrLen) {
        return ERR_OPERANDS;
    }

    /*申请临时buff存放需要复制的字符串*/
    lcp_TempBuff = (unsigned char *)pvPortMalloc(lsv_DestStrLen+2);
    configASSERT(lcp_TempBuff != NULL);

    /*读字符串*/
    for(i=0; i< lsv_DestStrLen; i+=2) {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Temp, (lsv_SrcStrLen-lsv_DestStrLen + i)/2, 1);
        if(lcv_Ret != pdPASS) {
            vPortFree(lcp_TempBuff);
            return lcv_Ret;
        }

        *(unsigned short *)&lcp_TempBuff[i] = lsv_Temp;
    }

    if(lsv_DestStrLen % 2) {
        lcp_TempBuff[lsv_DestStrLen] = 0x00;
    } else {
        *(unsigned short *)&lcp_TempBuff[lsv_DestStrLen/2] = 0x00;
    }

    /*保存*/
    if(lsv_DestStrLen % 2) {
        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lcp_TempBuff[0], 0, (lsv_DestStrLen+1)/2);
    } else {
        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lcp_TempBuff[0], 0, (lsv_DestStrLen+2)/2);
    }

    /*释放内存*/
    vPortFree(lcp_TempBuff);

    return lcv_Ret;
}

/**
  * @brief  从字符串中任意读取
  * @param  None
  * @retval None
  */
unsigned char run_ci_strmidr_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    unsigned char *lcp_TempBuff;
    unsigned short lsv_SrcStrLen=0, lsv_Temp;
    short lsv_StartIndex, lsv_ReadLen;
    unsigned short i;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsv_StartIndex, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_StartIndex < 1) {
        return ERR_OPERANDS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsv_ReadLen, 1, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*获取源字符串长度*/
    do {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Temp, lsv_SrcStrLen/2, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        if((lsv_Temp&0xFF) == 0x00) {
            break;
        }
        lsv_SrcStrLen++;

        if(((lsv_Temp>>8)&0xFF) == 0x00) {
            break;
        }
        lsv_SrcStrLen++;
    } while(1);

    /*参数合法性判断*/
    if(lsv_ReadLen < -2) {
        return ERR_OPERANDS;
    } else if(lsv_ReadLen == 0) {
        return pdPASS;
    } else if(lsv_ReadLen == -1) {
        lsv_StartIndex = 0;
        lsv_ReadLen = lsv_SrcStrLen;
    }

    if(lsv_StartIndex > lsv_SrcStrLen) {
        return ERR_OPERANDS;
    }

    if(lsv_SrcStrLen - lsv_StartIndex < lsv_ReadLen) {
        return ERR_OPERANDS;
    }

    /*申请缓存区*/
    lcp_TempBuff = (unsigned char *)pvPortMalloc(lsv_ReadLen+2);
    configASSERT(lcp_TempBuff != NULL);

    if(lsv_StartIndex % 2) {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Temp, lsv_StartIndex/2, 1);
        if(lcv_Ret != pdPASS) {
            vPortFree(lcp_TempBuff);
            return lcv_Ret;
        }

        lcp_TempBuff[0] = (lsv_Temp>>8)& 0xFF;

        for(i=1; i<lsv_ReadLen; i+=2) {
            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Temp, (lsv_StartIndex+i)/2, 1);
            if(lcv_Ret != pdPASS) {
                vPortFree(lcp_TempBuff);
                return lcv_Ret;
            }
            *(unsigned short *)&lcp_TempBuff[i] = lsv_Temp;
        }


    } else {
        for(i=0; i<lsv_ReadLen; i+=2) {
            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Temp, (lsv_StartIndex+i)/2, 1);
            if(lcv_Ret != pdPASS) {
                vPortFree(lcp_TempBuff);
                return lcv_Ret;
            }
            *(unsigned short *)&lcp_TempBuff[i] = lsv_Temp;
        }
    }

    if(lsv_ReadLen % 2) {
        lcp_TempBuff[lsv_ReadLen]= 0x00;
    } else {
        *(unsigned short *)&lcp_TempBuff[lsv_ReadLen/2] = 0x0000;
    }

    /*保存*/
    if(lsv_ReadLen % 2) {
        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lcp_TempBuff[0], 0, (lsv_ReadLen+1)/2);
    } else {
        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lcp_TempBuff[0], 0, (lsv_ReadLen+2)/2);
    }

    /*释放内存*/
    vPortFree(lcp_TempBuff);

    return lcv_Ret;
}

/**
  * @brief  从字符串中任意替换
  * @param  None
  * @retval None
  */
unsigned char run_ci_strmidw_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    unsigned short lsv_Temp;
    unsigned char *lcp_TempBuff;
    short lsv_SrcStrLen = 0, lsv_DestStrLen = 0, lsv_RelpaceLen = 0, lsv_StartIndex;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsv_StartIndex, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_StartIndex < 1) {
        return ERR_OPERANDS;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsv_RelpaceLen, 1, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    /*获取源字符串长度*/
    do {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Temp, lsv_SrcStrLen/2, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        if((lsv_Temp&0xFF) == 0x00) {
            break;
        }
        lsv_SrcStrLen++;

        if(((lsv_Temp>>8)&0xFF) == 0x00) {
            break;
        }
        lsv_SrcStrLen++;
    } while(1);

    /*获取要替换字符串长度*/
    do {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, &lsv_Temp, lsv_DestStrLen/2, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        if((lsv_Temp&0xFF) == 0x00) {
            break;
        }
        lsv_DestStrLen++;

        if(((lsv_Temp>>8)&0xFF) == 0x00) {
            break;
        }
        lsv_DestStrLen++;
    } while(1);

    /*参数合法性校验*/
    if(lsv_RelpaceLen > lsv_SrcStrLen) {
        return ERR_OPERANDS;
    }

    if(lsv_RelpaceLen == 0) {
        return pdPASS;
    }

    if( lsv_RelpaceLen == -1) {
        lsv_RelpaceLen = lsv_SrcStrLen;
    }

    if(lsv_RelpaceLen > (lsv_DestStrLen - lsv_StartIndex + 1)) {
        lsv_RelpaceLen = lsv_DestStrLen - lsv_StartIndex + 1;
    }

    /*申请内存*/
    lcp_TempBuff = (unsigned char *)pvPortMalloc(lsv_DestStrLen+2);
    configASSERT(lcp_TempBuff != NULL);

    /*复制目标字符串,开始位置前字符*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lcp_TempBuff[0], 0, lsv_SrcStrLen/2);
    if(lcv_Ret != pdPASS) {
        vPortFree(lcp_TempBuff);
        return lcv_Ret;
    }

    /*复制源字符串中数据*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lcp_TempBuff[lsv_StartIndex-1], 0, (lsv_RelpaceLen+1)/2);
    if(lcv_Ret != pdPASS) {
        vPortFree(lcp_TempBuff);
        return lcv_Ret;
    }

    /*复制目标字符串剩余字符*/
    if(lsv_DestStrLen - lsv_StartIndex +1 > lsv_RelpaceLen) {
        if((lsv_StartIndex + lsv_RelpaceLen) % 2) {
            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, &lsv_Temp, (lsv_StartIndex + lsv_RelpaceLen)/2, 1);
            if(lcv_Ret != pdPASS) {
                vPortFree(lcp_TempBuff);
                return lcv_Ret;
            }

            lcp_TempBuff[lsv_StartIndex + lsv_RelpaceLen-1] = (unsigned char)(lsv_Temp >> 8)&0xFF;

            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lcp_TempBuff[lsv_StartIndex + lsv_RelpaceLen], (lsv_StartIndex + lsv_RelpaceLen+1)/2, (lsv_DestStrLen - (lsv_StartIndex + lsv_RelpaceLen))/2);
            if(lcv_Ret != pdPASS) {
                vPortFree(lcp_TempBuff);
                return lcv_Ret;
            }
        } else {
            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lcp_TempBuff[lsv_StartIndex + lsv_RelpaceLen-1], (lsv_StartIndex + lsv_RelpaceLen)/2, (lsv_DestStrLen - (lsv_StartIndex + lsv_RelpaceLen) +1)/2);
            if(lcv_Ret != pdPASS) {
                vPortFree(lcp_TempBuff);
                return lcv_Ret;
            }
        }
    }

    lcp_TempBuff[lsv_DestStrLen] = 0x00;

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lcp_TempBuff[0], 0, lsv_DestStrLen/2+1);

    /*释放内存*/
    vPortFree(lcp_TempBuff);

    return lcv_Ret;
}

/**
  * @brief  字符串检索
  * @param  None
  * @retval None
  */
unsigned char run_ci_strinstr_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    short lsv_StartIndex;
    unsigned short lsv_SrcStrLen, lsv_SearchStrLen = 0;
    unsigned short lsv_Temp;
    unsigned char lcv_SearchUcodeLen;
    unsigned char *lcp_SearchStrPtr, *lcp_SrcStrPtr;
    unsigned short i;
    unsigned char *lcp_TempSearchPtr, *lcp_TempSrcPtr, *lcp_Temp2SrcPtr;
    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    /*获取检索字符串长度*/
    if(GET_PU8_DATA(ltp_RunEnv->mcp_PC+3) == ADDR_STR) {
        lsv_SearchStrLen = GET_PU8_DATA(ltp_RunEnv->mcp_PC+2);
        lcv_SearchUcodeLen = lsv_SearchStrLen;
    } else {

        do {
            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Temp, lsv_SearchStrLen/2, 1);
            if(lcv_Ret != pdPASS) {
                return lcv_Ret;
            }

            if((lsv_Temp & 0x00FF) == 0x00) {
                break;
            }

            lsv_SearchStrLen ++;

            if(((lsv_Temp>>8) & 0x00FF) == 0x00) {
                break;
            }

            lsv_SearchStrLen ++;
        } while(1);

        lcv_SearchUcodeLen = 2;
    }

    if(lsv_SearchStrLen > 32) {
        return ERR_OPERANDS;
    }

    /*获取字符串比较起始下标*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+12, (unsigned short *)&lsv_StartIndex, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_StartIndex <= 0) {
        return pdPASS;
    }

    /*获取被检索字符串长度*/
    do {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+lcv_SearchUcodeLen + 4, &lsv_Temp, lsv_SrcStrLen/2, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }

        if((lsv_Temp & 0x00FF) == 0x00) {
            break;
        }

        lsv_SrcStrLen ++;

        if(((lsv_Temp>>8) & 0x00FF) == 0x00) {
            break;
        }

        lsv_SrcStrLen ++;
    } while(1);

    if(lsv_StartIndex > lsv_SrcStrLen) {
        return ERR_OPERANDS;
    }

    if(lsv_SearchStrLen == 0) {
        lsv_SrcStrLen ++;
        lcv_Ret = save_word(ltp_RunEnv->mcp_PC+lcv_SearchUcodeLen+8, &lsv_SrcStrLen, 0, 1);
        if(lcv_Ret != pdPASS) {
            return lcv_Ret;
        }
    }

    /*分配内存,读取源,检索字符串*/
    lcp_SearchStrPtr = (unsigned char *)pvPortMalloc(lsv_SearchStrLen+2);
    configASSERT(lcp_SearchStrPtr != NULL);

    lcp_SrcStrPtr = (unsigned char *)pvPortMalloc(lsv_SrcStrLen+2);
    configASSERT(lcp_SrcStrPtr != NULL);

    if(GET_PU8_DATA(ltp_RunEnv->mcp_PC+3) == ADDR_STR) {
        for(i=0; i<lsv_SearchStrLen; i++) {
            lcp_SearchStrPtr[i] = GET_PU8_DATA(ltp_RunEnv->mcp_PC+4+i);
        }
    } else {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lcp_SearchStrPtr[0], 0, (lsv_SearchStrLen+1)/2);
        if(lcv_Ret != pdPASS) {
            vPortFree(lcp_SearchStrPtr);
            vPortFree(lcp_SrcStrPtr);
            return lcv_Ret;
        }
    }

    lcp_SearchStrPtr[lsv_SearchStrLen] = 0x00;

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+lcv_SearchUcodeLen+4, (unsigned short *)&lcp_SrcStrPtr[0], 0, (lsv_SrcStrLen+1)/2);
    if(lcv_Ret != pdPASS) {
        vPortFree(lcp_SearchStrPtr);
        vPortFree(lcp_SrcStrPtr);
        return lcv_Ret;
    }

    lcp_SrcStrPtr[lsv_SrcStrLen] = 0x00;

    /*检索*/
    lcp_TempSearchPtr = lcp_SearchStrPtr;
    lcp_TempSrcPtr = lcp_SrcStrPtr + lsv_StartIndex -1;
    lcp_Temp2SrcPtr = lcp_TempSrcPtr;

    while(*lcp_TempSearchPtr!=0x00 && *lcp_TempSrcPtr!= 0x00) {
        if(*lcp_TempSearchPtr ++ != *lcp_TempSrcPtr++) {
            lcp_TempSearchPtr = lcp_SearchStrPtr;
            lcp_TempSrcPtr = ++lcp_Temp2SrcPtr;
        }
    }

    /*完成匹配*/
    if(*lcp_TempSearchPtr == 0x00) {
        lsv_Temp = lcp_Temp2SrcPtr - lcp_SrcStrPtr;
    } else {
        lsv_Temp = 0;
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+lcv_SearchUcodeLen+8, &lsv_Temp, 0, 1);

    vPortFree(lcp_SearchStrPtr);
    vPortFree(lcp_SrcStrPtr);

    return lcv_Ret;
}


/**
  * @brief  字符串传送
  * @param  None
  * @retval None
  */
unsigned char run_ci_strmov_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    short lsv_SrcStrLen = 0;
    unsigned char lcv_SrcUcodeLen;
    unsigned char *lcp_SrcStrPtr;
    unsigned short lsv_Temp;
    unsigned char i;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    if(GET_PU8_DATA(ltp_RunEnv->mcp_PC+3) == ADDR_STR) {
        lsv_SrcStrLen = GET_PU8_DATA(ltp_RunEnv->mcp_PC+2);
        lcv_SrcUcodeLen = lsv_SrcStrLen;
    } else {

        do {
            lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, &lsv_Temp, lsv_SrcStrLen/2, 1);
            if(lcv_Ret != pdPASS) {
                return lcv_Ret;
            }

            if((lsv_Temp & 0x00FF) == 0x00) {
                break;
            }

            lsv_SrcStrLen ++;

            if(((lsv_Temp>>8) & 0x00FF) == 0x00) {
                break;
            }

            lsv_SrcStrLen ++;
        } while(1);

        /*包括后面0x00长度*/
        lsv_SrcStrLen ++;

        lcv_SrcUcodeLen = 2;
    }

    if(lsv_SrcStrLen > 32) {
        return ERR_OPERANDS;
    }

    lcp_SrcStrPtr = (unsigned char *)pvPortMalloc(lsv_SrcStrLen+2);
    configASSERT(lcp_SrcStrPtr != NULL);

    if(GET_PU8_DATA(ltp_RunEnv->mcp_PC+3) == ADDR_STR) {
        for(i=0; i<lsv_SrcStrLen; i++) {
            lcp_SrcStrPtr[i] = GET_PU8_DATA(ltp_RunEnv->mcp_PC+4+i);
        }
    } else {
        lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short *)&lcp_SrcStrPtr[0], 0, lsv_SrcStrLen/2);
        if(lcv_Ret != pdPASS) {
            vPortFree(lcp_SrcStrPtr);
            return lcv_Ret;
        }

        lcp_SrcStrPtr[lsv_SrcStrLen-1] = 0x00;
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+4+lcv_SrcUcodeLen, (unsigned short *)&lcp_SrcStrPtr[0], 0, (lsv_SrcStrLen+1)/2);

    vPortFree(lcp_SrcStrPtr);
    return lcv_Ret;
}

