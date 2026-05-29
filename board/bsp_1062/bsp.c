/**
  ******************************************************************************
  * @file    bsp.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   系统BSP初始化
  ******************************************************************************
  */

#include "bsp.h"
#include "FreeRTOSConfig.h"
#include "fsl_common.h"
#include "fsl_trng.h"
#include "fsl_snvs_hp.h"
#include "fsl_snvs_lp.h"
#include "fsl_debug_console.h"
//#include "fsl_flexram.h"
#include "fsl_ocotp.h"

#include "portmacro.h"
//#include "bsp_sdram.h"
//#include "bsp_mem.h"
#include "bsp_dct.h"
#include "bsp_led.h"
#include "bsp_tim.h"
#include "bsp_gpio.h"
//#include "bsp_usb.h"
#include "bsp_iwdg.h"
#include "bsp_flash.h"

#include "pin_mux.h"
#include "board.h"
#include "clock_config.h"
#include "kalyke_version.h"
#include "kalyke_ota.h"
#include "kalyke_monitor_task.h"
#include "kalyke_opts.h"
#include "kalyke_tool.h"
#include "kalyke_sntp_task.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint32_t gKalykeBootTime;
bool gBspIam1970 = false;
uint16_t gResetReason = 0;
uint16_t gWakeupSource = 0;

/*******************************************************************************
 * Code
 ******************************************************************************/


static void show_reset_log(void)
{
    uint16_t resetFlag = WDOG_GetStatusFlags(KALYKE_WDOG_BASE);

    switch (resetFlag & (kWDOG_PowerOnResetFlag | kWDOG_TimeoutResetFlag | kWDOG_SoftwareResetFlag))
    {
        case kWDOG_PowerOnResetFlag:
            LOGE("bsp", " Power On Reset!\r\n");
            gResetReason = kWDOG_PowerOnResetFlag;
            break;
        case kWDOG_TimeoutResetFlag:
            LOGE("bsp", " Time Out Reset!\r\n");
            gResetReason = kWDOG_TimeoutResetFlag;
            break;
        case kWDOG_SoftwareResetFlag:
            LOGE("bsp", " Software Reset!\r\n");
            gResetReason = kWDOG_SoftwareResetFlag;
            break;
        default:
            LOGE("bsp", " Error Reset status!");
            gResetReason = resetFlag & (kWDOG_PowerOnResetFlag | kWDOG_TimeoutResetFlag | kWDOG_SoftwareResetFlag);
            break;
    }
    LOGE("bsp", "\r\nCPU wakeup source 0x%X", SRC->SRSR);
    gWakeupSource = SRC->SRSR;
}

#if (KALYKE_LP_RTC == 1)
static status_t SNVS_HP_RTC_SetDatetimeBySecond(SNVS_Type *base, uint32_t sec)
{
    uint32_t tmp = base->HPCR;

    /* disable RTC */
    SNVS_LP_SRTC_StopTimer(base);

    sec += 28800; // 8 hours
    base->LPSRTCMR = (uint32_t)(sec >> 17U);
    base->LPSRTCLR = (uint32_t)(sec << 15U);

    /* reenable RTC in case that it was enabled before */
    if (tmp & SNVS_HPCR_RTC_EN_MASK)
    {
        SNVS_LP_SRTC_StartTimer(base);
    }
    SNVS_HP_RTC_TimeSynchronize(SNVS);
    return kStatus_Success;
}
#else
static status_t SNVS_HP_RTC_SetDatetimeBySecond(SNVS_Type *base, uint32_t sec)
{
    uint32_t tmp = base->HPCR;

    /* disable RTC */
    SNVS_HP_RTC_StopTimer(base);

    sec += 28800; // 8 hours
    base->HPRTCMR = (uint32_t)(sec >> 17U);
    base->HPRTCLR = (uint32_t)(sec << 15U);

    /* reenable RTC in case that it was enabled before */
    if (tmp & SNVS_HPCR_RTC_EN_MASK)
    {
        SNVS_HP_RTC_StartTimer(base);
    }
    return kStatus_Success;
}
#endif

// 1个RTC的tick等于30.517578125微秒
RAMFUNCTION_SECTION_CODE(static void SNVS_HP_RTC_GetSecondAndUSecond(SNVS_Type *base, uint32_t *pSecond, uint32_t *pUSecond))
{
    float weiMiao = 30.517578125f; 
    uint32_t seconds = 0;
    uint32_t tmp = 0;
    uint32_t tick;
    uint32_t uHPRTCLR;

    /* Do consecutive reads until value is correct */
    do
    {
        seconds = tmp;
        uHPRTCLR = base->HPRTCLR;
        tmp = (base->HPRTCMR << 17U) | (uHPRTCLR >> 15U);
    } while (tmp != seconds);
    tick = uHPRTCLR & 0x7FFFU;
    //LOGW("fsl_snvs", "tick = %u", tick);
    *pSecond = seconds;
    *pUSecond = tick * weiMiao;
}

RAMFUNCTION_SECTION_CODE(void kalyke_getSystemTime(uint32_t *pSecond, uint32_t *pUSecond))
{
    //LOGV("bsp", "Enter %s()", __func__);
    SNVS_HP_RTC_GetSecondAndUSecond(SNVS, pSecond, pUSecond);
    //LOGV("bsp", "%s(): second = %u, usec = %u", __func__, *pSecond, *pUSecond);
}

RAMFUNCTION_SECTION_CODE(uint32_t kalyke_SNVS_HP_RTC_GetDatetime(snvs_hp_rtc_datetime_t *datetime))
{
    uint32_t seconds, milliSeconds;
    SNVS_HP_RTC_GetSecondAndUSecond(SNVS, &seconds, &milliSeconds);
    SNVS_HP_ConvertSecondsToDatetime(seconds, datetime);
    return milliSeconds / 1000;
}

void kalyke_setRTC(uint32_t sec)
{
    snvs_hp_rtc_datetime_t rtcDate;
    LOGI("bsp", "Enter %s(), sec = %u", __func__, sec);
    gWorldSecondTick = sec;
    if (SNVS_HP_RTC_SetDatetimeBySecond(SNVS, sec) == kStatus_Success)
    {
        LOGD("bsp", "Set RTC time by second SUCCESS!");
        SNVS_HP_RTC_GetDatetime(SNVS, &rtcDate);
        SET_SD_ELEMENT_VALUE(100, rtcDate.year);
        SET_SD_ELEMENT_VALUE(101, rtcDate.month);
        SET_SD_ELEMENT_VALUE(102, rtcDate.day);
        SET_SD_ELEMENT_VALUE(103, rtcDate.hour);
        SET_SD_ELEMENT_VALUE(104, rtcDate.minute);
        SET_SD_ELEMENT_VALUE(105, rtcDate.second);
        kalyke_ds1302_set_time(&rtcDate);
    }
    else
    {
        LOGE("bsp", "Set RTC time by second ERROR!");
    }
}

#if (KALYKE_LP_RTC == 1)
static void init_RTC(void)
{
    LOGE("bsp", "Enter %s()", __func__);
    snvs_lp_srtc_datetime_t srtcDate;
    snvs_hp_rtc_config_t snvsRtcConfig;
    snvs_lp_srtc_config_t snvsSrtcConfig;

    /* Init SNVS_HP */
    SNVS_HP_RTC_GetDefaultConfig(&snvsRtcConfig);
    //snvsRtcConfig.periodicInterruptFreq = 0xF;
    SNVS_HP_RTC_Init(SNVS, &snvsRtcConfig);

    /* Init SNVS_LP */
    SNVS_LP_SRTC_GetDefaultConfig(&snvsSrtcConfig);
    SNVS_LP_SRTC_Init(SNVS, &snvsSrtcConfig);
    SNVS_LP_SRTC_StartTimer(SNVS);

    SNVS_LP_SRTC_GetDatetime(SNVS, &srtcDate);
    LOGI("bsp", "SRTC: %04u-%02u-%02u  %02u:%02u:%02u\r\n", srtcDate.year, srtcDate.month, srtcDate.day, srtcDate.hour, srtcDate.minute, srtcDate.second);
    if (srtcDate.year == 1970U)
    {
        gBspIam1970 = true;
        srtcDate.year = 2023;
        SNVS_LP_SRTC_SetDatetime(SNVS, &srtcDate);
    }
    else
    {
        gBspIam1970 = false;
    }

    /* Synchronize RTC time and date with SRTC and start RTC */
    SNVS_HP_RTC_TimeSynchronize(SNVS);
    SNVS_HP_RTC_StartTimer(SNVS);
    LOGE("bsp", "RTC started!!");
}
#else
static void init_RTC(void)
{
    snvs_hp_rtc_datetime_t rtcDate;
    snvs_hp_rtc_config_t snvsRtcConfig;
    SNVS_HP_RTC_GetDefaultConfig(&snvsRtcConfig);
    //snvsRtcConfig.periodicInterruptFreq = 0xF;
    SNVS_HP_RTC_Init(SNVS, &snvsRtcConfig);
    /* Set a start date time and start RT */
    rtcDate.year = 2019U;
    rtcDate.month = 6U;
    rtcDate.day = 21U;
    rtcDate.hour = 22U;
    rtcDate.minute = 59;
    rtcDate.second = 11;

    /* Set RTC time to default time and date and start the RTC */
    SNVS_HP_RTC_SetDatetime(SNVS, &rtcDate);
    SNVS_HP_RTC_StartTimer(SNVS);
    LOGE("bsp", "RTC started!!");
}
#endif

/* 初始化随机数    For LwIP. */
static void init_rand(void)
{    
    trng_config_t trngConfig;
    /* Initialize TRNG configuration structure to default.*/
    /*
     * trngConfig.lock = TRNG_USER_CONFIG_DEFAULT_LOCK;
     * trngConfig.clockMode = kTRNG_ClockModeRingOscillator;
     * trngConfig.ringOscDiv = TRNG_USER_CONFIG_DEFAULT_OSC_DIV;
     * trngConfig.sampleMode = kTRNG_SampleModeRaw;
     * trngConfig.entropyDelay = TRNG_USER_CONFIG_DEFAULT_ENTROPY_DELAY;
     * trngConfig.sampleSize = TRNG_USER_CONFIG_DEFAULT_SAMPLE_SIZE;
     * trngConfig.sparseBitLimit = TRNG_USER_CONFIG_DEFAULT_SPARSE_BIT_LIMIT;
     * trngConfig.retryCount = TRNG_USER_CONFIG_DEFAULT_RETRY_COUNT;
     * trngConfig.longRunMaxLimit = TRNG_USER_CONFIG_DEFAULT_RUN_MAX_LIMIT;
     * trngConfig.monobitLimit.maximum = TRNG_USER_CONFIG_DEFAULT_MONOBIT_MAXIMUM;
     * trngConfig.monobitLimit.minimum = TRNG_USER_CONFIG_DEFAULT_MONOBIT_MINIMUM;
     * trngConfig.runBit1Limit.maximum = TRNG_USER_CONFIG_DEFAULT_RUNBIT1_MAXIMUM;
     * trngConfig.runBit1Limit.minimum = TRNG_USER_CONFIG_DEFAULT_RUNBIT1_MINIMUM;
     * trngConfig.runBit2Limit.maximum = TRNG_USER_CONFIG_DEFAULT_RUNBIT2_MAXIMUM;
     * trngConfig.runBit2Limit.minimum = TRNG_USER_CONFIG_DEFAULT_RUNBIT2_MINIMUM;
     * trngConfig.runBit3Limit.maximum = TRNG_USER_CONFIG_DEFAULT_RUNBIT3_MAXIMUM;
     * trngConfig.runBit3Limit.minimum = TRNG_USER_CONFIG_DEFAULT_RUNBIT3_MINIMUM;
     * trngConfig.runBit4Limit.maximum = TRNG_USER_CONFIG_DEFAULT_RUNBIT4_MAXIMUM;
     * trngConfig.runBit4Limit.minimum = TRNG_USER_CONFIG_DEFAULT_RUNBIT4_MINIMUM;
     * trngConfig.runBit5Limit.maximum = TRNG_USER_CONFIG_DEFAULT_RUNBIT5_MAXIMUM;
     * trngConfig.runBit5Limit.minimum = TRNG_USER_CONFIG_DEFAULT_RUNBIT5_MINIMUM;
     * trngConfig.runBit6PlusLimit.maximum = TRNG_USER_CONFIG_DEFAULT_RUNBIT6PLUS_MAXIMUM;
     * trngConfig.runBit6PlusLimit.minimum = TRNG_USER_CONFIG_DEFAULT_RUNBIT6PLUS_MINIMUM;
     * trngConfig.pokerLimit.maximum = TRNG_USER_CONFIG_DEFAULT_POKER_MAXIMUM;
     * trngConfig.pokerLimit.minimum = TRNG_USER_CONFIG_DEFAULT_POKER_MINIMUM;
     * trngConfig.frequencyCountLimit.maximum = TRNG_USER_CONFIG_DEFAULT_FREQUENCY_MAXIMUM;
     * trngConfig.frequencyCountLimit.minimum = TRNG_USER_CONFIG_DEFAULT_FREQUENCY_MINIMUM;
     */
    TRNG_GetDefaultConfig(&trngConfig);
    /* Set sample mode of the TRNG ring oscillator to Von Neumann, for better random data.
     * It is optional.*/
    trngConfig.sampleMode = kTRNG_SampleModeVonNeumann;

    /* Initialize TRNG */
    status_t status = TRNG_Init(TRNG, &trngConfig);

    if (kStatus_Success == status)
    {
        //PRINTF("TRNG init SUCCESS!!!\r\n");
    }
    else
    {
        //PRINTF("TRNG init FAIL!!!\r\n");
    }
#if 0
    uint32_t r;
    status = TRNG_GetRandomData(TRNG, &r, 4);
    if (status == kStatus_Success)
    {
        PRINTF("The random is : 0x%08X\r\n", r);
    }
#endif
}

#if 0
status_t OCRAM_Reallocate(void)
{
    SCB_DisableICache();
    SCB_DisableDCache();
    flexram_allocate_ram_t ramAllocate = {
        .ocramBankNum = 4,
        .dtcmBankNum = 4,
        .itcmBankNum = 8,
    };
#if 0
    PRINTF("\r\nAllocate on-chip ram:\r\n");
    PRINTF("\r\n   OCRAM bank numbers %d\r\n", ramAllocate.ocramBankNum);
    PRINTF("\r\n   DTCM  bank numbers %d\r\n", ramAllocate.dtcmBankNum);
    PRINTF("\r\n   ITCM  bank numbers %d\r\n", ramAllocate.itcmBankNum);

    PRINTF("IOMUXC_GPR->GPR14 = 0x%08X\r\n", IOMUXC_GPR->GPR14);
    PRINTF("IOMUXC_GPR->GPR16 = 0x%08X\r\n", IOMUXC_GPR->GPR16);
    PRINTF("IOMUXC_GPR->GPR17 = 0x%08X\r\n", IOMUXC_GPR->GPR17);
#endif
    if (FLEXRAM_AllocateRam(&ramAllocate) != kStatus_Success)
    {
        PRINTF("\r\nAllocate on-chip ram fail\r\n");
        return kStatus_Fail;
    }
    else
    {
        PRINTF("\r\nAllocate on-chip ram success\r\n");
        PRINTF("IOMUXC_GPR->GPR14 = 0x%08X\r\n", IOMUXC_GPR->GPR14);
        PRINTF("IOMUXC_GPR->GPR16 = 0x%08X\r\n", IOMUXC_GPR->GPR16);
        PRINTF("IOMUXC_GPR->GPR17 = 0x%08X\r\n", IOMUXC_GPR->GPR17);
    }

    SCB_EnableDCache();
    SCB_EnableICache();
    return kStatus_Success;
}
#endif

#if 0
void flexRAM_init(void)
{
    //PRINTF("Enter %s()\r\n", __func__);
    //PRINTF("offset: 0x6D0, OCOTP->MISC_CONF0 = 0x%08X\r\n", OCOTP->MISC_CONF0);
    
    uint32_t mask = 0x2u << 16;
    uint32_t val = OCOTP->MISC_CONF0;
    if ((val & mask))
    {
        //PRINTF("OH, OCRAM=128K, DTCM=128k, ITCM=256K. Continue boot...\r\n");
        return;
    }

    val = (val >> 16) & 0x0FU;
    //PRINTF("FlexRAM config: 0x%04X\r\n", val);

    status_t retStatus = ocotp_program_once(OCOTP, 0x2D, &mask, 4); // MISC_CONF0
    if (retStatus == kStatus_Success)
    {
        //PRINTF("ota_program_once_MISC_CONF0 Success!!\r\n");
    }
    else
    {
        //PRINTF("ota_program_once_MISC_CONF0 Error!!\r\n");
    }

    volatile uint32_t i = 0;
    for (i = 0; i < 500000000; ++i)
    {
        __asm("NOP"); /* delay */
    }
    NVIC_SystemReset();
}
#endif

/**
  * @brief  BSP初始化前部分，初始化系统运行必备资源
  * @param  None
  * @retval None
  */
#if 0
void bsp_init_pre(void)
{
    RCC_ClocksTypeDef RCC_Clocks;

    /*设置中断向量表偏移地址*/
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x20000);

    /*配置系统中断优先级分组*/
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    /* 根据FreeRTOS   tick rate配置 systick */
    RCC_GetClocksFreq(&RCC_Clocks);
    SysTick_Config(RCC_Clocks.HCLK_Frequency / configTICK_RATE_HZ);

    /*读硬件配置信息*/
    bsp_get_device_config_table();

    /*初始化SDRAM*/
    if(gtv_DeviceConfigTable.isSupportSdram) {
        bsp_sdram_init();
    }

    /*FreeRTOS 内存管理内存堆初始化,在此之前不可以调用OS分配内存函数*/
    bsp_dynamic_mem_region_init();
}
#else
void bsp_init_pre(void)
{
    //flexRAM_init();
    BOARD_ConfigMPU();

    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    PRINTF("\r\nKalyke project started! SW version: %s, mcv_DeviceId = 0x%04X\r\n", SW_VERSION, bsp_get_deviceID());
    show_reset_log();
    //flexRAM_init();
    //bsp_flash_init();
    //OCRAM_Reallocate();
    Kalyke_PrintRunFrequency(0);
    {
        uint8_t *p_m_interrupts_ram_start = (uint8_t *)0x20000000;
        uint32_t m_interrupts_ram_size = 0x400;
        hexdump(p_m_interrupts_ram_start, m_interrupts_ram_size + 16);
    }
}
#endif

/**
  * @brief  BSP初始化后部分，按照主板配置设定各设备状态
  * @param  None
  * @retval None
  */
#if 0  
void bsp_init_post(void)
{
    /*输入输出端口初始化*/
    bsp_gpio_init();

    /*LED 初始化*/
    bsp_led_init();

    /*周期振荡时钟源配置*/
    bsp_init_cycle_clock_tim();

    /*USB 初始化*/
    bsp_usb_init();

    /*使能全局中断*/
    __set_PRIMASK(0);

    portENABLE_INTERRUPTS();

    /*看门狗初始化*/
    bsp_iwdg_init();
}
#else
void bsp_init_post(void)
{
    /*读硬件配置信息*/
    //bsp_get_device_config_table();
    
    /*输入输出端口初始化*/
    //bsp_gpio_init();

    /*LED 初始化*/
    bsp_led_init();

    /*周期振荡时钟源配置*/
    bsp_init_cycle_clock_tim();

    /*USB 初始化*/
    //bsp_usb_init();

    /*使能全局中断*/
    //__set_PRIMASK(0);
    //EnableGlobalIRQ(0);

    //portENABLE_INTERRUPTS();

    /*看门狗初始化*/
    //bsp_iwdg_init();

    //init_rand();

    init_RTC();
}

#endif

/**
  * @brief  系统重启
  * @param  None
  * @retval None
  */
#if 0
void bsp_reboot_system(void)
{
    pVoidFun  pFun;
    if(((*((vu32 *)(BSP_TARGET_REBOOT_ADDR))) & 0x2FFE0000) == 0x20000000) {

        SysTick->CTRL &= 0xFFFFFFFE;
        /*禁止全局中断*/
        __set_PRIMASK(1);

        __set_MSP(*(vu32 *)BSP_TARGET_REBOOT_ADDR);
        pFun = (pVoidFun)(*(vu32 *)(BSP_TARGET_REBOOT_ADDR + 4));
        pFun();
    }
}
#else
void bsp_reboot_system(void)
{
    //WDOG_TriggerSystemSoftwareReset(KALYKE_WDOG_BASE);
    //NVIC_SystemReset();
    uint32_t idx = ota_get_image_idx();
    ota_reset(idx);
}
#endif

void bsp_save_boot_time(void)
{
    gKalykeBootTime = bsp_get_BootTime();
    if (gKalykeBootTime == 0xFFFFFFFF) //系统第一次运行，初始化为1
    {
        gKalykeBootTime = 1;
    }
    else
    {
        gKalykeBootTime++;        
    }
    bsp_save_BootTime(gKalykeBootTime);
    uint32_t *pSD = (uint32_t *)&gtv_PlcElement.msp_SDElement[SD213];
    *pSD = gKalykeBootTime;
}

uint32_t bsp_get_kalyke_boot_time(void)
{
    return gKalykeBootTime;
}

