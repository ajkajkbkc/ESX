/**
  ******************************************************************************
  * @file    plsd_task.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   掉电保持任务
  ******************************************************************************
  */
#include <math.h>
#include "FreeRTOS.h"
#include "plc_variable.h"
#include "queue.h"
#include "plc_commonfunc.h"
#include "plsd_task.h"
#include "bsp_flash.h"
#include "bsp_gpio.h"
#include "fsl_debug_console.h"
#include "bsp_iwdg.h"
#include "kalyke_tool.h"
#include "kalyke_monitor_task.h"
#include "bsp_led.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* 参看flash_page_info变量 */
#define FLASH_ADDRESS_RESTORE_BASE      0x60110000 //4K, To save 'gtp_PowerLoseSaveDataInfo'
#define FLASH_ADDRESS_RESTORE_M         0x60111000 //16K, 实际用了4K
#define FLASH_ADDRESS_RESTORE_S         0x60115000 //4K
#define FLASH_ADDRESS_RESTORE_C         0x60116000 //4K
#define FLASH_ADDRESS_RESTORE_T         0x60117000 //8K
#define FLASH_ADDRESS_RESTORE_D         0x60119000 //16K

/*******************************************************************************
 * Variables
 ******************************************************************************/
TaskHandle_t gtv_PlsdTaskHandler;
volatile static bool gHadRunLowPowerLogic = false; //如果已经执行了低电逻辑则为true

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
  * @brief  系统掉电，保存数据
  * 当5个元件都擦除、保存时，该函数用时83ms左右
  */
void plsd_save_data(void)
{
    uint32_t tick = xTaskGetTickCount();
    LOGD("plsd", "Enter %s(), %u(ms).", __func__, tick);
#if 0
    mem_part_info_t ltv_MemInfo =
    {
        0x60110000,// 参看flash_page_info变量
        4 * 1024,
    };

    bsp_flash_erase_partition(&ltv_MemInfo);

    /*写数据到目标区*/
    bsp_flash_write_buffer(ltv_MemInfo.startAddr, (unsigned char *)&gtp_PowerLoseSaveDataInfo, sizeof(plc_plsd_st));
#endif
    mem_part_info_t flashAddr[PLSD_MAX] = {
        {FLASH_ADDRESS_RESTORE_M, 2048},
        {FLASH_ADDRESS_RESTORE_S, 4096},
        {FLASH_ADDRESS_RESTORE_C, 4096},
        {FLASH_ADDRESS_RESTORE_T, 8192},
        {FLASH_ADDRESS_RESTORE_D, 16384}
    };
    bool allLenZero = true;
    for (uint8_t i = PLSD_M; i < PLSD_MAX; i++)
    {
        if (gtp_PowerLoseSaveDataInfo.group1[i].msv_Length == 0 &&
            gtp_PowerLoseSaveDataInfo.group2[i].msv_Length == 0)
        {
            continue;
        }
        allLenZero = false;
        bsp_flash_erase_partition(&flashAddr[i]);
        switch (i)
        {
            case PLSD_M:
                bsp_flash_write_buffer(flashAddr[i].startAddr, (unsigned char *)gtv_PlcElement.msp_MElement, flashAddr[i].partSize);
                hexdump(gtv_PlcElement.msp_MElement, 64);
                hexdump(gtv_PlcElement.msp_MElement + 300, 64);
                break;
            case PLSD_S:
                bsp_flash_write_buffer(flashAddr[i].startAddr, (unsigned char *)gtv_PlcElement.msp_SElement, sizeof(gtv_PlcElement.msp_SElement));
                break;
            case PLSD_C:
                bsp_flash_write_buffer(flashAddr[i].startAddr, (unsigned char *)&gtv_PlcElement.mtv_CElement, sizeof(gtv_PlcElement.mtv_CElement));
                break;
            case PLSD_T:
                bsp_flash_write_buffer(flashAddr[i].startAddr, (unsigned char *)&gtv_PlcElement.mtv_TElement, sizeof(gtv_PlcElement.mtv_TElement));
                break;
            case PLSD_D:
                bsp_flash_write_buffer(flashAddr[i].startAddr, (unsigned char *)gtv_PlcElement.msp_DElement, sizeof(gtv_PlcElement.msp_DElement));
                break;
            default:
                LOGE("plsd", "%s: Something error!", __func__);
                break;
        }
    }

    LOGW("plsd", "allLenZero = %d", allLenZero);
    if (allLenZero == false)
    {
        uint32_t *pMagicNum = (uint32_t *)FLASH_ADDRESS_RESTORE_BASE;
        if (*pMagicNum != 0x44455649)
        {
            mem_part_info_t magicMem = {FLASH_ADDRESS_RESTORE_BASE, 4096};
            bsp_flash_erase_partition(&magicMem);
            uint32_t headFlash = 0x44455649;
            bsp_flash_write_buffer(magicMem.startAddr, (unsigned char *)&headFlash, 4);
        }
    }
    uint32_t interval = xTaskGetTickCount() - tick;
    LOGW("plsd", "Leave %s(), interval = %u(ms).", __func__, interval);
}

void plsd_erase_all(void)
{
    uint32_t *pMagicNum = (uint32_t *)FLASH_ADDRESS_RESTORE_BASE;
    LOGV("plsd", "Enter %s(), *pMagicNum = 0x%08X", __func__, *pMagicNum);
    if (*pMagicNum == 0xFFFFFFFF)
    {
        return;
    }
    mem_part_info_t magicMem = {FLASH_ADDRESS_RESTORE_BASE, 4096};
    bsp_flash_erase_partition(&magicMem);
}

static void set_bit_element_value(uint16_t *pDst, uint16_t *pSrc, uint16_t Element)
{
    uint8_t value = ((*pSrc) >> Element) & 0x01;
    if(value)
    {
        *pDst |= (0x01 << Element);
    }
    else
    {
        *pDst &= (~(0x01 << Element));
    }
}


/* pDst指向u16的bit数组中的数组元素的起始地址
 * 比如指向gtv_PlcElement.msp_MElement[i]，期中i为0、1、2、3等等
*/
static void copy_bit_element(uint16_t *pSrc, uint16_t *pDst, uint16_t elemLength)
{
    LOGV("plsd", "Enter %s(), elemLength = %u", __func__, elemLength);
    if (elemLength < 16)
    {
        for (uint16_t i = 0; i < elemLength; i++)
        {
            set_bit_element_value(pDst, pSrc, i);
        }
    }
    else
    {
        uint16_t u16Counts = elemLength >> 4;
        LOGV("plsd", "u16Counts = %u", u16Counts);
        for (int i = 0; i < u16Counts; i++)
        {
            *pDst = *pSrc;
            pDst++;
            pSrc++;
        }
        uint16_t leftLen = elemLength & 0x0F;// elemLength - (u16Counts * 16);
        if (leftLen != 0)
        {
            for (uint16_t i = 0; i < leftLen; i++)
            {
                set_bit_element_value(pDst, pSrc, i);
            }
        }
    }

#if 0
    if (elemLength < 16)
    {
        uint16_t targetVal = *pSrc;
        uint16_t len = 16 - elemLength;
        targetVal <<= len;
        targetVal >>= len;
        *pDst = targetVal;
    }
    else
    {
        uint16_t u16Counts = elemLength >> 4;
        LOGV("plsd", "u16Counts = %u", u16Counts);
        for (int i = 0; i < u16Counts; i++)
        {
            *pDst = *pSrc;
            pDst++;
            pSrc++;
        }
        if (elemLength % 16 != 0)
        {
            uint16_t leftLen = elemLength & 0x0F;
            uint16_t targetVal = *pSrc;
            LOGV("plsd", "targetVal1 = 0x%04X", targetVal);
            uint16_t len = 16 - leftLen;
            targetVal <<= len;
            targetVal >>= len;
            LOGD("plsd", "targetVal2 = 0x%04X", targetVal);
            *pDst = targetVal;
        }
    }
#endif
}

/** 
 * pSrc 指向flash起始地址
 * pDst 指向u16的位元件起始地址，例如gtv_PlcElement.msp_MElement
*/
static void restore_bit_element(uint16_t *pSrc, uint16_t *pDst, uint16_t elemStart, uint16_t elemLength)
{
    LOGV("plsd", "Enter %s(), elemStart = %u, elemLength = %u", __func__, elemStart, elemLength);
    pSrc += (elemStart >> 4);
    pDst += (elemStart >> 4);
    
    if (elemStart % 16 == 0)
    {
        copy_bit_element(pSrc, pDst, elemLength);
    }
    else
    {
        uint16_t begin = elemStart & 0x0F;
        uint16_t end = begin + elemLength;
        LOGD("plsd", "begin = %u, end = %u", begin, end);
        if (end > 16)
        {
            for (uint16_t i = begin; i < 16; i++)
            {
                set_bit_element_value(pDst, pSrc, i);
            }
            uint16_t len = 16 - begin;
            elemLength -= len;
            LOGD("plsd", "elemLength = %u", elemLength);
            if (elemLength == 0)
            {
                return;
            }
            pDst++;
            pSrc++;
            copy_bit_element(pSrc, pDst, elemLength);
        }
        else
        {
            for (uint16_t i = begin; i < end; i++)
            {
                set_bit_element_value(pDst, pSrc, i);
            }
        }
        
    #if 0
        uint16_t val = 0xFFFF;
        uint16_t targetVal = *pSrc;
        uint16_t begin = elemStart & 0x0F;
        val >>= begin;
        val <<= begin;
        LOGW("plsd", "val = 0x%04X", val);
        uint16_t len = 16 - begin;
        if (elemLength <= len)
        {
            uint16_t leftLen = len - elemLength;
            val <<= leftLen;
            val >>= leftLen;
            *pDst = val & targetVal;
        }
        else
        {
            *pDst = targetVal & val;
            pDst++;
            pSrc++;
            elemLength -= len;
            copy_bit_element(pSrc, pDst, elemLength);
        }
    #endif
    }
}

static void restore_u8_element(void *pS, void *pD, uint16_t elemStart, uint16_t elemLength)
{
    uint8_t *pSrc = (uint8_t *)pS;
    uint8_t *pDst = (uint8_t *)pD;
    pSrc += elemStart;
    pDst += elemStart;
    for (int i = 0; i < elemLength; i++)
    {
        *pDst = *pSrc;
        pDst++;
        pSrc++;
    }
}

static void restore_u16_element(uint16_t *pSrc, uint16_t *pDst, uint16_t elemStart, uint16_t elemLength)
{
    pSrc += elemStart;
    pDst += elemStart;
    for (int i = 0; i < elemLength; i++)
    {
        *pDst = *pSrc;
        pDst++;
        pSrc++;
    }
}

static void restore_u32_element(uint32_t *pSrc, uint32_t *pDst, uint16_t elemStart, uint16_t elemLength)
{
    pSrc += elemStart;
    pDst += elemStart;
    for (int i = 0; i < elemLength; i++)
    {
        *pDst = *pSrc;
        pDst++;
        pSrc++;
    }
}

static void restore_element(uint8_t elem, uint16_t elemStart, uint16_t elemLength)
{
    LOGI("plsd", "Enter %s(), elem = %u, elemStart = %u, elemLength = %u", __func__, elem, elemStart, elemLength);
    switch (elem)
    {
        case PLSD_M:
        case PLSD_S:
        {
            uint16_t *pSrc;
            uint16_t *pDst;
            if (elem == PLSD_M)
            {
                pSrc = (uint16_t *)FLASH_ADDRESS_RESTORE_M;
                pDst = M_ELEMENT;
            }
            else
            {
                pSrc = (uint16_t *)FLASH_ADDRESS_RESTORE_S;
                pDst = S_ELEMENT;
            }
            
            restore_bit_element(pSrc, pDst, elemStart, elemLength);
            break;
        }

        case PLSD_C:
        {
            plc_c_element_st *pCElementFlash = (plc_c_element_st *)FLASH_ADDRESS_RESTORE_C;
            uint16_t *pSrc = pCElementFlash->msp_BitElement;
            uint16_t *pDst = C_ELEMENT;
            restore_bit_element(pSrc, pDst, elemStart, elemLength);
            if (elemStart < C_ELEMENT_16_BIT_CNT)
            {
                pSrc = pCElementFlash->msp_16BitValue;
                pDst = gtv_PlcElement.mtv_CElement.msp_16BitValue;
                restore_u16_element(pSrc, pDst, elemStart, elemLength);
            }
            else
            {
                uint16_t realStart = elemStart - C_ELEMENT_16_BIT_CNT;
                restore_u32_element((uint32_t *)pCElementFlash->msp_32BitValue, (uint32_t *)gtv_PlcElement.mtv_CElement.msp_32BitValue, realStart, elemLength);
            }
            restore_u8_element(pCElementFlash->mtp_StatusInfo, gtv_PlcElement.mtv_CElement.mtp_StatusInfo, elemStart, elemLength);
            break;
        }

        case PLSD_T:
        {
            plc_t_element_st *pTElementFlash = (plc_t_element_st *)FLASH_ADDRESS_RESTORE_T;
            uint16_t *pSrc = pTElementFlash->msp_BitElement;
            uint16_t *pDst = T_ELEMENT;
            restore_bit_element(pSrc, pDst, elemStart, elemLength);
            pSrc = pTElementFlash->msp_CurrentValue;
            pDst = gtv_PlcElement.mtv_TElement.msp_CurrentValue;
            restore_u16_element(pSrc, pDst, elemStart, elemLength);
            pSrc = pTElementFlash->msp_DestValue;
            pDst = gtv_PlcElement.mtv_TElement.msp_DestValue;
            restore_u16_element(pSrc, pDst, elemStart, elemLength);
            restore_u32_element((uint32_t *)pTElementFlash->mlp_StartValue, (uint32_t *)gtv_PlcElement.mtv_TElement.mlp_StartValue, elemStart, elemLength);
            restore_u8_element(pTElementFlash->mtp_StatusInfo, gtv_PlcElement.mtv_TElement.mtp_StatusInfo, elemStart, elemLength);
            break;
        }

        case PLSD_D:
        {
            uint16_t *pDst = gtv_PlcElement.msp_DElement;
            uint16_t *pSrc = (uint16_t *)FLASH_ADDRESS_RESTORE_D;
            restore_u16_element(pSrc, pDst, elemStart, elemLength);
            break;
        }
        default:
            LOGE("plsd", "%s: Something error!!", __func__);
            break;
    }
}

/**
  * @brief  恢复掉电保持数据
  * @param  None
  * @retval None
  */
void plsd_restore_data(void)
{
#if 1
    uint32_t *pMagicNum = (uint32_t *)FLASH_ADDRESS_RESTORE_BASE;
    LOGD("plsd", "Enter %s(), magic num = 0x%08X", __func__, *pMagicNum);
    if (*pMagicNum == 0xFFFFFFFF)
    {
        return;// 尚未保存过，所以直接退出
    }
    for (uint8_t i = PLSD_M; i < PLSD_MAX; i++)
    {
        if (gtp_PowerLoseSaveDataInfo.group1[i].msv_Length != 0)
        {
            restore_element(i, gtp_PowerLoseSaveDataInfo.group1[i].msv_StartElement, gtp_PowerLoseSaveDataInfo.group1[i].msv_Length);
        }
        if (gtp_PowerLoseSaveDataInfo.group2[i].msv_Length != 0)
        {
            restore_element(i, gtp_PowerLoseSaveDataInfo.group2[i].msv_StartElement, gtp_PowerLoseSaveDataInfo.group2[i].msv_Length);
        }
    }
#endif
}

/**
  * @brief  系统电源检测
  * @param  None
  * @retval 0：供电正常
  *         1：系统失电
  */
static bool plsd_systeml_low_power(void)
{
    //LOGD("plsd", "Enter %s()", __func__);
    uint8_t pinStatus = GPIO_PinReadPadStatus(LOW_POWER_DETECT_GPIO, LOW_POWER_DETECT_PIN);
    //uint8_t pinStatus2 = GPIO_PinRead(LOW_POWER_DETECT_GPIO, LOW_POWER_DETECT_PIN);
    //LOGW("plsd", "pinStatus = %u, pinStatus2 = %u", pinStatus, pinStatus2);
    //LOGW("plsd", "low power pinStatus = %u", pinStatus);
    return (pinStatus);
}

bool kalyke_is_system_low_power(void)
{
    return gHadRunLowPowerLogic;
}

static void plsd_while(void)
{
    double a = 9876.78;
    double b = 9875.78;
    double c = 9874.78;
    double d = 9873.78;
    double sum = 0;
    double he = 0;
    while(1)
    {
        sum = sum + (a * b) + (c * d);
        sum = sum + (a * b) + (c * d);
        sum = sum + (a * b) + (c * d);
        sum = sum + (a * b) + (c * d);
        sum = sum + (a * b) + (c * d);
        sum = sum + (a * b) + (c * d);
        sum = sum + (a * b) + (c * d);
        sum = sum + (a * b) + (c * d);
        //sum = pow(sum);
        he = pow(b, 1000);
    }
}

/**
  * @brief  plsd_task
  * @param  None
  * @retval None
  */
void plsd_task(void *p_arg)
{
    if(gtv_DeviceConfigTable.mcv_IsSupportPlsd != 1)
    {
        LOGE("plsd", "We do not support low power detect, so just return");
        gpio_pin_config_t sw_config;
        sw_config.direction = kGPIO_DigitalOutput;
        sw_config.outputLogic = 1;
        sw_config.interruptMode = kGPIO_NoIntmode;
        GPIO_PinInit(LOW_POWER_DETECT_GPIO, LOW_POWER_DETECT_PIN, &sw_config);
        gtv_PlsdTaskHandler = NULL;
        vTaskDelete(NULL);
        return;
    }
    LOGD("plsd", "plsd_task RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());
    vTaskDelay(3000);
    LOGV("plsd", "plsd_task begin running... Free heap size is %d bytes", xPortGetFreeHeapSize());
#if 0
    uint32_t tick, tickInterval;
    tick = xTaskGetTickCount();
    plsd_save_data();
    tickInterval = xTaskGetTickCount() - tick;
    LOGW("plsd", "plsd_save_data takes %u(ms)", tickInterval);
#endif
#if 0
    LOGD("plsd", "Let us test watch dog reset!!");
    while (1)
    {
        __asm("NOP");
    }
#endif
    while(1)
    {
        //LOGV("plsd", "Free heap size is %d bytes", xPortGetFreeHeapSize());
        if (guv_PlcSysBlkAdSetting.bit.no_battery_mode == 1)
        {
            vTaskDelay(5000);
            continue;
        }
        if(!gHadRunLowPowerLogic && plsd_systeml_low_power())
        {
            bsp_open_all_led();
            gHadRunLowPowerLogic = true;
            LOGE("plsd", "Low power HAPPEN!!");
            /*停止系统运行*/
            /*清除上电自动运行标志*/
            gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.poweron_auto_run = 0;
            /*置位停止运行标志*/
            gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.cmd_stop = 1;
            /*等待其他程序运行完成*/
            vTaskDelay(100);

            /*保存数据*/
            plsd_save_data();
            /*系统掉电*/
            bsp_save_KalykeSecondTick(gKalykeSecondTick + 1);
            vTaskDelay(16000);
            bsp_reboot_system();
            //plsd_while();
        }
        vTaskDelay(1000);
        //bsp_toggle_led_mqtt();
    }
}

