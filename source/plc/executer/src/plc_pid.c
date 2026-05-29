/**
  ******************************************************************************
  * @file    plc_pid.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   pid指令等
  ******************************************************************************
  */
#include <math.h>
#include <stdlib.h>
#include "plc_errormsg.h"
#include "plc_commonfunc.h"
#include "plc_variable.h"
#include "plc_instruction.h"
#include "plc_parseaddr.h"
#include "plc_element.h"
#include "plc_pid.h"
#include "plc_counterins.h"
#include "plc_timeins.h"
#include "fsl_debug_console.h"
#include "fsl_trng.h"

#define PI					3.14159265358979f

/**pid parameter define**/
#define TS       pid[0]      //sample Time 采样时间
#define Act      pid[1]      //action parameter 动作方向
#define Alf      pid[2]      //filter coefficient  输入滤波常数

#define KP       pid[3]      //proportion coefficient  比例增益

#define TI       pid[4]      //shortegral constant   积分时间
#define KD       pid[5]      //differential coefficient 微分增益
#define TD       pid[6]      //differential constant  微分时间

#define PT       pid[7]      //previous time
#define NFlg     pid[9]      //sample times flag
#define PVf      pid[10]     //previous sample value(filter)
#define PPVf     pid[11]     //before previous sample value(filter)

#define PDV      pid[12]     //previous differential value
#define PEV      pid[13]     //previous sample value error
#define PMV      pid[14]     //previous output MV

#define AVIU     pid[15]     //input value change (up) alarm set     输入变化量(增侧)报警设定值0～32767
#define AVID     pid[16]     //input value change (down) alarm set  输入变化量(减侧)报警设定值0～32767
#define AVOU     pid[17]     //output value change (up) alarm set    报警无效
#define AVOD     pid[18]     //output value change (down) alarm set 报警无效
#define AFlg     pid[19]     //alarm output set 


/**********bit define***********/
#define DRT     0
#define ALMI    1
#define ALMO    2
#define ADJ     4
#define LMT     5

#define AFIU    0
#define AFID    1
#define AFOU    2
#define AFOD    3


unsigned char run_ci_pid_ins(plc_run_power_flow_st *ltp_RunEnv)
{
    unsigned char lcv_Ret;
    unsigned char s;
    short pid[20];
    short SV,PV;
    unsigned short time;
    int L_MV, L_PV, L_TD, L_TS, L_PVf, L_PPVf, L_EV, L_DV, L_Alf, L_DltMV;
    unsigned int sys_time,L_PT;

    if(!GET_POWER_FLOW(ltp_RunEnv)) 
    {
        return pdPASS;
    }
    //s1,设定目标值
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+2, (unsigned short*)&SV, 0, 1);
    if(lcv_Ret != pdPASS) 
    {
        return lcv_Ret;
    }
    //s2,设定当前测量值
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+6, (unsigned short*)&PV, 0, 1);
    if(lcv_Ret != pdPASS) 
    {
        return lcv_Ret;
    }
    //S3,PID设置参数
    lcv_Ret = get_word(ltp_RunEnv->mcp_PC+10, (unsigned short*)pid, 0, 19);
    if(lcv_Ret != pdPASS) 
    {
        return lcv_Ret;
    }

    pid[19]=0;  //初始化报警标志
    
    //OPERAND JUDGE
    if( (TS < 1)||(Alf < 0)||(Alf > 99)||(KP < 1)||(TI < 0)||(KD < 0)||(KD > 100)||(TD < 0) )
    {
        return ERR_OPERANDS;
    }
    if( (Act&(1 << ALMI))&&(( AVIU < 0)||(AVID < 0)) )
    {
        return ERR_OPERANDS;
    }
    if( (!(Act&(1 << ALMO)))&&(Act&(1 << LMT))&&(AVOU < AVOD) )
    {
        return ERR_OPERANDS;
    }
    if( (Act&(1 << ALMO))&&(!(Act&( 1<< LMT)))&&((AVOU < 0)||(AVOD < 0)) )
    {
        return ERR_OPERANDS;
    }

    //calculate time passed
    
    sys_time = GET_1MS_TICKS_COUNT();

    L_PT   = (((unsigned short)PT)<<16)+*(unsigned short*)((&PT)+1);
    time   = (unsigned short)(sys_time-L_PT);
    L_PV   = PV;
    L_PPVf = PPVf;
    L_TS   = TS;
    L_TD   = TD;

    plc_set_bit_element_value(SM_ELEMENT, 181, 0);

    if(time >= TS)
    {
        //sampling finished
        PT = *(unsigned short*)&sys_time;
		
        *(unsigned short*)((&PT)+1) = sys_time;

        L_Alf = Alf;
        L_PVf = L_PV + L_Alf * (L_PPVf - PV) /100;

        if( (L_PVf > 32767) || (L_PVf < -32768) )
        {
            plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        }

        if(Act & ( 1 << ALMI ) )
        {
            if( (L_PVf - PPVf) > AVIU)
            {
                //sampling value up overflow
                AFlg |= ( 1 << AFIU );
                //  PVf=PVf1+AVIU;
            }
            else if((L_PPVf - L_PVf)>AVID)
            {
                //sampling value down overflow
                AFlg |= (1<<AFID);
                //  PVf=PVf1-AVIU;
            }
        }

        if ( NFlg == 0x55 )
        {
            //sample time larger than 2
            if( Act & (1 << DRT ) )
            {
                //reverse act
                L_EV = SV - L_PVf;
                L_DV = L_TD*100*(2*L_PPVf - L_PVf - PVf)/(L_TS*10+L_TD*KD)+L_TD*PDV*KD/(L_TS*10+L_TD*KD);

                L_DltMV = (((L_EV-PEV)*100+(L_TS*L_EV)/TI+L_DV*100)*KP)/10000;
                L_MV = L_DltMV + PMV;
            }
            else
            {
                //obverse act
                L_EV = L_PVf - SV;
                L_DV = L_TD*100*(-2*L_PPVf + L_PVf + PVf)/(L_TS*10+L_TD*KD)+L_TD*PDV*KD/(L_TS*10+L_TD*KD);
                L_DltMV = (((L_EV-PEV)*100+(L_TS*L_EV)/TI+L_DV*100)*KP) /10000;
                L_MV = L_DltMV + PMV;
            }
            if((Act&(1<<ALMO))&&(!(Act&(1<<LMT))))
            {
                if((L_MV-PMV) > AVOU)
                {
                    //sampling value up overflow
                    AFlg|=(1<<AFOU);
                    //MV=MV1+AVOU;
                }
                else if((PMV - L_MV) > AVOD)
                {
                    //sampling value down overflow
                    AFlg|=(1<<AFOD);
                    //MV=MV1-AVOD;
                }
            }
            else if((!(Act & ( 1 << ALMO )))&& Act &(1 << LMT))
            {
                if(L_MV > AVOU)
                {
                    L_MV = AVOU;
                }
                else if(L_MV < AVOD)
                {
                    L_MV = AVOD;
                }
            }
        }
        else
        {
            //sample time smaller than 2
            NFlg++;
            if(NFlg>2)
            {
                NFlg=0;
            }
            else if(2==NFlg)
            {
                NFlg = 0x55;
                L_EV = L_PVf - SV;
            }
            L_MV = 0;
            L_DV = 0;
            L_EV = 0;
        }

        
        if( (L_PPVf > 32767) )
        {
            L_PPVf = 32767;
            plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        }
        if( (L_PPVf < -32768) )
        {
            L_PPVf = -32768;
            plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        }
        if( (L_PVf > 32767) )
        {
            L_PVf = 32767;
            plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        }
        if( (L_PVf < -32768) )
        {
            L_PVf = -32768;
            plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        }
        if( (L_DV > 32767) )
        {
            L_DV = 32767;
            plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        }
        if( (L_DV < -32768) )
        {
            L_DV = -32768;
            plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        }
        if( (L_EV > 32767) )
        {
            L_EV = 32767;
            plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        }
        if( (L_EV < -32768) )
        {
            L_EV = -32768;
            plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        }
        if( (L_MV > 32767) )
        {
            L_MV = 32767;
            plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        }
        if( (L_MV < -32768) )
        {
            L_MV = -32768;
            plc_set_bit_element_value(SM_ELEMENT, 181, 1);
        }

        PVf  = (short)L_PPVf;
        PPVf = (short)L_PVf;
        PDV  = (short)L_DV;
        PEV  = (short)L_EV;
        PMV  = (short)L_MV;

       //参数4, 输出运算结果
       lcv_Ret = save_word(ltp_RunEnv->mcp_PC+14, (unsigned short *)&PMV, 0, 1);
       if(lcv_Ret != pdPASS) 
       {
           return lcv_Ret;
       }

       lcv_Ret = save_word(ltp_RunEnv->mcp_PC+10,(unsigned short *)&pid[7],7,8);
       if(lcv_Ret != pdPASS) 
       {
           return lcv_Ret;
       }
	   
       // pid[19] alarm output set 
       lcv_Ret = save_word(ltp_RunEnv->mcp_PC+10,(unsigned short *)&AFlg,19,1);
       if(lcv_Ret != pdPASS) 
       {
           return lcv_Ret;
       }

    }
	
    return pdPASS;
}

