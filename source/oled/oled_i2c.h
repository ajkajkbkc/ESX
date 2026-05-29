/*
 * Copyright (c) 2022-2023, Fexlink Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-06-23     Arrbow       first implementation
 */

#ifndef __OLED_I2C_H
#define __OLED_I2C_H

/* Includes ------------------------------------------------------------------*/



/* Private defines -----------------------------------------------------------*/
#define OLED_I2C_ADDRESS         0x78U //通过调整0R电阻,屏可以0x78和0x7A两个地址 -- 默认0x78

#define OLED_CHAR_EN_SIZE_8x8    1     //8*8英文字符
#define OLED_CHAR_EN_SIZE_8x16   2     //8*16英文字符

#define OLED_BLACK_ON_WHITE      1     //白底黑字
#define OLED_WHITE_ON_BLACK      0     //黑底白字

#define OLED_DISPLAY_FILL_BLACK  0x00  //熄灭所有
#define OLED_DISPLAY_FILL_WHILE  0xFF  //点亮所有


/* Exported types ------------------------------------------------------------*/



/* Exported constants --------------------------------------------------------*/




/* Private functions ---------------------------------------------------------*/
extern unsigned char OLED_Init(void);

extern void OLED_SetPos(unsigned char x, unsigned char y);
extern void OLED_Fill(unsigned char fill_Data);
extern void OLED_Fill_Part(unsigned char fill_Data, unsigned char x1, unsigned char x2, unsigned char y1, unsigned char y2);
extern void OLED_CLS(void);
extern void OLED_ON(void);
extern void OLED_OFF(void);
extern void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize, unsigned char backlight);
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N, unsigned char backlight);
extern void OLED_DrawBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[]);


#endif /* __OLED_I2C_H */
