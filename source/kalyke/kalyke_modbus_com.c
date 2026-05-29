
#include <stdio.h>

#include "plc_errormsg.h"
#include "bsp_uart.h"
#include "verify_func.h"
#include "plc_netcfg.h"
#include "kalyke_event.h"
#include "bsp_iwdg.h"
#include "kalyke_internet_task.h"
#include "kalyke_oled_key_task.h"
#include "kalyke_collect_task.h"
static const char *TAG = "MODBUS";

//#define KALYKE_MODBUS_DEBUG

#ifdef KALYKE_MODBUS_DEBUG
#define LOGE_MODLINK    LOGE
#define LOGW_MODLINK    LOGW
#define LOGI_MODLINK    LOGI
#define LOGD_MODLINK    LOGD
#define LOGV_MODLINK    LOGV
#else
#define LOGE_MODLINK(...)
#define LOGW_MODLINK(...)
#define LOGI_MODLINK(...)
#define LOGD_MODLINK(...)
#define LOGV_MODLINK(...)
#endif

static TimerHandle_t gModbusRespTimer[MAX_SUPPORT_UART_PORT];
static TaskHandle_t gModbusHandle[MAX_SUPPORT_UART_PORT];
static bool gRespTimeout[MAX_SUPPORT_UART_PORT];
volatile uint8_t collect_switch;

static bool ifTriggered(modbus_item_st *pModbusItem)
{
    //LOGV(TAG, "Enter %s(), triggerType = %u, triggerAddr = %u, commType = %u", __func__, pModbusItem->triggerType, pModbusItem->triggerAddr, pModbusItem->commType);
    if (pModbusItem->commType == 0)
    {
        return true;
    }
    if (pModbusItem->triggerType == 1) // M element
    {
        if (plc_get_bit_element_value(M_ELEMENT, pModbusItem->triggerAddr))
        {
            return true;
        }
    }
    else // S element
    {
        if (plc_get_bit_element_value(S_ELEMENT, pModbusItem->triggerAddr))
        {
            return true;
        }
    }
    return false;
}

static void kalyke_modbus_next_index(unsigned short lsv_PortNum, modbus_cfg_st *ltp_ModlinkHead)
{
    ltp_ModlinkHead->index[lsv_PortNum]++;
    if (ltp_ModlinkHead->index[lsv_PortNum] >= ltp_ModlinkHead->listNum)
    {
        ltp_ModlinkHead->index[lsv_PortNum] = 0;
    }
}

static void resp_time_out(TimerHandle_t ltv_TimeHandle)
{
    uint16_t lsv_PortNum = (uint32_t)pvTimerGetTimerID(ltv_TimeHandle);
    LOGI_MODLINK(TAG, "Enter %s(), lsv_PortNum(%u)", __func__, lsv_PortNum);
    
    modbus_cfg_st *ltp_ModlinkHead = &g_modbus_cfg;
    modbus_item_st *ltp_ModlinkItem = &ltp_ModlinkHead->listPtr[ltp_ModlinkHead->index[lsv_PortNum]];
    
    gRespTimeout[lsv_PortNum] = true;
    *(ltp_ModlinkItem->execResult) = MODLINK_RUN_TIMEOUT;
    RST_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH);
    RST_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR);
    ltp_ModlinkItem->isExec = 0;     
    kalyke_modbus_next_index(lsv_PortNum, ltp_ModlinkHead);

    xEventGroupSetBits(g_kalyke_event_group, KALYKE_EVENT_UART_COM0_HAD_RESP << lsv_PortNum);
}

static void kalyke_modbus_start_resp_time_out_timer(uint16_t lsv_PortNum, uint32_t timeout)
{
    //bsp_feed_watch_dog();
    LOGV_MODLINK(TAG, "Enter %s(), lsv_PortNum: %u", __func__, lsv_PortNum);
    gRespTimeout[lsv_PortNum] = false;
    if (gModbusRespTimer[lsv_PortNum] == NULL)
    {
        if (timeout < 2000)
        {
            timeout = 2000;
        }

        char timerName[configMAX_TASK_NAME_LEN] = {0};
        sprintf(timerName, "modbus_timeout%u", lsv_PortNum);
        gModbusRespTimer[lsv_PortNum] = xTimerCreate((const char *)timerName,
                                     (TickType_t  )timeout / portTICK_PERIOD_MS,
                                     (UBaseType_t )pdFALSE,
                                     (void *      )(uint32_t)lsv_PortNum,
                                     (TimerCallbackFunction_t)resp_time_out);
        LOGD_MODLINK(TAG, "Create %s ok.", timerName);
    }
    xTimerStart(gModbusRespTimer[lsv_PortNum], 10);
}

static void kalyke_modbus_stop_resp_time_out_timer(uint16_t lsv_PortNum)
{
    LOGD_MODLINK(TAG, "Enter %s(), lsv_PortNum: %u", __func__, lsv_PortNum);
    if (gModbusRespTimer[lsv_PortNum])
    {
        xTimerStop(gModbusRespTimer[lsv_PortNum], 10);
    }
}

static int8_t kalyke_modbus_com_task_do(unsigned short lsv_PortNum)
{
    LOGV_MODLINK(TAG, "Enter %s(), lsv_PortNum(%u)", __func__, lsv_PortNum);

    uint8_t lcv_Ret;
    unsigned short lsv_SheetNum;
    unsigned short lsv_EdgeId;

    modbus_cfg_st *ltp_ModlinkHead = &g_modbus_cfg;
    modbus_item_st *ltp_ModlinkItem;

    uart_port_info_st *ltp_UartPort;
    uint8_t lcv_SuspendFlag;
    unsigned short i, j;
    unsigned short lsv_TxLength, lsv_RxLength;
    uint8_t *lcp_UartTxBuff = NULL;
    unsigned short lsv_TxCrc;
    
    /*取通信串口号*/
//  lsv_PortNum = g_modbus_cfg.listPtr[g_modbus_cfg.index].port;
    ltp_ModlinkItem = &ltp_ModlinkHead->listPtr[ltp_ModlinkHead->index[lsv_PortNum]];

    /*串口不匹配*/
    if(ltp_ModlinkItem->port != lsv_PortNum)
    {
        kalyke_modbus_next_index(lsv_PortNum, ltp_ModlinkHead);
        return pdPASS;
    }

    LOGW_MODLINK(TAG, "lsv_PortNum = %u, slaveAddr = %u", lsv_PortNum, ltp_ModlinkItem->slaveAddr);
    
    
    if(lsv_PortNum >= gtv_DeviceConfigTable.mtv_UartPort.mcv_SupportUartNum)
    {
        *(ltp_ModlinkItem->execResult) = MODLINK_COM_ERR; 
        LOGE_MODLINK(TAG, "Port number so big, leave %s()...001", __func__);
        kalyke_modbus_next_index(lsv_PortNum, ltp_ModlinkHead);
        return ERR_MODBUS_TBL;
    }

    ltp_UartPort = &gtp_UartPort[lsv_PortNum];
    LOGV_MODLINK(TAG, "ltp_UartPort->mcv_Mode = %u", ltp_UartPort->mcv_Mode);
    if(ltp_UartPort->mcv_Mode != UART_TYPE_MB_MASTER)
    {
        *(ltp_ModlinkItem->execResult) = MODLINK_COMCFG_ERR;
        SET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH);
        /*设置主从模式错误标志*/
        SET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR);
        SET_UART_SD_VALUE(lsv_PortNum, UART_SD_MASTER_ERROR_CODE, MB_SERVER_SLAVE_MODE_ERR);
        LOGW_MODLINK(TAG, "Uart %u mode is not master, leave %s()...003", lsv_PortNum, __func__);
        kalyke_modbus_next_index(lsv_PortNum, ltp_ModlinkHead);
        /*返回系统块配置错误*/
        return ERR_MODBUS_TBL;
    }

    LOGI_MODLINK(TAG, "ltp_ModlinkHead = 0x%08X", ltp_ModlinkHead);
    LOGV_MODLINK(TAG, "ltp_ModlinkHead->index = %u, ltp_ModlinkHead->listNum = %u", ltp_ModlinkHead->index[lsv_PortNum], ltp_ModlinkHead->listNum);

    if (ifTriggered(ltp_ModlinkItem) == false)
    {
        *(ltp_ModlinkItem->execResult) = MODLINK_NOT_RUN;
        ltp_ModlinkItem->isExec = 0;
        kalyke_modbus_next_index(lsv_PortNum, ltp_ModlinkHead);
        return pdPASS;
    }

    LOGI_MODLINK(TAG, "isExec = %u", ltp_ModlinkItem->isExec)
    /*指令未执行*/
    if (!ltp_ModlinkItem->isExec)
    {
        /*获取串口发送缓冲区*/
        lcp_UartTxBuff = ring_buffer_get_write_mem(&gtp_UartPort[lsv_PortNum].mtv_SendBuff);
        configASSERT(lcp_UartTxBuff != NULL);

        /*切换发送缓冲区，为下次消息准备*/
        ring_buffer_switch_write_mem(&gtp_UartPort[lsv_PortNum].mtv_SendBuff);
        LOGI_MODLINK(TAG, "ltp_ModlinkItem->funcCode = 0x%X", ltp_ModlinkItem->funcCode);
        switch(ltp_ModlinkItem->funcCode)
        {
        case MB_READ_COILS_STATUS:
        case MB_READ_DESCRETE_INPUT_STATUS:
            lcp_UartTxBuff[0] = ltp_ModlinkItem->slaveAddr;
            lcp_UartTxBuff[1] = ltp_ModlinkItem->funcCode;
            lcp_UartTxBuff[2] = (unsigned char)(ltp_ModlinkItem->slaveRegAddr >> 8);
            lcp_UartTxBuff[3] = (unsigned char)ltp_ModlinkItem->slaveRegAddr;
            lcp_UartTxBuff[4] = (unsigned char)(ltp_ModlinkItem->elementCnt >> 8);
            lcp_UartTxBuff[5] = (unsigned char)ltp_ModlinkItem->elementCnt;
            lsv_TxLength = 6;

            if(ltp_ModlinkItem->elementCnt % 8)
            {
                lsv_RxLength = (ltp_ModlinkItem->elementCnt >> 3) + 1 + 5;
            }
            else
            {
                lsv_RxLength = (ltp_ModlinkItem->elementCnt >> 3) + 5;
            }

            /*接收缓冲区越界检查*/
            if(ltp_ModlinkItem->masterBuf + lsv_RxLength > ltp_ModlinkItem->masterBufBoundary)
            {
                *(ltp_ModlinkItem->execResult) = MODLINK_PARA_ERR;
                LOGW_MODLINK(TAG, "Leave %s()...010", __func__);
                //  return ERR_OVER_ELEMENT_RANG;
                return ERR_MODBUS_TBL;
            }

            /*设置接收参数*/
            bsp_set_free_port_receive_para(lsv_PortNum, ltp_ModlinkItem->masterBuf, lsv_RxLength, 1);
            break;

        case MB_READ_HOLDING_REGISTER:
        case MB_READ_MULTIPLE_INPUT_REGISTER: 
                lcp_UartTxBuff[0] = ltp_ModlinkItem->slaveAddr;
                lcp_UartTxBuff[1] = ltp_ModlinkItem->funcCode;
                lcp_UartTxBuff[2] = (unsigned char)(ltp_ModlinkItem->slaveRegAddr >> 8);
                lcp_UartTxBuff[3] = (unsigned char)ltp_ModlinkItem->slaveRegAddr;
                lcp_UartTxBuff[4] = (unsigned char)(ltp_ModlinkItem->elementCnt >> 8);
                lcp_UartTxBuff[5] = (unsigned char)ltp_ModlinkItem->elementCnt;
                lsv_TxLength = 6;
                lsv_RxLength = (ltp_ModlinkItem->elementCnt << 1) + 5;
            
            /*接收缓冲区越界检查*/
            if(ltp_ModlinkItem->masterBuf + lsv_RxLength > ltp_ModlinkItem->masterBufBoundary)
            {
                *(ltp_ModlinkItem->execResult) = MODLINK_PARA_ERR;
                LOGW_MODLINK(TAG, "Leave %s()...011", __func__);
                // return ERR_OVER_ELEMENT_RANG;
                return ERR_MODBUS_TBL;
            }

            /*设置接收参数*/
            bsp_set_free_port_receive_para(lsv_PortNum, ltp_ModlinkItem->masterBuf, lsv_RxLength, 1);
            break;

        case MB_WRITE_SINGLE_COIL:
            LOGV_MODLINK(TAG, "msv_ElementCnt = %u", ltp_ModlinkItem->elementCnt);
            //LOGE_MODLINK(TAG, "msp_ResultData :\r\n");
            //hexdump(ltp_ModlinkItem->msp_ResultData, 32);

            lcp_UartTxBuff[0] = ltp_ModlinkItem->slaveAddr;
            lcp_UartTxBuff[1] = ltp_ModlinkItem->funcCode;
            lcp_UartTxBuff[2] = (unsigned char)(ltp_ModlinkItem->slaveRegAddr >> 8);
            lcp_UartTxBuff[3] = (unsigned char)ltp_ModlinkItem->slaveRegAddr;
            lcp_UartTxBuff[4] = 0x00;
            lcp_UartTxBuff[5] = 0x00;
            if(ltp_ModlinkItem->elementCnt > 0)
            {
                if (ltp_ModlinkItem->masterBuf[0] != 0)
                {
                    lcp_UartTxBuff[4] = 0xFF;
                    lcp_UartTxBuff[5] = 0x00;
                }
            }
            lsv_TxLength = 6;
            lsv_RxLength = 8;
            /*设置接收参数*/
            bsp_set_free_port_receive_para(lsv_PortNum, ltp_ModlinkHead->msp_RecvBuff, lsv_RxLength, 0);
            break;

        case MB_WRITE_REGISTER:
            lcp_UartTxBuff[0] = ltp_ModlinkItem->slaveAddr;
            lcp_UartTxBuff[1] = ltp_ModlinkItem->funcCode;
            lcp_UartTxBuff[2] = (unsigned char)(ltp_ModlinkItem->slaveRegAddr >> 8);
            lcp_UartTxBuff[3] = (unsigned char)ltp_ModlinkItem->slaveRegAddr;
        //    lcp_UartTxBuff[4] = (unsigned char)(ltp_ModlinkItem->elementCnt >> 8);
        //    lcp_UartTxBuff[5] = (unsigned char)ltp_ModlinkItem->elementCnt;
            lcp_UartTxBuff[4] = (unsigned char)(ltp_ModlinkItem->masterBuf[0] >> 8);
            lcp_UartTxBuff[5] = (unsigned char)ltp_ModlinkItem->masterBuf[0];
            lsv_TxLength = 6;
            lsv_RxLength = 8;
            /*设置接收参数*/
            bsp_set_free_port_receive_para(lsv_PortNum, ltp_ModlinkHead->msp_RecvBuff, lsv_RxLength, 0);
            break;

        case MB_WRITE_MULTIPLE_COILS: // 02 0F 07 D0 00 03 01 00 0F 27
            lcp_UartTxBuff[0] = ltp_ModlinkItem->slaveAddr;
            lcp_UartTxBuff[1] = ltp_ModlinkItem->funcCode;
            lcp_UartTxBuff[2] = (unsigned char)(ltp_ModlinkItem->slaveRegAddr >> 8);
            lcp_UartTxBuff[3] = (unsigned char)ltp_ModlinkItem->slaveRegAddr;
            lcp_UartTxBuff[4] = (unsigned char)(ltp_ModlinkItem->elementCnt >> 8);
            lcp_UartTxBuff[5] = (unsigned char)ltp_ModlinkItem->elementCnt;

            if(ltp_ModlinkItem->elementCnt % 8)
            {
                lsv_TxLength = (ltp_ModlinkItem->elementCnt >> 3) + 1;
            }
            else
            {
                lsv_TxLength = (ltp_ModlinkItem->elementCnt >> 3);
            }

            lcp_UartTxBuff[6] = lsv_TxLength;

            /*发送数据缓冲区越界检查*/
            if(ltp_ModlinkItem->masterBuf + lsv_TxLength > ltp_ModlinkItem->masterBufBoundary)
            {
                *(ltp_ModlinkItem->execResult) = MODLINK_PARA_ERR;
                LOGW_MODLINK(TAG, "Leave %s()...012", __func__);
                //  return ERR_OVER_ELEMENT_RANG;
                return ERR_MODBUS_TBL;
            }

            lsv_TxLength += 7;
            j = 0;

            for(i = 7; i < lsv_TxLength; i += 2)
            {
                lcp_UartTxBuff[i] = (unsigned char)ltp_ModlinkItem->masterBuf[j];
                lcp_UartTxBuff[i + 1] = (unsigned char)(ltp_ModlinkItem->masterBuf[j] >> 8);
                j++;
            }

            lsv_RxLength = 8;

            /*设置接收参数*/
            bsp_set_free_port_receive_para(lsv_PortNum, ltp_ModlinkHead->msp_RecvBuff, lsv_RxLength, 0);
            break;

        case MB_WRITE_MULTIPLE_REGISTERS:
            lcp_UartTxBuff[0] = ltp_ModlinkItem->slaveAddr;
            lcp_UartTxBuff[1] = ltp_ModlinkItem->funcCode;
            lcp_UartTxBuff[2] = (unsigned char)(ltp_ModlinkItem->slaveRegAddr >> 8);
            lcp_UartTxBuff[3] = (unsigned char)ltp_ModlinkItem->slaveRegAddr;
            lcp_UartTxBuff[4] = (unsigned char)(ltp_ModlinkItem->elementCnt >> 8);
            lcp_UartTxBuff[5] = (unsigned char)ltp_ModlinkItem->elementCnt;

            lsv_TxLength = ltp_ModlinkItem->elementCnt << 1;

            lcp_UartTxBuff[6] = lsv_TxLength;

            /*发送数据缓冲区越界检查*/
            if(ltp_ModlinkItem->masterBuf + lsv_TxLength > ltp_ModlinkItem->masterBufBoundary)
            {
                *(ltp_ModlinkItem->execResult) = MODLINK_PARA_ERR;
                LOGW_MODLINK(TAG, "Leave %s()...013", __func__);
                //  return ERR_OVER_ELEMENT_RANG;
                return ERR_MODBUS_TBL;
            }

            lsv_TxLength += 7;
            j = 0;
            for(i = 7; i < lsv_TxLength; i += 2)
            {
                lcp_UartTxBuff[i] = (unsigned char)(ltp_ModlinkItem->masterBuf[j] >> 8);
                lcp_UartTxBuff[i + 1] = (unsigned char)ltp_ModlinkItem->masterBuf[j];
                j++;
            }

            lsv_RxLength = 8;

            /*设置接收参数*/
            bsp_set_free_port_receive_para(lsv_PortNum, ltp_ModlinkHead->msp_RecvBuff, lsv_RxLength, 0);
            break;

        default:
            *(ltp_ModlinkItem->execResult) = MODLINK_PARA_ERR;
            LOGW_MODLINK(TAG, "Leave %s()...014", __func__);
            //return pdFAIL;
            return ERR_MODBUS_TBL;
        }

        lsv_TxCrc = calc_crc16(lcp_UartTxBuff, lsv_TxLength);
        lcp_UartTxBuff[lsv_TxLength + 1] = (unsigned char)(lsv_TxCrc >> 8);
        lcp_UartTxBuff[lsv_TxLength] = (unsigned char)lsv_TxCrc;

        /*清除相关标志位*/
        RST_UART_SM_FLAG(lsv_PortNum, UART_SM_TX_FINISH);
        RST_UART_SM_FLAG(lsv_PortNum, UART_SM_RX_FINISH);
        RST_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH);
        RST_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR);
        SET_UART_SD_VALUE(lsv_PortNum, UART_SD_MASTER_ERROR_CODE, 0);

        ltp_ModlinkItem->timeOut = GET_1MS_TICKS_COUNT() + gtp_UartPort[lsv_PortNum].mtv_ModeInfo.MasterPort.msv_TxTimeOut + 1;
        LOGW_MODLINK(TAG, "msv_TxTimeOut = %u, ltp_ModlinkItem->timeOut = %u", gtp_UartPort[lsv_PortNum].mtv_ModeInfo.MasterPort.msv_TxTimeOut, ltp_ModlinkItem->timeOut);
        ltp_ModlinkItem->isExec = 1;

        *(ltp_ModlinkItem->execResult) = MODLINK_RUNNING;

        /*发送数据*/
        if(gtp_UartPort[lsv_PortNum].pSendFunc)
        {
            gtp_UartPort[lsv_PortNum].pSendFunc(lcp_UartTxBuff, lsv_TxLength + 2);
        }
        kalyke_modbus_start_resp_time_out_timer(lsv_PortNum, gtp_UartPort[lsv_PortNum].mtv_ModeInfo.MasterPort.msv_TxTimeOut + 1);
        return -1;
    }
    else // isExec = 1
    {
        uint32_t curTick = GET_1MS_TICKS_COUNT();
        int32_t interVal = curTick - ltp_ModlinkItem->timeOut;
        LOGI_MODLINK(TAG, "ltp_ModlinkItem->timeOut = %u, curTick = %u, interVal = %d", ltp_ModlinkItem->timeOut, curTick, interVal);
        if(GET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH) &&
                GET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR))
        {
            *(ltp_ModlinkItem->execResult) = MODLINK_RUN_ERR;
            ltp_ModlinkItem->isExec = 0;
            kalyke_modbus_next_index(lsv_PortNum, ltp_ModlinkHead);
            LOGW_MODLINK(TAG, "Leave %s()...015", __func__);
            return pdPASS;
        }
        else if(GET_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH))
        {
            *(ltp_ModlinkItem->execResult) = MODLINK_PASS;
            ltp_ModlinkItem->isExec = 0;
            kalyke_modbus_next_index(lsv_PortNum, ltp_ModlinkHead);
            LOGW_MODLINK(TAG, "Leave %s()...016", __func__);
            return pdPASS;
        }
        else if(interVal < 0)
        {
            /*帧传输未超时，等待...*/
            LOGW_MODLINK(TAG, "Leave %s()...017, Not time out, waitting...", __func__);
            return pdPASS;
        }
        else
        {
            // 超时了
            *(ltp_ModlinkItem->execResult) = MODLINK_RUN_TIMEOUT;
            RST_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_FINISH);
            RST_UART_SM_FLAG(lsv_PortNum, UART_SM_MODBUS_ERROR);
            ltp_ModlinkItem->isExec = 0;
            kalyke_modbus_next_index(lsv_PortNum, ltp_ModlinkHead);
            LOGW_MODLINK(TAG, "Leave %s()...018, Time out", __func__);
            return pdPASS;
        }
    }
    LOGD_MODLINK(TAG, "Leave %s()...019", __func__);
    return pdPASS;
}

void kalyke_modbus_com_task(void *p_arg)
{
    LOGD_MODLINK(TAG, "kalyke_modbus_com_task RUN. Free heap size is %d bytes.(COM%u)", xPortGetFreeHeapSize(), *(uint16_t*)p_arg);

    vTaskDelay(3000);
    int8_t lcv_RetValue = pdPASS;
    uint16_t curDelay = 100;
    uint16_t lsv_PortNum = *(uint16_t *)p_arg; //取通信串口号

    while(1)
    {
        if (gTouChuan != 0xFF)
        {
            vTaskDelay(1000);
            continue;
        }
        if (gGUHUAing == 1 || gSuspendFlag == true)
        {
            LOGE(TAG, "Enter %s(),GUHUAing..........vTaskDelay(1000).", __func__);
            vTaskDelay(1000);
            continue;
        }

        lcv_RetValue = kalyke_modbus_com_task_do(lsv_PortNum);
        if (-1 == lcv_RetValue) // 数据已成功发送出去了
        {
            xEventGroupWaitBits(g_kalyke_event_group, KALYKE_EVENT_UART_COM0_HAD_RESP << lsv_PortNum, pdTRUE, pdFALSE, portMAX_DELAY);
            curDelay = g_modbus_cfg.listPtr[g_modbus_cfg.index[lsv_PortNum]].delay;
            if (gRespTimeout[lsv_PortNum] == false)
            {
                kalyke_modbus_stop_resp_time_out_timer(lsv_PortNum);
                *g_modbus_cfg.listPtr[g_modbus_cfg.index[lsv_PortNum]].execResult = MODLINK_PASS;
                g_modbus_cfg.listPtr[g_modbus_cfg.index[lsv_PortNum]].isExec = 0;
                kalyke_modbus_next_index(lsv_PortNum, &g_modbus_cfg);
            }
            LOGD_MODLINK(TAG, "kalyke_modbus_com%u_task RUN. Delay %d ms.\r\n", lsv_PortNum, curDelay);
            vTaskDelay(curDelay);
            continue;
        }
        /*执行错误，比如参数错误，主站站号错误等刷新错误标志元件值*/
        if(lcv_RetValue != pdPASS)
        {
            plc_refresh_error_msg(lcv_RetValue);
        }
        /*lcv_RetValue == pdPASS数据没有发送，比如触发元件为OFF的情况*/
        LOGD_MODLINK(TAG, "kalyke_modbus_com_task RUN. Delay 1 ms.\r\n");
        vTaskDelay(1);
    }
}

void start_modbus_com(void)
{ 
    LOGV(TAG, "Enter %s(), g_modbus_cfg.listNum = %u", __func__, g_modbus_cfg.listNum);
    if (g_modbus_cfg.listNum == 0)
    {
        return;
    }

    /*some parameter init*/
    for(uint16_t i = 0; i < MAX_SUPPORT_UART_PORT; i++)
    {
        gModbusRespTimer[i] = NULL;
        gRespTimeout[i] = false;
        gModbusHandle[i] = NULL;
        g_modbus_cfg.index[i] = 0;
    }
    
    for(uint16_t i = 0; i < g_modbus_cfg.listNum; i++)
    {
        if(gModbusHandle[g_modbus_cfg.listPtr[i].port] == NULL)
        {
            uint16_t *ptaskport = pvPortMalloc(sizeof(uint16_t));
            *ptaskport = g_modbus_cfg.listPtr[i].port;

            char taskName[configMAX_TASK_NAME_LEN] = {0};
            sprintf(taskName, "MODBUS_COM%u_TASK", g_modbus_cfg.listPtr[i].port);
            BaseType_t ret = xTaskCreate((TaskFunction_t)kalyke_modbus_com_task,
                                         (const char *)taskName,
                                         MODBUS_COM_TASK_STACK_SIZE,
                                         (void *)ptaskport,
                                         MODBUS_COM_TASK_PRIO,
                                         &gModbusHandle[g_modbus_cfg.listPtr[i].port]);
            LOGD_MODLINK(TAG, "Create MODBUS_COM%u_TASK!", g_modbus_cfg.listPtr[i].port);
            if (ret != pdPASS)
            {
                LOGE(TAG, "Create MODBUS_COM%u_TASK ERROR!", g_modbus_cfg.listPtr[i].port);
            }

            vTaskDelay(200);
        }
    }

    LOGD(TAG, "Leave %s()", __func__);
}

void stop_modbus_com(void)
{
    LOGV(TAG, "Enter %s()", __func__);
    for(uint16_t i = 0; i < MAX_SUPPORT_UART_PORT; i++)
    {
        if (gModbusHandle[i] != NULL)
        {
            vTaskDelete(gModbusHandle[i]);
            gModbusHandle[i] = NULL;
        }
    }
}

