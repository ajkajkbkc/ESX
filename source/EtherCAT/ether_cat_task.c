/**
  ******************************************************************************
  * @file    ether_cat_task.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2020-07-30
  * @brief
  ******************************************************************************
  */
#include <stdio.h>
#include "lwip/opt.h"

#include "lwip/api.h"
#include "lwip/apps/mqtt.h"
#include "lwip/dhcp.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/netifapi.h"
#include "lwip/prot/dhcp.h"
#include "lwip/tcpip.h"
#include "lwip/timeouts.h"
#include "netif/ethernet.h"
#include "enet_ethernetif.h"

#include "board.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"
//#include "fsl_pit.h"

#include "kalyke_opts.h"

#include "kalyke_event.h"
#include "plc_netcfg.h"

#include "ether_cat_task.h"
#include "fsl_debug_console.h"
#include "ether_cat_list.h"
#include "kalyke_internet_task.h"
#include "kalyke_monitor_task.h"
#include "plc_sysblock.h"
#include "plc_element.h"
#include "plc_errormsg.h"
#include "plc_variable.h"
#include "bsp_led.h"
#include "daisy_task.h"
#if 0
#include "kalyke_version.h"
#include "kalyke_tool.h"
#include "bsp_dct.h"
#include "bsp_gpio.h"
#endif
#include "ethercat.h"
#include "oshw.h"
#include "ethercatconfiglist.h"
#include "bsp_iwdg.h"
#if (ETHERCAT_SOEM == 1)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void kalyke_ethercat_check(void *p_arg);

/*
 *  8-channel digital input terminal 24 V DC, 3 ms
 *  Bit width in the process image: 8
 */
typedef struct
{
    uint8    in1;
    uint8    in2;
    uint8    in3;
    uint8    in4;
    uint8    in5;
    uint8    in6;
    uint8    in7;
    uint8    in8;
} in_EL1008_t;

/*
 *  2-channel relay output terminal 230 V AC, 5 A, potential-free make contacts, no power contacts
 *  Bit width in the process image: 2
 */
typedef struct
{
    uint8    out1;
    uint8    out2;
} out_EL2622_t;

/*
 *  1-channel analog output terminal 0…10 V, 12 bit
 *  Bit width in the process image: 1 x 16 bit AO output
 */
typedef struct
{
    int16    out1;
} out_EL4001_t;

/*
 *  1-channel analog input terminal 0…10 V, single-ended, 12 bit
 *  Bit width in the process image: inputs: 4 byte
 */
typedef struct
{
    int32    in1;
} in_EL3061_t;


/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define PHY_ADDRESS_1 (0x02U) /* Phy address of enet port 0. */
#define PHY_ADDRESS_2 (0x01U)


#define EK1100_1           1
#define EL4001_1           2
#define EL3061_1           3
#define EL1008_1           4
#define EL1008_2           5
#define EL2622_1           6
#define EL2622_2           7
#define EL2622_3           8
#define EL2622_4           9
#define NUMBER_OF_SLAVES   9

#define BIT(x)  (1U << (x))

#define SDO_READ(slaveId, idx, sub, buf, comment)  \
    {   \
        buf=0; \
        int __s = sizeof(buf); \
        int __ret = ec_SDOread(slaveId, idx, sub, FALSE, &__s, &buf, EC_TIMEOUTRXM); \
        LOGD(TAG, "Slave: %d - Read at 0x%04x:%d => wkc: %d; data: 0x%.*x (%d)\t[%s]\n", slaveId, idx, sub, __ret, __s, (unsigned int)buf, (unsigned int)buf, comment); \
    }

#define SDO_WRITE(slaveId, idx, sub, buf, value, comment) \
    {   \
        int __s = sizeof(buf); \
        buf = value; \
        int __ret = ec_SDOwrite(slaveId, idx, sub, FALSE, __s, &buf, EC_TIMEOUTRXM); \
        LOGI(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x\t{%s}\n", slaveId, idx, sub, __ret, __s, (unsigned int)buf, comment); \
    }

/*******************************************************************************
 * Variables
 ******************************************************************************/
static const char *TAG = "EtherCATTask";

TaskHandle_t gEtherCATTaskHandle;
TaskHandle_t gEtherCATCheckTaskHandle;

static TaskHandle_t gRecvDataTaskHandle;

static uint8_t gSOEMRecvBuffer[EC_BUFSIZE];
volatile static uint16_t gSOEMRecvLen = 0;

static list_label_ec_t gRecvBufList;

out_EL4001_t   slave_EL4001_1;
in_EL3061_t    slave_EL3061_1;
in_EL1008_t    slave_EL1008_1;
in_EL1008_t    slave_EL1008_2;
out_EL2622_t   slave_EL2622_1;
out_EL2622_t   slave_EL2622_2;
out_EL2622_t   slave_EL2622_3;

static char IOmap[4096];
int  dorun = 0;

#define SOEM_WKC_THRESHOLD    16
static volatile int gWkcThresholdCounter   = 0;
static volatile int gWkcThresholdCounterSum = 0; 

static volatile int gWkc;
static int gExpectedWKC;
static uint8 currentgroup = 0;


volatile bool gIfPLCWantProcessSDORead = false;
volatile bool gIfPLCWantProcessSDOWrite = false;
bool gSOEMInProcess = false;


static QueueHandle_t gSoemQueueHandle;
soem_msg_st gSOEMMsg;

/*******************************************************************************
 * Code
 ******************************************************************************/

static void logSomeSize(void)
{
    LOGV(TAG, "sizeof(ec_slave) = %u", sizeof(ec_slavet));
    LOGD(TAG, "sizeof(ecx_contextt) = %u", sizeof(ecx_contextt));
    LOGI(TAG, "sizeof(ecx_portt) = %u", sizeof(ecx_portt));
}

#if 0
uint32 network_configuration(void)
{
    /* Do we got expected number of slaves from config */
    if (ec_slavecount < NUMBER_OF_SLAVES)
        return 0;

    /* Verify slave by slave that it is correct*/
    if (strcmp(ec_slave[EK1100_1].name, "EK1100"))
        return 0;
    else if (strcmp(ec_slave[EL4001_1].name, "EL4001"))
        return 0;
    else if (strcmp(ec_slave[EL3061_1].name, "EL3061"))
        return 0;
    else if (strcmp(ec_slave[EL1008_1].name, "EL1008"))
        return 0;
    else if (strcmp(ec_slave[EL1008_2].name, "EL1008"))
        return 0;
    else if (strcmp(ec_slave[EL2622_1].name, "EL2622"))
        return 0;
    else if (strcmp(ec_slave[EL2622_2].name, "EL2622"))
        return 0;
    else if (strcmp(ec_slave[EL2622_3].name, "EL2622"))
        return 0;
    else if (strcmp(ec_slave[EL2622_4].name, "EL2622"))
        return 0;

    return 1;
}
#else
uint32 network_configuration(void)
{
    return 1;
}
#endif

#if 0
int32 get_input_int32(uint16 slave_no, uint8 module_index)
{
    int32 return_value;
    uint8 *data_ptr;
    /* Get the IO map pointer from the ec_slave struct */
    data_ptr = ec_slave[slave_no].inputs;
    /* Move pointer to correct module index */
    data_ptr += module_index * 4;
    /* Read value byte by byte since all targets can't handle misaligned
     * addresses
     */
    return_value = *data_ptr++;
    return_value += (*data_ptr++ << 8);
    return_value += (*data_ptr++ << 16);
    return_value += (*data_ptr++ << 24);

    return return_value;
}

void set_input_int32 (uint16 slave_no, uint8 module_index, int32 value)
{
    uint8 *data_ptr;
    /* Get the IO map pointer from the ec_slave struct */
    data_ptr = ec_slave[slave_no].inputs;
    /* Move pointer to correct module index */
    data_ptr += module_index * 4;
    /* Read value byte by byte since all targets can handle misaligned
     * addresses
     */
    *data_ptr++ = (value >> 0) & 0xFF;
    *data_ptr++ = (value >> 8) & 0xFF;
    *data_ptr++ = (value >> 16) & 0xFF;
    *data_ptr++ = (value >> 24) & 0xFF;
}

uint8 get_input_bit (uint16 slave_no, uint8 module_index)
{
    /* Get the the startbit position in slaves IO byte */
    uint8 startbit = ec_slave[slave_no].Istartbit;
    /* Mask bit and return boolean 0 or 1 */
    if (*ec_slave[slave_no].inputs & BIT (module_index - 1  + startbit))
        return 1;
    else
        return 0;
}

int16 get_output_int16(uint16 slave_no, uint8 module_index)
{
    int16 return_value;
    uint8 *data_ptr;

    /* Get the IO map pointer from the ec_slave struct */
    data_ptr = ec_slave[slave_no].outputs;
    /* Move pointer to correct module index */
    data_ptr += module_index * 2;
    /* Read value byte by byte since all targets can handle misaligned
     * addresses
     */
    return_value = *data_ptr++;
    return_value += (*data_ptr++ << 8);

    return return_value;
}

void set_output_int16 (uint16 slave_no, uint8 module_index, int16 value)
{
    uint8 *data_ptr;
    /* Get the IO map pointer from the ec_slave struct */
    data_ptr = ec_slave[slave_no].outputs;
    /* Move pointer to correct module index */
    data_ptr += module_index * 2;
    /* Read value byte by byte since all targets can handle misaligned
     * addresses
     */
    *data_ptr++ = (value >> 0) & 0xFF;
    *data_ptr++ = (value >> 8) & 0xFF;
}

uint8 get_output_bit (uint16 slave_no, uint8 module_index)
{
    /* Get the the startbit position in slaves IO byte */
    uint8 startbit = ec_slave[slave_no].Ostartbit;
    /* Mask bit and return boolean 0 or 1 */
    if (*ec_slave[slave_no].outputs & BIT (module_index - 1  + startbit))
        return 1;
    else
        return 0;
}

void set_output_bit (uint16 slave_no, uint8 module_index, uint8 value)
{
    /* Get the the startbit position in slaves IO byte */
    uint8 startbit = ec_slave[slave_no].Ostartbit;
    /* Set or Clear bit */
    if (value == 0)
        *ec_slave[slave_no].outputs &= ~(1 << (module_index - 1 + startbit));
    else
        *ec_slave[slave_no].outputs |= (1 << (module_index - 1 + startbit));
}
#endif

void read_io_task(void *p_arg)
{
    /* Function connected to cyclic TTOS task
     * The function is executed cyclic according
     * sceduel specified in schedule.tt
     */
    //*pPORTFIO_SET = BIT (6);
    /* Send and receive processdata
     * Note that you need som synchronization
     * case you modifey IO in local application
     */
    ec_send_processdata();
    ec_receive_processdata(EC_TIMEOUTRET);

    //*pPORTFIO_CLEAR = BIT (6);
}

void soem_plc_sdo_read(soem_msg_st *p_msgSet)
{
    LOGV(TAG, "Enter %s(), bufSize = %u, dataVal = 0x%08X (%u)", __func__, p_msgSet->bufSize, p_msgSet->dataVal, p_msgSet->dataVal);
    if (gSOEMInProcess == false)
    {
        p_msgSet->ret = -100;
        return;
    }
    
    gIfPLCWantProcessSDORead = true;
    
    memcpy(&gSOEMMsg, p_msgSet, sizeof(soem_msg_st));
    
    soem_msg_st msg;
    xQueueReceive(gSoemQueueHandle, &msg, portMAX_DELAY);

    memcpy(p_msgSet, &msg, sizeof(soem_msg_st));
}

void soem_plc_sdo_write(soem_msg_st *p_msgSet)
{
    LOGV(TAG, "Enter %s(), bufSize = %u, dataVal = 0x%08X (%u)", __func__, p_msgSet->bufSize, p_msgSet->dataVal, p_msgSet->dataVal);
    if (gSOEMInProcess == false)
    {
        p_msgSet->ret = -100;
        return;
    }
    gIfPLCWantProcessSDOWrite = true;
    
    memcpy(&gSOEMMsg, p_msgSet, sizeof(soem_msg_st));
    
    soem_msg_st msg;
    xQueueReceive(gSoemQueueHandle, &msg, portMAX_DELAY);

    memcpy(p_msgSet, &msg, sizeof(soem_msg_st));
}

RAMFUNCTION_SECTION_CODE(void notify_handle_data_recv_task(void))
{
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(gRecvDataTaskHandle, &higherPriorityTaskWoken);
    if (higherPriorityTaskWoken == pdTRUE)
    {
        portYIELD_FROM_ISR (higherPriorityTaskWoken);
    }
}

RAMFUNCTION_SECTION_CODE(static void handle_recv_data_ethercat_task(void *p_arg))
{
    LOGI(TAG, "handle_recv_data_ethercat_task RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());
    LOGV(TAG, "sizeof(list_element_ec_t) = %u", sizeof(list_element_ec_t));

    list_element_handle_ec_t pElement;
    EC_LIST_Init(&gRecvBufList, 0);
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        //LOGI(TAG, "wake up.");
        //hexdump(gSOEMRecvBuffer, gSOEMRecvLen);
#if 1
        //uint32_t regPrimask      = DisableGlobalIRQ();
        pElement = pvPortMalloc(sizeof(list_element_ec_t));
        pElement->length = gSOEMRecvLen;
        memcpy(pElement->data, gSOEMRecvBuffer, gSOEMRecvLen);
        EC_LIST_AddTail(&gRecvBufList, pElement);
        //EnableGlobalIRQ(regPrimask);
        //LOGD(TAG, "gRecvBufList size is : %u", gRecvBufList.size);
#endif
    }
}

static inline void set_output_bit(uint16 slave_no, uint8 ch_index, uint8 value)
{
#if 0
   /* Get the the startbit position in slaves IO byte */
   uint8 startbit = ecx_context.slavelist[slave_no].Ostartbit;
   /* Set or Clear bit */
   if (value == 0)
   {
      *ecx_context.slavelist[slave_no].outputs &= ~(1 << (ch_index + startbit));
   }
   else
   {
      *ecx_context.slavelist[slave_no].outputs |= (1 << (ch_index + startbit));
   }
#else
    uint8 offsetBytes = ch_index >> 3; // 除以8
    uint8 *pData = ecx_context.slavelist[slave_no].outputs + (offsetBytes);
    uint8 offsetIdx = ch_index % 8;
    if (value == 0)
    {
        *pData &= ~(1 << (offsetIdx));
    }
    else
    {
        *pData |= (1 << (offsetIdx));
    }
#endif
}

static inline uint8 get_input_bit(uint16 slave_no, uint8 ch_index)
{
#if 0
   /* Get the the startbit position in slaves IO byte */
   uint8 startbit = ecx_context.slavelist[slave_no].Istartbit;
   /* Mask bit and return boolean 0 or 1 */
   if (*ecx_context.slavelist[slave_no].inputs & BIT(ch_index + startbit))
   {
      return 1;
   }
   else
   {
      return 0;
   }
#else
    uint8 offsetBytes = ch_index >> 3; // 除以8
    uint8 *pData = ecx_context.slavelist[slave_no].inputs + (offsetBytes);
    uint8 offsetIdx = ch_index % 8;
    //LOGD(TAG, "%s: ch_index = %u, offsetBytes = %u, offsetIdx = %u", __func__, ch_index, offsetBytes, offsetIdx);
    if ((*pData) & BIT(offsetIdx))
    {
        return 1;
    }
    else
    {
        return 0;
    }
#endif
}

//Copy data(D register) to RxPDO.
#if 0
RAMFUNCTION_SECTION_CODE(static void kalyke_soem_process_RxPDO_data(void))
{
    uint8_t *pData = (uint8_t *)IOmap;
    for (int i = 0; i < ec_slavecount; i++)
    {
        for (int j = 0; j < gSoemConfig.pSlaves[i].pdoRxCount; j++)
        {
            switch(gSoemConfig.pSlaves[i].pRxPDO[j].eType)
            {
            case DAISY_E_TYPE_D:
            {
                if (gSoemConfig.pSlaves[i].pRxPDO[j].len == 4)
                {
                    pData[gSoemConfig.pSlaves[i].pRxPDO[j].offset] = gtv_PlcElement.msp_DElement[gSoemConfig.pSlaves[i].pRxPDO[j].eAddr] & 0x000000FFU;
                    pData[gSoemConfig.pSlaves[i].pRxPDO[j].offset + 1] = (gtv_PlcElement.msp_DElement[gSoemConfig.pSlaves[i].pRxPDO[j].eAddr] & 0x0000FF00U) >> 8;
                    pData[gSoemConfig.pSlaves[i].pRxPDO[j].offset + 2] = (gtv_PlcElement.msp_DElement[gSoemConfig.pSlaves[i].pRxPDO[j].eAddr] & 0x00FF0000U) >> 16;
                    pData[gSoemConfig.pSlaves[i].pRxPDO[j].offset + 3] = (gtv_PlcElement.msp_DElement[gSoemConfig.pSlaves[i].pRxPDO[j].eAddr] & 0xFF000000U) >> 24;
                }
                else if (gSoemConfig.pSlaves[i].pRxPDO[j].len == 2)
                {
                    pData[gSoemConfig.pSlaves[i].pRxPDO[j].offset] = gtv_PlcElement.msp_DElement[gSoemConfig.pSlaves[i].pRxPDO[j].eAddr] & 0x00FF;
                    pData[gSoemConfig.pSlaves[i].pRxPDO[j].offset + 1] = (gtv_PlcElement.msp_DElement[gSoemConfig.pSlaves[i].pRxPDO[j].eAddr] & 0xFF00) >> 8;
                }
                else// == 1
                {
                    pData[gSoemConfig.pSlaves[i].pRxPDO[j].offset] = (uint8_t)gtv_PlcElement.msp_DElement[gSoemConfig.pSlaves[i].pRxPDO[j].eAddr];
                }
            }
            break;
            default:
                LOGE(TAG, "ERROR: Slave : %d, eType = %u", i, gSoemConfig.pSlaves[i].pRxPDO[j].eType);
                break;
            }
        }
    }
}

Copy TxPDO(IOmap) to D register.
RAMFUNCTION_SECTION_CODE(static void kalyke_soem_process_TxPDO_data(void))
{
    uint8_t *pData = (uint8_t *)IOmap;
    for (int i = 0; i < ec_slavecount; i++)
    {
        for (int j = 0; j < gSoemConfig.pSlaves[i].pdoTxCount; j++)
        {
            switch(gSoemConfig.pSlaves[i].pTxPDO[j].eType)
            {
            case DAISY_E_TYPE_D:
            {
                if (gSoemConfig.pSlaves[i].pTxPDO[j].len == 4)
                {
                    uint32_t *pVal = (uint32_t *)(&gtv_PlcElement.msp_DElement[gSoemConfig.pSlaves[i].pTxPDO[j].eAddr]);
                    *pVal = (pData[gSoemConfig.pSlaves[i].pTxPDO[j].offset + 3] << 24) |
                            (pData[gSoemConfig.pSlaves[i].pTxPDO[j].offset + 2] << 16) |
                            (pData[gSoemConfig.pSlaves[i].pTxPDO[j].offset + 1] << 8)  |
                            (pData[gSoemConfig.pSlaves[i].pTxPDO[j].offset]);
                }
                else if (gSoemConfig.pSlaves[i].pTxPDO[j].len == 2)
                {
                    gtv_PlcElement.msp_DElement[gSoemConfig.pSlaves[i].pTxPDO[j].eAddr] = (pData[gSoemConfig.pSlaves[i].pTxPDO[j].offset + 1] << 8) |
                            (pData[gSoemConfig.pSlaves[i].pTxPDO[j].offset]);
                }
                else // == 1
                {
                    gtv_PlcElement.msp_DElement[gSoemConfig.pSlaves[i].pTxPDO[j].eAddr] = pData[gSoemConfig.pSlaves[i].pTxPDO[j].offset];
                }
            }
            break;
            default:
                LOGE(TAG, "ERROR: Slave : %d, eType = %u", i, gSoemConfig.pSlaves[i].pTxPDO[j].eType);
                break;
            }
        }
    }
}
#else
RAMFUNCTION_SECTION_CODE(static void kalyke_soem_process_RxPDO_data(void))
{
    uint8_t *pData = NULL;
//    uint16_t curOffset = 0;
    uint16_t curOffsetBit = 0;
    int kalykeSlaveID;
    for (int i = 1; i <= ec_slavecount; i++)
    {
        kalykeSlaveID = i - 1;
        pData = ecx_context.slavelist[i].outputs;
//        curOffset = 0;
        curOffsetBit = 0;
        for (int j = 0; j < gSoemConfig.pSlaves[kalykeSlaveID].pdoRxCount; j++)
        {
            switch(gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eType)
            {
            case DAISY_E_TYPE_D:
            {
                uint16_t curOffset = curOffsetBit/8;
                if (gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].len == 0x20)
                {
                    //D0双字占用了D1、D0两个元件
                    pData[curOffset] = gtv_PlcElement.msp_DElement[gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr] & 0x00FFU;
                    pData[curOffset + 1] = (gtv_PlcElement.msp_DElement[gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr] & 0xFF00U) >> 8;
                    pData[curOffset + 2] = gtv_PlcElement.msp_DElement[gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr + 1] & 0x00FFU;
                    pData[curOffset + 3] = (gtv_PlcElement.msp_DElement[gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr + 1] & 0xFF00U) >> 8;
                }
                else if (gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].len == 0x10)
                {
                    pData[curOffset] = gtv_PlcElement.msp_DElement[gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr] & 0x00FF;
                    pData[curOffset + 1] = (gtv_PlcElement.msp_DElement[gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr] & 0xFF00) >> 8;
                }
                else if (gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].len == 0x08)
                {
                    pData[curOffset] = (uint8_t)gtv_PlcElement.msp_DElement[gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr];	
                }
                else //bit
                {
                    LOGE(TAG, "%s: ERROR: Slave : %d, eType = %u", __func__, i, gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eType);
                }
                curOffsetBit += gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].len;	
            }
            break;

            case DAISY_E_TYPE_R:
            {
                uint16_t curOffset = curOffsetBit/8;
                if (gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].len == 0x20)
                {
                    //R0双字占用了R1、R0两个元件
                    pData[curOffset] = gtv_PlcElement.msp_RElement[gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr] & 0x00FFU;
                    pData[curOffset + 1] = (gtv_PlcElement.msp_RElement[gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr] & 0xFF00U) >> 8;
                    pData[curOffset + 2] = gtv_PlcElement.msp_RElement[gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr + 1] & 0x00FFU;
                    pData[curOffset + 3] = (gtv_PlcElement.msp_RElement[gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr + 1] & 0xFF00U) >> 8;
                }
                else if (gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].len == 0x10)
                {
                    pData[curOffset] = gtv_PlcElement.msp_RElement[gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr] & 0x00FF;
                    pData[curOffset + 1] = (gtv_PlcElement.msp_RElement[gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr] & 0xFF00) >> 8;
                }
                else if (gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].len == 0x08)
                {
                    pData[curOffset] = (uint8_t)gtv_PlcElement.msp_RElement[gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr];	
                }
                else //bit
                {
                    LOGE(TAG, "%s: ERROR: Slave : %d, eType = %u", __func__, i, gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eType);
                }
                curOffsetBit += gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].len;
            }
            break;


            case DAISY_E_TYPE_M:
            {
                if (gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].len == 0x01)
                {
                    set_output_bit(i, curOffsetBit, plc_get_bit_element_value(M_ELEMENT, gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr));
                }
                else
                {
                    uint16_t curOffset = curOffsetBit/8;
                    if (gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].len == 0x08)
                    {
                        pData[curOffset] = daisy_get_8bit_element_value(M_ELEMENT, gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr);
                    }
                    else if (gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].len == 0x10)
                    {
                        pData[curOffset] = daisy_get_8bit_element_value(M_ELEMENT, gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr);
                        pData[curOffset+1] = daisy_get_8bit_element_value(M_ELEMENT, gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr+8);
                    }
                }
                curOffsetBit += gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].len;
            }
            break;
            case DAISY_E_TYPE_X:
            {
                if (gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].len == 0x01)
                {
                    set_output_bit(i, curOffsetBit, plc_get_bit_element_value(X_ELEMENT, gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr));
                }
                else
                {
                    uint16_t curOffset = curOffsetBit/8;
                    if (gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].len == 0x08)
                    {
                        pData[curOffset] = daisy_get_8bit_element_value(X_ELEMENT, gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr);
                    }
                    else if (gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].len == 0x10)
                    {
                        pData[curOffset] = daisy_get_8bit_element_value(X_ELEMENT, gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr);
                        pData[curOffset+1] = daisy_get_8bit_element_value(X_ELEMENT, gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr+8);
                    }
                }

                curOffsetBit += gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].len;
            }
            break;
            case DAISY_E_TYPE_Y:
            {
                if (gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].len == 0x01)
                {
                    set_output_bit(i, curOffsetBit, plc_get_bit_element_value(Y_ELEMENT, gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr));
                }
                else
                {
                    uint16_t curOffset = curOffsetBit/8;
                    if (gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].len == 0x08)
                    {
                        pData[curOffset] = daisy_get_8bit_element_value(Y_ELEMENT, gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr);
                    }
                    else if (gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].len == 0x10)
                    {
                        pData[curOffset]   = daisy_get_8bit_element_value(Y_ELEMENT, gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr);
                        pData[curOffset+1] = daisy_get_8bit_element_value(Y_ELEMENT, gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eAddr+8);
                    }
                }

                curOffsetBit += gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].len;
            }
            break;
            
            case 0x00:
                curOffsetBit += gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].len;
                break;

            default:
                curOffsetBit += gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].len;
                LOGE(TAG, "%s: ERROR: Slave : %d, eType = %u", __func__, i, gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[j].eType);
                break;
            }
        }
    }
}

//Copy TxPDO(IOmap) to D register.
RAMFUNCTION_SECTION_CODE(static void kalyke_soem_process_TxPDO_data(void))
{
    uint8_t *pData = NULL;
//    uint16_t curOffset = 0;
    uint16_t curOffsetBit = 0;
    int kalykeSlaveID;
    for (int i = 1; i <= ec_slavecount; i++)
    {
        kalykeSlaveID = i - 1;
        pData = ecx_context.slavelist[i].inputs;
//        curOffset = 0;
        curOffsetBit = 0;
        for (int j = 0; j < gSoemConfig.pSlaves[kalykeSlaveID].pdoTxCount; j++)
        {
            switch(gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].eType)
            {
            case DAISY_E_TYPE_D:
            {
                uint16_t curOffset = curOffsetBit/8;
                if (gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].len == 0x20)
                {
                    uint32_t *pVal = (uint32_t *)(&gtv_PlcElement.msp_DElement[gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].eAddr]);
                    *pVal = (pData[curOffset + 3] << 24) |
                            (pData[curOffset + 2] << 16) |
                            (pData[curOffset + 1] << 8)  |
                            (pData[curOffset]);
                }
                else if (gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].len == 0x10)
                {
                    gtv_PlcElement.msp_DElement[gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].eAddr] = (pData[curOffset + 1] << 8) |
                            (pData[curOffset]);
                 }
                else if (gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].len == 0x08)
                {
                    gtv_PlcElement.msp_DElement[gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].eAddr] = pData[curOffset];
                }
                else
                {
                    LOGE(TAG, "%s: ERROR: Slave : %d, eType = %u", __func__, i, gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].eType);
                }
                curOffsetBit += gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].len;
            }
            break;

            case DAISY_E_TYPE_R:
            {
                uint16_t curOffset = curOffsetBit/8;
                if (gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].len == 0x20)
                {
                    uint32_t *pVal = (uint32_t *)(&gtv_PlcElement.msp_RElement[gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].eAddr]);
                    *pVal = (pData[curOffset + 3] << 24) |
                            (pData[curOffset + 2] << 16) |
                            (pData[curOffset + 1] << 8)  |
                            (pData[curOffset]);
                }
                else if (gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].len == 0x10)
                {
                    gtv_PlcElement.msp_RElement[gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].eAddr] = (pData[curOffset + 1] << 8) |
                            (pData[curOffset]);
                 }
                else if (gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].len == 0x08)
                {
                    gtv_PlcElement.msp_RElement[gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].eAddr] = pData[curOffset];
                }
                else
                {
                   LOGE(TAG, "%s: ERROR: Slave : %d, eType = %u", __func__, i, gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].eType);
                }
                curOffsetBit += gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].len;
            }
            break;
            case DAISY_E_TYPE_M:
            {
                if (gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].len == 0x01)
                {
                    plc_set_bit_element_value(M_ELEMENT, gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].eAddr, get_input_bit(i, curOffsetBit));
                }
                else
                {
                   uint16_t curOffset = curOffsetBit/8;
                   uint16_t curOffsetElem = gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].eAddr/8;
                   if(gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].len == 0x08)
                   {
                       daisy_set_8bit_element_value(M_ELEMENT,curOffsetElem, curOffset);
                   }
                   else if(gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].len == 0x10)
                   {
                        daisy_set_8bit_element_value(M_ELEMENT,curOffsetElem, curOffset+1);
                        daisy_set_8bit_element_value(M_ELEMENT,curOffsetElem+1, curOffset); 	
                   }
                }


                curOffsetBit += gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].len;	
            }
            break;
            case DAISY_E_TYPE_X:
            {
                if (gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].len == 0x01)
                {
                    plc_set_bit_element_value(X_ELEMENT, gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].eAddr, get_input_bit(i, curOffsetBit));
                }
                else
                {
                    uint16_t curOffset = curOffsetBit/8;
                    uint16_t curOffsetElem = gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].eAddr/8;
                    if(gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].len == 0x08)
                    {
                        daisy_set_8bit_element_value(X_ELEMENT,curOffsetElem, curOffset);
                    }
                    else if(gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].len == 0x10)
                    {
                        daisy_set_8bit_element_value(X_ELEMENT,curOffsetElem, curOffset+1);
                        daisy_set_8bit_element_value(X_ELEMENT,curOffsetElem+1, curOffset); 	
                    }
                }
                curOffsetBit += gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].len;	
            }
            break;
            case DAISY_E_TYPE_Y:
            {
                if (gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].len == 0x01)
                {
                    plc_set_bit_element_value(Y_ELEMENT, gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].eAddr, get_input_bit(i, curOffsetBit));
                }
                else
                {
                    uint16_t curOffset = curOffsetBit/8;
                    uint16_t curOffsetElem = gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].eAddr/8;
                    if(gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].len == 0x08)
                    {
                        daisy_set_8bit_element_value(Y_ELEMENT,curOffsetElem, curOffset);
                    }
                    else if(gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].len == 0x10)
                    {
                        daisy_set_8bit_element_value(Y_ELEMENT,curOffsetElem, curOffset+1);
                        daisy_set_8bit_element_value(Y_ELEMENT,curOffsetElem+1, curOffset); 
                    }
                }
                curOffsetBit += gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].len;	
            }
            break;
            
            case 0x00:
                curOffsetBit += gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].len;
                break;

            default:
                curOffsetBit += gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].len;
                LOGE(TAG, "%s: ERROR: Slave : %d, eType = %u", __func__, i, gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[j].eType);
                break;
            }
        }
    }
}

#endif

static void log_slaves(int cnt)
{
    ec_slavet *ec_slave = ecx_context.slavelist;

    LOGD(TAG, "Enter %s(), cnt = %d", __func__, cnt);
    LOGV(TAG, "Slave:%d, Name:%s, Output size: %dbits, Input size: %dbits, State: %d, Delay: %d[ns], Has DC: %d",
         cnt, ec_slave[cnt].name, ec_slave[cnt].Obits, ec_slave[cnt].Ibits,
         ec_slave[cnt].state, ec_slave[cnt].pdelay, ec_slave[cnt].hasdc);
    LOGD(TAG, " Configured address: %x", ec_slave[cnt].configadr);
    LOGI(TAG, " Outputs address: %p", ec_slave[cnt].outputs);
    LOGW(TAG, " Inputs address: %p", ec_slave[cnt].inputs);

    LOGW(TAG, " FMMUunused: %u", ec_slave[cnt].FMMUunused);
    for(int j = 0 ; j < ec_slave[cnt].FMMUunused ; j++)
    {
        LOGV(TAG, " FMMU%1d, Ls:%x, Ll:%4d, Lsb:%d, Leb:%d, Ps:%x, Psb:%d, Ty:%x, Act:%x", j,
             (int)ec_slave[cnt].FMMU[j].LogStart, ec_slave[cnt].FMMU[j].LogLength, ec_slave[cnt].FMMU[j].LogStartbit,
             ec_slave[cnt].FMMU[j].LogEndbit, ec_slave[cnt].FMMU[j].PhysStart, ec_slave[cnt].FMMU[j].PhysStartBit,
             ec_slave[cnt].FMMU[j].FMMUtype, ec_slave[cnt].FMMU[j].FMMUactive);
    }
    LOGV(TAG, " FMMUfunc 0:%d 1:%d 2:%d 3:%d\n",
         ec_slave[cnt].FMMU0func, ec_slave[cnt].FMMU1func, ec_slave[cnt].FMMU2func, ec_slave[cnt].FMMU3func);

    LOGD(TAG, "ALstatuscode = 0x%04X, aliasadr = 0x%04X", ec_slave[cnt].ALstatuscode, ec_slave[cnt].aliasadr);
    LOGI(TAG, "eep_man = 0x%08X, eep_id = 0x%08X, eep_rev = 0x%08X", ec_slave[cnt].eep_man, ec_slave[cnt].eep_id, ec_slave[cnt].eep_rev);
    LOGD(TAG, "Itype = 0x%04X, Dtype = 0x%04X", ec_slave[cnt].Itype, ec_slave[cnt].Dtype);

    LOGD(TAG, "Obytes = %u, Ostartbit = %u", ec_slave[cnt].Obytes, ec_slave[cnt].Ostartbit);
    LOGW(TAG, "Ibytes = %u, Istartbit = %u", ec_slave[cnt].Ibytes, ec_slave[cnt].Istartbit);

    LOGV(TAG, "mbx_rl = %u, mbx_ro = %u, mbx_l = %u, mbx_wo = %u", ec_slave[cnt].mbx_rl, ec_slave[cnt].mbx_ro, ec_slave[cnt].mbx_l, ec_slave[cnt].mbx_wo);
    LOGV(TAG, "mbx_proto = 0x%04X, mbx_cnt = %u", ec_slave[cnt].mbx_proto, ec_slave[cnt].mbx_cnt);

    LOGI(TAG, "CoEdetails = 0x%X, FoEdetails = 0x%X, EoEdetails = 0x%X, SoEdetails = 0x%X", ec_slave[cnt].CoEdetails, ec_slave[cnt].FoEdetails, ec_slave[cnt].EoEdetails, ec_slave[cnt].SoEdetails);
    LOGD(TAG, "Slave %d has CA? %s\n", cnt, ec_slave[cnt].CoEdetails & ECT_COEDET_SDOCA ? "true" : "false" );
}

#if 0
static void MiStudio_PDO_Map(int slaveID)
{
    LOGV(TAG, "Enter %s(), slaveID = %d", __func__, slaveID);
    int ret;
    uint32_t buf32;
    uint8_t buf8;

    /* RxPDO map */
    /* 参看LXM28E User Guide_Chinese.pdf的325页
     * 6040 - 索引
     *     00   - 子索引
     *       08   - 对象长度是8位
     *       10   - 对象长度是16位
     *       20   - 对象长度是32位
    */
    buf32 = 0x60400010;//控制字
    ret = ec_SDOwrite(slaveID, KOD_INDEX_1600, KOD_INDEX_1600_SUB_1, FALSE, 4, &buf32, EC_TIMEOUTRXM);
    LOGI(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, KOD_INDEX_1600, KOD_INDEX_1600_SUB_1, ret, 4, (unsigned int)buf32);

    buf32 = 0x60FF0020;//目标速度
    ret = ec_SDOwrite(slaveID, KOD_INDEX_1600, KOD_INDEX_1600_SUB_2, FALSE, 4, &buf32, EC_TIMEOUTRXM);
    LOGI(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, KOD_INDEX_1600, KOD_INDEX_1600_SUB_2, ret, 4, (unsigned int)buf32);

    buf32 = 0x607A0020;//目标位置
    ret = ec_SDOwrite(slaveID, KOD_INDEX_1600, KOD_INDEX_1600_SUB_3, FALSE, 4, &buf32, EC_TIMEOUTRXM);
    LOGI(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, KOD_INDEX_1600, KOD_INDEX_1600_SUB_3, ret, 4, (unsigned int)buf32);

    buf32 = 0x60FE0120;//物理输出
    ret = ec_SDOwrite(slaveID, KOD_INDEX_1600, KOD_INDEX_1600_SUB_4, FALSE, 4, &buf32, EC_TIMEOUTRXM);
    LOGI(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, KOD_INDEX_1600, KOD_INDEX_1600_SUB_4, ret, 4, (unsigned int)buf32);

    buf32 = 0x60600008;//操作模式
    ret = ec_SDOwrite(slaveID, KOD_INDEX_1600, KOD_INDEX_1600_SUB_5, FALSE, 4, &buf32, EC_TIMEOUTRXM);
    LOGI(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, KOD_INDEX_1600, KOD_INDEX_1600_SUB_5, ret, 4, (unsigned int)buf32);

    buf8 = 5;
    ret = ec_SDOwrite(slaveID, KOD_INDEX_1600, KOD_INDEX_1600_SUB_0, FALSE, 1, &buf8, EC_TIMEOUTRXM);
    LOGI(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, KOD_INDEX_1600, KOD_INDEX_1600_SUB_0, ret, 1, (unsigned int)buf8);

    /* TxPDO map */
    buf32 = 0x60410010; //状态字
    ret = ec_SDOwrite(slaveID, KOD_INDEX_1A00, KOD_INDEX_1A00_SUB_1, FALSE, 4, &buf32, EC_TIMEOUTRXM);
    LOGI(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, KOD_INDEX_1A00, KOD_INDEX_1A00_SUB_1, ret, 4, (unsigned int)buf32);

    buf32 = 0x60640020; //当前位置
    ret = ec_SDOwrite(slaveID, KOD_INDEX_1A00, KOD_INDEX_1A00_SUB_2, FALSE, 4, &buf32, EC_TIMEOUTRXM);
    LOGI(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, KOD_INDEX_1A00, KOD_INDEX_1A00_SUB_2, ret, 4, (unsigned int)buf32);

    buf32 = 0x606C0020; //当前速度
    ret = ec_SDOwrite(slaveID, KOD_INDEX_1A00, KOD_INDEX_1A00_SUB_3, FALSE, 4, &buf32, EC_TIMEOUTRXM);
    LOGI(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, KOD_INDEX_1A00, KOD_INDEX_1A00_SUB_3, ret, 4, (unsigned int)buf32);

    buf32 = 0x603F0010; //错误代码
    ret = ec_SDOwrite(slaveID, KOD_INDEX_1A00, KOD_INDEX_1A00_SUB_4, FALSE, 4, &buf32, EC_TIMEOUTRXM);
    LOGI(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, KOD_INDEX_1A00, KOD_INDEX_1A00_SUB_4, ret, 4, (unsigned int)buf32);

    buf32 = 0x60FD0020; //数字输入
    ret = ec_SDOwrite(slaveID, KOD_INDEX_1A00, KOD_INDEX_1A00_SUB_5, FALSE, 4, &buf32, EC_TIMEOUTRXM);
    LOGI(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, KOD_INDEX_1A00, KOD_INDEX_1A00_SUB_5, ret, 4, (unsigned int)buf32);

    buf8 = 5;
    ret = ec_SDOwrite(slaveID, KOD_INDEX_1A00, KOD_INDEX_1A00_SUB_0, FALSE, 1, &buf8, EC_TIMEOUTRXM);
    LOGI(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, KOD_INDEX_1A00, KOD_INDEX_1A00_SUB_0, ret, 1, (unsigned int)buf8);
}
#else
static void MiStudio_SDO_SET(int slaveID)
{
    LOGV(TAG, "Enter %s(), slaveID = %d", __func__, slaveID);
    int kalykeSlaveID;
    uint8_t buf8;
    uint16_t buf16;
    uint32_t buf32;
    ec_slavet *ec_slave = ecx_context.slavelist;
    kalykeSlaveID = slaveID - 1;
    for (int i = 0; i < gSoemConfig.pSlaves[kalykeSlaveID].nSdoCount; i++)
    {
        uint16_t idx = gSoemConfig.pSlaves[kalykeSlaveID].pSDO[i].idx;
        uint8_t subIdx = gSoemConfig.pSlaves[kalykeSlaveID].pSDO[i].subIdx;
        LOGD(TAG, "idx=0x%X, subIdx=0x%X, len = %u", idx, subIdx, gSoemConfig.pSlaves[kalykeSlaveID].pSDO[i].len);
        if (gSoemConfig.pSlaves[kalykeSlaveID].pSDO[i].len == 0x08)
        {
            SDO_WRITE(slaveID, idx, subIdx, buf8, gSoemConfig.pSlaves[kalykeSlaveID].pSDO[i].val, "");
        }
        else if (gSoemConfig.pSlaves[kalykeSlaveID].pSDO[i].len == 0x10)
        {
            SDO_WRITE(slaveID, idx, subIdx, buf16, gSoemConfig.pSlaves[kalykeSlaveID].pSDO[i].val, "");
        }
        else
        {
            SDO_WRITE(slaveID, idx, subIdx, buf32, gSoemConfig.pSlaves[kalykeSlaveID].pSDO[i].val, "");
        }
        LOGW(TAG, "ec_elist2string: %s", ec_elist2string());
    }
}


static void MiStudio_PDO_Map(int slaveID)
{
    LOGV(TAG, "Enter %s(), slaveID = %d", __func__, slaveID);
    int ret;
    uint32_t buf32;
    uint8_t  buf8;
    uint16_t buf16;	
    uint16_t sIdxHalf;
    uint16_t cIndex;
    uint8_t subIndex;
    int kalykeSlaveID = slaveID - 1;

/* RxPDO map */
/*
#if 1
    cIndex = gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[0].cIdx; //有可能没有RxPDO
    subIndex = 0;
#else
    cIndex = KOD_INDEX_1600;
#endif
*/
    uint8_t u8_1C12_Sub = 0;
    for (int i = 0; i < gSoemConfig.pSlaves[kalykeSlaveID].pdoRxCount; i++)
    {
        if (i == 0)
        {
          cIndex = gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[0].cIdx; 
          subIndex = 0;

          //1 停止 PDO 分配功能（0x1C12的子索引 0 设置为 0）
          buf8 = 0;
          ret = ec_SDOwrite(slaveID, KOD_INDEX_1C12, KOD_INDEX_1C12_SUB_0, FALSE, 1, &buf8, EC_TIMEOUTRXM);
          LOGV(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, KOD_INDEX_1C12, KOD_INDEX_1C12_SUB_0, ret, 1, (unsigned int)buf8);

          //2 停止 PDO 映射功能（0x1600～0x16FF 的子索引 0 全部设为 0）。
          buf8 = 0;
          ret = ec_SDOwrite(slaveID, cIndex, KOD_INDEX_1600_SUB_0, FALSE, 1, &buf8, EC_TIMEOUTRXM);
          LOGI(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, cIndex, KOD_INDEX_1600_SUB_0, ret, 1, (unsigned int)buf8);			
        }
        else if (cIndex != gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[i].cIdx)
        {
           //4 设置 PDO 映射对象（0x1600～0x16FF）映射入口的数值。
           ret = ec_SDOwrite(slaveID, cIndex, KOD_INDEX_1600_SUB_0, FALSE, 1, &subIndex, EC_TIMEOUTRXM);
           LOGV(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, cIndex, KOD_INDEX_1600_SUB_0, ret, 1, (unsigned int)subIndex);

           //5 设置 PDO 分配对象（ 0x1C12 的子索引 1）。
           u8_1C12_Sub++;
           buf16 = cIndex;
           ret = ec_SDOwrite(slaveID, KOD_INDEX_1C12, u8_1C12_Sub, FALSE, 2, &buf16, EC_TIMEOUTRXM);
           LOGV(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, KOD_INDEX_1C12, u8_1C12_Sub, ret, 2, (unsigned int)buf16);

           //2 停止 PDO 映射功能（0x1600～0x16FF的子索引 0 全部设为 0）。
           cIndex = gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[i].cIdx;
           subIndex = 0;
           buf8 = 0;
           ret = ec_SDOwrite(slaveID, cIndex, subIndex, FALSE, 1, &buf8, EC_TIMEOUTRXM);
           LOGI(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, cIndex, KOD_INDEX_1600_SUB_0, ret, 1, (unsigned int)buf8);      
       	}

        sIdxHalf = gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[i].sIdx >> 16;
        LOGD(TAG, "RxPDO, sIdxHalf = 0x%04X", sIdxHalf);
#if 0
#if 0
        switch (sIdxHalf)
        {
        case KOD_INDEX_6040:
            subIndex = 1;
            break;
        case KOD_INDEX_60FF:
            subIndex = 2;
            break;
        case KOD_INDEX_607A:
            subIndex = 3;
            break;
        case KOD_INDEX_60FE:
            subIndex = 4;
            break;
        case KOD_INDEX_6060:
            subIndex = 5;
            break;
        default:
            break;
        }
#else
        subIndex = i + 1;
#endif
#endif
        subIndex++;
        buf32 = gSoemConfig.pSlaves[kalykeSlaveID].pRxPDO[i].sIdx;
        ret = ec_SDOwrite(slaveID, cIndex, subIndex, FALSE, 4, &buf32, EC_TIMEOUTRXM);
        LOGI(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, cIndex, subIndex, ret, 4, (unsigned int)buf32);
    }
#if 0
#if 0
    buf8 = 5; //RxPDO的个数
#else
    buf8 = gSoemConfig.pSlaves[kalykeSlaveID].pdoRxCount; //RxPDO的个数
#endif
#endif
    if (gSoemConfig.pSlaves[kalykeSlaveID].pdoRxCount > 0)
    {
       //4 设置 PDO 映射对象（0x1600～0x16FF）映射入口的数值。
       ret = ec_SDOwrite(slaveID, cIndex, KOD_INDEX_1600_SUB_0, FALSE, 1, &subIndex, EC_TIMEOUTRXM);
       LOGV(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, cIndex, KOD_INDEX_1600_SUB_0, ret, 1, (unsigned int)subIndex);

       //5 设置 PDO 分配对象（设置 0x1C12的子索引 1）。
       u8_1C12_Sub++;
       buf16 = cIndex;
       ret = ec_SDOwrite(slaveID, KOD_INDEX_1C12, u8_1C12_Sub, FALSE, 2, &buf16, EC_TIMEOUTRXM);
       LOGV(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, KOD_INDEX_1C12, u8_1C12_Sub, ret, 2, (unsigned int)buf16);   

       //6 重新打开 PDO 分配功能（设置 0x1C12的子索引 0 为 个数）。
       // buf8 = 1; ????
       buf8 = u8_1C12_Sub;
       ret = ec_SDOwrite(slaveID, KOD_INDEX_1C12, KOD_INDEX_1C12_SUB_0, FALSE, 1, &buf8, EC_TIMEOUTRXM);
       LOGV(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, KOD_INDEX_1C12, KOD_INDEX_1C12_SUB_0, ret, 1, (unsigned int)buf8);
    }


    /* TxPDO map */
/*
#if 1
    cIndex = gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[0].cIdx; //有可能没有TxPDO
    subIndex = 0;
#else
    cIndex = KOD_INDEX_1A00;
#endif
*/

    uint8_t u8_1C13_Sub = 0;
    for (int i = 0; i < gSoemConfig.pSlaves[kalykeSlaveID].pdoTxCount; i++)
    {
        if (i == 0)
        {
          cIndex = gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[0].cIdx; 
          subIndex = 0;

         //1 停止 PDO 分配功能（0x1C13 的子索引 0 设置为 0）
         buf8 = 0;
         ret = ec_SDOwrite(slaveID, KOD_INDEX_1C13, KOD_INDEX_1C13_SUB_0, FALSE, 1, &buf8, EC_TIMEOUTRXM);
         LOGD(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, KOD_INDEX_1C13, KOD_INDEX_1C13_SUB_0, ret, 1, (unsigned int)buf8);
 

         //2 停止 PDO 映射功能（ 0x1A00～0x1AFF 的子索引 0 全部设为 0）。
         buf8 = 0;
          ret = ec_SDOwrite(slaveID, cIndex, KOD_INDEX_1A00_SUB_0, FALSE, 1, &buf8, EC_TIMEOUTRXM);
          LOGI(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, cIndex, KOD_INDEX_1A00_SUB_0, ret, 1, (unsigned int)buf8);		
        }

        else if (cIndex != gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[i].cIdx)
        {
           //4 设置 PDO 映射对象（0x1A00～0x1AFF）映射入口的数值。
           ret = ec_SDOwrite(slaveID, cIndex, KOD_INDEX_1A00_SUB_0, FALSE, 1, &subIndex, EC_TIMEOUTRXM);
           LOGI(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, cIndex, KOD_INDEX_1A00_SUB_0, ret, 1, (unsigned int)subIndex);

           //5 设置 PDO 分配对象（设置 0x1C13 的子索引）。
           u8_1C13_Sub++;
           buf16 = cIndex;
           ret = ec_SDOwrite(slaveID, KOD_INDEX_1C13, u8_1C13_Sub, FALSE, 2, &buf16, EC_TIMEOUTRXM);
           LOGV(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, KOD_INDEX_1C13, u8_1C13_Sub, ret, 2, (unsigned int)buf16);

           //2 停止 PDO 映射功能（ 0x1A00～0x1AFF 的子索引 0 全部设为 0）。
           cIndex   = gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[i].cIdx;
           subIndex = 0;
           buf8 = 0;
           ret = ec_SDOwrite(slaveID, cIndex, subIndex, FALSE, 1, &buf8, EC_TIMEOUTRXM);
           LOGI(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, cIndex, KOD_INDEX_1A00_SUB_0, ret, 1, (unsigned int)buf8);
       	}

        sIdxHalf = gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[i].sIdx >> 16;
        LOGD(TAG, "TxPDO, sIdxHalf = 0x%04X", sIdxHalf);
#if 0
#if 0
        switch (sIdxHalf)
        {
        case KOD_INDEX_6041:
            subIndex = 1;
            break;
        case KOD_INDEX_6064:
            subIndex = 2;
            break;
        case KOD_INDEX_606C:
            subIndex = 3;
            break;
        case KOD_INDEX_603F:
            subIndex = 4;
            break;
        case KOD_INDEX_60FD:
            subIndex = 5;
            break;
        default:
            break;
        }
#else
        subIndex = i + 1;
#endif
#endif
        subIndex++;
        buf32 = gSoemConfig.pSlaves[kalykeSlaveID].pTxPDO[i].sIdx;
        ret = ec_SDOwrite(slaveID, cIndex, subIndex, FALSE, 4, &buf32, EC_TIMEOUTRXM);
        LOGI(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, cIndex, subIndex, ret, 4, (unsigned int)buf32);
    }

#if 0
#if 0
    buf8 = 5; //TxPDO的个数
#else
    buf8 = gSoemConfig.pSlaves[kalykeSlaveID].pdoTxCount; //TxPDO的个数
#endif
#endif
    if (gSoemConfig.pSlaves[kalykeSlaveID].pdoTxCount > 0)
    {
       //4 设置 PDO 映射对象（ 0x1A00～0x1AFF）映射入口的数值。
       ret = ec_SDOwrite(slaveID, cIndex, KOD_INDEX_1A00_SUB_0, FALSE, 1, &subIndex, EC_TIMEOUTRXM);
       LOGI(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, cIndex, KOD_INDEX_1A00_SUB_0, ret, 1, (unsigned int)subIndex);

       //5 设置 PDO 分配对象（设置 0x1C13 的子索引）。
       u8_1C13_Sub++;
       buf16 = cIndex;
       ret = ec_SDOwrite(slaveID, KOD_INDEX_1C13, u8_1C13_Sub, FALSE, 2, &buf16, EC_TIMEOUTRXM);
       LOGV(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, KOD_INDEX_1C13, u8_1C13_Sub, ret, 2, (unsigned int)buf16);

       //6 重新打开 PDO 分配功能（设置 0x1C13的子索引 0 为 个数）。
       // buf8 = 1;
       buf8 = u8_1C13_Sub;
       ret = ec_SDOwrite(slaveID, KOD_INDEX_1C13, KOD_INDEX_1C13_SUB_0, FALSE, 1, &buf8, EC_TIMEOUTRXM);
       LOGD(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveID, KOD_INDEX_1C13, KOD_INDEX_1C13_SUB_0, ret, 1, (unsigned int)buf8);
    }
    LOGD(TAG, "Leave %s(), slaveID = %d", __func__, slaveID);
}
#endif

static void log_some_sdo(void)
{
    LOGV(TAG, "Enter %s(), ec_slavecount = %d", __func__, ec_slavecount);
    int32 ob2;
    int os;
    uint32_t buf32;
    uint8_t buf8;
    uint16_t buf16;
    int osize, isize;
    uint8_t rxPDOcount, txPDOcount;
    
    for (int i = 1; i <= 1; i++)
    {
        int ret = ec_readPDOmap(i, &osize, &isize);
        LOGD(TAG, "ec_readPDOmap return : %d, osize=%d, isize=%d", ret, osize, isize);
#if 1
        SDO_READ(i, 0x7000, 1, buf8, "7000:1");
        LOGW(TAG, "ec_elist2string: %s", ec_elist2string());
    #if 0
        SDO_READ(i, 0x1C00, 0, buf8, "1C00:0");
        LOGW(TAG, "ec_elist2string: %s", ec_elist2string());
    #endif
        SDO_READ(i, KOD_INDEX_1C12, 0, rxPDOcount, "rxPDO:0");
        LOGW(TAG, "ec_elist2string: %s", ec_elist2string());
        SDO_READ(i, KOD_INDEX_1C12, 1, buf16, "rxPDO:1");
        LOGW(TAG, "ec_elist2string: %s", ec_elist2string());
        SDO_READ(i, KOD_INDEX_1C12, 2, buf16, "rxPDO:2");
        LOGW(TAG, "ec_elist2string: %s", ec_elist2string());
        SDO_READ(i, KOD_INDEX_1C12, 3, buf16, "rxPDO:3");
        LOGW(TAG, "ec_elist2string: %s", ec_elist2string());
        
        SDO_READ(i, KOD_INDEX_1C13, 0, txPDOcount, "txPDO:0");
        LOGW(TAG, "ec_elist2string: %s", ec_elist2string());
        SDO_READ(i, KOD_INDEX_1C13, 1, buf16, "txPDO:1");
        LOGW(TAG, "ec_elist2string: %s", ec_elist2string());
        SDO_READ(i, KOD_INDEX_1C13, 2, buf16, "txPDO:2");
        LOGW(TAG, "ec_elist2string: %s", ec_elist2string());
        SDO_READ(i, KOD_INDEX_1C13, 3, buf16, "txPDO:3");
        LOGW(TAG, "ec_elist2string: %s", ec_elist2string());

    #if 1
        SDO_READ(i, KOD_INDEX_1600, 0, buf8, "");
        SDO_READ(i, KOD_INDEX_1600, 1, buf32, "");
        SDO_READ(i, KOD_INDEX_1600, 2, buf32, "");
        SDO_READ(i, KOD_INDEX_1600, 3, buf32, "");
        SDO_READ(i, KOD_INDEX_1600, 4, buf32, "");
    #endif
        SDO_READ(i, KOD_INDEX_1600, 5, buf32, "");
    #if 1
        SDO_READ(i, KOD_INDEX_1600, 6, buf32, "");
        SDO_READ(i, KOD_INDEX_1600, 7, buf32, "");
        SDO_READ(i, KOD_INDEX_1600, 8, buf32, "");
        SDO_READ(i, KOD_INDEX_1600, 9, buf32, "");
    #endif
        SDO_READ(i, KOD_INDEX_1600, 10, buf32, "");
        LOGW(TAG, "ec_elist2string: %s", ec_elist2string());

        SDO_READ(i, KOD_INDEX_1601, 0, buf8, "");
        SDO_READ(i, KOD_INDEX_1601, 1, buf32, "");
        SDO_READ(i, KOD_INDEX_1601, 2, buf32, "");
        SDO_READ(i, KOD_INDEX_1601, 3, buf32, "");
        SDO_READ(i, KOD_INDEX_1601, 4, buf32, "");
        LOGW(TAG, "ec_elist2string: %s", ec_elist2string());

        SDO_READ(i, KOD_INDEX_1A00, 0, buf8, "");
        SDO_READ(i, KOD_INDEX_1A00, 1, buf32, "");
        SDO_READ(i, KOD_INDEX_1A00, 2, buf32, "");
        SDO_READ(i, KOD_INDEX_1A00, 3, buf32, "");
        SDO_READ(i, KOD_INDEX_1A00, 4, buf32, "");

        SDO_READ(i, KOD_INDEX_1A01, 0, buf8, "");
        SDO_READ(i, KOD_INDEX_1A01, 1, buf32, "");
        SDO_READ(i, KOD_INDEX_1A01, 2, buf32, "");
        SDO_READ(i, KOD_INDEX_1A01, 3, buf32, "");
        SDO_READ(i, KOD_INDEX_1A01, 4, buf32, "");
        #if 0
        os = sizeof(ob2);
        ob2 = 0x16020001;
        ec_SDOwrite(i, 0x1c12, 0, TRUE, os, &ob2, EC_TIMEOUTRXM);
        os = sizeof(ob2);
        ob2 = 0x1a020001;
        ec_SDOwrite(i, 0x1c13, 0, TRUE, os, &ob2, EC_TIMEOUTRXM);
        #endif
#endif
    }
}

#if 0
static void kalyke_LXM28E_SDO_set(void)
{
    LOGV(TAG, "Enter %s()", __func__);
    int slaveId = 1;
    int kalykeSlaveID;
    uint8_t buf8;
    uint16_t buf16;
    uint32_t buf32;
    ec_slavet *ec_slave = ecx_context.slavelist;
AGAIN:
    ec_slave[slaveId].state = EC_STATE_PRE_OP;
    ec_statecheck(slaveId, EC_STATE_PRE_OP, EC_TIMEOUTSTATE);
    kalykeSlaveID = slaveId - 1;
    for (int i = 0; i < gSoemConfig.pSlaves[kalykeSlaveID].nSdoCount; i++)
    {
        uint16_t idx = gSoemConfig.pSlaves[kalykeSlaveID].pSDO[i].idx;
        uint8_t subIdx = gSoemConfig.pSlaves[kalykeSlaveID].pSDO[i].subIdx;
        if (gSoemConfig.pSlaves[kalykeSlaveID].pSDO[i].len == 1)
        {
            SDO_WRITE(slaveId, idx, subIdx, buf8, gSoemConfig.pSlaves[kalykeSlaveID].pSDO[i].val, "");
        }
        else if (gSoemConfig.pSlaves[kalykeSlaveID].pSDO[i].len == 2)
        {
            SDO_WRITE(slaveId, idx, subIdx, buf16, gSoemConfig.pSlaves[kalykeSlaveID].pSDO[i].val, "");
        }
        else
        {
            SDO_WRITE(slaveId, idx, subIdx, buf32, gSoemConfig.pSlaves[kalykeSlaveID].pSDO[i].val, "");
        }
    }

    slaveId++;
    if (slaveId > ec_slavecount)
    {
        return;
    }
    else
    {
        goto AGAIN;
    }
}
#endif

static void kalyke_RemotIO_SDO_SET(int slaveId)
{
    LOGD(TAG, "Enter %s()", __func__);

    ec_slavet *ec_slave = ecx_context.slavelist;

    ec_slave[slaveId].state = EC_STATE_PRE_OP;
    ec_statecheck(slaveId, EC_STATE_PRE_OP, EC_TIMEOUTSTATE);
    
    MiStudio_SDO_SET(slaveId);
    //log_slaves(slaveId);
    //if (slaveId == 1)
    //{
        //log_some_sdo();
    //}
}
//配置Ethercat伺服
static void kalyke_Drive_SDO_SET_PDO_map(int slaveId)
{
    LOGV(TAG, "Enter %s(), slaveId = %d", __func__, slaveId);

    ec_slavet *ec_slave = ecx_context.slavelist;

    ec_slave[slaveId].state = EC_STATE_PRE_OP;
    ec_statecheck(slaveId, EC_STATE_PRE_OP, EC_TIMEOUTSTATE);

    MiStudio_SDO_SET(slaveId);

    MiStudio_PDO_Map(slaveId);
}

int lxm28e_PO2SOconfigx(ecx_contextt *context, uint16 slave)
{
    LOGV(TAG, "Enter %s(), slave = %u", __func__, slave);

    int ret;
    int slaveId = slave;
    uint8_t buf8;
    uint16_t buf16;
/*
#if 1
    uint16_t cIndexRx = gSoemConfig.pSlaves[0].pRxPDO[0].cIdx; //All same
    uint16_t cIndexTx = gSoemConfig.pSlaves[0].pTxPDO[0].cIdx; //All same
#else
    uint16_t cIndexRx = KOD_INDEX_1600;
    uint16_t cIndexTx = KOD_INDEX_1A00;
#endif
    LOGD(TAG, "cIndexRx = 0x%04X, cIndexTx = 0x%04X", cIndexRx, cIndexTx);
*/
	
#if 0
    ec_slavet *ec_slave = ecx_context.slavelist;
    ec_slave[1].state = EC_STATE_PRE_OP;
    ec_statecheck(1, EC_STATE_PRE_OP, EC_TIMEOUTSTATE);
#endif
    //log_some_sdo();
AGAIN:
    //1 停止 PDO 分配功能（0x1C12 与 0x1C13 的子索引 0 设置为 0）。
    buf8 = 0;
    ret = ec_SDOwrite(slaveId, KOD_INDEX_1C12, KOD_INDEX_1C12_SUB_0, FALSE, 1, &buf8, EC_TIMEOUTRXM);
    LOGV(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveId, KOD_INDEX_1C12, KOD_INDEX_1C12_SUB_0, ret, 1, (unsigned int)buf8);
    buf8 = 0;
    ret = ec_SDOwrite(slaveId, KOD_INDEX_1C13, KOD_INDEX_1C13_SUB_0, FALSE, 1, &buf8, EC_TIMEOUTRXM);
    LOGD(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveId, KOD_INDEX_1C13, KOD_INDEX_1C13_SUB_0, ret, 1, (unsigned int)buf8);

    //2 停止 PDO 映射功能（0x1600～0x1603 和 0x1A00～0x1A03 的子索引 0 全部设为 0）。
    /*
    buf8 = 0;
    ret = ec_SDOwrite(slaveId, cIndexRx, KOD_INDEX_1600_SUB_0, FALSE, 1, &buf8, EC_TIMEOUTRXM);
    LOGI(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveId, cIndexRx, KOD_INDEX_1600_SUB_0, ret, 1, (unsigned int)buf8);
    buf8 = 0;
    ret = ec_SDOwrite(slaveId, cIndexTx, KOD_INDEX_1A00_SUB_0, FALSE, 1, &buf8, EC_TIMEOUTRXM);
    LOGW(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveId, cIndexTx, KOD_INDEX_1A00_SUB_0, ret, 1, (unsigned int)buf8);
    */

    //3 设置 PDO 映射对象（0x1600～0x1603 和 0x1A00～0x1A03）的映射入口。
    //4 设置 PDO 映射对象（0x1600～0x1603 和 0x1A00～0x1A03）映射入口的数值。
    MiStudio_PDO_Map(slaveId);

    //5 设置 PDO 分配对象（设置 0x1C12 和 0x1C13 的子索引 1）。
		/*
    buf16 = cIndexRx;
    ret = ec_SDOwrite(slaveId, KOD_INDEX_1C12, KOD_INDEX_1C12_SUB_1, FALSE, 2, &buf16, EC_TIMEOUTRXM);
    LOGV(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveId, KOD_INDEX_1C12, KOD_INDEX_1C12_SUB_1, ret, 2, (unsigned int)buf16);
    buf16 = cIndexTx;
    ret = ec_SDOwrite(slaveId, KOD_INDEX_1C13, KOD_INDEX_1C13_SUB_1, FALSE, 2, &buf16, EC_TIMEOUTRXM);
    LOGV(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveId, KOD_INDEX_1C13, KOD_INDEX_1C13_SUB_1, ret, 2, (unsigned int)buf16);
    */
    //6 重新打开 PDO 分配功能（设置 0x1C12 和 0x1C13 的子索引 0 为 1）。
    buf8 = 1;
    ret = ec_SDOwrite(slaveId, KOD_INDEX_1C12, KOD_INDEX_1C12_SUB_0, FALSE, 1, &buf8, EC_TIMEOUTRXM);
    LOGV(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveId, KOD_INDEX_1C12, KOD_INDEX_1C12_SUB_0, ret, 1, (unsigned int)buf8);
    buf8 = 1;
    ret = ec_SDOwrite(slaveId, KOD_INDEX_1C13, KOD_INDEX_1C13_SUB_0, FALSE, 1, &buf8, EC_TIMEOUTRXM);
    LOGD(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x", slaveId, KOD_INDEX_1C13, KOD_INDEX_1C13_SUB_0, ret, 1, (unsigned int)buf8);

    LOGV(TAG, "Leave %s(), slave = %u", __func__, slave);
    return 1;
}

void testReadSDOInProcess(void)
{
    LOGV(TAG, "Enter %s()", __func__);
    uint16_t buf16;
    uint32_t buf32;
    SDO_READ(1, 0x608F, 1, buf32, "0x608F:1");
}

static void logSDORead(void)
{
#if 1
    LOGV(TAG, "Enter %s(), sizeof(ec_mbxheadert) = %u", __func__, sizeof(ec_mbxheadert));
    ec_slavet *ec_slave = ecx_context.slavelist;

    LOGD(TAG, "ec_slave[0].state = %u", ec_slave[0].state);
    LOGV(TAG, "ec_slave[1].state = %u", ec_slave[1].state);
    ec_slave[0].state = EC_STATE_PRE_OP;
    ec_statecheck(0, EC_STATE_PRE_OP, EC_TIMEOUTSTATE);
    ec_statecheck(0, EC_STATE_PRE_OP, EC_TIMEOUTSTATE);
    LOGI(TAG, "ec_slave[0].state = %u", ec_slave[0].state);
    LOGW(TAG, "ec_slave[1].state = %u", ec_slave[1].state);
    while(1)
    {
        /*
            uint16_t slave = 1;
            uint16_t index = 0x431F;
            uint8_t  subindex = 0;
            int16_t valle16 = 0x9999;
              int sizee = 2;
            int ret =  ec_SDOwrite(slave, index, subindex,FALSE, sizee, &valle16, EC_TIMEOUTRXM);
        */

        uint16_t slave = 1;
        uint16_t index = 0x1C12;
        uint8_t  subindex = 1;
        uint16_t valle16 = 0x89AB;
        int sizee = 2;
        int ret = ecx_SDOread(&ecx_context, slave, index, subindex, FALSE, &sizee, &valle16, EC_TIMEOUTRXM);
        //LOGV(TAG, "After ec_SDOread return vaule = %d, Firmware Version(0x4000) = %u ", ret, valle16);
        LOGD(TAG, "Slave: %d - Read at 0x%04x:%d => wkc: %d; data: 0x%.*x (%d)", slave, index, subindex, ret, sizee, (unsigned int)valle16, (unsigned int)valle16);
        LOGV(TAG, "valle16 = %u", valle16);
#if 0
        slave = 2;
        index = 0x0016;
        subindex = 0;
        valle16 = 0x55;
        sizee = 2;
        ec_SDOwrite(slave, index, subindex, FALSE, sizee, &valle16, EC_TIMEOUTRXM);

        ret =  ec_SDOread(slave, index, subindex, FALSE, &sizee, &valle16, EC_TIMEOUTRXM);
        LOGV(TAG, "After return vaule  = %u, 0x1600sub00 = %u ", ret, valle16);
#endif
        vTaskDelay(6000000);
    }
#endif
}

RAMFUNCTION_SECTION_CODE(void kalyke_ethercat_task_do(void *p_arg))
{
    LOGV(TAG, "kalyke_ethercat_task_do RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());
    uint32_t flag = 0;
    TickType_t tickBeginMS;
    TickType_t intervalMS;
    ec_slavet *ec_slave = ecx_context.slavelist;
    while(1)
    {
        if  (ec_group[currentgroup].docheckstate || ec_slave[0].state != EC_STATE_OPERATIONAL)
        {
           LOGW(TAG, "Let us taskday 100ms and continue.");
           vTaskDelay(1000);
           continue;    
        }

        tickBeginMS = xTaskGetTickCount();
        kalyke_soem_process_RxPDO_data();
        ecx_main_send_processdata(&ecx_context, 0, FALSE);
        gWkc = ecx_receive_processdata_group(&ecx_context, 0, EC_TIMEOUTRET3);
#if 0        
        if (gWkc < gExpectedWKC)
        {
            gWkcThresholdCounterSum++;
            gWkcThresholdCounter += 8;
            gtv_PlcElement.msp_SDElement[SD305] = gWkcThresholdCounterSum;
            gtv_PlcElement.msp_SDElement[SD306] = gWkcThresholdCounter;
            LOGE(TAG, "gWkc = %d, gExpectedWKC = %d, gWkcThresholdCounter = %d", gWkc, gExpectedWKC, gWkcThresholdCounter);
            if (gWkcThresholdCounter > SOEM_WKC_THRESHOLD)
            {
                xEventGroupSetBits(g_kalyke_event_group, KALYKE_EVENT_WKC_ERROR);
                vTaskDelay(4000);
                if (GET_SD_ELEMENT_VALUE(SD302) == 1)//如果SD302为1，则主站停机
                {
                    LOGW(TAG, "Let us suspend me : %s", __func__);
                    gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.error_status_stop = 1;//停掉PLC的task
                    vTaskSuspend(gEtherCATTaskHandle);
                }
            }
        }
        else
        {
            if (gWkcThresholdCounter > 0)
            {
                gWkcThresholdCounter--;
                LOGW(TAG, "gWkc = %d, gExpectedWKC = %d, gWkcThresholdCounter = %d", gWkc, gExpectedWKC, gWkcThresholdCounter);
                gtv_PlcElement.msp_SDElement[SD306] = gWkcThresholdCounter;
            }
        }
        kalyke_soem_process_TxPDO_data();

        if (gIfPLCWantProcessSDORead == true)
        {
            int __s = gSOEMMsg.bufSize;
            int __ret = ec_SDOread(gSOEMMsg.slaveID, gSOEMMsg.sdoIndex, gSOEMMsg.sdoSubIndex, FALSE, &__s, &gSOEMMsg.dataVal, EC_TIMEOUTRXM);
            gSOEMMsg.ret = __ret;
            LOGD(TAG, "Slave: %d - Read at 0x%04x:%d => wkc: %d;size: %d, data: 0x%08X (%d)", gSOEMMsg.slaveID, gSOEMMsg.sdoIndex, gSOEMMsg.sdoSubIndex, __ret, __s, gSOEMMsg.dataVal, gSOEMMsg.dataVal);
            gIfPLCWantProcessSDORead = false;
            xQueueSend(gSoemQueueHandle, &gSOEMMsg, 0);
        }
        else if (gIfPLCWantProcessSDOWrite == true)
        {
            int __s = gSOEMMsg.bufSize;
            int __ret = ec_SDOwrite(gSOEMMsg.slaveID, gSOEMMsg.sdoIndex, gSOEMMsg.sdoSubIndex, FALSE, __s, &gSOEMMsg.dataVal, EC_TIMEOUTRXM);
            gSOEMMsg.ret = __ret;
            LOGW(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d;size: %d, data: 0x%08X (%u)", gSOEMMsg.slaveID, gSOEMMsg.sdoIndex, gSOEMMsg.sdoSubIndex, gSOEMMsg.ret, __s, gSOEMMsg.dataVal, gSOEMMsg.dataVal);

            gIfPLCWantProcessSDOWrite = false;
            xQueueSend(gSoemQueueHandle, &gSOEMMsg, 0);
        }
#endif
        if (gWkc >= gExpectedWKC)
        {
             kalyke_soem_process_TxPDO_data();
             if (gIfPLCWantProcessSDORead == true)
             {
                 int __s = gSOEMMsg.bufSize;
                 int __ret = ec_SDOread(gSOEMMsg.slaveID, gSOEMMsg.sdoIndex, gSOEMMsg.sdoSubIndex, FALSE, &__s, &gSOEMMsg.dataVal, EC_TIMEOUTRXM);
                 gSOEMMsg.ret = __ret;
                 LOGD(TAG, "Slave: %d - Read at 0x%04x:%d => wkc: %d;size: %d, data: 0x%08X (%d)", gSOEMMsg.slaveID, gSOEMMsg.sdoIndex, gSOEMMsg.sdoSubIndex, __ret, __s, gSOEMMsg.dataVal, gSOEMMsg.dataVal);
                 gIfPLCWantProcessSDORead = false;
                 xQueueSend(gSoemQueueHandle, &gSOEMMsg, 0);
              }
             else if (gIfPLCWantProcessSDOWrite == true)
             {
                 int __s = gSOEMMsg.bufSize;
                 int __ret = ec_SDOwrite(gSOEMMsg.slaveID, gSOEMMsg.sdoIndex, gSOEMMsg.sdoSubIndex, FALSE, __s, &gSOEMMsg.dataVal, EC_TIMEOUTRXM);
                 gSOEMMsg.ret = __ret;
                 LOGW(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d;size: %d, data: 0x%08X (%u)", gSOEMMsg.slaveID, gSOEMMsg.sdoIndex, gSOEMMsg.sdoSubIndex, gSOEMMsg.ret, __s, gSOEMMsg.dataVal, gSOEMMsg.dataVal);

                 gIfPLCWantProcessSDOWrite = false;
                 xQueueSend(gSoemQueueHandle, &gSOEMMsg, 0);
             }

             if (gWkcThresholdCounter > 0)
             {
                 gWkcThresholdCounter--;
//                 LOGW(TAG, "gWkc = %d, gExpectedWKC = %d, gWkcThresholdCounter = %d", gWkc, gExpectedWKC, gWkcThresholdCounter);
                 gtv_PlcElement.msp_SDElement[SD306] = gWkcThresholdCounter;
             }
        }
        else
        {
            gWkcThresholdCounterSum++;
            gWkcThresholdCounter++;
            gtv_PlcElement.msp_SDElement[SD305] = gWkcThresholdCounterSum;
            gtv_PlcElement.msp_SDElement[SD306] = gWkcThresholdCounter;
            if (gWkcThresholdCounter > SOEM_WKC_THRESHOLD)
            {
                LOGE(TAG, "My god!!! gWkcThresholdCounter = %d, begin kalyke_ethercat_check!!!", gWkcThresholdCounter);
                xEventGroupSetBits(g_kalyke_event_group, KALYKE_EVENT_WKC_ERROR);
                vTaskDelay(2000);
                if (GET_SD_ELEMENT_VALUE(SD302) == 1)//如果SD302为1，则主站停机
                {
                    LOGW(TAG, "Let us suspend me : %s", __func__);
                    gtv_PlcRunStatus.mtv_PlcRunStopFlag.bit.error_status_stop = 1;//停掉PLC的task
                    vTaskSuspend(gEtherCATTaskHandle);
                }
            }
         }

        intervalMS = xTaskGetTickCount() - tickBeginMS;
        
        if (intervalMS < GET_SD_ELEMENT_VALUE(SD300))
        {
            vTaskDelay(GET_SD_ELEMENT_VALUE(SD300) - intervalMS);
        }
        else
        {
            vTaskDelay(1);
        }
    }
}

void kalyke_set_ec_configlist_for_FR8200(ec_slavet *ec_slaveList, int theSlave, ec_configlist_t *pConfigListItem)
{
    LOGV(TAG, "Enter %s(), theSlave = %d", __func__, theSlave);

    pConfigListItem->man = ec_slaveList[theSlave].eep_man;
    pConfigListItem->id = ec_slaveList[theSlave].eep_id;
    strncpy(pConfigListItem->name, ec_slaveList[theSlave].name, EC_MAXNAME);
    pConfigListItem->Dtype = 3;
    pConfigListItem->Obits = gSoemConfig.pSlaves[theSlave - 1].outSize;
    pConfigListItem->Ibits = gSoemConfig.pSlaves[theSlave - 1].inSize;

    LOGV(TAG, "pConfigListItem->Obits = %u", pConfigListItem->Obits);
    LOGV(TAG, "pConfigListItem->Ibits = %u", pConfigListItem->Ibits);
    LOGD(TAG, "SM[2].StartAddr=%4.4x", ec_slaveList[theSlave].SM[2].StartAddr);
    LOGD(TAG, "SM[2].SMflags = %8.8x", ec_slaveList[theSlave].SM[2].SMflags);
    LOGI(TAG, "SM[3].StartAddr=%4.4x", ec_slaveList[theSlave].SM[3].StartAddr);
    LOGI(TAG, "SM[3].SMflags = %8.8x", ec_slaveList[theSlave].SM[3].SMflags);
    pConfigListItem->SM2a = ec_slaveList[theSlave].SM[2].StartAddr;
    pConfigListItem->SM2f = ec_slaveList[theSlave].SM[2].SMflags;
    pConfigListItem->SM3a = ec_slaveList[theSlave].SM[3].StartAddr;
    pConfigListItem->SM3f = ec_slaveList[theSlave].SM[3].SMflags;

    pConfigListItem->FM0ac = 0;
    pConfigListItem->FM1ac = 0;
}

static bool ifHave_FR8200(void)
{
    ec_slavet *ec_slave = ecx_context.slavelist;
    for (int i = 1; i <= ec_slavecount; i++)
    {
        if (memcmp(ec_slave[i].name, "FR8200", 6) == 0)
        {
            return true;
        }
    }
    return false;
}

static bool ifHaveNotOnly_LXM28E(void)
{
    ec_slavet *ec_slave = ecx_context.slavelist;
    for (int i = 1; i <= ec_slavecount; i++)
    {
        if (memcmp(ec_slave[i].name, "LXM28E", 6) != 0)
        {
            return true;
        }
    }
    return false;
}

void kalyke_ethercat_task(void *p_arg)
{
    LOGV(TAG, "kalyke_ethercat_task RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());
    xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_ENET_INIT_DONE_PLC, pdTRUE, pdFALSE, portMAX_DELAY);
    LOGD(TAG, "EtherCAT begin......SD300 = %u, SD301 = %u", GET_SD_ELEMENT_VALUE(SD300), GET_SD_ELEMENT_VALUE(SD301));
    vTaskDelay(GET_SD_ELEMENT_VALUE(SD301));
    logSomeSize();
    char *ifname = "kalyke_if";
    int cnt, i;

    //SET_SD_ELEMENT_VALUE(SD300, 10000);
#ifdef SOEM_SLAVE_USE_POINTER
    //ecx_context.slavelist = pvPortMalloc(128 * sizeof(ec_slavet));
    ecx_context.slavelist = pvPortMalloc((3 + gSoemConfig.nSubStationCount) * sizeof(ec_slavet));
    ecx_context.maxslave = (3 + gSoemConfig.nSubStationCount);
    memset(ecx_context.slavelist, 0x00, sizeof(ec_slavet) * ecx_context.maxslave);
    ec_slavet *ec_slave = ecx_context.slavelist;
#if 0
    for (int k = 1; k < ecx_context.maxslave; k++)
    {
        ec_slave[k].PO2SOconfigx = lxm28e_PO2SOconfigx;
    }
#endif
    ecx_context.port = pvPortMalloc(sizeof(ecx_portt));
    LOGD(TAG, "ecx_context.port = 0x%08X", ecx_context.port);
#endif
    /* initialise SOEM */
    LOGV(TAG, "Begin initialise SOEM...");
//    print_vTaskList();

ECINIT:
    if (!ec_init(ifname))
    {
        LOGE(TAG, "initialise SOEM error!");
        vTaskDelay(1500);
        goto ECINIT;

    }
    LOGV(TAG, "ec_init succeeded.");

    //    ec_configdc();
    /* find and auto-config slaves */
ECCONFIGINIT:
    if ( ec_config_init(FALSE) <= 0 )
    {
        LOGE(TAG, "No slaves found!");
        if (gSoemConfig.nSubStationCount != 0)
        {
           plc_refresh_error_msg(ERR_ECAT_SLAVE_NUM_ERR);
           guv_NonStopError.bit.ecat_err = 1;
        }
        vTaskDelay(2501);
        goto ECCONFIGINIT;

    }
    LOGD(TAG, "%d slaves found and configured.", ec_slavecount);
    SET_SD_ELEMENT_VALUE(SD320, ec_slavecount);

    if (gSoemConfig.nSubStationCount != ec_slavecount)
    {
        plc_refresh_error_msg(ERR_ECAT_SLAVE_NUM_ERR);
        guv_NonStopError.bit.ecat_err = 1;
        LOGE(TAG, "%d slaves found, but gSoemConfig.nSubStationCount = %u...", ec_slavecount, gSoemConfig.nSubStationCount);
        vTaskDelay(2502);
        goto ECCONFIGINIT;
    }

    //如果有非LXM28E的从站，则需再次调用ec_config_init，并且参数为TRUE
    if (ifHaveNotOnly_LXM28E() == true)
    {
        LOGV(TAG, "We have not only LXM28E, so let us do 'ec_config_init(TRUE)'");
        if ( ec_config_init(TRUE) <= 0 )
        {
            LOGE(TAG, "No slaves found!");
            vTaskDelay(2501);
            goto ECCONFIGINIT;
        }
    }

#if 0
    for (int i=1; i<=ec_slavecount; i++) 
    {
        LOGV(TAG, "Slave %d has CA? %s\n", i, ec_slave[i].CoEdetails & ECT_COEDET_SDOCA ? "true":"false" );

        /** CompleteAccess disabled for Elmo driver */
        ec_slave[i].CoEdetails ^= ECT_COEDET_SDOCA;
    }
#endif

    /* SDO and PDO set begin */
    for (i = 1; i <= ec_slavecount; i++)
    {
        LOGV(TAG, "SDO and PDO setting.....slave[%d].name = [%s].",i,ec_slave[i].name);
#if 0
        if (memcmp(ec_slave[i].name, "FR8200", 6) == 0 || 
             memcmp(ec_slave[i].name, "AMX EC2-IO16T", 13) == 0 || 
             memcmp(ec_slave[i].name, "AMX EC2-IO8R-A", 14) == 0 )
        {
            kalyke_RemotIO_SDO_SET(i);
        }
        else
        {
            kalyke_Drive_SDO_SET_PDO_map(i); 
        }
#else
        kalyke_Drive_SDO_SET_PDO_map(i);
#endif
        bsp_feed_watch_dog();
    }
    

    /* Run IO mapping */
    ec_config_map(&IOmap);
//    ec_configdc();
    LOGV(TAG, "Slaves mapped, state to SAFE_OP.");
    ec_slave[0].state = EC_STATE_SAFE_OP;
    /* wait for all slaves to reach SAFE_OP state */
    ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE*4);
    LOGD(TAG, "ec_slave[0].state = 0x%X", ec_slave[0].state);
    LOGV(TAG, "ec_slave[1].state = 0x%X", ec_slave[1].state);
    /* Print som information on the mapped network */
    for( cnt = 0 ; cnt <= ec_slavecount ; cnt++)
    {
        log_slaves(cnt);
    }
    LOGD(TAG, "outputsWKC = %u, inputsWKC = %u", ec_group[0].outputsWKC, ec_group[0].inputsWKC);
    gExpectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
    LOGD(TAG, "Calculated workcounter %d", gExpectedWKC);
    //logSDORead();

    LOGV(TAG, "Request operational state for all slaves");
    ec_slave[0].state = EC_STATE_OPERATIONAL;
    /* send one valid process data to make outputs in slaves happy*/
    ec_send_processdata();
    ec_receive_processdata(EC_TIMEOUTRET);
    /* request OP state for all slaves */
    ec_writestate(0);
    /* wait for all slaves to reach OP state */
    ec_statecheck(0, EC_STATE_OPERATIONAL,  EC_TIMEOUTSTATE);
    ec_statecheck(1, EC_STATE_OPERATIONAL,  EC_TIMEOUTSTATE);
    LOGI(TAG, "ec_slave[0].state = 0x%X...", ec_slave[0].state);
    LOGW(TAG, "ec_slave[1].state = 0x%X...", ec_slave[1].state);
    if (ec_slave[0].state == EC_STATE_OPERATIONAL )
    {
        LOGD(TAG, "Operational state reached for all slaves.");
        ec_readstate();
        for(i = 1; i <= ec_slavecount ; i++)
        {
            SET_SD_ELEMENT_VALUE(SD320+i, ec_slave[i].state);
        }
    }
    else
    {
        while (1)
        {
            LOGV(TAG, "Not all slaves reached operational state.");
            ec_readstate();
            for(i = 1; i <= ec_slavecount ; i++)
            {
                SET_SD_ELEMENT_VALUE(SD320+i, ec_slave[i].state);
                if(ec_slave[i].state != EC_STATE_OPERATIONAL)
                {
                    LOGV(TAG, "Slave %d State=0x%04x StatusCode=0x%04x", i, ec_slave[i].state, ec_slave[i].ALstatuscode);
                }
            }
            plc_refresh_error_msg(ERR_ECAT_NOT_OPMODE);
            guv_NonStopError.bit.ecat_err = 1;
            vTaskDelay(3000);
        }
    }

    guv_NonStopError.bit.ecat_err = 0;
    gSOEMInProcess = true;

    BaseType_t ret = xTaskCreate((TaskFunction_t)kalyke_ethercat_task_do,
                                 (const char *)"cat_task_do",
                                 512,
                                 (void *)NULL,
                                 SOEM_TASK_PRIO,
                                 (TaskHandle_t *)&gEtherCATTaskHandle);
    if (ret != pdPASS)
    {
        LOGE(TAG, "Create kalyke_ethercat_task_do error!\r\n");
    }
    ret = xTaskCreate((TaskFunction_t)kalyke_ethercat_check,
                      (const char *)"cat_check_task",
                      512,
                      (void *)NULL,
                      SOEM_TASK_PRIO,
                      (TaskHandle_t *)&gEtherCATCheckTaskHandle);
    if (ret != pdPASS)
    {
        LOGE(TAG, "Create kalyke_ethercat_check error!\r\n");
    }
    vTaskDelete(NULL);
}

static void kalyke_ethercat_check(void *p_arg)
{
    LOGV(TAG, "kalyke_ethercat_check RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());
    int slave;
    vTaskDelay(1000);
    ec_slavet *ec_slave = ecx_context.slavelist;
    while(1)
    {
        xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_WKC_ERROR, pdTRUE, pdFALSE, portMAX_DELAY);
        if( ((gWkc < gExpectedWKC) || ec_group[currentgroup].docheckstate) )
        {
            LOGI(TAG, "OH, gWkc < gExpectedWKC || ec_group[currentgroup].docheckstate !\r\n");
            /* one ore more slaves are not responding */
            ec_group[currentgroup].docheckstate = FALSE;
            ec_readstate();
            for (slave = 1; slave <= ec_slavecount; slave++)
            {
                SET_SD_ELEMENT_VALUE(SD320+slave, ec_slave[slave].state);
                LOGV(TAG, "MESSAGE :ec_slave[%d].state = 0x%02X, group = %u", slave, ec_slave[slave].state, ec_slave[slave].group);
                if ((ec_slave[slave].group == currentgroup) && (ec_slave[slave].state != EC_STATE_OPERATIONAL))
                {
                    ec_group[currentgroup].docheckstate = TRUE;
                    if (ec_slave[slave].state == (EC_STATE_SAFE_OP + EC_STATE_ERROR))
                    {
                        LOGE(TAG, "MESSAGE :ERROR : slave %d is in SAFE_OP + ERROR, attempting ack.\n", slave);
                        plc_refresh_error_msg(ERR_ECAT_NOT_OPMODE);
                        guv_NonStopError.bit.ecat_err = 1;
                        ec_slave[slave].state = (EC_STATE_SAFE_OP + EC_STATE_ACK);
                        ec_writestate(slave);
                    }
                    else if(ec_slave[slave].state == EC_STATE_SAFE_OP)
                    {
                        LOGE(TAG, "MESSAGE :WARNING : slave %d is in SAFE_OP, change to OPERATIONAL.\n", slave);
                        ec_slave[slave].state = EC_STATE_OPERATIONAL;
                        ec_writestate(slave);
                    }
                    else if(ec_slave[slave].state > EC_STATE_NONE)
                    {
                        #if 1
                        int ret = ec_reconfig_slave(slave, EC_TIMEOUTRET3);
                        LOGD(TAG, "MESSAGE :ec_reconfig_slave return %d", ret);
                        #else
                        int ret = ec_config_init(FALSE);
                        LOGD(TAG, "ec_config_init return %d", ret);
                        #endif
                        if (ret)
                        {
                            ec_slave[slave].islost = FALSE;
                            LOGV(TAG, "MESSAGE : slave %d reconfigured\n", slave);
                        }
                    }
                    else if(!ec_slave[slave].islost)
                    {
                        /* re-check state */
                        ec_statecheck(slave, EC_STATE_OPERATIONAL, EC_TIMEOUTRET*4);
                        if (ec_slave[slave].state == EC_STATE_NONE)
                        {
                            ec_slave[slave].islost = TRUE;
                            LOGE(TAG, "MESSAGE :ERROR : slave %d lost\n", slave);
                            plc_refresh_error_msg(ERR_ECAT_WKC_ERR);
                            guv_NonStopError.bit.ecat_err = 1;
                        }
                    }
                }
                LOGD(TAG, "MESSAGE :ec_slave[%d].islost = %u", slave, ec_slave[slave].islost);
                if (ec_slave[slave].islost)
                {
                    LOGV(TAG, "MESSAGE :ec_slave[%d].state = %u", slave, ec_slave[slave].state);
                    if(ec_slave[slave].state == EC_STATE_NONE)
                    {
                        if (ec_recover_slave(slave, EC_TIMEOUTRET3))
                        {
                            ec_slave[slave].islost = FALSE;
                            LOGV(TAG, "MESSAGE : slave %d recovered\n", slave);
                        }
                    }
                    else
                    {
                        bsp_close_err_led();
                        ec_slave[slave].islost = FALSE;
                        LOGD(TAG, "MESSAGE : slave %d found\n", slave);
                        guv_NonStopError.bit.ecat_err = 0;
                        if (ec_recover_slave(slave, EC_TIMEOUTRET3))
                        {
                            ec_slave[slave].islost = FALSE;
                            LOGV(TAG, "MESSAGE : slave %d recovered\n", slave);
                        }

                        uint16 theState1 = ec_statecheck(slave, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE);
                        LOGV(TAG, "MESSAGE : theState1 = %x", theState1);

                    #if 0
                        ec_slave[slave].state = EC_STATE_SAFE_OP;
                        /* wait for all slaves to reach SAFE_OP state */
                        ec_statecheck(slave, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE*4);
                    #endif
                       if(theState1 == EC_STATE_SAFE_OP)
                       {
                            ec_slave[slave].state = EC_STATE_OPERATIONAL;
                        #if 0
                            /* send one valid process data to make outputs in slaves happy*/
                            ec_send_processdata();
                            ec_receive_processdata(EC_TIMEOUTRET);
                        #endif
                            ec_writestate(slave);
                        }
                        uint16 theState2 = ec_statecheck(slave, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE);
                        LOGD(TAG, "MESSAGE : theState2 = %x", theState2);
                    }
                }
                else
                {
                #if 0
                    ec_slave[slave].state = EC_STATE_OPERATIONAL;
                    #if 0
                    /* send one valid process data to make outputs in slaves happy*/
                    ec_send_processdata();
                    ec_receive_processdata(EC_TIMEOUTRET);
                    #endif
                    ec_writestate(slave);
                    uint16 theState3 = ec_statecheck(slave, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE);
                    LOGD(TAG, "MESSAGE : theState3 = %x", theState3);
                #endif
                }
            }
            if(!ec_group[currentgroup].docheckstate)
            {
                LOGI(TAG, "MESSAGE :OK : all slaves resumed OPERATIONAL.\n");
                guv_NonStopError.bit.ecat_err = 0;
                bsp_close_err_led();

            }
        }
        else
        {
            guv_NonStopError.bit.ecat_err = 0;
            bsp_close_err_led();
        }
#if 0
        ec_readstate();
        for (slave = 1; slave <= ec_slavecount; slave++)
        {
             SET_SD_ELEMENT_VALUE(SD320+slave, ec_slave[slave].state);
        }
 #endif
        vTaskDelay(2000);
        if (ec_group[currentgroup].docheckstate)
        {
             xEventGroupSetBits(g_kalyke_event_group, KALYKE_EVENT_WKC_ERROR);
             LOGW(TAG, "MESSAGE :WARNING : ec_group[currentgroup].docheckstate is TRUE ,  recover slave again.\n", slave);	
        }
    }
}

#if 0
void kalyke_ethercat_task_test(void *p_arg)
{
    LOGV(TAG, "kalyke_ethercat_task_test RUN. Free heap size is %d bytes", xPortGetFreeHeapSize());
    xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_ENET_INIT_DONE_PLC, pdTRUE, pdFALSE, portMAX_DELAY);
    vTaskDelay(2000);

    char *ifname = "kalyke_if";
    int cnt, i, j;

    //*pPORTFIO_DIR |= BIT (6);

    /* initialise SOEM */
    LOGV(TAG, "Begin initialise SOEM...");
    if (!ec_init(ifname))
    {
        while(1)
        {
            LOGE(TAG, "initialise SOEM error!");
            vTaskDelay(1500);
        }
    }
    LOGV(TAG, "ec_init succeeded.");

    /* find and auto-config slaves */
    if ( ec_config_init(FALSE) <= 0 )
    {
        while (1)
        {
            LOGE(TAG, "No slaves found!");
            vTaskDelay(1501);
        }
    }

    LOGD(TAG, "%d slaves found and configured.", ec_slavecount);

    /* Check network  setup */
    if (!network_configuration())
    {
        while(1)
        {
            LOGE(TAG, "Mismatch of network units!");
            vTaskDelay(1502);
        }
    }

    /* Run IO mapping */
    ec_config_map(&IOmap);

    LOGV(TAG, "Slaves mapped, state to SAFE_OP.");
    /* wait for all slaves to reach SAFE_OP state */
    ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE);

    /* Print som information on the mapped network */
    for( cnt = 1 ; cnt <= ec_slavecount ; cnt++)
    {
        LOGV(TAG, "Slave:%d Name:%s Output size: %dbits Input size: %dbits State: %d Delay: %d[ns] Has DC: %d",
             cnt, ec_slave[cnt].name, ec_slave[cnt].Obits, ec_slave[cnt].Ibits,
             ec_slave[cnt].state, ec_slave[cnt].pdelay, ec_slave[cnt].hasdc);
        LOGV(TAG, " Configured address: %x", ec_slave[cnt].configadr);
        LOGV(TAG, " Outputs address: %x", ec_slave[cnt].outputs);
        LOGV(TAG, " Inputs address: %x", ec_slave[cnt].inputs);

        for(j = 0 ; j < ec_slave[cnt].FMMUunused ; j++)
        {
            LOGV(TAG, " FMMU%1d Ls:%x Ll:%4d Lsb:%d Leb:%d Ps:%x Psb:%d Ty:%x Act:%x", j,
                 (int)ec_slave[cnt].FMMU[j].LogStart, ec_slave[cnt].FMMU[j].LogLength, ec_slave[cnt].FMMU[j].LogStartbit,
                 ec_slave[cnt].FMMU[j].LogEndbit, ec_slave[cnt].FMMU[j].PhysStart, ec_slave[cnt].FMMU[j].PhysStartBit,
                 ec_slave[cnt].FMMU[j].FMMUtype, ec_slave[cnt].FMMU[j].FMMUactive);
        }
        LOGV(TAG, " FMMUfunc 0:%d 1:%d 2:%d 3:%d\n",
             ec_slave[cnt].FMMU0func, ec_slave[cnt].FMMU1func, ec_slave[cnt].FMMU2func, ec_slave[cnt].FMMU3func);

    }

    LOGV(TAG, "Request operational state for all slaves");
    ec_slave[0].state = EC_STATE_OPERATIONAL;
    /* send one valid process data to make outputs in slaves happy*/
    ec_send_processdata();
    ec_receive_processdata(EC_TIMEOUTRET);
    /* request OP state for all slaves */
    ec_writestate(0);
    /* wait for all slaves to reach OP state */
    ec_statecheck(0, EC_STATE_OPERATIONAL,  EC_TIMEOUTSTATE);
    if (ec_slave[0].state == EC_STATE_OPERATIONAL )
    {
        LOGD(TAG, "Operational state reached for all slaves.");
    }
    else
    {
        LOGV(TAG, "Not all slaves reached operational state.");
        ec_readstate();
        for(i = 1; i <= ec_slavecount ; i++)
        {
            if(ec_slave[i].state != EC_STATE_OPERATIONAL)
            {
                LOGV(TAG, "Slave %d State=0x%04x StatusCode=0x%04x", i, ec_slave[i].state, ec_slave[i].ALstatuscode);
            }
        }
    }

    /* Simple blinking lamps BOX demo */
    uint8 digout = 0;

    slave_EL4001_1.out1 = (int16)0x3FFF;
    set_output_int16(EL4001_1, 0, slave_EL4001_1.out1);

#if 0 //LiXianyu 20200730
    task_spawn("t_StatsPrint", my_cyclic_callback, 20, 1024, (void *)NULL);
    tt_start_wait (tt_sched[0]);
#else //TODO:

#endif

    while(1)
    {
        dorun = 0;
        slave_EL1008_1.in1 = get_input_bit(EL1008_1, 1); // Start button
        slave_EL1008_1.in2 = get_input_bit(EL1008_1, 2); // Turnkey RIGHT
        slave_EL1008_1.in3 = get_input_bit(EL1008_1, 3); // Turnkey LEFT

        /* (Turnkey MIDDLE + Start button) OR Turnkey RIGHT OR Turnkey LEFT
           Turnkey LEFT: Light positions bottom to top. Loop, slow operation.
           Turnkey MIDDLE: Press start button to light positions bottom to top. No loop, fast operation.
           Turnkey RIGHT: Light positions bottom to top. Loop, fast operation.
        */
        if (slave_EL1008_1.in1 || slave_EL1008_1.in2 || slave_EL1008_1.in3)
        {
            digout = 0;
            /* *ec_slave[6].outputs = digout; */
            /* set_output_bit(slave_name #,index as 1 output on module , value */
            set_output_bit(EL2622_1, 1, (digout & BIT (0))); /* Start button */
            set_output_bit(EL2622_1, 2, (digout & BIT (1))); /* Turnkey RIGHT */
            set_output_bit(EL2622_2, 1, (digout & BIT (2))); /* Turnkey LEFT */
            set_output_bit(EL2622_2, 2, (digout & BIT (3)));
            set_output_bit(EL2622_3, 1, (digout & BIT (4)));
            set_output_bit(EL2622_3, 2, (digout & BIT (5)));

            while(dorun < 95)
            {
                dorun++;

                if (slave_EL1008_1.in3)
                {
                    //task_delay(tick_from_ms(20));
                    vTaskDelay(20);
                }
                else
                {
                    //task_delay(tick_from_ms(5));
                    vTaskDelay(5);
                }

                digout = (uint8) (digout | BIT((dorun / 16) & 0xFF));

                set_output_bit(EL2622_1, 1, (digout & BIT (0))); /* LED1 */
                set_output_bit(EL2622_1, 2, (digout & BIT (1))); /* LED2 */
                set_output_bit(EL2622_2, 1, (digout & BIT (2))); /* LED3 */
                set_output_bit(EL2622_2, 2, (digout & BIT (3))); /* LED4 */
                set_output_bit(EL2622_3, 1, (digout & BIT (4))); /* LED5 */
                set_output_bit(EL2622_3, 2, (digout & BIT (5))); /* LED6 */

                slave_EL1008_1.in1 = get_input_bit(EL1008_1, 2); /* Turnkey RIGHT */
                slave_EL1008_1.in2 = get_input_bit(EL1008_1, 3); /* Turnkey LEFT */
                slave_EL3061_1.in1 = get_input_int32(EL3061_1, 0); /* Read AI */
            }
        }
        //task_delay(tick_from_ms(2));
        vTaskDelay(2);
    }
}
#endif

static void log_pbuf(struct pbuf *p)
{
    LOGD(TAG, "Enter %s(), p = 0x%08X", __func__, p);
    LOGV(TAG, "p->next = 0x%08X", p->next);
    LOGD(TAG, "p->payload = 0x%08X", p->payload);
    LOGV(TAG, "p->tot_len = %u, p->len = %u", p->tot_len, p->len);
    //hexdump(p->payload, p->len);
    LOGD(TAG, "p->type_internal = 0x%X", p->type_internal);
    LOGD(TAG, "p->flags = 0x%X", p->flags);
    LOGV(TAG, "p->ref = %u", p->ref);
    LOGD(TAG, "p->if_idx = %u", p->if_idx);
}

#endif /* ETHERCAT_SOEM == 1 */

//This funciton called in interrupt, so we can't call FreeRTOS API.
#if (ETHERCAT_SOEM == 1)
RAMFUNCTION_SECTION_CODE(err_t soem_ethernet_input(struct pbuf *p, struct netif *netif))
{
    //LOGW(TAG, "Enter %s(), p = 0x%08X, netif = 0x%08X", __func__, p, netif);
    /* points to packet payload, which starts with an Ethernet header */
    struct eth_hdr *ethhdr = (struct eth_hdr *)p->payload;

    //LOGW(TAG, "lwip_htons(ethhdr->type) = 0x%04X, ethhdr->type = 0x%04X, ETHTYPE_ETHERCAT = 0x%04X", lwip_htons(ethhdr->type), ethhdr->type, ETHTYPE_ETHERCAT);
    if (ETHTYPE_ETHERCAT != lwip_htons(ethhdr->type))
        //if (ETHTYPE_ETHERCAT != ethhdr->type)
    {
        //LOGE(TAG, "lwip_htons(ethhdr->type) = 0x%04X, ethhdr->type = 0x%04X", lwip_htons(ethhdr->type), ethhdr->type);
        return ERR_RTE;
    }
#if 0
    LOGW(TAG, "soem_ethernet_input: dest:%"X8_F":%"X8_F":%"X8_F":%"X8_F":%"X8_F":%"X8_F", src:%"X8_F":%"X8_F":%"X8_F":%"X8_F":%"X8_F":%"X8_F", type:%"X16_F"\n",
         (unsigned char)ethhdr->dest.addr[0], (unsigned char)ethhdr->dest.addr[1], (unsigned char)ethhdr->dest.addr[2],
         (unsigned char)ethhdr->dest.addr[3], (unsigned char)ethhdr->dest.addr[4], (unsigned char)ethhdr->dest.addr[5],
         (unsigned char)ethhdr->src.addr[0],  (unsigned char)ethhdr->src.addr[1],  (unsigned char)ethhdr->src.addr[2],
         (unsigned char)ethhdr->src.addr[3],  (unsigned char)ethhdr->src.addr[4],  (unsigned char)ethhdr->src.addr[5],
         lwip_htons(ethhdr->type));
    //log_pbuf(p);
    hexdump(p->payload, p->len);
#endif
    if (netif == (&fsl_netif1))
    {
#if 0
        gSOEMRecvLen = p->len - SIZEOF_ETH_HDR;
        memcpy(gSOEMRecvBuffer, (uint8_t *)p->payload + SIZEOF_ETH_HDR, gSOEMRecvLen);
#else
        gSOEMRecvLen = p->len;
        memcpy(gSOEMRecvBuffer, (uint8_t *)p->payload, gSOEMRecvLen);
#endif
        pbuf_free(p);
        notify_handle_data_recv_task();
        return ERR_OK;
    }
    //pbuf_free(p);
    //pbuf_free_callback(p);
    return ERR_RTE;
}


/* 因为我是主站，我总是发往下一级 */
RAMFUNCTION_SECTION_CODE(void soem_LAN_send(uint8_t *pBuf, uint16_t len))
{
#ifdef SOEM_SEND_RECV_HEXDUMP
    LOGV(TAG, "Enter %s()", __func__);
    hexdump(pBuf, len);
#endif
    struct pbuf *p = pbuf_alloc(PBUF_LINK, len, PBUF_RAM);
    if (p && p->payload)
    {
        memcpy(p->payload, pBuf, len);
    }
    else
    {
        pbuf_free(p);
        //pbuf_free_callback(p);
        LOGE(TAG, "%s: ERROR, p = 0x%08X, p->payload = 0x%08X", __func__, p, p->payload);
        return;
    }

    //err_t ret = ethernet_output(&fsl_netif1, p, (struct eth_addr *)(fsl_netif1.hwaddr), (struct eth_addr *)(gEthBroadcast), ETHTYPE_ETHERCAT);
    ethernet_output(&fsl_netif1, p, (struct eth_addr *)(fsl_netif1.hwaddr), (struct eth_addr *)(&ethbroadcast), ETHTYPE_ETHERCAT);

    pbuf_free(p);
    //pbuf_free_callback(p);

    //LOGD(TAG, "Leave %s(), ret = %d", __func__, ret);
}

void kalyke_slave_scan(md_slave_msg_pack *pMsg)
{
    LOGV(TAG, "Enter %s(), pMsg = 0x%08X", __func__, pMsg);
    int i;
    for(i = 0; i < 3; i++)
    {
        pMsg->mcp_RespBuff[i] = pMsg->mcp_ReceiveBuff[i];
    }

    uint16_t nb = (uint16_t)ec_slavecount; //从站个数
    uint16_t idBytesLen = nb * 2;

    //nb += 1; //从站个数 + 1
    LOGI(TAG, "idBytesLen = %u, nb = %u", idBytesLen, nb);
    pMsg->mcp_RespBuff[i++] = (uint8_t)(nb & 0xFF);
    pMsg->mcp_RespBuff[i++] = (uint8_t)(nb >> 8);

    ec_slavet *ec_slave = ecx_context.slavelist;
    for (uint8_t j = 1; j <= ec_slavecount; j++)
    {
        pMsg->mcp_RespBuff[i++] = (uint8_t)(j & 0xFF);
        pMsg->mcp_RespBuff[i++] = (uint8_t)(j >> 8);
        pMsg->mcp_RespBuff[i++] = (uint8_t)(ec_slave[j].eep_man &  0x000000FFU);
        pMsg->mcp_RespBuff[i++] = (uint8_t)((ec_slave[j].eep_man & 0x0000FF00U) >> 8);
        pMsg->mcp_RespBuff[i++] = (uint8_t)((ec_slave[j].eep_man & 0x00FF0000U) >> 16);
        pMsg->mcp_RespBuff[i++] = (uint8_t)((ec_slave[j].eep_man & 0xFF000000U) >> 24);
        pMsg->mcp_RespBuff[i++] = (uint8_t)(ec_slave[j].eep_id &  0x000000FFU);
        pMsg->mcp_RespBuff[i++] = (uint8_t)((ec_slave[j].eep_id & 0x0000FF00U) >> 8);
        pMsg->mcp_RespBuff[i++] = (uint8_t)((ec_slave[j].eep_id & 0x00FF0000U) >> 16);
        pMsg->mcp_RespBuff[i++] = (uint8_t)((ec_slave[j].eep_id & 0xFF000000U) >> 24);
    }

    pMsg->msv_RespLen = i;
    mb_slave_verify_resp_msg(pMsg);
    hexdump(pMsg->mcp_RespBuff, pMsg->msv_RespLen);
    LOGD(TAG, "Leave %s()", __func__);
}

void kalyke_start_EtherCAT(void)
{
    LOGV(TAG, "Enter %s(), g_plc_netcfg.lan.ioExp = %u", __func__, g_plc_netcfg.lan.ioExp);
    if (g_plc_netcfg.lan.ioExp != LAN_CONFIG_IO_EXP_ETHERCAT)
    {
        return;
    }

    gSoemQueueHandle = xQueueCreate(2, sizeof(soem_msg_st));
    gSOEMInProcess = false;
    gWkcThresholdCounterSum = 0;
    gWkcThresholdCounter = 0;
    BaseType_t ret = xTaskCreate((TaskFunction_t)kalyke_ethercat_task,
                                 (const char *)"cat_task",
                                 SOEM_TASK_STACK_SIZE,
                                 (void *)NULL,
                                 5,
                                 NULL);
    if (ret != pdPASS)
    {
        LOGE(TAG, "Create kalyke_ethercat_task error!\r\n");
    }
    ret = xTaskCreate((TaskFunction_t)handle_recv_data_ethercat_task,
                      (const char *)"cat_recv_task",
                      SOEM_DATA_TASK_STACK_SIZE,
                      (void *)NULL,
                      SOEM_DATA_TASK_PRIO,
                      (TaskHandle_t *)&gRecvDataTaskHandle);
    if (ret != pdPASS)
    {
        LOGE(TAG, "Create handle_recv_data_ethercat_task error!\r\n");
    }
}
#else
err_t soem_ethernet_input(struct pbuf *p, struct netif *netif)
{
    LOGV("EtherCATTask", "Enter %s()", __func__);
    return ERR_RTE;
}

void kalyke_start_EtherCAT(void)
{
    //configASSERT(0);
    LOGE("EtherCATTask", "Please Set ETHERCAT_SOEM to 1");
}

void soem_LAN_send(uint8_t *pBuf, uint16_t len)
{
}

void kalyke_set_ec_configlist_for_FR8200(ec_slavet *ec_slaveList, int theSlave, ec_configlist_t *pConfigListItem)
{
}
#endif

