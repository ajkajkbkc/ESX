/**
  ******************************************************************************
  * @file    plc_commonfunc.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   通用函数定义
  ******************************************************************************
  */
#ifndef __PLC_COMMON_FUNC_H
#define __PLC_COMMON_FUNC_H

/*------------------------------------------------------------------------------
* 通用宏定义
*-----------------------------------------------------------------------------*/
/*取指针地址值*/
#define GET_POINT_ADDR(x)           ((unsigned long)(x))

#define GET_PU8_DATA(x)             (*((unsigned char *)(x)))
#define GET_PS8_DATA(x)             (*((char *)(x)))

#define GET_PU16_DATA(x)             (*((unsigned short *)(x)))
#define GET_PS16_DATA(x)             (*((short *)(x)))

#define GET_PU32_DATA(x)             (*((unsigned long *)(x)))
#define GET_PS32_DATA(x)             (*((long *)(x)))

/*按照大端序取值*/
#define GET_BIGPU16_DATA(x)           (unsigned short)((*(x)<<8) + (*(x+1)))
#define GET_BIGPU32_DATA(x)           (unsigned long)((*(x)<<24) + (*(x+1)<<16) + (*(x+2)<<8) + (*(x+3)))

/*按照小端序取值*/
#define GET_SMLPU16_DATA(x)          (unsigned short)((*(x + 1)<<8) + (*(x)))
#define GET_SMLPU32_DATA(x)          (unsigned long)((*(x+3)<<24) + (*(x+2)<<16) + (*(x+1)<<8) + (*(x)))

/*RAM & EEPROM 文件格式相关宏定义*/
/*起始字符*/
#define FILE_START_CHARACTER        (0xAAAA)
/*文件名称起始位置*/
#define FILE_NAME_START_INDEX       6
/*文件长度信息起始位置*/
#define FILE_LEN_INFO_START_INDEX   6
/*文件名长度*/
#define FILE_NAME_LENGTH            16
/*文件信息起始位置*/
#define FILE_INFO_START_INDEX       22
/*文件长度信息长度*/
#define FILE_LEN_INFO_LENGTH        4
/*文件结束字符*/
#define FILE_TAIL_CHARACTER         (0x5555)

/*环形缓冲区节点数量*/
#define RING_BUFFER_NODE_NUM    2

/** @addtogroup Exported_macros
  * @{
  */
#define SET_BIT(REG, BIT)     ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT)   ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT)    ((REG) & (BIT))
#define CLEAR_REG(REG)        ((REG) = (0x0))
#define WRITE_REG(REG, VAL)   ((REG) = (VAL))
#define READ_REG(REG)         ((REG))
#define MODIFY_REG(REG, CLEARMASK, SETMASK)  WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))


typedef struct __RING_BUFFER_ST {
    /*动态分配缓冲区指针*/
    unsigned char * mcp_Buff[RING_BUFFER_NODE_NUM];
    /*当前访问下标*/
    unsigned char mcv_Index;
} ring_buffer_st;
/*------------------------------------------------------------------------------
* 函数定义
*-----------------------------------------------------------------------------*/
unsigned long plc_get_file_length(unsigned char *pData, unsigned short flag);
void plc_refresh_error_msg(unsigned char lcv_ErrorCode);
void plc_refresh_exec_error_record(unsigned char lcv_ErrorCode, unsigned char *lcp_Ucode);

//void ring_buffer_init(ring_buffer_st *ltp_RingBuff, unsigned short lsv_BuffSize);
void ring_buffer_init(ring_buffer_st *ltp_RingBuff, unsigned char uartPort);

void ring_buffer_switch_write_mem(ring_buffer_st *ltp_RingBuff);
unsigned char * ring_buffer_get_write_mem(ring_buffer_st *ltp_RingBuff);

#endif  /*__PLC_COMMON_FUNC_H*/
