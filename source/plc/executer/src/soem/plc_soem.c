/**
  ******************************************************************************
  * @file    plc_soem.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2020-09-11
  * @brief   SOEM 相关指令实现
  ******************************************************************************
  */
#include "plc_soem.h"
//#include "plc_variable.h"
#include "plc_commonfunc.h"
#include "plc_element.h"
#include "plc_parseaddr.h"
#include "plc_errormsg.h"
#include "plc_instruction.h"

#include "bsp_uart.h"
#include "bsp_dct.h"
#include "plc_sysblock.h"
#include "kalyke_internet_task.h"

#include "verify_func.h"
#include "fsl_debug_console.h"
#include "kalyke_tool.h"

#include "ethercat.h"
#include "ether_cat_task.h"


//#define DEBUG_SOEM

#ifdef DEBUG_SOEM
#define LOGE_SOEM    LOGE
#define LOGW_SOEM    LOGW
#define LOGI_SOEM    LOGI
#define LOGD_SOEM    LOGD
#define LOGV_SOEM    LOGV
#else
#define LOGE_SOEM(...)
#define LOGW_SOEM(...)
#define LOGI_SOEM(...)
#define LOGD_SOEM(...)
#define LOGV_SOEM(...)
#endif

#if (ETHERCAT_SOEM == 1)
static const char *TAG = "plc_soem";

/** CI_ECATSDORD : 0xF2D7
  D7 F2
  00 FF 01 00 -- 0:WAN,  1:LAN
  00 FF 01 00 -- 设备地址
  00 FF 8F 60 -- SDO索引
  00 FF 01 00 -- SDO子索引
  00 FF 04 00 -- 读取长度
  00 32 64 00 00 00 -- 数据存放地址
  00 05 0A 00 -- 指令执行状态
  00 11 C8 00 -- 指令执行错误码
*/
unsigned char run_ci_ecatsdord_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    uint16_t wanOrLan; //0:WAN,  1:LAN
    uint16_t slaveID;  //设备地址（从站ID）
    uint16_t sdoIndex; //SDO索引
    uint16_t sdoSubIndex;
    uint16_t bufSize;  //读取长度
    uint32_t dataVal;  //数据存放地址
    uint8_t ins_state; //指令执行状态，M寄存器
    int16_t errCode;   //指令执行错误码，D寄存器

    if(GET_POWER_FLOW(ltp_RunEnv))
    {
        LOGI_SOEM(TAG, "%s: POWER FLOW ON!", __func__);
        get_word(ltp_RunEnv->mcp_PC + 2, &wanOrLan, 0, 1);
        get_word(ltp_RunEnv->mcp_PC + 6, &slaveID, 0, 1);
        get_word(ltp_RunEnv->mcp_PC + 10, &sdoIndex, 0, 1);
        get_word(ltp_RunEnv->mcp_PC + 14, &sdoSubIndex, 0, 1);
        get_word(ltp_RunEnv->mcp_PC + 18, &bufSize, 0, 1);
        //get_dword(ltp_RunEnv->mcp_PC + 22, &dataVal, 0, 1);
        get_char(ltp_RunEnv->mcp_PC + 28, &ins_state, 0, 1);
        LOGV_SOEM(TAG, "wanOrLan=%u, slaveID=%u, sdoIndex=0x%04X, sdoSubIndex=%u, bufSize=%u, ins_state=%u", wanOrLan, slaveID, sdoIndex, sdoSubIndex, bufSize, ins_state);
        if (ins_state == 0)
        {
            ins_state = 1;
            save_char_default(ltp_RunEnv->mcp_PC + 28, &ins_state);

            int __s = bufSize;
            soem_msg_st soemMsg;
            soemMsg.slaveID = slaveID;
            soemMsg.sdoIndex = sdoIndex;
            soemMsg.sdoSubIndex = sdoSubIndex;
            soemMsg.bufSize = bufSize;
            soem_plc_sdo_read(&soemMsg);

            LOGD(TAG, "Slave: %d - Read at 0x%04x:%d => wkc: %d; data: 0x%.*x (%d)", slaveID, sdoIndex, sdoSubIndex, soemMsg.ret, __s, soemMsg.dataVal, soemMsg.dataVal);
            
            if (soemMsg.ret <= 0)
            {
                if (soemMsg.ret == 0)
                {
                    errCode = -1;
                }
                else
                {
                    errCode = soemMsg.ret;
                }
                save_word_default(ltp_RunEnv->mcp_PC + 32, (unsigned short *)&errCode);
            }
            else
            {
                save_dword(ltp_RunEnv->mcp_PC + 22, (unsigned long *)&soemMsg.dataVal, 0, 1);
                errCode = 0;
                save_word_default(ltp_RunEnv->mcp_PC + 32, (unsigned short *)&errCode);
            }
        }
    }
    else
    {
        LOGI_SOEM(TAG, "%s: POWER FLOW OFF", __func__);
        
    }
    LOGD_SOEM(TAG, "Leave %s()", __func__);
    return pdPASS;
}

/** CI_ECATSDOWR : 0xF2D8
    D8 F2
    00 FF 01 00 -- 0:WAN,  1:LAN
    00 FF 01 00 -- 设备地址
    00 FF 8F 60 -- SDO索引
    00 FF 01 00 -- SDO子索引
    00 FF 04 00 -- 读取长度
    01 32 2C 01 00 00 -- 数据存放地址
    00 05 0B 00 -- 指令执行状态
    02 32 90 01 -- 指令执行错误码
  */
unsigned char run_ci_ecatsdowr_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    uint16_t wanOrLan; //0:WAN,  1:LAN
    uint16_t slaveID;  //设备地址（从站ID）
    uint16_t sdoIndex; //SDO索引
    uint16_t sdoSubIndex;
    uint16_t bufSize;  //读取长度
    uint32_t dataVal;  //数据存放地址
    uint8_t ins_state; //指令执行状态，M寄存器
    int16_t errCode;   //指令执行错误码，D寄存器

    if(GET_POWER_FLOW(ltp_RunEnv))
    {
        LOGI_SOEM(TAG, "%s: POWER FLOW ON!", __func__);
        get_word(ltp_RunEnv->mcp_PC + 2, &wanOrLan, 0, 1);
        get_word(ltp_RunEnv->mcp_PC + 6, &slaveID, 0, 1);
        get_word(ltp_RunEnv->mcp_PC + 10, &sdoIndex, 0, 1);
        get_word(ltp_RunEnv->mcp_PC + 14, &sdoSubIndex, 0, 1);
        get_word(ltp_RunEnv->mcp_PC + 18, &bufSize, 0, 1);
        get_dword(ltp_RunEnv->mcp_PC + 22, (unsigned long *)&dataVal, 0, 1);
        get_char(ltp_RunEnv->mcp_PC + 28, &ins_state, 0, 1);
        LOGV_SOEM(TAG, "wanOrLan=%u, slaveID=%u, sdoIndex=0x%04X, sdoSubIndex=%u, bufSize=%u, ins_state=%u", wanOrLan, slaveID, sdoIndex, sdoSubIndex, bufSize, ins_state);
        if (ins_state == 0)
        {
            ins_state = 1;
            save_char_default(ltp_RunEnv->mcp_PC + 28, &ins_state);
            
            int __s = bufSize;
            soem_msg_st soemMsg;
            soemMsg.slaveID = slaveID;
            soemMsg.sdoIndex = sdoIndex;
            soemMsg.sdoSubIndex = sdoSubIndex;
            soemMsg.bufSize = bufSize;
            soemMsg.dataVal = dataVal;
            soem_plc_sdo_write(&soemMsg);
            LOGD(TAG, "Slave: %d - Write at 0x%04x:%d => wkc: %d; data: 0x%.*x (%d)", slaveID, sdoIndex, sdoSubIndex, soemMsg.ret, __s, soemMsg.dataVal, soemMsg.dataVal);

            if (soemMsg.ret <= 0)
            {
                if (soemMsg.ret == 0)
                {
                    errCode = -1;
                }
                else
                {
                    errCode = soemMsg.ret;
                }
                save_word_default(ltp_RunEnv->mcp_PC + 32, (unsigned short *)&errCode);
            }
            else
            {
                errCode = 0;
                save_word_default(ltp_RunEnv->mcp_PC + 32, (unsigned short *)&errCode);
            }
        }
    }
    else
    {
        LOGI_SOEM(TAG, "%s: POWER FLOW OFF", __func__);
        
    }
    LOGD_SOEM(TAG, "Leave %s()", __func__);
    return pdPASS;
}

#else
unsigned char run_ci_ecatsdord_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    return pdPASS;
}

unsigned char run_ci_ecatsdowr_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    return pdPASS;
}
#endif

