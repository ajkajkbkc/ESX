/**
  ******************************************************************************
  * @file    plc_password.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   上下载，监控密码逻辑处理
  ******************************************************************************
  */
#include "plc_password.h"
#include "FreeRTOS.h"
#include "bsp_flash.h"
#include "task.h"
#include "verify_func.h"
#include "plc_variable.h"

plc_password_info_st *gtp_PasswordInfo = NULL;

/**
  * @brief  系统上下载，监控密码信息初始化
  * @param  None
  * @retval None
  */
void plc_password_init(void)
{
    unsigned char i;
    unsigned char j;

    if(gtp_PasswordInfo==NULL) {
        gtp_PasswordInfo = (plc_password_info_st *)pvPortMalloc(sizeof(plc_password_info_st));
        configASSERT(gtp_PasswordInfo != NULL);

        gtp_PasswordInfo->msv_Head = 0xAA55;
        gtp_PasswordInfo->mcv_UploadForbid = 0;

        for(i=0; i<MAX_PASSWORD_NUM; i++) {
            gtp_PasswordInfo->mtv_Password[i].mcv_IsEnable = 0;
            gtp_PasswordInfo->mtv_Password[i].mcv_CheckPass = 0;
            gtp_PasswordInfo->mtv_Password[i].mlv_CheckTime = GET_1MS_TICKS_COUNT();
            gtp_PasswordInfo->mtv_Password[i].mcv_Len = MAX_PASSWORD_LEN;
            for(j=0; j<MAX_PASSWORD_LEN; j++) {
                gtp_PasswordInfo->mtv_Password[i].mcv_Password[j] = 0;
            }
        }

        gtp_PasswordInfo->mlv_Crc = calc_crc32((unsigned char *) gtp_PasswordInfo, (sizeof(plc_password_info_st)-4));
    }
}

/**
  * @brief  从FLASH读取密码
  * @param  None
  * @retval None
  */
void plc_read_password_info(void)
{
    flash_part_info_t *ltp_PasswordFlash;
    plc_password_info_st *ltp_PwInfo;
    unsigned long mlv_Crc32;
    unsigned char i;
    unsigned char j;

    ltp_PasswordFlash = bsp_get_flash_info(USER_PASSWORD_START_FLASH_PAGE);

    if(ltp_PasswordFlash == (flash_part_info_t *)0) {
        /*密码存储区域无效，直接返回*/
        return;
    }

    ltp_PwInfo = (plc_password_info_st*)ltp_PasswordFlash->startAddr;

    if(ltp_PwInfo->msv_Head != 0xAA55) {
        return;
    }

    mlv_Crc32 = calc_crc32((unsigned char *)ltp_PwInfo, (sizeof(plc_password_info_st)-4));
    if(mlv_Crc32 != ltp_PwInfo->mlv_Crc) {
        return;
    }

    /*校验通过，拷贝密码信息*/
    gtp_PasswordInfo->mcv_UploadForbid = ltp_PwInfo->mcv_UploadForbid;

    for(i=0; i<MAX_PASSWORD_NUM; i++) {
        gtp_PasswordInfo->mtv_Password[i].mcv_IsEnable = ltp_PwInfo->mtv_Password[i].mcv_IsEnable;
        gtp_PasswordInfo->mtv_Password[i].mcv_Len = ltp_PwInfo->mtv_Password[i].mcv_Len;
        for(j=0; j<gtp_PasswordInfo->mtv_Password[i].mcv_Len; j++) {
            gtp_PasswordInfo->mtv_Password[i].mcv_Password[j] = ltp_PwInfo->mtv_Password[i].mcv_Password[j];
        }
    }

    gtp_PasswordInfo->mlv_Crc = mlv_Crc32;
}

/**
  * @brief  更新密码信息
  * @param  None
  * @retval None
  */
unsigned char plc_update_password_info(unsigned char lcv_PwItem,  unsigned char *lcp_NewPw, unsigned char lcv_PwLen)
{
    unsigned char i;
    flash_part_info_t *ltp_PasswordFlash;
    mem_part_info_t ltv_MemInfo;

    gtp_PasswordInfo->mtv_Password[lcv_PwItem].mcv_Len = lcv_PwLen;

    for(i=0; i<lcv_PwLen; i++) {
        gtp_PasswordInfo->mtv_Password[lcv_PwItem].mcv_Password[i] = lcp_NewPw[i];
    }

    gtp_PasswordInfo->mtv_Password[lcv_PwItem].mlv_CheckTime = GET_1MS_TICKS_COUNT();

    if(*(unsigned long *)gtp_PasswordInfo->mtv_Password[lcv_PwItem].mcv_Password == 0) {
        gtp_PasswordInfo->mtv_Password[lcv_PwItem].mcv_IsEnable = 0;
    } else {
        gtp_PasswordInfo->mtv_Password[lcv_PwItem].mcv_IsEnable = 1;
    }

    gtp_PasswordInfo->mtv_Password[lcv_PwItem].mcv_CheckPass = 0;

    /*更新CRC值*/
    gtp_PasswordInfo->mlv_Crc = calc_crc32((unsigned char *)gtp_PasswordInfo, (sizeof(plc_password_info_st)-4));

    /*保存新密码到Flash*/
    ltp_PasswordFlash = bsp_get_flash_info(USER_PASSWORD_START_FLASH_PAGE);

    if(ltp_PasswordFlash == (flash_part_info_t *)0) {
        /*密码存储区域无效，直接返回错误*/
        return pdFAIL;
    }

    /*擦除原数据*/
    ltv_MemInfo.startAddr = ltp_PasswordFlash->startAddr;
    ltv_MemInfo.partSize = (ltp_PasswordFlash->endAddr - ltp_PasswordFlash->startAddr + 1);
    bsp_flash_erase_partition(&ltv_MemInfo);

    /*写新数据*/
    bsp_flash_write_buffer(ltp_PasswordFlash->startAddr, (unsigned char *)gtp_PasswordInfo, sizeof(plc_password_info_st));

    return pdPASS;
}

void plc_password_erase(void)
{
    flash_part_info_t *ltp_PasswordFlash = bsp_get_flash_info(USER_PASSWORD_START_FLASH_PAGE);
    mem_part_info_t ltv_MemInfo;
    ltv_MemInfo.startAddr = ltp_PasswordFlash->startAddr;
    ltv_MemInfo.partSize = (ltp_PasswordFlash->endAddr - ltp_PasswordFlash->startAddr + 1);
    bsp_flash_erase_partition(&ltv_MemInfo);

    if (gtp_PasswordInfo)
    {
        vPortFree(gtp_PasswordInfo);
        gtp_PasswordInfo = NULL;
    }
    plc_password_init();
}

/**
  * @brief  密码校验
  * @param  lcv_PWItem: 指示要校验密码项，从__PLC_PASSWORD_TYPE 取值
  *         ltp_Passwd: 密码
  *         lcv_PwLen: 密码长度
  * @retval None
  */
unsigned char plc_check_password_info(unsigned char lcv_PwItem, unsigned char *ltp_Passwd, unsigned char lcv_PwLen)
{
    unsigned char i;

    /*密码未使能，校验成功*/
    if(!(gtp_PasswordInfo->mtv_Password[lcv_PwItem].mcv_IsEnable)) {
        return pdPASS;
    }

    /*判断长度*/
    if(lcv_PwLen != gtp_PasswordInfo->mtv_Password[lcv_PwItem].mcv_Len) {
        return pdFAIL;
    }

    /*比较密码*/
    for(i=0; i<lcv_PwLen; i++) {
        if(ltp_Passwd[i] != gtp_PasswordInfo->mtv_Password[lcv_PwItem].mcv_Password[i]) {
            return pdFAIL;
        }
    }

    gtp_PasswordInfo->mtv_Password[lcv_PwItem].mcv_CheckPass = 1;

    return pdPASS;
}

/**
  * @brief  获取密码校验结果
  * @param  lcv_PWItem: 指示要校验密码项，从__PLC_PASSWORD_TYPE 取值
  * @retval None
  */
unsigned char plc_get_password_check_result(unsigned char lcv_PwItem)
{
    /*密码未使能，校验成功*/
    if(!(gtp_PasswordInfo->mtv_Password[lcv_PwItem].mcv_IsEnable)) {
        return pdPASS;
    }

    if(GET_1MS_TICKS_COUNT() - gtp_PasswordInfo->mtv_Password[lcv_PwItem].mlv_CheckTime > MAX_PASSWORD_VALIDITY) {
        gtp_PasswordInfo->mtv_Password[lcv_PwItem].mcv_CheckPass = 0;
        return pdFAIL;
    }

    /*连续的校验密码，更新密码校验时间*/
    gtp_PasswordInfo->mtv_Password[lcv_PwItem].mlv_CheckTime = GET_1MS_TICKS_COUNT();

    if(gtp_PasswordInfo->mtv_Password[lcv_PwItem].mcv_CheckPass)
        return pdPASS;
    else
        return pdFAIL;
}

/**
  * @brief  设置禁止上载标志
  * @param  None
  * @retval None
  */
unsigned char  plc_set_upload_forbid_flag(unsigned char lcv_Flag)
{
    flash_part_info_t *ltp_PasswordFlash;
    mem_part_info_t ltv_MemInfo;

    gtp_PasswordInfo->mcv_UploadForbid = lcv_Flag;

    /*更新CRC值*/
    gtp_PasswordInfo->mlv_Crc = calc_crc32((unsigned char *)gtp_PasswordInfo, (sizeof(plc_password_info_st)-4));

    /*保存新密码到Flash*/
    ltp_PasswordFlash = bsp_get_flash_info(USER_PASSWORD_START_FLASH_PAGE);

    if(ltp_PasswordFlash == (flash_part_info_t *)0) {
        /*密码存储区域无效，直接返回错误*/
        return pdFAIL;
    }

    /*擦除原数据*/
    ltv_MemInfo.startAddr = ltp_PasswordFlash->startAddr;
    ltv_MemInfo.partSize = (ltp_PasswordFlash->endAddr - ltp_PasswordFlash->startAddr + 1);
    bsp_flash_erase_partition(&ltv_MemInfo);

    /*写新数据*/
    bsp_flash_write_buffer(ltp_PasswordFlash->startAddr, (unsigned char *)gtp_PasswordInfo, sizeof(plc_password_info_st));

    return pdPASS;
}

/**
  * @brief  获取禁止上载标志
  * @param  None
  * @retval None
  */
unsigned char plc_get_upload_forbid_flag()
{
    return gtp_PasswordInfo->mcv_UploadForbid;
}

