/**
  ******************************************************************************
  * @file    kalyke_ota.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-04-25
  * @brief   系统中两个bin的管理
  ******************************************************************************
  */
#include "kalyke_ota.h"
#include "bl_api.h"
#include "fsl_ocotp.h"
#include "fsl_debug_console.h"
#include "kalyke_tool.h"
#include "bsp_flash.h"
#include "kalyke_version.h"
#include "plc_element.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
//typedef void (*pRunBootloaderFn)(void *arg);
//static void (*pFunc)(void);  /*定义一个函数指针*/
#define OCOTP_FREQ_HZ (CLOCK_GetFreq(kCLOCK_IpgClk))


/*******************************************************************************
 * Variables
 ******************************************************************************/
/**
 * 如果该值为0，则固件会放入0x60000000 ~ 0x6007FFFF
 * 如果该值为1，则固件会放入0x60080000 ~ 0x600FFFFF
 */
static uint32_t gIdx = 0; // Image index. 0 or 1

static uint32_t gImageID = 0;
static bootloader_api_entry_t *g_bootloaderTree;
//static pRunBootloaderFn gpRunBootloader = NULL;

/*******************************************************************************
 * Code
 ******************************************************************************/

void ota_reset(uint32_t idx)
{
    uint32_t misc = OCOTP->MISC_CONF1;
    PRINTF("OCOTP->MISC_CONF1 = 0x%08X\r\n", misc);
    uint32_t mask = 0x2<<16;
    OCOTP->MISC_CONF1 = mask | OCOTP->MISC_CONF1;
    PRINTF(".OCOTP->MISC_CONF1 = 0x%08X\r\n", OCOTP->MISC_CONF1);

#if 1
    g_bootloaderTree = (bootloader_api_entry_t *)*(uint32_t *)0x0020001c;
    //PRINTF("g_bootloaderTree->runBootloader = 0x%08X\r\n", g_bootloaderTree->runBootloader);//0x002019F7
    //vTaskDelay(10);
    run_bootloader_ctx_t boot_para;
    boot_para.B.imageIndex = idx; // specified firmware index : 0 or 1
    boot_para.B.serialBootInterface = kEnterBootloader_SerialInterface_Auto;
    boot_para.B.bootMode = kEnterBootloader_Mode_Default;
    boot_para.B.tag = kEnterBootloader_Tag;
    g_bootloaderTree->runBootloader( (void *)&boot_para ); // run the firmware
#else /* Not work! */
    uint32_t arg = 0xEB000000;
    if (gIdx == 1)
    {
        arg = 0xEB000001;
    }
    gpRunBootloader = (pRunBootloaderFn)(0x0020001c + 8);
    PRINTF("gpRunBootloader = 0x%08X, arg = 0x%08X\r\n", gpRunBootloader, arg);
    gpRunBootloader((void*)&arg); // run the firmware
#endif
}

#if 0
static void showMem(void)
{
    uint8_t *p = (uint8_t *)0x60080000;
    hexdump(p, 128);
}
#endif

static void ota_init(void)
{
    gIdx = bsp_get_image_idx();
    //系统第一次运行，初始化image index为0
    if (gIdx == 0xFFFFFFFF)
    {
        bsp_save_image_idx(0);
        gIdx = 0;
    }
    else if (gIdx != 0 && gIdx != 1)
    {
        PRINTF("Oh My God, gIdx had a wrong number(%u),will reset to 0! \r\n", gIdx);
        bsp_save_image_idx(0);
        gIdx = 0;
    }

    gImageID = bsp_get_imageID();
    if (gImageID == 0xFFFFFFFF) //系统第一次运行，初始化image ID为FIRMWARE_IMAGE_ID
    {
        bsp_save_imageID(FIRMWARE_IMAGE_ID);
        gImageID = FIRMWARE_IMAGE_ID;
    }
    //Test
    PRINTF("leave ota_init(void),gIdx = %u, gImageID = %u, FIRMWARE_IMAGE_ID = %u\r\n", gIdx, gImageID, FIRMWARE_IMAGE_ID);
}

#if 0
void test_ocotp(void)
{
    PRINTF("Enter %s()\r\n", __func__);
    uint32_t misc = OCOTP->MISC_CONF1;
    PRINTF("OCOTP->MISC_CONF1 = 0x%08X\r\n", misc);
    uint32_t mask = 0x2<<16;
    OCOTP->MISC_CONF1 = mask | OCOTP->MISC_CONF1;
    PRINTF(".OCOTP->MISC_CONF1 = 0x%08X\r\n", OCOTP->MISC_CONF1);

    //PRINTF("Let us program OCOTP->MISC_CONF1 :\r\n");
    PRINTF("Let us program OCOTP->GP30 :\r\n");
    uint32_t src = 1;
    //status_t retStatus = ocotp_program_once(OCOTP, 0x2E, &src, 4); // MISC_CONF1
    status_t retStatus = ocotp_program_once(OCOTP, 0x38, &src, 4); // GP30
    PRINTF("retStatus = %u\r\n", retStatus);
    //PRINTF("OCOTP->MISC_CONF1 = 0x%08X\r\n", OCOTP->MISC_CONF1);
    PRINTF("OCOTP->GP30 = 0x%08X\r\n", OCOTP->GP30);
}
#endif

/**使能双bin
 */
status_t ota_program_once_MISC_CONF1(void)
{
    PRINTF("Enter %s()\r\n", __func__);
    OCOTP_Init(OCOTP, OCOTP_FREQ_HZ);
    uint32_t mask = 0x2u << 16;
    //status_t retStatus = ocotp_program_once(OCOTP, 0x2E, &mask, 4); // MISC_CONF1
    status_t retStatus = OCOTP_WriteFuseShadowRegister(OCOTP, 0x2E, mask);// MISC_CONF1
    if (retStatus == kStatus_Success)
    {
        LOGI("OTA", "OCOTP Write operation success!!!");
        LOGV("OTA", "The new value is 0x%08X\r\n", OCOTP_ReadFuseShadowRegister(OCOTP, 0x2E));
    }
    else
    {
        LOGE("OTA", "OCOTP Write operation ERROR!!!");
    }
    OCOTP_Deinit(OCOTP);
    return retStatus;
}

#if (OTA_LOGIC == 0)
void ota_task(void *p_arg)
{
#if SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_SDK
    void (*pFunc)(uint32_t);
    PRINTF("ota_task RUN. Free heap size is %d bytes\r\n", xPortGetFreeHeapSize());
    pFunc = ota_reset;
    LOGV("OTA", "\r\npFunc = 0x%08X\r\n", pFunc);
    uint32_t mask = 0x2u << 16;
    LOGD("OTA", "mask = 0x%08X\r\n", mask);
#endif

    ota_init();

    if (gIdx == 1)
    {
        if (FIRMWARE_IMAGE_ID < gImageID)
        {
            PRINTF("FIRMWARE_IMAGE_ID < gImageID\r\n");
            ota_reset(gIdx);
        }
        else if (FIRMWARE_IMAGE_ID > gImageID)
        {
            bsp_save_imageID(FIRMWARE_IMAGE_ID);
            gImageID = FIRMWARE_IMAGE_ID;
        }
        else
        {
            // Do nothing.
        }
    }
    else // gIdx == 0
    {
        if (FIRMWARE_IMAGE_ID > gImageID)
        {
            bsp_save_imageID(FIRMWARE_IMAGE_ID);
            gImageID = FIRMWARE_IMAGE_ID;
        }
    }
    uint32_t *pSD = (uint32_t *)gtv_PlcElement.msp_SDElement[SD207];
    *pSD = gImageID;
    vTaskDelete(NULL);
}
#else
void ota_task(void *p_arg)
{
#if SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_SDK
    void (*pFunc)(uint32_t);
    PRINTF("ota_task RUN. Free heap size is %d bytes\r\n", xPortGetFreeHeapSize());
    pFunc = ota_reset;
    LOGV("OTA", "\r\npFunc = 0x%08X\r\n", pFunc);
    uint32_t mask = 0x2u << 16;
    LOGD("OTA", "mask = 0x%08X\r\n", mask);
#endif
    vTaskDelay(10);
    ota_init();

    if (gIdx == 1)
    {
        uint32_t nIfReset = bsp_get_Image1IfReset();
        LOGE("OTA", "bsp_get_Image1IfReset() = %u\r\n", nIfReset);
        if ( 0 == nIfReset || 0xFFFFFFFF == nIfReset)
        {
            PRINTF("bsp_save_Image1IfReset(1), ota_reset(%u)\r\n", gIdx);
            bsp_save_Image1IfReset(1);
            //vTaskDelay(1);
            ota_reset(gIdx);
        }
        LOGE("OTA", "AAAAAAAAAAAA");
        bsp_save_Image1IfReset(0);
        LOGE("OTA", "BBBBBBBBBBBBB");
        if (FIRMWARE_IMAGE_ID != gImageID)
        {
            bsp_save_imageID(FIRMWARE_IMAGE_ID);
            gImageID = FIRMWARE_IMAGE_ID;
        }
    }
    else // gIdx == 0
    {
        if (FIRMWARE_IMAGE_ID != gImageID)
        {
            bsp_save_imageID(FIRMWARE_IMAGE_ID);
            gImageID = FIRMWARE_IMAGE_ID;
        }
    }
    uint32_t *pSD = (uint32_t *)&gtv_PlcElement.msp_SDElement[SD207];
    *pSD = gImageID;
    vTaskDelete(NULL);
}

#endif

uint32_t ota_get_image_idx(void)
{
    return gIdx;
}

void ota_save_image_idx(uint32_t idx)
{
    gIdx = idx;
    bsp_save_image_idx(idx);
}

uint32_t ota_get_image_id(void)
{
    return gImageID;
}
