/**
  ******************************************************************************
  * @file    kalyke_cJSON.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2020-07-07
  * @brief   json解析, port from https://github.com/DaveGamble/cJSON
  ******************************************************************************
  */
#include "kalyke_cJSON.h"
#include "fsl_debug_console.h"
#include "plc_element.h"

#if (KALYKE_CJSON == 1)
#include "cJSON.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ERR_BASE ERR_MQTT_JSON_CONTENT

/*******************************************************************************
 * Variables
 ******************************************************************************/
static const char *TAG = "Kalyke_CJSON";

/*******************************************************************************
 * Code
 ******************************************************************************/

void Kalyke_cJSON_init(void)
{
    cJSON_Hooks hook = {
            .malloc_fn = pvPortMalloc,
            .free_fn = vPortFree
        };
    cJSON_InitHooks(&hook);
}

uint8_t Str2Elem(char *pstr)
{
    uint8_t Elem;

    if (memcmp(pstr, "M", 1) == 0)
    {
        Elem = ELEM_M;
    }
    else if (memcmp(pstr, "X", 1) == 0)
    {
        Elem = ELEM_X;
    }
    else if (memcmp(pstr, "Y", 1) == 0)
    {
        Elem = ELEM_Y;
    }
    else if (memcmp(pstr, "S", 1) == 0)
    {
        Elem = ELEM_S;
    }
    else if (memcmp(pstr, "SM", 2) == 0)
    {
        Elem = ELEM_SM;
    }
    else if (memcmp(pstr, "D", 1) == 0)
    {
        Elem = ELEM_D;
    }
    else if (memcmp(pstr, "R", 1) == 0)
    {
        Elem = ELEM_R;
    }
    else if (memcmp(pstr, "C", 1) == 0)
    {
        Elem = ELEM_C;
    }
    else if (memcmp(pstr, "SD", 2) == 0)
    {
        Elem = ELEM_SD;
    }
    else
    {
        LOGE("JSON_Parse", "element type error !");
        Elem = ELEM_M;
    }

    return Elem;
}

uint8_t Str2DType(char *pStr)
{
    uint8_t DType;

    if (strcmp(pStr, "float32") == 0)
    {
        DType = DTYPE_F32;
    }
    else if (strcmp(pStr, "uint16") == 0)
    {
        DType = DTYPE_U16;
    }
    else if (strcmp(pStr, "int16") == 0)
    {
        DType = DTYPE_I16;
    }
    else if (strcmp(pStr, "uint32") == 0)
    {
        DType = DTYPE_U32;
    }
    else if (strcmp(pStr, "int32") == 0)
    {
        DType = DTYPE_I32;
    }
    else if (strcmp(pStr, "bool") == 0)
    {
        DType = DTYPE_BOOL;
    }
    else
    {
        LOGE("JSON_Parse", "data type error !");
        DType = DTYPE_I16;
    }

    return DType;
}

uint8_t Str2Symbol(char *pStr)
{
    uint8_t Sym;

    if (strcmp(pStr, ">") == 0)
    {
        Sym = SYM_GT;
    }
    else if (strcmp(pStr, ">=") == 0)
    {
        Sym = SYM_GTE;
    }
    else if (strcmp(pStr, "<") == 0)
    {
        Sym = SYM_LT;
    }
    else if (strcmp(pStr, "<=") == 0)
    {
        Sym = SYM_LTE;
    }
    else if (strcmp(pStr, "==") == 0 || strcmp(pStr, "=") == 0)
    {
        Sym = SYM_EQUAL;
    }
    else if (strcmp(pStr, "<>") == 0)
    {
        Sym = SYM_UNEQUAL;
    }
    else if (strcmp(pStr, "CHG") == 0)
    {
        Sym = SYM_CHG;
    }
    else if (strcmp(pStr, "CFG") == 0)
    {
        Sym = SYM_CFG;
    }
    else
    {
        Sym = SYM_IDLE;
    }

    return Sym;
}

int8_t Kalyke_extractMqttConfig(const char *pJsonDocument, int32_t tokenCount, mqtt_config_st *pMqttConfig)
{
    LOGV(TAG, "Enter %s(), Free heap: %d(bytes)", __func__, xPortGetFreeHeapSize());
    cJSON *json = cJSON_Parse(pJsonDocument);
    if (json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            LOGE(TAG, "Error before: %s\r\n", error_ptr);
        }
        return ERR_MQTT_JSON_NOT_OBJECT;
    }
    LOGW(TAG, "After cJSON_Parse, free heap: %d(bytes)", xPortGetFreeHeapSize());

    int8_t retVal = 0;
    cJSON *pJSONTemp = NULL;
    cJSON *pConfnigs = NULL;
    cJSON *pAConfig = NULL;
    int arraySize;
    mqtt_config_array_st *pcfg;
    int j = 0;
    char ctemp[32];

    if (cJSON_IsObject(json) == false)
    {
        LOGE(TAG, "This is not object!");
        retVal = ERR_MQTT_JSON_NOT_OBJECT;
        goto ENDME;
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "vender"); // Must exist
    if (cJSON_IsString(pJSONTemp) == false)
    {
        retVal = ERR_MQTT_JSON_CONTENT; // vender parse error.
        goto ENDME;
    }
    strncpy(pMqttConfig->vender, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttConfig->vender) - 1);

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "reporting_cycle"); // Not necessary
    if (cJSON_IsNumber(pJSONTemp) == true)
    {
        pMqttConfig->reportingCycle = cJSON_GetNumberValue(pJSONTemp);
        if (pMqttConfig->reportingCycle < 10)
        {
            pMqttConfig->reportingCycle = 10;
        }
    }
    else
    {
        pMqttConfig->reportingCycle = 10;
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "ProductKey"); // Not necessary
    if (pJSONTemp != NULL)
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            retVal = (ERR_BASE + 2);
            goto ENDME;
        }
        strncpy(pMqttConfig->ProductKey, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttConfig->ProductKey) - 1);
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "ProductSecret"); // Not necessary
    if (pJSONTemp != NULL)
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            retVal = (ERR_BASE + 3);
            goto ENDME;
        }
        strncpy(pMqttConfig->ProductSecret, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttConfig->ProductSecret) - 1);
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "DeviceName"); // Not necessary
    if (pJSONTemp != NULL)
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            retVal = (ERR_BASE + 4);
            goto ENDME;
        }
        strncpy(pMqttConfig->DeviceName, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttConfig->DeviceName) - 1);
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "DeviceSecret"); // Not necessary
    if (pJSONTemp != NULL)
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            retVal = (ERR_BASE + 5);
            goto ENDME;
        }
        strncpy(pMqttConfig->DeviceSecret, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttConfig->DeviceSecret) - 1);
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "host"); // Must exist
    if (cJSON_IsString(pJSONTemp) == false)
    {
        retVal = (ERR_BASE + 6);
        goto ENDME;
    }
    strncpy(pMqttConfig->host, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttConfig->host) - 1);

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "port"); // Must exist
    if (cJSON_IsNumber(pJSONTemp) == false)
    {
        retVal = (ERR_BASE + 7);
        goto ENDME;
    }
    pMqttConfig->port = cJSON_GetNumberValue(pJSONTemp);

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "client_id"); // Not necessary
    if (pJSONTemp != NULL)
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            retVal = (ERR_BASE + 8);
            goto ENDME;
        }
        strncpy(pMqttConfig->client_id, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttConfig->client_id) - 1);
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "username"); // Must exist
    if (cJSON_IsString(pJSONTemp) == false)
    {
        retVal = (ERR_BASE + 9);
        goto ENDME;
    }
    strncpy(pMqttConfig->username, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttConfig->username) - 1);

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "password"); // Must exist
    if (cJSON_IsString(pJSONTemp) == false)
    {
        retVal = (ERR_BASE + 10);
        goto ENDME;
    }
    strncpy(pMqttConfig->password, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttConfig->password) - 1);

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "keep_alive"); // Must exist
    if (cJSON_IsNumber(pJSONTemp) == false)
    {
        retVal = (ERR_BASE + 11);
        goto ENDME;
    }
    pMqttConfig->keepalive = cJSON_GetNumberValue(pJSONTemp);

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "will_topic"); // Not necessary
    if (pJSONTemp != NULL)
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            retVal = (ERR_BASE + 12);
            goto ENDME;
        }
        strncpy(pMqttConfig->lwt_topic, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttConfig->lwt_topic) - 1);
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "will_msg"); // Not necessary
    if (pJSONTemp != NULL)
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            retVal = (ERR_BASE + 13);
            goto ENDME;
        }
        strncpy(pMqttConfig->lwt_msg, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttConfig->lwt_msg) - 1);
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "will_qos"); // Not necessary
    if (pJSONTemp != NULL)
    {
        if (cJSON_IsNumber(pJSONTemp) == false)
        {
            retVal = (ERR_BASE + 14);
            goto ENDME;
        }
        pMqttConfig->lwt_qos = cJSON_GetNumberValue(pJSONTemp);
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "will_retain"); // Not necessary
    if (pJSONTemp != NULL)
    {
        if (cJSON_IsBool(pJSONTemp) == false)
        {
            retVal = (ERR_BASE + 15);
            goto ENDME;
        }
        pMqttConfig->lwt_retain = pJSONTemp->valueint == 1 ? true : false;
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "publish_topic"); // Not necessary
    if (pJSONTemp != NULL)
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            retVal = (ERR_BASE + 16);
            goto ENDME;
        }
        strncpy(pMqttConfig->publish_topic, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttConfig->publish_topic) - 1);
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "publish_alarm"); // Not necessary
    if (pJSONTemp != NULL)
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            retVal = (ERR_BASE + 25);
            goto ENDME;
        }
        strncpy(pMqttConfig->publish_topic_alarm, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttConfig->publish_topic_alarm) - 1);
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "subscribe_topic"); // Not necessary
    if (pJSONTemp != NULL)
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            retVal = (ERR_BASE + 17);
            goto ENDME;
        }
        strncpy(pMqttConfig->subscribe_topic, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttConfig->subscribe_topic) - 1);
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "subscribe_reboot"); // Not necessary
    if (pJSONTemp != NULL)
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            retVal = (ERR_BASE + 26);
            goto ENDME;
        }
        strncpy(pMqttConfig->subscribe_topic_reboot, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttConfig->subscribe_topic_reboot) - 1);
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "subscribe_pub_cycle"); // Not necessary
    if (pJSONTemp != NULL)
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            retVal = (ERR_BASE + 27);
            goto ENDME;
        }
        strncpy(pMqttConfig->subscribe_topic_pub_cycle, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttConfig->subscribe_topic_pub_cycle) - 1);
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "subscribe_pub_now"); // Not necessary
    if (pJSONTemp != NULL)
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            retVal = (ERR_BASE + 28);
            goto ENDME;
        }
        strncpy(pMqttConfig->subscribe_topic_pub_now, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttConfig->subscribe_topic_pub_now) - 1);
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "subscribe_pause"); // Not necessary
    if (pJSONTemp != NULL)
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            retVal = (ERR_BASE + 29);
            goto ENDME;
        }
        strncpy(pMqttConfig->subscribe_topic_pause, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttConfig->subscribe_topic_pause) - 1);
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "serial_number"); // Not necessary
    if (pJSONTemp != NULL)
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            retVal = (ERR_BASE + 30);
            goto ENDME;
        }
        strncpy(pMqttConfig->serial_number, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttConfig->serial_number) - 1);
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "response_topic"); // Not necessary
    if (pJSONTemp != NULL)
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            retVal = (ERR_BASE + 18);
            goto ENDME;
        }
        strncpy(pMqttConfig->response_topic, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttConfig->response_topic) - 1);
    }

#if 0
    if (strcmp(pMqttConfig->vender, "HANYU") == 0)
    {
        LOGW(TAG, "goto HANYU_CONFIG");
        goto HANYU_CONFIG;
    }
#endif

    pConfnigs = cJSON_GetObjectItemCaseSensitive(json, "config"); // Not necessary
    if (pConfnigs == NULL)
    {
        LOGW(TAG, "\"config\" is NULL, so let's goto HANYU_CONFIG");
        goto HANYU_CONFIG;
    }
    LOGV(TAG, "pConfnigs = 0x%08X", pConfnigs);
    if (cJSON_IsArray(pConfnigs) == false) // If not a array
    {
        retVal = (ERR_BASE + 19);
        goto ENDME;
    }
    arraySize = cJSON_GetArraySize(pConfnigs);
    if (arraySize <= 0)
    {
        retVal = (ERR_BASE + 20);
        goto ENDME;
    }

    pMqttConfig->configLength = arraySize;
    if (pMqttConfig->pConfigs)
    {
        vPortFree(pMqttConfig->pConfigs);
    }
    pMqttConfig->pConfigs = pvPortMalloc(sizeof(mqtt_config_array_st) * arraySize);
    memset(pMqttConfig->pConfigs, 0, sizeof(mqtt_config_array_st) * arraySize);
    pcfg = pMqttConfig->pConfigs;
    cJSON_ArrayForEach(pAConfig, pConfnigs)
    {
        pcfg[j].alarmType = false;

        cJSON *nameJSON     = cJSON_GetObjectItemCaseSensitive(pAConfig, "name");
        if (cJSON_IsString(nameJSON) == false)
        {
            retVal = (ERR_BASE + 21);
            goto ENDME;
        }
        strncpy(pcfg[j].name, cJSON_GetStringValue(nameJSON), sizeof(pcfg[j].name) - 1);

        cJSON *elemJSON     = cJSON_GetObjectItemCaseSensitive(pAConfig, "element");
        if (cJSON_IsString(elemJSON) == false)
        {
            retVal = (ERR_BASE + 22);
            goto ENDME;
        }
#if 0
        strncpy(pcfg[j].element, cJSON_GetStringValue(elemJSON), sizeof(pcfg[j].element) - 1);
#else
        strncpy(ctemp, cJSON_GetStringValue(elemJSON), sizeof(ctemp) - 1);
        pcfg[j].element = Str2Elem(ctemp);
#endif

        cJSON *dataTypeJSON = cJSON_GetObjectItemCaseSensitive(pAConfig, "dataType");
        if (cJSON_IsString(dataTypeJSON) == false)
        {
            retVal = (ERR_BASE + 23);
            goto ENDME;
        }
#if 0
        strncpy(pcfg[j].dataType, cJSON_GetStringValue(dataTypeJSON), sizeof(pcfg[j].dataType) - 1);
#else
        strncpy(ctemp, cJSON_GetStringValue(dataTypeJSON), sizeof(ctemp) - 1);
        pcfg[j].dataType = Str2DType(ctemp);
#endif

        cJSON *addrJSON     = cJSON_GetObjectItemCaseSensitive(pAConfig, "address");
        if (cJSON_IsNumber(addrJSON) == false)
        {
            retVal = (ERR_BASE + 24);
            goto ENDME;
        }
        pcfg[j].address = cJSON_GetNumberValue(addrJSON);

        /* "alarm":{"sym":">", "val":"240", "content":"温度超过上限"} */
        cJSON *alarmJSON = cJSON_GetObjectItemCaseSensitive(pAConfig, "alarm"); // Not necessary
        if (cJSON_IsObject(alarmJSON) == true)
        {
            cJSON *pTempJSON = cJSON_GetObjectItemCaseSensitive(alarmJSON, "sym");
            if (cJSON_IsString(pTempJSON) == false)
            {
                retVal = (ERR_BASE + 25);
                goto ENDME;
            }
            char *pSymbol = cJSON_GetStringValue(pTempJSON);
            pcfg[j].sym = Str2Symbol(pSymbol);

            pTempJSON = cJSON_GetObjectItemCaseSensitive(alarmJSON, "val");
            if (pTempJSON == NULL)
            {
                #if 1
                pTempJSON = cJSON_GetObjectItemCaseSensitive(alarmJSON, "threshold");
                if (pTempJSON == NULL)
                {
                    pTempJSON = cJSON_GetObjectItemCaseSensitive(alarmJSON, "value");
                    if (pTempJSON == NULL)
                    {
                        retVal = (ERR_BASE + 26);
                        goto ENDME;
                    }
                }
                #else
                retVal = (ERR_BASE + 26);
                goto ENDME;
                #endif
            }
            if (cJSON_IsString(pTempJSON) == true)
            {
                pcfg[j].alarmVal = atof(cJSON_GetStringValue(pTempJSON));
            }
            else if (cJSON_IsNumber(pTempJSON) == true)
            {
                pcfg[j].alarmVal = cJSON_GetNumberValue(pTempJSON);
            }

            pTempJSON = cJSON_GetObjectItemCaseSensitive(alarmJSON, "content");// Not necessary
            if (cJSON_IsString(pTempJSON) == true)
            {
                strncpy(pcfg[j].alarmContent, cJSON_GetStringValue(pTempJSON), sizeof(pcfg[j].alarmContent) - 1);
            }
        }

        j++;
    }
    
HANYU_CONFIG:
    {
        int report_content_array_size = 0;

        cJSON *pConfig_report = cJSON_GetObjectItemCaseSensitive(json, "config_report"); // Not necessary
        if (pConfig_report == NULL)
        {
            LOGW(TAG, "There is no \"config_report\". pConfnigs = 0x%08X", pConfnigs);
            if (pConfnigs == NULL)
            {
                retVal = ERR_BASE + 100;
            }
            goto ENDME;
        }
        LOGV(TAG, "pConfig_report = 0x%08X", pConfig_report);
        if (cJSON_IsArray(pConfig_report) == false) // If not a array
        {
            retVal = (ERR_BASE + 26);
            goto ENDME;
        }
        arraySize = cJSON_GetArraySize(pConfig_report);
        LOGD(TAG, "config_report array size is %d", arraySize);
        if (arraySize <= 0)
        {
            retVal = (ERR_BASE + 27);
            goto ENDME;
        }

        pMqttConfig->configLength = arraySize;
        if (pMqttConfig->pConfigsHANYU)
        {
            vPortFree(pMqttConfig->pConfigsHANYU);
            if (pMqttConfig->pConfigsHANYU->pReportContent)
            {
                vPortFree(pMqttConfig->pConfigsHANYU->pReportContent);
            }
        }
        pMqttConfig->pConfigsHANYU = pvPortMalloc(sizeof(mqtt_config_array_hanyu_st) * arraySize);
        memset(pMqttConfig->pConfigsHANYU, 0, sizeof(mqtt_config_array_hanyu_st) * arraySize);
        mqtt_config_array_hanyu_st *pCfgHY = pMqttConfig->pConfigsHANYU;

        j = 0;
        cJSON_ArrayForEach(pAConfig, pConfig_report)
        {
            cJSON *report_cycleJSON = cJSON_GetObjectItemCaseSensitive(pAConfig, "report_cycle");// Must exist
            if (cJSON_IsNumber(report_cycleJSON) == false)
            {
                retVal = (ERR_BASE + 28);
                goto ENDME;
            }
            pCfgHY[j].report_cycle = cJSON_GetNumberValue(report_cycleJSON);
            if (pCfgHY[j].report_cycle < 10)
            {
                pCfgHY[j].report_cycle = 10;
            }

            cJSON *slave_idJSON = cJSON_GetObjectItemCaseSensitive(pAConfig, "slave_id");
            if (cJSON_IsNumber(slave_idJSON) == true)
            {
                pCfgHY[j].slave_id = cJSON_GetNumberValue(slave_idJSON);
            }
            cJSON *slave_nameJSON = cJSON_GetObjectItemCaseSensitive(pAConfig, "slave_name");
            if (cJSON_IsString(slave_nameJSON) == true)
            {
                strncpy(pCfgHY[j].slave_name, cJSON_GetStringValue(slave_nameJSON), sizeof(pCfgHY[j].slave_name) - 1);
            }

            cJSON *report_contentJSON = cJSON_GetObjectItemCaseSensitive(pAConfig, "report_content");// Must exist
            if (cJSON_IsArray(report_contentJSON) == false)
            {
                retVal = (ERR_BASE + 29);
                goto ENDME;
            }
            report_content_array_size = cJSON_GetArraySize(report_contentJSON);
            LOGD(TAG, "report_content_array_size is %d", report_content_array_size);
            if (report_content_array_size <= 0)
            {
                retVal = (ERR_BASE + 30);
                goto ENDME;
            }
            pCfgHY[j].reportContentLen = report_content_array_size;
            pCfgHY[j].pReportContent = pvPortMalloc(sizeof(mqtt_config_array_st) * report_content_array_size);
            LOGW(TAG, "After pCfgHY[%d].pReportContent.reportContentLen: %d(bytes), free heap: %d(bytes)", j, sizeof(mqtt_config_array_st) * report_content_array_size, xPortGetFreeHeapSize());
            memset(pCfgHY[j].pReportContent, 0, sizeof(mqtt_config_array_st) * report_content_array_size);
            pcfg = pCfgHY[j].pReportContent;
            int i = 0;
            cJSON *pEachContent;
            cJSON_ArrayForEach(pEachContent, report_contentJSON)
            {
                pcfg[i].alarmType = false;

                cJSON *nameJSON     = cJSON_GetObjectItemCaseSensitive(pEachContent, "name");
                if (cJSON_IsString(nameJSON) == false)
                {
                    retVal = (ERR_BASE + 31);
                    goto ENDME;
                }
                strncpy(pcfg[i].name, cJSON_GetStringValue(nameJSON), sizeof(pcfg[i].name) - 1);

                cJSON *elemJSON     = cJSON_GetObjectItemCaseSensitive(pEachContent, "element");
                if (cJSON_IsString(elemJSON) == false)
                {
                    retVal = (ERR_BASE + 32);
                    goto ENDME;
                }
#if 0
                strncpy(pcfg[i].element, cJSON_GetStringValue(elemJSON), sizeof(pcfg[i].element) - 1);
#else
                strncpy(ctemp, cJSON_GetStringValue(elemJSON), sizeof(ctemp) - 1);
                pcfg[i].element = Str2Elem(ctemp);
#endif

                cJSON *dataTypeJSON = cJSON_GetObjectItemCaseSensitive(pEachContent, "dataType");
                if (cJSON_IsString(dataTypeJSON) == false)
                {
                    retVal = (ERR_BASE + 33);
                    goto ENDME;
                }
#if 0
                strncpy(pcfg[i].dataType, cJSON_GetStringValue(dataTypeJSON), sizeof(pcfg[i].dataType) - 1);
#else
                strncpy(ctemp, cJSON_GetStringValue(dataTypeJSON), sizeof(ctemp) - 1);
                pcfg[i].dataType = Str2DType(ctemp);
#endif

                cJSON *addrJSON     = cJSON_GetObjectItemCaseSensitive(pEachContent, "address");
                if (cJSON_IsNumber(addrJSON) == false)
                {
                    retVal = (ERR_BASE + 34);
                    goto ENDME;
                }
                pcfg[i].address = cJSON_GetNumberValue(addrJSON);

                /* "alarm":{"sym":">", "val":"240", "content":"温度超过上限"} */
                cJSON *alarmJSON = cJSON_GetObjectItemCaseSensitive(pEachContent, "alarm"); // Not necessary
                if (cJSON_IsObject(alarmJSON) == true)
                {
                    cJSON *pTempJSON = cJSON_GetObjectItemCaseSensitive(alarmJSON, "sym");
                    if (cJSON_IsString(pTempJSON) == false)
                    {
                        retVal = (ERR_BASE + 35);
                        goto ENDME;
                    }
                    char *pSymbol = cJSON_GetStringValue(pTempJSON);
                    pcfg[i].sym = Str2Symbol(pSymbol);

                    pTempJSON = cJSON_GetObjectItemCaseSensitive(alarmJSON, "val");
                    if (pTempJSON == NULL)
                    {
                        pTempJSON = cJSON_GetObjectItemCaseSensitive(alarmJSON, "threshold");
                        if (pTempJSON == NULL)
                        {
                            pTempJSON = cJSON_GetObjectItemCaseSensitive(alarmJSON, "value");
                            if (pTempJSON == NULL)
                            {
                                retVal = (ERR_BASE + 36);
                                goto ENDME;
                            }
                        }
                    }
                    if (cJSON_IsString(pTempJSON) == true)
                    {
                        pcfg[i].alarmVal = atof(cJSON_GetStringValue(pTempJSON));
                    }
                    else if (cJSON_IsNumber(pTempJSON) == true)
                    {
                        pcfg[i].alarmVal = cJSON_GetNumberValue(pTempJSON);
                    }

                    pTempJSON = cJSON_GetObjectItemCaseSensitive(alarmJSON, "content");// Not necessary
                    if (cJSON_IsString(pTempJSON) == true)
                    {
                        strncpy(pcfg[i].alarmContent, cJSON_GetStringValue(pTempJSON), sizeof(pcfg[i].alarmContent) - 1);
                    }
                }
                i++;
            }// cJSON_ArrayForEach(pEachContent, report_contentJSON)
            j++;
        }// cJSON_ArrayForEach(pAConfig, pConfig_report)
    }// HANYU_CONFIG
ENDME:
    cJSON_Delete(json);
    LOGD(TAG, "Leave %s(), Free heap: %d(bytes)", __func__, xPortGetFreeHeapSize());
    return retVal;
}

#if 1 //KALYKE_NEW_MQTT
/*
{
    "device_id":"21120BID0001",
    "format":"keyword",
    "uuid":"adc0123456789def",
    "data":[
        {
            "name":"button1",
            "value":1
        },
        {
            "name":"button2",
            "value":0
        }
    ]
}
{
    "device_id":"21120BID0001",
    "format":"element",
    "uuid":"adc0123456789def",
    "data":[
        {
            "name":"D",
            "address":100,
            "data_type":"int16",
            "value":1
        },
        {
            "name":"D",
            "address":102,
            "data_type":"int16",
            "value":1000
        }
    ]
}
*/
static int8_t Kalyke_extractMqttRecv_DEFAULTMQTT(cJSON *json, mqtt_recv_st *pMqttRecv)
{
    LOGV(TAG, "Enter %s(), Free heap: %d(bytes)", __func__, xPortGetFreeHeapSize());
    int arraySize;
    int j = 0;
    mqtt_command_st *pCmd;
    cJSON *pAData;
    cJSON *pDatas;
    cJSON *pJSONTemp;
    char ctemp[32];

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "device_id");
    if (pJSONTemp == NULL)
    {
        return -11;
    }
    else
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            return -12;
        }
        strncpy(pMqttRecv->deviceCode, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttRecv->deviceCode) - 1);
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "slave_id");
    if (pJSONTemp == NULL)
    {
        pMqttRecv->slave_id = -1;
        //return -25;  //不一定都要有slave_id
    }
    else
    {
        if (cJSON_IsNumber(pJSONTemp) == false)
        {
            return -26;
        }
        pMqttRecv->slave_id = cJSON_GetNumberValue(pJSONTemp);
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "slave_name");
    if (pJSONTemp == NULL)
    {
        memset(pMqttRecv->slave_name, 0, sizeof(pMqttRecv->slave_name) - 1);
        //return -27;  //不一定都要有slave_name
    }
    else
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            return -28;
        }
        strncpy(pMqttRecv->slave_name, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttRecv->slave_name) - 1);
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "format");
    if (pJSONTemp == NULL)
    {
        return -13;
    }
    else
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            return -14;
        }
        strncpy(pMqttRecv->commandType, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttRecv->commandType) - 1);
        if ( (strcmp(pMqttRecv->commandType, "keyword") != 0) && (strcmp(pMqttRecv->commandType, "element") != 0) )
        {
            return -15;
        }
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "uuid");
    if (pJSONTemp == NULL)
    {
        return -16;
    }
    else
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            return -17;
        }
        strncpy(pMqttRecv->uuid, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttRecv->uuid) - 1);
    }

    pDatas = cJSON_GetObjectItemCaseSensitive(json, "data"); // Not necessary
    if (pDatas == NULL)
    {
        LOGE(TAG, "There is no data keyword!");
        return -18;
    }
    if (cJSON_IsArray(pDatas) == false) // If not a array
    {
        return -19;
    }
    arraySize = cJSON_GetArraySize(pDatas);
    if (arraySize <= 0)
    {
        return -20;
    }

    pMqttRecv->cmdLength = arraySize;
    pMqttRecv->pCmd = pvPortMalloc(sizeof(mqtt_command_st) * arraySize);
    memset(pMqttRecv->pCmd, 0, sizeof(mqtt_command_st) * arraySize);
    pCmd = pMqttRecv->pCmd;
    pAData = NULL;
    if (strcmp(pMqttRecv->commandType, "keyword") == 0)
    {
        cJSON_ArrayForEach(pAData, pDatas)
        {
            pJSONTemp = cJSON_GetObjectItemCaseSensitive(pAData, "name");
            if (cJSON_IsString(pJSONTemp) == false)
            {
                return -21;
            }
            strncpy(pCmd[j].name, cJSON_GetStringValue(pJSONTemp), sizeof(pCmd[j].name) - 1);


            pJSONTemp = cJSON_GetObjectItemCaseSensitive(pAData, "value");
            if (cJSON_IsNumber(pJSONTemp) == true)
            {
                LOGI(TAG, "value is a number.");
                pCmd[j].valueFloat = cJSON_GetNumberValue(pJSONTemp);
                pCmd[j].valueInt32 = cJSON_GetNumberValue(pJSONTemp);
                pCmd[j].valueInt16 = cJSON_GetNumberValue(pJSONTemp);
                pCmd[j].valueBool = pCmd[j].valueInt16;
            }
            else if (cJSON_IsString(pJSONTemp) == true)
            {
                LOGI(TAG, "value is a string.");
                pCmd[j].valueFloat = atof(cJSON_GetStringValue(pJSONTemp));
                pCmd[j].valueInt32 = atoi(cJSON_GetStringValue(pJSONTemp));
                pCmd[j].valueInt16 = pCmd[j].valueInt32;
                pCmd[j].valueBool = pCmd[j].valueInt16;
            }
            j++;
        }
    }
    else if (strcmp(pMqttRecv->commandType, "element") == 0)
    {
        cJSON_ArrayForEach(pAData, pDatas)
        {
            pJSONTemp = cJSON_GetObjectItemCaseSensitive(pAData, "name");
            if (cJSON_IsString(pJSONTemp) == false)
            {
                return -22;
            }
#if 0
            strncpy(pCmd[j].element, cJSON_GetStringValue(pJSONTemp), sizeof(pCmd[j].element) - 1);
#else
            strncpy(ctemp, cJSON_GetStringValue(pJSONTemp), sizeof(ctemp) - 1);
            pCmd[j].element = Str2Elem(ctemp);
#endif

            pJSONTemp = cJSON_GetObjectItemCaseSensitive(pAData, "address");
            if (cJSON_IsNumber(pJSONTemp) == false)
            {
                return -23;
            }
            pCmd[j].address = cJSON_GetNumberValue(pJSONTemp);

            pJSONTemp = cJSON_GetObjectItemCaseSensitive(pAData, "data_type");
            if (cJSON_IsString(pJSONTemp) == false)
            {
                return -24;
            }
#if 0
            strncpy(pCmd[j].dataType, cJSON_GetStringValue(pJSONTemp), sizeof(pCmd[j].dataType) - 1);
#else
            strncpy(ctemp, cJSON_GetStringValue(pJSONTemp), sizeof(ctemp) - 1);
            pCmd[j].dataType = Str2DType(ctemp);
#endif

            pJSONTemp = cJSON_GetObjectItemCaseSensitive(pAData, "value");
            if (cJSON_IsNumber(pJSONTemp) == true)
            {
                LOGI(TAG, "value is a number.");
                pCmd[j].valueFloat = cJSON_GetNumberValue(pJSONTemp);
                pCmd[j].valueInt32 = cJSON_GetNumberValue(pJSONTemp);
                pCmd[j].valueInt16 = cJSON_GetNumberValue(pJSONTemp);
                pCmd[j].valueBool = pCmd[j].valueInt16;
            }
            else if (cJSON_IsString(pJSONTemp) == true)
            {
                LOGI(TAG, "value is a string.");
                pCmd[j].valueFloat = atof(cJSON_GetStringValue(pJSONTemp));
                pCmd[j].valueInt32 = atoi(cJSON_GetStringValue(pJSONTemp));
                pCmd[j].valueInt16 = pCmd[j].valueInt32;
                pCmd[j].valueBool = pCmd[j].valueInt16;
            }
            j++;
        }
    }
    return 0;
}
#endif

/*
{
    "id": "2ui7SBz6bzzawjrL",
    "type": "write_variant",
    "version": "1.0",
    "time": 1600324099000,
    "params": {
        "开关1": 1
    }
}
*/
/*
{
    "id": "2ui7SBz6bzzawjrL",
    "type": "write_variant",
    "version": "1.0",
    "time": 1600324099000,
    "params": {
        "name" : "D", // D or R
        "data_type" : "int16", // int16, int32, float32, bool
        "address" : 100, //寄存器地址
        "value" : 36,    //寄存器地址的值
    }
} 
*/
static int8_t Kalyke_extractMqttRecv_JIONTECH(cJSON *json, mqtt_recv_st *pMqttRecv)
{
    LOGV(TAG, "Enter %s(), Free heap: %d(bytes)", __func__, xPortGetFreeHeapSize());
    int arraySize;
    int j = 0;
    mqtt_command_st *pCmd;
    cJSON *pAData;
    cJSON *pDatas;
    cJSON *pJSONTemp;
    char ctemp[32];

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "id");
    if (pJSONTemp == NULL)
    {
        return -11;
    }
    else
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            return -12;
        }
        strncpy(pMqttRecv->uuid, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttRecv->uuid) - 1);
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "type");
    if (pJSONTemp == NULL)
    {
        return -13;
    }
    else
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            return -14;
        }
        strncpy(pMqttRecv->commandType, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttRecv->commandType) - 1);
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "version");
    if (pJSONTemp == NULL)
    {
        return -15;
    }
    else
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            return -16;
        }
        strncpy(pMqttRecv->deviceCode, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttRecv->deviceCode) - 1);
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "time"); // Not necessary
    if (pJSONTemp == NULL)
    {
        LOGW(TAG, "No time key");
    }
    else
    {
        if (cJSON_IsNumber(pJSONTemp) == false)
        {
            LOGW(TAG, "time key is not number.");
        }
        pMqttRecv->time = cJSON_GetNumberValue(pJSONTemp);
    }

    pDatas = cJSON_GetObjectItemCaseSensitive(json, "params");
    if (pDatas == NULL)
    {
        LOGE(TAG, "There is no params keyword!");
        return -17;
    }
    if (cJSON_IsArray(pDatas) == false) // If not a array
    {
        return -18;
    }
    arraySize = cJSON_GetArraySize(pDatas);
    if (arraySize <= 0)
    {
        return -19;
    }

    pMqttRecv->cmdLength = arraySize;
    pMqttRecv->pCmd = pvPortMalloc(sizeof(mqtt_command_st) * arraySize);
    memset(pMqttRecv->pCmd, 0, sizeof(mqtt_command_st) * arraySize);
    pCmd = pMqttRecv->pCmd;
    pAData = NULL;

    cJSON_ArrayForEach(pAData, pDatas)
    {
        pJSONTemp = cJSON_GetObjectItemCaseSensitive(pAData, "name");
        if (cJSON_IsString(pJSONTemp) == false)
        {
            return -22;
        }
#if 0
        strncpy(pCmd[j].element, cJSON_GetStringValue(pJSONTemp), sizeof(pCmd[j].element) - 1);
#else
        strncpy(ctemp, cJSON_GetStringValue(pJSONTemp), sizeof(ctemp) - 1);
        pCmd[j].element = Str2Elem(ctemp);
#endif

        pJSONTemp = cJSON_GetObjectItemCaseSensitive(pAData, "address");
        if (cJSON_IsNumber(pJSONTemp) == false)
        {
            return -23;
        }
        pCmd[j].address = cJSON_GetNumberValue(pJSONTemp);

        pJSONTemp = cJSON_GetObjectItemCaseSensitive(pAData, "data_type");
        if (cJSON_IsString(pJSONTemp) == false)
        {
            return -24;
        }
#if 0
        strncpy(pCmd[j].dataType, cJSON_GetStringValue(pJSONTemp), sizeof(pCmd[j].dataType) - 1);
#else
        strncpy(ctemp, cJSON_GetStringValue(pJSONTemp), sizeof(ctemp) - 1);
        pCmd[j].dataType = Str2DType(ctemp);
#endif

        pJSONTemp = cJSON_GetObjectItemCaseSensitive(pAData, "value");
        if (cJSON_IsNumber(pJSONTemp) == true)
        {
            LOGI(TAG, "value is a number.");
            pCmd[j].valueFloat = cJSON_GetNumberValue(pJSONTemp);
            pCmd[j].valueInt32 = cJSON_GetNumberValue(pJSONTemp);
            pCmd[j].valueInt16 = cJSON_GetNumberValue(pJSONTemp);
            pCmd[j].valueBool = pCmd[j].valueInt16;
        }
        else if (cJSON_IsString(pJSONTemp) == true)
        {
            LOGI(TAG, "value is a string.");
            pCmd[j].valueFloat = atof(cJSON_GetStringValue(pJSONTemp));
            pCmd[j].valueInt32 = atoi(cJSON_GetStringValue(pJSONTemp));
            pCmd[j].valueInt16 = pCmd[j].valueInt32;
            pCmd[j].valueBool = pCmd[j].valueInt16;
        }
        j++;
    }
    return 0;
}


/*
    { 
        "Data":[ 
            {"name":"Temp","value":"30"} 
        ]
    }
*/
static int8_t Kalyke_extractMqttRecv_HANYU(cJSON *json, mqtt_recv_st *pMqttRecv)
{
    LOGV(TAG, "Enter %s(), Free heap: %d(bytes)", __func__, xPortGetFreeHeapSize());
    int arraySize;
    int j = 0;
    mqtt_command_st *pCmd;
    cJSON *pAData;
    cJSON *pDatas;
    cJSON *pJSONTemp;

    pDatas = cJSON_GetObjectItemCaseSensitive(json, "Data"); // Not necessary
    if (pDatas == NULL)
    {
        LOGE(TAG, "There is no Data keyword!");
        return -11;
    }
    if (cJSON_IsArray(pDatas) == false) // If not a array
    {
        return -12;
    }
    arraySize = cJSON_GetArraySize(pDatas);
    if (arraySize <= 0)
    {
        return -13;
    }

    pMqttRecv->cmdLength = arraySize;
    pMqttRecv->pCmd = pvPortMalloc(sizeof(mqtt_command_st) * arraySize);
    memset(pMqttRecv->pCmd, 0, sizeof(mqtt_command_st) * arraySize);
    pCmd = pMqttRecv->pCmd;
    pAData = NULL;
    cJSON_ArrayForEach(pAData, pDatas)
    {
        pJSONTemp = cJSON_GetObjectItemCaseSensitive(pAData, "name");
        if (cJSON_IsString(pJSONTemp) == false)
        {
            return -14;
        }
        strncpy(pCmd[j].name, cJSON_GetStringValue(pJSONTemp), sizeof(pCmd[j].name) - 1);


        pJSONTemp = cJSON_GetObjectItemCaseSensitive(pAData, "value");
        if (cJSON_IsNumber(pJSONTemp) == true)
        {
            LOGI(TAG, "value is a number.");
            pCmd[j].valueFloat = cJSON_GetNumberValue(pJSONTemp);
            pCmd[j].valueInt32 = cJSON_GetNumberValue(pJSONTemp);
            pCmd[j].valueInt16 = cJSON_GetNumberValue(pJSONTemp);
            pCmd[j].valueBool = pCmd[j].valueInt16;
        }
        else if (cJSON_IsString(pJSONTemp) == true)
        {
            LOGI(TAG, "value is a string.");
            pCmd[j].valueFloat = atof(cJSON_GetStringValue(pJSONTemp));
            pCmd[j].valueInt32 = atoi(cJSON_GetStringValue(pJSONTemp));
            pCmd[j].valueInt16 = pCmd[j].valueInt32;
            pCmd[j].valueBool = pCmd[j].valueInt16;
        }
        j++;
    }
    return 0;
}

/*
{
    "commandType":"modbuswrt",
    "uuid":"abcd1234567",
    "command":[
    {
        "name":"D",
        "value":11,
        "dataType":"int16",
        "address":100
    },
    {
        "name":"D",
        "value":10010,
        "dataType":"int16",
        "address":102
    }
    ]
}
*/
int8_t Kalyke_extractMqttRecv(const char *pJsonDocument, int32_t tokenCount, mqtt_recv_st *pMqttRecv)
{
    LOGV(TAG, "Enter %s(), Free heap: %d(bytes)", __func__, xPortGetFreeHeapSize());
    cJSON *json = cJSON_Parse(pJsonDocument);
    if (json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            LOGE(TAG, "Error before: %s\r\n", error_ptr);
        }
        SET_SD_ELEMENT_VALUE(SD229, ERR_MQTT_JSON_NOT_OBJECT);
        LOGW(TAG, "Leave %s(), Free heap: %d(bytes)", __func__, xPortGetFreeHeapSize());
        return ERR_MQTT_JSON_NOT_OBJECT;
    }

    int8_t retVal = 0;
    cJSON *pJSONTemp = NULL;
    cJSON *pCommands = NULL;
    cJSON *pACommand = NULL;
    mqtt_command_st *pCmd;
    char ctemp[32];
    int j = 0;
    int arraySize;

    if (cJSON_IsObject(json) == false)
    {
        LOGE(TAG, "This is not object!");
        retVal = ERR_MQTT_JSON_NOT_OBJECT;
        goto ENDME;
    }

#if 1 //KALYKE_NEW_MQTT
    if (memcmp(g_plc_netcfg.mqtt.vender, "DEFAULTMQTT", 11) == 0 || memcmp(g_plc_netcfg.mqtt.vender, "GDMQTT", 6) == 0)
    {
        retVal = Kalyke_extractMqttRecv_DEFAULTMQTT(json, pMqttRecv);
        goto ENDME;
    }
    if (memcmp(g_plc_netcfg.mqtt.vender, "HANYU", 5) == 0)
    {
        retVal = Kalyke_extractMqttRecv_HANYU(json, pMqttRecv);
        goto ENDME;
    }
    if (memcmp(g_plc_netcfg.mqtt.vender, "JIONTECH", 8) == 0)
    {
        retVal = Kalyke_extractMqttRecv_JIONTECH(json, pMqttRecv);
        goto ENDME;
    }
#else
    if (memcmp(g_plc_netcfg.mqtt.vender, "HANYU", 5) == 0 || memcmp(g_plc_netcfg.mqtt.vender, "DEFAULTMQTT", 11) == 0)
    {
        retVal = Kalyke_extractMqttRecv_HANYU(json, pMqttRecv);
        goto ENDME;
    }
#endif

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "commandType"); // Not necessary
    if (pJSONTemp != NULL)
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            retVal = -1;
            goto ENDME;
        }
        strncpy(pMqttRecv->commandType, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttRecv->commandType) - 1);
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "uuid"); // Not necessary
    if (pJSONTemp != NULL)
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            retVal = -2;
            goto ENDME;
        }
        strncpy(pMqttRecv->uuid, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttRecv->uuid) - 1);
    }

    pJSONTemp = cJSON_GetObjectItemCaseSensitive(json, "deviceCode"); // Not necessary
    if (pJSONTemp != NULL)
    {
        if (cJSON_IsString(pJSONTemp) == false)
        {
            retVal = -3;
            goto ENDME;
        }
        strncpy(pMqttRecv->deviceCode, cJSON_GetStringValue(pJSONTemp), sizeof(pMqttRecv->deviceCode) - 1);
    }

    pCommands = cJSON_GetObjectItemCaseSensitive(json, "command"); // Not necessary
    if (pCommands == NULL)
    {
        return 0;
    }
    if (cJSON_IsArray(pCommands) == false) // If not a array
    {
        retVal = -4;
        goto ENDME;
    }
    arraySize = cJSON_GetArraySize(pCommands);
    if (arraySize <= 0)
    {
        retVal = -5;
        goto ENDME;
    }

    pMqttRecv->cmdLength = arraySize;
    pMqttRecv->pCmd = pvPortMalloc(sizeof(mqtt_command_st) * arraySize);
    memset(pMqttRecv->pCmd, 0, sizeof(mqtt_command_st) * arraySize);
    pCmd = pMqttRecv->pCmd;
    pACommand = NULL;
    cJSON_ArrayForEach(pACommand, pCommands)
    {
        pJSONTemp = cJSON_GetObjectItemCaseSensitive(pACommand, "name");
        if (cJSON_IsString(pJSONTemp) == false)
        {
            retVal = -6;
            goto ENDME;
        }
        strncpy(pCmd[j].name, cJSON_GetStringValue(pJSONTemp), sizeof(pCmd[j].name) - 1);

        pJSONTemp = cJSON_GetObjectItemCaseSensitive(pACommand, "value");
        if (cJSON_IsNumber(pJSONTemp) == true)
        {
            pCmd[j].valueFloat = cJSON_GetNumberValue(pJSONTemp);
            pCmd[j].valueInt32 = cJSON_GetNumberValue(pJSONTemp);
            pCmd[j].valueInt16 = cJSON_GetNumberValue(pJSONTemp);
            pCmd[j].valueBool = pCmd[j].valueInt16;
        }

        pJSONTemp = cJSON_GetObjectItemCaseSensitive(pACommand, "dataType");
        if (cJSON_IsString(pJSONTemp) == false)
        {
            retVal = -8;
            goto ENDME;
        }
#if 0
        strncpy(pCmd[j].dataType, cJSON_GetStringValue(pJSONTemp), sizeof(pCmd[j].dataType) - 1);
#else
        strncpy(ctemp, cJSON_GetStringValue(pJSONTemp), sizeof(ctemp) - 1);
        pCmd[j].dataType = Str2DType(ctemp);
#endif

        pJSONTemp = cJSON_GetObjectItemCaseSensitive(pACommand, "address");
        if (cJSON_IsNumber(pJSONTemp) == false)
        {
            retVal = -9;
            goto ENDME;
        }
        pCmd[j].address = cJSON_GetNumberValue(pJSONTemp);
        j++;
    }
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ENDME:
    cJSON_Delete(json);
    LOGD(TAG, "Leave %s(), Free heap: %d(bytes)", __func__, xPortGetFreeHeapSize());
    return retVal;
}

#endif /* KALYKE_CJSON == 1 */

