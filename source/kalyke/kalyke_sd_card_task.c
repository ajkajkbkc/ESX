/**
  ******************************************************************************
  * @file    kalyke_sd_card_task.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-05-02
  * @brief   SD card
  ******************************************************************************
  */
#include "kalyke_sd_card_task.h"
TaskHandle_t gKalykeSDCardTaskHandle = NULL;

#if (KALYKE_FEATURE_SD_CARD_TASK == 1)
#include "board.h"
#include "semphr.h"
#include "sdmmc_config.h"
#include "fsl_debug_console.h"
#include "fsl_sd.h"
#include "fsl_common.h"
#include "kalyke_tool.h"
#include "kalyke_version.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void SDCARD_DetectCallBack(bool isInserted, void *userData);

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief Data block count accessed in card */
#define DATA_BLOCK_COUNT (1U)
/*! @brief Start data block number accessed in card */
#define DATA_BLOCK_START (20153U)
/*! @brief Data buffer size. */
//#define DATA_BUFFER_SIZE (FSL_SDMMC_DEFAULT_BLOCK_SIZE * DATA_BLOCK_COUNT)
#define DATA_BUFFER_SIZE (1024)

/*******************************************************************************
 * Variables
 ******************************************************************************/
static const char *TAG = "sd_card";

/*! @brief Card descriptor. */
static sd_card_t g_sd;

/* @brief decription about the read/write buffer
 * The size of the read/write buffer should be a multiple of 512, since SDHC/SDXC card uses 512-byte fixed
 * block length and this driver example is enabled with a SDHC/SDXC card.If you are using a SDSC card, you
 * can define the block length by yourself if the card supports partial access.
 * The address of the read/write buffer should align to the specific DMA data buffer address align value if
 * DMA transfer is used, otherwise the buffer address is not important.
 * At the same time buffer address/size should be aligned to the cache line size if cache is supported.
 */
/*! @brief Data written to the card */
//SDK_ALIGN(uint8_t g_dataWrite[DATA_BUFFER_SIZE], BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE);
static uint8_t g_dataWrite[DATA_BUFFER_SIZE];

/*! @brief Data read from the card */
//SDK_ALIGN(uint8_t g_dataRead[DATA_BUFFER_SIZE], BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE);

/*! @brief SD card detect flag  */
static volatile bool s_cardInserted     = false;
static volatile bool s_cardInsertStatus = false;

/*! @brief Card semaphore  */
//static SemaphoreHandle_t s_CardAccessSemaphore = NULL;
static SemaphoreHandle_t s_CardDetectSemaphore = NULL;

static volatile bool gHadEverInitSDCard = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

static void CID_InformationLog(sd_card_t *card)
{
    LOGV(TAG, "cid.manufacturerID : %d\r\n", card->cid.manufacturerID);
    LOGV(TAG, "cid.applicationID : 0x%X\r\n", card->cid.applicationID);
    LOGV(TAG, "cid.productVersion : 0x%X\r\n", card->cid.productVersion);
    LOGV(TAG, "cid.productSerialNumber : 0x%X\r\n", card->cid.productSerialNumber);
    LOGV(TAG, "cid.manufacturerData : %d\r\n", card->cid.manufacturerData);
    LOGV(TAG, "cid.productName : %02X %02X %02X %02X %02X\r\n",
           card->cid.productName[0], card->cid.productName[1], card->cid.productName[2],
           card->cid.productName[3], card->cid.productName[4]);
}

static void CSD_InformationLog(sd_card_t *card)
{
    LOGD(TAG, "csd.csdStructure : %u", card->csd.csdStructure);
    LOGD(TAG, "csd.dataReadAccessTime1 : %u", card->csd.dataReadAccessTime1);
    LOGD(TAG, "csd.dataReadAccessTime2 : %u", card->csd.dataReadAccessTime2);
    LOGD(TAG, "csd.transferSpeed : %u", card->csd.transferSpeed);
    LOGD(TAG, "csd.cardCommandClass : %u", card->csd.cardCommandClass);
    LOGD(TAG, "csd.readBlockLength : %u", card->csd.readBlockLength);
    LOGD(TAG, "csd.flags : %u", card->csd.flags);
    LOGD(TAG, "csd.deviceSize : %u", card->csd.deviceSize);
    LOGD(TAG, "csd.readCurrentVddMin : %u", card->csd.readCurrentVddMin);
    LOGD(TAG, "csd.readCurrentVddMax : %u", card->csd.readCurrentVddMax);
    LOGD(TAG, "csd.writeCurrentVddMin : %u", card->csd.writeCurrentVddMin);
    LOGD(TAG, "csd.writeCurrentVddMax : %u", card->csd.writeCurrentVddMax);

    LOGD(TAG, "csd.deviceSizeMultiplier : %u", card->csd.deviceSizeMultiplier);
    LOGD(TAG, "csd.eraseSectorSize : %u", card->csd.eraseSectorSize);
    LOGD(TAG, "csd.writeProtectGroupSize : %u", card->csd.writeProtectGroupSize);
    LOGD(TAG, "csd.writeSpeedFactor : %u", card->csd.writeSpeedFactor);
    LOGD(TAG, "csd.writeBlockLength : %u", card->csd.writeBlockLength);
    LOGD(TAG, "csd.fileFormat : %u", card->csd.fileFormat);
}

static void SCR_InformationLog(sd_card_t *card)
{
    LOGI(TAG, "scr.scrStructure : %u", card->scr.scrStructure);
    LOGI(TAG, "scr.sdSpecification : %u", card->scr.sdSpecification);
    LOGI(TAG, "scr.flags : %u", card->scr.flags);
    LOGI(TAG, "scr.sdSecurity : %u", card->scr.sdSecurity);
    LOGI(TAG, "scr.sdBusWidths : %u", card->scr.sdBusWidths);
    LOGI(TAG, "scr.commandSupport : %u", card->scr.commandSupport);
    LOGI(TAG, "scr.reservedForManufacturer : %u", card->scr.reservedForManufacturer);
}

static void Status_InformationLog(sd_card_t *card)
{
    LOGI(TAG, "stat.busWidth : %u", card->stat.busWidth);
    LOGI(TAG, "stat.secureMode : %u", card->stat.secureMode);
    LOGI(TAG, "stat.cardType : %u", card->stat.cardType);
    LOGI(TAG, "stat.protectedSize : %u", card->stat.protectedSize);
    LOGI(TAG, "stat.speedClass : %u", card->stat.speedClass);
    LOGI(TAG, "stat.performanceMove : %u", card->stat.performanceMove);
    LOGI(TAG, "stat.auSize : %u", card->stat.auSize);
    LOGI(TAG, "stat.eraseSize : %u", card->stat.eraseSize);
    LOGI(TAG, "stat.eraseTimeout : %u", card->stat.eraseTimeout);
    LOGI(TAG, "stat.eraseOffset : %u", card->stat.eraseOffset);
    LOGI(TAG, "stat.uhsSpeedGrade : %u", card->stat.uhsSpeedGrade);
    LOGI(TAG, "stat.uhsAuSize : %u", card->stat.uhsAuSize);
}

void CardInformationLog(sd_card_t *card)
{
    //assert(card);

    LOGE(TAG, "Card size %d * %d bytes\r\n", card->blockCount, card->blockSize);
    LOGD(TAG, "Working condition:\r\n");
    if (card->operationVoltage == kSDMMC_OperationVoltage330V)
    {
        LOGD(TAG, "Voltage : 3.3V\r\n");
    }
    else if (card->operationVoltage == kSDMMC_OperationVoltage180V)
    {
        LOGD(TAG, "Voltage : 1.8V\r\n");
    }

    if (card->currentTiming == kSD_TimingSDR12DefaultMode)
    {
        if (card->operationVoltage == kSDMMC_OperationVoltage330V)
        {
            LOGD(TAG, "Timing mode: Default mode\r\n");
        }
        else if (card->operationVoltage == kSDMMC_OperationVoltage180V)
        {
            LOGD(TAG, "Timing mode: SDR12 mode\r\n");
        }
    }
    else if (card->currentTiming == kSD_TimingSDR25HighSpeedMode)
    {
        if (card->operationVoltage == kSDMMC_OperationVoltage180V)
        {
            LOGD(TAG, "Timing mode: SDR25\r\n");
        }
        else
        {
            LOGD(TAG, "Timing mode: High Speed\r\n");
        }
    }
    else if (card->currentTiming == kSD_TimingSDR50Mode)
    {
        LOGD(TAG, "Timing mode: SDR50\r\n");
    }
    else if (card->currentTiming == kSD_TimingSDR104Mode)
    {
        LOGD(TAG, "Timing mode: SDR104\r\n");
    }
    else if (card->currentTiming == kSD_TimingDDR50Mode)
    {
        LOGD(TAG, "Timing mode: DDR50\r\n");
    }
    LOGV(TAG, "Freq : %d HZ\r\n", card->busClock_Hz);

    LOGD(TAG, "isHostReady : %d\r\n", card->isHostReady);
    LOGD(TAG, "noInteralAlign : %d\r\n", card->noInteralAlign);
    LOGD(TAG, "relativeAddress : 0x%08X\r\n", card->relativeAddress);
    LOGD(TAG, "version : 0x%08X\r\n", card->version);
    LOGD(TAG, "flags : 0x%08X\r\n", card->flags);
    LOGW(TAG, "sizeof(card->internalBuffer) = %u", sizeof(card->internalBuffer));

    LOGD(TAG, "ocr : 0x%08X\r\n", card->ocr);
    LOGD(TAG, "driverStrength : %d\r\n", card->driverStrength);
    LOGD(TAG, "maxCurrent : %d\r\n", card->maxCurrent);

    CID_InformationLog(card);
    CSD_InformationLog(card);
    SCR_InformationLog(card);
    Status_InformationLog(card);
}

static void SDCARD_DetectCallBack(bool isInserted, void *userData)
{
    LOGW(TAG, "Enter %s(), isInserted = %u", __func__, isInserted);
    s_cardInsertStatus = isInserted;
    xSemaphoreGiveFromISR(s_CardDetectSemaphore, NULL);
}

static void powerOnAndInitSDCard(uint32_t flag)
{
    LOGI(TAG, "Enter %s(), flag = %u", __func__, flag);
    /* power on the card */
    SD_SetCardPower(&g_sd, true);
    /* Init card. */
    if (SD_CardInit(&g_sd))
    {
        CardInformationLog(&g_sd);
        LOGE(TAG, "SD card init failed.\r\n");
    }
    else
    {
        CardInformationLog(&g_sd);
        gHadEverInitSDCard = true;
        LOGV(TAG, "SD card init OKOKOK.\r\n");
    }
}

static void CardDetectTask(void *pvParameters)
{
    uint32_t lastIRQTime = 0;
    uint32_t curIRQTime = 0;
    LOGV(TAG, "CardDetectTask RUN. Free heap size is %d bytes, pvParameters=%u", xPortGetFreeHeapSize(), (uint32_t)pvParameters);
    while (true)
    {        
        /* take card detect semaphore */
        xSemaphoreTake(s_CardDetectSemaphore, portMAX_DELAY);
#if 0        
        curIRQTime = xTaskGetTickCount();
        LOGD(TAG, "curIRQTime = %u, lastIRQTime = %u", curIRQTime, lastIRQTime);
        if (curIRQTime - lastIRQTime < 2001)
        {
            if (curIRQTime > 200)
            {
                LOGD(TAG, "%s(): Just return\r\n", __func__);
                continue;
            }
        }
        lastIRQTime = xTaskGetTickCount(); 
        LOGI(TAG, "%s(): s_cardInserted = %d\r\n", __func__, s_cardInserted);
#endif
        if (s_cardInserted != s_cardInsertStatus)
        {
            s_cardInserted = s_cardInsertStatus;
            /* power off card */
            SD_SetCardPower(&g_sd, false);

            if (s_cardInserted)
            {
                LOGD(TAG, "Card inserted.\r\n");
                powerOnAndInitSDCard(0);
            }
        }
        else
        {
            if (SD_IsCardPresent(&g_sd))//有时拔了TF卡，但是s_cardInsertStatus是1
            {
                powerOnAndInitSDCard(1);
            }
            LOGW(TAG, "s_cardInserted == s_cardInsertStatus, so do nothing!");
        }
        LOGD(TAG, "s_cardInserted = %u, s_cardInsertStatus = %u", s_cardInserted, s_cardInsertStatus);
    }
}

int8_t kalyke_sd_init(void)
{
    s_CardDetectSemaphore = xSemaphoreCreateBinary();
    
    BOARD_SD_Config(&g_sd, SDCARD_DetectCallBack, BOARD_SDMMC_SD_HOST_IRQ_PRIORITY, NULL);
    
    /* SD host init function */
    if (SD_HostInit(&g_sd) != kStatus_Success)
    {
        LOGE(TAG, "SD host init fail\r\n");
        return -1;
    }
    else
    {
        LOGI(TAG, "SD host init SUCCESS!\r\n");
    }
#if 0
    /* power on the card */
    SD_PowerOnCard(g_sd.host.base, g_sd.usrParam.pwr);
    /* Init card. */
    if (SD_CardInit(&g_sd))
    {
        PRINTF("\r\nSD card init failed.\r\n");
        return -2;
    }
    else
    {
        PRINTF("\r\nSD card init SUCCESS!\r\n");
    }
#else
    //xTaskCreate(CardDetectTask, "CardDetectTask", 1024, NULL, 5, NULL);
#endif
    return 0;
}

void test_sd_read(uint8_t *pBuf, uint32_t len)
{
    //SCB_DisableDCache();
    static uint32_t curBlock = DATA_BLOCK_START;
    LOGD(TAG, "Enter %s(), pBuf = 0x%08X, len = %u\r\n", __func__, pBuf, len);
    //LOGD(TAG, "sizeof(g_dataRead) = %u\r\n", len);

    /* Check if card is readonly. */
    bool isReadOnly = SD_CheckReadOnly(&g_sd);
    PRINTF("isReadOnly = %d\r\n", isReadOnly);

    memset(pBuf, 3, len);
    LOGD(TAG, "Read data block......%u\r\n", curBlock);
    if (kStatus_Success != SD_ReadBlocks(&g_sd, pBuf, curBlock, 2U))
    {
        LOGE(TAG, "Read data block failed.\r\n");
    }
    else
    {
        LOGV(TAG, "Read data block SUCCESS!\r\n");
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
    if (len == 512)
    {
        hexdump(pBuf, 512);
    }
    else
    {
        hexdump(pBuf, 600);
    }
    //SCB_EnableDCache();
}

void test_sd_write(void)
{
    //SCB_DisableDCache();
    LOGV(TAG, "Enter %s()", __func__);
    #if 1
    PRINTF("sizeof(g_dataWrite) = %u\r\n", sizeof(g_dataWrite));
    memset(g_dataWrite, 0, sizeof(g_dataWrite));
    memset(g_dataWrite, 0xE8, 512);
    g_dataWrite[512] = 0x33;
    g_dataWrite[513] = 0x43;
    #else
    uint8_t *pBuf = pvPortMalloc(1024);
    LOGW(TAG, "pBuf = 0x%08X", pBuf);
    memset(pBuf, 0, sizeof(1024));
    memset(pBuf, 0xC5, 512);
    pBuf[512] = 0x33;
    pBuf[513] = 0x43;
    #endif
    if (kStatus_Success != SD_WriteBlocks(&g_sd, g_dataWrite, DATA_BLOCK_START, 2))
    //if (kStatus_Success != SD_WriteBlocks(&g_sd, pBuf, DATA_BLOCK_START, 2))
    {
        LOGE(TAG, "Write data block failed.\r\n");
    }
    else
    {
        LOGV(TAG, "Write data block SUCCESS!!\r\n");
    }
    //SCB_EnableDCache();
}

void kalyke_sd_task(void *p_arg)
{
    PRINTF("kalyke_sd_task RUN. Free heap size is %d bytes\r\n", xPortGetFreeHeapSize());
    gHadEverInitSDCard = false;
    s_cardInserted = false;
    s_cardInsertStatus = false;
    kalyke_sd_init();
    xTaskCreate(CardDetectTask, "CardDetectTask", 
            SD_TASK_STACK_SIZE, (void *)xTaskGetTickCount(), SD_TASK_PRIO, &gKalykeSDCardTaskHandle);
#if 1
    static uint64_t flag = 0xFFffFFfffffffffE;
    for(;;)
    {
    #if 1
        if (SD_IsCardPresent(&g_sd) == false)//这个判断是绝对正确的
        {
            LOGE(TAG, "SD card is not present!\r\n");
            LOGD(TAG, "s_cardInserted = %d, flag = 0x%016llX, s_cardInsertStatus = %d\r\n", s_cardInserted, flag, s_cardInsertStatus);
            vTaskDelay(4001);
            continue;
        }
        else
        {
            PRINTF("SD card is present!\r\n");
            if (gHadEverInitSDCard == false)
            {
                powerOnAndInitSDCard(2);
            }
        }
    #endif
        flag++;
        LOGV(TAG, "s_cardInserted = %d, flag = 0x%016llX, s_cardInsertStatus = %d", s_cardInserted, flag, s_cardInsertStatus);
    #if 1
        if (flag == 3)
        {
            test_sd_write();
        }
        else if (flag == 4)
        {
            uint8_t *readBuf = pvPortMalloc(1024);
            
            test_sd_read(readBuf, 512);
            if (memcmp(readBuf, g_dataWrite, 512))
            {
                LOGE(TAG, "The read/write content isn't consistent.\r\n");
            }
            else
            {
                LOGV(TAG, "The read/write content is consistent.\r\n");
            }
            vPortFree(readBuf);
        }
        else if (flag == 6)
        {
            uint8_t *readBuf = pvPortMalloc(1024);
            //test_sd_read(g_dataRead2, 1024);
            test_sd_read(readBuf, 600);
            if (memcmp(readBuf, g_dataWrite, DATA_BUFFER_SIZE))
            {
                LOGE(TAG, "The read/write content isn't consistent.\r\n");
            }
            else
            {
                LOGV(TAG, "The read/write content is consistent.\r\n");
            }
            vPortFree(readBuf);
        }
    #endif
        vTaskDelay(4001);
    }
#endif
    vTaskDelete(NULL);
}
#endif

