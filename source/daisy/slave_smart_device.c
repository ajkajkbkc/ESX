/**
  ******************************************************************************
  * @file    slave_smart_device.c
  * @author  hejunbin
  * @version V0.0.1
  * @date    2022-06-07
  * @brief
  ******************************************************************************
  */

#include "daisy_task.h"
#include "kalyke_opts.h"
#include "kalyke_version.h"







/*******************************************************************************
 * Prototypes
 ******************************************************************************/




/*******************************************************************************
 * Definitions
 ******************************************************************************/




/*******************************************************************************
 * Variables
 ******************************************************************************/
static const char *TAG = "Smart_device";




/*******************************************************************************
 * Code
 ******************************************************************************/

/**
@configSmart
00 01
66 00
07 00 08 00
1E 81 60 02 10 1A
1C 84 60 02 00 00
1C 85 60 02 0E 00
1C 86 60 02 00 00
1C 87 60 02 02 00
1C 88 60 02 0E 00
1C 8A 60 02 04 00
1B 00 00 03 01 00 00  (Maping to 03000a）
1B 00 00 09 02 01 00  (Maping to 09000a）
1B 02 00 09 02 03 00  (Maping to 09000b）
1B 04 00 09 02 05 00  (Maping to 09000e）
1A 08 00 03 01 07 00  (Maping to 030012）
1A 01 00 09 02 08 00  (Maping to 09000b）
1A 03 00 09 02 0A 00  (Maping to 09000d）
1A 05 00 09 02 0C 00  (Maping to 09000f）
属性(1) 元件地址(2) 元件类型(1) 字节长度(1) 帧中偏移(2)
*/
uint16_t daisy_handle_config(uint8_t *pBuf, uint16_t *pErrContent)
{
    uint8_t *pTemp = pBuf;
    uint32_t BfmValue;
    uint16_t SdoCount;
    uint16_t PdoCount;
    uint16_t BfmIdx;
    uint16_t BfmLen;
    uint16_t ia;
    uint16_t ib;
    uint8_t sType;
    uint8_t i;
    pdo_st *pPdo;

    pTemp += 4;
    SdoCount = GET_SMLPU16_DATA(pTemp);
    pTemp += 2;
    PdoCount = GET_SMLPU16_DATA(pTemp);
    LOGD(TAG, "SdoCount = %u, PdoCount = %u", SdoCount, PdoCount);
    pTemp += 2;

    //now pTemp -> 1E 81 60 02 10 1A
    for(i = 0; i < SdoCount; i++)
    {
        pTemp += 1;
        //Get BFM index
        BfmIdx = GET_SMLPU16_DATA(pTemp);
        pTemp += 2;
        //Get BFM length
        BfmLen = GET_PU8_DATA(pTemp);
        LOGV(TAG, "BfmIdx = 0x%04X,BfmLen = %d", BfmIdx, BfmLen);
        //Get BFM value
        pTemp += 1;
        if(BfmLen == 1)
        {
            BfmValue = GET_PU8_DATA(pTemp);
        }
        else if(BfmLen == 2)
        {
            BfmValue = GET_SMLPU16_DATA(pTemp);
        }
        else if(BfmLen == 4)
        {
            BfmValue = GET_SMLPU32_DATA(pTemp);
        }
        else
        {
            LOGE(TAG, "BfmLen err %d", BfmLen);
            *pErrContent = BfmLen;
            return DAISY_BFM_LEN_ERR_IN_CONFIG_SMART;
        }
        //pTemp准备下次循环
        pTemp += BfmLen;
        //赋值
        if(BfmIdx == BFM_IDX_0x6081)
        {
            LOGI(TAG, "moduleNum = 0x%04X", BfmValue);
            if(BfmValue != SLAVE_NUMBER)
            {
                *pErrContent = BfmValue;
                return DAISY_SMART_ID_ERR_IN_CONFIG_SMART;
            }
        }
        else if(BfmIdx == BFM_IDX_0x6084)
        {
            gSmartBFM.bfm6084 = BfmValue;
        }
        else if(BfmIdx == BFM_IDX_0x6085)
        {
            gSmartBFM.bfm6085 = BfmValue;
        }
        else if(BfmIdx == BFM_IDX_0x6086)
        {
            gSmartBFM.bfm6086 = BfmValue;
        }
        else if(BfmIdx == BFM_IDX_0x6087)
        {
            gSmartBFM.bfm6087 = BfmValue;
        }
        else if(BfmIdx == BFM_IDX_0x6088)
        {
            gSmartBFM.bfm6088 = BfmValue;
        }
        else if(BfmIdx == BFM_IDX_0x608A)
        {
            gSmartBFM.bfm608A = BfmValue;
            if(gSmartBFM.bfm608A < 1)
            {
                gSmartBFM.bfm608A = 1;
            }
        }
        else
        {
            LOGE(TAG, "SDO BfmIdx err 0x%04X", BfmIdx);
            if(BfmIdx < 0x6000 &&  BfmIdx > 0x7FFF)
            {
                *pErrContent = BfmIdx;
                return DAISY_BFM_IDX_SDO_ERR_IN_CONFIG_SMART;
            }
        }
    }

    //now pTemp -> 1B 00 00 03 01 00 00  (Maping to 03000a）
    ia = 0;
    ib = 0;
    gSmartBFM.nPdoCountSmart = PdoCount;
    gSmartBFM.nPdoBytesSmart = 0;
    gSmartBFM.pdo1ACountSmart = 0;
    gSmartBFM.pdo1BCountSmart = 0;
    gSmartBFM.pPDO1A_Smart = pvPortMalloc(sizeof(pdo_st) * PdoCount);
    gSmartBFM.pPDO1B_Smart = pvPortMalloc(sizeof(pdo_st) * PdoCount);
    for(i = 0; i < PdoCount; i++)
    {
        //Get BFM 属性
        sType = GET_PU8_DATA(pTemp);
        if(sType == 0x1A)
        {
            pPdo = gSmartBFM.pPDO1A_Smart + ia++;
            gSmartBFM.pdo1ACountSmart++;
        }
        else if(sType == 0x1B)
        {
            pPdo = gSmartBFM.pPDO1B_Smart + ib++;
            gSmartBFM.pdo1BCountSmart++;
        }
        else
        {
            *pErrContent = sType;
            return DAISY_BFM_PDO_STYPE_ERR_IN_CONFIG_SMART;
        }
        pPdo->sType = sType;
        pTemp += 1;

        //Get BFM 元件地址
        pPdo->eAddr = GET_SMLPU16_DATA(pTemp);
        pTemp += 2;

        //Get BFM 元件类型
        pPdo->eType = GET_PU8_DATA(pTemp);
        pTemp += 1;

        //Get BFM 字节长度
        pPdo->len = GET_PU8_DATA(pTemp);
        pTemp += 1;

        //Get BFM 帧中偏移
        pPdo->offset = GET_SMLPU16_DATA(pTemp) + gSmartBFM.bfm6084;
        pTemp += 2;
    }
    gSmartBFM.bfm6081 = SLAVE_NUMBER;
    gSmartBFM.bfm6082 = SW_VERSION_ID;

    *pErrContent = 0;
    return DAISY_COMMON_NO_ERR;
}

