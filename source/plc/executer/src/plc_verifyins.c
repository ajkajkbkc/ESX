/**
  ******************************************************************************
  * @file    plc_verifyins.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   数据校验指令
  ******************************************************************************
  */

#include "plc_variable.h"
#include "plc_commonfunc.h"
#include "plc_element.h"
#include "plc_parseaddr.h"
#include "plc_timeins.h"
#include "plc_errormsg.h"
#include "plc_instruction.h"
#include "verify_func.h"

/**
  * @brief  CCITT校验指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_ccitt_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short *lsp_DataBuff;
    short lsv_DataNum;
    unsigned char lcv_Ret;
    unsigned short lsv_Ccitt;
    unsigned short i;
    unsigned char lcv_HighByte;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    /*获取待校验数据数量*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_DataNum, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_DataNum < 0) {
        return ERR_OPERANDS;
    }

    if(lsv_DataNum == 0) {
        return pdPASS;
    }

    lsp_DataBuff = (unsigned short *)pvPortMalloc(sizeof(unsigned short)*lsv_DataNum);
    configASSERT(lsp_DataBuff != NULL);

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, lsp_DataBuff, 0, lsv_DataNum);
    if(lcv_Ret != pdPASS) {
        vPortFree(lsp_DataBuff);
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, &lsv_Ccitt, 0, 1);
    if(lcv_Ret != pdPASS) {
        vPortFree(lsp_DataBuff);
        return lcv_Ret;
    }

    for(i=0; i<lsv_DataNum; i++) {
        lcv_HighByte = (unsigned char)(lsv_Ccitt >> 8);
        lsv_Ccitt <<= 8;
        lsv_Ccitt ^= gsv_CcittCrcTbl[lcv_HighByte ^ (unsigned char)lsp_DataBuff[i]];
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+10, &lsv_Ccitt, 0, 1);

    vPortFree(lsp_DataBuff);

    return lcv_Ret;
}

/**
  * @brief  CRC16校验指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_crc16_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short *lsp_DataBuff;
    short lsv_DataNum;
    unsigned char lcv_Ret;
    unsigned short lsv_Crc16;
    unsigned short lsv_CrcLow, lsv_CrcHigh;
    unsigned short i;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    /*获取待校验数据数量*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_DataNum, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_DataNum < 0) {
        return ERR_OPERANDS;
    }

    if(lsv_DataNum == 0) {
        return pdPASS;
    }

    lsp_DataBuff = (unsigned short *)pvPortMalloc(sizeof(unsigned short)*lsv_DataNum);
    configASSERT(lsp_DataBuff != NULL);

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, lsp_DataBuff, 0, lsv_DataNum);
    if(lcv_Ret != pdPASS) {
        vPortFree(lsp_DataBuff);
        return lcv_Ret;
    }

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, &lsv_Crc16, 0, 1);
    if(lcv_Ret != pdPASS) {
        vPortFree(lsp_DataBuff);
        return lcv_Ret;
    }

    for(i=0; i<lsv_DataNum; i++) {
        lsv_CrcLow = lsv_Crc16 & 0xFF;
        lsv_CrcHigh = lsv_Crc16 >> 8;

        lsv_Crc16 = (gcv_CRCLo[lsv_CrcLow ^ ((unsigned char)lsp_DataBuff[i])]<<8) + (unsigned char)( lsv_CrcHigh ^ (gcv_CRCHi[lsv_CrcLow ^(unsigned char)lsp_DataBuff[i]]));
    }

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+10, &lsv_Crc16, 0, 1);

    vPortFree(lsp_DataBuff);

    return lcv_Ret;
}

/**
  * @brief  LRC校验指令
  * @param  None
  * @retval None
  */
unsigned char run_ci_lrc_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned short *lsp_DataBuff;
    short lsv_DataNum;
    unsigned char lcv_Ret;
    unsigned char lcv_Lrc;
    unsigned short lsv_Temp;
    unsigned short i;

    if(!GET_POWER_FLOW(ltp_RunEnv)) {
        return pdPASS;
    }

    /*获取待校验数据数量*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short *)&lsv_DataNum, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    if(lsv_DataNum < 0) {
        return ERR_OPERANDS;
    }

    if(lsv_DataNum == 0) {
        return pdPASS;
    }

    /*取校验前内容,带入后续运算*/
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short *)&lsv_Temp, 0, 1);
    if(lcv_Ret != pdPASS) {
        return lcv_Ret;
    }

    lcv_Lrc = (unsigned char)lsv_Temp;


    lsp_DataBuff = (unsigned short *)pvPortMalloc(sizeof(unsigned short)*lsv_DataNum);
    configASSERT(lsp_DataBuff != NULL);

    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, lsp_DataBuff, 0, lsv_DataNum);
    if(lcv_Ret != pdPASS) {
        vPortFree(lsp_DataBuff);
        return lcv_Ret;
    }

    for(i=0; i<lsv_DataNum; i++) {
        lcv_Lrc += (unsigned char)lsp_DataBuff[i];
    }

    lcv_Lrc = (unsigned char)(- (char)lcv_Lrc);

    lsv_Temp = (unsigned short)lcv_Lrc;

    lcv_Ret = save_word(ltp_RunEnv->mcp_PC+10, &lsv_Temp, 0, 1);

    vPortFree(lsp_DataBuff);

    return lcv_Ret;
}

