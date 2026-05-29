/**
  ******************************************************************************
  * @file    plc_io_interrupt.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2020-01-13
  * @brief   
  ******************************************************************************
  */
#include "fsl_debug_console.h"
#include "plc_io_interrupt.h"
#include "plc_variable.h"
#include "plc_parseaddr.h"
#include "plc_highspeedins.h"
#include "plc_interrupt.h"
#include "bsp_gpio.h"
#include "plc_spd.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/


/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Variables
 ******************************************************************************/
static const char *TAG = "IO_INTERRUPT";


/*******************************************************************************
 * Code
 ******************************************************************************/
/*
 @retval true:增计数
         false:减计数
 */
static bool isUpperCounter(uint32_t counterNum)
{
    return (plc_get_bit_element_value(SM_ELEMENT, counterNum)==1 ? false : true);
}

/*
 @param mode true = 增计数; false = 减计数
*/
static void setCounterMode(uint32_t counterNum, bool mode)
{
    if (mode)
    {
        plc_set_bit_element_value(SM_ELEMENT, counterNum, 0);
    }
    else
    {
        plc_set_bit_element_value(SM_ELEMENT, counterNum, 1);
    }
}
static int32_t getCounterCompareValue(uint32_t counterNum)
{
    return gHCNT.hcnts[counterNum - HCOUNTER236].compareNum;
}

// Add or minus
static void handleCounterValueAddOrMinus(uint16_t counterNum)
{
    long *pCounterValue = GET_C32_ADDRESS(counterNum);
    bool upperCounter = isUpperCounter(counterNum);
    if (upperCounter)
    {
        (*pCounterValue)++;
    }
    else
    {
        (*pCounterValue)--;
    }
    if(*pCounterValue == getCounterCompareValue(counterNum))
    {
        if(upperCounter)
        {
            plc_set_bit_element_value(C_ELEMENT, counterNum, 1);
        }
        else
        {
            plc_set_bit_element_value(C_ELEMENT, counterNum, 0);
        }
    }
}

// Add only
static void handleCounterValueAddOnly(uint16_t counterNum)
{
    long *pCounterValue = GET_C32_ADDRESS(counterNum);
    (*pCounterValue)++;
    setCounterMode(counterNum, true);
    if(*pCounterValue == getCounterCompareValue(counterNum))
    {
        plc_set_bit_element_value(C_ELEMENT, HCOUNTER248, 1);
    }
}

// Minus only
static void handleCounterValueMinusOnly(uint16_t counterNum)
{
    long *pCounterValue = GET_C32_ADDRESS(counterNum);
    (*pCounterValue)--;
    setCounterMode(counterNum, false);
    if(*pCounterValue == getCounterCompareValue(counterNum))
    {
        plc_set_bit_element_value(C_ELEMENT, HCOUNTER248, 0);
    }
}

static void handleCounterValueBPhase(uint16_t counterNum)
{
    long *pCounterValue = GET_C32_ADDRESS(counterNum);
    GPIO_Type *base;
    uint32_t pin;

    switch (counterNum)
    {
        case HCOUNTER256:
        case HCOUNTER260:
        case HCOUNTER262:
            base = X0_GPIO;
            pin = X0_GPIO_PIN;
            break;
            
        case HCOUNTER257:
            base = X2_GPIO;
            pin = X2_GPIO_PIN;
            break;
            
        case HCOUNTER258:
            base = X4_GPIO;
            pin = X4_GPIO_PIN;
            break;
            
        case HCOUNTER261:
        case HCOUNTER263:
            base = X3_GPIO;
            pin = X3_GPIO_PIN;
            break;
    }
    
    if(GPIO_PinReadPadStatus(base, pin))//如果此时A相为高电平
    {
        setCounterMode(counterNum, true);
        (*pCounterValue)++;
        if(*pCounterValue == getCounterCompareValue(counterNum))
        {
            plc_set_bit_element_value(C_ELEMENT, counterNum, 1);
        }
    }
    else
    {
        setCounterMode(counterNum, false);
        (*pCounterValue)--;
        if(*pCounterValue == getCounterCompareValue(counterNum))
        {
            plc_set_bit_element_value(C_ELEMENT, counterNum, 0);
        }
    }
}

static void X002_handleCounterReset(uint16_t counterNum)
{
    switch (counterNum)
    {
        case HCOUNTER244:
        case HCOUNTER246:
        case HCOUNTER252:
        case HCOUNTER254:
        case HCOUNTER260:
        case HCOUNTER262:
            SET_C32_CURRENT_VALUE(counterNum, 0);
            plc_set_bit_element_value(C_ELEMENT, counterNum, 0);
            break;
    }
}

static void X005_handleCounterReset(uint16_t counterNum)
{
    switch (counterNum)
    {
        case HCOUNTER245:
        case HCOUNTER247:
        case HCOUNTER253:
        case HCOUNTER255:
        case HCOUNTER261:
        case HCOUNTER263:
            SET_C32_CURRENT_VALUE(counterNum, 0);
            plc_set_bit_element_value(C_ELEMENT, counterNum, 0);
            break;
    }
}

#if 0
static void handleDHSCS(void)
{
    for (uint32_t i = 0; i < MAX_INSTRUCTION_NUM; i++)
    {
        if (gDHSCS.dhscs[i].started)
        {
            if (gDHSCS.dhscs[i].compareNum == GET_C32_CURRENT_VALUE(gDHSCS.dhscs[i].counterNum))
            {
                switch (gDHSCS.dhscs[i].elemType)
                {
                    case ADDR_Y:
                        kalyke_Y_output(gDHSCS.dhscs[i].address, 1);
                        plc_set_bit_element_value(Y_ELEMENT, gDHSCS.dhscs[i].address, 1);
                        break;

                    case ADDR_M:
                        plc_set_bit_element_value(M_ELEMENT, gDHSCS.dhscs[i].address, 1);
                        break;

                    case ADDR_S:
                        plc_set_bit_element_value(S_ELEMENT, gDHSCS.dhscs[i].address, 1);
                        break;
                }
            }
        }
    }
}
#else
static void handleDHSCS(hs_dhscs_state_t *pDHSCS)
{
    if (pDHSCS->compareNum == GET_C32_CURRENT_VALUE(pDHSCS->counterNum))
    {
        switch (pDHSCS->elemType)
        {
            case ADDR_Y:
                kalyke_Y_output(pDHSCS->address, 1);
                plc_set_bit_element_value(Y_ELEMENT, pDHSCS->address, 1);
                break;

            case ADDR_M:
                plc_set_bit_element_value(M_ELEMENT, pDHSCS->address, 1);
                break;

            case ADDR_S:
                plc_set_bit_element_value(S_ELEMENT, pDHSCS->address, 1);
                break;
        }
    }
}
#endif

static void handleDHSCI(hs_dhsci_t *pDHSCI)
{
    if (pDHSCI->compareNum == GET_C32_CURRENT_VALUE(pDHSCI->counterNum))
    {
        plc_user_interrupt_enable(1);
        plc_add_int_to_interrupt_queue(pDHSCI->intNum, 1);
    }
}

#if 0
static void handleDHSCR(void)
{
    for (uint32_t i = 0; i < MAX_INSTRUCTION_NUM; i++)
    {
        if (gDHSCR[i].started)
        {
            if (gDHSCR[i].compareNum == GET_C32_CURRENT_VALUE(gDHSCR[i].counterNum))
            {
                switch (gDHSCR[i].elemType)
                {
                    case ADDR_Y:
                        kalyke_Y_output(gDHSCR[i].address, 0);
                        plc_set_bit_element_value(Y_ELEMENT, gDHSCR[i].address, 0);
                        break;

                    case ADDR_M:
                        plc_set_bit_element_value(M_ELEMENT, gDHSCR[i].address, 0);
                        break;

                    case ADDR_S:
                        plc_set_bit_element_value(S_ELEMENT, gDHSCR[i].address, 0);
                        break;
                    case ADDR_bC:
                        plc_set_bit_element_value(C_ELEMENT, gDHSCR[i].address, 0);
                        break;
                }
            }
        }
    }
}
#else
static void handleDHSCR(hs_dhscr_t *pDHSCR)
{
    if (pDHSCR->compareNum == GET_C32_CURRENT_VALUE(pDHSCR->counterNum))
    {
        switch (pDHSCR->elemType)
        {
            case ADDR_Y:
                kalyke_Y_output(pDHSCR->address, 0);
                plc_set_bit_element_value(Y_ELEMENT, pDHSCR->address, 0);
                break;
            case ADDR_M:
                plc_set_bit_element_value(M_ELEMENT, pDHSCR->address, 0);
                break;
            case ADDR_S:
                plc_set_bit_element_value(S_ELEMENT, pDHSCR->address, 0);
                break;
            case ADDR_bC:
                plc_set_bit_element_value(C_ELEMENT, pDHSCR->address, 0);
                break;
        }
    }
}
#endif

#if 0
static void handleDHSZ(void)
{
    for (uint32_t i = 0; i < MAX_INSTRUCTION_NUM; i++)
    {
        if (gDHSZ[i].started)
        {
            int32_t counterVal = GET_C32_CURRENT_VALUE(gDHSZ[i].counterNum);
            if (counterVal < gDHSZ[i].compareNum1)
            {
                switch (gDHSZ[i].elemType)
                {
                    case ADDR_Y:
                        kalyke_Y_output(gDHSZ[i].address, 0);
                        plc_set_bit_element_value(Y_ELEMENT, gDHSZ[i].address, 1);
                        plc_set_bit_element_value(Y_ELEMENT, gDHSZ[i].address + 1, 0);
                        plc_set_bit_element_value(Y_ELEMENT, gDHSZ[i].address + 2, 0);
                        break;

                    case ADDR_M:
                        plc_set_bit_element_value(M_ELEMENT, gDHSZ[i].address, 1);
                        plc_set_bit_element_value(M_ELEMENT, gDHSZ[i].address + 1, 0);
                        plc_set_bit_element_value(M_ELEMENT, gDHSZ[i].address + 2, 0);
                        break;

                    case ADDR_S:
                        plc_set_bit_element_value(S_ELEMENT, gDHSZ[i].address, 1);
                        plc_set_bit_element_value(S_ELEMENT, gDHSZ[i].address + 1, 0);
                        plc_set_bit_element_value(S_ELEMENT, gDHSZ[i].address + 2, 0);
                        break;
                }
            }
            else if (counterVal <= gDHSZ[i].compareNum2)
            {
                switch (gDHSZ[i].elemType)
                {
                    case ADDR_Y:
                        kalyke_Y_output(gDHSZ[i].address, 0);
                        plc_set_bit_element_value(Y_ELEMENT, gDHSZ[i].address, 0);
                        plc_set_bit_element_value(Y_ELEMENT, gDHSZ[i].address + 1, 1);
                        plc_set_bit_element_value(Y_ELEMENT, gDHSZ[i].address + 2, 0);
                        break;

                    case ADDR_M:
                        plc_set_bit_element_value(M_ELEMENT, gDHSZ[i].address, 0);
                        plc_set_bit_element_value(M_ELEMENT, gDHSZ[i].address + 1, 1);
                        plc_set_bit_element_value(M_ELEMENT, gDHSZ[i].address + 2, 0);
                        break;

                    case ADDR_S:
                        plc_set_bit_element_value(S_ELEMENT, gDHSZ[i].address, 0);
                        plc_set_bit_element_value(S_ELEMENT, gDHSZ[i].address + 1, 1);
                        plc_set_bit_element_value(S_ELEMENT, gDHSZ[i].address + 2, 0);
                        break;
                }
            }
            else
            {
                switch (gDHSZ[i].elemType)
                {
                    case ADDR_Y:
                        kalyke_Y_output(gDHSZ[i].address, 0);
                        plc_set_bit_element_value(Y_ELEMENT, gDHSZ[i].address, 0);
                        plc_set_bit_element_value(Y_ELEMENT, gDHSZ[i].address + 1, 0);
                        plc_set_bit_element_value(Y_ELEMENT, gDHSZ[i].address + 2, 1);
                        break;

                    case ADDR_M:
                        plc_set_bit_element_value(M_ELEMENT, gDHSZ[i].address, 0);
                        plc_set_bit_element_value(M_ELEMENT, gDHSZ[i].address + 1, 0);
                        plc_set_bit_element_value(M_ELEMENT, gDHSZ[i].address + 2, 1);
                        break;

                    case ADDR_S:
                        plc_set_bit_element_value(S_ELEMENT, gDHSZ[i].address, 0);
                        plc_set_bit_element_value(S_ELEMENT, gDHSZ[i].address + 1, 0);
                        plc_set_bit_element_value(S_ELEMENT, gDHSZ[i].address + 2, 1);
                        break;
                }
            }
        }
    }
}
#else
static void handleDHSZ(hs_dhsz_t *pDHSZ)
{
    int32_t counterVal = GET_C32_CURRENT_VALUE(pDHSZ->counterNum);
    if (counterVal < pDHSZ->compareNum1)
    {
        switch (pDHSZ->elemType)
        {
            case ADDR_Y:
                kalyke_Y_output(pDHSZ->address, 0);
                plc_set_bit_element_value(Y_ELEMENT, pDHSZ->address, 1);
                plc_set_bit_element_value(Y_ELEMENT, pDHSZ->address + 1, 0);
                plc_set_bit_element_value(Y_ELEMENT, pDHSZ->address + 2, 0);
                break;
            case ADDR_M:
                plc_set_bit_element_value(M_ELEMENT, pDHSZ->address, 1);
                plc_set_bit_element_value(M_ELEMENT, pDHSZ->address + 1, 0);
                plc_set_bit_element_value(M_ELEMENT, pDHSZ->address + 2, 0);
                break;
            case ADDR_S:
                plc_set_bit_element_value(S_ELEMENT, pDHSZ->address, 1);
                plc_set_bit_element_value(S_ELEMENT, pDHSZ->address + 1, 0);
                plc_set_bit_element_value(S_ELEMENT, pDHSZ->address + 2, 0);
                break;
        }
    }
    else if (counterVal <= pDHSZ->compareNum2)
    {
        switch (pDHSZ->elemType)
        {
            case ADDR_Y:
                kalyke_Y_output(pDHSZ->address, 0);
                plc_set_bit_element_value(Y_ELEMENT, pDHSZ->address, 0);
                plc_set_bit_element_value(Y_ELEMENT, pDHSZ->address + 1, 1);
                plc_set_bit_element_value(Y_ELEMENT, pDHSZ->address + 2, 0);
                break;
            case ADDR_M:
                plc_set_bit_element_value(M_ELEMENT, pDHSZ->address, 0);
                plc_set_bit_element_value(M_ELEMENT, pDHSZ->address + 1, 1);
                plc_set_bit_element_value(M_ELEMENT, pDHSZ->address + 2, 0);
                break;
            case ADDR_S:
                plc_set_bit_element_value(S_ELEMENT, pDHSZ->address, 0);
                plc_set_bit_element_value(S_ELEMENT, pDHSZ->address + 1, 1);
                plc_set_bit_element_value(S_ELEMENT, pDHSZ->address + 2, 0);
                break;
        }
    }
    else
    {
        switch (pDHSZ->elemType)
        {
            case ADDR_Y:
                kalyke_Y_output(pDHSZ->address, 0);
                plc_set_bit_element_value(Y_ELEMENT, pDHSZ->address, 0);
                plc_set_bit_element_value(Y_ELEMENT, pDHSZ->address + 1, 0);
                plc_set_bit_element_value(Y_ELEMENT, pDHSZ->address + 2, 1);
                break;
            case ADDR_M:
                plc_set_bit_element_value(M_ELEMENT, pDHSZ->address, 0);
                plc_set_bit_element_value(M_ELEMENT, pDHSZ->address + 1, 0);
                plc_set_bit_element_value(M_ELEMENT, pDHSZ->address + 2, 1);
                break;
            case ADDR_S:
                plc_set_bit_element_value(S_ELEMENT, pDHSZ->address, 0);
                plc_set_bit_element_value(S_ELEMENT, pDHSZ->address + 1, 0);
                plc_set_bit_element_value(S_ELEMENT, pDHSZ->address + 2, 1);
                break;
        }
    }
}
#endif

#if 0
static void handleDHST(void)
{
    for (uint32_t i = 0; i < MAX_INSTRUCTION_NUM; i++)
    {
        if (gDHST[i].started)
        {
            uint32_t dAddress = gDHST[i].address + gDHST[i].curRecord * 4;
            int32_t *pSD = (int32_t *)&gtv_PlcElement.msp_SDElement[SD182];/* SD182、SD183表示当前要比较的数据 */
            if (gDHST[i].elemType == ADDR_D)
            {
                int32_t *pCompareVal = (int32_t *)&gtv_PlcElement.msp_DElement[dAddress];
                if (GET_C32_CURRENT_VALUE(gDHST[i].counterNum) == *pCompareVal)
                {
                    if(GET_D_ELEMENT_VALUE(dAddress + 3) == 1)
                    {
                        kalyke_Y_output(GET_D_ELEMENT_VALUE(dAddress + 2), 1);
                        plc_set_bit_element_value(Y_ELEMENT, GET_D_ELEMENT_VALUE(dAddress + 2), 1);
                    }
                    else if(GET_D_ELEMENT_VALUE(dAddress + 3) == 0)
                    {
                        kalyke_Y_output(GET_D_ELEMENT_VALUE(dAddress + 2), 0);
                        plc_set_bit_element_value(Y_ELEMENT, GET_D_ELEMENT_VALUE(dAddress + 2), 0);
                    }
                    gDHST[i].curRecord++;
                    if (gDHST[i].curRecord >= gDHST[i].recordNum)
                    {
                        plc_set_bit_element_value(SM_ELEMENT, SM185, 1);
                        gDHST[i].curRecord = 0;
                    }
                    SET_SD_ELEMENT_VALUE(SD184, gDHST[i].curRecord + 1);
                    dAddress = gDHST[i].address + gDHST[i].curRecord * 4;
                    pCompareVal = (int32_t *)&gtv_PlcElement.msp_DElement[dAddress];
                    *pSD = *pCompareVal;
                }
            }
            else if (gDHST[i].elemType == ADDR_R)
            {
                int32_t *pCompareVal = (int32_t *)&gtv_PlcElement.msp_RElement[dAddress];
                if (GET_C32_CURRENT_VALUE(gDHST[i].counterNum) == *pCompareVal)
                {
                    if(GET_R_ELEMENT_VALUE(dAddress + 3) == 1)
                    {
                        kalyke_Y_output(GET_R_ELEMENT_VALUE(dAddress + 2), 1);
                        plc_set_bit_element_value(Y_ELEMENT, GET_R_ELEMENT_VALUE(dAddress + 2), 1);
                    }
                    else if(GET_R_ELEMENT_VALUE(dAddress + 3) == 0)
                    {
                        kalyke_Y_output(GET_R_ELEMENT_VALUE(dAddress + 2), 0);
                        plc_set_bit_element_value(Y_ELEMENT, GET_R_ELEMENT_VALUE(dAddress + 2), 0);
                    }
                    gDHST[i].curRecord++;
                    if (gDHST[i].curRecord >= gDHST[i].recordNum)
                    {
                        plc_set_bit_element_value(SM_ELEMENT, SM185, 1);
                        gDHST[i].curRecord = 0;
                    }
                    SET_SD_ELEMENT_VALUE(SD184, gDHST[i].curRecord + 1);
                    dAddress = gDHST[i].address + gDHST[i].curRecord * 4;
                    pCompareVal = (int32_t *)&gtv_PlcElement.msp_RElement[dAddress];
                    *pSD = *pCompareVal;
                }
            }
        }
    }
}
#else
static void handleDHST(hs_dhst_t *pDHST)
{
    uint32_t dAddress = pDHST->address + pDHST->curRecord * 4;
    int32_t *pSD = (int32_t *)&gtv_PlcElement.msp_SDElement[SD182];/* SD182、SD183表示当前要比较的数据 */
    if (pDHST->elemType == ADDR_D)
    {
        int32_t *pCompareVal = (int32_t *)&gtv_PlcElement.msp_DElement[dAddress];
        if (GET_C32_CURRENT_VALUE(pDHST->counterNum) == *pCompareVal)
        {
            if(GET_D_ELEMENT_VALUE(dAddress + 3) == 1)
            {
                kalyke_Y_output(GET_D_ELEMENT_VALUE(dAddress + 2), 1);
                plc_set_bit_element_value(Y_ELEMENT, GET_D_ELEMENT_VALUE(dAddress + 2), 1);
            }
            else if(GET_D_ELEMENT_VALUE(dAddress + 3) == 0)
            {
                kalyke_Y_output(GET_D_ELEMENT_VALUE(dAddress + 2), 0);
                plc_set_bit_element_value(Y_ELEMENT, GET_D_ELEMENT_VALUE(dAddress + 2), 0);
            }
            pDHST->curRecord++;
            if (pDHST->curRecord >= pDHST->recordNum)
            {
                plc_set_bit_element_value(SM_ELEMENT, SM185, 1);
                pDHST->curRecord = 0;
            }
            SET_SD_ELEMENT_VALUE(SD184, pDHST->curRecord + 1);
            dAddress = pDHST->address + pDHST->curRecord * 4;
            pCompareVal = (int32_t *)&gtv_PlcElement.msp_DElement[dAddress];
            *pSD = *pCompareVal;
        }
    }
    else if (pDHST->elemType == ADDR_R)
    {
        int32_t *pCompareVal = (int32_t *)&gtv_PlcElement.msp_RElement[dAddress];
        if (GET_C32_CURRENT_VALUE(pDHST->counterNum) == *pCompareVal)
        {
            if(GET_R_ELEMENT_VALUE(dAddress + 3) == 1)
            {
                kalyke_Y_output(GET_R_ELEMENT_VALUE(dAddress + 2), 1);
                plc_set_bit_element_value(Y_ELEMENT, GET_R_ELEMENT_VALUE(dAddress + 2), 1);
            }
            else if(GET_R_ELEMENT_VALUE(dAddress + 3) == 0)
            {
                kalyke_Y_output(GET_R_ELEMENT_VALUE(dAddress + 2), 0);
                plc_set_bit_element_value(Y_ELEMENT, GET_R_ELEMENT_VALUE(dAddress + 2), 0);
            }
            pDHST->curRecord++;
            if (pDHST->curRecord >= pDHST->recordNum)
            {
                plc_set_bit_element_value(SM_ELEMENT, SM185, 1);
                pDHST->curRecord = 0;
            }
            SET_SD_ELEMENT_VALUE(SD184, pDHST->curRecord + 1);
            dAddress = pDHST->address + pDHST->curRecord * 4;
            pCompareVal = (int32_t *)&gtv_PlcElement.msp_RElement[dAddress];
            *pSD = *pCompareVal;
        }
    }
}
#endif

#if 0
static void handleDHSP(void)
{
    for (uint32_t i = 0; i < MAX_INSTRUCTION_NUM; i++)
    {
        if (gDHSP[i].started)
        {
            uint32_t dAddress = gDHSP[i].address + gDHSP[i].curRecord * 4;
            uint32_t *pSD = (uint32_t *)&gtv_PlcElement.msp_SDElement[SD180];//输出到SD180、SD181
            if (gDHSP[i].elemType == ADDR_D)
            {
                int32_t *pCompareVal = (int32_t *)&gtv_PlcElement.msp_DElement[dAddress];
                if (GET_C32_CURRENT_VALUE(gDHSP[i].counterNum) == *pCompareVal)
                {
                    uint32_t *pD = (uint32_t *)&gtv_PlcElement.msp_DElement[dAddress + 2];
                    *pSD = *pD;
                    gDHSP[i].curRecord++;
                    if (gDHSP[i].curRecord >= gDHSP[i].recordNum)
                    {
                        plc_set_bit_element_value(SM_ELEMENT, SM185, 1);
                        gDHSP[i].curRecord = 0;
                    }
                    SET_SD_ELEMENT_VALUE(SD184, gDHSP[i].curRecord + 1);
                    dAddress = gDHSP[i].address + gDHSP[i].curRecord * 4;
                    pCompareVal = (int32_t *)&gtv_PlcElement.msp_DElement[dAddress];
                    pSD = (uint32_t *)&gtv_PlcElement.msp_SDElement[SD182];/* SD182、SD183表示当前要比较的数据 */
                    *pSD = *pCompareVal;
                }
            }
            else if (gDHSP[i].elemType == ADDR_R)
            {
                int32_t *pCompareVal = (int32_t *)&gtv_PlcElement.msp_RElement[dAddress];
                if (GET_C32_CURRENT_VALUE(gDHSP[i].counterNum) == *pCompareVal)
                {
                    uint32_t *pR = (uint32_t *)&gtv_PlcElement.msp_RElement[dAddress + 2];
                    *pSD = *pR;
                    gDHSP[i].curRecord++;
                    if (gDHSP[i].curRecord >= gDHSP[i].recordNum)
                    {
                        plc_set_bit_element_value(SM_ELEMENT, SM185, 1);
                        gDHSP[i].curRecord = 0;
                    }
                    SET_SD_ELEMENT_VALUE(SD184, gDHSP[i].curRecord + 1);
                    dAddress = gDHSP[i].address + gDHSP[i].curRecord * 4;
                    pCompareVal = (int32_t *)&gtv_PlcElement.msp_RElement[dAddress];
                    pSD = (uint32_t *)&gtv_PlcElement.msp_SDElement[SD182];/* SD182、SD183表示当前要比较的数据 */
                    *pSD = *pCompareVal;
                }
            }
        }
    }
}
#else
static void handleDHSP(hs_dhsp_t *pDHSP)
{
    uint32_t dAddress = pDHSP->address + pDHSP->curRecord * 4;
    uint32_t *pSD = (uint32_t *)&gtv_PlcElement.msp_SDElement[SD180];//输出到SD180、SD181
    if (pDHSP->elemType == ADDR_D)
    {
        int32_t *pCompareVal = (int32_t *)&gtv_PlcElement.msp_DElement[dAddress];
        if (GET_C32_CURRENT_VALUE(pDHSP->counterNum) == *pCompareVal)
        {
            uint32_t *pD = (uint32_t *)&gtv_PlcElement.msp_DElement[dAddress + 2];
            *pSD = *pD;
            pDHSP->curRecord++;
            if (pDHSP->curRecord >= pDHSP->recordNum)
            {
                plc_set_bit_element_value(SM_ELEMENT, SM185, 1);
                pDHSP->curRecord = 0;
            }
            SET_SD_ELEMENT_VALUE(SD184, pDHSP->curRecord + 1);
            dAddress = pDHSP->address + pDHSP->curRecord * 4;
            pCompareVal = (int32_t *)&gtv_PlcElement.msp_DElement[dAddress];
            pSD = (uint32_t *)&gtv_PlcElement.msp_SDElement[SD182];/* SD182、SD183表示当前要比较的数据 */
            *pSD = *pCompareVal;
        }
    }
    else if (pDHSP->elemType == ADDR_R)
    {
        int32_t *pCompareVal = (int32_t *)&gtv_PlcElement.msp_RElement[dAddress];
        if (GET_C32_CURRENT_VALUE(pDHSP->counterNum) == *pCompareVal)
        {
            uint32_t *pR = (uint32_t *)&gtv_PlcElement.msp_RElement[dAddress + 2];
            *pSD = *pR;
            pDHSP->curRecord++;
            if (pDHSP->curRecord >= pDHSP->recordNum)
            {
                plc_set_bit_element_value(SM_ELEMENT, SM185, 1);
                pDHSP->curRecord = 0;
            }
            SET_SD_ELEMENT_VALUE(SD184, pDHSP->curRecord + 1);
            dAddress = pDHSP->address + pDHSP->curRecord * 4;
            pCompareVal = (int32_t *)&gtv_PlcElement.msp_RElement[dAddress];
            pSD = (uint32_t *)&gtv_PlcElement.msp_SDElement[SD182];/* SD182、SD183表示当前要比较的数据 */
            *pSD = *pCompareVal;
        }
    }
}
#endif

static void handleDHSx(void)
{
    if (gHS_DHSx.head == gHS_DHSx.tail)//空链表
    {
        return;
    }

    uint8_t idx = gHS_DHSx.head;
    while (idx != gHS_DHSx.tail)
    {
        switch (gHS_DHSx.hstate[idx].type)
        {
            case DHSx_TYPE_DHSCS:
                handleDHSCS(&gHS_DHSx.hstate[idx].dhsx.dhscs);
                break;
            case DHSx_TYPE_DHSCR:
                handleDHSCR(&gHS_DHSx.hstate[idx].dhsx.dhscr);
                break;
            case DHSx_TYPE_DHSZ:
                handleDHSZ(&gHS_DHSx.hstate[idx].dhsx.dhsz);
                break;
            case DHSx_TYPE_DHST:
                handleDHST(&gHS_DHSx.hstate[idx].dhsx.dhst);
                break;
            case DHSx_TYPE_DHSP:
                handleDHSP(&gHS_DHSx.hstate[idx].dhsx.dhsp);
                break;
            case DHSx_TYPE_DHSCI:
                handleDHSCI(&gHS_DHSx.hstate[idx].dhsx.dhsci);
                break;
        }
        idx = gHS_DHSx.hstate[idx].next;
    }
}

#if (KALYKE_HIGH_SPEED_IO == 1)
void X000_Interrupt(void)
{
    /*For SPD instruction */
    if (gSPD[0].started)
    {
        gSPD[0].intCount++;
        return; // SPD和Counter只有一个存在
    }
    if (IS_C32_STARTED(HCOUNTER236))
    {
        handleCounterValueAddOrMinus(HCOUNTER236);
    }
    if (IS_C32_STARTED(HCOUNTER244))
    {
        handleCounterValueAddOrMinus(HCOUNTER244);
    }
    if (IS_C32_STARTED(HCOUNTER246))
    {
        handleCounterValueAddOrMinus(HCOUNTER246);
    }
    if (IS_C32_STARTED(HCOUNTER248))
    {
        handleCounterValueAddOnly(HCOUNTER248);
    }
    if (IS_C32_STARTED(HCOUNTER252))
    {
        handleCounterValueAddOnly(HCOUNTER252);
    }
    if (IS_C32_STARTED(HCOUNTER254))
    {
        handleCounterValueAddOnly(HCOUNTER254);
    }
    handleDHSx();
}

void X001_Interrupt(void)
{
    /*For SPD instruction */
    if (gSPD[1].started)
    {
        gSPD[1].intCount++;
        return;
    }
    if (IS_C32_STARTED(HCOUNTER237))
    {
        handleCounterValueAddOrMinus(HCOUNTER237);
    }
    
    if (IS_C32_STARTED(HCOUNTER248))
    {
        handleCounterValueMinusOnly(HCOUNTER248);
    }
    if (IS_C32_STARTED(HCOUNTER252))
    {
        handleCounterValueMinusOnly(HCOUNTER252);
    }
    if (IS_C32_STARTED(HCOUNTER254))
    {
        handleCounterValueMinusOnly(HCOUNTER254);
    }

    if (IS_C32_STARTED(HCOUNTER256))
    {
        handleCounterValueBPhase(HCOUNTER256);
    }
    if (IS_C32_STARTED(HCOUNTER260))
    {
        handleCounterValueBPhase(HCOUNTER260);
    }
    if (IS_C32_STARTED(HCOUNTER262))
    {
        handleCounterValueBPhase(HCOUNTER262);
    }
    handleDHSx();
}

void X002_Interrupt(void)
{
    /*For SPD instruction */
    if (gSPD[2].started)
    {
        gSPD[2].intCount++;
        return;
    }
    if (IS_C32_STARTED(HCOUNTER238))
    {
        handleCounterValueAddOrMinus(HCOUNTER238);
    }
    if (IS_C32_STARTED(HCOUNTER249))
    {
        handleCounterValueAddOnly(HCOUNTER249);
    }
    handleDHSx();

    if (gHCNT2.head != gHCNT2.tail)
    {
        uint8_t idx = gHCNT2.head;
        while (idx != gHCNT2.tail)
        {
            X002_handleCounterReset(gHCNT2.hcnts[idx].counterNum);
            idx = gHCNT2.hcnts[idx].next;
        }
    }
}

void X003_Interrupt(void)
{
    /*For SPD instruction */
    if (gSPD[3].started)
    {
        gSPD[3].intCount++;
        return;
    }
    if (IS_C32_STARTED(HCOUNTER239))
    {
        handleCounterValueAddOrMinus(HCOUNTER239);
    }
    if (IS_C32_STARTED(HCOUNTER245))
    {
        handleCounterValueAddOrMinus(HCOUNTER245);
    }
    if (IS_C32_STARTED(HCOUNTER247))
    {
        handleCounterValueAddOrMinus(HCOUNTER247);
    }
    if (IS_C32_STARTED(HCOUNTER249))
    {
        handleCounterValueMinusOnly(HCOUNTER249);
    }
    if (IS_C32_STARTED(HCOUNTER253))
    {
        handleCounterValueAddOnly(HCOUNTER253);
    }
    if (IS_C32_STARTED(HCOUNTER255))
    {
        handleCounterValueAddOnly(HCOUNTER255);
    }
    if (IS_C32_STARTED(HCOUNTER257))
    {
        handleCounterValueBPhase(HCOUNTER257);
    }
    
    handleDHSx();
}

void X004_Interrupt(void)
{

    if (IS_C32_STARTED(HCOUNTER240))
    {
        handleCounterValueAddOrMinus(HCOUNTER240);
    }
    if (IS_C32_STARTED(HCOUNTER250))
    {
        handleCounterValueAddOnly(HCOUNTER250);
    }
    if (IS_C32_STARTED(HCOUNTER253))
    {
        handleCounterValueMinusOnly(HCOUNTER253);
    }
    if (IS_C32_STARTED(HCOUNTER255))
    {
        handleCounterValueMinusOnly(HCOUNTER255);
    }
    if (IS_C32_STARTED(HCOUNTER261))
    {
        handleCounterValueBPhase(HCOUNTER261);
    }
    if (IS_C32_STARTED(HCOUNTER263))
    {
        handleCounterValueBPhase(HCOUNTER263);
    }
}

void X005_Interrupt(void)
{

    if (IS_C32_STARTED(HCOUNTER241))
    {
        handleCounterValueAddOrMinus(HCOUNTER241);
    }
    if (IS_C32_STARTED(HCOUNTER250))
    {
        handleCounterValueMinusOnly(HCOUNTER250);
    }
    if (IS_C32_STARTED(HCOUNTER258))
    {
        handleCounterValueBPhase(HCOUNTER258);
    }

    for (int i = 0; i < gHCNT2.cntNumbers; i++)
    {
        X005_handleCounterReset(gHCNT2.hcnts[i].counterNum);
    }
}

void X006_Interrupt(void)
{
    if (IS_C32_STARTED(HCOUNTER246))
    {
        // Do nothing, 因为计数器是否启动已在run_ci_hcnt_ins函数中处理了
    }
    if (IS_C32_STARTED(HCOUNTER254))
    {
        // Do nothing, 因为计数器是否启动已在run_ci_hcnt_ins函数中处理了
    }
    if (IS_C32_STARTED(HCOUNTER262))
    {
        // Do nothing, 因为计数器是否启动已在run_ci_hcnt_ins函数中处理了
    }
}

void X007_Interrupt(void)
{
    if (IS_C32_STARTED(HCOUNTER247))
    {
        // Do nothing, 因为计数器是否启动已在run_ci_hcnt_ins函数中处理了
    }
}
#endif /* KALYKE_HIGH_SPEED_IO */

void kalyke_highspped_init(void)
{
    init_high_speed();
}

