/**
  ******************************************************************************
  * @file    plc_parseaddr.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   UCODE参数寻址解析相关函数
  ******************************************************************************
  */
#ifndef __PLC_PARSE_ADDR_H
#define __PLC_PARSE_ADDR_H

/*UCODE参数寻址类型表*/
#define ADDR_CONST      0xFF
#define ADDR_X          0x00
#define ADDR_Y          0x01
#define ADDR_bC         0x02
#define ADDR_bT         0x03
#define ADDR_SM         0x04
#define ADDR_M          0x05
#define ADDR_S          0x06
#define ADDR_LM         0x07
#define ADDR_DX_Y       0x09

#define ADDR_D          0x11
#define ADDR_Z          0x12
#define ADDR_SD         0x13
#define ADDR_C          0x14
#define ADDR_T          0x15
#define ADDR_V          0x16
#define ADDR_R          0x1A

/*位串组合寻址*/
#define ADDR_KnX        0x0C
#define ADDR_KnY        0x0D
#define ADDR_KnM        0x0E
#define ADDR_KnS        0x0F
#define ADDR_KnLM       0x10
#define ADDR_KnSM       0x17

/*变址寻址*/
#define ADDR_RZ         0x31
#define ADDR_DZ         0x32
#define ADDR_CZ         0x33
#define ADDR_TZ         0x34
#define ADDR_VZ         0x3C

#define ADDR_XZ         0x35
#define ADDR_YZ         0x36
#define ADDR_MZ         0x37
#define ADDR_SZ         0x38
#define ADDR_LMZ        0x39
#define ADDR_bTZ        0x3A
#define ADDR_bCZ        0x3B

/*位串组合变址寻址*/
#define ADDR_KnXZ       0x46
#define ADDR_KnYZ       0x50
#define ADDR_KnMZ       0x5A
#define ADDR_KnSZ       0x64
#define ADDR_KnLMZ      0x6E

/*字符串寻址类型*/
#define ADDR_STR        0x82

unsigned char get_char(unsigned char *lcp_UcodeAddr, unsigned char *lcp_Value, unsigned short lsv_Offset, unsigned short lsv_Count);
unsigned char get_char_default(unsigned char *lcp_UcodeAddr, unsigned char *lcp_Value);
unsigned char save_char(unsigned char *lcp_UcodeAddr, unsigned char *lcp_Value, unsigned short lsv_Offset, unsigned short lsv_Count);
unsigned char save_char_default(unsigned char *lcp_UcodeAddr, unsigned char *lcp_Value);

unsigned char get_word(unsigned char *lcp_UcodeAddr, unsigned short *lsp_Value, unsigned short lsv_Offset, unsigned short lsv_Count);
unsigned char get_word_default(unsigned char *lcp_UcodeAddr, unsigned short *lsp_Value);
unsigned char get_word_addr(unsigned char *lcp_UcodeAddr, unsigned short **lsp_Addr, unsigned short lsv_Num);
unsigned char save_word(unsigned char *lcp_UcodeAddr, unsigned short *lsp_Value, unsigned short lsv_Offset, unsigned short lsv_Count);
unsigned char save_word_default(unsigned char *lcp_UcodeAddr, unsigned short *lsp_Value);

unsigned char get_dword(unsigned char *lcp_UcodeAddr, unsigned long *llp_Value, unsigned short lsv_Offset, unsigned short lsv_Count);
unsigned char get_dword_addr(unsigned char *lcp_UcodeAddr, unsigned long **llp_Addr, unsigned short lsv_Num);
unsigned char save_dword(unsigned char *lcp_UcodeAddr, unsigned long *llp_Value, unsigned short lsv_Offset, unsigned short lsv_Count);

unsigned char get_float(unsigned char *lcp_UcodeAddr, float *lfp_Value, unsigned short lsv_Offset, unsigned short lsv_Count);
unsigned char save_float(unsigned char *lcp_UcodeAddr, float *lfp_Value, unsigned short lsv_Offset, unsigned short lsv_Count);

#endif /*__PLC_PARSE_ADDR_H*/

