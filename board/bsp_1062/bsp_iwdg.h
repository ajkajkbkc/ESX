/**
  ******************************************************************************
  * @file    bsp_iwdg.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   ¿´ÃÅ¹·Çý¶¯
  ******************************************************************************
  */
#ifndef __BSP_IWDG_H
#define __BSP_IWDG_H

#include "fsl_wdog.h"


#define KALYKE_WDOG_BASE WDOG1

extern void bsp_iwdg_init(void);
extern void bsp_watch_dog_enable(void);
extern uint16_t gResetReason;
extern uint16_t gWakeupSource;
//void bsp_feed_watch_dog(void);

/**
  * @brief  Î¹¹·
  * @param  None
  * @retval None
  */
static inline void bsp_feed_watch_dog(void)
{
#if 0
    ((WDOG_Type*)(KALYKE_WDOG_BASE))->WSR = WDOG_REFRESH_KEY & 0xFFFFU;
    ((WDOG_Type*)(KALYKE_WDOG_BASE))->WSR = (WDOG_REFRESH_KEY >> 16U) & 0xFFFFU;
#else
    WDOG_Refresh(KALYKE_WDOG_BASE);
#endif
}

#endif /*__BSP_IWDG_H*/  
