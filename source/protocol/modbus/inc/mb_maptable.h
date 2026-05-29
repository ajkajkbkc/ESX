/**
  ******************************************************************************
  * @file    mb_maptable.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   Modbus 元件映射表
  ******************************************************************************
  */
#ifndef __PROTOCOL_MB_MAP_TABLE_H
#define __PROTOCOL_MB_MAP_TABLE_H
/*Modbus元件寻址方式*/
enum __MODBUS_ELEMENT_ADDR_E{
    MB_BIT_ELEMENT  = 0x01,
    MB_WORD_ELEMENT,
    MB_ELEMENT_MAX
};

/*Modbus 元件类型定义*/
enum __MODBUS_ELEMENT_TYPE_E{
    /*Y元件*/
    MB_BIT_Y    = 0x00,
    MB_BIT_X,
    MB_BIT_M,
    MB_BIT_SM,
    MB_BIT_S,
    MB_BIT_T,
    MB_BIT_C,
    MB_WORD_D, //7
    MB_WORD_SD,
    MB_WORD_Z,
    MB_WORD_T,
    MB_WORD_C,
    MB_WORD_R,
};

/*Modbus协议地址段号*/
enum __MODBUS_PRO_ADD_SECTION_E{
    MB_SECTION_1    = 0x01,
    MB_SECTION_2,
    MB_SECTION_3,
};

typedef struct __MODBUS_ELEMENT_MAP_TABLE_INFO_T{
    /*寻址类型：位、字*/
    unsigned char mcv_AddrType;
    /*元件类型*/
    unsigned char mcv_ElementType;
    /*协议地址段号*/
    unsigned char mcv_AddrSection;
    /*开始协议地址*/
    unsigned short msv_StartAddr;
    /*结束协议地址*/
    unsigned short msv_EndAddr;
    /*开始元件编号*/
    unsigned short msv_StartElement;
    /*最大元件数量*/
    unsigned short msv_MaxNum;
}mb_element_map_table_info;

unsigned char mb_slave_convert_element_info(unsigned char lcv_AddrType, unsigned short lsv_MbAddr, unsigned char *lcp_ElementType, unsigned short *lsp_ElementAddr);

#endif /*__PROTOCOL_MB_MAP_TABLE_H*/

