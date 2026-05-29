/**
  ******************************************************************************
  * @file    kalyke_ad_task.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-06-13
  * @brief   AD task
  ******************************************************************************
  */
#include <stdio.h>

#include "main.h"
#include "kalyke_ad_task.h"
#include "fsl_debug_console.h"
#include "fsl_lpspi_freertos.h"
#include "kalyke_tool.h"

#include "plc_sysblock.h"
#include "plc_element.h"
#include "bsp_dct.h"


//#define DEBUG_AD

#ifdef DEBUG_AD
#define LOGE_AD    LOGE
#define LOGW_AD    LOGW
#define LOGI_AD    LOGI
#define LOGD_AD    LOGD
#define LOGV_AD    LOGV
#else
#define LOGE_AD(...)
#define LOGW_AD(...)
#define LOGI_AD(...)
#define LOGD_AD(...)
#define LOGV_AD(...)
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
typedef int     AD_Type;
//typedef AD_Type uint16_t;

typedef struct _nodeAverage
{
    uint16_t point;
    uint16_t data;
} nodeAverage_t;

typedef struct _headTail
{
    uint16_t head;
    uint16_t constHead;
    uint8_t sampleTimes;
    bool canFast;
    uint32_t all;
    
} headTail_t;

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define ADC_FULL_SCALE_RANGE     4096

#define ADS8668_CMD_NO_OP     0x00
#define ADS8668_CMD_STDBY     0x82
#define ADS8668_CMD_PWR_DN    0x83
#define ADS8668_CMD_RST       0x85
#define ADS8668_CMD_AUTO_RST  0xA0
#define ADS8668_CMD_MAN_CH0   0xC0
#define ADS8668_CMD_MAN_CH1   0xC4
#define ADS8668_CMD_MAN_CH2   0xC8
#define ADS8668_CMD_MAN_CH3   0xCC
#define ADS8668_CMD_MAN_CH4   0xD0
#define ADS8668_CMD_MAN_CH5   0xD4
#define ADS8668_CMD_MAN_CH6   0xD8
#define ADS8668_CMD_MAN_CH7   0xDC
#define ADS8668_CMD_MAN_AUX   0xE0

#define ADS8668_PRG_AUTO_SEQ_EN  0x01
#define ADS8668_PRG_CH_PWR_DN    0x02
#define ADS8668_PRG_FEATURE_SEL  0x03
#define ADS8668_PRG_RANGE_CH0    0x05

#define AUTO_SEQ_DN_CH0_MASK    0x01
#define AUTO_SEQ_DN_CH1_MASK    0x02
#define AUTO_SEQ_DN_CH2_MASK    0x04
#define AUTO_SEQ_DN_CH3_MASK    0x08
#define AUTO_SEQ_DN_CH4_MASK    0x10
#define AUTO_SEQ_DN_CH5_MASK    0x20
#define AUTO_SEQ_DN_CH6_MASK    0x40
#define AUTO_SEQ_DN_CH7_MASK    0x80

// For Kalyke project, Vref is 4096mV
#define INPUT_RANGE_POSITIVE_NEGATIVE_2_POINT_5_MULTIPLY_VREF     0x00 //±10.24V
#define INPUT_RANGE_POSITIVE_NEGATIVE_1_POINT_25_MULTIPLY_VREF    0x01 //±5.12V
#define INPUT_RANGE_POSITIVE_NEGATIVE_0_POINT_625_MULTIPLY_VREF   0x02 //±2.56V
#define INPUT_RANGE_POSITIVE_NEGATIVE_0_POINT_3125_MULTIPLY_VREF  0x03 //±1.28V
#define INPUT_RANGE_POSITIVE_NEGATIVE_0_POINT_15625_MULTIPLY_VREF 0x0B //±0.64V
#define INPUT_RANGE_POSITIVE_2_POINT_5_MULTIPLY_VREF              0x05 //0 ~ 10.24V
#define INPUT_RANGE_POSITIVE_1_POINT_25_MULTIPLY_VREF             0x06 //0 ~ 5.12V
#define INPUT_RANGE_POSITIVE_0_POINT_625_MULTIPLY_VREF            0x07 //0 ~ 2.56V
#define INPUT_RANGE_POSITIVE_0_POINT_3125_MULTIPLY_VREF           0x0F //0 ~ 1.28V

// The following program register no use for Kalyke project
#if 0
/* ALARM FLAG REGISTERS (Read-Only) */
#define ADS8668_PRG_TRIP_ALL     0x10 //ALARM Overview Tripped-Flag
#define ADS8668_PRG_TRIP0        0x11 //ALARM Ch 0-3 Tripped-Flag
#define ADS8668_PRG_ACTIVE0      0x12 //ALARM Ch 0-3 Active-Flag
#define ADS8668_PRG_TRIP1        0x13 //ALARM Ch 4-7 Tripped-Flag
#define ADS8668_PRG_ACTIVE1      0x14 //ALARM Ch 4-7 Active-Flag

/* ALARM THRESHOLD REGISTERS */
#define ADS8668_PRG_HYSTERESIS_CH0         0x15 //Ch 0 Hysteresis
#define ADS8668_PRG_HIGH_THRESHOLD_MSB_CH0 0x16
#define ADS8668_PRG_HIGH_THRESHOLD_LSB_CH0 0x17
#define ADS8668_PRG_LOW_THRESHOLD_MSB_CH0  0x18
#define ADS8668_PRG_LOW_THRESHOLD_LSB_CH0  0x19

#define ADS8668_PRG_HYSTERESIS_CH1         0x1A //Ch 1 Hysteresis
#define ADS8668_PRG_HIGH_THRESHOLD_MSB_CH1 0x1B
#define ADS8668_PRG_HIGH_THRESHOLD_LSB_CH1 0x1C
#define ADS8668_PRG_LOW_THRESHOLD_MSB_CH1  0x1D
#define ADS8668_PRG_LOW_THRESHOLD_LSB_CH1  0x1E

#define ADS8668_PRG_HYSTERESIS_CH2         0x1F //Ch 2 Hysteresis
#define ADS8668_PRG_HIGH_THRESHOLD_MSB_CH2 0x20
#define ADS8668_PRG_HIGH_THRESHOLD_LSB_CH2 0x21
#define ADS8668_PRG_LOW_THRESHOLD_MSB_CH2  0x22
#define ADS8668_PRG_LOW_THRESHOLD_LSB_CH2  0x23

#define ADS8668_PRG_HYSTERESIS_CH3         0x24 //Ch 3 Hysteresis
#define ADS8668_PRG_HIGH_THRESHOLD_MSB_CH3 0x25
#define ADS8668_PRG_HIGH_THRESHOLD_LSB_CH3 0x26
#define ADS8668_PRG_LOW_THRESHOLD_MSB_CH3  0x27
#define ADS8668_PRG_LOW_THRESHOLD_LSB_CH3  0x28

#define ADS8668_PRG_HYSTERESIS_CH4         0x29 //Ch 4 Hysteresis
#define ADS8668_PRG_HIGH_THRESHOLD_MSB_CH4 0x2A
#define ADS8668_PRG_HIGH_THRESHOLD_LSB_CH4 0x2B
#define ADS8668_PRG_LOW_THRESHOLD_MSB_CH4  0x2C
#define ADS8668_PRG_LOW_THRESHOLD_LSB_CH4  0x2D

#define ADS8668_PRG_HYSTERESIS_CH5         0x2E //Ch 5 Hysteresis
#define ADS8668_PRG_HIGH_THRESHOLD_MSB_CH5 0x2F
#define ADS8668_PRG_HIGH_THRESHOLD_LSB_CH5 0x30
#define ADS8668_PRG_LOW_THRESHOLD_MSB_CH5  0x31
#define ADS8668_PRG_LOW_THRESHOLD_LSB_CH5  0x32

#define ADS8668_PRG_HYSTERESIS_CH6         0x33 //Ch 6 Hysteresis
#define ADS8668_PRG_HIGH_THRESHOLD_MSB_CH6 0x34
#define ADS8668_PRG_HIGH_THRESHOLD_LSB_CH6 0x35
#define ADS8668_PRG_LOW_THRESHOLD_MSB_CH6  0x36
#define ADS8668_PRG_LOW_THRESHOLD_LSB_CH6  0x37

#define ADS8668_PRG_HYSTERESIS_CH7         0x38 //Ch 7 Hysteresis
#define ADS8668_PRG_HIGH_THRESHOLD_MSB_CH7 0x39
#define ADS8668_PRG_HIGH_THRESHOLD_LSB_CH7 0x3A
#define ADS8668_PRG_LOW_THRESHOLD_MSB_CH7  0x3B
#define ADS8668_PRG_LOW_THRESHOLD_LSB_CH7  0x3C
#endif

#define KALYKE_SPI_BASE    LPSPI3
/* Select USB1 PLL PFD0 (720 MHz) as lpspi clock source */
#define KALYKE_LPSPI_CLOCK_SOURCE_SELECT (1U)
/* Clock divider for master lpspi clock source */
#define KALYKE_LPSPI_CLOCK_SOURCE_DIVIDER (7U)

#define KALYKE_LPSPI_MASTER_CLK_FREQ (CLOCK_GetFreq(kCLOCK_Usb1PllPfd0Clk) / (KALYKE_LPSPI_CLOCK_SOURCE_DIVIDER + 1U))
#define KALYKE_LPSPI_TRANSFER_BAUDRATE 16500000U /*! Transfer baudrate - 500k */
#define KALYKE_LPSPI_MASTER_PCS_FOR_INIT (kLPSPI_Pcs0)

#if (KALYKE_FEATURE_AD_TASK == 1)
/*******************************************************************************
 * Variables
 ******************************************************************************/
static const char *TAG = "AD";
TaskHandle_t gKalykeADTaskHandle = NULL;
static bool gEverStartAD = false;
static lpspi_rtos_handle_t gSpiRtosHandle;

static uint8_t g_tx_buffer[16];
static uint8_t g_rx_buffer[16];

static lpspi_transfer_t gSpiTf;

//static int gTail = 0;
static headTail_t gHeadTail[ADS8668_CHANNEL_NUMBERS];
static nodeAverage_t *gpAverage[ADS8668_CHANNEL_NUMBERS]; // 用于计算平均值
static uint8_t gSampleCounts = 8;

//static uint8_t gHead[ADS8668_CHANNEL_NUMBERS] = {0};
//static 
/*******************************************************************************
 * Code
 ******************************************************************************/
inline AD_Type map(AD_Type x, AD_Type in_min, AD_Type in_max, AD_Type out_min, AD_Type out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static inline AD_Type mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static AD_Type getVoltageOrCurrentMappedValue(uint8_t ch, uint16_t adVal)
{
    LOGV_AD(TAG, "Enter %s(), ch = %u, adVal = %u, zeroDta = %d, maxDta = %d\r\n", __func__, ch, adVal, gt_InAdDaCfg.ad[ch].zeroDta, gt_InAdDaCfg.ad[ch].maxDta);
    AD_Type tempMap = 0; 
    AD_Type voltage = 0;
    switch (gt_InAdDaCfg.ad[ch].modSlc)
        {
        case sysAdCfgZ10v:  //0 ~ 10.24V
            tempMap = mapFloat(adVal, 0, ADC_FULL_SCALE_RANGE, 0, 10240);
            voltage = mapFloat(tempMap, 0, 10000, gt_InAdDaCfg.ad[ch].zeroDta, gt_InAdDaCfg.ad[ch].maxDta);
            break;
        case sysAdCfgZ05v:  //0 ~ 5.12V
            tempMap = mapFloat(adVal, 0, ADC_FULL_SCALE_RANGE, 0, 5120);
            voltage = mapFloat(tempMap, 0, 5000, gt_InAdDaCfg.ad[ch].zeroDta, gt_InAdDaCfg.ad[ch].maxDta);
            break;
        case sysAdCfgZf10v: //±10.24V
            tempMap = mapFloat(adVal, 0, ADC_FULL_SCALE_RANGE, -10240, 10240);
            voltage = mapFloat(tempMap, -10000, 10000, gt_InAdDaCfg.ad[ch].zeroDta, gt_InAdDaCfg.ad[ch].maxDta);
            break;
        case sysAdCfgZf05v: //±5.12V
            tempMap = mapFloat(adVal, 0, ADC_FULL_SCALE_RANGE, -5120, 5120);
            voltage = mapFloat(tempMap, -5000, 5000, gt_InAdDaCfg.ad[ch].zeroDta, gt_InAdDaCfg.ad[ch].maxDta);
            break;
        case sysAdCfgZf025v://±2.56V
            tempMap = mapFloat(adVal, 0, ADC_FULL_SCALE_RANGE, -2560, 2560);
            voltage = mapFloat(tempMap, -2500, 2500, gt_InAdDaCfg.ad[ch].zeroDta, gt_InAdDaCfg.ad[ch].maxDta);
            break;
        
        case sysAdCfgZ04_Z20ma://1 ~ 5.12V
        case sysAdCfgZ00_Z20ma://0 ~ 5.12V
        {
            voltage = mapFloat(adVal, 0, ADC_FULL_SCALE_RANGE, 0, 5120);// mV
            //Ohm's law: I = U/R
            float current = voltage / 250.0; //mA, This is real current value.
            if (gt_InAdDaCfg.ad[ch].modSlc == sysAdCfgZ00_Z20ma)
            {
                voltage = mapFloat(current, 0, 20, gt_InAdDaCfg.ad[ch].zeroDta, gt_InAdDaCfg.ad[ch].maxDta);
            }
            else
            {
                voltage = mapFloat(current, 4, 20, gt_InAdDaCfg.ad[ch].zeroDta, gt_InAdDaCfg.ad[ch].maxDta);
            }
            break;
        }
        case sysAdCfgF20_Z20ma://±5.12V
        {
            voltage = mapFloat(adVal, 0, ADC_FULL_SCALE_RANGE, -5120, 5120);// mV
            //Ohm's law: I = U/R
            float current = voltage / 250.0; //mA, This is real current value.
            voltage = mapFloat(current, -20, 20, gt_InAdDaCfg.ad[ch].zeroDta, gt_InAdDaCfg.ad[ch].maxDta);
            break;
        }
        default:
            voltage = -2019;
            break;
        }
    if (voltage > gt_InAdDaCfg.ad[ch].maxDta)
    {
        voltage = gt_InAdDaCfg.ad[ch].maxDta;
    }
    LOGV_AD(TAG, "Leave %s(), voltage = %u\r\n", __func__, voltage);
    return voltage;
}

// Just for test
static inline float getVoltageOrCurrentRealValue(uint8_t ch, uint16_t adVal)
{
    float voltage = 0.0;
    switch (gt_InAdDaCfg.ad[ch].modSlc)
        {
        case sysAdCfgZf10v: //±10.24V
            voltage = (adVal - 2048.0) * 0.005;
            break;
        case sysAdCfgZf05v: //±5.12V
            voltage = (adVal - 2048.0) * 0.0025;
            break;
        case sysAdCfgZf025v://±2.56V
            voltage = (adVal - 2048.0) * 0.00125;
            break;
        case sysAdCfgZ10v: //0 ~ 10.24V
            voltage = adVal * 0.0025;
            break;
        case sysAdCfgZ05v: //0 ~ 5.12V
            voltage = adVal * 0.00125;
            break;
        case sysAdCfgZ04_Z20ma://1 ~ 5.12V
            voltage = adVal * 0.00125;
            voltage *= 1000; //mV
            //Ohm's law: I = U/R
            voltage = voltage / 250; //mA
            break;
        case sysAdCfgZ00_Z20ma://0 ~ 5.12V
            voltage = adVal * 0.00125;
            voltage *= 1000; //mV
            //Ohm's law: I = U/R
            voltage = voltage / 250; //mA
            break;
        case sysAdCfgF20_Z20ma://±5.12V
            voltage = (adVal - 2048.0) * 0.0025;
            voltage *= 1000; //mV
            //Ohm's law: I = U/R
            voltage = voltage / 250; //mA
            break;
        default:
            break;
        }
    return voltage;
}

static inline uint8_t getADS8668Mode(uint16_t mod)
{
    switch (mod)
        {
        case sysAdCfgZf10v:
            return INPUT_RANGE_POSITIVE_NEGATIVE_2_POINT_5_MULTIPLY_VREF;
        case sysAdCfgZf05v:
            return INPUT_RANGE_POSITIVE_NEGATIVE_1_POINT_25_MULTIPLY_VREF;
        case sysAdCfgZf025v:
            return INPUT_RANGE_POSITIVE_NEGATIVE_0_POINT_625_MULTIPLY_VREF;
        case sysAdCfgZ10v:
            return INPUT_RANGE_POSITIVE_2_POINT_5_MULTIPLY_VREF;
        case sysAdCfgZ05v:
            return INPUT_RANGE_POSITIVE_1_POINT_25_MULTIPLY_VREF;
        case sysAdCfgZ04_Z20ma:
            return INPUT_RANGE_POSITIVE_1_POINT_25_MULTIPLY_VREF;
        case sysAdCfgZ00_Z20ma:
            return INPUT_RANGE_POSITIVE_1_POINT_25_MULTIPLY_VREF;
        case sysAdCfgF20_Z20ma:
            return INPUT_RANGE_POSITIVE_NEGATIVE_1_POINT_25_MULTIPLY_VREF;
        default:
            break;
        }
    return INPUT_RANGE_POSITIVE_NEGATIVE_2_POINT_5_MULTIPLY_VREF;
}

#if 0
static inline uint16_t addAverageValueAndCalculate(uint8_t ch, uint16_t ad)
{
    uint8_t cur = gHead[ch];
    
    gpAverage[ch][gHead[ch]].data = ad;
    gHead[ch] = gpAverage[ch][gHead[ch]].point;

    uint8_t sampleCounts = 0;
    uint32_t aveVal = 0;
    
    do
    {
        aveVal += gpAverage[ch][cur].data;
        cur = gpAverage[ch][cur].point;
        sampleCounts++;
    } while (gpAverage[ch][cur].data != 0xFFFF);

    aveVal = aveVal / sampleCounts;
    return aveVal;
}
#endif
static inline uint16_t addAverageValueAndCalculate(uint8_t ch, uint16_t ad)
{
    LOGI_AD(TAG, "Enter %s()", __func__);
    uint16_t sampleCounts = gSampleCounts;
    uint32_t aveVal = 0;
    if (gHeadTail[ch].canFast)
    {
        gHeadTail[ch].all -= gpAverage[ch][gHeadTail[ch].head].data;//减掉最老的
        gHeadTail[ch].all += ad; //加上最新的
        aveVal = gHeadTail[ch].all / sampleCounts;

        gpAverage[ch][gHeadTail[ch].head].data = ad;
        gHeadTail[ch].head = gpAverage[ch][gHeadTail[ch].head].point;
        LOGI_AD(TAG, "all = %u, aveVal = %u", gHeadTail[ch].all, aveVal);
        return aveVal;
    }

    gHeadTail[ch].sampleTimes = 0;
    gpAverage[ch][gHeadTail[ch].head].data = ad;
    gHeadTail[ch].head = gpAverage[ch][gHeadTail[ch].head].point;
    uint8_t cur = gHeadTail[ch].constHead;
    do 
    {
        aveVal += gpAverage[ch][cur].data;
        cur = gpAverage[ch][cur].point;
        gHeadTail[ch].sampleTimes++;
    } while (cur != gHeadTail[ch].head);

    
    if (gHeadTail[ch].sampleTimes >= sampleCounts)
    {
        gHeadTail[ch].canFast = true;
        gHeadTail[ch].all = aveVal;
        LOGW_AD(TAG, "sampleTimes = %u, all = %u, head = %u", gHeadTail[ch].sampleTimes, aveVal, gHeadTail[ch].head);
    }
    aveVal = aveVal / gHeadTail[ch].sampleTimes;

    return aveVal;
}

static inline void kalyke_AD_convert(void)
{
    LOGD_AD(TAG, "Enter %s()", __func__);
    
    status_t ret = LPSPI_RTOS_Transfer(&gSpiRtosHandle, &gSpiTf);
    LOGD_AD(TAG, "LPSPI_RTOS_Transfer return : %d, cmd=%X\r\n", ret, g_tx_buffer[0] >> 1);
    //hexdump(g_rx_buffer, sizeof(g_rx_buffer));

    uint16_t ad = (g_rx_buffer[2] << 4) | (g_rx_buffer[3] >> 4);
    
    uint8_t chAddress = g_rx_buffer[4] >> 4;
#if SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_SDK
    uint8_t inputRange = ((g_rx_buffer[4] & 0x03) << 1) | (g_rx_buffer[5] >> 7);
    LOGD_AD(TAG, "ad = %u(0x%X), chAddress = %u, inputRange = %u\r\n", ad, ad, chAddress, inputRange);
#endif
    //LOGD(TAG, "ad = %u(0x%X), chAddress = %u, inputRange = %u\r\n", ad, ad, chAddress, inputRange);
    if (chAddress >= gt_InAdDaCfg.adNum) return;

    SET_SD_ELEMENT_VALUE(SD240 + chAddress, ad);
    if (gt_InAdDaCfg.ad[chAddress].pCurU16 != 0x7FFF)
    {
    #if SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_SDK
        float val = getVoltageOrCurrentRealValue(chAddress, ad);
        LOGW_AD(TAG, "Real voltage = %f", val);
    #endif
        AD_Type mapVal = getVoltageOrCurrentMappedValue(chAddress, ad);
        LOGW_AD(TAG, "Mapped voltage = %u", mapVal);
        //LOGW(TAG, "Mapped voltage = %u(0x%08X)", mapVal, mapVal);
        if (gt_InAdDaCfg.ad[chAddress].pCurU16 & 0x8000) // R element
        {
            #if 1
            SET_R_ELEMENT_VALUE((gt_InAdDaCfg.ad[chAddress].pCurU16 & 0x7FFF), mapVal);
            #else
            AD_Type *pFloatVal = (AD_Type*)&gtv_PlcElement.msp_RElement[(gt_InAdDaCfg.ad[chAddress].pCurU16 & 0x7FFF)];
            *pFloatVal = mapVal;
            #endif
        }
        else // D element
        {
            #if 1
            SET_D_ELEMENT_VALUE(gt_InAdDaCfg.ad[chAddress].pCurU16, mapVal);
            #else
            AD_Type *pFloatVal = (AD_Type*)&gtv_PlcElement.msp_DElement[gt_InAdDaCfg.ad[chAddress].pCurU16];
            *pFloatVal = mapVal;
            #endif
            //LOGW(TAG, "D%u = %u", gt_InAdDaCfg.ad[chAddress].pCurU16, GET_D_ELEMENT_VALUE(gt_InAdDaCfg.ad[chAddress].pCurU16));
        }
    }
    if (gt_InAdDaCfg.ad[chAddress].pAverageU16 != 0x7FFF)
    {
        uint16_t adAveValue = addAverageValueAndCalculate(chAddress, ad);
        LOGW_AD(TAG, "adAveValue = %u", adAveValue);

    #if SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_SDK
        float aveVal = getVoltageOrCurrentRealValue(chAddress, adAveValue);
        LOGW_AD(TAG, "Real average voltage = %f", aveVal);
    #endif
        AD_Type mapAveVal = getVoltageOrCurrentMappedValue(chAddress, adAveValue);
        LOGE_AD(TAG, "Mapped average = %u", mapAveVal);
        if (gt_InAdDaCfg.ad[chAddress].pAverageU16 & 0x8000) // R element
        {
            #if 1
            SET_R_ELEMENT_VALUE((gt_InAdDaCfg.ad[chAddress].pAverageU16 & 0x7FFF), mapAveVal);
            #else
            AD_Type *pFloatVal = (AD_Type*)&gtv_PlcElement.msp_RElement[(gt_InAdDaCfg.ad[chAddress].pAverageU16 & 0x7FFF)];
            *pFloatVal = mapAveVal;
            #endif
        }
        else // D element
        {
            #if 1
            SET_D_ELEMENT_VALUE(gt_InAdDaCfg.ad[chAddress].pAverageU16, mapAveVal);
            #else
            AD_Type *pFloatVal = (AD_Type*)&gtv_PlcElement.msp_DElement[gt_InAdDaCfg.ad[chAddress].pAverageU16];
            *pFloatVal = mapAveVal;
            #endif
        }
    }
}

static void kalyke_power_down_ADS8668(void)
{
    LOGV(TAG, "Enter %s()", __func__);
    lpspi_transfer_t spi_transfer;
    spi_transfer.txData = g_tx_buffer;
    spi_transfer.rxData = g_rx_buffer;
    spi_transfer.dataSize = 4;
    spi_transfer.configFlags = kLPSPI_MasterPcs0 | kLPSPI_MasterPcsContinuous | kLPSPI_MasterByteSwap;

    g_tx_buffer[0] = ADS8668_CMD_PWR_DN;
    g_tx_buffer[1] = 0x00;
    status_t ret = LPSPI_RTOS_Transfer(&gSpiRtosHandle, &spi_transfer);
    LOGV(TAG, "LPSPI_RTOS_Transfer return : %d, cmd=%X\r\n", ret, g_tx_buffer[0]);
    hexdump(g_rx_buffer, sizeof(g_rx_buffer));

    ret = LPSPI_RTOS_Deinit(&gSpiRtosHandle);
    LOGD(TAG, "LPSPI_RTOS_Deinit return : %d", ret);
}

static int8_t kalyke_config_ADS8668(void)
{
    LOGV(TAG, "Enter %s()\r\n", __func__);

    lpspi_transfer_t spi_transfer;
    spi_transfer.txData = g_tx_buffer;
    spi_transfer.rxData = g_rx_buffer;
    spi_transfer.dataSize = 4;
    spi_transfer.configFlags = kLPSPI_MasterPcs0 | kLPSPI_MasterPcsContinuous | kLPSPI_MasterByteSwap;

    memset(g_rx_buffer, 0, sizeof(g_rx_buffer));
    memset(g_tx_buffer, 0, sizeof(g_tx_buffer));
    g_tx_buffer[0] = (ADS8668_PRG_AUTO_SEQ_EN << 1) | 1;
    //LOGV_AD(TAG, "g_tx_buffer[0] = 0x%X\r\n", g_tx_buffer[0]);
    for (uint8_t i = 0; i < gt_InAdDaCfg.adNum; i++)
    {
        if (gt_InAdDaCfg.ad[i].modSlc != sysAdCfgNonUse)
        {
            g_tx_buffer[1] |= 1 << i;
        }
    }    
    //g_tx_buffer[1] = AUTO_SEQ_DN_CH0_MASK | AUTO_SEQ_DN_CH1_MASK;
    status_t ret = LPSPI_RTOS_Transfer(&gSpiRtosHandle, &spi_transfer);
    LOGV(TAG, "LPSPI_RTOS_Transfer return : %d, cmd=%X\r\n", ret, g_tx_buffer[0] >> 1);
    hexdump(g_rx_buffer, sizeof(g_rx_buffer));
    // 说明AD板子未和Kalyke连接
    if (g_rx_buffer[0] == 0xFF && g_rx_buffer[1] == 0xFF && g_rx_buffer[2] == 0xFF && g_rx_buffer[3] == 0xFF)
    {
        return -1;
    }

    g_tx_buffer[0] = (ADS8668_PRG_CH_PWR_DN << 1) | 1;
    g_tx_buffer[1] = ~g_tx_buffer[1];
    memset(g_rx_buffer, 0, sizeof(g_rx_buffer));
    ret = LPSPI_RTOS_Transfer(&gSpiRtosHandle, &spi_transfer);
    LOGV(TAG, "LPSPI_RTOS_Transfer return : %d, cmd=%X\r\n", ret, g_tx_buffer[0] >> 1);
    hexdump(g_rx_buffer, sizeof(g_rx_buffer));

    g_tx_buffer[0] = (ADS8668_PRG_FEATURE_SEL << 1) | 1;
    g_tx_buffer[1] = 0x03;
    memset(g_rx_buffer, 0, sizeof(g_rx_buffer));
    ret = LPSPI_RTOS_Transfer(&gSpiRtosHandle, &spi_transfer);
    LOGV(TAG, "LPSPI_RTOS_Transfer return : %d, cmd=%X\r\n", ret, g_tx_buffer[0] >> 1);
    hexdump(g_rx_buffer, sizeof(g_rx_buffer));

    for (uint8_t i = 0; i < gt_InAdDaCfg.adNum; i++)
    {
        if (gt_InAdDaCfg.ad[i].modSlc == sysAdCfgNonUse)
        {
            continue;
        }
        g_tx_buffer[0] = ((ADS8668_PRG_RANGE_CH0 + i) << 1) | 1;
        //g_tx_buffer[1] = INPUT_RANGE_POSITIVE_NEGATIVE_1_POINT_25_MULTIPLY_VREF;
        g_tx_buffer[1] = getADS8668Mode(gt_InAdDaCfg.ad[i].modSlc);
        ret = LPSPI_RTOS_Transfer(&gSpiRtosHandle, &spi_transfer);
        LOGV(TAG, "LPSPI_RTOS_Transfer return : %d, cmd=%X\r\n", ret, g_tx_buffer[0] >> 1);
        hexdump(g_rx_buffer, sizeof(g_rx_buffer));
    }

    memset(g_rx_buffer, 0, sizeof(g_rx_buffer));
    g_tx_buffer[0] = ADS8668_CMD_AUTO_RST;
    g_tx_buffer[1] = 0x00;
    spi_transfer.dataSize = 6;
    ret = LPSPI_RTOS_Transfer(&gSpiRtosHandle, &spi_transfer);
    LOGV(TAG, "LPSPI_RTOS_Transfer return : %d, cmd=%X\r\n", ret, g_tx_buffer[0]);
    hexdump(g_rx_buffer, sizeof(g_rx_buffer));

    return 0;
}

static int8_t kalyke_rtos_init_ADS8668(void)
{
    LOGV(TAG, "Enter %s()", __func__);
    NVIC_SetPriority(LPSPI3_IRQn, 6);

    /*Set clock source for LPSPI*/
    CLOCK_SetMux(kCLOCK_LpspiMux, KALYKE_LPSPI_CLOCK_SOURCE_SELECT);
    CLOCK_SetDiv(kCLOCK_LpspiDiv, KALYKE_LPSPI_CLOCK_SOURCE_DIVIDER);

    uint32_t srcClock_Hz = KALYKE_LPSPI_MASTER_CLK_FREQ;
    LOGV(TAG, "srcClock_Hz = %u", srcClock_Hz);

    lpspi_master_config_t masterConfig;
    /*Master config*/
    masterConfig.baudRate = KALYKE_LPSPI_TRANSFER_BAUDRATE;
    masterConfig.bitsPerFrame = 8;
    masterConfig.cpol = kLPSPI_ClockPolarityActiveHigh;
    masterConfig.cpha = kLPSPI_ClockPhaseSecondEdge;
    masterConfig.direction = kLPSPI_MsbFirst;

    masterConfig.pcsToSckDelayInNanoSec = 1000000000 / masterConfig.baudRate;
    masterConfig.lastSckToPcsDelayInNanoSec = 1000000000 / masterConfig.baudRate;
    masterConfig.betweenTransferDelayInNanoSec = 1000000000 / masterConfig.baudRate;

    masterConfig.whichPcs = KALYKE_LPSPI_MASTER_PCS_FOR_INIT;
    masterConfig.pcsActiveHighOrLow = kLPSPI_PcsActiveLow;

    masterConfig.pinCfg = kLPSPI_SdoInSdiOut;
    masterConfig.dataOutConfig = kLpspiDataOutTristate;

    status_t ret = LPSPI_RTOS_Init(&gSpiRtosHandle, KALYKE_SPI_BASE, &masterConfig, srcClock_Hz);
    LOGD(TAG, "LPSPI_RTOS_Init return : %d", ret);

    return kalyke_config_ADS8668();
}

// Exit because there is no AD module connected to Kalyke.
static void kalyke_exit_AD(void)
{
    LOGV(TAG, "Enter %s()", __func__);
    for (uint8_t i = 0; i < ADS8668_CHANNEL_NUMBERS; i++)
    {
        vPortFree(gpAverage[i]);
    }
    status_t ret = LPSPI_RTOS_Deinit(&gSpiRtosHandle);
    LOGD(TAG, "LPSPI_RTOS_Deinit return : %d", ret);
    vTaskDelete(gKalykeADTaskHandle);
    LOGV(TAG, "Leave %s()", __func__);
}

void kalyke_start_AD(void)
{
    LOGV(TAG, "Enter %s()", __func__);
    if (bspIsHaveAD())
    {
        goto HAVE_AD;
    }
    else
    {
        LOGW(TAG, "Just return because ONLY FX_08R06AI08_C1 and FX_08R06AI08_C2 have AD feature.");
        return;
    }
HAVE_AD:
    if (gt_InAdDaCfg.AdFsm == AD_FsmNonUse)
    {
        LOGW(TAG, "Just return because there is no AD module connected to Kalyke");
        return;
    }
    uint16_t sampleCounts = gSampleCounts;
    for (uint8_t ch = 0; ch < ADS8668_CHANNEL_NUMBERS; ch++)
    {
        LOGV(TAG, "%u: sampleCounts = %u", ch, sampleCounts);
        gpAverage[ch] = pvPortMalloc(sizeof(nodeAverage_t) * sampleCounts);
        // 链表初始化
        uint8_t j;
        for (j = 0; j < sampleCounts - 1; j++)
        {
            gpAverage[ch][j].point = j + 1;
            gpAverage[ch][j].data = 0xFFFF;
        }
        gpAverage[ch][j].point = 0;
        gpAverage[ch][j].data = 0;

        gHeadTail[ch].head = 0;
        gHeadTail[ch].constHead = 0;
        gHeadTail[ch].canFast = false;
        gHeadTail[ch].sampleTimes = 0;
        gHeadTail[ch].all = 0;
    }

    LOGD(TAG, "gKalykeADTaskHandle = 0x%08X", gKalykeADTaskHandle);
    if (gKalykeADTaskHandle == NULL)
    { 
#if (KALYKE_FEATURE_AD_TASK == 1)
        BaseType_t ret = xTaskCreate((TaskFunction_t)kalyke_ad_task,
                          (const char *)"kalyke_ad_task",
                          AD_TASK_STACK_SIZE,
                          (void *)NULL,
                          AD_TASK_PRIO,
                          &gKalykeADTaskHandle);
        if (ret != pdPASS)
        {
            LOGV(TAG, "Create kalyke_ad_task error!\r\n");
        }
        //gt_InAdDaCfg.AdFsm = AD_FsmIsInit;
#endif
    }
    else
    {
        vTaskResume(gKalykeADTaskHandle);
    }
    gEverStartAD = true;
}

void kalyke_stop_AD(void)
{
    LOGV(TAG, "Enter %s(), gEverStartAD = %u", __func__, gEverStartAD);
    if (gEverStartAD == false)
    {
        return;
    }
    gEverStartAD = false;
    if (gt_InAdDaCfg.AdFsm == AD_FsmNonUse)
    {
        LOGV(TAG, "Just return because there is no AD module connected to Kalyke");
        gKalykeADTaskHandle = NULL;
        return;
    }
    for (uint8_t i = 0; i < ADS8668_CHANNEL_NUMBERS; i++)
    {
        vPortFree(gpAverage[i]);
    }
    gt_InAdDaCfg.AdFsm = AD_FsmIDLE;
    vTaskSuspend(gKalykeADTaskHandle);
    kalyke_power_down_ADS8668();
}

void kalyke_ad_task(void *p_arg)
{
    LOGW(TAG, "kalyke_ad_task RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());
    vTaskDelay(1000 / portTICK_PERIOD_MS);
#if 0
    uint16_t ad_speed = gt_InAdDaCfg.adSpeed < 1 ? 1 : gt_InAdDaCfg.adSpeed;
    LOGE(TAG, "ad_speed = %u(ms)", ad_speed);
#endif
    gSpiTf.txData = g_tx_buffer;
    gSpiTf.rxData = g_rx_buffer;
    gSpiTf.dataSize = 6;
    gSpiTf.configFlags = kLPSPI_MasterPcs0 | kLPSPI_MasterPcsContinuous | kLPSPI_MasterByteSwap;

    for(;;)
    {
        // AD Control block -----------------------------------------------------------------------
        switch(gt_InAdDaCfg.AdFsm)
        {
        case AD_FsmRun:
            kalyke_AD_convert();
            break;

        case AD_FsmIDLE:
            LOGV(TAG, "*** AD_FsmIDLE ***\r\n");
            break;
            
        case AD_FsmStop:
            LOGV(TAG, "*** AD_FsmStop ***\r\n");
            break;
        
        case AD_FsmIsInit:
            LOGV(TAG, "*** AD_FsmIsInit ***\r\n");
            if (kalyke_rtos_init_ADS8668() == 0)
            {
                gt_InAdDaCfg.AdFsm = AD_FsmStrart;
            }
            else
            {
                gt_InAdDaCfg.AdFsm = AD_FsmNonUse;
            }
            break;

        case AD_FsmStrart:
            LOGV(TAG, "*** AD_FsmStrart ***\r\n");
            memset(g_rx_buffer, 0, sizeof(g_rx_buffer));
            g_tx_buffer[0] = ADS8668_CMD_NO_OP;
            g_tx_buffer[1] = 0x00;
            gt_InAdDaCfg.AdFsm = AD_FsmRun;
            break;
        
        case AD_FsmNonUse:
            LOGV(TAG, "*** AD_FsmNonUse ***\r\n");
            kalyke_exit_AD();
            return;

        default:
            LOGV(TAG, "*** default ***\r\n");
            gt_InAdDaCfg.AdFsm = AD_FsmStop;
            break;
        }

        //vTaskDelay(gt_InAdDaCfg.adSpeed < 1 ? 1 : gt_InAdDaCfg.adSpeed);
        if (gt_InAdDaCfg.adSpeed < 1)
        {
            vTaskDelay(1 / portTICK_PERIOD_MS);
        }
        else
        {
            vTaskDelay(gt_InAdDaCfg.adSpeed / portTICK_PERIOD_MS);
        }
    }
}
#else

#endif
