/**
  ******************************************************************************
  * @file    rc6.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   RC6加解密算法,移植自https://github.com/m-chrome/RC6.git
  ******************************************************************************
  */

#ifndef __COM_RC6_H
#define __COM_RC6_H
//#include "stm32f4xx.h"
#include "FreeRTOS.h"

#define ROUNDS      20
#define KEY_LENGTH  256
#define W           32

typedef struct rc6_ctx{
    unsigned char r;
    uint32_t S[ROUNDS*2+4];
}rc6_ctx_t;

void rc6_ctx_decrypt(void *block, void *decrypt);
void rc6_ctx_encrypt(void* block);
void rc6_init(void *pKey);

#endif /*__COM_RC6_H*/

