/**
  ******************************************************************************
  * @file    mb_upload.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   Modbus slave …œ‘ÿπ¶ƒ‹
  ******************************************************************************
  */
  
#ifndef __PROTOCOL_MB_UPLOAD_H
#define __PROTOCOL_MB_UPLOAD_H
#include "list_func.h"

#define MAX_UPLOAD_FILE_LENGTH     1000

void mb_slave_upload_manage(md_slave_msg_pack *pMsg);
#endif /*__PROTOCOL_MB_UPLOAD_H*/
