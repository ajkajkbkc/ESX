/**
  ******************************************************************************
  * @file    plc_element.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   PLC中各种原件相关操作
  ******************************************************************************
  */
#include "plc_element.h"
#include "bsp_dct.h"
#include "FreeRTOS.h"
#include "task.h"
#include "plc_variable.h"
#include "stdlib.h"
#include "fsl_debug_console.h"

/*------------------------------------------------------------------------------
*   变量申明
*-----------------------------------------------------------------------------*/
/*PLC元件信息*/
plc_element_info_st *gtp_PlcElementInfo = NULL;

/*PLC各种元件值指针结构体*/
plc_element_st gtv_PlcElement = {NULL, };

#if (INLINE_BIT_ELEMENT == 0)
// Inline functions
/**
  * @brief  设置位元件值
  * @param  BaseAddr 位元件基地址
  *         Element  位元件
  *         Value   设置值
  * @retval None
  */
void plc_set_bit_element_value(unsigned short * BaseAddr, unsigned short Element, unsigned char Value)
{
    unsigned short * lsp_BaseAddr = BaseAddr;

    lsp_BaseAddr += (Element >> 4);

    if(Value) {
        *lsp_BaseAddr |= (0x01 << (Element & 0x0F));
    } else {
        *lsp_BaseAddr &= (~(0x01 << (Element & 0x0F)));
    }
}

/**
  * @brief  获取位元件值
  * @param  BaseAddr 位元件基地址
  *         Element  位元件
  * @retval None
  */
unsigned char plc_get_bit_element_value(unsigned short * BaseAddr, unsigned short Element)
{
    unsigned char lcv_ElementValue;

    lcv_ElementValue = (*(BaseAddr + (Element >> 4)) >> (Element & 0x0F)) & 0x01;

    return lcv_ElementValue;
}

/**
  * @brief  获取指定layer层LM位元件值
  * @param  layer 嵌套层数,小于MAX_SBR_NEST_LAYER
  *         Element  位元件
  * @retval None
  */
unsigned char plc_get_lm_element_value(unsigned char layer, unsigned short Element)
{
    unsigned char lcv_ElementValue;
    unsigned short *lsp_BaseAddr = gtv_PlcElement.msp_LMElement + ((LM_RANG)/16)*layer;

    lcv_ElementValue = (*(lsp_BaseAddr + (Element >> 4)) >> (Element & 0x0F)) & 0x01;

    return lcv_ElementValue;
}

/**
  * @brief  设置指定layer层LM位元件值
  * @param  layer 嵌套层数,小于MAX_SBR_NEST_LAYER
  *         Element  位元件
  * @retval None
  */
void plc_set_lm_element_value(unsigned char layer, unsigned short Element, unsigned char Value)
{
    unsigned short *lsp_BaseAddr = gtv_PlcElement.msp_LMElement + ((LM_RANG)/16)*layer;

    lsp_BaseAddr += (Element >> 4);

    if(Value) {
        *lsp_BaseAddr |= (0x01 << (Element & 0x0F));
    } else {
        *lsp_BaseAddr &= (~(0x01 << (Element & 0x0F)));
    }
}

/**
  * @brief  清除指定layer层LM位元件值
  * @param  layer 嵌套层数,小于MAX_SBR_NEST_LAYER
  * @retval None
  */

void plc_clean_layer_lm_element(unsigned char layer)
{
    unsigned char i;
    unsigned short *lsp_BaseAddr = gtv_PlcElement.msp_LMElement + ((LM_RANG)/16)*layer;

    for(i=0; i<((LM_RANG)/16); i++) {
        lsp_BaseAddr[i] = 0x0;
    }
}
#endif

/**
  * @brief  plc 元件值初始化
  * @param  None
  * @retval None
  */

void plc_element_value_init(void)
{
    LOGV("plc_element", "Enter %s(), gtv_PlcElement.msp_SMElement = 0x%08X", __func__, gtv_PlcElement.msp_SMElement);
    unsigned short i;
    /*X元件初始化*/
    for(i=0; i<(gtp_PlcElementInfo->msv_XElementCnt)/16; i++) {
        gtv_PlcElement.msp_XElement[i] = 0;
    }

    /*Y元件初始化*/
    for(i=0; i<(gtp_PlcElementInfo->msv_YElementCnt)/16; i++) {
        gtv_PlcElement.msp_YElement[i] = 0;
    }

    /*M元件初始化*/
    for(i=0; i<((gtp_PlcElementInfo->msv_MElementCnt)/16)* MAX_SBR_NESTED_LAYER; i++) {
        gtv_PlcElement.msp_MElement[i] = 0;
    }

    /*LM元件初始化*/
    for(i=0; i<((gtp_PlcElementInfo->msv_LMElementCnt)/16)*MAX_SBR_NESTED_LAYER; i++) {
        gtv_PlcElement.msp_LMElement[i] = 0;
    }

    /*SM元件初始化*/
    for(i=0; i<(gtp_PlcElementInfo->msv_SMElementCnt)/16; i++) {
        gtv_PlcElement.msp_SMElement[i] = 0;
    }

    /*S元件初始化*/
    for(i=0; i<(gtp_PlcElementInfo->msv_SElementCnt)/16; i++) {
        gtv_PlcElement.msp_SElement[i] = 0;
    }

    /*D元件初始化*/
    for(i=0; i<gtp_PlcElementInfo->msv_DElementCnt; i++) {
        gtv_PlcElement.msp_DElement[i] = 0;
    }

    /*SD元件初始化*/
    for(i=0; i<gtp_PlcElementInfo->msv_SDElementCnt; i++) 
    {
       if ((i >= SD233 && i <= SD384) || (i >= 100 && i <= 106) )
           continue;
        gtv_PlcElement.msp_SDElement[i] = 0;
    }

    /*V元件初始化*/
    for(i=0; i<gtp_PlcElementInfo->msv_VElementCnt*MAX_SBR_NESTED_LAYER; i++) {
        gtv_PlcElement.msp_VElement[i] = 0;
    }

    /*R元件初始化*/
    for(i=0; i<gtp_PlcElementInfo->msv_RElementCnt; i++) {
        gtv_PlcElement.msp_RElement[i] = 0;
    }

    /*Z元件初始化*/
    for(i=0; i<gtp_PlcElementInfo->msv_ZElementCnt; i++) {
        gtv_PlcElement.msp_ZElement[i] = 0;
    }

    /*C位元件初始化*/
    for(i=0; i<(gtp_PlcElementInfo->msv_CElement.msv_ElementCnt)/16; i++) {
        gtv_PlcElement.mtv_CElement.msp_BitElement[i] = 0;
    }
    /*C元件状态信息初始化*/
    for(i=0; i<(gtp_PlcElementInfo->msv_CElement.msv_ElementCnt); i++) {
        *(unsigned char *)&gtv_PlcElement.mtv_CElement.mtp_StatusInfo[i] = 0;
    }
    /*C元件16位计数器初始化*/
    for(i=0; i<(gtp_PlcElementInfo->msv_CElement.msv_16bitCnt); i++) {
        gtv_PlcElement.mtv_CElement.msp_16BitValue[i] = 0;
    }
    /*C元件32位计数器初始化*/
    for(i=0; i<(gtp_PlcElementInfo->msv_CElement.msv_32bitCnt); i++) {
        gtv_PlcElement.mtv_CElement.msp_32BitValue[i] = 0;
    }

    /*T位元件初始化*/
    for(i=0; i<(gtp_PlcElementInfo->msv_TElement.msv_ElementCnt)/16; i++) {
        gtv_PlcElement.mtv_TElement.msp_BitElement[i] = 0;
    }
    /*T元件状态信息初始化*/
    for(i=0; i<(gtp_PlcElementInfo->msv_TElement.msv_ElementCnt); i++) {
        *(unsigned char *)&gtv_PlcElement.mtv_TElement.mtp_StatusInfo[i] = 0;
    }
    /*T元件起始值初始化*/
    for(i=0; i<(gtp_PlcElementInfo->msv_TElement.msv_ElementCnt); i++) {
        gtv_PlcElement.mtv_TElement.mlp_StartValue[i] = 0;
    }
    /*T元件目标值初始化*/
    for(i=0; i<(gtp_PlcElementInfo->msv_TElement.msv_ElementCnt); i++) {
        gtv_PlcElement.mtv_TElement.msp_DestValue[i] = 0;
    }
    /*T元件当前值初始化*/
    for(i=0; i<(gtp_PlcElementInfo->msv_TElement.msv_ElementCnt); i++) {
        gtv_PlcElement.mtv_TElement.msp_CurrentValue[i] = 0;
    }

}

/**
  * @brief  plc 元件内存空间分配
  * @param  None
  * @retval None
  */

void plc_element_init(void)
{
    gtp_PlcElementInfo = (plc_element_info_st *)&gtv_DeviceConfigTable.mtv_PlcElementInfo;

    /*X元件分配内存*/
    #ifdef PLC_ELEMENT_USE_DYNAMIC_MALLOC
    gtv_PlcElement.msp_XElement = (unsigned short *)pvPortMalloc(sizeof(unsigned short)*((gtp_PlcElementInfo->msv_XElementCnt)/16));//256 bytes
    configASSERT(gtv_PlcElement.msp_XElement != NULL);
    #endif
    
    /*Y元件分配内存*/
    #ifdef PLC_ELEMENT_USE_DYNAMIC_MALLOC
    gtv_PlcElement.msp_YElement = (unsigned short *)pvPortMalloc(sizeof(unsigned short)*((gtp_PlcElementInfo->msv_YElementCnt)/16));//256 bytes
    configASSERT(gtv_PlcElement.msp_YElement != NULL);
    #endif
    
    /*M元件分配内存*/
    #ifdef PLC_ELEMENT_USE_DYNAMIC_MALLOC
    gtv_PlcElement.msp_MElement = (unsigned short *)pvPortMalloc(sizeof(unsigned short)*((gtp_PlcElementInfo->msv_MElementCnt)/16) * MAX_SBR_NESTED_LAYER);//16K 
    configASSERT(gtv_PlcElement.msp_MElement != NULL);
    #endif
    
    /*LM元件分配内存*/
    #ifdef PLC_ELEMENT_USE_DYNAMIC_MALLOC
    gtv_PlcElement.msp_LMElement = (unsigned short *)pvPortMalloc(sizeof(unsigned short)*(((gtp_PlcElementInfo->msv_LMElementCnt)/16) *MAX_SBR_NESTED_LAYER));//256 bytes
    configASSERT(gtv_PlcElement.msp_LMElement != NULL);
    #endif
    
    /*SM元件分配内存*/
    #ifdef PLC_ELEMENT_USE_DYNAMIC_MALLOC
    gtv_PlcElement.msp_SMElement = (unsigned short *)pvPortMalloc(sizeof(unsigned short)*((gtp_PlcElementInfo->msv_SMElementCnt)/16));//512 bytes
    configASSERT(gtv_PlcElement.msp_SMElement != NULL);
    #endif
    
    /*S元件分配内存*/
    #ifdef PLC_ELEMENT_USE_DYNAMIC_MALLOC
    gtv_PlcElement.msp_SElement = (unsigned short *)pvPortMalloc(sizeof(unsigned short)*((gtp_PlcElementInfo->msv_SElementCnt)/16));//512 bytes
    configASSERT(gtv_PlcElement.msp_SElement != NULL);
    #endif
    
    /*D元件分配内存*/
    #ifdef PLC_ELEMENT_USE_DYNAMIC_MALLOC
    gtv_PlcElement.msp_DElement = (unsigned short *)pvPortMalloc(sizeof(unsigned short)*gtp_PlcElementInfo->msv_DElementCnt);//16000 bytes
    configASSERT(gtv_PlcElement.msp_DElement != NULL);
    #endif
    
    /*R元件分配内存*/
    gtv_PlcElement.msp_RElement = (unsigned short *)pvPortMalloc(sizeof(unsigned short)*gtp_PlcElementInfo->msv_RElementCnt);//64K
    configASSERT(gtv_PlcElement.msp_RElement != NULL);

    /*SD元件分配内存*/
    #ifdef PLC_ELEMENT_USE_DYNAMIC_MALLOC
    gtv_PlcElement.msp_SDElement = (unsigned short *)pvPortMalloc(sizeof(unsigned short)*gtp_PlcElementInfo->msv_SDElementCnt);//8K
    configASSERT(gtv_PlcElement.msp_SDElement != NULL);
    #endif

    /*V元件分配内存*/
    #ifdef PLC_ELEMENT_USE_DYNAMIC_MALLOC
    gtv_PlcElement.msp_VElement = (unsigned short *)pvPortMalloc(sizeof(unsigned short)*gtp_PlcElementInfo->msv_VElementCnt*MAX_SBR_NESTED_LAYER);//4K
    configASSERT(gtv_PlcElement.msp_VElement != NULL);
    #endif

    /*Z元件分配内存*/
    #ifdef PLC_ELEMENT_USE_DYNAMIC_MALLOC
    gtv_PlcElement.msp_ZElement = (unsigned short *)pvPortMalloc(sizeof(unsigned short)*gtp_PlcElementInfo->msv_ZElementCnt);//32 bytes
    configASSERT(gtv_PlcElement.msp_ZElement != NULL);
    #endif
    
    /*C 元件分配内存*/
    /*C 位元件*/
    #ifdef PLC_ELEMENT_USE_DYNAMIC_MALLOC
    gtv_PlcElement.mtv_CElement.msp_BitElement = (unsigned short *)pvPortMalloc(sizeof(unsigned short)*((gtp_PlcElementInfo->msv_CElement.msv_ElementCnt)/16));//64 bytes
    configASSERT(gtv_PlcElement.mtv_CElement.msp_BitElement != NULL);
    #endif
    
    /*C 元件状态信息*/
    #ifdef PLC_ELEMENT_USE_DYNAMIC_MALLOC
    gtv_PlcElement.mtv_CElement.mtp_StatusInfo = (plc_c_element_status_st *)pvPortMalloc(sizeof(plc_c_element_status_st)*(gtp_PlcElementInfo->msv_CElement.msv_ElementCnt));//512 bytes
    configASSERT(gtv_PlcElement.mtv_CElement.mtp_StatusInfo != NULL);
    #endif
    
    /*16bit计数器当前值*/
    #ifdef PLC_ELEMENT_USE_DYNAMIC_MALLOC
    gtv_PlcElement.mtv_CElement.msp_16BitValue = (unsigned short *)pvPortMalloc(sizeof(unsigned short)*(gtp_PlcElementInfo->msv_CElement.msv_16bitCnt));//400 bytes
    configASSERT(gtv_PlcElement.mtv_CElement.msp_16BitValue != NULL);
    #endif
    
    /*32bit计数器当前值*/
    #ifdef PLC_ELEMENT_USE_DYNAMIC_MALLOC
    gtv_PlcElement.mtv_CElement.msp_32BitValue = (long *)pvPortMalloc(sizeof(long)*(gtp_PlcElementInfo->msv_CElement.msv_32bitCnt));//1248 bytes
    configASSERT(gtv_PlcElement.mtv_CElement.msp_32BitValue != NULL);
    #endif

    /*T 元件相关内存分配*/
    /*位元件地址*/
    #ifdef PLC_ELEMENT_USE_DYNAMIC_MALLOC
    gtv_PlcElement.mtv_TElement.msp_BitElement = (unsigned short *)pvPortMalloc(sizeof(unsigned short)*((gtp_PlcElementInfo->msv_TElement.msv_ElementCnt)/16));//64 bytes
    configASSERT(gtv_PlcElement.mtv_TElement.msp_BitElement != NULL);
    #endif

    /*T 元件状态信息*/
    #ifdef PLC_ELEMENT_USE_DYNAMIC_MALLOC
    gtv_PlcElement.mtv_TElement.mtp_StatusInfo = (plc_t_element_status_st *)pvPortMalloc(sizeof(plc_t_element_status_st)*(gtp_PlcElementInfo->msv_TElement.msv_ElementCnt));//512 bytes
    configASSERT(gtv_PlcElement.mtv_TElement.mtp_StatusInfo != NULL);
    #endif

    /*T 定时器开始时间*/
    #ifdef PLC_ELEMENT_USE_DYNAMIC_MALLOC
    gtv_PlcElement.mtv_TElement.mlp_StartValue = (unsigned long *)pvPortMalloc(sizeof(unsigned long)*(gtp_PlcElementInfo->msv_TElement.msv_ElementCnt));//2048 bytes
    configASSERT(gtv_PlcElement.mtv_TElement.mlp_StartValue != NULL);
    #endif
    /*T 定时器目标时间*/
    #ifdef PLC_ELEMENT_USE_DYNAMIC_MALLOC
    gtv_PlcElement.mtv_TElement.msp_DestValue = (unsigned short *)pvPortMalloc(sizeof(unsigned short)*(gtp_PlcElementInfo->msv_TElement.msv_ElementCnt));//1024 bytes
    configASSERT(gtv_PlcElement.mtv_TElement.msp_DestValue != NULL);
    #endif
    /*T 定时器当前时间*/
    #ifdef PLC_ELEMENT_USE_DYNAMIC_MALLOC
    gtv_PlcElement.mtv_TElement.msp_CurrentValue = (unsigned short *)pvPortMalloc(sizeof(unsigned short)*(gtp_PlcElementInfo->msv_TElement.msv_ElementCnt));//1024 bytes
    configASSERT(gtv_PlcElement.mtv_TElement.msp_CurrentValue != NULL);
    #endif
    /*元件值初始化*/
    plc_element_value_init();
}

