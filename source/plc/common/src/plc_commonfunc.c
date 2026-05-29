/**
  ******************************************************************************
  * @file    plc_commonfunc.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   通用函数定义
  ******************************************************************************
  */
#include "fsl_debug_console.h"
#include "plc_commonfunc.h"
#include "plc_element.h"
#include "plc_errormsg.h"
#include "plc_variable.h"

/**
  * @brief  获取文件长度信息
  * @param  pData   文件指针
  *         flag    文件长度字节数
  * @retval None
  */
unsigned long plc_get_file_length(unsigned char *pData, unsigned short flag)
{
    unsigned long mlv_FileLength;

    switch(flag) {
        case 1:
            mlv_FileLength = (unsigned long)GET_PU8_DATA(pData);
            break;
        case 2:
            /*地址未对齐*/
            if(GET_POINT_ADDR(pData)&0x01) {
                mlv_FileLength = (unsigned long)(GET_PU8_DATA(pData) + (GET_PU8_DATA(pData+1) << 8));
            } else {
                mlv_FileLength = (unsigned long)(GET_PU16_DATA(pData));
            }
            break;
        case 4:
            if(GET_POINT_ADDR(pData)&0x03) {
                mlv_FileLength = (unsigned long)(GET_PU8_DATA(pData) + (GET_PU8_DATA(pData+1) << 8) + (GET_PU8_DATA(pData+2) << 16) + (GET_PU8_DATA(pData+3) << 24));
            } else {
                mlv_FileLength = (GET_PU32_DATA(pData));
            }
            break;
        default:
            mlv_FileLength = 0;
    }

    return mlv_FileLength;
}

/**
  * @brief  错误信息处理,刷新SD元件
  * @param  None
  * @retval None
  */
void plc_refresh_error_msg(unsigned char lcv_ErrorCode)
{
    unsigned char i;

    LOGE("plc_common", "lcv_ErrorCode = %u", lcv_ErrorCode);
    plc_set_bit_element_value(SM_ELEMENT, 20, 1);

    switch(lcv_ErrorCode) {
        case ERR_COMPILER:
        case ERR_ILLEGAL_INSTRCTION:
        case ERR_ELEMENT_TYPE:
        case ERR_EXEC_OVER_TIME:
            guv_StopError.bit.exec_err = 1;
            gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.error_status_stop = 1;
            break;

        case ERR_OPERANDS:
        case ERR_MDTCP_EXCEEDMAXCONN:
            plc_set_bit_element_value(SM_ELEMENT, 22, 1);
            guv_NonStopError.bit.operands_err = 1;
            break;

        case ERR_OVER_ELEMENT_RANG:
            plc_set_bit_element_value(SM_ELEMENT, 21, 1);
            guv_NonStopError.bit.operands_err = 1;
            break;

        case ERR_MODBUS_TBL:
        case ERR_MODBUSTCP_TBL:
        case ERR_NET_CONFIG:
            guv_NonStopError.bit.netconfig_err = 1;
            guv_StopError.bit.exec_err = 1;	
            break;
        case ERR_USER_PROGRAM:
            guv_NonStopError.bit.ucode_err = 1;
            guv_StopError.bit.exec_err = 1;
            //gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.error_status_stop = 1;
            break;
        case ERR_DATA_BLOCK:
            guv_NonStopError.bit.datablock_err = 1;
            guv_StopError.bit.exec_err = 1;
            //gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.error_status_stop = 1;
            break;
        case ERR_SYSTEM_BLOCK:
            guv_NonStopError.bit.sysblock_err = 1;
            guv_StopError.bit.exec_err = 1;
            //gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.error_status_stop = 1;
            break;
            
        case ERR_EXTEND_ERR:
        case ERR_SLAVE_ERR:
            guv_NonStopError.bit.extend_bus_err = 1;
            break;
        case ERR_ECAT_SLAVE_NUM_ERR:
        case ERR_ECAT_WKC_ERR:
        case ERR_EXPANSION_MODULE:
        case ERR_ECAT_NOT_OPMODE:
            guv_NonStopError.bit.ecat_err = 1;
            break;

        case ERR_NULL_UCODE:
        case ERR_NULL_DATA_BLOCK:
        case ERR_NULL_POU_INFO:
        case ERR_NULL_SYSTEM_BLOCK:
        case ERR_NULL_NET_CONFIG:
        case ERR_NULL_GVT:
            break;

        default:
            guv_NonStopError.bit.operands_err = 1;
            LOGE("undefined ERR", "lcv_ErrorCode = %u", lcv_ErrorCode);
            break;

    }

    if(GET_SD_ELEMENT_VALUE(20) != lcv_ErrorCode) {
        for(i=24; i>20; i--) {
            SET_SD_ELEMENT_VALUE(i, GET_SD_ELEMENT_VALUE(i-1));
        }
        SET_SD_ELEMENT_VALUE(20, lcv_ErrorCode);
    }
}

/**
  * @brief  错误信息处理,供KS定位具体错误
  * @param  None
  * @retval None
  */
void plc_refresh_exec_error_record(unsigned char lcv_ErrorCode, unsigned char *lcp_Ucode)
{
    unsigned char i;
    unsigned long llv_UcodeOffset;
    //PRINTF("lcv_ErrorCode = %d\r\n", lcv_ErrorCode);
    llv_UcodeOffset = GET_POINT_ADDR(lcp_Ucode) - GET_POINT_ADDR(gtv_UserFilePtrSt.UCodePtr);
    //PRINTF("llv_UcodeOffset = %d, lcp_Ucode = 0x%08X, UCodePtr = 0x%08X\r\n", llv_UcodeOffset, GET_POINT_ADDR(lcp_Ucode), GET_POINT_ADDR(gtv_UserFilePtrSt.UCodePtr));

    //PRINTF("lcp_Ucode = 0x%08X, gtv_UserFilePtrSt.UCodePtr = 0x%08X\r\n\r\n", lcp_Ucode, gtv_UserFilePtrSt.UCodePtr);
    /*查找是否存在相同错误记录*/
    for(i=0; i<gtv_PlcExecErrorRecord.mcv_ErrorCnt; i++) {
        if(gtv_PlcExecErrorRecord.msv_ErrorAddr[i] == llv_UcodeOffset)
            break;
    }

    if(i >= gtv_PlcExecErrorRecord.mcv_ErrorCnt) {

        if(gtv_PlcExecErrorRecord.mcv_ErrorCnt >= MAX_EXEC_ERROR_RECORD_CNT) {
            /*丢弃最早一次错误*/
            for(i=0; i<MAX_EXEC_ERROR_RECORD_CNT-1; i++) {
                gtv_PlcExecErrorRecord.msv_ErrorMsg[i] = gtv_PlcExecErrorRecord.msv_ErrorMsg[i+1];
                gtv_PlcExecErrorRecord.msv_ErrorAddr[i] = gtv_PlcExecErrorRecord.msv_ErrorAddr[i+1];
            }

            gtv_PlcExecErrorRecord.msv_ErrorMsg[MAX_EXEC_ERROR_RECORD_CNT-1] = lcv_ErrorCode;
            gtv_PlcExecErrorRecord.msv_ErrorAddr[MAX_EXEC_ERROR_RECORD_CNT-1] = llv_UcodeOffset;

        } else {

            gtv_PlcExecErrorRecord.msv_ErrorMsg[gtv_PlcExecErrorRecord.mcv_ErrorCnt] = lcv_ErrorCode;
            gtv_PlcExecErrorRecord.msv_ErrorAddr[gtv_PlcExecErrorRecord.mcv_ErrorCnt] = llv_UcodeOffset;

            gtv_PlcExecErrorRecord.mcv_ErrorCnt++;
        }
    }

}

// Just for COM0 use!
static uint8_t gRing1[1024];
static uint8_t gRing2[1024];
// Just for COM1 use!
static uint8_t gRing3[1024];
static uint8_t gRing4[1024];
// Just for COM2 use!
static uint8_t gRing5[1024];
static uint8_t gRing6[1024];

/**
  * @brief  获取环形缓冲区写指针
  * @param  None
  * @retval None
  */
unsigned char * ring_buffer_get_write_mem(ring_buffer_st *ltp_RingBuff)
{
    return ltp_RingBuff->mcp_Buff[ltp_RingBuff->mcv_Index];
}

/**
  * @brief  移动访问指针
  * @param  None
  * @retval None
  */
void ring_buffer_switch_write_mem(ring_buffer_st *ltp_RingBuff)
{
    if(ltp_RingBuff->mcv_Index+1 < RING_BUFFER_NODE_NUM) {
        ltp_RingBuff->mcv_Index += 1;
    } else {
        ltp_RingBuff->mcv_Index = 0;
    }
}

/**
  * @brief  环形缓冲区初始化
  * @param  None
  * @retval None
  */
#if 0
void ring_buffer_init(ring_buffer_st *ltp_RingBuff, unsigned short lsv_BuffSize)
{
    unsigned char i;

    for(i=0; i<RING_BUFFER_NODE_NUM; i++) {
        if(ltp_RingBuff->mcp_Buff[i] == NULL) {
            ltp_RingBuff->mcp_Buff[i] = (unsigned char *)pvPortMalloc(sizeof(unsigned char) * lsv_BuffSize);
            configASSERT(ltp_RingBuff->mcp_Buff[i] != NULL);
        }
    }

    ltp_RingBuff->mcv_Index = 0;
}
#else
void ring_buffer_init(ring_buffer_st *ltp_RingBuff, unsigned char uartPort)
{
    if (uartPort == 0)
    {
        ltp_RingBuff->mcp_Buff[0] = gRing1;
        ltp_RingBuff->mcp_Buff[1] = gRing2;
    }
    else if (uartPort == 1)
    {
        ltp_RingBuff->mcp_Buff[0] = gRing3;
        ltp_RingBuff->mcp_Buff[1] = gRing4;
    }
    else if (uartPort == 2)
    {
        ltp_RingBuff->mcp_Buff[0] = gRing5;
        ltp_RingBuff->mcp_Buff[1] = gRing6;
    }
    ltp_RingBuff->mcv_Index = 0;
}

#endif

