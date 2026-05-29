/*
 * Copyright (c) 2022-2023, Fexlink Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-06-23     Arrbow       first implementation
 */

/* Private includes ----------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "oled_i2c.h"
#include "oled_codetab.h"
#include "fsl_lpi2c.h"
#include "fsl_debug_console.h"
#include "fsl_lpi2c_freertos.h"


/* Private define ------------------------------------------------------------*/
#define I2C_Command_SIZE_8BIT           0U
#define I2C_Data_SIZE_8BIT              40U
#define I2C_MEMADD_SIZE_8BIT            1U

#define I2C_BAUDRATE                 (400000)
#define EXAMPLE_I2C_MASTER_BASE      (LPI2C1_BASE)
#define EXAMPLE_I2C_MASTER           ((LPI2C_Type *)EXAMPLE_I2C_MASTER_BASE)
/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_SELECT    (0U)
/* Clock divider for master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_DIVIDER   (5U)
/* Get frequency of lpi2c clock */
#define LPI2C_CLOCK_FREQUENCY        ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (LPI2C_CLOCK_SOURCE_DIVIDER + 1U))



/* Private variables ---------------------------------------------------------*/





/* Private function prototypes -----------------------------------------------*/






/* Private user code ---------------------------------------------------------*/
/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{
    status_t status;
    lpi2c_master_config_t masterConfig;
    lpi2c_rtos_handle_t master_rtos_handle;

    LPI2C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Hz = I2C_BAUDRATE;
    LPI2C_MasterInit(EXAMPLE_I2C_MASTER, &masterConfig, LPI2C_CLOCK_FREQUENCY);
    //    status = LPI2C_RTOS_Init(&master_rtos_handle, EXAMPLE_I2C_MASTER, &masterConfig, LPI2C_CLOCK_FREQUENCY);
    //    if (status != kStatus_Success)
    //    {
    //        PRINTF("LPI2C master: Error initializing LPI2C!\r\n");
    //        vTaskSuspend(NULL);
    //    }

}

bool LPI2C_WriteByte(uint8_t SalveAddr, uint8_t RegAddr, uint8_t *DateByte)
{
    lpi2c_master_transfer_t transfer;
    status_t err_flag;

    /*
    * @data         :要发送的数据
    * @datasize     :发送的数据个数
    * @direction    :读写模式选择
    * @flags        :传输失败的标志位
    * @slaveAaddress:从机地址
    * @subaddress   :寄存器/内存地址
    * @subaddressSize:地址寄存器大小
    */
    transfer.data = DateByte;
    transfer.dataSize = 1;
    transfer.direction = kLPI2C_Write;
    transfer.flags = kLPI2C_TransferDefaultFlag;
    transfer.slaveAddress = (SalveAddr >> 1);
    transfer.subaddress = RegAddr;
    transfer.subaddressSize = 0x01;
    err_flag = LPI2C_MasterTransferBlocking(LPI2C1, &transfer);

    if(err_flag != kStatus_Success)  return false;

    return true;
}



/**
  * @brief  向OLED写入命令
  * @param  I2C_Command：命令代码
  * @retval 无
  */
static void WriteCmd(unsigned char I2C_Command)
{
    LPI2C_WriteByte(OLED_I2C_ADDRESS, 0x00U, &I2C_Command);
}

/**
  * @brief  向OLED写入数据
  * @param  I2C_Data：数据
  * @retval 无
  */
static void WriteData(unsigned char I2C_Data)
{
    LPI2C_WriteByte(OLED_I2C_ADDRESS, 0x40U, &I2C_Data);
}

/**
  * @brief  OLED_Init，初始化OLED
  * @param  无
  * @retval 0：正常  1：异常
  */
unsigned char OLED_Init(void)
{
    MX_I2C1_Init();

    vTaskDelay(100);     // 1s,这里的延时很重要,上电后延时，没有错误的冗余设计

    unsigned char I2C_Command = 0xAE;
    if(LPI2C_WriteByte(OLED_I2C_ADDRESS, 0x00U, &I2C_Command) != true)  //display off
    {
        return 1;  //OLED disconnect
    }

    WriteCmd(0xAE); //display off
    WriteCmd(0x20);	//Set Memory Addressing Mode
    WriteCmd(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
    WriteCmd(0xb0);	//Set Page Start Address for Page Addressing Mode,0-7
    WriteCmd(0xc8);	//Set COM Output Scan Direction
    WriteCmd(0x00); //---set low column address
    WriteCmd(0x10); //---set high column address
    WriteCmd(0x40); //--set start line address
    WriteCmd(0x81); //--set contrast control register
    WriteCmd(0xff); //亮度调节 0x00~0xff
    WriteCmd(0xa1); //--set segment re-map 0 to 127
    WriteCmd(0xa6); //--set normal display
    WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
    WriteCmd(0x3F); //
    WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
    WriteCmd(0xd3); //-set display offset
    WriteCmd(0x00); //-not offset
    WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
    WriteCmd(0xf0); //--set divide ratio
    WriteCmd(0xd9); //--set pre-charge period
    WriteCmd(0x22); //
    WriteCmd(0xda); //--set com pins hardware configuration
    WriteCmd(0x12);
    WriteCmd(0xdb); //--set vcomh
    WriteCmd(0x20); //0x20,0.77xVcc
    WriteCmd(0x8d); //--set DC-DC enable
    WriteCmd(0x14); //
    WriteCmd(0xaf); //--turn on oled panel

    OLED_CLS();

    return 0;
}

/**
  * @brief  OLED_SetPos，设置光标 设置起始点坐标
  * @param  x,光标x位置
  *         y,光标y位置
  * @retval 无
  */
void OLED_SetPos(unsigned char x, unsigned char y)
{
    WriteCmd(0xb0 + y);
    WriteCmd(((x & 0xf0) >> 4) | 0x10);
    WriteCmd((x & 0x0f) | 0x01);
}

/**
  * @brief  OLED_Fill，填充整个屏幕 全屏填充
  * @param  fill_Data:要填充的数据
  *            @arg OLED_DISPLAY_FILL: 点亮所有
  *            @arg OLED_DISPLAY_CLEAR: 清屏
  * @retval 无
  */
void OLED_Fill(unsigned char fill_Data)
{
    unsigned char m, n;
    for(m = 0; m < 8; m++)
    {
        WriteCmd(0xb0 + m); //page0-page1
        WriteCmd(0x00); //low column start address
        WriteCmd(0x10); //high column start address
        for(n = 0; n < 128; n++)
        {
            WriteData(fill_Data);
        }
    }
}

/**
  * @brief  OLED_Fill，填充部分屏幕
  * @param  fill_Data:要填充的数据
  *            @arg OLED_DISPLAY_FILL_WHILE: 点亮所有
  *            @arg OLED_DISPLAY_FILL_BLACK: 熄灭所有
  * @param  x1 [0, 128]
  * @param  x2 [0, 128]
  * @param  y1 [0, 8]
  * @param  y2 [0, 8]
  * @retval 无
  */
void OLED_Fill_Part(unsigned char fill_Data, unsigned char x1, unsigned char x2, unsigned char y1, unsigned char y2)
{
    unsigned char m, n, x_end, y_end;

    for(m = y1; m < y2; m++)
    {
        WriteCmd(0xb0 + m); //page0-page1
        WriteCmd((x1 & 0x0f));
        WriteCmd(((x1 & 0xf0) >> 4) | 0x10);
        for(n = x1; n < x2; n++)
        {
            WriteData(fill_Data);
        }
    }
}

/**
  * @brief  OLED_CLS，清屏
  * @param  无
  * @retval 无
  */
void OLED_CLS(void)
{
    OLED_Fill(0x00);
}

/**
  * @brief  OLED_ON，将OLED从休眠中唤醒
  * @param  无
  * @retval 无
  */
void OLED_ON(void)
{
    WriteCmd(0X8D);  //设置电荷泵
    WriteCmd(0X14);  //开启电荷泵
    WriteCmd(0XAF);  //OLED唤醒
}


/**
  * @brief  OLED_OFF，让OLED休眠 -- 休眠模式下,OLED功耗不到10uA
  * @param  无
  * @retval 无
  */
void OLED_OFF(void)
{
    WriteCmd(0X8D);  //设置电荷泵
    WriteCmd(0X10);  //关闭电荷泵
    WriteCmd(0XAE);  //OLED休眠
}

/**
  * @brief  OLED_ShowStr，显示codetab.h中的ASCII字符,有6*8和8*16可选择
  * @param  x,y : 起始点坐标(x:0~127, y:0~7);
  *         ch[] :- 要显示的字符串;
  *         TextSize : 字符大小(1:6*8 ; 2:8*16)
  *         backlight: OLED_BLACK_ON_WHITE(1)-白底黑字 OLED_WHITE_ON_BLACK(0)-黑底白字
  * @retval 无
 */
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize, unsigned char backlight)
{
    unsigned char c = 0, i = 0, j = 0;
    switch(TextSize)
    {
    case 1:
    {
        while(ch[j] != '\0')
        {
            c = ch[j] - 32;
            if(x > 126)
            {
                x = 0;
                y++;
            }
            OLED_SetPos(x, y);
            for(i = 0; i < 6; i++)
            {
                if(backlight)
                {
                    WriteData(~F6x8[c][i]);
                }
                else
                {
                    WriteData(F6x8[c][i]);
                }
            }
            x += 6;
            j++;
        }
    }
    break;
    case 2:
    {
        while(ch[j] != '\0')
        {
            c = ch[j] - 32;
            if(x > 120)
            {
                x = 0;
                y++;
            }
            OLED_SetPos(x, y);
            for(i = 0; i < 8; i++)
            {
                if(backlight)
                {
                    WriteData(~F8X16[c * 16 + i]);
                }
                else
                {
                    WriteData(F8X16[c * 16 + i]);
                }
            }
            OLED_SetPos(x, y + 1);
            for(i = 0; i < 8; i++)
            {
                if(backlight)
                {
                    WriteData(~F8X16[c * 16 + i + 8]);
                }
                else
                {
                    WriteData(F8X16[c * 16 + i + 8]);
                }
            }
            x += 8;
            j++;
        }
    }
    break;
    }
}

/**
  * @brief  OLED_ShowCN，显示codetab.h中的汉字,16*16点阵
  * @param  x,y: 起始点坐标(x:0~127, y:0~7);
  *         N:汉字在codetab.h中的索引
  *         backlight: 1-白底黑字 0-黑底白字
  * @retval 无
  */
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N, unsigned char backlight)
{
    unsigned char wm = 0;
    unsigned int  adder = 32 * N;
    OLED_SetPos(x, y);
    for(wm = 0; wm < 16; wm++)
    {
        if(backlight)
        {
            WriteData(~F16x16[adder]);
        }
        else
        {
            WriteData(F16x16[adder]);
        }
        adder += 1;
    }
    OLED_SetPos(x, y + 1);
    for(wm = 0; wm < 16; wm++)
    {
        if(backlight)
        {
            WriteData(~F16x16[adder]);
        }
        else
        {
            WriteData(F16x16[adder]);
        }
        adder += 1;
    }
}

/**
  * @brief  OLED_DrawBMP，显示BMP位图
  * @param  x0,y0 :起始点坐标(x0:0~127, y0:0~7);
  *         x1,y1 : 起点对角线(结束点)的坐标(x1:1~128,y1:1~8)
  * @retval 无
  */
void OLED_DrawBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[])
{
    unsigned int j = 0;
    unsigned char x, y;

    if(y1 % 8 == 0)
        y = y1 / 8;
    else
        y = y1 / 8 + 1;
    for(y = y0; y < y1; y++)
    {
        OLED_SetPos(x0, y);
        for(x = x0; x < x1; x++)
        {
            WriteData(BMP[j++]);
        }
    }
}
