/**
  ******************************************************************************
  * @file    bsp_flash.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   内部flash操作接口
  ******************************************************************************
  */
#include "bsp_flash.h"
#include "fsl_debug_console.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_iwdg.h"
#include "evkmimxrt1060_flexspi_nor_config.h"
#include "kalyke_internet_task.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static inline uint32_t get_addr(uint32_t sectorID, uint32_t offset);


/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define FLASH_Sector_0     ((uint16_t)0x0000) /*!< Sector Number 0   */
#define FLASH_Sector_1     ((uint16_t)0x0008) /*!< Sector Number 1   */
#define FLASH_Sector_2     ((uint16_t)0x0010) /*!< Sector Number 2   */
#define FLASH_Sector_3     ((uint16_t)0x0018) /*!< Sector Number 3   */
#define FLASH_Sector_4     ((uint16_t)0x0020) /*!< Sector Number 4   */
#define FLASH_Sector_5     ((uint16_t)0x0028) /*!< Sector Number 5   */
#define FLASH_Sector_6     ((uint16_t)0x0030) /*!< Sector Number 6   */
#define FLASH_Sector_7     ((uint16_t)0x0038) /*!< Sector Number 7   */
#define FLASH_Sector_8     ((uint16_t)0x0040) /*!< Sector Number 8   */
#define FLASH_Sector_9     ((uint16_t)0x0048) /*!< Sector Number 9   */
#define FLASH_Sector_10    ((uint16_t)0x0050) /*!< Sector Number 10  */
#define FLASH_Sector_11    ((uint16_t)0x0058) /*!< Sector Number 11  */
#define FLASH_Sector_12    ((uint16_t)0x0080) /*!< Sector Number 12  */
#define FLASH_Sector_13    ((uint16_t)0x0088) /*!< Sector Number 13  */
#define FLASH_Sector_14    ((uint16_t)0x0090) /*!< Sector Number 14  */
#define FLASH_Sector_15    ((uint16_t)0x0098) /*!< Sector Number 15  */
#define FLASH_Sector_16    ((uint16_t)0x00A0) /*!< Sector Number 16  */
#define FLASH_Sector_17    ((uint16_t)0x00A8) /*!< Sector Number 17  */
#define FLASH_Sector_18    ((uint16_t)0x00B0) /*!< Sector Number 18  */
#define FLASH_Sector_19    ((uint16_t)0x00B8) /*!< Sector Number 19  */
#define FLASH_Sector_20    ((uint16_t)0x00C0) /*!< Sector Number 20  */
#define FLASH_Sector_21    ((uint16_t)0x00C8) /*!< Sector Number 21  */
#define FLASH_Sector_22    ((uint16_t)0x00D0) /*!< Sector Number 22  */
#define FLASH_Sector_23    ((uint16_t)0x00D8) /*!< Sector Number 23  */

#define KALYKE_BASE_FLASH_ADDRESS (FlexSPI_AMBA_BASE+0x100000)

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* 0x60100000 ~ 0x603FFFFF
   第1M用于程序 0x60002400 ~ 0x0x600FFFFF
   第2、3M用于下面的 0x60100000 ~ 0x602FFFFF
   第4M给一些单独的小变量使用，参看line341开始的代码 0x60300000 ~ 0x603FFFFF
 */
static flash_part_info_t flash_page_info[] =
{
    //BOOTLOADER_START_FLASH_PAGE
    {FLASH_Sector_0,    0x60100000, 0x60103FFF },  /*  0   16K */   /*BootLoad  128K*/  // No use for now
    {FLASH_Sector_1,    0x60104000, 0x60107FFF },  /*  1   16K */ // No use for now
    {FLASH_Sector_2,    0x60108000, 0x6010BFFF },  /*  2   16K */ // No use for now

    //DEV_INFO_START_FLASH_PAGE
    {FLASH_Sector_3,    0x6010C000, 0x6010CFFF },  /*  3   4K 设备信息*/

    //PLSD_SAVE_DATA_FLASH_PAGE
    {FLASH_Sector_4,    0x60110000, 0x6011FFFF },  /*  4   64K */ // No use for now

    //PLC_CODE_START_FLASH_PAGE
    {FLASH_Sector_5,    0x60120000, 0x6013FFFF },  /*  5   128K*/   /*AppCode   640K */ // No use for now
    {FLASH_Sector_6,    0x60140000, 0x6015FFFF },  /*  6   128K*/ // No use for now
    {FLASH_Sector_7,    0x60160000, 0x6017FFFF },  /*  7   128K*/ // No use for now
    {FLASH_Sector_8,    0x60180000, 0x6019FFFF },  /*  8   128K*/ // No use for now
    {FLASH_Sector_9,    0x601A0000, 0x601BFFFF },  /*  9   128K*/ // Use for SYS_BLOCK_START_PAGE

    //DEV_NETWORK_INFO_START_FLASH_PAGE
    {FLASH_Sector_10,   0x601C0000, 0x601DFFFF },  /*  10  128K*/

    //CLOUD_DATA_TABLE_START_FLASH_PAGE暂未使用，和网络信息存到了一起
    {FLASH_Sector_11,   0x601E0000, 0x601FFFFF },  /*  11  128K*/  /*icloudTable 128K */

    //USER_PASSWORD_START_FLASH_PAGE
    {FLASH_Sector_12,   0x60200000, 0x60200FFF },  /*  12  4K password oth  */

    //SYS_BLOCK_START_PAGE
    {FLASH_Sector_13,   0x601A0000, 0x601AFFFF },  /*  13  64K sys      */

    //POU_FILE_START_PAGE
    {FLASH_Sector_14,   0x60208000, 0x6020BFFF },  /*  14  16K pou      */

    //GVT_FILE_START_PAGE
    {FLASH_Sector_15,   0x6020C000, 0x6020FFFF },  /*  15  16K gvt      */

    //DATA_BLOCK_START_PAGE
    {FLASH_Sector_16,   0x60210000, 0x6021FFFF },  /*  16  64K db       */

    //UCODE_FILE_START_PAGE
    {FLASH_Sector_17,   0x60220000, 0x6023FFFF },  /*  17  128K ucode   */
    {FLASH_Sector_18,   0x60240000, 0x6025FFFF },  /*  18  128K clange  */

    //CBIN_UPGRADE_START_PAGE
    {FLASH_Sector_19,   0x60260000, 0x6027FFFF },  /*  19  128K */
    {FLASH_Sector_20,   0x60280000, 0x6029FFFF },  /*  20  128K*/
    {FLASH_Sector_21,   0x602A0000, 0x602BFFFF },  /*  21  128K*/


    //PID1_START_PAGE
    {FLASH_Sector_22,   0x602FE000, 0x602FEFFF },  /*  22  4K*/

    //PID2_START_PAGE
    {FLASH_Sector_23,   0x602FF000, 0x602FFFFF }   /*  23  4K*/
};


/*******************************************************************************
 * Code
 ******************************************************************************/
/**
  * @brief  获取FLASH内部页信息
  * @param  None
  * @retval None
  */
flash_part_info_t *bsp_get_flash_info(unsigned char page)
{
    flash_part_info_t *ltp_PageInfo;
    if(page < MAX_FLASH_PAGE_NUM)
    {
        ltp_PageInfo = &flash_page_info[page];
    }
    else
    {
        ltp_PageInfo = (flash_part_info_t *)(0);
    }

    return ltp_PageInfo;
}

/**
  * @brief  擦除FLASH页
  * @param  None
  * @retval None
  */
/*
 *The W25Q32JV array is organized into 16,384 programmable pages of 256-bytes each.
  Up to 256 bytes can be programmed at a time. Pages can be erased in groups of 16(4KB sector erase),
  groups of 128(32KB block erase), groups of 256 (64KB block erase) or the entire chip (chip erase).
  The W25Q32JV has 1,024 erasable sectors and 64 erasable blocks respectively.
  The small 4KB sectors allow for greater flexibility in applications that require data and parameter storage.

  So far, we just erase sector one by one.
  Erase sector time is 45ms typically and 400ms at maximum.
 */
unsigned char bsp_flash_erase_partition(mem_part_info_t *pPart)
{
    int32_t     status;
    uint32_t    startAddr = pPart->startAddr - KALYKE_FLEXSPI_AMBA_BASE;
    uint32_t    partSize = pPart->partSize;
    uint32_t    erasedSize = 0;
    uint8_t     dogs = 0;
    vTaskDelay(10);   
    SCB_DisableDCache();

    //PRINTF("Enter %s(), startAddr = 0x%x, partSize = 0x%x\r\n", __func__, startAddr, partSize);
    //status = mflash_drv_init();
    //PRINTF("status = %d\r\n", status);
    //taskENTER_CRITICAL();
    while (1)
    {
        vTaskDelay(1);   
        //PRINTF("mflash_drv_erase_sector() dogs = %u!\r\n",dogs);  //这里打log可能会导致 重写设备信息 时死机
        /* Erase sectors. */
        //PRINTF("Erasing Serial NOR over FlexSPI...\r\n");
        //status = flexspi_nor_flash_erase_sector(KALYKE_FLEXSPI, startAddr + erasedSize);
        status = mflash_drv_erase_sector((void*)(startAddr + erasedSize));
        //status = flexspi_nor_flash_erase_sector(KALYKE_FLEXSPI, EXAMPLE_SECTOR * 0x1111); // Error
        if (status != 0)
        {
            PRINTF("Erase sector failure !\r\n");
            //taskEXIT_CRITICAL();
            SCB_EnableDCache();
            return MEM_ADDR_ERR;
        }
        else
        {
            //PRINTF("Erase sector success: 0x%x !\r\n", startAddr + erasedSize);
        }
        erasedSize += SECTOR_SIZE;
        if (erasedSize >= partSize)
        {
            break;
        }
        dogs++;
        if (dogs >= 9)
        {
            PRINTF("dogs > 9,bsp_feed_watch_dog!\r\n");
            bsp_feed_watch_dog();
            dogs = 0;
        }
    }
    vTaskDelay(1);
//    taskEXIT_CRITICAL();
    SCB_EnableDCache();
    return MEM_OK;
}

/**
  * @brief  写缓存区内容到flash特定页
  * @param  None
  * @retval None
  */
/*
  The W25Q32JV array is organized into 16,384 programmable pages of 256-bytes each.
  Up to 256 bytes can be programmed at a time. Pages can be erased in groups of 16(4KB sector erase),
  groups of 128(32KB block erase), groups of 256 (64KB block erase) or the entire chip (chip erase).
  The W25Q32JV has 1,024 erasable sectors and 64 erasable blocks respectively.
  The small 4KB sectors allow for greater flexibility in applications that require data and parameter storage.

  Page Program Time is 3ms.
*/
static uint8_t gFlashBuf[512];
uint32_t bsp_flash_write_buffer(uint32_t startAddr, unsigned char *pBuff, uint32_t size)
{
    startAddr -= KALYKE_FLEXSPI_AMBA_BASE;
    uint16_t idx = 0;
    int32_t progSizeLeft = size;
    uint32_t curAddr = startAddr;
    status_t status;

    SCB_DisableDCache();
    PRINTF("Enter %s(), startAddr=0x%08X, pBuff = 0x%08X, size=%u\r\n", __func__, startAddr, pBuff, size);
    if (size > FLASH_PAGE_SIZE)
    {
        goto HANDLE_BIG_SIZE;
    }
    //status = flexspi_nor_flash_page_program_bytes(KALYKE_FLEXSPI, startAddr, (void *)pBuff, size);
    if (((uint32_t)pBuff % 4) != 0)
    {
        PRINTF("gFlashBuf = 0x%08X\r\n", gFlashBuf);
        memcpy(gFlashBuf, pBuff, size);
        status = mflash_drv_write_page_bytes((void*)startAddr, (uint32_t*)gFlashBuf, size);
    }
    else
    {
        status = mflash_drv_write_page_bytes((void*)startAddr, (uint32_t*)pBuff, size);
    }
    SCB_EnableDCache();
    if (status != kStatus_Success)
    {
        PRINTF("Page program failure 1!\r\n");
        return MEM_ADDR_ERR;
    }
    else
    {
        //PRINTF("Page program success 1!\r\n");
        return MEM_OK;
    }
    
HANDLE_BIG_SIZE:
    PRINTF("HANDLE_BIG_SIZE\r\n");
    for (;;)
    {
        //status = flexspi_nor_flash_page_program_bytes(KALYKE_FLEXSPI, curAddr, (void *)pBuff, FLASH_PAGE_SIZE);
        if (((uint32_t)pBuff % 4) != 0)
        {
            PRINTF("gFlashBuf = 0x%08X\r\n", gFlashBuf);
            memcpy(gFlashBuf, pBuff, FLASH_PAGE_SIZE);
            status = mflash_drv_write_page((void*)curAddr, (uint32_t*)gFlashBuf);
        }
        else
        {
            status = mflash_drv_write_page((void*)curAddr, (uint32_t*)pBuff);
        }
        if (status != kStatus_Success)
        {
            PRINTF("Page program failure 2!\r\n");
            SCB_EnableDCache();
            return MEM_ADDR_ERR;
        }
        else
        {
            //PRINTF("Page program success 2!\r\n");
        }
        curAddr += FLASH_PAGE_SIZE;
        pBuff += FLASH_PAGE_SIZE;
        progSizeLeft -= FLASH_PAGE_SIZE;
        if (progSizeLeft <= FLASH_PAGE_SIZE)
        {
            //status = flexspi_nor_flash_page_program_bytes(KALYKE_FLEXSPI, curAddr, (void *)pBuff, progSizeLeft);
            if (((uint32_t)pBuff % 4) != 0)
            {
                memcpy(gFlashBuf, pBuff, progSizeLeft);
                status = mflash_drv_write_page_bytes((void*)curAddr, (uint32_t*)gFlashBuf, progSizeLeft);
            }
            else
            {
                status = mflash_drv_write_page_bytes((void*)curAddr, (uint32_t*)pBuff, progSizeLeft);
            }
            if (status != kStatus_Success)
            {
                PRINTF("Page program failure 3!\r\n");
                SCB_EnableDCache();
                return MEM_ADDR_ERR;
            }
            else
            {
                PRINTF("Page program success 3!\r\n");
            }
            break;
        }
        idx++;
        if (idx > 200) 
        {
            PRINTF("idx > 200,bsp_feed_watch_dog!\r\n");
            bsp_feed_watch_dog();
            idx = 0;
        }
    }
    SCB_EnableDCache();
    return MEM_OK;
}

void bsp_write_m_flash_config(uint32_t addRess)
{
    uint8_t xip[600];
    memcpy(xip, &qspiflash_config, sizeof(flexspi_nor_config_t));
    bsp_flash_write_buffer(addRess, 
                    xip, 
                    sizeof(flexspi_nor_config_t));
}

/**
 * 参考line 57，在0x60300000 ~ 0x603FFFFF空间中，
 * 我们选取一处记录当前正在使用的是哪个bin固件
 * 
 * 因为最小erase单元是4K，所以为了充分利用资源，当更新一个变量值时，
 * 不是每次都erase一个4K，而是当这4K区域都写操作一次之后，再erase，
 * 从而再重新使用这个4K区域。这个4K区域命名为SECTOR_ID。
 * 对于1M大小的区域，可以有256个4K区域，故可以存储256个独立变量。
 * 每个4K区域，根据所存变量的大小，划分为同等大小的block，需4字节对齐。
 * 为了存储0xFFFFFFFF这样的特殊变量，最小的block大小为8字节。每个变量
 * 头4个字节定义为0xABABABAB
*/
#define PARAMETER_BASE_ADDRESS  0x60300000
#define PARAMETER_SECTOR_SIZE   0x1000
#define PARAMETER_MAGIC         0xABABABABu
static inline uint32_t get_addr(uint32_t sectorID, uint32_t offset)
{
    uint32_t address = PARAMETER_BASE_ADDRESS;
    address += (PARAMETER_SECTOR_SIZE * sectorID);
    address += offset;
    return address;
}

/* .......................................................................................................... */
/* 存储当前系统运行的是哪个bin固件，只存在0和1两个值 */
#define BLOCK_BIN_SECTOR_ID  0
#define BLOCK_SIZE_BIN       8
#define BLOCK_NUMBERS_BIN    512 // BLOCK_SIZE_BIN x BLOCK_NUMBER_BIN = 4096
static uint32_t gBinCurOffset = 0;
//Return which bin are current running
uint32_t bsp_get_image_idx(void)
{
    uint32_t offset = gBinCurOffset == 0 ? gBinCurOffset : gBinCurOffset - BLOCK_SIZE_BIN;
    uint32_t addr = get_addr(BLOCK_BIN_SECTOR_ID, offset);
    uint32_t *pIdx = (uint32_t *)(addr + sizeof(uint32_t));
    return *pIdx;
}
void bsp_save_image_idx(uint32_t idx)
{
    if (gBinCurOffset + BLOCK_SIZE_BIN >= PARAMETER_SECTOR_SIZE)
    {
        gBinCurOffset = 0;
        mem_part_info_t mpi;
        mpi.startAddr = PARAMETER_BASE_ADDRESS + BLOCK_BIN_SECTOR_ID * PARAMETER_SECTOR_SIZE;
        mpi.partSize = PARAMETER_SECTOR_SIZE;
        bsp_flash_erase_partition(&mpi);
    }
    uint32_t addr = get_addr(BLOCK_BIN_SECTOR_ID, gBinCurOffset);
    uint32_t array[2];
    array[0] = PARAMETER_MAGIC;
    array[1] = idx;
    bsp_flash_write_buffer(addr, (unsigned char *)array, BLOCK_SIZE_BIN);
    gBinCurOffset += BLOCK_SIZE_BIN;
    //PRINTF("gBinCurOffset = %u\r\n", gBinCurOffset);
}
static void init_image_idx(void)
{
    uint16_t i;
    uint32_t addr = PARAMETER_BASE_ADDRESS + BLOCK_BIN_SECTOR_ID * PARAMETER_SECTOR_SIZE;
    uint32_t *pValBegin = (uint32_t *)addr;
    uint32_t *pValCur = pValBegin;
    for (i = 0; i < BLOCK_NUMBERS_BIN; i++)
    {
        if (*pValCur != PARAMETER_MAGIC)
        {
            break;
        }
        pValCur++; pValCur++; // BLOCK_SIZE_BIN / sizeof(uint32_t)
    }
    gBinCurOffset = i * BLOCK_SIZE_BIN;
    //PRINTF("gBinCurOffset = %u, pValCur=0x%08X, pValBegin=0x%08X\r\n", gBinCurOffset, pValCur, pValBegin);
}

/* .......................................................................................................... */
/* 存储image ID，配合image idx来决定运行哪个bin 
 * 因为不可能存在0xFFFFFFFF这样的值，故不需要0xABABABAB头标示，
 * block size取4
 */
#define BLOCK_IMAGE_ID_SECTOR_ID  1
#define BLOCK_SIZE_IMAGE_ID       4
#define BLOCK_NUMBERS_IMAGE_ID    1024 // BLOCK_SIZE_IMAGE_ID x BLOCK_NUMBERS_IMAGE_ID = 4096
static uint32_t gImageIDCurOffset = 0;
// Return ROM ID.
uint32_t bsp_get_imageID(void)
{
    uint32_t offset = gImageIDCurOffset == 0 ? gImageIDCurOffset : gImageIDCurOffset - BLOCK_SIZE_IMAGE_ID;
    uint32_t addr = get_addr(BLOCK_IMAGE_ID_SECTOR_ID, offset);
    uint32_t *pIdx = (uint32_t *)addr;
    return *pIdx;
}
void bsp_save_imageID(uint32_t idx)
{
    PRINTF("Enter %s(), idx = %u, gImageIDCurOffset = %u\r\n", __func__, idx, gImageIDCurOffset);
    if (gImageIDCurOffset + BLOCK_SIZE_IMAGE_ID >= PARAMETER_SECTOR_SIZE)
    {
        gImageIDCurOffset = 0;
        mem_part_info_t mpi;
        mpi.startAddr = PARAMETER_BASE_ADDRESS + BLOCK_IMAGE_ID_SECTOR_ID * PARAMETER_SECTOR_SIZE;
        mpi.partSize = PARAMETER_SECTOR_SIZE;
        bsp_flash_erase_partition(&mpi);
    }
    uint32_t addr = get_addr(BLOCK_IMAGE_ID_SECTOR_ID, gImageIDCurOffset);
    bsp_flash_write_buffer(addr, (unsigned char *)&idx, BLOCK_SIZE_IMAGE_ID);
    gImageIDCurOffset += BLOCK_SIZE_IMAGE_ID;
    PRINTF("%s: gImageIDCurOffset = %u\r\n", __func__, gImageIDCurOffset);
}
void bsp_imageID_plus_one(void)
{
    uint32_t imageID = bsp_get_imageID();
    PRINTF("Enter %s(), imageID = %u\r\n", __func__, imageID);
    //走到这里，说明的新的bin已经写入Flash，所以先必须加一
    bsp_save_imageID(imageID + 1);
    //vTaskDelay(30);
}

static void init_imageID(void)
{
    uint16_t i;
    uint32_t addr = PARAMETER_BASE_ADDRESS + BLOCK_IMAGE_ID_SECTOR_ID * PARAMETER_SECTOR_SIZE;
    uint32_t *pValBegin = (uint32_t *)addr;
    uint32_t *pValCur = pValBegin;
    for (i = 0; i < BLOCK_NUMBERS_IMAGE_ID; i++)
    {
        if (*pValCur == 0xFFFFFFFF)
        {
            break;
        }
        pValCur++; // BLOCK_SIZE_IMAGE_ID / sizeof(uint32_t)
    }
    gImageIDCurOffset = i * BLOCK_SIZE_IMAGE_ID;
    PRINTF("gImageIDCurOffset = %u, pValCur=0x%08X, pValBegin=0x%08X\r\n", gImageIDCurOffset, pValCur, pValBegin);
}

/* .......................................................................................................... */
/* 存储gKalykeSecondTick，系统运行的秒数时间
 * 因为不可能存在0xFFFFFFFF这样的值，故不需要0xABABABAB头标识，
 * block size取4
 */
#define BLOCK_KALYKE_SECOND_TICK_SECTOR_ID  2
#define BLOCK_SIZE_KALYKE_SECOND_TICK       4
#define BLOCK_NUMBERS_KALYKE_SECOND_TICK    1024 // BLOCK_SIZE_KALYKE_SECOND_TICK x BLOCK_NUMBERS_KALYKE_SECOND_TICK = 4096
static uint32_t gKalykeSecondCurOffset = 0;

uint32_t bsp_get_KalykeSecondTick(void)
{
    uint32_t offset = gKalykeSecondCurOffset == 0 ? gKalykeSecondCurOffset : gKalykeSecondCurOffset - BLOCK_SIZE_KALYKE_SECOND_TICK;
    uint32_t addr = get_addr(BLOCK_KALYKE_SECOND_TICK_SECTOR_ID, offset);
    uint32_t *pIdx = (uint32_t *)addr;
    return *pIdx;
}
void bsp_save_KalykeSecondTick(uint32_t tick)
{
    LOGW("bsp_flash", "Enter %s(), tick = %u, gKalykeSecondCurOffset = %u\r\n", __func__, tick, gKalykeSecondCurOffset);
    if (gKalykeSecondCurOffset + BLOCK_SIZE_KALYKE_SECOND_TICK >= PARAMETER_SECTOR_SIZE)
    {
        gKalykeSecondCurOffset = 0;
        mem_part_info_t mpi;
        mpi.startAddr = PARAMETER_BASE_ADDRESS + BLOCK_KALYKE_SECOND_TICK_SECTOR_ID * PARAMETER_SECTOR_SIZE;
        mpi.partSize = PARAMETER_SECTOR_SIZE;
        bsp_flash_erase_partition(&mpi);
    }
    uint32_t addr = get_addr(BLOCK_KALYKE_SECOND_TICK_SECTOR_ID, gKalykeSecondCurOffset);
    bsp_flash_write_buffer(addr, (unsigned char *)&tick, BLOCK_SIZE_KALYKE_SECOND_TICK);
    gKalykeSecondCurOffset += BLOCK_SIZE_KALYKE_SECOND_TICK;
    //LOGW("bsp_flash", "%s: gKalykeSecondCurOffset = %u\r\n", __func__, gKalykeSecondCurOffset);
}
static void init_KalykeSecondTick(void)
{
    uint16_t i;
    uint32_t addr = PARAMETER_BASE_ADDRESS + BLOCK_KALYKE_SECOND_TICK_SECTOR_ID * PARAMETER_SECTOR_SIZE;
    uint32_t *pValBegin = (uint32_t *)addr;
    uint32_t *pValCur = pValBegin;
    for (i = 0; i < BLOCK_NUMBERS_KALYKE_SECOND_TICK; i++)
    {
        if (*pValCur == 0xFFFFFFFF)
        {
            break;
        }
        pValCur++;
    }
    gKalykeSecondCurOffset = i * BLOCK_SIZE_KALYKE_SECOND_TICK;
    LOGW("bsp_flash", "gKalykeSecondCurOffset = %u, pValCur=0x%08X, pValBegin=0x%08X\r\n", gKalykeSecondCurOffset, pValCur, pValBegin);
}

/* .......................................................................................................... */
/* 存储系统启动次数
 * 因为不可能存在0xFFFFFFFF这样的值，故不需要0xABABABAB头标识，
 * block size取4
 */
#define BLOCK_BOOT_TIME_SECTOR_ID  3
#define BLOCK_SIZE_BOOT_TIME       4
#define BLOCK_NUMBERS_BOOT_TIME    1024 // BLOCK_SIZE_BOOT_TIME * BLOCK_NUMBERS_BOOT_TIME = 4096
static uint32_t gBootTimeCurOffset = 0;

uint32_t bsp_get_BootTime(void)
{
    uint32_t offset = gBootTimeCurOffset == 0 ? gBootTimeCurOffset : gBootTimeCurOffset - BLOCK_SIZE_BOOT_TIME;
    uint32_t addr = get_addr(BLOCK_BOOT_TIME_SECTOR_ID, offset);
    uint32_t *pIdx = (uint32_t *)addr;
    return *pIdx;
}
void bsp_save_BootTime(uint32_t bootTime)
{
    LOGW("bsp_flash", "Enter %s(), bootTime = %u, gBootTimeCurOffset = %u\r\n", __func__, bootTime, gBootTimeCurOffset);
    if (gBootTimeCurOffset + BLOCK_SIZE_BOOT_TIME >= PARAMETER_SECTOR_SIZE)
    {
        gBootTimeCurOffset = 0;
        mem_part_info_t mpi;
        mpi.startAddr = PARAMETER_BASE_ADDRESS + BLOCK_BOOT_TIME_SECTOR_ID * PARAMETER_SECTOR_SIZE;
        mpi.partSize = PARAMETER_SECTOR_SIZE;
        bsp_flash_erase_partition(&mpi);
    }
    uint32_t addr = get_addr(BLOCK_BOOT_TIME_SECTOR_ID, gBootTimeCurOffset);
    bsp_flash_write_buffer(addr, (unsigned char *)&bootTime, BLOCK_SIZE_BOOT_TIME);
    gBootTimeCurOffset += BLOCK_SIZE_BOOT_TIME;
    LOGW("bsp_flash", "%s: gBootTimeCurOffset = %u\r\n", __func__, gBootTimeCurOffset);
}
static void init_BootTime(void)
{
    uint16_t i;
    uint32_t addr = PARAMETER_BASE_ADDRESS + BLOCK_BOOT_TIME_SECTOR_ID * PARAMETER_SECTOR_SIZE;
    uint32_t *pValBegin = (uint32_t *)addr;
    uint32_t *pValCur = pValBegin;
    for (i = 0; i < BLOCK_NUMBERS_BOOT_TIME; i++)
    {
        if (*pValCur == 0xFFFFFFFF)
        {
            break;
        }
        pValCur++;
    }
    gBootTimeCurOffset = i * BLOCK_SIZE_BOOT_TIME;
    LOGW("bsp_flash", "gBootTimeCurOffset = %u, pValCur=0x%08X, pValBegin=0x%08X, boottime = %u\r\n", gBootTimeCurOffset, pValCur, pValBegin, bsp_get_BootTime());
}

/* .......................................................................................................... */
/* 存储Image 1是否已重启过，0 = 尚未重启；1 = 已重启过，不需重启，直接运行Image 1
 * 因为不可能存在0xFFFFFFFF这样的值，故不需要0xABABABAB头标识，
 * block size取4
 */
#define BLOCK_SECTOR_ID_IMAGE_1  4
#define BLOCK_SIZE_IMAGE_1       4
#define BLOCK_NUMBERS_IMAGE_1    1024 // BLOCK_SIZE_IMAGE_1 * BLOCK_NUMBERS_IMAGE_1 = 4096
static uint32_t gImage1CurOffset = 0;

uint32_t bsp_get_Image1IfReset(void)
{
    uint32_t offset = gImage1CurOffset == 0 ? gImage1CurOffset : gImage1CurOffset - BLOCK_SIZE_IMAGE_1;
    uint32_t addr = get_addr(BLOCK_SECTOR_ID_IMAGE_1, offset);
    uint32_t *pIdx = (uint32_t *)addr;
    return *pIdx;
}
void bsp_save_Image1IfReset(uint32_t flag)
{
    LOGV("bsp_flash", "Enter %s(), flag = %u, gImage1CurOffset = %u\r\n", __func__, flag, gImage1CurOffset);
    if (gImage1CurOffset + BLOCK_SIZE_IMAGE_1 >= PARAMETER_SECTOR_SIZE)
    {
        gImage1CurOffset = 0;
        mem_part_info_t mpi;
        mpi.startAddr = PARAMETER_BASE_ADDRESS + BLOCK_SECTOR_ID_IMAGE_1 * PARAMETER_SECTOR_SIZE;
        mpi.partSize = PARAMETER_SECTOR_SIZE;
        bsp_flash_erase_partition(&mpi);
    }
    uint32_t addr = get_addr(BLOCK_SECTOR_ID_IMAGE_1, gImage1CurOffset);
    bsp_flash_write_buffer(addr, (unsigned char *)&flag, BLOCK_SIZE_IMAGE_1);
    gImage1CurOffset += BLOCK_SIZE_IMAGE_1;
    LOGW("bsp_flash", "%s: gImage1CurOffset = %u\r\n", __func__, gImage1CurOffset);
}
static void init_Image1IfReset(void)
{
    uint16_t i;
    uint32_t addr = PARAMETER_BASE_ADDRESS + BLOCK_SECTOR_ID_IMAGE_1 * PARAMETER_SECTOR_SIZE;
    uint32_t *pValBegin = (uint32_t *)addr;
    uint32_t *pValCur = pValBegin;
    for (i = 0; i < BLOCK_NUMBERS_IMAGE_1; i++)
    {
        if (*pValCur == 0xFFFFFFFF)
        {
            break;
        }
        pValCur++;
    }
    gImage1CurOffset = i * BLOCK_SIZE_IMAGE_1;
    LOGW("bsp_flash", "gImage1CurOffset = %u, pValCur=0x%08X, pValBegin=0x%08X, ifReset = %u", gImage1CurOffset, pValCur, pValBegin, bsp_get_Image1IfReset());
}

/* .......................................................................................................... */
/* 存储Image 1是否已重启过，0 = 尚未重启；1 = 已重启过，不需重启，直接运行Image 1
 * 因为不可能存在0xFFFFFFFF这样的值，故不需要0xABABABAB头标识，
 * block size取4
 */
#define BLOCK_SECTOR_ID_TOUCHUAN  5
#define BLOCK_SIZE_TOUCHUAN       4
#define BLOCK_NUMBERS_TOUCHUAN    1024 // BLOCK_SIZE_TOUCHUAN * BLOCK_NUMBERS_TOUCHUAN = 4096
static uint32_t gTouchuanCurOffset = 0;

uint32_t bsp_get_TouChuan(void)
{
    uint32_t offset = gTouchuanCurOffset == 0 ? gTouchuanCurOffset : gTouchuanCurOffset - BLOCK_SIZE_TOUCHUAN;
    uint32_t addr = get_addr(BLOCK_SECTOR_ID_TOUCHUAN, offset);
    uint32_t *pIdx = (uint32_t *)addr;
    return *pIdx;
}
void bsp_save_TouChuan(uint32_t flag)
{
    LOGV("bsp_flash", "Enter %s(), flag = %u, gTouchuanCurOffset = %u\r\n", __func__, flag, gTouchuanCurOffset);
    if (gTouchuanCurOffset + BLOCK_SIZE_TOUCHUAN >= PARAMETER_SECTOR_SIZE)
    {
        gTouchuanCurOffset = 0;
        mem_part_info_t mpi;
        mpi.startAddr = PARAMETER_BASE_ADDRESS + BLOCK_SECTOR_ID_TOUCHUAN * PARAMETER_SECTOR_SIZE;
        mpi.partSize = PARAMETER_SECTOR_SIZE;
        bsp_flash_erase_partition(&mpi);
    }
    uint32_t addr = get_addr(BLOCK_SECTOR_ID_TOUCHUAN, gTouchuanCurOffset);
    bsp_flash_write_buffer(addr, (unsigned char *)&flag, BLOCK_SIZE_TOUCHUAN);
    gTouchuanCurOffset += BLOCK_SIZE_TOUCHUAN;
    LOGW("bsp_flash", "%s: gTouchuanCurOffset = %u\r\n", __func__, gTouchuanCurOffset);
}
static void init_TouChuan(void)
{
    uint16_t i;
    uint32_t addr = PARAMETER_BASE_ADDRESS + BLOCK_SECTOR_ID_TOUCHUAN * PARAMETER_SECTOR_SIZE;
    uint32_t *pValBegin = (uint32_t *)addr;
    uint32_t *pValCur = pValBegin;
    for (i = 0; i < BLOCK_NUMBERS_TOUCHUAN; i++)
    {
        if (*pValCur == 0xFFFFFFFF)
        {
            break;
        }
        pValCur++;
    }
    gTouchuanCurOffset = i * BLOCK_SIZE_TOUCHUAN;

    gTouChuan = bsp_get_TouChuan();
    LOGW("bsp_flash", "gTouchuanCurOffset = %u, pValCur=0x%08X, pValBegin=0x%08X, gTouChuan = 0x%X", gTouchuanCurOffset, pValCur, pValBegin, gTouChuan);
}


/* ********************************************************************************************** */
static void parameter_init(void)
{
    init_image_idx();
    init_imageID();
    init_KalykeSecondTick();
    init_BootTime();
    init_Image1IfReset();
    init_TouChuan();
}

status_t bsp_flash_init(void)
{
    PRINTF("Enter %s\r\n", __func__);
    int32_t ret = mflash_drv_init();
    parameter_init();
    return ret;
}

