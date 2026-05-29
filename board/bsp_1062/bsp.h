/**
  ******************************************************************************
  * @file    bsp.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   Header for bsp.c module
  ******************************************************************************
  */
#ifndef __BSP_H
#define __BSP_H

#include "FreeRTOS.h"
#include "fsl_snvs_hp.h"

typedef void(*pVoidFun)(void);

/*设备重启地址*/
#define BSP_TARGET_REBOOT_ADDR      0x08000000


extern void bsp_init_pre(void);
extern void bsp_init_post(void);
extern void bsp_reboot_system(void);
extern void bsp_save_boot_time(void);
extern uint32_t bsp_get_kalyke_boot_time(void);
extern uint32_t kalyke_SNVS_HP_RTC_GetDatetime(snvs_hp_rtc_datetime_t *datetime);
extern void kalyke_getSystemTime(uint32_t *pSecond, uint32_t *pUSecond);

extern bool gBspIam1970;
#endif /*__BSP_H*/

