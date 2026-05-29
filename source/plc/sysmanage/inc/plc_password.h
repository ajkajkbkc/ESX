/**
  ******************************************************************************
  * @file    plc_password.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   上下载，监控密码逻辑处理
  ******************************************************************************
  */
#ifndef __PLC_PASSWORD_H
#define __PLC_PASSWORD_H

/*最大密码长度*/
#define MAX_PASSWORD_LEN        8 
/*密码有效期，5分钟*/
#define MAX_PASSWORD_VALIDITY   300000

enum __PLC_PASSWORD_TYPE{
    DOWNLOAD_PASSWORD = 0x00,
    UPLOAD_PASSWORD,
    MONITOR_PASSWORD,
    TIMER_PASSWORD,
    MAX_PASSWORD_NUM
};

/*密码项数据结构*/
typedef struct __PLC_PASSWORD_ST{
    /*密码是否使能*/
    unsigned char mcv_IsEnable;
    /*密码是否验证通过*/
    unsigned char mcv_CheckPass;
    /*密码校验时间，提供五分钟有效期*/
    unsigned char mlv_CheckTime;
    /*密码长度*/
    unsigned char mcv_Len;
    /*密码*/
    unsigned char mcv_Password[MAX_PASSWORD_LEN];
}plc_password_st;

/*系统密码信息结构体*/
typedef struct __PLC_PASSWORD_INFO_ST{
    /*头信息，固定为0xAA55*/
    unsigned short msv_Head;
    /*是否禁止上载*/
    unsigned char mcv_UploadForbid;
    /*密码项*/
    plc_password_st mtv_Password[MAX_PASSWORD_NUM];
    /*CRC32校验和*/
    unsigned long mlv_Crc;
}plc_password_info_st;

extern plc_password_info_st *gtp_PasswordInfo;

void plc_password_init(void);
void plc_read_password_info(void);
unsigned char plc_update_password_info(unsigned char lcv_PwItem,  unsigned char *lcp_NewPw, unsigned char lcv_PwLen);
unsigned char plc_check_password_info(unsigned char lcv_PwItem, unsigned char *ltp_Passwd, unsigned char lcv_PwLen);
unsigned char plc_get_password_check_result(unsigned char lcv_PwItem);
unsigned char plc_set_upload_forbid_flag(unsigned char lcv_Flag);
unsigned char plc_get_upload_forbid_flag(void);
void plc_password_erase(void);

#endif /*__PLC_PASSWORD_H*/
