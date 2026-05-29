/**
  ******************************************************************************
  * @file    kalyke_version.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-04-13
  * @brief   Versions
  ******************************************************************************
  */
#ifndef __KALYKE_VERSION_H
#define __KALYKE_VERSION_H

/*实际上飞凌核心板GPIO_AD_B0_09是作为LED使用的，
 *Kalyke的P1板将不再使用该脚作为X0
 */
//#define X0_AS_LED



/**
  * @brief  固件版本号，每更新一次，小版本号或临时版本号都要更新
        @arg SW_VERSION     : "a.b.c.d", (a)大版本号 (b)中版本号 (c)日期月日 (d)小版本号
        @arg SW_VERSION_ID  : (abcd), (a)大版本号 (bcd)小版本号
*/

#define SW_VERSION      "7.0.0704.00"
#define SW_VERSION_ID   (0)  //主站不需要




//黑、黄、白


#define PROGRAM_CAPACITY    128
/**
 * MiStudio收到该值后，会除以1000得到PLC版本号
 * 例如：1002 -> 1.002
 *          1 -> 0.001
 *
 * 每次提供新的OTA升级固件时，该值必须增加（每次加一即可）。
 * 每个具体客户的版本号可达10000个，从0.000至9.999
 */
#define FIRMWARE_IMAGE_ID    1U

#endif /* __KALYKE_VERSION_H */

