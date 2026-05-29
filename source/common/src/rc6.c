/**
  ******************************************************************************
  * @file    rc6.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   RC6加解密算法,移植自https://github.com/m-chrome/RC6.git
  ******************************************************************************
  */
#include "rc6.h"

#define P32 0xB7E15163
#define Q32 0x9E3779B9
#define LG_W 5

static rc6_ctx_t s_rc6_ctx;

static unsigned char rc6_key[32] = {
    0x00, 0xE8, 0x8A, 0xB1, 0xE6, 0x9C, 0xAA, 0xE5,
    0x00, 0x85, 0xA8, 0xE5, 0xBC, 0x80, 0xE6, 0x88,
    0x00, 0xE6, 0x9C, 0xAA, 0xE5, 0x9B, 0xAD, 'k',
    0x00,  'a',  'w',  'a',  'k',  'p', 0x08, 0x12
};

uint32_t rol32(uint32_t a, uint8_t n)
{
    return (a << n) | (a >> (32 - n));
}

uint32_t ror32(uint32_t a, uint8_t n)
{
    return (a >> n) | (a << (32 - n));
}

static void rc6_ctx_key_schedule(void *key)
{
    unsigned char k;
    uint8_t i = 0, j = 0;
    uint32_t a = 0, b = 0;

    s_rc6_ctx.S[0] = P32;
    for(i = 1; i <= 2*s_rc6_ctx.r+3; ++i)
        s_rc6_ctx.S[i] = s_rc6_ctx.S[i-1] + Q32;

    i = 0;

    for(k=1; k<=3*(2*s_rc6_ctx.r+4); ++k) {
        a = s_rc6_ctx.S[i] = rol32((s_rc6_ctx.S[i] + a + b), 3);
        b = ((uint32_t*)key)[j] = rol32(((uint32_t*)key)[j] + a + b, a + b);
        i = (i+1) % (2*s_rc6_ctx.r+4);
        j = (j+1) % (KEY_LENGTH/W);
    }
}

void rc6_ctx_encrypt(void* block)
{
    uint32_t A = ((uint32_t *)block)[0];
    uint32_t B = ((uint32_t *)block)[1];
    uint32_t C = ((uint32_t *)block)[2];
    uint32_t D = ((uint32_t *)block)[3];
    uint32_t t=0, u=0, temp_reg;

    B += s_rc6_ctx.S[0];
    D += s_rc6_ctx.S[1];

    for(uint8_t i = 1; i <= s_rc6_ctx.r; ++i) {
        t = rol32((B * (2 * B + 1)), LG_W);
        u = rol32((D * (2 * D + 1)), LG_W);
        A = rol32(A ^ t, u) + s_rc6_ctx.S[2*i];
        C = rol32(C ^ u, t) + s_rc6_ctx.S[2*i+1];
        temp_reg = A;
        A = B;
        B = C;
        C = D;
        D = temp_reg;
    }
    A += s_rc6_ctx.S[2*s_rc6_ctx.r + 2];
    C += s_rc6_ctx.S[2*s_rc6_ctx.r + 3];
    ((uint32_t *)block)[0]=A;
    ((uint32_t *)block)[1]=B;
    ((uint32_t *)block)[2]=C;
    ((uint32_t *)block)[3]=D;
}

void rc6_ctx_decrypt(void *block, void *decrypt)
{
    uint32_t t=0, u=0, temp_reg;

    uint32_t A = ((uint32_t *)block)[0];
    uint32_t B = ((uint32_t *)block)[1];
    uint32_t C = ((uint32_t *)block)[2];
    uint32_t D = ((uint32_t *)block)[3];

    C = C - s_rc6_ctx.S[2*s_rc6_ctx.r + 3];
    A = A - s_rc6_ctx.S[2*s_rc6_ctx.r + 2];

    for(uint8_t i = s_rc6_ctx.r; i > 0; --i) {
        temp_reg = D;
        D = C;
        C = B;
        B = A;
        A = temp_reg;
        t = rol32((B*(2*B+1)), LG_W);
        u = rol32((D*(2*D+1)), LG_W);
        C = ror32((C-s_rc6_ctx.S[2*i+1]), t) ^ u;
        A = ror32((A-s_rc6_ctx.S[2*i]), u) ^ t;
    }

    D = D - s_rc6_ctx.S[1];
    B = B - s_rc6_ctx.S[0];

    if(!decrypt) {
        ((uint32_t *)block)[0] = A;
        ((uint32_t *)block)[1] = B;
        ((uint32_t *)block)[2] = C;
        ((uint32_t *)block)[3] = D;

    } else {
        ((uint32_t *)decrypt)[0] = A;
        ((uint32_t *)decrypt)[1] = B;
        ((uint32_t *)decrypt)[2] = C;
        ((uint32_t *)decrypt)[3] = D;
    }
}

void rc6_init(void *pKey)
{
    s_rc6_ctx.r = ROUNDS;

    rc6_key[0] = ((unsigned char*)pKey)[0];
    rc6_key[8] = ((unsigned char*)pKey)[1];
    rc6_key[16] = ((unsigned char*)pKey)[2];
    rc6_key[24] = ((unsigned char*)pKey)[3];
    rc6_ctx_key_schedule(rc6_key);
}

