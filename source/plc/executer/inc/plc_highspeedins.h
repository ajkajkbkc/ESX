/**
  ******************************************************************************
  * @file    plc_highspeedins.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   控制相关指令函数
  ******************************************************************************
  */

#ifndef __PLC_HIGH_SPEED_H
#define __PLC_HIGH_SPEED_H

#define HCNT_MAX_NUMBER     30
#define HSP_TIME_CHANGE_FREQUENCY 20

typedef struct _hs_hcnt_state
{
    uint16_t counterNum;// 高速计数器编号
    int32_t compareNum; // HCNT要比较的数
    uint8_t *pc;
    uint8_t next;
} hs_hcnt_state_t;

typedef struct _hs_hcnt
{
    hs_hcnt_state_t hcnts[HCNT_MAX_NUMBER];
    uint8_t cntNumbers; // How many counter is running
    uint8_t head;
    uint8_t tail;
} hs_hcnt_t;


/* DHSCS, DHSCR, DHSZ, DHST, DHSP, DHSCI同时驱动的最大个数 */
#define MAX_INSTRUCTION_NUM     6

typedef struct _hs_dhscs_state
{
    volatile bool started; // true = had started
    uint16_t counterNum;   // 高速计数器编号
    uint8_t  sNum;         // 指令序号
    uint8_t  elemType;     // ADDR_Y, ADDR_M, ADDR_S
    uint16_t address;      // 软元件地址
    int32_t  compareNum;   // 要比较的数，数据范围: -2147483648 ~ 2147483647
    uint8_t next;
} hs_dhscs_state_t;
typedef struct _hs_dhscs
{
    hs_dhscs_state_t dhscs[MAX_INSTRUCTION_NUM];
    uint8_t dhscsNumbers; // How many dhscs is running
    uint8_t head;
    uint8_t tail;
} hs_dhscs_t;

typedef struct _hs_dhscr
{
    volatile bool started; // true = had started
    uint16_t counterNum;   // 高速计数器编号
    uint8_t  sNum;         // 指令序号
    uint8_t  elemType;     // ADDR_Y, ADDR_M, ADDR_S, ADDR_C, 对C元件只能是本身
    uint16_t address;      // 软元件地址
    int32_t  compareNum;   // 要比较的数，数据范围: -2147483648 ~ 2147483647
} hs_dhscr_t;

typedef struct _hs_dhsz
{
    volatile bool started; // true = had started
    uint16_t counterNum;   // 高速计数器编号
    uint8_t  sNum;         // 指令序号
    uint8_t  elemType;     // ADDR_Y, ADDR_M, ADDR_S
    uint16_t address;      // 软元件地址
    int32_t  compareNum1;  // 要比较的数1，数据范围: -2147483648 ~ 2147483647
    int32_t  compareNum2;  // 要比较的数2，数据范围: -2147483648 ~ 2147483647
} hs_dhsz_t;

typedef struct _hs_dhst
{
    volatile bool started; // true = had started
    uint16_t counterNum;   // 高速计数器编号
    uint8_t  sNum;         // 指令序号 1 ~ 128
    uint8_t  curRecord;    // 当前正处理的记录号
    uint8_t  recordNum;    // 要比较的记录数量
    uint8_t  elemType;     // ADDR_D for now
    uint32_t address;      // 软元件地址
} hs_dhst_t;

typedef struct _hs_dhsp
{
    volatile bool started; // true = had started
    uint16_t counterNum;   // 高速计数器编号
    uint8_t  sNum;         // 指令序号 1 ~ 128
    uint8_t  curRecord;    // 当前正处理的记录号
    uint8_t  recordNum;    // 要比较的记录数量
    uint8_t  elemType;     // ADDR_D for now
    uint32_t address;      // 软元件地址
} hs_dhsp_t;

typedef struct _hs_dhsci
{
    uint16_t counterNum;   // 高速计数器编号
    uint8_t  sNum;         // 指令序号 1 ~ 128
    uint8_t  intNum;       // 中断号 20 ~ 25
    int32_t  compareNum;   // 要比较的数，数据范围: -2147483648 ~ 2147483647
} hs_dhsci_t;

typedef enum _DHS_type
{
    DHSx_TYPE_DHSCS,
    DHSx_TYPE_DHSCR,
    DHSx_TYPE_DHSZ,
    DHSx_TYPE_DHST,
    DHSx_TYPE_DHSP,
    DHSx_TYPE_DHSCI
}DHS_type;

typedef struct _hs_state
{
    DHS_type type;
    uint8_t next;
    union
    {
        hs_dhscs_state_t dhscs;
        hs_dhscr_t dhscr;
        hs_dhsz_t dhsz;
        hs_dhst_t dhst;
        hs_dhsp_t dhsp;
        hs_dhsci_t dhsci;
    }dhsx;
} hs_state_t;

typedef struct _hs_
{
    hs_state_t hstate[MAX_INSTRUCTION_NUM + 1];
    uint8_t dhsNumbers; // How many DHSx is running ?
    uint8_t head;
    uint8_t tail;
} hs_t;



unsigned char run_ci_plsy_ins(plc_run_power_flow_st *ltp_RunEnv);

unsigned char run_ci_hcnt_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_spd_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dhscs_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dhscr_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dhsci_ins(plc_run_power_flow_st *ltp_RunEnv);

unsigned char run_ci_dhsz_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dhst_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dhsp_ins(plc_run_power_flow_st *ltp_RunEnv);

unsigned char run_ci_dhspi_ins(plc_run_power_flow_st *ltp_RunEnv);


unsigned char run_ci_dszr_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_abs_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_drva_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_zrn_ins(plc_run_power_flow_st *ltp_RunEnv);


unsigned char run_ci_drvc_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_drvi_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_plsv_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_pls_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dvit_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_stopdv_ins(plc_run_power_flow_st *ltp_RunEnv);


unsigned char run_ci_plsr_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_plsb_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_pwm_ins(plc_run_power_flow_st *ltp_RunEnv);

unsigned char run_ci_camtable_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_cambox_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_gearbox_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_movelink_ins(plc_run_power_flow_st *ltp_RunEnv);


unsigned char run_ci_lin_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_cw_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_ccw_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_drv_ins(plc_run_power_flow_st *ltp_RunEnv);


extern hs_hcnt_t gHCNT;
extern hs_hcnt_t gHCNT2;
extern hs_dhscs_t gDHSCS;
extern hs_dhscr_t gDHSCR[MAX_INSTRUCTION_NUM];
extern hs_dhsz_t gDHSZ[MAX_INSTRUCTION_NUM];
extern hs_dhst_t gDHST[MAX_INSTRUCTION_NUM];
extern hs_dhsp_t gDHSP[MAX_INSTRUCTION_NUM];
extern hs_t gHS_DHSx;

extern void init_high_speed(void);
extern void uninit_high_speed(void);
#endif /*__PLC_HIGH_SPEED_H*/

