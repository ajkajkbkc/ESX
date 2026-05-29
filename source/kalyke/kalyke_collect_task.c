/*
 * Copyright (c) 2022-2023, Fexlink Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-23     JPeng        first implementation
 */
 
#include "stdio.h"
#include "main.h"

#include "bsp_uart.h"
#include "verify_func.h"
#include "kalyke_collect_task.h"
#include "kalyke_oled_key_task.h"
#include "plc_netcfg.h"
#include "plc_executer.h" 
#include "kalyke_event.h"
/* Private define ------------------------------------------------------------*/ 
uint8_t        gRtu_Num;     //已接的终端数量
__IO uint16_t  gRtu_Idx;     //终端查询计数Index
rtu_info_packet_t  gRtu_InfoPacket;   //要存入flash
volatile uint8_t collect_num, cur_addr, collect_complete;
TaskHandle_t gtv_CollectTaskHandler;
volatile uint8_t gGetAtLeastOnce;
/* Private user code ---------------------------------------------------------*/

/**
  * @brief  配址命令
  * @param  None
  * @param  None
  * @retval None
  */
void SETRtuID(void)
{      
    unsigned short lsv_TxLength, lsv_RxLength, lsv_TxCrc;
    uint8_t ucSendBuf[6];
    
    modbus_item_st *ltp_ModlinkItem;
    
    ucSendBuf[0] = 0x00;
    ucSendBuf[1] = 0x6A;
    ucSendBuf[2] = 0xA0;
    ucSendBuf[3] = 0x01;
    lsv_TxLength = 4;
    lsv_RxLength = 6;
      
    bsp_set_free_port_receive_para(1, ltp_ModlinkItem->masterBuf, lsv_RxLength, 1);

    lsv_TxCrc = calc_crc16(ucSendBuf, lsv_TxLength);
    ucSendBuf[lsv_TxLength + 1] = (unsigned char)(lsv_TxCrc >> 8);
    ucSendBuf[lsv_TxLength] = (unsigned char)lsv_TxCrc;
    
    RST_UART_SM_FLAG(1, UART_SM_TX_FINISH); 
    RST_UART_SM_FLAG(1, UART_SM_RX_FINISH);
    RST_UART_SM_FLAG(1, UART_SM_MODBUS_FINISH);
    RST_UART_SM_FLAG(1, UART_SM_MODBUS_ERROR);
    SET_UART_SD_VALUE(1, UART_SD_MASTER_ERROR_CODE, 0);
    
    bsp_uart1_send_buffer(ucSendBuf, lsv_TxLength + 2);
    vTaskDelay(500);
    plc_status_run_to_stop();

}

/**
  * @brief  配址命令
  * @param  None
  * @param  None
  * @retval None
  */
void ADDRtuID(void)
{      
    unsigned short lsv_TxLength, lsv_RxLength, lsv_TxCrc;
    uint8_t ucSendBuf[6];
    
    modbus_item_st *ltp_ModlinkItem;
    
    ucSendBuf[0] = 0x00;
    ucSendBuf[1] = 0x6A;
    ucSendBuf[2] = 0xA0;
    ucSendBuf[3] = lsv_collect++;
    lsv_TxLength = 4;
    lsv_RxLength = 6;
      
    bsp_set_free_port_receive_para(1, ltp_ModlinkItem->masterBuf, lsv_RxLength, 1);

    lsv_TxCrc = calc_crc16(ucSendBuf, lsv_TxLength);
    ucSendBuf[lsv_TxLength + 1] = (unsigned char)(lsv_TxCrc >> 8);
    ucSendBuf[lsv_TxLength] = (unsigned char)lsv_TxCrc;
    
    RST_UART_SM_FLAG(1, UART_SM_TX_FINISH); 
    RST_UART_SM_FLAG(1, UART_SM_RX_FINISH);
    RST_UART_SM_FLAG(1, UART_SM_MODBUS_FINISH);
    RST_UART_SM_FLAG(1, UART_SM_MODBUS_ERROR);
    SET_UART_SD_VALUE(1, UART_SD_MASTER_ERROR_CODE, 0);
    
    bsp_uart1_send_buffer(ucSendBuf, lsv_TxLength + 2);
    vTaskDelay(500);
    plc_status_run_to_stop();

}

/**
  * @brief  配址完成
  * @param  None
  * @param  None
  * @retval None
  */
void SETRtuIDED(void)
{      
    unsigned short lsv_TxLength, lsv_RxLength, lsv_TxCrc;
    uint8_t ucSendBuf[6];
    
    modbus_item_st *ltp_ModlinkItem;
    
    ucSendBuf[0] = 0x00;
    ucSendBuf[1] = 0x6A;
    ucSendBuf[2] = 0xA1;
    ucSendBuf[3] = 0xFF;
    lsv_TxLength = 4;
    lsv_RxLength = 6;
      
    bsp_set_free_port_receive_para(1, ltp_ModlinkItem->masterBuf, lsv_RxLength, 1);

    lsv_TxCrc = calc_crc16(ucSendBuf, lsv_TxLength);
    ucSendBuf[lsv_TxLength + 1] = (unsigned char)(lsv_TxCrc >> 8);
    ucSendBuf[lsv_TxLength] = (unsigned char)lsv_TxCrc;
    
    RST_UART_SM_FLAG(1, UART_SM_TX_FINISH); 
    RST_UART_SM_FLAG(1, UART_SM_RX_FINISH);
    RST_UART_SM_FLAG(1, UART_SM_MODBUS_FINISH);
    RST_UART_SM_FLAG(1, UART_SM_MODBUS_ERROR);
    SET_UART_SD_VALUE(1, UART_SD_MASTER_ERROR_CODE, 0);
    
    bsp_uart1_send_buffer(ucSendBuf, lsv_TxLength + 2);
    vTaskDelay(500);
    NVIC_SystemReset();

}

/**
  * @brief  发送终端寻址命令
  * @param  None
  * @retval None
  */
void Handle_RtuType_ToSend(uint8_t RtuId)
{
    uint8_t lcp_UartTxBuff[8];
    unsigned short lsv_TxLength, lsv_RxLength, lsv_TxCrc;
    
    modbus_item_st *ltp_ModlinkItem;
            
    lcp_UartTxBuff[0] = RtuId;
    lcp_UartTxBuff[1] = 0x03;
    lcp_UartTxBuff[2] = 0x00;
    lcp_UartTxBuff[3] = 0x01;
    lcp_UartTxBuff[4] = 0x00;
    lcp_UartTxBuff[5] = 0x0B;
    lsv_TxLength = 6;
    lsv_RxLength = 27;
    
    bsp_set_free_port_receive_para(1, ltp_ModlinkItem->masterBuf, lsv_RxLength, 1);
    
    lsv_TxCrc = calc_crc16(lcp_UartTxBuff, lsv_TxLength);
    lcp_UartTxBuff[lsv_TxLength + 1] = (unsigned char)(lsv_TxCrc >> 8);
    lcp_UartTxBuff[lsv_TxLength] = (unsigned char)lsv_TxCrc;
    
    RST_UART_SM_FLAG(1, UART_SM_TX_FINISH);     //发送完成
    RST_UART_SM_FLAG(1, UART_SM_RX_FINISH);     //接收完成
    RST_UART_SM_FLAG(1, UART_SM_MODBUS_FINISH); //modbus通讯完成
    RST_UART_SM_FLAG(1, UART_SM_MODBUS_ERROR);  //
    SET_UART_SD_VALUE(1, UART_SD_MASTER_ERROR_CODE, 0);
    
    bsp_uart1_send_buffer(lcp_UartTxBuff, lsv_TxLength + 2);
}


/**
  * @brief  扫描终端所有设备
  * @param  None
  * @retval None
  */
void GetAllRtuID(void)
{            
//    collect_num = 1;
    
//    for(gRtu_Idx = 0; gRtu_Idx <= MB_RTU_ADDR_MAX; gRtu_Idx++)
//    {        
//        if(gRtu_Num >= MAX_RTU_NUM)
//        {
//            break;
//        }
//        
//        Handle_RtuType_ToSend(gRtu_Idx);
//        //vTaskDelay(10);
//        vTaskDelay(200);     
//    }
    
//    collect_num = 0;     
    
    if(gRtu_Num < collect_maxnum)
    {
        collect_num = 1;    
        gRtu_Idx++;
        Handle_RtuType_ToSend(gRtu_Idx);        
    }
    else if(gRtu_Num >= collect_maxnum)
    {
        collect_num = 0; 
    }
}

/**
  * @brief  处理扫描终端设备时返回的数据（设备ID）
  * @param  pBuf 数据起始地址
  * @param  len 数据长度
  * @retval None
  */
  
void Handle_FindFlkRtu_RecvData(uint8_t *pBuf)
{       
//    uint8_t CURIdInfo[22]; //终端ID号
    
//    memcpy(&CURIdInfo, &pBuf[3], 22);

//    if(!collect_complete)
//    {
//        memcpy(&gRtu_InfoPacket.Rtu_Info[pBuf[0]].RtuIdInfo, &pBuf[3], 22);
//        if(cur_addr == collect_maxnum)
//        {
//            collect_complete =1;
//        }
//    }
//    else
//    {
//        if(pBuf[0] <= collect_maxnum)
//        {
//            if(0 ==memcmp(&gRtu_InfoPacket.Rtu_Info[pBuf[0]].RtuIdInfo, &pBuf[3], 22))
//            {
//                memcpy(&gRtu_InfoPacket.Rtu_Info[pBuf[0]].RtuIdInfo, &pBuf[3], 22);
//                LOGE("collect", "FEXLINK node ID is %s", gRtu_InfoPacket.Rtu_Info[pBuf[0]].RtuIdInfo);
//            }

if(gGetAtLeastOnce != 1)
{
        if(cur_addr >= pBuf[0])
        {
            memcpy(&gRtu_InfoPacket.Rtu_Info[pBuf[0]].RtuIdInfo, &pBuf[3], 22);
            
//            if(0 !=memcmp(&gRtu_InfoPacket.Rtu_Info[pBuf[0]-1].RtuIdInfo, &pBuf[3], 22))
//            {
//                gRtu_Num++;
//                lsv_collect = gRtu_Num;
//            }

            for(uint8_t i=1; i <= collect_maxnum; i++)
            { 
                LOGE("collect", "FEXLINK node ID is %s", gRtu_InfoPacket.Rtu_Info[i].RtuIdInfo);
                if(0 ==memcmp(&gRtu_InfoPacket.Rtu_Info[i].RtuIdInfo, &pBuf[3], 22))
                {
                    if(0 !=memcmp(&gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuIdInfo, &pBuf[3], 22))
                    { 
                        gRtu_Num++;
                        lsv_collect = gRtu_Num;
                    //memcpy(&gRtu_InfoPacket.Rtu_Info[i].RtuIdInfo, "0000000000000000000000", 22);
                    }  
                }                    
            }
        }
}
        
//        if(0 !=memcmp(&gRtu_InfoPacket.Rtu_Info[pBuf[0]-1].RtuIdInfo, &pBuf[3], 22))
//        {
//            if(lsv_collect != collect_maxnum)
//            {
//                for(uint8_t i=1; i <= collect_maxnum; i++)
//                { 
//                    LOGE("collect", "FEXLINK node ID is %s", gRtu_InfoPacket.Rtu_Info[i].RtuIdInfo);                    
//                    if(0 ==memcmp(&gRtu_InfoPacket.Rtu_Info[i].RtuIdInfo, &pBuf[3], 22))
//                    {
//                        gRtu_Num++;
//                        lsv_collect = gRtu_Num;
//                        //memcpy(&gRtu_InfoPacket.Rtu_Info[i].RtuIdInfo, "0000000000000000000000", 22);
//                    }
//                }
//            }
//        } 

//        for(uint8_t i=1; i < collect_maxnum; i++)
//        {
//            LOGE("collect", "FEXLINK node ID is %s", gRtu_InfoPacket.Rtu_Info[i].RtuIdInfo);
//            if(0 ==memcmp(&gRtu_InfoPacket.Rtu_Info[i].RtuIdInfo, &pBuf[3], 22))
//            {
//                gRtu_Num++;
//                lsv_collect = gRtu_Num;
//            }        
//        }
        
//        if(pBuf[0]==1)
//        {
//            if(0 ==memcmp("KSDE1A2220501001", &pBuf[3], 22))
//            {
//                NVIC_SystemReset();
//            }
//        }
        
//    memcpy(&gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuIdInfo, &pBuf[3], 22);
//    
//    if(0 != memcmp(&gRtu_InfoPacket.Rtu_Info[gRtu_Num-1].RtuIdInfo, &gRtu_InfoPacket.Rtu_Info[gRtu_Num].RtuIdInfo, 22))
//    {
//        gRtu_Num++;
//        lsv_collect = gRtu_Num;
//    }
}

void collect_task(void)
{
    while(1)
    {     
//        vTaskDelay(5000);
//        GetAllRtuID();
//        //vTaskDelete(NULL); 
        vTaskDelay(500);
        
        

    }
}




