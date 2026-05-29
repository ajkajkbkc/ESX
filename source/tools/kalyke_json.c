/**
  ******************************************************************************
  * @file    kalyke_json.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-05-07
  * @brief   json½âÎö
  ******************************************************************************
  */
#include "kalyke_json.h"
#include "fsl_debug_console.h"
#include "plc_element.h"

#if (KALYKE_CJSON == 0)

#include "jsmn.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef enum
{
    /** Success return value - no error occurred */
    SUCCESS = 0,
    /** A generic error. Not enough information for a specific error code */
    FAILURE = -1,
    /** An error occurred while parsing the JSON string.  Usually malformed JSON. */
    JSON_PARSE_ERROR = -42,
    /** Shadow: The response Ack table is currently full waiting for previously published updates */
    SHADOW_WAIT_FOR_PUBLISH = -43,
    /** Any time an snprintf writes more than size value, this error will be returned */
    SHADOW_JSON_BUFFER_TRUNCATED = -44,
    /** Any time an snprintf encounters an encoding error or not enough space in the given buffer */
    SHADOW_JSON_ERROR = -45,
} JSON_Error_t;

#define UINT8_MAX 	 255
#define INT8_MIN 	-128
#define INT8_MAX 	 127
#define INT16_MIN 	-32768
#define INT16_MAX 	 32767

#if 0
#define __STRINGIFY(a) #a
#define __SCN32(x) __STRINGIFY(l##x)
#define __SCN16(x) __STRINGIFY(h##x)

#define SCNu32		__SCN32(u)
#define SCNu16		__SCN16(u)
#define SCNi32		__SCN32(i)
#endif
/*******************************************************************************
 * Variables
 ******************************************************************************/
static jsmn_parser json_parser;
static jsmntok_t token[256];

/*******************************************************************************
 * Code
 ******************************************************************************/

static int8_t jsoneq(const char *json, jsmntok_t *tok, const char *s)
{
    if(tok->type == JSMN_STRING)
    {
        if((int) strlen(s) == tok->end - tok->start)
        {
            if(strncmp(json + tok->start, s, (size_t) (tok->end - tok->start)) == 0)
            {
                return 0;
            }
        }
    }
    return -1;
}

JSON_Error_t parseUnsignedInteger32Value(uint32_t *i, const char *jsonString, jsmntok_t *token)
{
    if(token->type != JSMN_PRIMITIVE)
    {
        PRINTF("Token was not an integer\r\n");
        return JSON_PARSE_ERROR;
    }

    if(('-' == (char) (jsonString[token->start])) || (1 != sscanf(jsonString + token->start, "%" SCNu32, i)))
    {
        PRINTF("Token was not an unsigned integer.\r\n");
        return JSON_PARSE_ERROR;
    }

    return SUCCESS;
}

static JSON_Error_t parseUnsignedInteger16Value(uint16_t *i, const char *jsonString, jsmntok_t *token)
{
    if(token->type != JSMN_PRIMITIVE)
    {
        PRINTF("Token was not an integer\r\n");
        return JSON_PARSE_ERROR;
    }

    if(('-' == (char) (jsonString[token->start])) || (1 != sscanf(jsonString + token->start, "%" SCNu16, i)))
    {
        PRINTF("Token was not an unsigned integer.\r\n");
        return JSON_PARSE_ERROR;
    }

    return SUCCESS;
}

static JSON_Error_t parseUnsignedInteger8Value(uint8_t *i, const char *jsonString, jsmntok_t *token)
{
    if(token->type != JSMN_PRIMITIVE)
    {
        PRINTF("Token was not an integer\r\n");
        return JSON_PARSE_ERROR;
    }

    uint32_t i_word;
    if(('-' == (char) (jsonString[token->start])) || (1 != sscanf(jsonString + token->start, "%" SCNu32, &i_word)))
    {
        PRINTF("Token was not an unsigned integer.\r\n");
        return JSON_PARSE_ERROR;
    }
    if (i_word > UINT8_MAX)
    {
        PRINTF("Token value %u exceeds 8 bits\r\n", i_word);
        return JSON_PARSE_ERROR;
    }
    *i = i_word;

    return SUCCESS;
}

JSON_Error_t parseInteger32Value(int32_t *i, const char *jsonString, jsmntok_t *token)
{
    if(token->type != JSMN_PRIMITIVE)
    {
        PRINTF("Token was not an integer\r\n");
        return JSON_PARSE_ERROR;
    }

    if(1 != sscanf(jsonString + token->start, "%" SCNi32, i))
    {
        PRINTF("Token was not an integer.\r\n");
        return JSON_PARSE_ERROR;
    }

    return SUCCESS;
}

JSON_Error_t parseInteger16Value(int16_t *i, const char *jsonString, jsmntok_t *token)
{
    if(token->type != JSMN_PRIMITIVE)
    {
        PRINTF("Token was not an integer\r\n");
        return JSON_PARSE_ERROR;
    }

    int32_t i_word;
    if(1 != sscanf(jsonString + token->start, "%" SCNi32, &i_word))
    {
        PRINTF("Token was not an integer.\r\n");
        return JSON_PARSE_ERROR;
    }
    if(i_word < INT16_MIN || i_word > INT16_MAX)
    {
        PRINTF("Token value %d out of range for 16-bit int\r\n", i_word);
        return JSON_PARSE_ERROR;
    }
    *i = i_word;

    return SUCCESS;
}

JSON_Error_t parseInteger8Value(int8_t *i, const char *jsonString, jsmntok_t *token)
{
    if(token->type != JSMN_PRIMITIVE)
    {
        PRINTF("Token was not an integer\r\n");
        return JSON_PARSE_ERROR;
    }

    int32_t i_word;
    if(1 != sscanf(jsonString + token->start, "%" SCNi32, &i_word))
    {
        PRINTF("Token was not an integer.\r\n");
        return JSON_PARSE_ERROR;
    }
    if(i_word < INT8_MIN || i_word > INT8_MAX)
    {
        PRINTF("Token value %d out of range for 8-bit int\r\n", i_word);
        return JSON_PARSE_ERROR;
    }
    *i = i_word;

    return SUCCESS;
}

JSON_Error_t parseFloatValue(float *f, const char *jsonString, jsmntok_t *token)
{
    if(token->type != JSMN_PRIMITIVE)
    {
        PRINTF("Token was not a float.\r\n");
        return JSON_PARSE_ERROR;
    }

    if(1 != sscanf(jsonString + token->start, "%f", f))
    {
        PRINTF("Token was not a float.\r\n");
        return JSON_PARSE_ERROR;
    }

    return SUCCESS;
}

JSON_Error_t parseDoubleValue(double *d, const char *jsonString, jsmntok_t *token)
{
    if(token->type != JSMN_PRIMITIVE)
    {
        PRINTF("Token was not a double.\r\n");
        return JSON_PARSE_ERROR;
    }

    if(1 != sscanf(jsonString + token->start, "%lf", d))
    {
        PRINTF("Token was not a double.\r\n");
        return JSON_PARSE_ERROR;
    }

    return SUCCESS;
}

static JSON_Error_t parseBooleanValue(bool *b, const char *jsonString, jsmntok_t *token)
{
    if(token->type != JSMN_PRIMITIVE)
    {
        PRINTF("Token was not a primitive.\r\n");
        return JSON_PARSE_ERROR;
    }
    if(jsonString[token->start] == 't' && jsonString[token->start + 1] == 'r' && jsonString[token->start + 2] == 'u'
            && jsonString[token->start + 3] == 'e')
    {
        *b = true;
    }
    else if(jsonString[token->start] == 'f' && jsonString[token->start + 1] == 'a'
            && jsonString[token->start + 2] == 'l' && jsonString[token->start + 3] == 's'
            && jsonString[token->start + 4] == 'e')
    {
        *b = false;
    }
    else
    {
        PRINTF("Token was not a bool.\r\n");
        return JSON_PARSE_ERROR;
    }
    return SUCCESS;
}

static JSON_Error_t parseStringValue(char *buf, const char *jsonString, jsmntok_t *token)
{
    uint16_t size = 0;
    if(token->type != JSMN_STRING)
    {
        PRINTF("Token was not a string.\r\n");
        return JSON_PARSE_ERROR;
    }
    size = (uint16_t) (token->end - token->start);
    memcpy(buf, jsonString + token->start, size);
    buf[size] = '\0';
    return SUCCESS;
}

bool Kalyke_extractHost(const char *pJsonDocument, int32_t tokenCount, char *pHost)
{
    int32_t i;
    JSON_Error_t ret_val = SUCCESS;

    for(i = 1; i < tokenCount; i++)
    {
        if(jsoneq(pJsonDocument, &(token[i]), "cmd") == 0)
        {
            ret_val = parseStringValue(pHost, pJsonDocument, &token[i + 1]);
            if(ret_val == SUCCESS)
            {
                return true;
            }
        }
    }
    return false;
}
bool Kalyke_extractPort(const char *pJsonDocument, int32_t tokenCount, uint16_t *pPort)
{
    int32_t i;
    JSON_Error_t ret_val = SUCCESS;

    for(i = 1; i < tokenCount; i++)
    {
        if(jsoneq(pJsonDocument, &(token[i]), "cmd") == 0)
        {
            ret_val = parseUnsignedInteger16Value(pPort, pJsonDocument, &token[i + 1]);
            if(ret_val == SUCCESS)
            {
                return true;
            }
        }
    }
    return false;
}

/**
{
    "host": "broker.hivemq.com",
    "port": 1883,
    "client_id": "kalyke001",
    "username": "",
    "password": "",
    "keep_alive": 120,
    "will_topic": "",
    "will_msg": "",
    "will_qos": 0,
    "will_retain": true
    "publish_topic":"sys/teiobCPILLff/device/plctest/post",
    "subscribe_topic":"sys/teiobCPILLff/device/plctest/set",
    "response_topic":"sys/teiobCPILLff/device/plctest/setback",
    "config":[
            { --- 8 tokens
                    "name":"onoff",  
                    "element":"D",
                    "address":1000,
                    "dataType":"int16"
            },
            {
                    "name":"speed",
                    "element":"D",
                    "address":1001,
                    "dataType":"int16"
            },
            {
                    "name":"ua",
                    "element":"D",
                    "address":1002,
                    "dataType":"float32"
            }
    ]
}
*/

#define ERR_BASE ERR_MQTT_JSON_CONTENT
int8_t Kalyke_extractMqttConfig(const char *pJsonDocument, int32_t tokenCount, mqtt_config_st *pMqttConfig)
{
    int32_t i, j, k;
    JSON_Error_t ret_val = SUCCESS;

    for(i = 1; i < tokenCount; i++)
    {
        if(jsoneq(pJsonDocument, &(token[i]), "vender") == 0)
        {
            ret_val = parseStringValue(pMqttConfig->vender, pJsonDocument, &token[i + 1]);
            if(ret_val != SUCCESS)
            {
                return ERR_MQTT_JSON_CONTENT;
            }
            i++;
        }
        else if(jsoneq(pJsonDocument, &(token[i]), "reporting_cycle") == 0)
        {
            ret_val = parseUnsignedInteger32Value(&pMqttConfig->reportingCycle, pJsonDocument, &token[i + 1]);
            if(ret_val != SUCCESS)
            {
                return (ERR_BASE + 1);
            }
            if (pMqttConfig->reportingCycle < 3)
            {
                pMqttConfig->reportingCycle = 3;
            }
            i++;
        }
        else if(jsoneq(pJsonDocument, &(token[i]), "ProductKey") == 0)
        {
            ret_val = parseStringValue(pMqttConfig->ProductKey, pJsonDocument, &token[i + 1]);
            if(ret_val != SUCCESS)
            {
                return (ERR_BASE + 2);
            }
            i++;
        }
        else if(jsoneq(pJsonDocument, &(token[i]), "ProductSecret") == 0)
        {
            ret_val = parseStringValue(pMqttConfig->ProductSecret, pJsonDocument, &token[i + 1]);
            if(ret_val != SUCCESS)
            {
                return (ERR_BASE + 3);
            }
            i++;
        }
        else if(jsoneq(pJsonDocument, &(token[i]), "DeviceName") == 0)
        {
            ret_val = parseStringValue(pMqttConfig->DeviceName, pJsonDocument, &token[i + 1]);
            if(ret_val != SUCCESS)
            {
                return (ERR_BASE + 4);
            }
            i++;
        }
        else if(jsoneq(pJsonDocument, &(token[i]), "DeviceSecret") == 0)
        {
            ret_val = parseStringValue(pMqttConfig->DeviceSecret, pJsonDocument, &token[i + 1]);
            if(ret_val != SUCCESS)
            {
                return (ERR_BASE + 5);
            }
            i++;
        }
        else if(jsoneq(pJsonDocument, &(token[i]), "host") == 0)
        {
            ret_val = parseStringValue(pMqttConfig->host, pJsonDocument, &token[i + 1]);
            if(ret_val != SUCCESS)
            {
                return (ERR_BASE + 6);
            }
            i++;
        }
        else if(jsoneq(pJsonDocument, &(token[i]), "port") == 0)
        {
            ret_val = parseUnsignedInteger16Value(&pMqttConfig->port, pJsonDocument, &token[i + 1]);
            if(ret_val != SUCCESS)
            {
                return (ERR_BASE + 7);
            }
            i++;
        }
        else if(jsoneq(pJsonDocument, &(token[i]), "client_id") == 0)
        {
            ret_val = parseStringValue(pMqttConfig->client_id, pJsonDocument, &token[i + 1]);
            if(ret_val != SUCCESS)
            {
                return (ERR_BASE + 8);
            }
            i++;
        }
        else if(jsoneq(pJsonDocument, &(token[i]), "username") == 0)
        {
            ret_val = parseStringValue(pMqttConfig->username, pJsonDocument, &token[i + 1]);
            if(ret_val != SUCCESS)
            {
                return (ERR_BASE + 9);
            }
            i++;
        }
        else if(jsoneq(pJsonDocument, &(token[i]), "password") == 0)
        {
            ret_val = parseStringValue(pMqttConfig->password, pJsonDocument, &token[i + 1]);
            if(ret_val != SUCCESS)
            {
                return (ERR_BASE + 10);
            }
            i++;
        }
        else if(jsoneq(pJsonDocument, &(token[i]), "keep_alive") == 0)
        {
            ret_val = parseUnsignedInteger16Value(&pMqttConfig->keepalive, pJsonDocument, &token[i + 1]);
            if(ret_val != SUCCESS)
            {
                return (ERR_BASE + 11);
            }
            i++;
        }
        else if(jsoneq(pJsonDocument, &(token[i]), "will_topic") == 0)
        {
            ret_val = parseStringValue(pMqttConfig->lwt_topic, pJsonDocument, &token[i + 1]);
            if(ret_val != SUCCESS)
            {
                return (ERR_BASE + 12);
            }
            i++;
        }
        else if(jsoneq(pJsonDocument, &(token[i]), "will_msg") == 0)
        {
            ret_val = parseStringValue(pMqttConfig->lwt_msg, pJsonDocument, &token[i + 1]);
            if(ret_val != SUCCESS)
            {
                return (ERR_BASE + 13);
            }
            i++;
        }
        else if(jsoneq(pJsonDocument, &(token[i]), "will_qos") == 0)
        {
            ret_val = parseUnsignedInteger8Value(&pMqttConfig->lwt_qos, pJsonDocument, &token[i + 1]);
            if(ret_val != SUCCESS)
            {
                return (ERR_BASE + 14);
            }
            i++;
        }
        else if(jsoneq(pJsonDocument, &(token[i]), "will_retain") == 0)
        {
            ret_val = parseBooleanValue(&pMqttConfig->lwt_retain, pJsonDocument, &token[i + 1]);
            if(ret_val != SUCCESS)
            {
                return (ERR_BASE + 15);
            }
            i++;
        }
        else if(jsoneq(pJsonDocument, &(token[i]), "publish_topic") == 0)
        {
            ret_val = parseStringValue(pMqttConfig->publish_topic, pJsonDocument, &token[i + 1]);
            if(ret_val != SUCCESS)
            {
                return (ERR_BASE + 16);
            }
            i++;
        }
        else if(jsoneq(pJsonDocument, &(token[i]), "subscribe_topic") == 0)
        {
            ret_val = parseStringValue(pMqttConfig->subscribe_topic, pJsonDocument, &token[i + 1]);
            if(ret_val != SUCCESS)
            {
                return (ERR_BASE + 17);
            }
            i++;
        }
        else if(jsoneq(pJsonDocument, &(token[i]), "response_topic") == 0)
        {
            ret_val = parseStringValue(pMqttConfig->response_topic, pJsonDocument, &token[i + 1]);
            if(ret_val != SUCCESS)
            {
                return (ERR_BASE + 18);
            }
            i++;
        }
        else if(jsoneq(pJsonDocument, &(token[i]), "config") == 0)
        {
            if (token[i + 1].type != JSMN_ARRAY)
            {
                return (ERR_BASE + 19);
            }
            int arraySize = token[i + 1].size;
            if (arraySize <= 0)
            {
                return (ERR_BASE + 20);
            }
            pMqttConfig->configLength = arraySize;
            if (pMqttConfig->pConfigs)
            {
                vPortFree(pMqttConfig->pConfigs);
            }
            pMqttConfig->pConfigs = pvPortMalloc(sizeof(mqtt_command_st) * arraySize);
            mqtt_config_array_st *pcfg = pMqttConfig->pConfigs;
            
            i += 3;
            for (j = 0; j < arraySize; j++)
            {
                for (k = i; k < 8 + i; k++) // There are 8 tokens: name, element, dataType, address
                {
                    if(jsoneq(pJsonDocument, &(token[k]), "name") == 0)
                    {
                        ret_val = parseStringValue(pcfg[j].name, pJsonDocument, &token[k + 1]);
                        if(ret_val != SUCCESS)
                        {
                            return (ERR_BASE + 21);
                        }
                        k++;
                    }
                    else if(jsoneq(pJsonDocument, &(token[k]), "element") == 0)
                    {
                        LOGV("kalyke_json", "token[%d + 1].type = %d", k, token[k + 1].type);
                        ret_val = parseStringValue(pcfg[j].element, pJsonDocument, &token[k + 1]);
                        if(ret_val != SUCCESS)
                        {
                            return (ERR_BASE + 22);
                        }
                        k++;
                    }
                    else if(jsoneq(pJsonDocument, &(token[k]), "dataType") == 0)
                    {
                        ret_val = parseStringValue(pcfg[j].dataType, pJsonDocument, &token[k + 1]);
                        if(ret_val != SUCCESS)
                        {
                            return (ERR_BASE + 23);
                        }
                        k++;
                    }
                    else if(jsoneq(pJsonDocument, &(token[k]), "address") == 0)
                    {
                        ret_val = parseUnsignedInteger16Value(&pcfg[j].address, pJsonDocument, &token[k + 1]);
                        if(ret_val != SUCCESS)
                        {
                            return (ERR_BASE + 24);
                        }
                        k++;
                    }
                }
                LOGD("kalyke_json", "k = %d", k);
                i = k + 1;
            }
        }
        else
        {
            LOGE("kalyke_json", "Unexpected key: %.*s\n", token[i].end - token[i].start,
                 pJsonDocument + token[i].start);
        }
    }
    return 0;
}

int8_t Kalyke_extractMqttRecv(const char *pJsonDocument, int32_t tokenCount, mqtt_recv_st *pMqttRecv)
{
    PRINTF("Enter %s()\r\n", __func__);
    int32_t i, j, k;
    JSON_Error_t ret_val = SUCCESS;

    for(i = 1; i < tokenCount; i++)
    {
        if(jsoneq(pJsonDocument, &(token[i]), "commandType") == 0)
        {
            ret_val = parseStringValue(pMqttRecv->commandType, pJsonDocument, &token[i + 1]);
            if(ret_val != SUCCESS)
            {
                return -1;
            }
            i++;
        }
        else if(jsoneq(pJsonDocument, &(token[i]), "uuid") == 0)
        {
            ret_val = parseStringValue(pMqttRecv->uuid, pJsonDocument, &token[i + 1]);
            if(ret_val != SUCCESS)
            {
                return -2;
            }
            i++;
        }
        else if(jsoneq(pJsonDocument, &(token[i]), "deviceCode") == 0)
        {
            ret_val = parseStringValue(pMqttRecv->deviceCode, pJsonDocument, &token[i + 1]);
            if(ret_val != SUCCESS)
            {
                return -3;
            }
            i++;
        }
        else if(jsoneq(pJsonDocument, &(token[i]), "command") == 0)
        {
            if (token[i + 1].type != JSMN_ARRAY)
            {
                return -4;
            }
            int arraySize = token[i + 1].size;
            if (arraySize <= 0)
            {
                return -5;
            }
            pMqttRecv->cmdLength = arraySize;
            pMqttRecv->pCmd = pvPortMalloc(sizeof(mqtt_command_st) * arraySize);
            mqtt_command_st *pCommand = pMqttRecv->pCmd;
            
            i += 3;
            for (j = 0; j < arraySize; j++)
            {
                for (k = i; k < 8 + i; k++)
                {
                    if(jsoneq(pJsonDocument, &(token[k]), "name") == 0)
                    {
                        ret_val = parseStringValue(pCommand[j].name, pJsonDocument, &token[k + 1]);
                        if(ret_val != SUCCESS)
                        {
                            return -6;
                        }
                        k++;
                    }
                    else if(jsoneq(pJsonDocument, &(token[k]), "value") == 0)
                    {
                        LOGV("kalyke_json", "token[%d + 1].type = %d", k, token[k + 1].type);
                        ret_val = parseFloatValue(&pCommand[j].valueFloat, pJsonDocument, &token[k + 1]);
                        ret_val = parseInteger32Value(&pCommand[j].valueInt32, pJsonDocument, &token[k + 1]);
                        ret_val = parseInteger16Value(&pCommand[j].valueInt16, pJsonDocument, &token[k + 1]);
                        ret_val = parseBooleanValue(&pCommand[j].valueBool, pJsonDocument, &token[k + 1]);
                        if(ret_val != SUCCESS)
                        {
                            //return -7;
                        }
                        k++;
                    }
                    else if(jsoneq(pJsonDocument, &(token[k]), "dataType") == 0)
                    {
                        ret_val = parseStringValue(pCommand[j].dataType, pJsonDocument, &token[k + 1]);
                        if(ret_val != SUCCESS)
                        {
                            return -8;
                        }
                        k++;
                    }
                    else if(jsoneq(pJsonDocument, &(token[k]), "address") == 0)
                    {
                        ret_val = parseUnsignedInteger16Value(&pCommand[j].address, pJsonDocument, &token[k + 1]);
                        if(ret_val != SUCCESS)
                        {
                            return -9;
                        }
                        k++;
                    }
                }
                LOGD("kalyke_json", "k = %d", k);
                i = k + 1;
            }
        }
        else
        {
            LOGE("kalyke_json", "Unexpected key: %.*s", token[i].end - token[i].start,
                 pJsonDocument + token[i].start);
        }
        
    }
    return 0;
}

bool Kalyke_isJsonValidAndParse(const char *pJsonDocument, int *pTokenCount)
{
    int tokenCount;

    //IOT_UNUSED(pJsonHandler);

    jsmn_init(&json_parser);

    tokenCount = jsmn_parse(&json_parser, pJsonDocument, strlen(pJsonDocument), token,
                            sizeof(token) / sizeof(token[0]));

    if(tokenCount < 0)
    {
        PRINTF("Failed to parse JSON: %d\r\n", tokenCount);
        SET_SD_ELEMENT_VALUE(SD229, ERR_MQTT_JSON_NO_MEMORY);
        return false;
    }

    /* Assume the top-level element is an object */
    if(tokenCount < 1 || token[0].type != JSMN_OBJECT)
    {
        PRINTF("Top Level is not an object\r\n");
        SET_SD_ELEMENT_VALUE(SD229, ERR_MQTT_JSON_NOT_OBJECT);
        return false;
    }

    PRINTF("token[0].size = %d, tokenCount = %d\r\n", token[0].size, tokenCount);
    *pTokenCount = tokenCount;

    return true;
}

#if 0
// 13 token
static const char *JSON_STRING =
    "{\"user\": \"johndoe\", \"admin\": false, \"uid\": 1000,\n  "
    "\"groups\": [\"users\", \"wheel\", \"audio\", \"video\"]}";
#else
// 27 token
static const char *JSON_STRING =
    "{\"commandType\": \"modbuswrt\", \"uuid\": \"asdfasdfjklasjdvjwq\", \"deviceCode\": \"testplc\",\n  "
    "\"command\": [{\"name\":\"temperature\", \"value\":1.23, \"dataType\":\"float32\", \"address\":1000},\n  "
    "{\"name\":\"second\", \"value\":209, \"dataType\":\"int16\", \"address\":1002}]}";

#endif

#if 0
int Kalyke_test_json(void)
{
    int i;
#if 0 
    int r;
    jsmn_parser p;
    jsmntok_t t[128]; /* We expect no more than 128 tokens */

    jsmn_init(&p);
    r = jsmn_parse(&p, JSON_STRING, strlen(JSON_STRING), t, sizeof(t) / sizeof(t[0]));
    LOGD("kalyke_json", "tokenCount = %d, t[0].size = %d", r, t[0].size);
    if (r < 0)
    {
        LOGE("kalyke_json", "Failed to parse JSON: %d\n", r);
        return 1;
    }

    /* Assume the top-level element is an object */
    if (r < 1 || t[0].type != JSMN_OBJECT)
    {
        LOGE("kalyke_json", "Object expected\n");
        return 1;
    }
    #else
    int tokenCount;
    if (Kalyke_isJsonValidAndParse(JSON_STRING, &tokenCount) == false)
    {
        LOGE("mqtt", "This is not json data, just reutrn.");
        return -99;
    }
    #endif

#if 1
    mqtt_recv_st mr = {0};
    int8_t ret = Kalyke_extractMqttRecv(JSON_STRING, tokenCount, &mr);
    LOGI("kalyke_json", "Kalyke_extractMqttRecv return : %d", ret);
    LOGW("kalyke_json", "mr.commandType : %s", mr.commandType);
    LOGW("kalyke_json", "mr.uuid : %s", mr.uuid);
    LOGW("kalyke_json", "mr.deviceCode : %s", mr.deviceCode);

    LOGW("kalyke_json", "mr.cmdLength : %d", mr.cmdLength);
    for (i = 0; i < mr.cmdLength; i++)
    {
        LOGV("kalyke_json", "mr.pCmd[%d].name : %s", i, mr.pCmd[i].name);
        LOGV("kalyke_json", "mr.pCmd[%d].valueFloat : %f", i, mr.pCmd[i].valueFloat);
        LOGV("kalyke_json", "mr.pCmd[%d].dataType : %s", i, mr.pCmd[i].dataType);
        LOGV("kalyke_json", "mr.pCmd[%d].address : %u\r\n", i, mr.pCmd[i].address);
    }
#else
    /* Loop over all keys of the root object */
    for (i = 1; i < r; i++)
    {
        if (jsoneq(JSON_STRING, &t[i], "user") == 0)
        {
            /* We may use strndup() to fetch string value */
            LOGV("kalyke_json", "- User: %.*s\n", t[i + 1].end - t[i + 1].start,
                 JSON_STRING + t[i + 1].start);
            i++;
        }
        else if (jsoneq(JSON_STRING, &t[i], "admin") == 0)
        {
            /* We may additionally check if the value is either "true" or "false" */
            LOGW("kalyke_json", "- Admin: %.*s\n", t[i + 1].end - t[i + 1].start,
                 JSON_STRING + t[i + 1].start);
            i++;
        }
        else if (jsoneq(JSON_STRING, &t[i], "uid") == 0)
        {
            /* We may want to do strtol() here to get numeric value */
            LOGI("kalyke_json", "- UID: %.*s\n", t[i + 1].end - t[i + 1].start,
                 JSON_STRING + t[i + 1].start);
            i++;
        }
        else if (jsoneq(JSON_STRING, &t[i], "groups") == 0)
        {
            int j;
            LOGD("kalyke_json", "- Groups:\n");
            if (t[i + 1].type != JSMN_ARRAY)
            {
                continue; /* We expect groups to be an array of strings */
            }
            LOGV("kalyke_json", "t[i + 1].size = %d", t[i + 1].size);
            LOGV("kalyke_json", "t[i + 2].size = %d", t[i + 2].size);
            for (j = 0; j < t[i + 1].size; j++)
            {
                jsmntok_t *g = &t[i + j + 2];
                LOGV("kalyke_json", "  * %.*s\n", g->end - g->start, JSON_STRING + g->start);
            }
            i += t[i + 1].size + 1;
        }
        else
        {
            LOGE("kalyke_json", "Unexpected key: %.*s\n", t[i].end - t[i].start,
                 JSON_STRING + t[i].start);
        }
    }
#endif
    return 0;
}
#endif

#endif

