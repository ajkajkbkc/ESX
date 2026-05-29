/**
  ******************************************************************************
  * @file    bsp_uart.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   串口驱动程序
  ******************************************************************************
  */

#ifndef __BSP_UART_H
#define __BSP_UART_H
#include "plc_variable.h"

/*UART状态定义*/
enum __UART_STATUS_E {
    /*端口空闲*/
    UART_IDLE = 0x01,
    /*接收*/
    UART_RX = 0x02,
    /*发送*/
    UART_TX = 0x04,
};

/*uart 缓冲区定义*/
typedef struct __BSP_UART_STATUS_INFO_ST {
    /*当前端口状态*/
    unsigned char mcv_Status;
    /*接收数据长度*/
    unsigned short msv_RxLength;
    /*待发送数据长度*/
    unsigned short msv_TxTotalLength;
    /*已发送数据长度*/
    unsigned short msv_TxLength;
    /*接收缓存区*/
    unsigned char *mcp_RxBuff;
    /*发送缓存区*/
    unsigned char *mcp_TxBuff;
    /*自由口、Modbus master接收缓冲区地址*/
    unsigned short *msp_FreeRxBuff;
    /*自由口、Modbus master接收最大长度*/
    unsigned short msv_FreeRxMaxCnt;
    /*Modbus master重发次数*/
    unsigned char mcv_RetryCnt;

    unsigned char aFlag; // 0, modbus; 1, modlink
    unsigned char *aBuf;
} bsp_uart_status_info_st;

extern void bsp_get_uart_configure_info(uart_port_info_st *ltp_UartInfo);
extern unsigned char bsp_get_uart_port_status(unsigned short lsv_PortNum);
extern void bsp_set_free_port_receive_para(unsigned short lsv_PortNum, unsigned short *lsp_RxBuff, unsigned short lsv_RxCnt, uint8_t flag);
extern void resume_uart1_task(void);
extern void suspend_uart1_task(void);
extern void bsp_com0_send_data(uint8_t *pData, uint16_t len);
extern void bsp_com1_send_data(uint8_t *pData, uint16_t len);
extern void bsp_com2_send_data(uint8_t *pData, uint16_t len);
extern void bsp_uart1_send_buffer(uint8_t *pData, uint16_t len);
extern void bsp_uart_modbus_send(uint16_t port, unsigned char *lcp_SendBuff, unsigned short lsv_Length);

#endif /*__BSP_UART_H*/
