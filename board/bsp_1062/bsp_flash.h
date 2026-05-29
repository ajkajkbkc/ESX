/**
  ******************************************************************************
  * @file    bsp_flash.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   内部flash操作接口
  ******************************************************************************
  */

#ifndef __BSP_FLASH_H
#define __BSP_FLASH_H
#include "bsp.h"
#include "fsl_flexspi.h"
#include "mflash_drv.h"


#define MEM_OK  0
#define MEM_ADDR_ERR    1
#define MAX_FLASH_PAGE_NUM  24

typedef struct _MEM_PART_INFO_T{
    /*分区开始地址，一般为page的开始位置*/
    uint32_t    startAddr;
    /*分区大小*/
    uint32_t    partSize;
} mem_part_info_t;

typedef struct _FLASH_PAGE_INFO_T{
    uint16_t    pageNum;
    uint32_t    startAddr;
    uint32_t    endAddr;
} flash_part_info_t;

/*------------------------------------------------------------------------------
* 内部FLASH分区定义
*-----------------------------------------------------------------------------*/
/*bootloader, 占用Page 0, 1, 2页, 共 48K*/
#define BOOTLOADER_START_FLASH_PAGE         0

/*设备信息, 占用Page 3, 共16K*/
#define DEV_INFO_START_FLASH_PAGE           3

/*掉电保持数据，占用Page4, 64K*/
#define PLSD_SAVE_DATA_FLASH_PAGE           4
#define MAX_PLSD_SAVE_DATA_SIZE             64*1024

/*PLC主程序, 占用Page 5,6,7,8,9, 共640K*/
#define PLC_CODE_START_FLASH_PAGE           5
#define PLC_CODE_PAGE_SIZE                  128*1024
#define PLC_CODE_PAGE_NUM                   5

/*设备网络参数配置信息, 占用Page 10, 共128K*/
#define DEV_NETWORK_INFO_START_FLASH_PAGE   10
#define DEV_NETWORK_INFO_SIZE               128*1024

/*云数据表, 占用Page 11, 共128K*/
#define CLOUD_DATA_TABLE_START_FLASH_PAGE   11

/*用户下载\上载等密码保存, 占用Page 12, 共16K*/
#define USER_PASSWORD_START_FLASH_PAGE      12

/*系统块, 占用Page 13, 共64K*/
#define SYS_BLOCK_START_PAGE                13
#define SYS_BLOCK_MAX_SIZE                  64*1024

/*POU信息, 占用Page 14, 共16K*/
#define POU_FILE_START_PAGE                 14
#define POU_FILE_MAX_SIZE                   16*1024

/*全局变量表, 占用Page 15, 共16K*/
#define GVT_FILE_START_PAGE                 15
#define GVT_FILE_MAX_SIZE                   16*1024

/*数据块, 占用Page 16, 共64K*/
#define DATA_BLOCK_START_PAGE               16
#define DATA_BLOCK_MAX_SIZE                 64*1024

/*UCODE 代码, 占用Page 17 18, 共256K*/
#define UCODE_FILE_START_PAGE               17
#define UCODE_FILE_MAX_SIZE                 256*1024

/*cbin区域, 占用Page 19, 20, 21, 22,23 共640K*/
#define CBIN_UPGRADE_START_PAGE             19
#define CBIN_MAX_SIZE                       128*1024

/*PID参数1配置信息, 占用Page 23, 共4K*/
#define PID1_START_PAGE                      22
#define PID1_MAX_SIZE                        4*1024

/*PID参数2配置信息, 占用Page 23, 共4K*/
#define PID2_START_PAGE                      23
#define PID2_MAX_SIZE                        4*1024


//Kalyke的OTA固件大小
#define SYSTEM_UPGRADE_MAX_SIZE             500*1024


#define KALYKE_FLEXSPI MFLASH_FLEXSPI
#define KALYKE_FLASH_SIZE MFLASH_SIZE /* 32Mb/KByte */ 
#define KALYKE_FLEXSPI_AMBA_BASE FlexSPI_AMBA_BASE
#define FLASH_PAGE_SIZE MFLASH_PAGE_SIZE // bytes
#define SECTOR_SIZE MFLASH_SECTOR_SIZE /* 4K */

#define EXAMPLE_FLEXSPI_CLOCK kCLOCK_FlexSpi
#define EXAMPLE_SECTOR 103

//void BOARD_InitHardware(void);
static inline void flexspi_clock_init(void)
{
#if defined(XIP_EXTERNAL_FLASH) && (XIP_EXTERNAL_FLASH == 1)
    CLOCK_SetMux(kCLOCK_FlexspiMux, 0x2); /* Choose PLL2 PFD2 clock as flexspi source clock. 396M */
    CLOCK_SetDiv(kCLOCK_FlexspiDiv, 2);   /* flexspi clock 133M. */
#else
    const clock_usb_pll_config_t g_ccmConfigUsbPll = {.loopDivider = 0U};
    CLOCK_InitUsb1Pll(&g_ccmConfigUsbPll);
    CLOCK_InitUsb1Pfd(kCLOCK_Pfd0, 24);   /* Set PLL3 PFD0 clock 360MHZ. */
    CLOCK_SetMux(kCLOCK_FlexspiMux, 0x3); /* Choose PLL3 PFD0 clock as flexspi source clock. */
    CLOCK_SetDiv(kCLOCK_FlexspiDiv, 2);   /* flexspi clock 120M. */
#endif
}

/*------------------------------------------------------------------------------
* 函数申明
*-----------------------------------------------------------------------------*/
unsigned char bsp_flash_erase_partition(mem_part_info_t *pPart);
uint32_t bsp_flash_write_buffer(uint32_t startAddr, unsigned char *pBuff, uint32_t size);
flash_part_info_t * bsp_get_flash_info(unsigned char page);
void bsp_write_m_flash_config(uint32_t addRess);
uint32_t bsp_get_image_idx(void);
void bsp_save_image_idx(uint32_t idx);
uint32_t bsp_get_imageID(void);
void bsp_save_imageID(uint32_t idx);
void bsp_imageID_plus_one(void);
uint32_t bsp_get_KalykeSecondTick(void);
void bsp_save_KalykeSecondTick(uint32_t tick);
uint32_t bsp_get_BootTime(void);
void bsp_save_BootTime(uint32_t tick);



extern status_t flexspi_nor_flash_erase_sector(FLEXSPI_Type *base, uint32_t address);
extern status_t flexspi_nor_flash_page_program(FLEXSPI_Type *base, uint32_t dstAddr, const uint32_t *src);
extern status_t flexspi_nor_flash_page_program_bytes(FLEXSPI_Type *base, uint32_t dstAddr, const uint32_t *src, size_t size);
extern status_t flexspi_nor_get_vendor_id(FLEXSPI_Type *base, uint8_t *vendorId);
extern status_t flexspi_nor_enable_quad_mode(FLEXSPI_Type *base);
extern status_t flexspi_nor_erase_chip(FLEXSPI_Type *base);
extern void flexspi_nor_flash_init(FLEXSPI_Type *base);
extern status_t bsp_flash_init(void);
extern uint32_t bsp_get_Image1IfReset(void);
extern void bsp_save_Image1IfReset(uint32_t flag);
extern void bsp_save_TouChuan(uint32_t flag);
extern uint32_t bsp_get_TouChuan(void);
#endif /*__BSP_FLASH_H*/

