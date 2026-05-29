
#include <stdio.h>
#include <string.h>
#include "plc_variable.h"
#include "plc_commonfunc.h"
#include "plc_element.h"
#include "plc_parseaddr.h"
#include "plc_highspeedins.h"
#include "plc_errormsg.h"
#include "plc_instruction.h"
#include "bsp_tim.h"
#include "fsl_debug_console.h"
#include "bsp_gpio.h"

#include "highcount_cmd.h"

static const char *TAG = "highcount_cmd";

// Kalyke supported Counter
static bool isSupportThisCounter(int counterNum)
{
    if (counterNum == HCOUNTER240 || counterNum == HCOUNTER241 ||
        counterNum == HCOUNTER253 || counterNum == HCOUNTER255 ||
        counterNum == HCOUNTER261 || counterNum == HCOUNTER263 )
    {
        return false;
    }
    return true;
}

static int handleCounterX000(int countno, int out_set_num, plc_run_power_flow_st *ltp_RunEnv)
{
    use_hc_attr[0].counterno = countno;
    use_hc_attr[0].hcno = countno - HCOUNTER236;
    if(!catch_flag.BIT.catch_x0F)
    {
        catch_flag.BIT.catch_x0F = 1;
        cancel_capture(0, 0);
    }
    if(countno == HCOUNTER244)
    {
        if(!catch_flag.BIT.catch_x2F)
        {
            catch_flag.BIT.catch_x2F = 1;
            cancel_capture(2, 0);
        }
    }
    else if(countno == HCOUNTER246)
    {
        if(!catch_flag.BIT.catch_x2F)
        {
            catch_flag.BIT.catch_x2F = 1;
            cancel_capture(2, 0);
        }
        if(!catch_flag.BIT.catch_x6F)
        {
            catch_flag.BIT.catch_x6F = 1;
            cancel_capture(6, 0);
        }
    }

    if(GET_POWER_FLOW(ltp_RunEnv)) // Instruction effective
    {
        use_hc_attr[0].out_cmp_num = out_set_num;
        if(!use_hc_attr[0].bit_use.BIT.initF)               //Judge IO initted
        {
            if(use_hc_attr[0].bit_use.BIT.outhcF)
            {
                return 1;
            }
            use_hc_attr[0].bit_use.BIT.outhcF = 1;

            if(countno == HCOUNTER246)                      //C244 specially manage
            {
                cancel_capture(6, 3);
                if (!GPIO_PinReadPadStatus(X6_GPIO, X6_GPIO_PIN)) // Judge C244' start
                {
                    return 1;
                }
            }

            hc_X0_init(countno, 1);                         //Init the hardware of high-counter;
            use_hc_attr[0].bit_use.BIT.initF = 1;
        }
        else if(countno != HCOUNTER236)                     //Dispose the high-count of X002
        {
            if(GPIO_PinReadPadStatus(X2_GPIO, X2_GPIO_PIN))
            {
                //g_C_HCD32[use_hc_attr[0].hcno] = 0;
                //g_C[use_hc_attr[0].counterno] = 0;
                SET_C32_CURRENT_VALUE(use_hc_attr[0].counterno, 0);
                plc_set_bit_element_value(C_ELEMENT, use_hc_attr[0].counterno, 0);
                return 1;
            }
        }
    }
    else
    {
        if(use_hc_attr[0].bit_use.BIT.outhcF)               //jude the use_hc_attr[0]'s counter initted?
        {
            use_hc_attr[0].bit_use.BIT.outhcF = 0;
            use_hc_attr[0].counterno = 0;
            hc_X0_init(countno, 0);                         //init the hardware of high-counter;
            use_hc_attr[0].bit_use.BIT.initF = 0;
        }
    }
    return 0;
}

static int handleCounterX001(int countno, int out_set_num, plc_run_power_flow_st *ltp_RunEnv)
{
    use_hc_attr[1].counterno = countno;
    use_hc_attr[1].hcno = countno - HCOUNTER236;
    if(!catch_flag.BIT.catch_x1F)
    {
        catch_flag.BIT.catch_x1F = 1;
        cancel_capture(1, 0);
    }
    if(GET_POWER_FLOW(ltp_RunEnv))                          //Instruction effective
    {
        use_hc_attr[1].out_cmp_num = out_set_num;
        if(!use_hc_attr[1].bit_use.BIT.initF)               //Judge hardware IO initted
        {
            if(use_hc_attr[1].bit_use.BIT.outhcF)
            {
                return 1;
            }
            use_hc_attr[1].bit_use.BIT.outhcF = 1;
            hc_X1_init(1);                                //Init the hardware of high-counter;
            use_hc_attr[1].bit_use.BIT.initF = 1;
        }
    }
    else
    {
        if(use_hc_attr[1].bit_use.BIT.outhcF)               //Judge the use_hc_attr[1]'s counter initted?
        {
            use_hc_attr[1].bit_use.BIT.outhcF = 0;
            use_hc_attr[1].counterno = 0;
            hc_X1_init(0);
            use_hc_attr[1].bit_use.BIT.initF = 0;
        }
    }
    return 0;
}

static int handleCounterX002(int countno, int out_set_num, plc_run_power_flow_st *ltp_RunEnv)
{
    use_hc_attr[2].counterno = countno;
    use_hc_attr[2].hcno = countno - HCOUNTER236;
    if(!catch_flag.BIT.catch_x2F)
    {
        catch_flag.BIT.catch_x2F = 1;
        cancel_capture(2, 0);
    }
    if(GET_POWER_FLOW(ltp_RunEnv))                          //Instruction effective
    {
        use_hc_attr[2].out_cmp_num = out_set_num;
        if(!use_hc_attr[2].bit_use.BIT.initF)               //Judge hardware IO initted
        {
            if(use_hc_attr[2].bit_use.BIT.outhcF)
            {
                return 1;
            }
            use_hc_attr[2].bit_use.BIT.outhcF = 1;
            hc_X2_init(1);                                  //Init the hardware of high-counter;
            use_hc_attr[2].bit_use.BIT.initF = 1;
        }
    }
    else
    {
        if(use_hc_attr[2].bit_use.BIT.outhcF)               //Judge the use_hc_attr[1]'s counter initted?
        {
            use_hc_attr[2].bit_use.BIT.outhcF = 0;
            use_hc_attr[2].counterno = 0;
            hc_X2_init(0);
            use_hc_attr[2].bit_use.BIT.initF = 0;
        }
    }
    return 0;
}

static int handleCounterX003(int countno, int out_set_num, plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char *usep = ltp_RunEnv->mcp_PC;
    use_hc_attr[3].counterno = countno;
    use_hc_attr[3].hcno = countno - HCOUNTER236;
    if(!catch_flag.BIT.catch_x3F)
    {
        catch_flag.BIT.catch_x3F = 1;
        cancel_capture(3, 0);
    }
    if(countno == HCOUNTER245)
    {
        if(!catch_flag.BIT.catch_x5F)
        {
            catch_flag.BIT.catch_x5F = 1;
            cancel_capture(5, 0);
        }
    }
    else if(countno == HCOUNTER247)
    {
        if(!catch_flag.BIT.catch_x5F)
        {
            catch_flag.BIT.catch_x5F = 1;
            cancel_capture(5, 0);
        }
        if(!catch_flag.BIT.catch_x7F)
        {
            catch_flag.BIT.catch_x7F = 1;
            cancel_capture(7, 0);
        }
    }
    if(GET_POWER_FLOW(ltp_RunEnv))
    {
        use_hc_attr[3].out_cmp_num = out_set_num;
        if(!use_hc_attr[3].bit_use.BIT.initF)
        {
            if(use_hc_attr[3].bit_use.BIT.outhcF)
            {
                return 1;
            }
            use_hc_attr[3].bit_use.BIT.outhcF = 1;
            if(countno == HCOUNTER247)
            {
                cancel_capture(7, 3);
                if(!GPIO_PinReadPadStatus(X7_GPIO, X7_GPIO_PIN))
                {
                    return 1;
                }
            }
            hc_X3_init(countno, 1);                         //init the hardware of high-counter;
            use_hc_attr[3].bit_use.BIT.initF = 1;
        }
        else if(countno != HCOUNTER239)
        {
            if(GPIO_PinReadPadStatus(X5_GPIO, X5_GPIO_PIN))
            {
                //g_C_HCD32[use_hc_attr[3].hcno] = 0;
                //g_C[use_hc_attr[3].counterno] = 0;
                SET_C32_CURRENT_VALUE(use_hc_attr[3].counterno, 0);
                plc_set_bit_element_value(C_ELEMENT, use_hc_attr[3].counterno, 0);
                return 1;
            }
        }
    }
    else
    {
        if(use_hc_attr[3].bit_use.BIT.outhcF)           //??????
        {
            use_hc_attr[3].bit_use.BIT.outhcF = 0;
            use_hc_attr[3].counterno = 0;
            hc_X3_init(countno, 0);                         //init the hardware of high-counter;
            use_hc_attr[3].bit_use.BIT.initF = 0;
        }
    }
    return 0;
}

static int handleCounterX004(int countno, int out_set_num, plc_run_power_flow_st *ltp_RunEnv)
{
    use_hc_attr[4].counterno = countno;
    use_hc_attr[4].hcno = countno - HCOUNTER236;
    if(!catch_flag.BIT.catch_x4F)
    {
        catch_flag.BIT.catch_x4F = 1;
        cancel_capture(4, 0);
    }
    if(GET_POWER_FLOW(ltp_RunEnv))
    {
        use_hc_attr[4].out_cmp_num = out_set_num;
        if(!use_hc_attr[4].bit_use.BIT.initF)
        {
            if(use_hc_attr[4].bit_use.BIT.outhcF)
            {
                return 1;
            }
            use_hc_attr[4].bit_use.BIT.outhcF = 1;
            hc_X4_init(1);
            use_hc_attr[4].bit_use.BIT.initF = 1;
        }
    }   //end of if((*exc_env).pf)
    else
    {
        if(use_hc_attr[4].bit_use.BIT.outhcF)
        {
            use_hc_attr[4].bit_use.BIT.outhcF = 0;
            use_hc_attr[4].counterno = 0;
            hc_X4_init(0);
            use_hc_attr[4].bit_use.BIT.initF = 0;
        }
    }
    return 0;
}

static int handleCounterX005(int countno, int out_set_num, plc_run_power_flow_st *ltp_RunEnv)
{
    use_hc_attr[5].counterno = countno;
    use_hc_attr[5].hcno = countno - HCOUNTER236;
    if(!catch_flag.BIT.catch_x5F)
    {
        catch_flag.BIT.catch_x5F = 1;
        cancel_capture(5, 0);
    }
    if(GET_POWER_FLOW(ltp_RunEnv))
    {
        use_hc_attr[5].out_cmp_num = out_set_num;
        if(!use_hc_attr[5].bit_use.BIT.initF)
        {
            if(use_hc_attr[5].bit_use.BIT.outhcF)
            {
                return 1;
            }
            use_hc_attr[5].bit_use.BIT.outhcF = 1;
            hc_X5_init(1);
            use_hc_attr[5].bit_use.BIT.initF = 1;
        }
    }
    else
    {
        if(use_hc_attr[5].bit_use.BIT.outhcF)
        {
            use_hc_attr[5].bit_use.BIT.outhcF = 0;
            use_hc_attr[5].counterno = 0;
            hc_X5_init(0);
            use_hc_attr[5].bit_use.BIT.initF = 0;
        }
    }
    return 0;
}

static int handleCounterX000X001(int countno, int out_set_num, plc_run_power_flow_st *ltp_RunEnv)
{
    use_hc_attr[1].counterno = countno;
    use_hc_attr[1].hcno = countno - HCOUNTER236;
    if(!catch_flag.BIT.catch_x0F)
    {
        catch_flag.BIT.catch_x0F = 1;
        cancel_capture(0, 0);
    }
    if(!catch_flag.BIT.catch_x1F)
    {
        catch_flag.BIT.catch_x1F = 1;
        cancel_capture(1, 0);
    }
    if(countno == HCOUNTER252)
    {
        if(!catch_flag.BIT.catch_x2F)
        {
            catch_flag.BIT.catch_x2F = 1;
            cancel_capture(2, 0);
        }
    }
    else if(countno == HCOUNTER254)
    {
        if(!catch_flag.BIT.catch_x2F)
        {
            catch_flag.BIT.catch_x2F = 1;
            cancel_capture(2, 0);
        }
        if(!catch_flag.BIT.catch_x6F)
        {
            catch_flag.BIT.catch_x6F = 1;
            cancel_capture(6, 0);
        }
    }

    if(GET_POWER_FLOW(ltp_RunEnv))
    {
        use_hc_attr[1].out_cmp_num = out_set_num;
        if(!use_hc_attr[1].bit_use.BIT.initF)
        {
            if(use_hc_attr[1].bit_use.BIT.outhcF)
            {
                return 1;
            }
            use_hc_attr[1].bit_use.BIT.outhcF = 1;
            if(countno == HCOUNTER254)
            {
                cancel_capture(6, 3);
                if(!GPIO_PinReadPadStatus(X6_GPIO, X6_GPIO_PIN))
                {
                    return 1;
                }
            }
            hc_X0X1_init(countno, 1);
            use_hc_attr[1].bit_use.BIT.initF = 1;
        }
        else if(countno != HCOUNTER248)
        {
            if(GPIO_PinReadPadStatus(X2_GPIO, X2_GPIO_PIN))
            {
                //g_C_HCD32[use_hc_attr[1].hcno] = 0;
                //g_C[use_hc_attr[1].counterno] = 0;
                SET_C32_CURRENT_VALUE(use_hc_attr[1].counterno, 0);
                plc_set_bit_element_value(C_ELEMENT, use_hc_attr[1].counterno, 0);
                return 1;
            }
        }
    }
    else
    {
        if(use_hc_attr[1].bit_use.BIT.outhcF)
        {
            use_hc_attr[1].bit_use.BIT.outhcF = 0;
            use_hc_attr[1].counterno = 0;
            hc_X0X1_init(countno, 0);
            use_hc_attr[1].bit_use.BIT.initF = 0;
        }
    }
    return 0;
}

static int handleCounterX003X004(int countno, int out_set_num, plc_run_power_flow_st *ltp_RunEnv)
{
    use_hc_attr[4].counterno = countno;
    use_hc_attr[4].hcno = countno - HCOUNTER236;
    if(!catch_flag.BIT.catch_x3F)
    {
        catch_flag.BIT.catch_x3F = 1;
        cancel_capture(3, 0);
    }
    if(!catch_flag.BIT.catch_x4F)
    {
        catch_flag.BIT.catch_x4F = 1;
        cancel_capture(4, 0);
    }
    if(!catch_flag.BIT.catch_x5F)
    {
        catch_flag.BIT.catch_x5F = 1;
        cancel_capture(5, 0);
    }
    if(countno == HCOUNTER255)
    {
        if(!catch_flag.BIT.catch_x7F)
        {
            catch_flag.BIT.catch_x7F = 1;
            cancel_capture(7, 0);
        }
    }

    if(GET_POWER_FLOW(ltp_RunEnv))
    {
        use_hc_attr[4].out_cmp_num = out_set_num;
        if(!use_hc_attr[4].bit_use.BIT.initF)
        {
            if(use_hc_attr[4].bit_use.BIT.outhcF)
            {
                return 1;
            }
            use_hc_attr[4].bit_use.BIT.outhcF = 1;
            if(countno == HCOUNTER255)
            {
                cancel_capture(7, 3);
                if(!GPIO_PinReadPadStatus(X7_GPIO, X7_GPIO_PIN))
                {
                    return 1;
                }
            }
            hc_X3X4_init(countno, 1);
            use_hc_attr[4].bit_use.BIT.initF = 1;
        }
        else if(GPIO_PinReadPadStatus(X5_GPIO, X5_GPIO_PIN))
        {
            //g_C_HCD32[use_hc_attr[4].hcno] = 0;
            //g_C[use_hc_attr[4].counterno] = 0;
            SET_C32_CURRENT_VALUE(use_hc_attr[4].counterno, 0);
            plc_set_bit_element_value(C_ELEMENT, use_hc_attr[4].counterno, 0);
            return 1;
        }
    }
    else
    {
        if(use_hc_attr[4].bit_use.BIT.outhcF)
        {
            use_hc_attr[4].bit_use.BIT.outhcF = 0;
            //g_C[use_hc_attr[4].counterno] = 0;
            plc_set_bit_element_value(C_ELEMENT, use_hc_attr[4].counterno, 0);
            hc_X3X4_init(countno, 0);
            use_hc_attr[4].bit_use.BIT.initF = 0;
        }
    }

    return 0;
}

static int handleCounterX000X001_AB(int countno, int out_set_num, plc_run_power_flow_st *ltp_RunEnv)
{
    use_hc_attr[1].counterno = countno;
    use_hc_attr[1].hcno = countno - HCOUNTER236;
    if(!catch_flag.BIT.catch_x0F)
    {
        catch_flag.BIT.catch_x0F = 1;
        cancel_capture(0, 0);
    }
    if(!catch_flag.BIT.catch_x1F)
    {
        catch_flag.BIT.catch_x1F = 1;
        cancel_capture(1, 0);
    }
    if(countno != HCOUNTER256)
    {
        if(!catch_flag.BIT.catch_x2F)
        {
            catch_flag.BIT.catch_x2F = 1;
            cancel_capture(2, 0);
        }
    }
    if(countno == HCOUNTER262)
    {
        if(!catch_flag.BIT.catch_x6F)
        {
            catch_flag.BIT.catch_x6F = 1;
            cancel_capture(6, 0);
        }
    }

    if(GET_POWER_FLOW(ltp_RunEnv))
    {
        use_hc_attr[1].out_cmp_num = out_set_num;
        if(!use_hc_attr[1].bit_use.BIT.initF)
        {
            if(use_hc_attr[1].bit_use.BIT.outhcF)
            {
                return 1;
            }
            use_hc_attr[1].bit_use.BIT.outhcF = 1;
            if(countno == HCOUNTER262)
            {
                cancel_capture(6, 3);
                if(!GPIO_PinReadPadStatus(X6_GPIO, X6_GPIO_PIN))
                {
                    return 1;
                }
            }
            hc_AB0_init(countno, 1);
            use_hc_attr[1].bit_use.BIT.initF = 1;
        }
        else if(countno != HCOUNTER256)
        {
            if(GPIO_PinReadPadStatus(X2_GPIO, X2_GPIO_PIN))
            {
                //g_C_HCD32[use_hc_attr[1].hcno] = 0;
                //g_C[use_hc_attr[1].counterno] = 0;
                SET_C32_CURRENT_VALUE(use_hc_attr[1].counterno, 0);
                plc_set_bit_element_value(C_ELEMENT, use_hc_attr[1].counterno, 0);
                return 1;
            }
        }
    }
    else
    {
        if(use_hc_attr[1].bit_use.BIT.outhcF)
        {
            use_hc_attr[1].bit_use.BIT.outhcF = 0;
            //g_C[use_hc_attr[1].counterno] = 0;
            plc_set_bit_element_value(C_ELEMENT, use_hc_attr[1].counterno, 0);
            hc_AB0_init(countno, 0);
            use_hc_attr[1].bit_use.BIT.initF = 0;
        }
    }

    return 0;
}

static int handleCounterX003X004_AB(int countno, int out_set_num, plc_run_power_flow_st *ltp_RunEnv)
{
    use_hc_attr[4].counterno = countno;
    use_hc_attr[4].hcno = countno - HCOUNTER236;
    if(!catch_flag.BIT.catch_x3F)
    {
        catch_flag.BIT.catch_x3F = 1;
        cancel_capture(3, 0);
    }
    if(!catch_flag.BIT.catch_x4F)
    {
        catch_flag.BIT.catch_x4F = 1;
        cancel_capture(4, 0);
    }
    if(!catch_flag.BIT.catch_x5F)
    {
        catch_flag.BIT.catch_x5F = 1;
        cancel_capture(5, 0);
    }
    if(countno == HCOUNTER263)
    {
        if(!catch_flag.BIT.catch_x7F)
        {
            catch_flag.BIT.catch_x7F = 1;
            cancel_capture(7, 0);
        }
    }
    if(GET_POWER_FLOW(ltp_RunEnv))
    {
        use_hc_attr[4].out_cmp_num = out_set_num;
        if(!use_hc_attr[4].bit_use.BIT.initF)
        {
            if(use_hc_attr[4].bit_use.BIT.outhcF)
            {
                return 1;
            }
            use_hc_attr[4].bit_use.BIT.outhcF = 1;
            if(countno == HCOUNTER263)
            {
                cancel_capture(7, 3);
                if(!GPIO_PinReadPadStatus(X7_GPIO, X7_GPIO_PIN))
                {
                    return 1;
                }
            }
            hc_AB1_init(countno, 1);
            use_hc_attr[4].bit_use.BIT.initF = 1;
        }
        else if(GPIO_PinReadPadStatus(X5_GPIO, X5_GPIO_PIN))
        {
            //g_C_HCD32[use_hc_attr[4].hcno] = 0;
            //g_C[use_hc_attr[4].counterno] = 0;
            SET_C32_CURRENT_VALUE(use_hc_attr[4].counterno, 0);
            plc_set_bit_element_value(C_ELEMENT, use_hc_attr[4].counterno, 0);
            return 1;
        }
    }
    else
    {
        if(use_hc_attr[4].bit_use.BIT.outhcF)
        {
            use_hc_attr[4].bit_use.BIT.outhcF = 0;
            //g_C[use_hc_attr[4].counterno] = 0;
            plc_set_bit_element_value(C_ELEMENT, use_hc_attr[4].counterno, 0);
            hc_AB1_init(countno, 0);
            use_hc_attr[4].bit_use.BIT.initF = 0;
        }
    }
    return 0;
}

/* 
  E8 F0 
  00 14 ED 00 00 00 
  02 FF 0A 00 00 00
 */
unsigned char f_CI_HCNT(plc_run_power_flow_st *ltp_RunEnv)
{
    static bool logFlag = false;
#if 0
    static uint32_t mTick = 0;
    uint32_t curTick = xTaskGetTickCount();
    if (curTick - mTick > 2000)
    {
        logFlag = true;
        mTick = curTick;
    }
    else
    {
        logFlag = false;
    }
#endif
#if 1
    if(GET_POWER_FLOW(ltp_RunEnv))
    {
        ref_hccmd_buf();
    }
#endif
    int countno, out_set_num;
    int *pCountNum;
    unsigned char *usep;
    unsigned char jude_unit;

    usep = ltp_RunEnv->mcp_PC;
    if (*(usep + 3) != 0x14) //counter type judge
    {
        if (logFlag) LOGE(TAG, "This is not highspeed counter!");
        return ERR_ELEMENT_TYPE;
    }
    pCountNum = (int *)(usep + 4);
    countno = *(pCountNum);
    if (isSupportThisCounter(countno) == false)
    {
        if (logFlag) LOGE(TAG, "ERR_OPERANDS: This product does not support this counter: C%u", countno);
        return ERR_OPERANDS;
    }
    
    if (logFlag) LOGV(TAG, "countno = %u", countno);
    if (countno < HCOUNTER236 || countno > HCOUNTER263)   //counter serial number judge
    {
        if (logFlag) LOGE(TAG, "ERR_OVER_ELEMENT_RANG");
        return ERR_OVER_ELEMENT_RANG;
    }
    
    jude_unit = get_dword(usep + 8, (unsigned long *)&out_set_num, 0, 1); //get the compare number
    if (logFlag) LOGV(TAG, "out_set_num = %u", out_set_num);
    if (jude_unit != pdPASS)
    {
        return jude_unit;
    }
    switch(countno)
    {
    case HCOUNTER236:    case HCOUNTER244:    case HCOUNTER246:     //high-counters of X000 input
        if (handleCounterX000(countno, out_set_num, ltp_RunEnv) == 1)
        {
            return pdPASS;
        }
        break;

    case HCOUNTER237:                                               //high-counters of X001 input
        if (handleCounterX001(countno, out_set_num, ltp_RunEnv) == 1)
        {
            return pdPASS;
        }
        break;

    case HCOUNTER238:                                               //high-counters of X002 input
        if (handleCounterX002(countno, out_set_num, ltp_RunEnv) == 1)
        {
            return pdPASS;
        }
        break;

    case HCOUNTER239:    case HCOUNTER245:    case HCOUNTER247:     //high-counters of X003 input
        if (handleCounterX003(countno, out_set_num, ltp_RunEnv) == 1)
        {
            return pdPASS;
        }
        break;

    case HCOUNTER240:                                               //high-counters of X004 input
        if (handleCounterX004(countno, out_set_num, ltp_RunEnv) == 1)
        {
            return pdPASS;
        }
        break;

    case HCOUNTER241:                                               //high-counters of X005 input
        if (handleCounterX005(countno, out_set_num, ltp_RunEnv) == 1)
        {
            return pdPASS;
        }
        break;


    case HCOUNTER248:    case HCOUNTER252:    case HCOUNTER254: //high-counters of X000 and X001 input
        if (handleCounterX000X001(countno, out_set_num, ltp_RunEnv) == 1)
        {
            return pdPASS;
        }
        break;

    case HCOUNTER253:    case HCOUNTER255:                                         //high-counters of X003 and X004 input
        if (handleCounterX003X004(countno, out_set_num, ltp_RunEnv) == 1)
        {
            return pdPASS;
        }
        break;

    case HCOUNTER256:    case HCOUNTER260:    case HCOUNTER262:                               //high-counters of X000 and X001 input
        if (handleCounterX000X001_AB(countno, out_set_num, ltp_RunEnv) == 1)
        {
            return pdPASS;
        }
        break;

    case HCOUNTER261:    case HCOUNTER263:
        if (handleCounterX003X004_AB(countno, out_set_num, ltp_RunEnv) == 1)
        {
            return pdPASS;
        }
        break;

    default:
        break;
    }

    return pdPASS;
}

/************************************************************************
function: DHSCS+high_counters instruction;
description: fit the high_counters's number to set user's element;
input   : EXC_ENV,the struction of system execution;
output  : no;
************************************************************************/
/* DF F0 
   02 FF 64 00 00 00 
   00 14 EC 00 00 00 
   00 01 00 00 
   00 00 -> cmdno
*/
unsigned char f_CI_DHSCS(plc_run_power_flow_st *ltp_RunEnv)
{
    static bool logFlag = false;
#if 1
    static uint32_t mTick = 0;
    uint32_t curTick = xTaskGetTickCount();
    if (curTick - mTick > 2000)
    {
        logFlag = true;
        mTick = curTick;
    }
    else
    {
        logFlag = false;
    }
#endif
    if (logFlag) LOGD(TAG, "hc_cmd_copy[0].cmdno=0x%X", hc_cmd_copy[0].cmdno);
    struct cmd_copy_str dhscs_cmd;
    unsigned char *usep = ltp_RunEnv->mcp_PC;
    if(!GET_POWER_FLOW(ltp_RunEnv))
    {
        if (logFlag) LOGE(TAG, "f_CI_DHSCS: Power flow not effective, just return.");
        dhscs_cmd.cmdno = *(usep + 18); //送高速指令序号
        hc_cmd_del(&dhscs_cmd);          //删除本高速指令
        return pdPASS;
    }

    unsigned char jude_unit;
    unsigned long get_long;
    
    if(*(usep + 9) != 0x14) //Counter type judge
    {
        return ERR_ELEMENT_TYPE;
    }
    int *pCountNum = (int *)(usep + 10);
    int countno = *(pCountNum);
    if (isSupportThisCounter(countno) == false)
    {
        if (logFlag) LOGE(TAG, "f_CI_DHSCS: This product does not support this counter: C%u", countno);
        return ERR_OPERANDS;
    }
    
    if (logFlag) LOGV(TAG, "countno = %u", countno);
    if (countno < HCOUNTER236 || countno > HCOUNTER263)   //Counter serial number judge
    {
        if (logFlag) LOGE(TAG, "ERR_OVER_ELEMENT_RANG");
        return ERR_OVER_ELEMENT_RANG;
    }
    
    /* DF F0 
       02 FF 64 00 00 00 
       00 14 EC 00 00 00 
       00 01 00 00 
       00 00 
   */
    if(GET_POWER_FLOW(ltp_RunEnv))      //Instruction effective
    {
        dhscs_cmd.cmdno = *(usep + 18); //Dispose the instruction's parameter
        dhscs_cmd.hcno = countno - HCOUNTER236;
        dhscs_cmd.cmdtype = 1;
        jude_unit = get_dword(usep + 2, &get_long, 0, 1);
        if(jude_unit != pdPASS)
        {
            return jude_unit;
        }
        if (logFlag) LOGD(TAG, "get_long = %u", get_long);
        dhscs_cmd.cmptype.dhscs.cmppoint = (int)get_long;
        if(!parse_ucode_bit(&dhscs_cmd.cmptype.dhscs.optype, &dhscs_cmd.cmptype.dhscs.opnum, (usep + 15)))
        {
            return ERR_ELEMENT_TYPE;
        }
        if (logFlag) LOGD(TAG, "optype = %u, opnum = %u, cmdno = %u", dhscs_cmd.cmptype.dhscs.optype, dhscs_cmd.cmptype.dhscs.opnum, dhscs_cmd.cmdno);
        hc_cmd_save(&dhscs_cmd);
    }
    else //命令无效处理
    {
        dhscs_cmd.cmdno = *(usep + 18); //送高速指令序号
        hc_cmd_del(&dhscs_cmd);          //删除本高速指令
    }
    if (logFlag) LOGD(TAG, "sizeof(dhscs_cmd) = %u", sizeof(dhscs_cmd));
    return pdPASS;
}

/************************************************************************
function: DHSCR+high_counters instruction;
description: fit the high_counters's number to set user's element;
input   : EXC_ENV,the struction of system execution;
output  : no;
************************************************************************/
/*
  E0 F0
  02 FF 64 00 00 00
  00 14 EC 00 00 00
  00 01 01 00
  01 00
 */
unsigned char f_CI_DHSCR(plc_run_power_flow_st *ltp_RunEnv)
{
    static bool logFlag = false;
#if 1
    static uint32_t mTick = 0;
    uint32_t curTick = xTaskGetTickCount();
    if (curTick - mTick > 2000)
    {
        logFlag = true;
        mTick = curTick;
    }
    else
    {
        logFlag = false;
    }
#endif
    unsigned char *usep, jude_unit;
    struct cmd_copy_str dhscr_cmd;
    usep = ltp_RunEnv->mcp_PC;
    if(!GET_POWER_FLOW(ltp_RunEnv))
    {
        if (logFlag) LOGE(TAG, "f_CI_DHSCR: Power flow not effective, just return.");
        dhscr_cmd.cmdno = *(usep + 18); //送高速指令序号
        hc_cmd_del(&dhscr_cmd);          //删除本高速指令
        return pdPASS;
    }
    if(*(usep + 9) != 0x14)            					//判断是否对计数器进行操作
    {
        return ERR_ELEMENT_TYPE;
    }
    int *pCountNum = (int *)(usep + 10);
    int countno = *(pCountNum);
    if (isSupportThisCounter(countno) == false)
    {
        if (logFlag) LOGE(TAG, "f_CI_DHSCR: This product does not support this counter: C%u", countno);
        return ERR_OPERANDS;
    }
    if (countno < HCOUNTER236 || countno > HCOUNTER263)   //Counter serial number judge
    {
        if (logFlag) LOGE(TAG, "ERR_OVER_ELEMENT_RANG");
        return ERR_OVER_ELEMENT_RANG;
    }
    if(GET_POWER_FLOW(ltp_RunEnv))            						//命令有效处理
    {
        dhscr_cmd.cmdno = *(usep + 18);       				//处理命令参数
        dhscr_cmd.hcno = countno - HCOUNTER236;
        dhscr_cmd.cmdtype = 2;
        jude_unit = get_dword(usep + 2, (unsigned long *)&(dhscr_cmd.cmptype.dhscr.cmppoint), 0, 1); //解析长整型数据
        if(jude_unit != pdPASS)
        {
            return jude_unit;
        }
        if(!parse_ucode_bit(&dhscr_cmd.cmptype.dhscr.optype, &dhscr_cmd.cmptype.dhscr.opnum, (usep + 15)))
        {
            return ERR_ELEMENT_TYPE;
        }
        hc_cmd_save(&dhscr_cmd);
    }
    else                         						//命令无效处理
    {
        dhscr_cmd.cmdno = *(usep + 18);
        hc_cmd_del(&dhscr_cmd);       					//删除本高速指令
    }
    return pdPASS;
}

/************************************************************************
function: DHSZ+high_counters instruction;
description: fit the high_counters's number to set user's element;
input   : EXC_ENV,the struction of system execution;
output  : no;
************************************************************************/
/*
 E1 F0 
 02 FF 64 00 00 00 
 02 FF C8 00 00 00 
 00 14 EC 00 00 00 
 00 01 04 00 
 00 00 
 */
unsigned char f_CI_DHSZ(plc_run_power_flow_st *ltp_RunEnv)
{
    static bool logFlag = false;
#if 1
    static uint32_t mTick = 0;
    uint32_t curTick = xTaskGetTickCount();
    if (curTick - mTick > 2000)
    {
        logFlag = true;
        mTick = curTick;
    }
    else
    {
        logFlag = false;
    }
#endif
    unsigned char *usep, jude_unit;
    struct cmd_copy_str dhsz_cmd;
    usep = ltp_RunEnv->mcp_PC;
    if(!GET_POWER_FLOW(ltp_RunEnv))
    {
        if (logFlag) LOGE(TAG, "f_CI_DHSCR: Power flow not effective, just return.");
        dhsz_cmd.cmdno = *(usep + 24); //高速指令序号
        hc_cmd_del(&dhsz_cmd);          //删除本高速指令
        return pdPASS;
    }
    if(*(usep + 15) != 0x14)            			//判断是否对计数器进行操作
    {
        return ERR_ELEMENT_TYPE;
    }
    int *pCountNum = (int *)(usep + 16);
    int countno = *(pCountNum);
    if (isSupportThisCounter(countno) == false)
    {
        if (logFlag) LOGE(TAG, "f_CI_DHSZ: This product does not support this counter: C%u", countno);
        return ERR_OPERANDS;
    }
    if (countno < HCOUNTER236 || countno > HCOUNTER263)   //Counter serial number judge
    {
        if (logFlag) LOGE(TAG, "f_CI_DHSZ: ERR_OVER_ELEMENT_RANG");
        return ERR_OVER_ELEMENT_RANG;
    }
    if(GET_POWER_FLOW(ltp_RunEnv))            				//命令有效处理
    {
        dhsz_cmd.cmdno = *(usep + 24);       		//处理命令参数
        dhsz_cmd.hcno = countno - HCOUNTER236;
        dhsz_cmd.cmdtype = 3;
        jude_unit = get_dword(usep + 2, (unsigned long *)&(dhsz_cmd.cmptype.dhsz.cmppoint1), 0, 1); //解析长整型数据
        if(jude_unit != pdPASS)
        {
            return jude_unit;
        }
        jude_unit = get_dword(usep + 8, (unsigned long *)&(dhsz_cmd.cmptype.dhsz.cmppoint2), 0, 1); //解析长整型数据
        if(jude_unit != pdPASS)
        {
            return jude_unit;
        }
        if(dhsz_cmd.cmptype.dhsz.cmppoint1 > dhsz_cmd.cmptype.dhsz.cmppoint2)
        {
            hc_cmd_del(&dhsz_cmd);       			//删除本高速指令
            return ERR_OPERANDS;
        }
        if(!parse_ucode_bit(&dhsz_cmd.cmptype.dhsz.optype, &dhsz_cmd.cmptype.dhsz.opnum, (usep + 21)))
        {
            return ERR_ELEMENT_TYPE;
        }
        hc_cmd_save(&dhsz_cmd);
    }
    else                         				//命令无效处理
    {
        dhsz_cmd.cmdno = *(usep + 24);
        hc_cmd_del(&dhsz_cmd);       			//删除本高速指令
    }
    return pdPASS;
}
/************************************************************************
function: DHST+高速计数器的指令;
description:高速计数器的表格比较指令对Y端口输出操作;
input   : EXC_ENV结构的变量指针;
output  : no;
************************************************************************/
/* DHST		 D100 2 C236
 E2 F0 
 00 11 64 00 00 00 
 00 FF 02 00 
 00 14 EC 00 00 00 
 00 00 -> cmdno
 */
unsigned char f_CI_DHST(plc_run_power_flow_st *ltp_RunEnv)
{
    static bool logFlag = false;
#if 1
    static uint32_t mTick = 0;
    uint32_t curTick = xTaskGetTickCount();
    if (curTick - mTick > 2000)
    {
        logFlag = true;
        mTick = curTick;
    }
    else
    {
        logFlag = false;
    }
#endif
    unsigned char *usep, i;
    struct cmd_copy_str dhst_cmd;
    usep = ltp_RunEnv->mcp_PC;
    if(*(usep + 13) != 0x14)        //判断是否对计数器进行操作
    {
        return ERR_ELEMENT_TYPE;
    }
    int *pCountNum = (int *)(usep + 14);
    int countno = *(pCountNum);
    if (logFlag) LOGV(TAG, "%s: countno = %u", __func__, countno);
    if (isSupportThisCounter(countno) == false)
    {
        if (logFlag) LOGE(TAG, "f_CI_DHST: This product does not support this counter: C%u", countno);
        return ERR_OPERANDS;
    }
    if (countno < HCOUNTER236 || countno > HCOUNTER263)   //Counter serial number judge
    {
        if (logFlag) LOGE(TAG, "f_CI_DHST: ERR_OVER_ELEMENT_RANG");
        return ERR_OVER_ELEMENT_RANG;
    }
    unsigned char cmdNo = *(usep + 18);
    if (logFlag) LOGV(TAG, "cmdNo = %u", cmdNo);
    /* DHST		 D100 2 C236
     E2 F0 
     00 11 64 00 00 00 
     00 FF 02 00 
     00 14 EC 00 00 00 
     00 00 -> cmdno
    */
    if(GET_POWER_FLOW(ltp_RunEnv))            //命令有效处理
    {
        if(cmdNo == hc_cmd_table.cmdno)   //刷新参数
        {
            dhst_cmd.cmdno = hc_cmd_table.cmdno;     //处理命令参数
            dhst_cmd.hcno = countno - HCOUNTER236;
            dhst_cmd.cmdtype = 4;
            read_dhsp_dhst_num(0, &dhst_cmd);
            hc_cmd_save(&dhst_cmd);
            if (logFlag) LOGV(TAG, "opnum = %u, opdo = %u", dhst_cmd.cmptype.dhst.opnum, dhst_cmd.cmptype.dhst.opdo);
            if(dhst_cmd.cmptype.dhst.opnum >= Y_RANG)
            {
                return ERR_OPERANDS;                         //判断常数大于0并且小于129
            }
            if(dhst_cmd.cmptype.dhst.opdo > 1)
            {
                return ERR_OPERANDS;                         //判断常数大于0并且小于129
            }
        }
        else if((hc_cmd_table.cmdno == 0xFF) && (hccmdall < 6)) //设定初始位置
        {
            if(*(usep + 3) != ADDR_D)               //判断是否为D寄存器
            {
                return ERR_ELEMENT_TYPE;
            }
            #if 0 // LiXianyu 20191224
            if(*(usep + 3) != ADDR_R)               //判断是否为R寄存器
            {
                return ERR_ELEMENT_TYPE;
            }
            
            if(*(usep + 8) != 0)               //判断是否为整型常数
            {
                return ERR_ELEMENT_TYPE;
            }
            
            if(*(usep + 9) > 1)
            {
                return ERR_ELEMENT_TYPE;
            }
            #endif
            unsigned short constValue = 0;
            get_word(usep + 8, &constValue, 0, 1);
            if (logFlag) LOGI(TAG, "constValue = %u", constValue);
            if(constValue == 0 || constValue > 128)//判断常数大于0并且小于129
            {
                return ERR_OPERANDS;
            }

            /* 描述DHST或DHSP内表格结构的变量的赋值 */
            hc_cmd_table.cmdno = cmdNo;
            hc_cmd_table.pointall = constValue;
            hc_cmd_table.startaddr = *((unsigned short *)(usep + 4));
            hc_cmd_table.curpoint = 0;
            /* 高速指令的数据备份结构赋值 */
            dhst_cmd.cmdno = cmdNo;   //处理命令参数
            dhst_cmd.hcno = countno - HCOUNTER236;
            dhst_cmd.cmdtype = 4;
            read_dhsp_dhst_num(0, &dhst_cmd);
            hc_cmd_save(&dhst_cmd);
            if (logFlag) LOGD(TAG, "opnum = %u, opdo = %u", dhst_cmd.cmptype.dhst.opnum, dhst_cmd.cmptype.dhst.opdo);

            if(dhst_cmd.cmptype.dhst.opnum >= Y_RANG)
            {
                return ERR_OPERANDS;
            }
            if(dhst_cmd.cmptype.dhst.opdo > 1)
            {
                return ERR_OPERANDS;
            }
        }
    }
    else //命令无效处理
    {
        if(cmdNo == hc_cmd_table.cmdno)
        {
            dhst_cmd.cmdno = cmdNo;
            //清除描述DHST或DHSP内表格结构的变量
            usep = (unsigned char *)&hc_cmd_table;
            for(i = 0; i < sizeof(hc_cmd_table); i++)
            {
                *usep++ = 0;
            }
            //g_SM[185] = 0;
            plc_set_bit_element_value(SM_ELEMENT, SM185, 0);
            //g_SD[182] = 0;
            SET_SD_ELEMENT_VALUE(SD182, 0);
            //g_SD[183] = 0;
            SET_SD_ELEMENT_VALUE(SD183, 0);
            //g_SD[184] = 0;
            SET_SD_ELEMENT_VALUE(SD184, 0);
            hc_cmd_del(&dhst_cmd);           //删除本高速指令
        }
    }
    return pdPASS;
}

/************************************************************************
function: DHSP+high_counters instruction;
description: fit the high_counters's number to set user's element;
input   : EXC_ENV,the struction of system execution;
output  : no;
************************************************************************/
/*
 E3 F0 
 00 11 C8 00 00 00 
 00 FF 03 00 
 00 14 EC 00 00 00 
 00 00
 */
unsigned char f_CI_DHSP(plc_run_power_flow_st *ltp_RunEnv)
{
    static bool logFlag = false;
#if 1
    static uint32_t mTick = 0;
    uint32_t curTick = xTaskGetTickCount();
    if (curTick - mTick > 2000)
    {
        logFlag = true;
        mTick = curTick;
    }
    else
    {
        logFlag = false;
    }
#endif
    unsigned char *usep, i;
    struct cmd_copy_str dhsp_cmd;
    usep = ltp_RunEnv->mcp_PC;
    if(*(usep + 13) != 0x14)        //判断是否对计数器进行操作
    {
        return ERR_ELEMENT_TYPE;
    }
    int *pCountNum = (int *)(usep + 14);
    int countno = *(pCountNum);
    if (logFlag) LOGV(TAG, "%s: countno = %u", __func__, countno);
    if (isSupportThisCounter(countno) == false)
    {
        if (logFlag) LOGE(TAG, "f_CI_DHSP: This product does not support this counter: C%u", countno);
        return ERR_OPERANDS;
    }
    if (countno < HCOUNTER236 || countno > HCOUNTER263)   //Counter serial number judge
    {
        if (logFlag) LOGE(TAG, "f_CI_DHSP: ERR_OVER_ELEMENT_RANG");
        return ERR_OVER_ELEMENT_RANG;
    }
    unsigned char cmdNo = *(usep + 18);
    if(GET_POWER_FLOW(ltp_RunEnv))            //命令有效处理
    {
        if(cmdNo == hc_cmd_table.cmdno)   //刷新参数
        {
            dhsp_cmd.cmdno = hc_cmd_table.cmdno;     //处理命令参数
            dhsp_cmd.hcno = *(usep + 15) - 236;
            dhsp_cmd.cmdtype = 5;
            read_dhsp_dhst_num(1, &dhsp_cmd);
            if(hc_cmd_table.curpoint == (hc_cmd_table.pointall - 1))
            {
                if(dhsp_cmd.cmptype.dhsp.movnum == 0)
                {
                    return pdPASS;
                }
            }
            hc_cmd_save(&dhsp_cmd);
        }
        else if((hc_cmd_table.cmdno == 0xFF) && (hccmdall < 6)) //设定初始位置
        {
            if(*(usep + 3) != ADDR_D)               //判断是否为D寄存器
            {
                return ERR_ELEMENT_TYPE;
            }
            #if 0 // LiXianyu 20191224
            if(*(usep + 3) != ADDR_R)               //判断是否为R寄存器
            {
                return ERR_ELEMENT_TYPE;
            }
            if(*(usep + 8) != 0)               //判断是否为整型常数
                return ERR_ELEMENT_TYPE;
            if(*(usep + 9) > 1)
                return ERR_ELEMENT_TYPE;
            #endif
            unsigned short constValue = 0;
            get_word(usep + 8, &constValue, 0, 1);
            if(constValue == 0 || constValue > 128)//判断常数大于0并且小于129
            {
                return ERR_OPERANDS;
            }
            if (logFlag) LOGI(TAG, "constValue = %u", constValue);

            //描述DHST或DHSP内表格结构的变量的赋值
            hc_cmd_table.cmdno = cmdNo;
            hc_cmd_table.pointall = constValue;
            hc_cmd_table.startaddr = *(unsigned short *)(usep + 4);
            hc_cmd_table.curpoint = 0;
            //高速指令的数据备份结构赋值
            dhsp_cmd.cmdno = cmdNo;   //处理命令参数
            dhsp_cmd.hcno = countno - HCOUNTER236;
            dhsp_cmd.cmdtype = 5;
            read_dhsp_dhst_num(1, &dhsp_cmd);
            hc_cmd_save(&dhsp_cmd);
        }
    }
    else                         //命令无效处理
    {
        if(cmdNo == hc_cmd_table.cmdno)  //
        {
            dhsp_cmd.cmdno = cmdNo;
            //清除描述DHST或DHSP内表格结构的变量
            usep = (unsigned char *)&hc_cmd_table;
            for(i = 0; i < sizeof(hc_cmd_table); i++)
            {
                *usep++ = 0;
            }
            //g_SM[185] = 0;
            plc_set_bit_element_value(SM_ELEMENT, SM185, 0);
            //g_SD[180] = 0;
            SET_SD_ELEMENT_VALUE(SD180, 0);
            //g_SD[181] = 0;
            SET_SD_ELEMENT_VALUE(SD181, 0);
            //g_SD[182] = 0;
            SET_SD_ELEMENT_VALUE(SD182, 0);
            //g_SD[183] = 0;
            SET_SD_ELEMENT_VALUE(SD183, 0);
            //g_SD[184] = 0;
            SET_SD_ELEMENT_VALUE(SD184, 0);
            hc_cmd_del(&dhsp_cmd);           //删除本高速指令
        }
    }
    return pdPASS;
}

#if 0
/************************************************************************
function: SPD pulse test instruction;
description: test the X000－X003 input pulse;
input   : EXC_ENV structure;
output  : no;
************************************************************************/
/* SPD X0 9000 D300
  E7 F0 
  00 00 00 00 
  00 FF 28 23 
  00 11 2C 01
*/
unsigned char f_CI_SPD(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char *usep, jude_unit;
    unsigned short spd_time_unit;
    usep = ltp_RunEnv->mcp_PC;
    if(*(usep + 3) != ADDR_X)// Must be X
    {
        return ERR_ELEMENT_TYPE;
    }
    jude_unit = get_word(usep + 6, (unsigned short *)&spd_time_unit, 0, 1);
    if(jude_unit != pdPASS)
    {
        return jude_unit;
    }
    if(*(usep + 5) == 0)
    {
        if(GET_POWER_FLOW(ltp_RunEnv))
        {
            spd_time_save[0] = (unsigned short)spd_time_unit * 1000;
            if(spd_flag.BIT.spd_x0F)
            {
                spd_x0(2);                                          //count the system pulse
            }
            else
            {
                jude_unit = get_word_point(usep, 0);
                if(jude_unit)
                    return jude_unit;
                spd_count[0] = get_word_addr;
                *spd_count[0] = 0;
                *(spd_count[0] + 1) = 0;
                *(spd_count[0] + 2) = 0;
                spd_time_savet[0] = 0;
                spd_flag.BIT.spd_x0F = 1;
                spd_time_rec[0] = RTICNTR_bit.CNTR20_0;
                spd_x0(1);
            }
        }
        else
        {
            spd_flag.BIT.spd_x0F = 0;
            spd_x0(0);
        }
    }
    else if(*(usep + 5) == 1)
    {
        if(PF(exc_env))
        {
            spd_time_save[1] = (unsigned int)spd_time_unit * 1000;
            if(spd_flag.BIT.spd_x1F)
                spd_x1(2);                     //进行时间的判断
            else
            {
                jude_unit = get_word_point(usep, 0);
                if(jude_unit)
                    return jude_unit;
                spd_count[1] = get_word_addr;
                *spd_count[1] = 0;
                *(spd_count[1] + 1) = 0;
                *(spd_count[1] + 2) = 0;
                spd_time_savet[1] = 0;
                spd_flag.BIT.spd_x1F = 1;
                spd_time_rec[1] = RTICNTR_bit.CNTR20_0;
                spd_x1(1);
            }
        }
        else
        {
            spd_flag.BIT.spd_x1F = 0;
            spd_x1(0);
        }
    }
    else if(*(usep + 5) == 2)
    {
        if(PF(exc_env))
        {
            spd_time_save[2] = (unsigned int)spd_time_unit * 1000;
            if(spd_flag.BIT.spd_x2F)
                spd_x2(2);                     //进行时间的判断
            else
            {
                jude_unit = get_word_point(usep, 0);
                if(jude_unit)
                    return jude_unit;
                spd_count[2] = get_word_addr;
                *spd_count[2] = 0;
                *(spd_count[2] + 1) = 0;
                *(spd_count[2] + 2) = 0;
                spd_time_savet[2] = 0;
                spd_flag.BIT.spd_x2F = 1;
                spd_time_rec[2] = RTICNTR_bit.CNTR20_0;
                spd_x2(1);
            }
        }
        else
        {
            spd_flag.BIT.spd_x2F = 0;
            spd_x2(0);
        }
    }
    else if(*(usep + 5) == 3)
    {
        if(PF(exc_env))
        {
            spd_time_save[3] = (unsigned int)spd_time_unit * 1000;
            if(spd_flag.BIT.spd_x3F)
                spd_x3(2);                     //进行时间的判断
            else
            {
                jude_unit = get_word_point(usep, 0);
                if(jude_unit)
                    return jude_unit;
                spd_count[3] = get_word_addr;
                *spd_count[3] = 0;
                *(spd_count[3] + 1) = 0;
                *(spd_count[3] + 2) = 0;
                spd_time_savet[3] = 0;
                spd_flag.BIT.spd_x3F = 1;
                spd_time_rec[3] = RTICNTR_bit.CNTR20_0;
                spd_x3(1);
            }
        }
        else
        {
            spd_flag.BIT.spd_x3F = 0;
            spd_x3(0);
        }
    }
    else if(*(usep + 5) == 4)
    {
        if(PF(exc_env))
        {
            spd_time_save[4] = (unsigned int)spd_time_unit * 1000;
            if(spd_flag.BIT.spd_x4F)
                spd_x4(2);                     //进行时间的判断
            else
            {
                jude_unit = get_word_point(usep, 0);
                if(jude_unit)
                    return jude_unit;
                spd_count[4] = get_word_addr;
                *spd_count[4] = 0;
                *(spd_count[4] + 1) = 0;
                *(spd_count[4] + 2) = 0;
                spd_time_savet[4] = 0;
                spd_flag.BIT.spd_x4F = 1;
                spd_time_rec[4] = RTICNTR_bit.CNTR20_0;
                spd_x4(1);
            }
        }
        else
        {
            spd_flag.BIT.spd_x4F = 0;
            spd_x4(0);
        }
    }
    else if(*(usep + 5) == 5)
    {
        if(PF(exc_env))
        {
            spd_time_save[5] = (unsigned int)spd_time_unit * 1000;
            if(spd_flag.BIT.spd_x5F)
                spd_x5(2);                     //进行时间的判断
            else
            {
                jude_unit = get_word_point(usep, 0);
                if(jude_unit)
                    return jude_unit;
                spd_count[5] = get_word_addr;
                *spd_count[5] = 0;
                *(spd_count[5] + 1) = 0;
                *(spd_count[5] + 2) = 0;
                spd_time_savet[5] = 0;
                spd_flag.BIT.spd_x5F = 1;
                spd_time_rec[5] = RTICNTR_bit.CNTR20_0;
                spd_x5(1);
            }
        }
        else
        {
            spd_flag.BIT.spd_x5F = 0;
            spd_x5(0);
        }
    }
    return pdPASS;
}
#endif

#if 0
/************************************************************************
function: INTR instruction;
description:X000-X007 rise pulse and fall pulse;
input   : interrupter number;
output  : no;
************************************************************************/
unsigned char INTR(unsigned char intr_num)
{
    switch(intr_num)
    {
    case 0:
    case 10:
    {
        if(intr_flag.BIT.intr_x0F)
            return RIGHT;
        intr_flag.BIT.intr_x0F = 1;
        intr_num_save[0] = intr_num;
        cancel_capture(0, 2);
    }
    break;
    case 1:
    case 11:
    {
        if(intr_flag.BIT.intr_x1F)
            return RIGHT;
        intr_flag.BIT.intr_x1F = 1;
        intr_num_save[1] = intr_num;
        cancel_capture(1, 2);
    }
    break;
    case 2:
    case 12:
    {
        if(intr_flag.BIT.intr_x2F)
            return RIGHT;
        intr_flag.BIT.intr_x2F = 1;
        intr_num_save[2] = intr_num;
        cancel_capture(2, 2);
    }
    break;
    case 3:
    case 13:
    {
        if(intr_flag.BIT.intr_x3F)
            return RIGHT;
        intr_flag.BIT.intr_x3F = 1;
        intr_num_save[3] = intr_num;
        cancel_capture(3, 2);
    }
    break;
    case 4:
    case 14:
    {
        if(intr_flag.BIT.intr_x4F)
            return RIGHT;
        intr_flag.BIT.intr_x4F = 1;
        intr_num_save[4] = intr_num;
        cancel_capture(4, 2);
    }
    break;
    case 5:
    case 15:
    {
        if(intr_flag.BIT.intr_x5F)
            return RIGHT;
        intr_flag.BIT.intr_x5F = 1;
        intr_num_save[5] = intr_num;
        cancel_capture(5, 2);
    }
    break;
    case 6:
    case 16:
    {
        if(intr_flag.BIT.intr_x6F)
            return RIGHT;
        intr_flag.BIT.intr_x6F = 1;
        intr_num_save[6] = intr_num;
        cancel_capture(6, 2);
    }
    break;
    case 7:
    case 17:
    {
        if(intr_flag.BIT.intr_x7F)
            return RIGHT;
        intr_flag.BIT.intr_x7F = 1;
        intr_num_save[7] = intr_num;
        cancel_capture(7, 2);
    }
    break;
    default:
        break;
    }
    return RIGHT;
}
#endif

