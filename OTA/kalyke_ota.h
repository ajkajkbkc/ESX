/**
  ******************************************************************************
  * @file    kalyke_ota.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-04-25
  * @brief   系统中两个bin的管理
  ******************************************************************************
  */
#ifndef __KALYKE_OTA_H
#define __KALYKE_OTA_H
#include "FreeRTOS.h"
#include "task.h"
#include "fsl_common.h"

extern void ota_task(void *p_arg);
extern uint32_t ota_get_image_idx(void);
extern void ota_reset(uint32_t idx);
extern void ota_save_image_idx(uint32_t idx);
extern status_t ota_program_once_MISC_CONF1(void);
extern uint32_t ota_get_image_id(void);
#endif /* __KALYKE_OTA_H */

