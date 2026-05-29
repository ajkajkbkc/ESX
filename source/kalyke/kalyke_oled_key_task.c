/*
 * Copyright (c) 2022-2023, Fexlink Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-06-23     JPeng        first implementation
 * 2022-07-11     Arrbow       code transplantation
 */

/* Private includes ----------------------------------------------------------*/
#include "stdio.h"
#include "main.h"
#include "fsl_debug_console.h"
#include "fsl_gpio.h"

#include "mb.h"
#include "oled_i2c.h"
#include "oled_codetab.h"
#include "plc_commonfunc.h"
#include "kalyke_oled_key_task.h"
#include "kalyke_version.h"
#include "plc_netcfg.h"
#include "bsp_dct.h"
#include "kalyke_collect_task.h"
/* Private define ------------------------------------------------------------*/
static const char *TAG = "OLED";
volatile uint8_t lsv_collect, collect_maxnum;
volatile uint8_t GetAllID;
/* Private variables ---------------------------------------------------------*/
/* Definitions for keyTask */
static flex_button_t iotb_user_button[USER_BUTTON_MAX];
volatile uint8_t key_scan_cycle = KEY_SCAN_CYCLE;
bool iotb_button_scan_enable = true;

key_oled_run_info_t gOledRunInfo;

key_oled_gui_t g_oled_display_table[] =
{
    //开始界面
    {GUI_START, GUI_MAIN_MENU_COMMON_CONFIG, GUI_MAIN_MENU_COMMON_CONFIG, module_oled_display_start_gui},
    //主菜单
    {GUI_MAIN_MENU_COMMON_CONFIG, GUI_TITLE_DEVICE_INFO, GUI_COMMON_CONFIG_SERIAL_NUMBER, oled_display_main_menu_gui},
    {GUI_TITLE_DEVICE_INFO, GUI_MAIN_MENU_COMMON_CONFIG, GUI_DEVIVE_LOAD, oled_display_main_menu_gui},
//    {GUI_MAIN_MENU_MODBUS_SET, GUI_MAIN_MENU_COMMON_CONFIG, GUI_DEVIVE_MODBUS_START, oled_display_main_menu_gui},

    //通用设置
    {GUI_COMMON_CONFIG_SERIAL_NUMBER, GUI_COMMON_CONFIG_FMW_VERSION, GUI_CONFIG_SERIAL_NUMBER, oled_display_common_config_gui},
    {GUI_COMMON_CONFIG_FMW_VERSION, GUI_COMMON_CONFIG_FACTORY_RESET, GUI_CONFIG_FMW_VERSION, oled_display_common_config_gui},
    {GUI_COMMON_CONFIG_FACTORY_RESET, GUI_COMMON_CONFIG_RESET, GUI_CONFIG_FACTORY_RESET_POINT_NO, oled_display_common_config_gui},
    {GUI_COMMON_CONFIG_RESET, GUI_COMMON_CONFIG_SERIAL_NUMBER, GUI_START, oled_display_common_config_gui},
    //通用设置内容
    {GUI_CONFIG_SERIAL_NUMBER, GUI_CONFIG_SERIAL_NUMBER, GUI_MAIN_MENU_COMMON_CONFIG, oled_display_config_serial_number_gui},
    {GUI_CONFIG_FMW_VERSION, GUI_CONFIG_FMW_VERSION, GUI_MAIN_MENU_COMMON_CONFIG, oled_display_config_fmw_version_gui},
    {GUI_CONFIG_FACTORY_RESET_POINT_NO, GUI_CONFIG_FACTORY_RESET_POINT_YES, GUI_MAIN_MENU_COMMON_CONFIG, oled_display_config_factory_reset_gui},
    {GUI_CONFIG_FACTORY_RESET_POINT_YES, GUI_CONFIG_FACTORY_RESET_POINT_NO, GUI_START, oled_display_config_factory_reset_gui},
    //设备信息
    {GUI_DEVIVE_LOAD, GUI_DEVIVE_LOAD, GUI_MAIN_MENU_COMMON_CONFIG, oled_display_device_info_gui},
    {GUI_DEVIVE_INFO_SIGNAL, GUI_DEVIVE_INFO_SIGNAL, GUI_MAIN_MENU_COMMON_CONFIG, oled_display_device_info_gui},
//    {GUI_DEVIVE_INFO_NUMBER, GUI_DEVIVE_INFO_NUMBER, GUI_MAIN_MENU_COMMON_CONFIG, oled_display_device_info_gui},
    //站号配置
//    {GUI_DEVIVE_MODBUS_START, GUI_DEVIVE_MODBUS_CONTINUE, GUI_DEVIVE_MODBUS_STARTING, oled_display_config_modbus_set_gui},
//    {GUI_DEVIVE_MODBUS_CONTINUE, GUI_DEVIVE_MODBUS_CANCEL, GUI_DEVIVE_MODBUS_CONTINUEING, oled_display_config_modbus_set_gui},
//    {GUI_DEVIVE_MODBUS_CANCEL, GUI_DEVIVE_MODBUS_START, GUI_MAIN_MENU_COMMON_CONFIG, oled_display_config_modbus_set_gui},
    //站号配置中
//    {GUI_DEVIVE_MODBUS_STARTING, GUI_DEVIVE_MODBUS_STARTING, GUI_DEVIVE_MODBUS_START, oled_display_config_modbus_setting_gui},
//    {GUI_DEVIVE_MODBUS_CONTINUEING, GUI_DEVIVE_MODBUS_CONTINUEING, GUI_DEVIVE_MODBUS_CONTINUE, oled_display_config_modbus_setting_gui},    

};

TaskHandle_t gKalykeOledKeyTaskHandle;


/* Private function prototypes -----------------------------------------------*/




/* Private user code ---------------------------------------------------------*/
/**
  * @brief button 1 read Pin
  * @retval None
  */
uint8_t button_key1_read(void)
{
    return GPIO_PinRead(SW_1_Pin, SW_1_GPIO_Port);
}

/**
  * @brief button 2 read Pin
  * @retval None
  */
uint8_t button_key2_read(void)
{
    return GPIO_PinRead(SW_2_Pin, SW_2_GPIO_Port);
}

/**
  * @brief key init
  * @retval None
  */
void iotb_key_init(void)
{
    uint8_t i;

    memset(&iotb_user_button[0], 0x0, sizeof(iotb_user_button));

    iotb_user_button[USER_BUTTON_1].usr_button_read = button_key1_read;
    iotb_user_button[USER_BUTTON_1].cb = (flex_button_response_callback)btn_1_cb;

    iotb_user_button[USER_BUTTON_2].usr_button_read = button_key2_read;
    iotb_user_button[USER_BUTTON_2].cb = (flex_button_response_callback)btn_2_cb;

    for (i = 0; i < USER_BUTTON_MAX; i ++)
    {
        iotb_user_button[i].status = 0;
        iotb_user_button[i].pressed_logic_level = KEY_TURN_ON_LEVEL;

#if 0 //default
        iotb_user_button[i].click_start_tick = 20;         //click press: keep time > click_start_tick * iotb_key_scan_cycle
        iotb_user_button[i].short_press_start_tick = 100;  //short press: keep time > short_press_start_tick * iotb_key_scan_cycle
        iotb_user_button[i].long_press_start_tick = 200;   //long press: keep time > long_press_start_tick * iotb_key_scan_cycle
        iotb_user_button[i].long_hold_start_tick = 300;    //long hold: keep time > long_hold_start_tick * iotb_key_scan_cycle
#else
        iotb_user_button[i].click_start_tick = 12;         //click press: keep time > click_start_tick * iotb_key_scan_cycle
        iotb_user_button[i].short_press_start_tick = 60;  //short press: keep time > short_press_start_tick * iotb_key_scan_cycle
        iotb_user_button[i].long_press_start_tick = 120;   //long press: keep time > long_press_start_tick * iotb_key_scan_cycle
        iotb_user_button[i].long_hold_start_tick = 180;    //long hold: keep time > long_hold_start_tick * iotb_key_scan_cycle
#endif

        flex_button_register(&iotb_user_button[i]);
    }
}


/**
  * @brief  显示标题内容
  * @param  title 显示标题
  * @param  colour 颜色
  * @retval None
  */
void oled_display_title_content(uint8_t title, uint8_t colour)
{
    uint8_t x, y, index;

    index = 0;
    y = 0;
    OLED_Fill_Part( colour == OLED_WHITE_ON_BLACK ? OLED_DISPLAY_FILL_BLACK : OLED_DISPLAY_FILL_WHILE, 0, 128, y, y + 2);
    switch(title)
    {
    case GUI_TITLE_MAIN_MENU:
        x = 40;
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_zhu, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_cai, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_dan, colour);
        break;

    case GUI_TITLE_COMMON_CONFIG:
        x = 32;
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_tong, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_yong, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_she, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_zhi, colour);
        break;

    case GUI_TITLE_COMMON_CONFIG_SERIAL_NUMBER:
        x = 40;
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_xu, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_lie, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_hao, colour);
        break;

    case GUI_TITLE_COMMON_CONFIG_FMW_VERSION:
        x = 32;
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_gu, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_jian1, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_ban, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_ben, colour);
        break;

    case GUI_TITLE_COMMON_CONFIG_FACTORY_RESET:
        x = 32;
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_shi2, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_fou, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_hui, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_fu, colour);
        break;
    
    case GUI_TITLE_LOAD:
        x = 32;
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_she, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_bei, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_xin, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_xi, colour);
        break;
    
    case GUI_TITLE_MODBUS_SET:
        x = 32;
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_zhan, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_hao, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_pei, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_zhi, colour);
        break;

    default:
        break;
    }

}

/**
  * @brief  显示标题
  * @param  title 显示标题
  *            @arg GUI_TITLE_MAIN_MENU: 主菜单
  *            @arg GUI_TITLE_COMMON_CONFIG: 通用设置
  * @param  part 显示箭头
  *            @arg GUI_TITLE_HAVE_NONE:         XXX
  *            @arg GUI_TITLE_HAVE_LEFT:       < XXX
  *            @arg GUI_TITLE_HAVE_RIGHT:        XXX >
  *            @arg GUI_TITLE_HAVE_LEFT_RIGHT: < XXX >
  * @param  clear 清除使能
  *            @arg OLED_DISPLAY_KEEP: 保持（不清除）
  *            @arg OLED_DISPLAY_CLEAR: 清除
  * @param  colour 颜色
  *            @arg OLED_WHITE_ON_BLACK: 黑底白字
  *            @arg OLED_BLACK_ON_WHITE: 白底黑字
  * @retval None
  */
void oled_display_title(uint8_t title, uint8_t part, uint8_t clear, uint8_t colour)
{
    uint8_t x1, x2, y;

    if(clear == OLED_DISPLAY_CLEAR)
    {
        oled_display_title_content(title, colour);
    }

    x1 = 1;
    x2 = 16 * 7 - 1;
    y = 0;

    if(colour != OLED_BLACK_ON_WHITE)
    {
        OLED_ShowCN(x1, y, OLED_CHAR_zuojiantou0, colour);
        OLED_ShowCN(x2, y, OLED_CHAR_youjiantou0, colour);
    }

    if(part & GUI_TITLE_HAVE_LEFT)
    {
        OLED_ShowCN(x1, y, OLED_CHAR_zuojiantou, colour);
    }

    if(part & GUI_TITLE_HAVE_RIGHT)
    {
        OLED_ShowCN(x2, y, OLED_CHAR_youjiantou, colour);
    }
}

/**
  * @brief  通用设置显示部分内容
  * @param  part 显示部分
  *            @arg GUI_COMMON_CONFIG_SERIAL_NUMBER: 1.序列号
  *            @arg GUI_COMMON_CONFIG_FMW_VERSION: 2.固件版本
  *            @arg GUI_COMMON_CONFIG_FACTORY_RESET: 3.恢复出厂设置
  *            @arg GUI_COMMON_CONFIG_RESET: 4.重启设备
  * @param  colour 显示颜色
  *            @arg OLED_BLACK_ON_WHITE: 白底黑字
  *            @arg OLED_WHITE_ON_BLACK: 黑底白字
  * @retval None
  */
void oled_display_common_config_gui_part(uint8_t part, uint8_t colour)
{
    uint8_t x, y, index;

    switch(part)
    {
    case GUI_COMMON_CONFIG_SERIAL_NUMBER:
        x = 0;
        y = 2;
        index = 0;
        OLED_Fill_Part( colour == OLED_WHITE_ON_BLACK ? OLED_DISPLAY_FILL_BLACK : OLED_DISPLAY_FILL_WHILE, 0, 128, y, y + 2);
        OLED_ShowStr(x, y, (uint8_t *)"1. ", OLED_CHAR_EN_SIZE_8x16, colour);
        x = 24;
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_xu, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_lie, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_hao, colour);
        break;

    case GUI_COMMON_CONFIG_FMW_VERSION:
        x = 0;
        y = 4;
        index = 0;
        OLED_Fill_Part( colour == OLED_WHITE_ON_BLACK ? OLED_DISPLAY_FILL_BLACK : OLED_DISPLAY_FILL_WHILE, 0, 128, y, y + 2);
        OLED_ShowStr(x, y, (uint8_t *)"2. ", OLED_CHAR_EN_SIZE_8x16, colour);
        x = 24;
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_gu, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_jian1, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_ban, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_ben, colour);
        break;

    case GUI_COMMON_CONFIG_FACTORY_RESET:
        x = 0;
        y = 6;
        index = 0;
        OLED_Fill_Part( colour == OLED_WHITE_ON_BLACK ? OLED_DISPLAY_FILL_BLACK : OLED_DISPLAY_FILL_WHILE, 0, 128, y, y + 2);
        OLED_ShowStr(x, y, (uint8_t *)"3. ", OLED_CHAR_EN_SIZE_8x16, colour);
        x = 24;
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_hui, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_fu, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_chu, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_chang1, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_she, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_zhi, colour);
        break;

    case GUI_COMMON_CONFIG_RESET:
        x = 0;
        y = 2;
        index = 0;
        OLED_Fill_Part( colour == OLED_WHITE_ON_BLACK ? OLED_DISPLAY_FILL_BLACK : OLED_DISPLAY_FILL_WHILE, 0, 128, y, y + 2);
        OLED_ShowStr(x, y, (uint8_t *)"4. ", OLED_CHAR_EN_SIZE_8x16, colour);
        x = 24;
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_chong, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_qi2, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_she, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_bei, colour);
        break;

    default:
        break;
    }
}

/**
  * @brief  通用设置
            ————————————
            |       通用设置       |
            | 1.序列号             |
            | 2.固件版本           |
            | 3.恢复出厂设置       |
            ————————————
  * @param  page_index 页面索引
  * @param  key_val 按键值
  * @retval None
  */
void oled_display_common_config_gui(uint8_t page_index, uint8_t key_val)
{
    switch(gOledRunInfo.last_index)
    {
    case GUI_COMMON_CONFIG_SERIAL_NUMBER:
    case GUI_COMMON_CONFIG_FMW_VERSION:
        oled_display_common_config_gui_part(gOledRunInfo.last_index, OLED_WHITE_ON_BLACK); //变为黑底白字
        break;

    case GUI_COMMON_CONFIG_FACTORY_RESET: //第1页最后一行
        oled_display_title(GUI_TITLE_COMMON_CONFIG, GUI_TITLE_HAVE_LEFT, OLED_DISPLAY_KEEP, OLED_WHITE_ON_BLACK);
        OLED_Fill_Part(OLED_DISPLAY_FILL_BLACK, 0, 128, 2, 8); //下3行 清屏
        oled_display_common_config_gui_part(GUI_COMMON_CONFIG_RESET, OLED_WHITE_ON_BLACK);
        break;

    case GUI_COMMON_CONFIG_RESET:
        oled_display_title(GUI_TITLE_COMMON_CONFIG, GUI_TITLE_HAVE_RIGHT, OLED_DISPLAY_KEEP, OLED_WHITE_ON_BLACK);
        OLED_Fill_Part(OLED_DISPLAY_FILL_BLACK, 0, 128, 2, 8); //下3行 清屏
        oled_display_common_config_gui_part(GUI_COMMON_CONFIG_SERIAL_NUMBER, OLED_WHITE_ON_BLACK);
        oled_display_common_config_gui_part(GUI_COMMON_CONFIG_FMW_VERSION, OLED_WHITE_ON_BLACK);
        oled_display_common_config_gui_part(GUI_COMMON_CONFIG_FACTORY_RESET, OLED_WHITE_ON_BLACK);
        break;

    default:
        OLED_Fill(OLED_DISPLAY_FILL_BLACK); //清屏
        if(page_index >= GUI_COMMON_CONFIG_SERIAL_NUMBER && page_index <= GUI_COMMON_CONFIG_FACTORY_RESET)
        {
            oled_display_title(GUI_TITLE_COMMON_CONFIG, GUI_TITLE_HAVE_RIGHT, OLED_DISPLAY_CLEAR, OLED_WHITE_ON_BLACK);
            oled_display_common_config_gui_part(GUI_COMMON_CONFIG_SERIAL_NUMBER, OLED_WHITE_ON_BLACK);
            oled_display_common_config_gui_part(GUI_COMMON_CONFIG_FMW_VERSION, OLED_WHITE_ON_BLACK);
            oled_display_common_config_gui_part(GUI_COMMON_CONFIG_FACTORY_RESET, OLED_WHITE_ON_BLACK);
        }
        else if(page_index >= GUI_COMMON_CONFIG_RESET && page_index <= GUI_COMMON_CONFIG_RESET)
        {
            oled_display_title(GUI_TITLE_COMMON_CONFIG, GUI_TITLE_HAVE_LEFT, OLED_DISPLAY_CLEAR, OLED_WHITE_ON_BLACK);
            oled_display_common_config_gui_part(GUI_COMMON_CONFIG_RESET, OLED_WHITE_ON_BLACK);
        }
        break;
    }

    oled_display_common_config_gui_part(page_index, OLED_BLACK_ON_WHITE); //显示当前行

    gOledRunInfo.last_index = page_index;
}

/**
  * @brief  序列号
            ————————————
            |        序列号        |
            |   KSDE1E2220501001   |
            |                      |
            |                      |
            ————————————
  * @param  page_index 页面索引
  * @param  key_val 按键值
  * @retval None
  */
void oled_display_config_serial_number_gui(uint8_t page_index, uint8_t key_val)
{
    uint8_t str[13];

    OLED_Fill(OLED_DISPLAY_FILL_BLACK); //清屏
    oled_display_title(GUI_TITLE_COMMON_CONFIG_SERIAL_NUMBER, GUI_TITLE_HAVE_NONE, OLED_DISPLAY_CLEAR, OLED_BLACK_ON_WHITE);
    //memcpy(str, "2306F0ID0001", 12);
    memcpy((char *)str, (char *)gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId, 12);
    str[12] = '\0';
    OLED_ShowStr(0, 2, str, OLED_CHAR_EN_SIZE_8x16, OLED_WHITE_ON_BLACK);

    gOledRunInfo.last_index = page_index;
}

/**
  * @brief  固件版本
            ————————————
            |       固件版本       |
            |   FLK-1.2.0818.01    |
            |                      |
            |                      |
            ————————————
  * @param  page_index 页面索引
  * @param  key_val 按键值
  * @retval None
  */
void oled_display_config_fmw_version_gui(uint8_t page_index, uint8_t key_val)
{
    uint8_t x;

    OLED_Fill(OLED_DISPLAY_FILL_BLACK); //清屏
    oled_display_title(GUI_TITLE_COMMON_CONFIG_SERIAL_NUMBER, GUI_TITLE_HAVE_NONE, OLED_DISPLAY_CLEAR, OLED_BLACK_ON_WHITE);
    x = 0;
    OLED_ShowStr(x, 2, (uint8_t *)"FLK-", OLED_CHAR_EN_SIZE_8x16, OLED_WHITE_ON_BLACK);
    x += 32;
    OLED_ShowStr(x, 2, (uint8_t *)SW_VERSION, OLED_CHAR_EN_SIZE_8x16, OLED_WHITE_ON_BLACK);

    gOledRunInfo.last_index = page_index;
}

/**
  * @brief  显示部分内容
  * @param  part 显示部分
  * @param  colour 显示颜色
  *            @arg OLED_BLACK_ON_WHITE: 白底黑字
  *            @arg OLED_WHITE_ON_BLACK: 黑底白字
  * @retval None
  */
void oled_display_config_factory_reset_gui_part(uint8_t part, uint8_t colour)
{
    uint8_t x, y, index;

    switch(part)
    {
    case GUI_CONFIG_FACTORY_RESET_POINT_NO:
        x = 0;
        y = 2;
        index = 0;
        OLED_Fill_Part( colour == OLED_WHITE_ON_BLACK ? OLED_DISPLAY_FILL_BLACK : OLED_DISPLAY_FILL_WHILE, 0, 128, y, y + 2);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_fou, colour);
        break;

    case GUI_CONFIG_FACTORY_RESET_POINT_YES:
        x = 0;
        y = 4;
        index = 0;
        OLED_Fill_Part( colour == OLED_WHITE_ON_BLACK ? OLED_DISPLAY_FILL_BLACK : OLED_DISPLAY_FILL_WHILE, 0, 128, y, y + 2);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_shi2, colour);
        break;

    default:
        break;
    }
}

/**
  * @brief  恢复出厂设置
  * @param  page_index 页面索引
  * @param  key_val 按键值
  * @retval None
  */
void oled_display_config_factory_reset_gui(uint8_t page_index, uint8_t key_val)
{
    switch(gOledRunInfo.last_index)
    {
    case GUI_CONFIG_FACTORY_RESET_POINT_NO:
    case GUI_CONFIG_FACTORY_RESET_POINT_YES:
        oled_display_config_factory_reset_gui_part(gOledRunInfo.last_index, OLED_WHITE_ON_BLACK); //变为黑底白字
        break;

    default:
        OLED_Fill(OLED_DISPLAY_FILL_BLACK); //清屏
        oled_display_title(GUI_TITLE_COMMON_CONFIG_FACTORY_RESET, GUI_TITLE_HAVE_NONE, OLED_DISPLAY_CLEAR, OLED_WHITE_ON_BLACK);
        oled_display_config_factory_reset_gui_part(GUI_CONFIG_FACTORY_RESET_POINT_NO, OLED_WHITE_ON_BLACK);
        oled_display_config_factory_reset_gui_part(GUI_CONFIG_FACTORY_RESET_POINT_YES, OLED_WHITE_ON_BLACK);
        break;
    }

    oled_display_config_factory_reset_gui_part(page_index, OLED_BLACK_ON_WHITE); //显示当前行

    gOledRunInfo.last_index = page_index;
}

/**
  * @brief  主菜单内容
  * @param  part 显示部分
  *            @arg GUI_MAIN_MENU_COMMON_CONFIG: 通用设置
  * @param  colour 显示颜色
  *            @arg OLED_BLACK_ON_WHITE: 白底黑字
  *            @arg OLED_WHITE_ON_BLACK: 黑底白字
  * @retval None
  */
void oled_display_main_menu_gui_part(uint8_t part, uint8_t colour)
{
    uint8_t x, y, index;

    switch(part)
    {
    case GUI_MAIN_MENU_COMMON_CONFIG: //1.通用设置
        y = 2;
        index = 0;
        OLED_Fill_Part( colour == OLED_WHITE_ON_BLACK ? OLED_DISPLAY_FILL_BLACK : OLED_DISPLAY_FILL_WHILE, 0, 128, y, y + 2);
        OLED_ShowStr(x, y, (uint8_t *)"1. ", OLED_CHAR_EN_SIZE_8x16, colour);
        x = 24;
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_tong, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_yong, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_she, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_zhi, colour);
        break;
    
    case GUI_MAIN_MENU_DEVICE_INFO:       //2.设备信息
        y = 4;
        index = 0;
        OLED_Fill_Part( colour == OLED_WHITE_ON_BLACK ? OLED_DISPLAY_FILL_BLACK : OLED_DISPLAY_FILL_WHILE, 0, 128, y, y + 2);
        OLED_ShowStr(x, y, (uint8_t *)"2. ", OLED_CHAR_EN_SIZE_8x16, colour);
        x = 24;
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_she, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_bei, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_xin, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_xi, colour);
        break;
   
//    case GUI_MAIN_MENU_MODBUS_SET:       //3.站号配置
//        y = 6;
//        index = 0;
//        OLED_Fill_Part( colour == OLED_WHITE_ON_BLACK ? OLED_DISPLAY_FILL_BLACK : OLED_DISPLAY_FILL_WHILE, 0, 128, y, y + 2);
//        OLED_ShowStr(x, y, (uint8_t *)"3. ", OLED_CHAR_EN_SIZE_8x16, colour);
//        x = 24;
//        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_zhan, colour);
//        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_hao, colour);
//        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_pei, colour);
//        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_zhi, colour);
//        break;
    
    default:
        break;
    }
}

/**
  * @brief  主菜单
            ————————————
            |        主菜单      > |
            | 1.通用设置           |
            | 2.设备信息           |
            | 3.站号配置           |
            ————————————
  * @param  page_index 页面索引
  * @param  key_val 按键值
  * @retval None
  */
void oled_display_main_menu_gui(uint8_t page_index, uint8_t key_val)
{
    switch(gOledRunInfo.last_index)
    {
    case GUI_MAIN_MENU_COMMON_CONFIG:
    case GUI_MAIN_MENU_DEVICE_INFO:
//    case GUI_MAIN_MENU_MODBUS_SET:
        oled_display_main_menu_gui_part(gOledRunInfo.last_index, OLED_WHITE_ON_BLACK);
        break;

    default:
        OLED_Fill(0); //清屏
        if(page_index >= GUI_MAIN_MENU_COMMON_CONFIG && page_index <= GUI_MAIN_MENU_DEVICE_INFO)
        {
            oled_display_title(GUI_TITLE_MAIN_MENU, GUI_TITLE_HAVE_NONE, OLED_DISPLAY_CLEAR, OLED_WHITE_ON_BLACK); //   主菜单 >
            oled_display_main_menu_gui_part(GUI_MAIN_MENU_COMMON_CONFIG, OLED_WHITE_ON_BLACK);
            oled_display_main_menu_gui_part(GUI_MAIN_MENU_DEVICE_INFO, OLED_WHITE_ON_BLACK);
//            oled_display_main_menu_gui_part(GUI_MAIN_MENU_MODBUS_SET, OLED_WHITE_ON_BLACK);
        }
        break;
    }

    oled_display_main_menu_gui_part(page_index, OLED_BLACK_ON_WHITE); //显示当前行

    gOledRunInfo.last_index = page_index;
}

/**
  * @brief  界面
            ————————————
            |                      |
            |       操作中...      |
            |                      |
            ————————————
  * @param  None
  * @retval None
  */
void module_oled_display_operation(void)
{
    uint8_t x, index;
    OLED_Fill(0); //清屏

    x = 32;
    index = 0;
    OLED_ShowCN(x + (16 * index++), 3, OLED_CHAR_cao, OLED_WHITE_ON_BLACK);
    OLED_ShowCN(x + (16 * index++), 3, OLED_CHAR_zuo, OLED_WHITE_ON_BLACK);
    OLED_ShowCN(x + (16 * index++), 3, OLED_CHAR_zhong, OLED_WHITE_ON_BLACK);
    x = x + (16 * index++);
    OLED_ShowStr(x, 3, (uint8_t *)"...", OLED_CHAR_EN_SIZE_8x16, OLED_WHITE_ON_BLACK);
}

/**
  * @brief  界面
            ————————————
            |                      |
            |    请在终端确认      |
            |                      |
            ————————————
  * @param  None
  * @retval None
  */
void module_oled_display_config(void)
{
    uint8_t x, index;
    OLED_Fill(0); //清屏

    x = 16;
    index = 0;
    OLED_ShowCN(x + (16 * index++), 3, OLED_CHAR_qing, OLED_WHITE_ON_BLACK);
    OLED_ShowCN(x + (16 * index++), 3, OLED_CHAR_zai, OLED_WHITE_ON_BLACK);
    OLED_ShowCN(x + (16 * index++), 3, OLED_CHAR_zh0ng, OLED_WHITE_ON_BLACK);
    OLED_ShowCN(x + (16 * index++), 3, OLED_CHAR_duan, OLED_WHITE_ON_BLACK);
    OLED_ShowCN(x + (16 * index++), 3, OLED_CHAR_que, OLED_WHITE_ON_BLACK);
    OLED_ShowCN(x + (16 * index++), 3, OLED_CHAR_ren, OLED_WHITE_ON_BLACK);
}

/**
  * @brief  开始界面
            ————————————
            |                      |
            |       智能网关       |
            |                      |
            ————————————
  * @param  page_index 页面索引
  * @param  key_val 按键值
  * @retval None
  */
void mb_slave_ctrl_clean_all_user_data(md_slave_msg_pack *pMsg);
void module_oled_display_start_gui(uint8_t page_index, uint8_t key_val)
{
    uint8_t x, index;
    md_slave_msg_pack msg;

    if(gOledRunInfo.last_index == GUI_CONFIG_FACTORY_RESET_POINT_YES)
    {
        module_oled_display_operation();
        mb_slave_ctrl_clean_all_user_data(&msg);
        NVIC_SystemReset();
    }
    else if(gOledRunInfo.last_index == GUI_COMMON_CONFIG_RESET)
    {
        NVIC_SystemReset();
    }

    OLED_Fill(0); //清屏
    x = 32;
    index = 0;
    OLED_ShowCN(x + (16 * index++), 3, OLED_CHAR_zhi4, OLED_WHITE_ON_BLACK);
    OLED_ShowCN(x + (16 * index++), 3, OLED_CHAR_neng, OLED_WHITE_ON_BLACK);
    OLED_ShowCN(x + (16 * index++), 3, OLED_CHAR_wang, OLED_WHITE_ON_BLACK);
    OLED_ShowCN(x + (16 * index++), 3, OLED_CHAR_guan, OLED_WHITE_ON_BLACK);

    gOledRunInfo.last_index = page_index;
}

/**
  * @brief  通用设置显示部分内容
  * @param  part 显示部分
  *            @arg GUI_DEVIVE_INFO_SIGNAL: 1.4G信号
  *            @arg GUI_DEVIVE_INFO_NUMBER: 2.终端已连接
  * @param  colour 显示颜色
  *            @arg OLED_BLACK_ON_WHITE: 白底黑字
  *            @arg OLED_WHITE_ON_BLACK: 黑底白字
  * @retval None
  */
void oled_display_device_info_gui_part_info(uint8_t part, uint8_t colour, uint8_t clear)
{
    uint8_t rssi;
    uint8_t str[10];
    uint16_t str_len;
    str_len = 0;
    str_len += sprintf((char *)str, "%.u", lsv_collect);
    str[str_len++] = '\0';
    
    uint8_t str1[10];
    uint16_t str1_len;
    str1_len = 0;
    str1_len += sprintf((char *)str1, "%.u", collect_maxnum);
    str1[str1_len++] = '\0';
    
    switch(part)
    {
    case GUI_DEVIVE_LOAD:
        if(clear == OLED_DISPLAY_CLEAR)
        {
            rssi = gtv_PlcElement.msp_SDElement[SD226];
            rssi+=100;
            //LOGE(TAG, "rssi = %d", rssi);
            if(rssi>20)
            {
                OLED_ShowCN(64, 4, OLED_CHAR_zheng, colour);
                OLED_ShowCN(80, 4, OLED_CHAR_chang, colour);
            }
            else if(rssi<20 && rssi>0)
            {
                OLED_ShowCN(64, 4, OLED_CHAR_yl, colour);
                OLED_ShowCN(80, 4, OLED_CHAR_baN, colour);
            }
            else
            {
                OLED_ShowCN(64, 4, OLED_CHAR_ji, colour);
                OLED_ShowCN(80, 4, OLED_CHAR_cha, colour);
            }
            OLED_ShowStr(74, 6, str, OLED_CHAR_EN_SIZE_8x16, colour);
            
//            if(collect_maxnum<10)
//            {
//                OLED_ShowStr(88, 6, (uint8_t *)"/ ", OLED_CHAR_EN_SIZE_8x16, colour);
//                OLED_ShowStr(102, 6, str1, OLED_CHAR_EN_SIZE_8x16, colour);
//            }
//            else
//            {
//                OLED_ShowStr(92, 6, (uint8_t *)"/ ", OLED_CHAR_EN_SIZE_8x16, colour);
//                OLED_ShowStr(104, 6, str1, OLED_CHAR_EN_SIZE_8x16, colour);
//            }
        }
        else
        {
            OLED_Fill_Part(OLED_DISPLAY_FILL_BLACK, 64, 128, 4, 6);
            OLED_Fill_Part(OLED_DISPLAY_FILL_BLACK, 74, 128, 6, 8);
        }
        break;

    default:
        break;
    }
}

void oled_display_device_info_gui_part(uint8_t part, uint8_t colour)
{
    uint8_t x, y, index;
    
    switch(part)
    {
    case GUI_DEVIVE_INFO_SIGNAL:
        x = 0;
        y = 4;
        index = 0;
        OLED_Fill_Part( colour == OLED_WHITE_ON_BLACK ? OLED_DISPLAY_FILL_BLACK : OLED_DISPLAY_FILL_WHILE, 0, 128, y, y + 2);
        OLED_ShowStr(x + (16 * index++), y, (uint8_t *)"4G", OLED_CHAR_EN_SIZE_8x16, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_xin, colour);
        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_hao, colour);
        OLED_ShowStr(x + (16 * index++), y, (uint8_t *)": ", OLED_CHAR_EN_SIZE_8x16, colour);
        break;

//    case GUI_DEVIVE_INFO_NUMBER:
//        x = 0;
//        y = 6;
//        index = 0;
//        OLED_Fill_Part( colour == OLED_WHITE_ON_BLACK ? OLED_DISPLAY_FILL_BLACK : OLED_DISPLAY_FILL_WHILE, 0, 128, y, y + 2);
//        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_zh0ng, colour);
//        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_duan, colour);
//        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_lian, colour);
//        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_jie, colour);
//        OLED_ShowStr(x + (16 * index++), y, (uint8_t *)": ", OLED_CHAR_EN_SIZE_8x16, colour);
//        break;

    default:
        break;
    }
}

/**
  * @brief  待机界面
            ————————————
            |       设备信息       |
            | 4G信号：             |
            | 终端已连接：         |
            |                      |
            ————————————
  * @param  page_index 页面索引
  * @param  key_val 按键值
  * @retval None
  */
void oled_display_device_info_gui(uint8_t page_index, uint8_t key_val)
{
    uint8_t clear;
    uint8_t str[13];
    memcpy((char *)str, (char *)gtv_DeviceConfigTable.mtv_DevInfo.mcv_DeviceId, 12);
    str[12] = '\0';
    
    OLED_Fill(OLED_DISPLAY_FILL_BLACK); //清屏
    
    switch(page_index)
    {
    case GUI_DEVIVE_LOAD:    
        oled_display_title(GUI_TITLE_LOAD, GUI_TITLE_HAVE_NONE, OLED_DISPLAY_CLEAR, OLED_BLACK_ON_WHITE);
        OLED_ShowStr(0, 2, str, OLED_CHAR_EN_SIZE_8x16, OLED_WHITE_ON_BLACK);
        oled_display_device_info_gui_part(GUI_DEVIVE_INFO_SIGNAL, OLED_WHITE_ON_BLACK);
//        oled_display_device_info_gui_part(GUI_DEVIVE_INFO_NUMBER, OLED_WHITE_ON_BLACK);
        break;
    case GUI_DEVIVE_INFO_SIGNAL:
//    case GUI_DEVIVE_INFO_NUMBER:
        break;

    default:
        break;
    }

    //oled_display_device_info_gui_part_info(page_index, OLED_WHITE_ON_BLACK, clear); //显示当前行

    gOledRunInfo.last_index = page_index;
}

void oled_display_config_modbus_set_gui_part(uint8_t part, uint8_t colour)
{
    uint8_t x, y, index;

    switch(part)
    {
//    case GUI_DEVIVE_MODBUS_START:    //1.从零开始配置
//        y = 2;
//        index = 0;
//        OLED_Fill_Part( colour == OLED_WHITE_ON_BLACK ? OLED_DISPLAY_FILL_BLACK : OLED_DISPLAY_FILL_WHILE, 0, 128, y, y + 2);
//        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_kai, colour);
//        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_shi, colour);
//        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_pei, colour);
//        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_zhi, colour);
//        break;
//    
//    case GUI_DEVIVE_MODBUS_CONTINUE:   //2.先扫描再配置
//        y = 4;
//        index = 0;
//        OLED_Fill_Part( colour == OLED_WHITE_ON_BLACK ? OLED_DISPLAY_FILL_BLACK : OLED_DISPLAY_FILL_WHILE, 0, 128, y, y + 2);
//        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_tian, colour);
//        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_jia, colour);
//        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_she, colour);
//        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_bei, colour);
//        break;
//    
//    case GUI_DEVIVE_MODBUS_CANCEL:    //3.返回主菜单
//        y = 6;
//        index = 0;
//        OLED_Fill_Part( colour == OLED_WHITE_ON_BLACK ? OLED_DISPLAY_FILL_BLACK : OLED_DISPLAY_FILL_WHILE, 0, 128, y, y + 2);
//        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_fan, colour);
//        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_hul, colour);
//        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_zhu, colour);
//        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_cai, colour);
//        OLED_ShowCN(x + (16 * index++), y, OLED_CHAR_dan, colour);
//        break;
    
    default:
        break;
    }
}

void oled_display_config_modbus_set_gui(uint8_t page_index, uint8_t key_val)
{
    switch(gOledRunInfo.last_index)
    {
//    case GUI_DEVIVE_MODBUS_START:
//    case GUI_DEVIVE_MODBUS_CONTINUE:
//    case GUI_DEVIVE_MODBUS_CANCEL:
//        oled_display_config_modbus_set_gui_part(gOledRunInfo.last_index, OLED_WHITE_ON_BLACK);
//        break;
//    case GUI_DEVIVE_MODBUS_STARTING:
//        SETRtuIDED();
//        break;

//    default:
//        OLED_Fill(0); //清屏
//        if(page_index >= GUI_DEVIVE_MODBUS_START && page_index <= GUI_DEVIVE_MODBUS_CANCEL)
//        {
//            oled_display_title(GUI_TITLE_MODBUS_SET, GUI_TITLE_HAVE_NONE, OLED_DISPLAY_CLEAR, OLED_WHITE_ON_BLACK); //   主菜单 >
//            oled_display_config_modbus_set_gui_part(GUI_DEVIVE_MODBUS_START, OLED_WHITE_ON_BLACK);
//            oled_display_config_modbus_set_gui_part(GUI_DEVIVE_MODBUS_CONTINUE, OLED_WHITE_ON_BLACK);
//            oled_display_config_modbus_set_gui_part(GUI_DEVIVE_MODBUS_CANCEL, OLED_WHITE_ON_BLACK);
//        }
//        break;
    }

    oled_display_config_modbus_set_gui_part(page_index, OLED_BLACK_ON_WHITE); //显示当前行

    gOledRunInfo.last_index = page_index;
}

void oled_display_config_modbus_setting_gui(uint8_t page_index, uint8_t key_val)
{
    switch(gOledRunInfo.cur_index)
    {
//    case GUI_DEVIVE_MODBUS_STARTING:
//        module_oled_display_config();
//        SETRtuID();
//        break;
//    case GUI_DEVIVE_MODBUS_CONTINUEING:
//        module_oled_display_config();
//        ADDRtuID();
//        break;

    default:
        break;
    }

    oled_display_config_modbus_set_gui_part(page_index, OLED_BLACK_ON_WHITE); //显示当前行

    gOledRunInfo.last_index = page_index;
}

/**
  * @brief  参数刷新显示
  * @param  None
  * @retval None
  */
void module_oled_refresh_param_gui(void)
{
    static uint32_t mstick;
    static uint32_t times;

    if(xTaskGetTickCount() - mstick < TIME_REFRESH_TO_DISPLAY_GUI) //刷新时间未到
    {
        oled_display_device_info_gui_part_info(gOledRunInfo.cur_index, OLED_WHITE_ON_BLACK, OLED_DISPLAY_CLEAR);
        return;
    }
    mstick = xTaskGetTickCount();
    
    if(gOledRunInfo.cur_index == GUI_DEVIVE_LOAD)
    {
        oled_display_device_info_gui_part_info(gOledRunInfo.cur_index, OLED_WHITE_ON_BLACK, OLED_DISPLAY_KEEP);
    }
    
    if(gOledRunInfo.cur_index != GUI_DEVIVE_LOAD)
    {
        times++;
        if(times>60)
        {
            gOledRunInfo.cur_index = GUI_DEVIVE_LOAD;
            g_oled_display_table[gOledRunInfo.cur_index].current_operation(gOledRunInfo.cur_index, NULL);
            times = 0;
        }
    }
    
}

/**
  * @brief button 1 Callback
  * @retval None
  */
void btn_1_cb(flex_button_t *btn)
{
    switch(btn->event)
    {
    case FLEX_BTN_PRESS_DOWN:
        LOGD("oled_key", "Button 1 FLEX_BTN_PRESS_DOWN");
        break;

    case FLEX_BTN_PRESS_CLICK:
        LOGD("oled_key", "Button 1 FLEX_BTN_PRESS_CLICK");
        gOledRunInfo.cur_index = g_oled_display_table[gOledRunInfo.cur_index].next;
        g_oled_display_table[gOledRunInfo.cur_index].current_operation(gOledRunInfo.cur_index, btn->event);
        break;

    case FLEX_BTN_PRESS_DOUBLE_CLICK:
        LOGD("oled_key", "Button 1 FLEX_BTN_PRESS_DOUBLE_CLICK");
        break;

    case FLEX_BTN_PRESS_SHORT_START:
        LOGD("oled_key", "Button 1 FLEX_BTN_PRESS_SHORT_START");
        gOledRunInfo.cur_index = g_oled_display_table[gOledRunInfo.cur_index].next;
        g_oled_display_table[gOledRunInfo.cur_index].current_operation(gOledRunInfo.cur_index, btn->event);
        break;

    default:
        LOGW("oled_key", "Button 1 other btn->event: %d", btn->event);
        break;
    }
}

/**
  * @brief button 2 Callback
  * @retval None
  */
void btn_2_cb(flex_button_t *btn)
{
    CLEAR_BIT(gOledRunInfo.flag, OLED_FLAG_LOOP_PARAM); //确认键退出轮询显示

    switch(btn->event)
    {
    case FLEX_BTN_PRESS_DOWN:
        LOGD("oled_key", "Button 2 FLEX_BTN_PRESS_DOWN");
        break;

    case FLEX_BTN_PRESS_CLICK:
        LOGD("oled_key", "Button 2 FLEX_BTN_PRESS_CLICK");
        gOledRunInfo.cur_index = g_oled_display_table[gOledRunInfo.cur_index].enter;
        g_oled_display_table[gOledRunInfo.cur_index].current_operation(gOledRunInfo.cur_index, btn->event);
        break;

    case FLEX_BTN_PRESS_DOUBLE_CLICK:
        LOGD("oled_key", "Button 2 FLEX_BTN_PRESS_DOUBLE_CLICK");
        break;

    case FLEX_BTN_PRESS_SHORT_START:
        LOGD("oled_key", "Button 2 FLEX_BTN_PRESS_SHORT_START");
        break;

    default:
        LOGW("oled_key", "Button 2 other btn->event: %d", btn->event);
        break;
    }
}

/**
  * @brief  Function implementing the keyOledTask thread.
  * @param  argument: Not used
  * @retval None
  */
void kalyke_oled_key_task(void *pvParameters)
{
    //vTaskDelay(1000);
    OLED_Init();

    iotb_key_init();

    gOledRunInfo.flag = 0;
    gOledRunInfo.cur_index = GUI_START;
    gOledRunInfo.last_index = GUI_START;

    module_oled_display_start_gui(gOledRunInfo.cur_index, NULL);
    vTaskDelay(TIME_OUT_TO_DISPLAY_START_GUI);

#if 1
    gOledRunInfo.cur_index = GUI_DEVIVE_LOAD;
    oled_display_device_info_gui(gOledRunInfo.cur_index, NULL);
#else
    gOledRunInfo.cur_index = GUI_MAIN_MENU_COMMON_CONFIG;
    oled_display_main_menu_gui(gOledRunInfo.cur_index, NULL);
#endif

    for(;;)
    {
        if (iotb_button_scan_enable)
        {
            flex_button_scan();
        }
        vTaskDelay(key_scan_cycle);
        
        module_oled_refresh_param_gui();
    }
}
