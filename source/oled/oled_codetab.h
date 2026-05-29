/*
 * Copyright (c) 2022-2023, Fexlink Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-06-23     Arrbow       first implementation
 */

#ifndef __OLED_I2C_CODETAB_H
#define __OLED_I2C_CODETAB_H

/* Includes ------------------------------------------------------------------*/



/* Private defines -----------------------------------------------------------*/

/**
  @brief 中文字符
  */
typedef enum __OLED_CHAR_CN_E
{
    OLED_CHAR_tong        =   0,  /* 通 */
    OLED_CHAR_yong        =   1,  /* 用 */
    OLED_CHAR_she         =   2,  /* 设 */
    OLED_CHAR_zhi         =   3,  /* 置 */
    OLED_CHAR_zuojiantou  =   4,  /* 实心左三角 */
    OLED_CHAR_youjiantou  =   5,  /* 实心右三角 */
    OLED_CHAR_zuojiantou0 =   6,  /* 空心左三角 */
    OLED_CHAR_youjiantou0 =   7,  /* 空心右三角 */
    OLED_CHAR_zhu         =   8,  /* 主 */
    OLED_CHAR_cai         =   9,  /* 菜 */
    OLED_CHAR_dan         =   10,  /* 单 */
    OLED_CHAR_xu          =   11,  /* 序 */
    OLED_CHAR_lie         =   12,  /* 列 */
    OLED_CHAR_hao         =   13,  /* 号 */
    OLED_CHAR_gu          =   14,  /* 固 */
    OLED_CHAR_jian1       =   15,  /* 件 */
    OLED_CHAR_ban         =   16,  /* 版 */
    OLED_CHAR_ben         =   17,  /* 本 */
    OLED_CHAR_chu         =   18,  /* 出 */
    OLED_CHAR_neng        =   19,  /* 能 */
    OLED_CHAR_hui         =   20,  /* 恢 */
    OLED_CHAR_fu          =   21,  /* 复 */
    OLED_CHAR_chang1      =   22,  /* 厂 */
    OLED_CHAR_shi2        =   23,  /* 是 */
    OLED_CHAR_fou         =   24, /* 否 */
    OLED_CHAR_zhi4        =   25, /* 智 */
    OLED_CHAR_chong       =   26, /* 重 */
    OLED_CHAR_qi2         =   27, /* 启 */
    OLED_CHAR_bei         =   28, /* 备 */
    OLED_CHAR_wang        =   29, /* 网 */
    OLED_CHAR_guan        =   30, /* 关 */
    OLED_CHAR_cao         =   31, /* 操 */
    OLED_CHAR_zuo         =   32, /* 作 */
    OLED_CHAR_zhong       =   33, /* 中 */
    OLED_CHAR_xin         =   34, /* 信 */
    OLED_CHAR_xi          =   35, /* 息 */
    OLED_CHAR_zh0ng       =   36, /* 终 */
    OLED_CHAR_duan        =   37, /* 端 */
    OLED_CHAR_yi          =   38, /* 已 */
    OLED_CHAR_lian        =   39, /* 连 */
    OLED_CHAR_jie         =   40, /* 接 */
    OLED_CHAR_zheng       =   41, /* 正 */
    OLED_CHAR_chang       =   42, /* 常 */
    OLED_CHAR_yl          =   43, /* 一 */
    OLED_CHAR_baN         =   44, /* 般 */
    OLED_CHAR_ji          =   45, /* 极 */
    OLED_CHAR_cha         =   46, /* 差 */
    OLED_CHAR_zhan        =   47, /* 站 */
    OLED_CHAR_pei         =   48, /* 配 */
    OLED_CHAR_kai         =   49, /* 开 */
    OLED_CHAR_shi         =   50, /* 始 */
    OLED_CHAR_fan         =   51, /* 取 */
    OLED_CHAR_hul         =   52, /* 消 */    
    OLED_CHAR_tian        =   53, /* 添 */
    OLED_CHAR_jia         =   54, /* 加 */
    OLED_CHAR_qing        =   55, /* 取 */
    OLED_CHAR_zai         =   56, /* 消 */    
    OLED_CHAR_que         =   57, /* 添 */
    OLED_CHAR_ren         =   58, /* 加 */

} oled_char_cn_e;


/* Exported types ------------------------------------------------------------*/



/* Exported constants --------------------------------------------------------*/
extern unsigned char F16x16[];
extern const unsigned char F6x8[][6];
extern const unsigned char F8X16[];
extern unsigned char BMP_8x8_Sigma[];
extern unsigned char BMP_8x16_Epsilon[];
extern unsigned char BMP_8x16_Delta[];
extern unsigned char BMP_8x16_Angle[];
extern unsigned char BMP_8x16_Degree[];

/* Private functions ---------------------------------------------------------*/



#endif /* __OLED_I2C_CODETAB_H */
