/*
 * Copyright (c) 2022-2023, Fexlink Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-06-23     JPeng        first implementation
 * 2022-07-11     Arrbow       code transplantation
 */

#ifndef __KALYKE_OLED_KEY_H
#define __KALYKE_OLED_KEY_H


/* Includes ------------------------------------------------------------------*/
#include "flexible_button.h"


/* Exported types ------------------------------------------------------------*/
/**
  @brief 各界面的索引值 （值小于8）
  */
typedef enum __OLED_GUI_INDEX_E
{
    GUI_START                               =   0,   /* 开始页面 */

    GUI_MAIN_MENU_COMMON_CONFIG             =   1,   /* 主菜单：通用设置 */
    GUI_MAIN_MENU_DEVICE_INFO               =   2,   /* 主菜单：设备信息 */
//    GUI_MAIN_MENU_MODBUS_SET                =   3,   /* 主菜单：一键配置 */

    GUI_COMMON_CONFIG_SERIAL_NUMBER         =   3,   /* 通用设置：序列号 */
    GUI_COMMON_CONFIG_FMW_VERSION           =   4,   /* 通用设置：固件版本 */
    GUI_COMMON_CONFIG_FACTORY_RESET         =   5,   /* 通用设置：恢复出厂设置 */
    GUI_COMMON_CONFIG_RESET                 =   6,   /* 通用设置：重启 */

    GUI_CONFIG_SERIAL_NUMBER                =   7,   /* 序列号 */
    GUI_CONFIG_FMW_VERSION                  =   8,   /* 固件版本 */
    GUI_CONFIG_FACTORY_RESET_POINT_NO       =   9,  /* 恢复出厂设置否 */
    GUI_CONFIG_FACTORY_RESET_POINT_YES      =   10,  /* 恢复出厂设置是 */
    
    GUI_DEVIVE_LOAD                         =   11, /* 待机界面 */
    GUI_DEVIVE_INFO_SIGNAL                  =   12, /* 信号 */
//    GUI_DEVIVE_INFO_NUMBER                  =   14, /* 终端数量 */
//    
//    GUI_DEVIVE_MODBUS_START                 =   15, /* 从一开始配置 */
//    GUI_DEVIVE_MODBUS_CONTINUE              =   16, /* 先扫描再配置 */
//    GUI_DEVIVE_MODBUS_CANCEL                =   17, /* 取消 */
//    
//    GUI_DEVIVE_MODBUS_STARTING              =   18, /* 从一开始配置中 */
//    GUI_DEVIVE_MODBUS_CONTINUEING           =   19, /* 先扫描再配置中 */

} oled_gui_index_e;

typedef struct
{
    unsigned char cur_index;//当前索引项
    unsigned char next;//选择
    unsigned char enter;//确定
    void (*current_operation)(unsigned char, unsigned char); //	当前索引执行的函数(界面)
} key_oled_gui_t;

typedef struct
{
    unsigned char cur_index; //当前页面索引
    unsigned char last_index; //上一页面索引
    unsigned char flag; //轮询显示参数标志
    unsigned char alarm_status; //报警状态
} key_oled_run_info_t;



/* Exported constants --------------------------------------------------------*/
extern TaskHandle_t gKalykeOledKeyTaskHandle;
extern volatile uint8_t lsv_collect, collect_maxnum;
extern volatile uint8_t GetAllID;
//extern volatile unsigned char key_scan_cycle;
//extern bool iotb_button_scan_enable;


/* Private defines -----------------------------------------------------------*/
#define  KEY_TURN_ON_LEVEL        1   //按键有效电平
#define  KEY_SCAN_CYCLE           20  //扫描按键频率(ms)

#define KEY_ON  1U
#define KEY_OFF 0U

#define SW_1_Pin GPIO1
#define SW_1_GPIO_Port (18U)
#define SW_2_Pin GPIO1
#define SW_2_GPIO_Port (19U)

typedef enum
{
    USER_BUTTON_1 = 0,
    USER_BUTTON_2,
    USER_BUTTON_MAX
} iotb_user_button_t;

/**
  @brief 时间
  */
#define TIME_OUT_TO_DISPLAY_START_GUI            5000   //显示开始界面时间（ms）
#define TIME_REFRESH_TO_DISPLAY_GUI              1000   //页面刷新时间（ms）
#define TIME_REFRESH_TO_LOOP_DISPLAY_GUI         10     //轮询切换界面时间等于10*TIME_REFRESH_TO_DISPLAY_GUI（ms）

/**
  @brief 显示位
  */
#define OLED_DISPLAY_POS_BIT_COLOUR(cur_bit, set_bit)  ((set_bit == cur_bit) ? OLED_BLACK_ON_WHITE : OLED_WHITE_ON_BLACK)

/**
  @brief 通用标题 （值小于20）
  */
#define GUI_TITLE_MAIN_MENU                             0   //主菜单
#define GUI_TITLE_COMMON_CONFIG                         1   //通用设置
#define GUI_TITLE_DEVICE_INFO                           2   //设备信息

#define GUI_TITLE_COMMON_CONFIG_SERIAL_NUMBER           3   //序列号
#define GUI_TITLE_COMMON_CONFIG_FMW_VERSION             4   //固件版本
#define GUI_TITLE_COMMON_CONFIG_FACTORY_RESET           5   //恢复出厂设置
#define GUI_TITLE_LOAD                                  6   //待机界面
#define GUI_TITLE_MODBUS_SET                            7   //站号配置
/**
  @brief 左右箭头
  */
#define GUI_TITLE_HAVE_NONE          0   //没有箭头
#define GUI_TITLE_HAVE_LEFT          1   //有左箭头
#define GUI_TITLE_HAVE_RIGHT         2   //有右箭头
#define GUI_TITLE_HAVE_LEFT_RIGHT    3   //有左右箭头

/**
  @brief 清除保持
  */
#define OLED_DISPLAY_CLEAR           0   //清屏
#define OLED_DISPLAY_KEEP            1   //保持

/**
  @brief 标志位定义
  */
#define OLED_FLAG_LOOP_PARAM     0x01


/* Private functions ---------------------------------------------------------*/
extern unsigned char button_key1_read(void);
extern unsigned char button_key2_read(void);
extern void btn_1_cb(flex_button_t *btn);
extern void btn_2_cb(flex_button_t *btn);
extern void iotb_key_init(void);

void kalyke_oled_key_task(void *pvParameters);

extern void oled_display_main_menu_gui(unsigned char page_index, unsigned char key_val);
extern void oled_display_common_config_gui(unsigned char page_index, unsigned char key_val);
extern void oled_display_config_serial_number_gui(unsigned char page_index, unsigned char key_val);
extern void oled_display_config_fmw_version_gui(unsigned char page_index, unsigned char key_val);
extern void oled_display_config_factory_reset_gui(unsigned char page_index, unsigned char key_val);
extern void module_oled_display_start_gui(unsigned char page_index, unsigned char key_val);
extern void oled_display_device_info_gui(unsigned char page_index, unsigned char key_val);
extern void oled_display_config_modbus_set_gui(unsigned char page_index, unsigned char key_val);
extern void oled_display_config_modbus_setting_gui(unsigned char page_index, unsigned char key_val);
#endif /* __KALYKE_OLED_KEY_H */
