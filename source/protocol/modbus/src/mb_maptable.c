/**
  ******************************************************************************
  * @file    mb_maptable.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   Modbus 元件映射表
  ******************************************************************************
  */
#include "mb_maptable.h"
#include "FreeRTOS.h"

/*
* Modbus 协议地址映射表，代码通过查表得到元件类型，物理元件编号
*
*/

static const mb_element_map_table_info stv_ModbusMapTable[] =
{
    /*AddrType          ElementType     AddrSection     StartAddr   EndAddr   StartElement  MaxNum*/
    {MB_BIT_ELEMENT,    MB_BIT_Y,       MB_SECTION_1,   0,          1199,     0,            256},
    {MB_BIT_ELEMENT,    MB_BIT_Y,       MB_SECTION_2,   10000,      11999,    256,          1792},

    {MB_BIT_ELEMENT,    MB_BIT_X,       MB_SECTION_1,   1200,       1999,     0,            256},
    {MB_BIT_ELEMENT,    MB_BIT_X,       MB_SECTION_2,   12000,      13999,    256,          1792},

    {MB_BIT_ELEMENT,    MB_BIT_M,       MB_SECTION_1,   2000,       4399,     0,            2048},
    {MB_BIT_ELEMENT,    MB_BIT_M,       MB_SECTION_2,   34000,      53999,    2048,         14336},

    {MB_BIT_ELEMENT,    MB_BIT_SM,      MB_SECTION_1,   4400,       5999,     0,            256},
    {MB_BIT_ELEMENT,    MB_BIT_SM,      MB_SECTION_2,   14000,      17999,    256,          3840},

    {MB_BIT_ELEMENT,    MB_BIT_S,       MB_SECTION_1,   6000,       7999,     0,            1024},
    {MB_BIT_ELEMENT,    MB_BIT_S,       MB_SECTION_2,   18000,      25999,    1024,         7168},

    {MB_BIT_ELEMENT,    MB_BIT_T,       MB_SECTION_1,   8000,       9199,     0,            256},
    {MB_BIT_ELEMENT,    MB_BIT_T,       MB_SECTION_2,   26000,      29999,    256,          256},

    {MB_BIT_ELEMENT,    MB_BIT_C,       MB_SECTION_1,   9200,       10000,    0,            256},
    {MB_BIT_ELEMENT,    MB_BIT_C,       MB_SECTION_2,   30000,      33999,    256,          256},

    {MB_WORD_ELEMENT,   MB_WORD_D,      MB_SECTION_1,   0,          7999,     0,            8000},

    {MB_WORD_ELEMENT,   MB_WORD_SD,     MB_SECTION_1,   8000,       8499,     0,            256},
    {MB_WORD_ELEMENT,   MB_WORD_SD,     MB_SECTION_2,   46000,      49999,    256,          3840},

    {MB_WORD_ELEMENT,   MB_WORD_Z,      MB_SECTION_1,   8500,       8999,     0,            16},
    {MB_WORD_ELEMENT,   MB_WORD_Z,      MB_SECTION_2,   10000,      11999,    16,           0},

    {MB_WORD_ELEMENT,   MB_WORD_T,      MB_SECTION_1,   9000,       9499,     0,            256},
    {MB_WORD_ELEMENT,   MB_WORD_T,      MB_SECTION_2,   50000,      53999,    256,          256},

    {MB_WORD_ELEMENT,   MB_WORD_C,      MB_SECTION_1,   9500,       9699,     0,            200},
    /*注意：32bit按照两个16bit来读，元件个数翻倍*/
    {MB_WORD_ELEMENT,   MB_WORD_C,      MB_SECTION_2,   9700,       9811,     200,          56 * 2},
    {MB_WORD_ELEMENT,   MB_WORD_C,      MB_SECTION_3,   54000,      61999,    256,          256 * 2},

    {MB_WORD_ELEMENT,   MB_WORD_R,      MB_SECTION_1,   13000,      45999,    0,            32768},

    {MB_ELEMENT_MAX,    0,              0,              0,          0,        0,            0},
};

/**
  * @brief  Modbus 协议地址到物理元件地址转换
  * @param  None
  * @retval None
  */
unsigned char mb_slave_convert_element_info(unsigned char lcv_AddrType, unsigned short lsv_MbAddr, unsigned char *lcp_ElementType, unsigned short *lsp_ElementAddr)
{
    unsigned char i;

    /*根据寻址类型，定位搜索区段*/
    if(lcv_AddrType == MB_BIT_ELEMENT)
    {
        i = 0;
    }
    else
    {
        i = 14;
    }

    while(stv_ModbusMapTable[i].mcv_AddrType != MB_ELEMENT_MAX)
    {
        if(stv_ModbusMapTable[i].mcv_AddrType != lcv_AddrType)
        {
            i++;
            continue;
        }

        if((lsv_MbAddr >= stv_ModbusMapTable[i].msv_StartAddr) &&
                (lsv_MbAddr <= stv_ModbusMapTable[i].msv_EndAddr))
        {
            *lcp_ElementType = stv_ModbusMapTable[i].mcv_ElementType;
            *lsp_ElementAddr = lsv_MbAddr - stv_ModbusMapTable[i].msv_StartAddr + stv_ModbusMapTable[i].msv_StartElement;

            if(*lsp_ElementAddr >= stv_ModbusMapTable[i].msv_StartAddr + stv_ModbusMapTable[i].msv_MaxNum)
            {
                return pdFAIL;
            }

            return pdPASS;
        }

        i++;
    }

    return pdFAIL;
}

