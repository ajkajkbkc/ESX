/**
  ******************************************************************************
  * @file    kalyke_json.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-05-07
  * @brief   json½âÎö
  ******************************************************************************
  */
#ifndef __KALYKE_CJSON_H
#define __KALYKE_CJSON_H

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "FreeRTOS.h"
#include "plc_netcfg.h"
#include "kalyke_internet_task.h"

extern void Kalyke_cJSON_init(void);
extern bool Kalyke_isJsonValidAndParse(const char *pJsonDocument, int *pTokenCount);
extern bool Kalyke_extractHost(const char *pJsonDocument, int32_t tokenCount, char *pHost);
extern bool Kalyke_extractPort(const char *pJsonDocument, int32_t tokenCount, uint16_t *pPort);
extern int8_t Kalyke_extractMqttConfig(const char *pJsonDocument, int32_t tokenCount, mqtt_config_st *pMqttConfig);
extern int8_t Kalyke_extractMqttRecv(const char *pJsonDocument, int32_t tokenCount, mqtt_recv_st *pMqttRecv);
#endif /* __KALYKE_CJSON_H */

