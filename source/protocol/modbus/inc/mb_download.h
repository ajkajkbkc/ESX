/**
  ******************************************************************************
  * @file    mb_download.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   Modbus slave 下载子功能
  ******************************************************************************
  */

#ifndef __PROTOCOL_MB_DOWNLOAD_H
#define __PROTOCOL_MB_DOWNLOAD_H

/*下载文件信息保存结构体*/
typedef struct __PRO_MB_FILE_TRANS_ST{
    /*总传输帧数*/
    unsigned char mcv_FrameCnt;
    /*上一帧帧号*/
    unsigned char mcv_PreFrame;
    /*标志位: bit0:是否开始传输, bit1:是否传输完成, bit2~bit4:帧号反转次数*/
    unsigned char mcv_Flag;
    /*保留字节*/
    unsigned char mcv_Reserved;

    /*文件长度*/
    unsigned long   mlv_FileLen;
    /*文件缓存区指针*/
    unsigned char * mcp_FileHandler;
}mb_file_trans_st;

/*PLC需要上下载文件列表*/
typedef enum{
    MB_DL_UCODE = 0x00,
    MB_DL_SYS_BLOCK,
    MB_DL_DATA_BLOCK,
    MB_DL_POU_INFO,
    MB_DL_GVT,
    MB_DL_NETCFG,
    
    MB_DL_SYS_UPGRADE,
    MB_DL_PLC_CBIN,

    MB_DL_PID1,
    MB_DL_PID2,
    
    MB_DL_MAX    
}MB_DL_FILE_CNT;

extern mb_file_trans_st *gtv_ModbusFileTrans[MB_DL_MAX];

void mb_slave_download_manage(md_slave_msg_pack *pMsg);
void mb_slave_init_file_info(mb_file_trans_st *gvt_pFile);
#endif /*__PROTOCOL_MB_DOWNLOAD_H*/
