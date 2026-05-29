
#include <stdio.h>
#include <string.h>
#include "highspeed_int.h"
#include "plc_element.h"
#include "bsp_gpio.h"
#include "fsl_debug_console.h"
#include "plc_spd.h"
#include "kalyke_opts.h"

#if (KALYKE_HIGH_SPEED_IO == 0)
static const char *TAG = "highspeed_int";
static const bool logFlag = false;

/*******************************************************************
** function                :  X000_operation
** function description    :  X000 interrupt manage
** input                   :  NO。
** transfer other function :
** return                  :  void
*******************************************************************/

void X000_Interrupt(void)
{
    //LOGD(TAG, "Enter %s()", __func__);
    unsigned int  cmp_long_num;
    unsigned int  spd_time_rd;
    unsigned char bitValue;
    //if(X000 || g_x0flag)
    {
        //g_x0flag = 0;

        if(use_hc_attr[0].bit_use.BIT.initF)
        {
            // C236
            if(use_hc_attr[0].hcno == 0)
            {
                bitValue = plc_get_bit_element_value(SM_ELEMENT, SM236);
                if(bitValue == 1)
                {
                    //g_C_HCD32[0]--;
                    MINUS_C32_CURRENT_VALUE(HCOUNTER236);
                }
                else if(bitValue == 0)
                {
                    //g_C_HCD32[0]++;
                    PLUS_C32_CURRENT_VALUE(HCOUNTER236);
                }
                if(GET_C32_CURRENT_VALUE(HCOUNTER236) == use_hc_attr[0].out_cmp_num)
                {
                    if(bitValue == 1)
                    {
                        //g_C[236] = 0;
                        plc_set_bit_element_value(C_ELEMENT, HCOUNTER236, 0);
                    }
                    else if(bitValue == 0)
                    {
                        //g_C[236] = 1;
                        plc_set_bit_element_value(C_ELEMENT, HCOUNTER236, 1);
                    }
                }
                if(hc_cmdnum[0])
                {
                    hc01cmd_in_out(GET_C32_CURRENT_VALUE(HCOUNTER236), 0);
                }
            }
            // C242, C244
            else if((use_hc_attr[0].hcno == 6) || (use_hc_attr[0].hcno == 8))
            {
                bitValue = plc_get_bit_element_value(SM_ELEMENT, use_hc_attr[0].counterno);
                //if(g_SM[use_hc_attr[0].counterno] == 1)
                if (bitValue == 1)
                {
                    //g_C_HCD32[use_hc_attr[0].hcno]--;
                    MINUS_C32_CURRENT_VALUE(use_hc_attr[0].counterno);
                }
                else if(bitValue == 0)
                {
                    //g_C_HCD32[use_hc_attr[0].hcno]++;
                    PLUS_C32_CURRENT_VALUE(use_hc_attr[0].counterno);
                }
                if(GPIO_PinReadPadStatus(X2_GPIO, X2_GPIO_PIN))
                {
                    //g_C_HCD32[use_hc_attr[0].hcno] = 0;
                    SET_C32_CURRENT_VALUE(use_hc_attr[0].counterno, 0);
                    //g_C[use_hc_attr[0].counterno] = 0;
                    plc_set_bit_element_value(C_ELEMENT, use_hc_attr[0].counterno, 0);
                }
                //else if(g_C_HCD32[use_hc_attr[0].hcno] == use_hc_attr[0].out_cmp_num)
                else if(GET_C32_CURRENT_VALUE(use_hc_attr[0].counterno) == use_hc_attr[0].out_cmp_num)
                {
                    //if(g_SM[use_hc_attr[0].counterno] == 1)
                    if (bitValue == 1)
                    {
                        //g_C[use_hc_attr[0].counterno] = 0;
                        plc_set_bit_element_value(C_ELEMENT, use_hc_attr[0].counterno, 0);
                    }
                    else if(bitValue == 0)
                    {
                        //g_C[use_hc_attr[0].counterno] = 1;
                        plc_set_bit_element_value(C_ELEMENT, use_hc_attr[0].counterno, 1);
                    }
                }
                if(hc_cmdnum[0])
                {
                    //hc01cmd_in_out(g_C_HCD32[use_hc_attr[0].hcno], 0);
                    hc01cmd_in_out(GET_C32_CURRENT_VALUE(use_hc_attr[0].counterno), 0);
                }
            }
            else if((!catch_flag.BIT.catch_x0F) && GPIO_PinReadPadStatus(X0_GPIO, X0_GPIO_PIN))
            {
                //g_SM[90] = 1;
                plc_set_bit_element_value(SM_ELEMENT, SM90, 1);
            }
            return;
        }
    #if 1 // LiXianyu 20191217
        else if(use_hc_attr[1].bit_use.BIT.initF)
        {
            //C246
            //if(use_hc_attr[1].hcno == 10)
            if(use_hc_attr[1].counterno == HCOUNTER246)
            {
                //g_SM[246] = 0;
                plc_set_bit_element_value(SM_ELEMENT, SM246, 0);
                //g_C_HCD32[10]++;
                PLUS_C32_CURRENT_VALUE(HCOUNTER246);
                //if(g_C_HCD32[10] == use_hc_attr[1].out_cmp_num)
                if(GET_C32_CURRENT_VALUE(HCOUNTER246) == use_hc_attr[1].out_cmp_num)
                {
                    //g_C[246] = 1;
                    plc_set_bit_element_value(C_ELEMENT, HCOUNTER246, 1);
                }
                if(hc_cmdnum[1])
                {
                    //hc01cmd_in_out(g_C_HCD32[10], 1);
                    hc01cmd_in_out(GET_C32_CURRENT_VALUE(HCOUNTER246), 1);
                }
            }
            //C247, C249
            //else if((use_hc_attr[1].hcno == 11) || (use_hc_attr[1].hcno == 13))
            else if((use_hc_attr[1].counterno == HCOUNTER247) || (use_hc_attr[1].counterno == HCOUNTER249))
            {
                //g_SM[use_hc_attr[1].counterno] = 0;
                plc_set_bit_element_value(SM_ELEMENT, use_hc_attr[1].counterno, 0);
                //g_C_HCD32[use_hc_attr[1].hcno]++;
                PLUS_C32_CURRENT_VALUE(use_hc_attr[1].counterno);
                if(GPIO_PinReadPadStatus(X2_GPIO, X2_GPIO_PIN))
                {
                    //g_C_HCD32[use_hc_attr[1].hcno] = 0;
                    SET_C32_CURRENT_VALUE(use_hc_attr[1].counterno, 0);
                    //g_C[use_hc_attr[1].counterno] = 0;
                    plc_set_bit_element_value(C_ELEMENT, use_hc_attr[1].counterno, 0);
                }
                //else if(g_C_HCD32[use_hc_attr[1].hcno] == use_hc_attr[1].out_cmp_num)
                else if(GET_C32_CURRENT_VALUE(use_hc_attr[1].counterno) == use_hc_attr[1].out_cmp_num)
                {
                    //g_C[use_hc_attr[1].counterno] = 1;
                    plc_set_bit_element_value(C_ELEMENT, use_hc_attr[1].counterno, 1);
                }
                if(hc_cmdnum[1])
                {
                    //hc01cmd_in_out(g_C_HCD32[use_hc_attr[1].hcno], 1);
                    hc01cmd_in_out(GET_C32_CURRENT_VALUE(use_hc_attr[1].counterno), 1);
                }
            }
            else if((!catch_flag.BIT.catch_x0F) && GPIO_PinReadPadStatus(X0_GPIO, X0_GPIO_PIN))
            {
                //g_SM[90] = 1;
                plc_set_bit_element_value(SM_ELEMENT, SM90, 1);
            }
            //return;
        }
    #endif
    }

    if (gSPD[0].started)
    {
        gSPD[0].intCount++;
    }
#if 0 // LiXianyu 20191217
    //外部中断处理
    if(intr_flag.BIT.intr_x0F)
    {
        if((g_SM[40] == 1) && g_intrpt_en)
        {
            if(X000)
            {
                if(intr_num_save[0] == 0)
                    add_intr(0);
            }
            else
            {
                if(intr_num_save[0] == 10)
                    add_intr(10);
            }
        }
    }
#endif
    if((!catch_flag.BIT.catch_x0F) && GPIO_PinReadPadStatus(X0_GPIO, X0_GPIO_PIN))
    {
        //g_SM[90] = 1;
        plc_set_bit_element_value(SM_ELEMENT, SM90, 1);
    }
}

/*******************************************************************
** function                :  X002 operation
** function description    :  X002 interrupt manage
** input                   :  NO。
** transfer other function :
** return                  :  void
*******************************************************************/
void X002_Interrupt(void)
{
    unsigned int  cmp_long_num;
    unsigned int  spd_time_rd;

#if 0 // LiXianyu 20191220
    if(GIOFLG1 & 8)
    {
        g_x0flag = 1;
    }
#endif
    //外部高速计数器
    if(use_hc_attr[2].bit_use.BIT.initF)
    {
        unsigned char bitValue = plc_get_bit_element_value(SM_ELEMENT, SM238);
        if(bitValue == 1)      								    //判断方向继电器
        {
            //g_C_HCD32[2] -= 1;
            MINUS_C32_CURRENT_VALUE(HCOUNTER238);
        }
        else if(bitValue == 0)
        {
            //g_C_HCD32[2] += 1;
            PLUS_C32_CURRENT_VALUE(HCOUNTER238);
        }
        long counterCurValue = GET_C32_CURRENT_VALUE(HCOUNTER238);
        if(counterCurValue == use_hc_attr[2].out_cmp_num)      	    //计数触点判断
        {
            if(bitValue == 1)
            {
                //g_C[238] = 0;
                plc_set_bit_element_value(C_ELEMENT, HCOUNTER238, 0);
            }
            else if(bitValue == 0)
            {
                //g_C[238] = 1;
                plc_set_bit_element_value(C_ELEMENT, HCOUNTER238, 1);
            }
        }
        if(hc_cmdnum[2])
        {
            hccmd_in_out(counterCurValue, 2);  					       //输出处理函数
        }
        return;
    }

    if (gSPD[2].started)
    {
        gSPD[2].intCount++;
    }
#if 0 // LiXianyu 20191220
    //外部中断处理X2
    if(intr_flag.BIT.intr_x2F)
    {
        if((g_SM[42] == 1) && g_intrpt_en)
        {
            if(X002)
            {
                if(intr_num_save[2] == 2)
                    add_intr(2);
            }
            else
            {
                if(intr_num_save[2] == 12)
                    add_intr(12);
            }
        }
    }
#endif
    if((!catch_flag.BIT.catch_x2F) && (GPIO_PinReadPadStatus(X2_GPIO, X2_GPIO_PIN)))
    {
        //g_SM[92] = 1;
        plc_set_bit_element_value(SM_ELEMENT, SM92, 1);
    }
}

/*******************************************************************
** function                :  X003_operation
** function description    :  X003 interrupt manage
** input                   :  NO。
** transfer other function :
** return                  :  void
*******************************************************************/
void X003_Interrupt(void)
{
    unsigned int  cmp_long_num;
    unsigned int  spd_time_rd;

#if 0 // LiXianyu 20191220
    if(GIOFLG1 & 8)
    {
        g_x0flag = 1;
    }
#endif

    if(use_hc_attr[3].bit_use.BIT.initF)
    {
        unsigned char bitValue = plc_get_bit_element_value(SM_ELEMENT, use_hc_attr[3].counterno);
        if(bitValue == 1)
        {
            //g_C_HCD32[use_hc_attr[3].hcno]--;
            MINUS_C32_CURRENT_VALUE(use_hc_attr[3].counterno);
        }
        else if(bitValue == 0)
        {
            //g_C_HCD32[use_hc_attr[3].hcno]++;
            PLUS_C32_CURRENT_VALUE(use_hc_attr[3].counterno);
        }

        long counterCurValue = GET_C32_CURRENT_VALUE(use_hc_attr[3].counterno);
        if(use_hc_attr[3].counterno == HCOUNTER239)
        {
            if(counterCurValue == use_hc_attr[3].out_cmp_num)    //常闭触点判断
            {
                if(bitValue == 1)
                {
                    //g_C[use_hc_attr[3].counterno] = 0;
                    plc_set_bit_element_value(C_ELEMENT, use_hc_attr[3].counterno, 0);
                }
                else if(bitValue == 0)
                {
                    //g_C[use_hc_attr[3].counterno] = 1;
                    plc_set_bit_element_value(C_ELEMENT, use_hc_attr[3].counterno, 1);
                }
            }
        }
        else
        {
            if(GPIO_PinReadPadStatus(X5_GPIO, X5_GPIO_PIN))
            {
                //g_C_HCD32[use_hc_attr[3].hcno] = 0;
                SET_C32_CURRENT_VALUE(use_hc_attr[3].counterno, 0);
                counterCurValue = 0;
                //g_C[use_hc_attr[3].counterno] = 0;
                plc_set_bit_element_value(C_ELEMENT, use_hc_attr[3].counterno, 0);
            }
            else if(counterCurValue == use_hc_attr[3].out_cmp_num)    //常闭触点判断
            {
                if(bitValue == 1)
                {
                    //g_C[use_hc_attr[3].counterno] = 0;
                    plc_set_bit_element_value(C_ELEMENT, use_hc_attr[3].counterno, 0);
                }
                else if(bitValue == 0)
                {
                    //g_C[use_hc_attr[3].counterno] = 1;
                    plc_set_bit_element_value(C_ELEMENT, use_hc_attr[3].counterno, 1);
                }
            }
        }
        if(hc_cmdnum[3])
        {
            hccmd_in_out(counterCurValue, 3);  		//输出处理函数
        }
        return;
    }
    //else if(use_hc_attr[4].bit_use.BIT.initF && (use_hc_attr[4].hcno == 12))
    else if(use_hc_attr[4].bit_use.BIT.initF && (use_hc_attr[4].counterno == HCOUNTER248))
    {
        //g_SM[HCOUNTER248] = 0;
        plc_set_bit_element_value(SM_ELEMENT, SM248, 0);
        //g_C_HCD32[12]++;
        PLUS_C32_CURRENT_VALUE(HCOUNTER248);
        if(GPIO_PinReadPadStatus(X5_GPIO, X5_GPIO_PIN))
        {
            //g_C[248] = 0;
            plc_set_bit_element_value(C_ELEMENT, HCOUNTER248, 0);
            //g_C_HCD32[12] = 0;
            SET_C32_CURRENT_VALUE(HCOUNTER248, 0);
        }
        //else if(g_C_HCD32[12] == use_hc_attr[4].out_cmp_num)    //常闭触点判断
        else if(GET_C32_CURRENT_VALUE(HCOUNTER248) == use_hc_attr[4].out_cmp_num)    //常闭触点判断
        {
            //g_C[248] = 1;
            plc_set_bit_element_value(C_ELEMENT, HCOUNTER248, 1);
        }
        if(hc_cmdnum[4])
        {
            //hccmd_in_out(g_C_HCD32[use_hc_attr[4].hcno], 4);  		//输出处理函数
            hccmd_in_out(GET_C32_CURRENT_VALUE(use_hc_attr[4].counterno), 4);  		//输出处理函数
        }
        return;
    }
    //else if(use_hc_attr[4].bit_use.BIT.initF && (use_hc_attr[4].hcno == 14))
    else if(use_hc_attr[4].bit_use.BIT.initF && (use_hc_attr[4].counterno == HCOUNTER250))
    {
        //g_SM[250] = 0;
        plc_set_bit_element_value(SM_ELEMENT, SM250, 0);
        //g_C_HCD32[14]++;
        PLUS_C32_CURRENT_VALUE(HCOUNTER250);
        //if(X005)
        if (GPIO_PinReadPadStatus(X5_GPIO, X5_GPIO_PIN))
        {
            //g_C[250] = 0;
            plc_set_bit_element_value(C_ELEMENT, HCOUNTER250, 0);
            //g_C_HCD32[14] = 0;
            SET_C32_CURRENT_VALUE(HCOUNTER250, 0);
        }
        //else if(g_C_HCD32[14] == use_hc_attr[4].out_cmp_num)      	//常闭触点判断
        else if(GET_C32_CURRENT_VALUE(HCOUNTER250) == use_hc_attr[4].out_cmp_num)      	//常闭触点判断
        {
            //g_C[250] = 1;
            plc_set_bit_element_value(C_ELEMENT, HCOUNTER250, 1);
        }
        if(hc_cmdnum[4])
        {
            //hccmd_in_out(g_C_HCD32[use_hc_attr[4].hcno], 4);  		//输出处理函数
            hccmd_in_out(GET_C32_CURRENT_VALUE(use_hc_attr[4].counterno), 4);  		//输出处理函数
        }
        return;
    }

    if (gSPD[3].started)
    {
        gSPD[3].intCount++;
    }
#if 0 // LiXianyu 20191220
    //外部中断处理
    if(intr_flag.BIT.intr_x3F)
    {
        if((g_SM[43] == 1) && g_intrpt_en)
        {
            if(X003)
            {
                if(intr_num_save[3] == 3)
                    add_intr(3);
            }
            else
            {
                if(intr_num_save[3] == 13)
                    add_intr(13);
            }
        }
    }
#endif
    if((!catch_flag.BIT.catch_x3F) && GPIO_PinReadPadStatus(X3_GPIO, X3_GPIO_PIN))
    {
        //g_SM[93] = 1;
        plc_set_bit_element_value(SM_ELEMENT, SM93, 1);
    }
}

#if 0 // LiXianyu 20191216
/*******************************************************************
** function                :  X005_operation
** function description    :  X005 interrupt manage
** input                   :  NO。
** transfer other function :
** return                  :  void
*******************************************************************/
void GIOA_IN_FIVE(void)
{
    unsigned int  cmp_long_num;
    unsigned int  spd_time_rd;

    if(GIOFLG1 & 8)
    {
        g_x0flag = 1;
    }

    //外部高速计数器
    if(use_hc_attr[5].bit_use.BIT.initF)
    {
        if(g_SM[241] == 1)         					                    //判断方向继电器
            g_C_HCD32[5] -= 1;
        else if(g_SM[241] == 0)
            g_C_HCD32[5] += 1;
        if(g_C_HCD32[5] == use_hc_attr[5].out_cmp_num)                  //常闭触点判断
        {
            if(g_SM[241] == 1)
                g_C[241] = 0;
            else if(g_SM[241] == 0)
                g_C[241] = 1;
        }
        if(hc_cmdnum[5])
            hccmd_in_out(g_C_HCD32[5], 5);  		                       //输出处理函数
        return;
    }

    //SPD检测X5
    if(spd_flag.BIT.spd_x5F)
    {
        spd_time_rd = RTICNTR;
        spd_time_rd >>= 11;
        if(spd_time_save[5] == 0)
        {
            spd_time_rec[5] = spd_time_rd;
            *spd_count[5] = 0;                          					//送计时内的脉冲数
            *(spd_count[5] + 1) = 0;
            *(spd_count[5] + 2) = 0;
            spd_time_savet[5] = 0;
            return;
        }
        *(spd_count[5] + 1) += 1;
        cmp_long_num = spd_time_rd - spd_time_rec[5];
        spd_time_rec[5] = spd_time_rd;
        spd_time_savet[5] += (cmp_long_num & 0x1FFFFF);
        if(spd_time_savet[5] >= spd_time_save[5])
        {
            spd_time_rec[5] = spd_time_rd;
            spd_time_savet[5] = 0;
            *spd_count[5] = *(spd_count[5] + 1);      	              //pulse number save
            *(spd_count[5] + 1) = 0;
            *(spd_count[5] + 2) = 0;
        }
        return;
    }
    //外部中断处理
    if(intr_flag.BIT.intr_x5F)
    {
        if((g_SM[45] == 1) && g_intrpt_en)
        {
            if(X005)
            {
                if(intr_num_save[5] == 5)
                    add_intr(5);
            }
            else
            {
                if(intr_num_save[5] == 15)
                    add_intr(15);
            }
        }
    }
    //脉冲捕捉有效
    if((!catch_flag.BIT.catch_x5F) && X005)
        g_SM[95] = 1;
}
#endif

/*******************************************************************
** function                :  X001_operation
** function description    :  X001 interrupt manage
** input                   :  NO。
** transfer other function :
** return                  :  void
*******************************************************************/
void X001_Interrupt(void)
{
    unsigned int  cmp_long_num;
    unsigned int  spd_time_rd;
    unsigned char bitValue;
    long counterCurValue;

    //LOGD(TAG, "Enter %s()", __func__);
#if 0 // LiXianyu 20191216
    if(GIOFLG1 & 8)
    {
        g_x0flag = 1;
    }
#endif
    //C237的处理
    if(use_hc_attr[1].bit_use.BIT.initF)
    {
        // C237
        if(use_hc_attr[1].hcno == 1)
        {
            bitValue = plc_get_bit_element_value(SM_ELEMENT, SM237);
            if(bitValue)//减计数
            {
                //g_C_HCD32[1]--;
                MINUS_C32_CURRENT_VALUE(HCOUNTER237);
            }
            else
            {
                //g_C_HCD32[1]++;
                PLUS_C32_CURRENT_VALUE(HCOUNTER237);
            }
            counterCurValue = GET_C32_CURRENT_VALUE(HCOUNTER237);
            if(counterCurValue == use_hc_attr[1].out_cmp_num)      //常闭触点判断
            {
                if(bitValue)//减计数
                {
                    //g_C[237] = 0;
                    plc_set_bit_element_value(C_ELEMENT, HCOUNTER237, 0);
                }
                else
                {
                    //g_C[237] = 1;
                    plc_set_bit_element_value(C_ELEMENT, HCOUNTER237, 1);
                }
            }
            if(hc_cmdnum[1])
            {
                hc01cmd_in_out(counterCurValue, 1);
            }
            return;
        }
        //C246的处理
        //else if(use_hc_attr[1].hcno == 10)
        else if(use_hc_attr[1].counterno == HCOUNTER246)
        {
            //g_SM[246] = 1;
            plc_set_bit_element_value(SM_ELEMENT, SM246, 1);
            //g_C_HCD32[10]--;
            MINUS_C32_CURRENT_VALUE(HCOUNTER246);
            //if(g_C_HCD32[10] == use_hc_attr[1].out_cmp_num)    //常闭触点判断
            if(GET_C32_CURRENT_VALUE(HCOUNTER246) == use_hc_attr[1].out_cmp_num)
            {
                //g_C[246] = 0;
                plc_set_bit_element_value(C_ELEMENT, SM246, 0);
            }
            if(hc_cmdnum[1])
            {
                hc01cmd_in_out(GET_C32_CURRENT_VALUE(HCOUNTER246), 1);
            }
            return;
        }
        //C251的处理
        //else if(use_hc_attr[1].hcno == 15 || (use_hc_attr[1].hcno == 16) || (use_hc_attr[1].hcno == 18))
        else if(use_hc_attr[1].counterno == HCOUNTER251 || (use_hc_attr[1].hcno == HCOUNTER252) || (use_hc_attr[1].hcno == HCOUNTER254))
        {
            if(GPIO_PinReadPadStatus(X0_GPIO, X0_GPIO_PIN))
            {
                if(GPIO_PinReadPadStatus(X1_GPIO, X1_GPIO_PIN))
                {
                    //g_SM[use_hc_attr[1].counterno] = 0;
                    plc_set_bit_element_value(SM_ELEMENT, use_hc_attr[1].counterno, 0);
                    //g_C_HCD32[use_hc_attr[1].hcno]++;
                    PLUS_C32_CURRENT_VALUE(use_hc_attr[1].counterno);
                    //if(g_C_HCD32[use_hc_attr[1].hcno] == use_hc_attr[1].out_cmp_num)    //常闭触点判断
                    if(GET_C32_CURRENT_VALUE(use_hc_attr[1].counterno) == use_hc_attr[1].out_cmp_num)
                    {
                        //g_C[use_hc_attr[1].counterno] = 1;
                        plc_set_bit_element_value(C_ELEMENT, use_hc_attr[1].counterno, 1);
                    }
                }
                else
                {
                    //g_SM[use_hc_attr[1].counterno] = 1;
                    plc_set_bit_element_value(SM_ELEMENT, use_hc_attr[1].counterno, 1);
                    //g_C_HCD32[use_hc_attr[1].hcno]--;
                    MINUS_C32_CURRENT_VALUE(use_hc_attr[1].counterno);
                    //if(g_C_HCD32[use_hc_attr[1].hcno] == use_hc_attr[1].out_cmp_num)    //常闭触点判断
                    if(GET_C32_CURRENT_VALUE(use_hc_attr[1].counterno) == use_hc_attr[1].out_cmp_num)
                    {
                        //g_C[use_hc_attr[1].counterno] = 0;
                        plc_set_bit_element_value(C_ELEMENT, use_hc_attr[1].counterno, 0);
                    }
                }
            }
        #if 0 // LiXianyu 20191223 好象和四倍频有关
            if((g_SM[F_X0] == 1) && (!GPIO_PinReadPadStatus(X0_GPIO, X0_GPIO_PIN)))
            {
                if(GPIO_PinReadPadStatus(X1_GPIO, X1_GPIO_PIN))
                {
                    //g_SM[use_hc_attr[1].counterno] = 1;
                    plc_set_bit_element_value(SM_ELEMENT, use_hc_attr[1].counterno, 1);
                    //g_C_HCD32[use_hc_attr[1].hcno]--;
                    MINUS_C32_CURRENT_VALUE(use_hc_attr[1].counterno);
                    //if(g_C_HCD32[use_hc_attr[1].hcno] == use_hc_attr[1].out_cmp_num)    //常闭触点判断
                    if(GET_C32_CURRENT_VALUE(use_hc_attr[1].counterno) == use_hc_attr[1].out_cmp_num)
                    {
                        //g_C[use_hc_attr[1].counterno] = 0;
                        plc_set_bit_element_value(C_ELEMENT, use_hc_attr[1].counterno, 0);
                    }
                }
                else
                {
                    //g_SM[use_hc_attr[1].counterno] = 0;
                    plc_set_bit_element_value(SM_ELEMENT, use_hc_attr[1].counterno, 0);
                    //g_C_HCD32[use_hc_attr[1].hcno]++;
                    PLUS_C32_CURRENT_VALUE(use_hc_attr[1].counterno);
                    if(GET_C32_CURRENT_VALUE(use_hc_attr[1].counterno) == use_hc_attr[1].out_cmp_num)    //常闭触点判断
                    {
                        //g_C[use_hc_attr[1].counterno] = 1;
                        plc_set_bit_element_value(C_ELEMENT, use_hc_attr[1].counterno, 1);
                    }
                }
            }
        #endif
            //if((use_hc_attr[1].hcno == 16) || (use_hc_attr[1].hcno == 18))
            if((use_hc_attr[1].counterno == HCOUNTER252) || (use_hc_attr[1].counterno == HCOUNTER254))
            {
                //if(X002)
                if (GPIO_PinReadPadStatus(X2_GPIO, X2_GPIO_PIN))
                {
                    //g_C_HCD32[use_hc_attr[1].hcno] = 0;
                    SET_C32_CURRENT_VALUE(use_hc_attr[1].counterno, 0);
                    //g_C[use_hc_attr[1].counterno] = 0;
                    plc_set_bit_element_value(C_ELEMENT, use_hc_attr[1].counterno, 0);
                }
            }

            if(hc_cmdnum[1])
            {
                //hc01cmd_in_out(g_C_HCD32[use_hc_attr[1].hcno], 1);
                hc01cmd_in_out(GET_C32_CURRENT_VALUE(use_hc_attr[1].counterno), 1);
            }
            return;
        }
        //247和C249的处理
        else if((use_hc_attr[1].hcno == 11) || (use_hc_attr[1].hcno == 13))
        {
            //g_SM[use_hc_attr[1].counterno] = 1;
            plc_set_bit_element_value(SM_ELEMENT, use_hc_attr[1].counterno, 1);
            //g_C_HCD32[use_hc_attr[1].hcno]--;
            MINUS_C32_CURRENT_VALUE(use_hc_attr[1].counterno);
            //if(X002)
            if (GPIO_PinReadPadStatus(X2_GPIO, X2_GPIO_PIN))
            {
                //g_C_HCD32[use_hc_attr[1].hcno] = 0;
                SET_C32_CURRENT_VALUE(use_hc_attr[1].counterno, 0);
                //g_C[use_hc_attr[1].counterno] = 0;
                plc_set_bit_element_value(C_ELEMENT, use_hc_attr[1].counterno, 0);
            }
            //else if(g_C_HCD32[use_hc_attr[1].hcno] == use_hc_attr[1].out_cmp_num)    //常闭触点判断
            else if(GET_C32_CURRENT_VALUE(use_hc_attr[1].counterno) == use_hc_attr[1].out_cmp_num)
            {
                //g_C[use_hc_attr[1].counterno] = 0;
                plc_set_bit_element_value(C_ELEMENT, use_hc_attr[1].counterno, 0);
            }
            if(hc_cmdnum[1])
            {
                //hc01cmd_in_out(g_C_HCD32[use_hc_attr[1].hcno], 1);
                hc01cmd_in_out(GET_C32_CURRENT_VALUE(use_hc_attr[1].counterno), 1);
            }
            return;
        }
    }

    if (gSPD[1].started)
    {
        gSPD[1].intCount++;
    }

#if 0 // LiXianyu 20191216
    //外部中断处理
    if(intr_flag.BIT.intr_x1F)
    {
        if((g_SM[41] == 1) && g_intrpt_en)
        {
            if(X001)
            {
                if(intr_num_save[1] == 1)
                    add_intr(1);
            }
            else
            {
                if(intr_num_save[1] == 11)
                    add_intr(11);
            }
        }
    }
#endif

#if 1
    if((!catch_flag.BIT.catch_x1F) && (GPIO_PinReadPadStatus(X1_GPIO, X1_GPIO_PIN)))
    {
        //g_SM[91] = 1;
        plc_set_bit_element_value(SM_ELEMENT, SM91, 1);
    }
#endif
}

#if 0 // LiXianyu 20191216
/*******************************************************************
** function                :  X004_operation
** function description    :  X004 interrupt manage
** input                   :  NO。
** transfer other function :
** return                  :  void
*******************************************************************/
void HET1_IN_FOUR(void)
{
    unsigned int  cmp_long_num;
    unsigned int  spd_time_rd;

    if(GIOFLG1 & 8)
    {
        g_x0flag = 1;
    }

    if(use_hc_attr[4].bit_use.BIT.initF)
    {
        //C240的处理
        if(use_hc_attr[4].hcno == 4)
        {
            if(g_SM[240] == 1)
                g_C_HCD32[4]--;
            else if(g_SM[240] == 0)
                g_C_HCD32[4]++;

            if(g_C_HCD32[4] == use_hc_attr[4].out_cmp_num)    //常闭触点判断
            {
                if(g_SM[240] == 1)
                    g_C[240] = 0;
                else if(g_SM[240] == 0)
                    g_C[240] = 1;
            }
            if(hc_cmdnum[4])
                hccmd_in_out(g_C_HCD32[4], 4);
            return;
        }
        //248和C250的处理
        else if((use_hc_attr[4].hcno == 12) || (use_hc_attr[4].hcno == 14))
        {
            g_SM[use_hc_attr[4].counterno] = 1;
            g_C_HCD32[use_hc_attr[4].hcno]--;
            if(X005)
            {
                g_C_HCD32[use_hc_attr[4].hcno] = 0;
                g_C[use_hc_attr[4].counterno] = 0;
            }
            else if(g_C_HCD32[use_hc_attr[4].hcno] == use_hc_attr[4].out_cmp_num)    //常闭触点判断
                g_C[use_hc_attr[4].counterno] = 0;

            if(hc_cmdnum[4])
                hccmd_in_out(g_C_HCD32[use_hc_attr[4].hcno], 4);
            return;
        }

        //253和C255的处理
        else if((use_hc_attr[4].hcno == 17) || (use_hc_attr[4].hcno == 19))
        {
            if(X003)
            {
                if(X004)
                {
                    g_SM[use_hc_attr[4].counterno] = 0;
                    g_C_HCD32[use_hc_attr[4].hcno]++;
                    if(X005)
                    {
                        g_C_HCD32[use_hc_attr[4].hcno] = 0;
                        g_C[use_hc_attr[4].counterno] = 0;
                    }
                    else if(g_C_HCD32[use_hc_attr[4].hcno] == use_hc_attr[4].out_cmp_num)    //常闭触点判断
                        g_C[use_hc_attr[4].counterno] = 1;
                }
                else
                {
                    g_SM[use_hc_attr[4].counterno] = 1;
                    g_C_HCD32[use_hc_attr[4].hcno]--;
                    if(X005)
                    {
                        g_C_HCD32[use_hc_attr[4].hcno] = 0;
                        g_C[use_hc_attr[4].counterno] = 0;
                    }
                    else if(g_C_HCD32[use_hc_attr[4].hcno] == use_hc_attr[4].out_cmp_num)    //常闭触点判断
                        g_C[use_hc_attr[4].counterno] = 0;
                }
                if(hc_cmdnum[4])
                    hccmd_in_out(g_C_HCD32[use_hc_attr[4].hcno], 4);
            }
            return;
        }
    }
    //SPD检测X4
    if(spd_flag.BIT.spd_x4F)
    {
        spd_time_rd = RTICNTR;
        spd_time_rd >>= 11;
        if(spd_time_save[4] == 0)
        {
            spd_time_rec[4] = spd_time_rd;
            *spd_count[4] = 0;      					                        //送计时内的脉冲数
            *(spd_count[4] + 1) = 0;
            *(spd_count[4] + 2) = 0;
            spd_time_savet[4] = 0;
            return;
        }
        *(spd_count[4] + 1) += 1;
        cmp_long_num = spd_time_rd - spd_time_rec[4];
        spd_time_rec[4] = spd_time_rd;
        spd_time_savet[4] += (cmp_long_num & 0x1FFFFF);
        if(spd_time_savet[4] >= spd_time_save[4])
        {
            spd_time_rec[4] = spd_time_rd;
            spd_time_savet[4] = 0;
            *spd_count[4] = *(spd_count[4] + 1);      	              //pulse number save
            *(spd_count[4] + 1) = 0;
            *(spd_count[4] + 2) = 0;
        }
        return;
    }

    //外部中断处理
    if(intr_flag.BIT.intr_x4F)
    {
        if((g_SM[44] == 1) && g_intrpt_en)
        {
            if(X004)
            {
                if(intr_num_save[4] == 4)
                    add_intr(4);
            }
            else
            {
                if(intr_num_save[4] == 14)
                    add_intr(14);
            }
        }
    }
    if((!catch_flag.BIT.catch_x4F) && X004)
        g_SM[94] = 1;
}

/*******************************************************************
** function                :  X006_operation
** function description    :  X006 interrupt manage
** input                   :  NO。
** transfer other function :
** return                  :  void
*******************************************************************/
void HET1_IN_SIX(void)
{
    if(GIOFLG1 & 8)
    {
        g_x0flag = 1;
    }


    if(catch_flag.BIT.catch_x6F)
    {
        if(use_hc_attr[0].counterno == 244)
        {
            if(X006)
            {
                if((!use_hc_attr[0].bit_use.BIT.initF) && (use_hc_attr[0].bit_use.BIT.outhcF))
                {
                    hc_X0_init(244, 1);	 							//初始化计数器硬件
                    use_hc_attr[0].bit_use.BIT.initF = 1;
                }
            }
            else
            {
                if(use_hc_attr[0].bit_use.BIT.initF)
                {
                    //                            hc_X0_init(244,0);	 							//关闭计数器硬件
                    use_hc_attr[0].bit_use.BIT.initF = 0;
                }
            }
        }
        else if	(use_hc_attr[1].counterno == 249)
        {
            if(X006)
            {
                if((!use_hc_attr[1].bit_use.BIT.initF) && (use_hc_attr[1].bit_use.BIT.outhcF))
                {
                    hc_X0X1_init(249, 1);	 						//初始化计数器硬件
                    use_hc_attr[1].bit_use.BIT.initF = 1;
                }
            }
            else
            {
                if(use_hc_attr[1].bit_use.BIT.initF)
                {
                    //							hc_X0X1_init(249,0);	 						//关闭计数器硬件
                    use_hc_attr[1].bit_use.BIT.initF = 0;
                }
            }
        }
        else if	(use_hc_attr[1].counterno == 254)
        {
            if(X006)
            {
                if((!use_hc_attr[1].bit_use.BIT.initF) && (use_hc_attr[1].bit_use.BIT.outhcF))
                {
                    hc_AB0_init(254, 1);	 							//初始化计数器硬件
                    use_hc_attr[1].bit_use.BIT.initF = 1;
                }
            }
            else
            {
                if(use_hc_attr[1].bit_use.BIT.initF)
                {
                    //							hc_AB0_init(254,0);	 							//关闭计数器硬件
                    use_hc_attr[1].bit_use.BIT.initF = 0;
                }
            }
        }
        return;
    }
    //外部中断处理
    if(intr_flag.BIT.intr_x6F)
    {
        if((g_SM[46] == 1) && g_intrpt_en)
        {
            if(X006)
            {
                if(intr_num_save[6] == 6)
                    add_intr(6);
            }
            else
            {
                if(intr_num_save[6] == 16)
                    add_intr(16);
            }
        }
    }
    if(X006)
        g_SM[96] = 1;
}

/*******************************************************************
** function                :  X007_operation
** function description    :  X007 interrupt manage
** input                   :  NO。
** transfer other function :
** return                  :  void
*******************************************************************/
void HET1_IN_SEVEN(void)
{

    if(GIOFLG1 & 8)
    {
        g_x0flag = 1;
    }


    if(catch_flag.BIT.catch_x7F)
    {
        if(use_hc_attr[3].counterno == 245)
        {
            if(X007)
            {
                if((!use_hc_attr[3].bit_use.BIT.initF) && (use_hc_attr[3].bit_use.BIT.outhcF))
                {
                    hc_X3_init(245, 1);	 							//初始化计数器硬件
                    use_hc_attr[3].bit_use.BIT.initF = 1;
                }
            }
            else
            {
                if(use_hc_attr[3].bit_use.BIT.initF)
                {
                    //							hc_X3_init(245,0);	 							//关闭计数器硬件
                    use_hc_attr[3].bit_use.BIT.initF = 0;
                }
            }
        }
        else if	(use_hc_attr[4].counterno == 250)
        {
            if(X007)
            {
                if((!use_hc_attr[4].bit_use.BIT.initF) && (use_hc_attr[4].bit_use.BIT.outhcF))
                {
                    hc_X3X4_init(250, 1);	 						//初始化计数器硬件
                    use_hc_attr[4].bit_use.BIT.initF = 1;
                }
            }
            else
            {
                if(use_hc_attr[4].bit_use.BIT.initF)
                {
                    //							hc_X3X4_init(250,0);	 						//关闭计数器硬件
                    use_hc_attr[4].bit_use.BIT.initF = 0;
                }
            }
        }
        else if	(use_hc_attr[4].counterno == 255)
        {
            if(X007)
            {
                if((!use_hc_attr[4].bit_use.BIT.initF) && (use_hc_attr[4].bit_use.BIT.outhcF))
                {
                    hc_AB1_init(255, 1);	 							//初始化计数器硬件
                    use_hc_attr[4].bit_use.BIT.initF = 1;
                }
            }
            else
            {
                if(use_hc_attr[4].bit_use.BIT.initF)
                {
                    //							hc_AB1_init(255,0);	 							//关闭计数器硬件
                    use_hc_attr[4].bit_use.BIT.initF = 0;
                }
            }
        }
        return;
    }   //end of else

    //外部中断处理
    if(intr_flag.BIT.intr_x7F)
    {
        if((g_SM[47] == 1) && g_intrpt_en)
        {
            if(X007)
            {
                if(intr_num_save[7] == 7)
                    add_intr(7);
            }
            else
            {
                if(intr_num_save[7] == 17)
                    add_intr(17);
            }
        }
    }

    if(X007)
        g_SM[97] = 1;
}
#endif

#else
static void dummy_init(void)
{
}
#endif /* KALYKE_HIGH_SPEED_IO */
