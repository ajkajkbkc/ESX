
#include <stdio.h>
#include <string.h>
#include <intrinsic.h>
#include <float.h>
#include "iotms470r1a288.h"
#include "std_het.h"
#include "highspeed.h"
#include "pto.h"
#include "global.h"
#include "io_def.h"


//////////////////////////////////////////////////////////////////////////
/*PWM指令的处理程序*/
//////////////////////////////////////////////////////////////////////////
/************************************************************************
function: 对Y0输出的硬件初始化
description:
input   : op_mode为0初始化为IO输出；为1对应PWM硬件初始化；
output  : no;
************************************************************************/
void y0_init_pwm(unsigned char op_mode)
{
  unsigned int js_unit[2],use_unit;
  unsigned char i;
  HET_L00_0.memory.data_word = 0;                                                   //Y0 pulse  disable
  HET_L05_0.memory.control_word &= 0xFFFFFFFE;
  //低电平输出
  if(pto_str[0].cmd_type.pwm.out_breadth == 0)
  {
    if(HETDOUT & 0x00000001)
      HETDOUT ^= 0x00000001;
    g_Y[0] = 0;
    HET_L02_0.memory.program_word &= 0xFFFFFFFE;
    HET_L03_0.memory.control_word &= 0xFFFFFFFE;
  }
  //高电平输出
  else if(pto_str[0].cmd_type.pwm.out_breadth == pto_str[0].cmd_type.pwm.out_freq)
  {
    if((HETDOUT & 0x00000001)==0)
      HETDOUT ^= 0x00000001;
    g_Y[0] = 1;
    HET_L02_0.memory.program_word &= 0xFFFFFFFE;
    HET_L03_0.memory.control_word &= 0xFFFFFFFE;
  }

  else
  {
    js_unit[0] = pto_str[0].cmd_type.pwm.out_freq;
    js_unit[0] = js_unit[0]*3000;//总宽计数
    js_unit[1] = pto_str[0].cmd_type.pwm.out_breadth;
    js_unit[1] = js_unit[1]*3000;//脉宽计数
    if(op_mode)
    {
      //for us
      if(op_mode-1)
      {
        js_unit[0]/=1000;
        js_unit[1]/=1000;
        if(js_unit[0]<10 || js_unit[1]<10 ||(js_unit[0]-js_unit[1])<10)
          return;
      }
		    //PWM模式1输出
      js_unit[1] = js_unit[1]<<5;
      HET_L02_0.memory.control_word = (js_unit[0]-1);
      HET_L03_0.memory.data_word = js_unit[1];
      HET_L02_0.memory.program_word &= 0xFFFFFFFE;
      HET_L03_0.memory.control_word &= 0xFFFFFFFE;
      HET_L03_0.memory.control_word |= 1<<20;
    }
    //普通I/O翻转输出
    else
    {
		    cyc_union[0].cmp_num = (js_unit[0]) >> 7;//5;总宽除以128
                    cyc_union[0].cyc_num = 127;//127;//31;
                    if(js_unit[1] < cyc_union[0].cmp_num )
                    {
                      pwm_cyle[0] = 0;
                      cyc_union[2].cyc_num = 0;
                      cyc_union[2].cmp_num = js_unit[1];
                    }
                    else
                    {
                      for(i=1; i<129; i++)//(i=1; i<128; i++)//(i=1; i<32; i++)
                      {
                        use_unit = cyc_union[0].cmp_num*i;
                        if(js_unit[1] < use_unit)
                        {
                          pwm_cyle[0] = i-1;
                          cyc_union[2].cyc_num = i-1;
                          cyc_union[2].cmp_num = ((js_unit[1] + cyc_union[0].cmp_num)- use_unit);
                          if((cyc_union[0].cmp_num - cyc_union[2].cmp_num) < 10)
                            cyc_union[2].cmp_num = (cyc_union[0].cmp_num-10);
                          break;
                        }
                        else if(js_unit[1] == use_unit)
                        {
                          pwm_cyle[0] = i;
                          cyc_union[2].cyc_num = i;
                          cyc_union[2].cmp_num = 10;
                          break;
                        }
                      }
                    }
                    cyc_union[2].cmp_num <<= 5;//7;//5;
                    if((HETDOUT & 0x00000001)==0)
                      HETDOUT ^= 0x00000001;
                    HET_L02_0.memory.control_word = cyc_union[0].cmp_num-1;
                    HET_L03_0.memory.data_word = cyc_union[2].cmp_num;
                    HET_L03_0.memory.control_word &= 0xFFEFFFFF;                                   //IO output disable
                    HET_L02_0.memory.program_word |= 1;
                    HET_L03_0.memory.control_word |= 1;

    }
    HET_L00_0.memory.data_word = 0x10<<5;                                       //Y0 pulse output enable
  }
}

/************************************************************************
function: 对Y1输出的硬件初始化
description:
input   : op_mode为0初始化为IO输出；为1对应PWM硬件初始化；
output  : no;
************************************************************************/
void y1_init_pwm(unsigned char op_mode)
{
  unsigned int js_unit[2],use_unit;
  unsigned char i;
  HET_L06_0.memory.data_word = 0;                                                     //Y1 pulse  disable
  HET_L11_0.memory.control_word &= 0xFFFFFFFE;
  //低电平输出
  if(pto_str[1].cmd_type.pwm.out_breadth == 0)
  {
    if(HETDOUT & 0x00000010)
      HETDOUT ^= 0x00000010;
    g_Y[1] = 0;
    HET_L08_0.memory.program_word &= 0xFFFFFFFE;
    HET_L09_0.memory.control_word &= 0xFFFFFFFE;
  }
  //高电平输出
  else if(pto_str[1].cmd_type.pwm.out_breadth==pto_str[1].cmd_type.pwm.out_freq)
  {
    if((HETDOUT & 0x00000010) == 0)
      HETDOUT ^= 0x00000010;
    g_Y[1] = 1;
    HET_L08_0.memory.program_word &= 0xFFFFFFFE;
    HET_L09_0.memory.control_word &= 0xFFFFFFFE;
  }

  else
  {
    js_unit[0] = pto_str[1].cmd_type.pwm.out_freq;
    js_unit[0] = js_unit[0]*3000;
    js_unit[1] = pto_str[1].cmd_type.pwm.out_breadth;
    js_unit[1] = js_unit[1]*3000;
    if(op_mode)
    {
      //for us
      if(op_mode-1)
      {
        js_unit[0]/=1000;
        js_unit[1]/=1000;
        if(js_unit[0]<10 || js_unit[1]<10 ||(js_unit[0]-js_unit[1])<10)
          return;
      }
		    //PWM模式1输出
      js_unit[1] = js_unit[1]<<5;
      HET_L08_0.memory.control_word = (js_unit[0]-1);
      HET_L09_0.memory.data_word = js_unit[1];
      HET_L08_0.memory.program_word &= 0xFFFFFFFE;
      HET_L09_0.memory.control_word &= 0xFFFFFFFE;
      HET_L09_0.memory.control_word |= 1<<20;
    }
    //普通I/O翻转输出
    else
    {
		    cyc_union[1].cmp_num = js_unit[0] >> 7;//5;
                    cyc_union[1].cyc_num = 127;//31;
                    if(js_unit[1] < cyc_union[1].cmp_num )
                    {
                      pwm_cyle[1] = 0;
                      cyc_union[3].cyc_num = 0;
                      cyc_union[3].cmp_num = js_unit[1];
                    }
                    else
                    {
                      for(i=1; i<129; i++)//(i=1; i<32; i++)
                      {
                        use_unit = cyc_union[1].cmp_num*i;
                        if(js_unit[1] < use_unit)
                        {
                          pwm_cyle[1] = i-1;
                          cyc_union[3].cyc_num = i-1;
                          cyc_union[3].cmp_num = ((js_unit[1] + cyc_union[1].cmp_num)- use_unit);
                          if((cyc_union[1].cmp_num - cyc_union[3].cmp_num) < 10)
                            cyc_union[3].cmp_num = (cyc_union[1].cmp_num-10);
                          break;
                        }
                        else if(js_unit[1] == use_unit)
                        {
                          pwm_cyle[1] = i;
                          cyc_union[3].cyc_num = i;
                          cyc_union[3].cmp_num = 10;
                          break;
                        }
                      }
                    }
                    cyc_union[3].cmp_num <<= 5;//7;//5;
                    if((HETDOUT & 0x00000010) == 0)
                      HETDOUT ^= 0x00000010;
                    HET_L08_0.memory.control_word = cyc_union[1].cmp_num-1;
                    HET_L09_0.memory.data_word = cyc_union[3].cmp_num;
                    HET_L09_0.memory.control_word &= 0xFFEFFFFF;                                   //IO output disable
                    HET_L08_0.memory.program_word |= 1;
                    HET_L09_0.memory.control_word |= 1;
    }
    HET_L06_0.memory.data_word = 0x10<<5;                                       //Y1 pulse output enable
  }
}

/************************************************************************
function: 对Y0输出的硬件初始化
description:
input   : op_mode为0对应硬件Y0为普通设定；为1对应硬件的初始化；为2对应输出
脉冲频率的设定;为3脉冲数据的计算
output  : no;
************************************************************************/
void pwm_init_y0(unsigned char op_mode)
{
  unsigned char i,*usep;
  //对Y0的脉冲输出初始化
  if (op_mode == 1)
  {
    pto_str[0].op_type = 2;                       //为PWM模式
    HET_L02_0.memory.data_word = 0;
    if(pto_str[0].cmd_type.pwm.out_freq > 349 && g_SM[84]==0)    //输入频率大于1048MS
    {
      pto_str[0].out_mode = 2;                  //设为IO输出方式
      y0_init_pwm(0);		
    }
    else if(g_SM[84]==1)
    {
      pto_str[0].out_mode = 1;
      y0_init_pwm(2);		
    }
    else
    {
      pto_str[0].out_mode = 1;                  //自动翻转输出方式
      y0_init_pwm(1);		
    }
  }
  else
  {
    //停止Y0的脉冲输出
    if(op_mode==0)
    {
      HET_L00_0.memory.data_word = 0;                                               //Y0 pulse  disable
      HET_L02_0.memory.data_word = 0;
      HET_L02_0.memory.program_word &= 0xFFFFFFFE;
      HET_L03_0.memory.control_word &= 0xFFEFFFFE;
      HET_L05_0.memory.control_word &= 0xFFFFFFFE;
      if(HETDOUT & 0x00000001)
        HETDOUT ^= 0x00000001;
      g_Y[0] = 0;
      //清除对应的属性变量
      usep=(unsigned char *)&pto_str[0];
      for(i=0; i<sizeof(pto_str[0]);i++)
        *usep++=0;
    }
    //频率有变化的处理
    else if(op_mode==2)
    {
      if(pto_str[0].cmd_type.pwm.out_freq > 349 && g_SM[84]==0)    //大于1048MS
      {
        pto_str[0].out_mode = 2;
        y0_init_pwm(0);		
      }
      else if(g_SM[84]==1)
      {
        pto_str[0].out_mode = 1;
        y0_init_pwm(2);		
      }
      else
      {
        pto_str[0].out_mode = 1;
        y0_init_pwm(1);		
      }
    }  //end of else if(op_mode==2)
  }
}

/************************************************************************
function: 对Y1输出的硬件初始化
description:
input   : op_mode为0对应硬件Y1为普通设定；为1对应硬件的初始化；为2对应输出
脉冲频率的设定;为3脉冲数据的计算
output  : no;
************************************************************************/
void pwm_init_y1(unsigned char op_mode)
{
  unsigned char i,*usep;
  //对Y0的脉冲输出初始化
  if (op_mode==1)
  {
    pto_str[1].op_type = 2;                       //为PWM模式
    HET_L08_0.memory.data_word = 0;
    if(pto_str[1].cmd_type.pwm.out_freq > 349 && g_SM[84]==0)    //频率大于1048MS
    {
      pto_str[1].out_mode = 2;                  //设为IO输出方式
      y1_init_pwm(0);		
    }
    else if(g_SM[84]==1)
    {
      pto_str[1].out_mode=1;
      y1_init_pwm(2);	
    }
    else
    {
      pto_str[1].out_mode=1;                  //自动翻转输出方式
      y1_init_pwm(1);		
    }
  }
  else
  {
    //停止Y1的脉冲输出
    if(op_mode==0)
    {
      HET_L06_0.memory.data_word = 0;                                               //Y0 pulse  disable
      HET_L08_0.memory.data_word = 0;
      HET_L08_0.memory.program_word &= 0xFFFFFFFE;
      HET_L09_0.memory.control_word &= 0xFFEFFFFE;
      HET_L11_0.memory.control_word &= 0xFFFFFFFE;
      if(HETDOUT & 0x00000010)
        HETDOUT ^= 0x00000010;
      g_Y[1] = 0;
      usep=(unsigned char *)&pto_str[1];
      for(i=0;i<sizeof(pto_str[1]);i++)
        *usep++=0;
    }
    //刷新输入的频率
    else if(op_mode==2)
    {
      if(pto_str[1].cmd_type.pwm.out_freq > 349 && g_SM[84]==0)    //大于1048MS
      {
        pto_str[1].out_mode=2;
        y1_init_pwm(0);		
      }
      else if(g_SM[84]==1)
      {
        pto_str[1].out_mode=1;
        y1_init_pwm(2);	
      }
      else
      {
        pto_str[1].out_mode=1;
        y1_init_pwm(1);		
      }
    }  //end of else if(op_mode==2)
  }
}

/************************************************************************
function: PWM脉冲输出指令;
description:对Y端口输出操作;
input   : EXC_ENV结构的变量指针;
output  : no;
************************************************************************/
unsigned char f_CI_PWM(EXC_ENV *exc_env)
{
  unsigned char *usep,jude_unit;
  unsigned short  use_code[2];
  usep=(*exc_env).pc_ucode;
  if(*(usep+10)!=2)                                   //判断Y0和Y1有效吗？
    return ERR_ELEM_TYPE;
  if(*(usep+13)>1)
    return ERR_ELEM_RANG;



  //能流和特殊辅助寄存器有效的处理
  if(PF(exc_env) && (g_SM[*(usep+13)+80]==0))
  {
    jude_unit=get_word(usep+2,&use_code[0],0,1);    //取幅值数据
    if(jude_unit)
    {
      if(pto_str[*(usep+13)].op_type==2)
      {
        if(pto_str[*(usep+13)].ucode_pc!=(unsigned int)usep)
          return jude_unit;
        //置对应的监控位为0
        g_SM[*(usep+13)+82]=0;
        if(*(usep+13))
          pwm_init_y1(0);
        else
          pwm_init_y0(0);
      }
      return jude_unit;
    }
    jude_unit=get_word(usep+6,&use_code[1],0,1);    //取周期数据
    if(jude_unit)
    {
      if(pto_str[*(usep+13)].op_type==2)
      {
        if(pto_str[*(usep+13)].ucode_pc!=(unsigned int)usep)
          return jude_unit;
        //置对应的监控位为0
        g_SM[*(usep+13)+82]=0;
        if(*(usep+13))
          pwm_init_y1(0);
        else
          pwm_init_y0(0);
      }
      return jude_unit;
    }
    //参数区间判断
    if((use_code[1]<use_code[0])||(use_code[0]>32767)||(use_code[1]>32767)||(use_code[1]==0)
       ||(g_SM[84]==1&&(use_code[0]<10||use_code[1]-use_code[0]<10)))
    {
      if(pto_str[*(usep+13)].op_type==2)
      {
        if(pto_str[*(usep+13)].ucode_pc!=(unsigned int)usep)
          return ERR_OPERAND_VAL;
        //置对应的监控位为0
        g_SM[*(usep+13)+82]=0;
        if(*(usep+13))
          pwm_init_y1(0);
        else
          pwm_init_y0(0);
      }
      return ERR_OPERAND_VAL;
    }
    //判断指令是否在执行
    if(pto_str[*(usep+13)].op_type==2)
    {
      if(pto_str[*(usep+13)].ucode_pc!=(unsigned int)usep)
        return RIGHT;       //保存指令在用户程序中的位置
      //判断频率是否改变
      if((use_code[0]!=pto_str[*(usep+13)].cmd_type.pwm.out_breadth)
         ||(use_code[1]!=pto_str[*(usep+13)].cmd_type.pwm.out_freq)
           ||pto_str[*(usep+13)].cmd_type.pwm.sm84!=g_SM[84])
      {
        //刷新用户指令的参数
        pto_str[*(usep+13)].cmd_type.pwm.out_breadth=use_code[0];
        pto_str[*(usep+13)].cmd_type.pwm.out_freq=use_code[1];
        pto_str[*(usep+13)].cmd_type.pwm.sm84=g_SM[84];
        //刷新频率的处理
        if(*(usep+13))
          pwm_init_y1(2);
        else
          pwm_init_y0(2);
      }
    }
    //判断对应的Y0或Y1没有输出指令
    else if(pto_str[*(usep+13)].op_type==0)
    {
      pto_str[*(usep+13)].cmd_type.pwm.out_breadth=use_code[0];
      pto_str[*(usep+13)].cmd_type.pwm.out_freq=use_code[1];
      pto_str[*(usep+13)].cmd_type.pwm.sm84=g_SM[84];
      pto_str[*(usep+13)].ucode_pc=(unsigned int)usep;
      //置对应的监控位为1
      g_SM[*(usep+13)+82]=1;
      //初始化处理
      if(*(usep+13))
      {
        if(HETDOUT & 0x00000010)
          HETDOUT ^= 0x00000010;
        pwm_init_y1(1);
      }
      else
      {
        if(HETDOUT & 0x00000001)
          HETDOUT ^= 0x00000001;
        pwm_init_y0(1);
      }
    }
  }
  //能流无效处理
  else
  {
    if(pto_str[*(usep+13)].op_type==2)
    {
      if(pto_str[*(usep+13)].ucode_pc!=(unsigned int)usep)
        return RIGHT;
      //置对应的监控位为0
      g_SM[*(usep+13)+82]=0;
      if(*(usep+13))
        pwm_init_y1(0);
      else
        pwm_init_y0(0);
    }
  }  //end of else
  return RIGHT;
}

//////////////////////////////////////////////////////////////////////////
/*PLSY指令和PLSR指令的处理程序*/
//////////////////////////////////////////////////////////////////////////
/************************************************************************
function: 对Y0输出的硬件初始化
description:
input   : op_mode为1初始化硬件；为2刷新输入频率；
output  : no;
************************************************************************/
void y0_init_pto(unsigned char op_mode)
{
  unsigned int use_unit[2];
  unsigned char use_flag;
  unsigned char i;
  unsigned int unit3;
  unsigned int js_unit[2];
  int microFrq=0;
  float delta=0;

  HET_L00_0.memory.data_word = 0;                                               //Y0 pulse  disable
  if((op_mode==1))
  {
    pto_str[0].out_mode = 1;
    if(pto_str[0].op_type == 1)
    {
		    pto_str[0].count_ones=1;
                    if(pto_str[0].cmd_type.plsy.out_pulse == 1 && pto_str[0].cmd_type.plsy.out_freq > 75000)
                    {
                      pto_str[0].cmd_type.plsy.out_freq = 75000;
                    }
                    if(pto_str[0].cmd_type.plsy.out_pulse == 0)
                    {
                      HET_L05_0.memory.data_word = 0xFFFFF<<5;                                //pulse number set;
                      pulse_abs_num[0] = 0xFFFFF;
                      plsy_zerof[0] = 1;
                    }
                    else
                    {
                      use_unit[0] = pto_str[0].cmd_type.plsy.out_pulse & 0xFFF00000;
                      use_unit[0] >>= 20;
                      pulse_cyc_num[0] = use_unit[0];
                      use_unit[0] = pto_str[0].cmd_type.plsy.out_pulse & 0xFFFFF;
                      pulse_abs_num[0] = use_unit[0];
                      use_unit[0] <<= 5;
                      HET_L05_0.memory.data_word = use_unit[0];                                //pulse number interrupt ennable;
                      plsy_zerof[0] = 0;
                    }
                    HET_L05_0.memory.control_word |= 0x01;                                       //count pulse enable;
                    use_unit[0] = (unsigned int)(3000000/(float)pto_str[0].cmd_type.plsy.out_freq+0.5);

                    //frq micro
                    delta = 3000000/(float)pto_str[0].cmd_type.plsy.out_freq;
                    if(use_unit[0]-delta > 0.25)
                      microFrq = -1;
                    else if(delta - use_unit[0] > 0.25)
                      microFrq = 1;


                    if(use_unit[0]>1000000)
                    {
                      //**************************
                      js_unit[0] = (unsigned int)(1000000/(float)pto_str[0].cmd_type.plsy.out_freq+0.5);
                      js_unit[1] = (unsigned int)js_unit[0]/2;

                      cyc_union[0].cmp_num = (3*js_unit[0]) >> 7;//5;
                      cyc_union[0].cyc_num = 127;//31;
                      if(3*js_unit[1] < cyc_union[0].cmp_num )
                      {
                        pwm_cyle[0] = 0;
                        cyc_union[2].cyc_num = 0;
                        cyc_union[2].cmp_num = 3*js_unit[1];
                      }
                      else
                      {
                        for(i=1; i<128; i++)//(i=1; i<32; i++)
                        {
                          unit3 = cyc_union[0].cmp_num*i;
                          if(3*js_unit[1] < unit3)
                          {
                            pwm_cyle[0] = i-1;
                            cyc_union[2].cyc_num = i-1;
                            cyc_union[2].cmp_num = ((3*js_unit[1] + cyc_union[0].cmp_num)- unit3);
                            if((cyc_union[0].cmp_num - cyc_union[2].cmp_num) < 10)
                              cyc_union[2].cmp_num = (cyc_union[0].cmp_num-10);
                            break;
                          }
                          else if(3*js_unit[1] == unit3)
                          {
                            pwm_cyle[0] = i;
                            cyc_union[2].cyc_num = i;
                            cyc_union[2].cmp_num = 10;
                            break;
                          }
                        }
                      }
                      cyc_union[2].cmp_num <<= 7;//5;
                      use_unit[0]=cyc_union[0].cmp_num;
                      //if((HETDOUT & 0x00000001)==0)
                      //HETDOUT ^= 0x00000001;
                      HET_L02_0.memory.control_word = cyc_union[0].cmp_num-1;
                      HET_L03_0.memory.data_word = cyc_union[2].cmp_num;
                      HET_L03_0.memory.control_word &= 0xFFEFFFFF;                                   //IO output disable
                      HET_L02_0.memory.program_word |= 1;
                      HET_L03_0.memory.control_word |= 1;

                      use_flag = use_unit[0] & 0x01;
                      use_unit[1] = use_unit[0]>>1;
                      use_unit[1] <<= 5;
                      if(use_flag)
                      {
                        use_unit[1] += 8;
                      }
                      if(pto_str[0].out_mode==3)
                        return;
                      HET_L00_0.memory.data_word = 0x10<<5;                                                //Y0 pulse output enable
                      return;
                    }
                    use_flag = use_unit[0] & 0x01;
                    use_unit[1] = use_unit[0]>>1;
                    use_unit[1] <<= 5;
                    if(use_flag)
                    {
                      use_unit[1] += 8;
                    }

    }
    else if(pto_str[0].op_type == 3)
    {
      pto_str[0].count_ones=1;
      plsy_zerof[0] = 0;
      use_unit[0] = plsr_attr[0].pulse_array[0] & 0xFFF00000;
      use_unit[0] >>= 20;
      pulse_cyc_num[0] = use_unit[0];
      use_unit[0] = plsr_attr[0].pulse_array[0] & 0xFFFFF;
      pulse_abs_num[0] = use_unit[0];
      use_unit[0] <<= 5;
      HET_L05_0.memory.data_word = use_unit[0]+1;                                        //pulse number interrupt ennable;
      HET_L05_0.memory.control_word |= 0x01;                                           //count pulse enable;
      use_unit[0] = plsr_attr[0].freq_array[0];
      use_flag = use_unit[0] & 0x01;
      use_unit[1] = use_unit[0]>>1;
      use_unit[1] <<= 5;
      if(use_flag)
      {
        use_unit[1] += 8;
      }
    }
    HET_L04_0.memory.data_word = 0;
  }
  else if(op_mode == 2)
  {
    use_unit[0] = (unsigned int)(3000000/(float)pto_str[0].cmd_type.plsy.out_freq+0.5);

    //frq micro
    delta = 3000000/(float)pto_str[0].cmd_type.plsy.out_freq;
    if(use_unit[0]-delta > 0.25)
      microFrq = -1;
    else if(delta - use_unit[0] > 0.25)
      microFrq = 1;


    if(use_unit[0]>1000000)
    {
      //**************************
      js_unit[0] = (unsigned int)(1000000/(float)pto_str[0].cmd_type.plsy.out_freq+0.5);
      js_unit[1] = (unsigned int)js_unit[0]/2;

      cyc_union[0].cmp_num = (3*js_unit[0]) >> 7;//5;
      cyc_union[0].cyc_num = 127;//31;
      if(3*js_unit[1] < cyc_union[0].cmp_num )
      {
        pwm_cyle[0] = 0;
        cyc_union[2].cyc_num = 0;
        cyc_union[2].cmp_num = 3*js_unit[1];
      }
      else
      {
        for(i=1; i<128; i++)//(i=1; i<32; i++)
        {
          unit3 = cyc_union[0].cmp_num*i;
          if(3*js_unit[1] < unit3)
          {
            pwm_cyle[0] = i-1;
            cyc_union[2].cyc_num = i-1;
            cyc_union[2].cmp_num = ((3*js_unit[1] + cyc_union[0].cmp_num)- unit3);
            if((cyc_union[0].cmp_num - cyc_union[2].cmp_num) < 10)
              cyc_union[2].cmp_num = (cyc_union[0].cmp_num-10);
            break;
          }
          else if(3*js_unit[1] == unit3)
          {
            pwm_cyle[0] = i;
            cyc_union[2].cyc_num = i;
            cyc_union[2].cmp_num = 10;
            break;
          }
        }
      }
      cyc_union[2].cmp_num <<= 7;//5;
      use_unit[0]=cyc_union[0].cmp_num;
      //if((HETDOUT & 0x00000001)==0)
      //HETDOUT ^= 0x00000001;
      HET_L02_0.memory.control_word = cyc_union[0].cmp_num-1;
      HET_L03_0.memory.data_word = cyc_union[2].cmp_num;
      HET_L03_0.memory.control_word &= 0xFFEFFFFF;                                   //IO output disable
      HET_L02_0.memory.program_word |= 1;
      HET_L03_0.memory.control_word |= 1;
      use_flag = use_unit[0] & 0x01;
      use_unit[1] = use_unit[0]>>1;
      use_unit[1] <<= 5;
      if(use_flag)
      {
        use_unit[1] += 8;
      }
      HET_L05_0.memory.control_word |= 0x01;                                           //count pulse enable;
      if(pto_str[0].out_mode==3)
        return;
      HET_L00_0.memory.data_word = 0x10<<5;                                                //Y0 pulse output enable
      return;

      //use_unit[0]=1000000;
    }
    use_flag = use_unit[0] & 0x01;
    use_unit[1] = use_unit[0]>>1;
    use_unit[1] <<= 5;
    if(use_flag)
    {
      use_unit[1] += 8;
    }
    HET_L05_0.memory.control_word |= 0x01;                                           //count pulse enable;


  }

  //2006.12.12 added
  HET_L02_0.memory.program_word &= 0xFFFFFFFE;
  HET_L03_0.memory.control_word &= 0xFFFFFFFE;
  //added end

  HET_L02_0.memory.data_word = 0;
  HET_L02_0.memory.control_word = use_unit[0]-1;//+microFrq;
  //HET_L02_0.memory.control_word |= 31;
  //use_unit[1] |= 31;
  HET_L03_0.memory.data_word = use_unit[1];
  HET_L03_0.memory.control_word |= 1<<20;                                                 //enable output enable
  if(pto_str[0].out_mode==3)
    return;
  HET_L00_0.memory.data_word = 0x10<<5;                                                //Y0 pulse output enable
}

/************************************************************************
function: 对Y1输出的硬件初始化
description:
input   :op_mode为1初始化硬件；为2刷新输入频率；
output  : no;
************************************************************************/
void y1_init_pto(unsigned char op_mode)
{
  unsigned int use_unit[2];
  unsigned char use_flag;
  unsigned char i;
  unsigned int unit3;
  unsigned int js_unit[2];
  int microFrq=0;
  float delta=0;



  HET_L06_0.memory.data_word = 0;                                                  //Y0 pulse  disable
  if((op_mode==1))
  {
    pto_str[1].out_mode = 1;
    if(pto_str[1].op_type == 1)
    {
		    pto_str[1].count_ones=1;
                    if(pto_str[1].cmd_type.plsy.out_pulse == 1 && pto_str[1].cmd_type.plsy.out_freq > 75000)
                    {
                      pto_str[1].cmd_type.plsy.out_freq = 75000;
                    }
                    if(pto_str[1].cmd_type.plsy.out_pulse == 0)
                    {
                      HET_L11_0.memory.data_word = 0xFFFFF<<5;                                //pulse number set;
                      pulse_abs_num[1] = 0xFFFFF;
                      plsy_zerof[1] = 1;
                    }
                    else
                    {
                      use_unit[0] = pto_str[1].cmd_type.plsy.out_pulse & 0xFFF00000;
                      use_unit[0] >>= 20;
                      pulse_cyc_num[1] = use_unit[0];
                      use_unit[0] = pto_str[1].cmd_type.plsy.out_pulse & 0xFFFFF;
                      pulse_abs_num[1] = use_unit[0];
                      use_unit[0] <<= 5;
                      HET_L11_0.memory.data_word = use_unit[0];                                //pulse number interrupt ennable;
                      plsy_zerof[1] = 0;
                    }
                    HET_L11_0.memory.control_word |= 0x01;                                       //count pulse enable;
                    use_unit[0] = (unsigned int)(3000000/(float)pto_str[1].cmd_type.plsy.out_freq+0.5);

                    //frq micro
                    delta = 3000000/(float)pto_str[1].cmd_type.plsy.out_freq;
                    if(use_unit[0]-delta > 0.25)
                      microFrq = -1;
                    else if(delta - use_unit[0] > 0.25)
                      microFrq = 1;

                    if(use_unit[0]>1000000)
                    {
                      js_unit[0] = (unsigned int)(1000000/(float)pto_str[1].cmd_type.plsy.out_freq+0.5);
                      js_unit[1] = (unsigned int)js_unit[0]/2;

                      cyc_union[1].cmp_num = (3*js_unit[0]) >> 7;//5;
                      cyc_union[1].cyc_num = 127;//31;
                      if(3*js_unit[1] < cyc_union[1].cmp_num )
                      {
                        pwm_cyle[1] = 0;
                        cyc_union[3].cyc_num = 0;
                        cyc_union[3].cmp_num = 3*js_unit[1];
                      }
                      else
                      {
                        for(i=1; i<128; i++)//(i=1; i<32; i++)
                        {
                          unit3 = cyc_union[1].cmp_num*i;
                          if(3*js_unit[1] < unit3)
                          {
                            pwm_cyle[1] = i-1;
                            cyc_union[3].cyc_num = i-1;
                            cyc_union[3].cmp_num = ((3*js_unit[1] + cyc_union[1].cmp_num)- unit3);
                            if((cyc_union[1].cmp_num - cyc_union[3].cmp_num) < 10)
                              cyc_union[3].cmp_num = (cyc_union[1].cmp_num-10);
                            break;
                          }
                          else if(3*js_unit[1] == unit3)
                          {
                            pwm_cyle[1] = i;
                            cyc_union[3].cyc_num = i;
                            cyc_union[3].cmp_num = 10;
                            break;
                          }
                        }
                      }
                      cyc_union[3].cmp_num <<= 7;//5;
                      use_unit[0]=cyc_union[1].cmp_num;
                      //if((HETDOUT & 0x00000001)==0)
                      //HETDOUT ^= 0x00000001;
                      HET_L08_0.memory.control_word = cyc_union[1].cmp_num-1;
                      HET_L09_0.memory.data_word = cyc_union[3].cmp_num;
                      HET_L09_0.memory.control_word &= 0xFFEFFFFF;                                   //IO output disable
                      HET_L08_0.memory.program_word |= 1;
                      HET_L09_0.memory.control_word |= 1;


                      use_flag = use_unit[0] & 0x01;
                      use_unit[1] = use_unit[0]>>1;
                      use_unit[1] <<= 5;
                      if(use_flag)
                      {
                        use_unit[1] += 8;
                      }
                      if(pto_str[1].out_mode==3)
                        return;
                      HET_L06_0.memory.data_word = 0x10<<5;                                                //Y0 pulse output enable
                      return;
                    }
                    use_flag = use_unit[0] & 0x01;
                    use_unit[1] = use_unit[0]>>1;
                    use_unit[1] <<= 5;
                    if(use_flag)
                    {
                      use_unit[1] += 8;
                    }
    }
    else if(pto_str[1].op_type == 3)
    {
      plsy_zerof[1] = 0;
      pto_str[1].count_ones=1;
      use_unit[0] = plsr_attr[1].pulse_array[0] & 0xFFF00000;
      use_unit[0] >>= 20;
      pulse_cyc_num[1] = use_unit[0];
      use_unit[0] = plsr_attr[1].pulse_array[0] & 0xFFFFF;
      pulse_abs_num[1] = use_unit[0];
      use_unit[0] <<= 5;
      HET_L11_0.memory.data_word = use_unit[0]+1;                                        //pulse number interrupt ennable;
      HET_L11_0.memory.control_word |= 0x01;                                           //count pulse enable;
      use_unit[0] = plsr_attr[1].freq_array[0];
      use_flag = use_unit[0] & 0x01;
      use_unit[1] = use_unit[0]>>1;
      use_unit[1] <<= 5;
      if(use_flag)
      {
        use_unit[1] += 8;
      }
    }
    HET_L10_0.memory.data_word = 0;
  }
  else if(op_mode == 2)
  {
    use_unit[0] = (unsigned int)(3000000/(float)pto_str[1].cmd_type.plsy.out_freq+0.5);

    //frq micro
    delta = 3000000/(float)pto_str[1].cmd_type.plsy.out_freq;
    if(use_unit[0]-delta > 0.25)
      microFrq = -1;
    else if(delta - use_unit[0] > 0.25)
      microFrq = 1;

    if(use_unit[0]>1000000)
    {
      js_unit[0] = (unsigned int)(1000000/(float)pto_str[1].cmd_type.plsy.out_freq+0.5);
      js_unit[1] = (unsigned int)js_unit[0]/2;

      cyc_union[1].cmp_num = (3*js_unit[0]) >> 7;//5;
      cyc_union[1].cyc_num = 127;//31;
      if(3*js_unit[1] < cyc_union[1].cmp_num )
      {
        pwm_cyle[1] = 0;
        cyc_union[3].cyc_num = 0;
        cyc_union[3].cmp_num = 3*js_unit[1];
      }
      else
      {
        for(i=1; i<128; i++)//(i=1; i<32; i++)
        {
          unit3 = cyc_union[1].cmp_num*i;
          if(3*js_unit[1] < unit3)
          {
            pwm_cyle[1] = i-1;
            cyc_union[3].cyc_num = i-1;
            cyc_union[3].cmp_num = ((3*js_unit[1] + cyc_union[1].cmp_num)- unit3);
            if((cyc_union[1].cmp_num - cyc_union[3].cmp_num) < 10)
              cyc_union[3].cmp_num = (cyc_union[1].cmp_num-10);
            break;
          }
          else if(3*js_unit[1] == unit3)
          {
            pwm_cyle[1] = i;
            cyc_union[3].cyc_num = i;
            cyc_union[3].cmp_num = 10;
            break;
          }
        }
      }
      cyc_union[3].cmp_num <<= 7;//5;
      use_unit[0]=cyc_union[1].cmp_num;
      //if((HETDOUT & 0x00000001)==0)
      //HETDOUT ^= 0x00000001;
      HET_L08_0.memory.control_word = cyc_union[1].cmp_num-1;
      HET_L09_0.memory.data_word = cyc_union[3].cmp_num;
      HET_L09_0.memory.control_word &= 0xFFEFFFFF;                                   //IO output disable
      HET_L08_0.memory.program_word |= 1;
      HET_L09_0.memory.control_word |= 1;


      use_flag = use_unit[0] & 0x01;
      use_unit[1] = use_unit[0]>>1;
      use_unit[1] <<= 5;
      if(use_flag)
      {
        use_unit[1] += 8;
      }
      if(pto_str[1].out_mode==3)
        return;
      HET_L06_0.memory.data_word = 0x10<<5;                                                //Y0 pulse output enable
      return;
    }
    use_flag = use_unit[0] & 0x01;
    use_unit[1] = use_unit[0]>>1;
    use_unit[1] <<= 5;
    if(use_flag)
    {
      use_unit[1] += 8;
    }
    HET_L11_0.memory.control_word |= 0x01;                                           //count pulse enable;
  }
  //2006.12.12 added
  HET_L08_0.memory.program_word &= 0xFFFFFFFE;
  HET_L09_0.memory.control_word &= 0xFFFFFFFE;
  //added end

  HET_L08_0.memory.data_word = 0;
  HET_L08_0.memory.control_word = use_unit[0]-1;//+microFrq;
  HET_L09_0.memory.data_word = use_unit[1];
  HET_L09_0.memory.control_word |= 1<<20;                                                 //enable output enable
  if(pto_str[1].out_mode==3)
    return;
  HET_L06_0.memory.data_word = 0x10<<5;                                                //Y0 pulse output enable
}

//////////////////////////////////////////////////////////////////////////
/*PLSY指令的处理程序*/
//////////////////////////////////////////////////////////////////////////
/************************************************************************
function: 对Y0输出的硬件初始化
description:
input   : op_mode为0对应硬件Y0为普通设定；为1对应硬件的初始化；
为2对应输出频率的刷新;   为3脉冲数据的计算
output  : no;
************************************************************************/
void plsy_init_y0(unsigned char op_mode)
{
  unsigned char i,*usep;
  unsigned int use_unit,use_unit1;
  //对Y0的脉冲输出初始化
  if (op_mode == 1)
  {
    pto_str[0].op_type = 1;      					                        //为 PLSY模式
    y0_init_pto(1);                                                     //init the hardware            		
  }
  else if(op_mode == 2)                                                   //reflash the pto frequency and the
  {
    y0_init_pto(2);                                         		
  }
  else
  {
    if(pto_str[0].out_mode != 3)
    {
    __disable_interrupt();  
      use_unit = HET_L04_0.memory.data_word;
      use_unit1 = HET_L10_0.memory.data_word;
      use_unit >>= 5;
      use_unit1 >>= 5;
      if(pto_str[0].count_ones != 1)
      {
        if (use_unit < pulse_abs_num[0])
            use_unit += 0x100000;
        use_unit -= pulse_abs_num[0];
        use_unit &= 0xFFFFF;
      }

      if(pto_str[1].count_ones != 1)
      {
        use_unit1 -= pulse_abs_num[1];
        use_unit1 &= 0xFFFFF;
      }
      *pulse_count0 = pto_pluse_add[0]+use_unit;    	   						//处理Y0输出的脉冲总数
      *pulse_count2 = pto_pluse_add[2]+use_unit1+use_unit;       					    //处理Y1和Y0输出的脉冲总数

     if(pto_count_flag.y0F)
      {
        if(pto_str[0].count_ones != 1)
         {

            if (pto_count_now[0] < pulse_abs_num[0])
              {
               *pulse_count0 -= (0x100000+pto_count_now[0]-pulse_abs_num[0]);
              }
            else
              *pulse_count0 -= (pto_count_now[0]-pulse_abs_num[0]);
         }
       else
          *pulse_count0 -= pto_count_now[0];
      }
__enable_interrupt();

      if(pto_count_flag.addy0F)
        *pulse_count2 -= pto_count_now[2];
    }
    if(op_mode == 0)                                                         //init the Y000 for the normal IO
    {
      if(pto_str[0].out_mode != 3)
      {
        pto_pluse_add[0] = *pulse_count0;    	   						          //处理Y0输出的脉冲总数
        pto_pluse_add[2] = (*pulse_count2-use_unit1);       					              //处理Y1和Y0输出的脉冲总数
      }
      HET_L00_0.memory.data_word = 0;
      HET_L02_0.memory.data_word = 0;
      HET_L03_0.memory.control_word &= 0xFFEFFFFE;
      HET_L04_0.memory.data_word = 0;
      HET_L05_0.memory.control_word &= 0xFFFFFFFE;
      pto_count_now[0] = 0;
      pto_count_now[2] = 0;
      pulse_abs_num[0] = 0;   //zhao add for SD[54]

      if(HETDOUT & 0x00000001)
        HETDOUT ^= 0x00000001;
      g_Y[0] = 0;
      usep=(unsigned char *)&pto_str[0];
      for(i=0;i<sizeof(pto_str[0]);i++)
        *usep++=0;
    }
  }
}

/************************************************************************
function: 对Y1输出的硬件初始化
description:
input   : op_mode为0对应硬件Y0为普通设定；为1对应硬件的初始化；为2对应输出
频率的刷新;为3脉冲数据的计算
output  : no;
************************************************************************/
void plsy_init_y1(unsigned char op_mode)
{
  unsigned char i,*usep;
  unsigned int use_unit,use_unit1;
  //对Y0的脉冲输出初始化
  if (op_mode == 1)
  {
    pto_str[1].op_type=1;      					//为 PLSY模式
    y1_init_pto(1);                             //init the hardware            		
  }
  else if(op_mode == 2)                           //reflash the pto frequency and the
  {
    y1_init_pto(2);                                         		
  }
  else
  {
    if(pto_str[1].out_mode != 3)
    {
/*      use_unit = HET_L10_0.memory.data_word;
      use_unit1 = HET_L04_0.memory.data_word;
      use_unit >>= 5;
      use_unit1 >>= 5;
      if(pto_str[0].count_ones != 1)
      {
        use_unit1 -= pulse_abs_num[0];
        use_unit1 &= 0xFFFFF;
      }
      if(pto_str[1].count_ones != 1)
      {
        use_unit -= pulse_abs_num[1];
        use_unit &= 0xFFFFF;
      }
      *pulse_count1 = pto_pluse_add[1]+use_unit;    	   					    //处理Y0输出的脉冲总数
      *pulse_count2 = pto_pluse_add[2]+use_unit1+use_unit;       					    //处理Y1和Y0输出的脉冲总数
      if(pto_count_flag.y1F)
        *pulse_count1-=pto_count_now[1];
      if(pto_count_flag.addy1F)
        *pulse_count2-=pto_count_now[3];
      */
      __disable_interrupt();  
      use_unit = HET_L10_0.memory.data_word;
      use_unit1 = HET_L04_0.memory.data_word;
      use_unit >>= 5;
      use_unit1 >>= 5;
      if(pto_str[1].count_ones != 1)
      {
        if (use_unit < pulse_abs_num[1])
            use_unit += 0x100000;
        use_unit -= pulse_abs_num[1];
        use_unit &= 0xFFFFF;
      }

      if(pto_str[0].count_ones != 1)
      {
        use_unit1 -= pulse_abs_num[0];
        use_unit1 &= 0xFFFFF;
      }
      *pulse_count1 = pto_pluse_add[1]+use_unit;    	   						//处理Y0输出的脉冲总数
      *pulse_count2 = pto_pluse_add[2]+use_unit1+use_unit;       					    //处理Y1和Y0输出的脉冲总数

     if(pto_count_flag.y1F)
      {
        if(pto_str[1].count_ones != 1)
         {

            if (pto_count_now[1] < pulse_abs_num[1])
              {
               *pulse_count1 -= (0x100000+pto_count_now[1]-pulse_abs_num[1]);
              }
            else
              *pulse_count1 -= (pto_count_now[1]-pulse_abs_num[1]);
         }
       else
          *pulse_count1 -= pto_count_now[1];
      }

	__enable_interrupt();
      if(pto_count_flag.addy1F)
        *pulse_count2 -= pto_count_now[3];
    }
    if(op_mode == 0)                                                   //init the Y000 for the normal IO
    {
      if(pto_str[1].out_mode != 3)
      {
        pto_pluse_add[1] = *pulse_count1;    	   					        //处理Y0输出的脉冲总数
        pto_pluse_add[2] = (*pulse_count2-use_unit1);
      }
      HET_L06_0.memory.data_word = 0;
      HET_L08_0.memory.data_word = 0;
      HET_L09_0.memory.control_word &= 0xFFEFFFFE;
      HET_L10_0.memory.data_word = 0;
      HET_L11_0.memory.control_word &= 0xFFFFFFFE;
      pto_count_now[1] = 0;
      pto_count_now[3] = 0;
      pulse_abs_num[1] = 0;   //zhao add for SD[54]
      if(HETDOUT & 0x00000010)
        HETDOUT ^= 0x00000010;
      g_Y[1] = 0;
      usep=(unsigned char *)&pto_str[1];
      for(i=0; i<sizeof(pto_str[1]); i++)
        *usep++ = 0;
    }
  }
}

/************************************************************************
function: PLSY脉冲输出指令;
description:对Y端口输出操作;
input   : EXC_ENV结构的变量指针;
output  : no;
************************************************************************/
unsigned char f_CI_PLSY(EXC_ENV *exc_env)
{
  unsigned char *usep,jude_unit;
  unsigned int  use_code[2];
  usep=(*exc_env).pc_ucode;
  if(*(usep+14)!=2)              //判断Y0和Y1有效吗？
    return ERR_ELEM_TYPE;
  if(*(usep+17)>1)
    return ERR_ELEM_RANG;
  //能流有效处理
  if(PF(exc_env) && (g_SM[*(usep+17)+80]==0))
  {
    //取输出频率的处理；
    jude_unit=get_dword(usep+2,&use_code[0],0,1);
    if(jude_unit)
    {
      if(pto_str[*(usep+17)].ucode_pc != (unsigned int)usep)
        return jude_unit;
      g_SM[*(usep+17)+82]=0;
      if(*(usep+17))
        plsy_init_y1(0);
      else
        plsy_init_y0(0);
      return jude_unit;
    }
    //判断输出是否完成
    if(pto_str[*(usep+17)].out_mode==3 &&  g_SM[86+*(usep+17)]==0)//g_SM[86+*(usep+17)]==0)
    {
      g_Y[*(usep+17)]=0;
      g_SM[*(usep+17)+82] = 0;
      return RIGHT;
    }
    //判断指令是否在执行
    if(pto_str[*(usep+17)].op_type==1)
    {
      if(pto_str[*(usep+17)].ucode_pc != (unsigned int)usep)
        return RIGHT;

      if(use_code[0]>100000||(use_code[0]>50000&&MODULE_TUPE==2))												//判断频率为零的处理
      {
        //置对应的监控位为0
        g_SM[*(usep+17)+82]=0;
        if(*(usep+17))
          plsy_init_y1(0);
        else
          plsy_init_y0(0);
        return ERR_OPERAND_VAL;
      }
      else if(use_code[0]==0)											//判断频率为零的处理
      {
        //置对应的监控位为0
        g_SM[*(usep+17)+82]=0;
        if(*(usep+17))
          plsy_init_y1(0);
        else
          plsy_init_y0(0);
        return RIGHT;
      }
      //判断频率是否改变
      if(use_code[0]!=pto_str[*(usep+17)].cmd_type.plsy.out_freq)
      {
        pto_str[*(usep+17)].cmd_type.plsy.out_freq=use_code[0];
        //刷新频率的处理
        if(*(usep+17))
          plsy_init_y1(2);
        else
          plsy_init_y0(2);
      }
      //脉冲数据的计算
      else
      {
        if(*(usep+17))
          plsy_init_y1(3);
        else
          plsy_init_y0(3);
      }
    }
    //判断Y[*(usep+17)]没有输出指令
    else if(pto_str[*(usep+17)].op_type==0)
    {
      if(use_code[0]==0)									//判断频率为零的处理
        return RIGHT;
      else if(use_code[0]>100000||(use_code[0]>50000&&MODULE_TUPE==2))	
        return ERR_OPERAND_VAL;

      jude_unit=get_dword(usep+8,&use_code[1],0,1);    //取输出脉冲数
      if(jude_unit)
        return jude_unit;
      if(use_code[1]>2147483647)
        return ERR_OPERAND_VAL;
      pto_str[*(usep+17)].cmd_type.plsy.out_freq=use_code[0];
      pto_str[*(usep+17)].cmd_type.plsy.out_pulse=use_code[1];
      pto_str[*(usep+17)].ucode_pc=(unsigned int)usep;
      //置对应的监控位为1
      g_SM[*(usep+17)+82]=1;
      //初始化处理
      if(*(usep+17))
      {
        if(HETDOUT & 0x00000010)
          HETDOUT ^= 0x00000010;
        plsy_init_y1(1);
      }
      else
      {
        if(HETDOUT & 0x00000001)
          HETDOUT ^= 0x00000001;
        plsy_init_y0(1);
      }
    }
  }
  //能流无效处理
  else
  {
    if(pto_str[*(usep+17)].op_type==1 && (g_SM[86+*(usep+17)]==0 || (g_SM[86+*(usep+17)]==1 && pto_str[*(usep+17)].cmd_type.plsy.out_pulse==0)|| g_SM[*(usep+17)+80]==1))
    {
      if(pto_str[*(usep+17)].ucode_pc!=(unsigned int)usep)
        return RIGHT;
      //置对应的监控位为0
      g_SM[*(usep+17)+82]=0;
      if(*(usep+17))
        plsy_init_y1(0);
      else
        plsy_init_y0(0);
    }
  }  //end of else
  return RIGHT;
}

//////////////////////////////////////////////////////////////////////////
/*PLSR指令的处理程序*/
//////////////////////////////////////////////////////////////////////////
/************************************************************************
function: PLSR指令对Y0输出的频率变化函数
description:
input   : o
output  : no;
************************************************************************/
void plsr_change_y0()
{
  unsigned char use_flag;
  unsigned int  use_unit[2];
  HET_L00_0.memory.data_word = 0;                                               //Y0 pulse  disable
  if(plsr_attr[0].rec_num<g_plsrGrades[0])
    plsr_attr[0].rec_num++;
  else
  {
    HET_L03_0.memory.control_word &= 0xFFEFFFFF;
    HET_L00_0.memory.data_word = 0;
    if(HETDOUT & 0x00000001)
      HETDOUT ^= 0x00000001;
    pto_str[0].out_mode=3;
    g_SM[82]=0;
    if(g_SM[63] == 1)
      add_intr(18);

    return;
  }
  pto_str[0].count_ones=1;
  use_unit[0] = plsr_attr[0].pulse_array[plsr_attr[0].rec_num] & 0xFFF00000;
  use_unit[0] >>= 20;
  pulse_cyc_num[0] = use_unit[0];
  use_unit[0] = plsr_attr[0].pulse_array[plsr_attr[0].rec_num] & 0xFFFFF;
  pulse_abs_num[0] = use_unit[0];
  use_unit[0] <<= 5;
  HET_L04_0.memory.data_word = 0;
  HET_L05_0.memory.data_word = use_unit[0];                                        //pulse number interrupt ennable;
  HET_L05_0.memory.control_word |= 0x01;                                           //count pulse enable;
  use_unit[0] = plsr_attr[0].freq_array[plsr_attr[0].rec_num];

  if(use_unit[0]>1000000)
  {
    use_unit[0]=1000000;
  }

  use_flag = use_unit[0] & 0x01;
  use_unit[1] = use_unit[0]>>1;
  use_unit[1] <<= 5;
  if(use_flag)
  {
    use_unit[1] += 8;
  }
  HET_L02_0.memory.data_word = use_unit[1]-0x100;
  HET_L02_0.memory.control_word = use_unit[0]-1;
  HET_L03_0.memory.data_word = use_unit[1];
  HET_L03_0.memory.control_word |= 1<<20;                                                 //enable output enable
  HET_L00_0.memory.data_word = 0x10<<5;                                                //Y0 pulse output enable
}

/************************************************************************
function: PLSR指令对Y1输出的频率变化函数
description:
input   : o
output  : no;
************************************************************************/
void plsr_change_y1()
{
  unsigned char use_flag;
  unsigned int  use_unit[2];
  HET_L06_0.memory.data_word = 0;                                               //Y0 pulse  disable
  if(plsr_attr[1].rec_num<g_plsrGrades[1])
    plsr_attr[1].rec_num++;
  else
  {

    HET_L09_0.memory.control_word &= 0xFFEFFFFF;
    HET_L06_0.memory.data_word = 0;
    if(HETDOUT & 0x00000010)
      HETDOUT ^= 0x00000010;
    pto_str[1].out_mode=3;
    g_SM[83]=0;
    if(g_SM[64] == 1)
      add_intr(19);


    return;
  }
  pto_str[1].count_ones=1;
  use_unit[0] = (plsr_attr[1].pulse_array[plsr_attr[1].rec_num] & 0xFFF00000);
  use_unit[0] >>= 20;
  pulse_cyc_num[1] = use_unit[0];
  use_unit[0] = (plsr_attr[1].pulse_array[plsr_attr[1].rec_num] & 0xFFFFF);
  pulse_abs_num[1] = use_unit[0];
  use_unit[0] <<= 5;
  HET_L10_0.memory.data_word = 0;
  HET_L11_0.memory.data_word = use_unit[0];                                        		//pulse number interrupt ennable;
  HET_L11_0.memory.control_word |= 0x01;                                           		//count pulse enable;
  use_unit[0] = plsr_attr[1].freq_array[plsr_attr[1].rec_num];

  if(use_unit[0]>1000000)
  {
    use_unit[0]=1000000;
  }

  use_flag = use_unit[0] & 0x01;
  use_unit[1] = use_unit[0]>>1;
  use_unit[1] <<= 5;
  if(use_flag)
  {
    use_unit[1] += 8;
  }
  HET_L08_0.memory.data_word = use_unit[1]-0x100;
  HET_L08_0.memory.control_word = use_unit[0]-1;
  HET_L09_0.memory.data_word = use_unit[1];
  HET_L09_0.memory.control_word |= 1<<20;                                                 //enable output enable
  HET_L06_0.memory.data_word = 0x10<<5;                                                //Y0 pulse output enable
}

/************************************************************************
function: 对Y0输出的硬件初始化
description:
input   : op_mode为0对应硬件Y0为普通设定；为1对应硬件的初始化；为3脉冲数据的计算
output  : no;
************************************************************************/
void plsr_init_y0(unsigned char op_mode)
{
  unsigned char *usep;
  unsigned int i;
  unsigned int  use_unit,use_unit1;
  //对Y0的脉冲输出初始化
  if (op_mode==1)
  {
    pto_str[0].op_type=3;       											//为PLSR模式
    pto_str[0].count_ones=1;
    y0_init_pto(1);		
  }
  else
  {
    if(pto_str[0].out_mode != 3)
    {
      use_unit = HET_L04_0.memory.data_word;
      use_unit1 = HET_L10_0.memory.data_word;
      use_unit >>= 5;
      use_unit1 >>= 5;
      if(pto_str[0].count_ones != 1)
      {
        use_unit -= pulse_abs_num[0];
        use_unit &= 0xFFFFF;
      }
      if(pto_str[1].count_ones != 1)
      {
        use_unit1 -= pulse_abs_num[1];
        use_unit1 &= 0xFFFFF;
      }
      *pulse_count0 = pto_pluse_add[0]+use_unit;    	   						//处理Y0输出的脉冲总数
      *pulse_count2 = pto_pluse_add[2]+use_unit1+use_unit;       					    //处理Y1和Y0输出的脉冲总数
      if(pto_count_flag.y0F)
        *pulse_count0 -= pto_count_now[0];
      if(pto_count_flag.addy0F)
        *pulse_count2 -= pto_count_now[2];
    }
    if(op_mode == 0)                                                   		//init the Y000 for the normal IO
    {
      if(pto_str[0].out_mode != 3)
      {
        pto_pluse_add[0] = *pulse_count0;    	   						          //处理Y0输出的脉冲总数
        pto_pluse_add[2] = (*pulse_count2-use_unit1);       					              //处理Y1和Y0输出的脉冲总数
      }
      HET_L00_0.memory.data_word = 0;
      HET_L02_0.memory.data_word = 0;
      HET_L03_0.memory.control_word &= 0xFFEFFFFE;
      HET_L04_0.memory.data_word = 0;
      HET_L05_0.memory.control_word &= 0xFFFFFFFE;
      pto_count_now[0] = 0;
      pto_count_now[2] = 0;

      if(HETDOUT & 0x00000001)
        HETDOUT ^= 0x00000001;
      g_Y[0] = 0;
      usep = (unsigned char *)&pto_str[0];
      for(i=0; i<sizeof(pto_str[0]); i++)
        *usep++=0;
      usep = (unsigned char *)&plsr_attr[0];
      for(i=0; i<sizeof(plsr_attr[0]); i++)
        *usep++=0;
    }
  }
}

/************************************************************************
function: 对Y1输出的硬件初始化
description:
input   : op_mode为0对应硬件Y1为普通设定；为1对应硬件的初始化；为3脉冲数据的计算
output  : no;
************************************************************************/
void plsr_init_y1(unsigned char op_mode)
{
  unsigned char *usep;
  unsigned int i;
  unsigned int  use_unit,use_unit1;
  //对Y0的脉冲输出初始化
  if (op_mode==1)
  {
    pto_str[1].op_type=3;       											//为PLSR模式
    pto_str[1].count_ones=1;
    y1_init_pto(1);		
  }
  else
  {
    if(pto_str[1].out_mode != 3)
    {
      use_unit = HET_L10_0.memory.data_word;
      use_unit1 = HET_L04_0.memory.data_word;
      use_unit >>= 5;
      use_unit1 >>= 5;
      if(pto_str[0].count_ones != 1)
      {
        use_unit1 -= pulse_abs_num[0];
        use_unit1 &= 0xFFFFF;
      }
      if(pto_str[1].count_ones != 1)
      {
        use_unit -= pulse_abs_num[1];
        use_unit &= 0xFFFFF;
      }
      *pulse_count1 = pto_pluse_add[1]+use_unit;    	   					    //处理Y0输出的脉冲总数
      *pulse_count2 = pto_pluse_add[2]+use_unit1+use_unit;       					    //处理Y1和Y0输出的脉冲总数
      if(pto_count_flag.y1F)
        *pulse_count1-=pto_count_now[1];
      if(pto_count_flag.addy1F)
        *pulse_count2-=(pto_count_now[3]);
    }
    if(op_mode == 0)                                                   		//init the Y000 for the normal IO
    {
      if(pto_str[1].out_mode != 3)
      {
        pto_pluse_add[1] = *pulse_count1;    	   					        //处理Y0输出的脉冲总数
        pto_pluse_add[2] = (*pulse_count2-use_unit1);
      }
      HET_L06_0.memory.data_word = 0;
      HET_L08_0.memory.data_word = 0;
      HET_L09_0.memory.control_word &= 0xFFEFFFFE;
      HET_L10_0.memory.data_word = 0;
      HET_L11_0.memory.control_word &= 0xFFFFFFFE;
      pto_count_now[1] = 0;
      pto_count_now[3] = 0;
      if(HETDOUT & 0x00000010)
        HETDOUT ^= 0x00000010;
      g_Y[1] = 0;
      usep = (unsigned char *)&pto_str[1];
      for(i=0; i<sizeof(pto_str[1]); i++)
        *usep++=0;
      usep = (unsigned char *)&plsr_attr[1];
      for(i=0; i<sizeof(plsr_attr[1]); i++)
        *usep++=0;
    }
  }
}

/************************************************************************
function: PLSR加减脉冲输出指令;
description:对Y端口输出操作;
input   : EXC_ENV结构的变量指针;
output  : no;
************************************************************************/
unsigned char f_CI_PLSR(EXC_ENV *exc_env)
{
  unsigned char *usep,jude_unit,i;
  unsigned short  use_code[2];
  unsigned int  use_code2,cmp_num_ok[2];
  float    pulsep,freqp;
  //判断Y0和Y1有效吗？
  usep=(*exc_env).pc_ucode;
  if(*(usep+16)!=2)              						
    return ERR_ELEM_TYPE;
  if(*(usep+19)>1)
    return ERR_ELEM_RANG;

  //能流有效处理
  if(PF(exc_env) && (g_SM[*(usep+19)+80]==0))
  {
    if(pto_str[*(usep+19)].out_mode==3)
    {
      g_Y[*(usep+19)]=0;
      g_SM[*(usep+19)+82] = 0;
      return RIGHT;
    }
    //判断指令是否在执行
    if(pto_str[*(usep+19)].op_type==3)
    {
      //判断是否为本指令
      if(pto_str[*(usep+19)].ucode_pc!=(unsigned int)usep)
        return RIGHT;
      //输出脉冲数量计算
      if(*(usep+19))
        plsr_init_y1(3);
      else
        plsr_init_y0(3);
    }
    //判断Y[*(usep+19)]没有输出指令
    else if(pto_str[*(usep+19)].op_type==0)
    {
      //取输出频率并进行有效性判断
      jude_unit=get_word(usep+2,&use_code[0],0,1);
      if(jude_unit)	
        return jude_unit;
      if(use_code[0]>20000)
      {
        use_code[0]=20000;
        g_SD[20]=ERR_OPERAND_VAL;
      }
      else if(use_code[0]<10)
      {
        use_code[0]=10;
        g_SD[20]=ERR_OPERAND_VAL;
      }
      //取上升时间并进行有效性判断
      jude_unit=get_word(usep+12,&use_code[1],0,1);
      if(jude_unit)
        return jude_unit;
      if(use_code[1]>10000)
        return ERR_OPERAND_VAL;

      //取输出脉冲总数并进行有效性判断
      jude_unit=get_dword(usep+6,&use_code2,0,1);    	
      if(jude_unit)
        return jude_unit;
      if((use_code2<12)||(use_code2>2147483647))                    		
        return ERR_OPERAND_VAL;

      //输出最大频率的合理性
      cmp_num_ok[0]=use_code[0];//频率
      cmp_num_ok[0]*=use_code[1];//频率×时间
      cmp_num_ok[1]=use_code2*909;//脉冲×909
      if(cmp_num_ok[0]>cmp_num_ok[1])
      {
        g_SD[20]= ERR_PLSR_T;
        cmp_num_ok[0]=cmp_num_ok[1];
      }
      if(cmp_num_ok[0]<100000)
      {
        g_SD[20]= ERR_PLSR_T;
        cmp_num_ok[0]=100000;
      }

      //计算运行参数的辅助量
      pulsep=cmp_num_ok[0];
      pulsep=pulsep/use_code[0];//时间
      if(pulsep < 50)
      {
        use_code[1] = 50;
        pulsep = cmp_num_ok[0];
        pulsep = (pulsep/(use_code[1]*10))+0.5;
        use_code[0] = (short)pulsep;
        use_code[0] *= 10;
      }
      else
      {
        pulsep = cmp_num_ok[0];//频率×时间
        pulsep = pulsep/use_code[1];//频率
        if(pulsep < 10)
        {
          use_code[0] =10;
          pulsep = cmp_num_ok[0];
          use_code[1] = (short)(pulsep/10);
        }
        else if(pulsep > 20000)
        {
          use_code[0] = 20000;
          pulsep = cmp_num_ok[0];
          use_code[1] = (short)(pulsep/20000);

        }
      }
      pulsep=cmp_num_ok[0];//频率×时间
      pulsep=pulsep/100000;//频率×时间/10 单位秒
      freqp=use_code[0];//频率
      freqp=freqp/10;//频率/10
      //保存输入参数
      pto_str[*(usep+19)].cmd_type.plsr.out_freq = use_code[0];
      pto_str[*(usep+19)].cmd_type.plsr.change_time = use_code[1];
      pto_str[*(usep+19)].ucode_pc = (unsigned int)usep;
      pto_str[*(usep+19)].cmd_type.plsr.out_pulse = use_code2;


      //对输出梯度的初始化；
      cmp_num_ok[0] = 0;
      if(use_code2>109)//10
      {
        for(i=0;i<9;i++)
        {
          plsr_attr[*(usep+19)].pulse_array[i] = (unsigned int)(pulsep*(i+1));
          plsr_attr[*(usep+19)].pulse_array[18-i] = plsr_attr[*(usep+19)].pulse_array[i];
          cmp_num_ok[0] += plsr_attr[*(usep+19)].pulse_array[i];
          cmp_num_ok[0] += plsr_attr[*(usep+19)].pulse_array[i];
          plsr_attr[*(usep+19)].freq_array[i] = (unsigned short)(freqp*(i+1));
          plsr_attr[*(usep+19)].freq_array[18-i] = plsr_attr[*(usep+19)].freq_array[i];
        }
        plsr_attr[*(usep+19)].freq_array[9] = use_code[0];
        plsr_attr[*(usep+19)].pulse_array[9] = use_code2-cmp_num_ok[0];
        plsr_attr[*(usep+19)].rec_num = 0;
        plsr_attr[*(usep+19)].out_freq_now = plsr_attr[*(usep+19)].freq_array[0];
        //频率输出处理
        for(i=0; i<19; i++)
        {
          plsr_attr[*(usep+19)].freq_array[i]= (unsigned int)(3000000/(float)plsr_attr[*(usep+19)].freq_array[i]+0.5);
        }
        g_plsrGrades[*(usep+19)]=18;
      }
      else if(use_code2>89)//9
      {
        for(i=0;i<8;i++)
        {
          plsr_attr[*(usep+19)].pulse_array[i] = (unsigned int)(pulsep*(i+1));
          plsr_attr[*(usep+19)].pulse_array[16-i] = plsr_attr[*(usep+19)].pulse_array[i];
          cmp_num_ok[0] += plsr_attr[*(usep+19)].pulse_array[i];
          cmp_num_ok[0] += plsr_attr[*(usep+19)].pulse_array[i];
          plsr_attr[*(usep+19)].freq_array[i] = (unsigned short)(freqp*(i+1));
          plsr_attr[*(usep+19)].freq_array[16-i] = plsr_attr[*(usep+19)].freq_array[i];
        }
        plsr_attr[*(usep+19)].freq_array[8] = (unsigned short)(freqp*9);//use_code[0];
        plsr_attr[*(usep+19)].pulse_array[8] = use_code2-cmp_num_ok[0];
        plsr_attr[*(usep+19)].rec_num = 0;
        plsr_attr[*(usep+19)].out_freq_now = plsr_attr[*(usep+19)].freq_array[0];
        //频率输出处理
        for(i=0; i<17; i++)
        {
          plsr_attr[*(usep+19)].freq_array[i]= (unsigned int)(3000000/(float)plsr_attr[*(usep+19)].freq_array[i]+0.5);
        }
        g_plsrGrades[*(usep+19)]=16;
      }
      else if(use_code2>71)//8
      {
        for(i=0;i<7;i++)
        {
          plsr_attr[*(usep+19)].pulse_array[i] = (unsigned int)(pulsep*(i+1));
          plsr_attr[*(usep+19)].pulse_array[14-i] = plsr_attr[*(usep+19)].pulse_array[i];
          cmp_num_ok[0] += plsr_attr[*(usep+19)].pulse_array[i];
          cmp_num_ok[0] += plsr_attr[*(usep+19)].pulse_array[i];
          plsr_attr[*(usep+19)].freq_array[i] = (unsigned short)(freqp*(i+1));
          plsr_attr[*(usep+19)].freq_array[14-i] = plsr_attr[*(usep+19)].freq_array[i];
        }
        plsr_attr[*(usep+19)].freq_array[7] = (unsigned short)(freqp*8);//use_code[0];
        plsr_attr[*(usep+19)].pulse_array[7] = use_code2-cmp_num_ok[0];
        plsr_attr[*(usep+19)].rec_num = 0;
        plsr_attr[*(usep+19)].out_freq_now = plsr_attr[*(usep+19)].freq_array[0];
        //频率输出处理
        for(i=0; i<15; i++)
        {
          plsr_attr[*(usep+19)].freq_array[i]= (unsigned int)(3000000/(float)plsr_attr[*(usep+19)].freq_array[i]+0.5);
        }
        g_plsrGrades[*(usep+19)]=14;
      }
      else if(use_code2>55)//7
      {
        for(i=0;i<6;i++)
        {
          plsr_attr[*(usep+19)].pulse_array[i] = (unsigned int)(pulsep*(i+1));
          plsr_attr[*(usep+19)].pulse_array[12-i] = plsr_attr[*(usep+19)].pulse_array[i];
          cmp_num_ok[0] += plsr_attr[*(usep+19)].pulse_array[i];
          cmp_num_ok[0] += plsr_attr[*(usep+19)].pulse_array[i];
          plsr_attr[*(usep+19)].freq_array[i] = (unsigned short)(freqp*(i+1));
          plsr_attr[*(usep+19)].freq_array[12-i] = plsr_attr[*(usep+19)].freq_array[i];
        }
        plsr_attr[*(usep+19)].freq_array[6] = (unsigned short)(freqp*7);//use_code[0];
        plsr_attr[*(usep+19)].pulse_array[6] = use_code2-cmp_num_ok[0];
        plsr_attr[*(usep+19)].rec_num = 0;
        plsr_attr[*(usep+19)].out_freq_now = plsr_attr[*(usep+19)].freq_array[0];
        //频率输出处理
        for(i=0; i<13; i++)
        {
          plsr_attr[*(usep+19)].freq_array[i]= (unsigned int)(3000000/(float)plsr_attr[*(usep+19)].freq_array[i]+0.5);
        }
        g_plsrGrades[*(usep+19)]=12;
      }
      else if(use_code2>41)//6
      {
        for(i=0;i<5;i++)
        {
          plsr_attr[*(usep+19)].pulse_array[i] = (unsigned int)(pulsep*(i+1));
          plsr_attr[*(usep+19)].pulse_array[10-i] = plsr_attr[*(usep+19)].pulse_array[i];
          cmp_num_ok[0] += plsr_attr[*(usep+19)].pulse_array[i];
          cmp_num_ok[0] += plsr_attr[*(usep+19)].pulse_array[i];
          plsr_attr[*(usep+19)].freq_array[i] = (unsigned short)(freqp*(i+1));
          plsr_attr[*(usep+19)].freq_array[10-i] = plsr_attr[*(usep+19)].freq_array[i];
        }
        plsr_attr[*(usep+19)].freq_array[5] = (unsigned short)(freqp*6);//use_code[0];
        plsr_attr[*(usep+19)].pulse_array[5] = use_code2-cmp_num_ok[0];
        plsr_attr[*(usep+19)].rec_num = 0;
        plsr_attr[*(usep+19)].out_freq_now = plsr_attr[*(usep+19)].freq_array[0];
        //频率输出处理
        for(i=0; i<11; i++)
        {
          plsr_attr[*(usep+19)].freq_array[i]= (unsigned int)(3000000/(float)plsr_attr[*(usep+19)].freq_array[i]+0.5);
        }
        g_plsrGrades[*(usep+19)]=10;
      }
      else if(use_code2>29)//5
      {
        for(i=0;i<4;i++)
        {
          plsr_attr[*(usep+19)].pulse_array[i] = (unsigned int)(pulsep*(i+1));
          plsr_attr[*(usep+19)].pulse_array[8-i] = plsr_attr[*(usep+19)].pulse_array[i];
          cmp_num_ok[0] += plsr_attr[*(usep+19)].pulse_array[i];
          cmp_num_ok[0] += plsr_attr[*(usep+19)].pulse_array[i];
          plsr_attr[*(usep+19)].freq_array[i] = (unsigned short)(freqp*(i+1));
          plsr_attr[*(usep+19)].freq_array[8-i] = plsr_attr[*(usep+19)].freq_array[i];
        }
        plsr_attr[*(usep+19)].freq_array[4] = (unsigned short)(freqp*5);//use_code[0];
        plsr_attr[*(usep+19)].pulse_array[4] = use_code2-cmp_num_ok[0];
        plsr_attr[*(usep+19)].rec_num = 0;
        plsr_attr[*(usep+19)].out_freq_now = plsr_attr[*(usep+19)].freq_array[0];
        //频率输出处理
        for(i=0; i<9; i++)
        {
          plsr_attr[*(usep+19)].freq_array[i]= (unsigned int)(3000000/(float)plsr_attr[*(usep+19)].freq_array[i]+0.5);
        }
        g_plsrGrades[*(usep+19)]=8;
      }
      else if(use_code2>19)//4
      {
        for(i=0;i<3;i++)
        {
          plsr_attr[*(usep+19)].pulse_array[i] = (unsigned int)(pulsep*(i+1));
          plsr_attr[*(usep+19)].pulse_array[6-i] = plsr_attr[*(usep+19)].pulse_array[i];
          cmp_num_ok[0] += plsr_attr[*(usep+19)].pulse_array[i];
          cmp_num_ok[0] += plsr_attr[*(usep+19)].pulse_array[i];
          plsr_attr[*(usep+19)].freq_array[i] = (unsigned short)(freqp*(i+1));
          plsr_attr[*(usep+19)].freq_array[6-i] = plsr_attr[*(usep+19)].freq_array[i];
        }
        plsr_attr[*(usep+19)].freq_array[3] = (unsigned short)(freqp*4);//use_code[0];
        plsr_attr[*(usep+19)].pulse_array[3] = use_code2-cmp_num_ok[0];
        plsr_attr[*(usep+19)].rec_num = 0;
        plsr_attr[*(usep+19)].out_freq_now = plsr_attr[*(usep+19)].freq_array[0];
        //频率输出处理
        for(i=0; i<7; i++)
        {
          plsr_attr[*(usep+19)].freq_array[i]= (unsigned int)(3000000/(float)plsr_attr[*(usep+19)].freq_array[i]+0.5);
        }
        g_plsrGrades[*(usep+19)]=6;
      }
      else if(use_code2>11)//3
      {
        for(i=0;i<2;i++)
        {
          plsr_attr[*(usep+19)].pulse_array[i] = (unsigned int)(pulsep*(i+1));
          plsr_attr[*(usep+19)].pulse_array[4-i] = plsr_attr[*(usep+19)].pulse_array[i];
          cmp_num_ok[0] += plsr_attr[*(usep+19)].pulse_array[i];
          cmp_num_ok[0] += plsr_attr[*(usep+19)].pulse_array[i];
          plsr_attr[*(usep+19)].freq_array[i] = (unsigned short)(freqp*(i+1));
          plsr_attr[*(usep+19)].freq_array[4-i] = plsr_attr[*(usep+19)].freq_array[i];
        }
        plsr_attr[*(usep+19)].freq_array[2] = (unsigned short)(freqp*3);//use_code[0];
        plsr_attr[*(usep+19)].pulse_array[2] = use_code2-cmp_num_ok[0];
        plsr_attr[*(usep+19)].rec_num = 0;
        plsr_attr[*(usep+19)].out_freq_now = plsr_attr[*(usep+19)].freq_array[0];
        //频率输出处理
        for(i=0; i<5; i++)
        {
          plsr_attr[*(usep+19)].freq_array[i]= (unsigned int)(3000000/(float)plsr_attr[*(usep+19)].freq_array[i]+0.5);
        }
        g_plsrGrades[*(usep+19)]=4;
      }

      //置对应的监控位为1
      g_SM[*(usep+19)+82] = 1;

      //初始化处理
      if(*(usep+19))
      {
        if(HETDOUT & 0x00000010)
          HETDOUT ^= 0x00000010;
        plsr_init_y1(1);
      }
      else
      {
        if(HETDOUT & 0x00000001)
          HETDOUT ^= 0x00000001;
        plsr_init_y0(1);
      }
    }
  }
  //能流无效处理
  else
  {
    if(pto_str[*(usep+19)].op_type == 3)
    {
      if(pto_str[*(usep+19)].ucode_pc != (unsigned int)usep)
        return RIGHT;
      //置对应的监控位为0
      g_SM[*(usep+19)+82] = 0;
      if(*(usep+19))
        plsr_init_y1(0);
      else
        plsr_init_y0(0);
    }
    if(pto_str[*(usep+19)].ucode_pc == (unsigned int)usep)
    {
      if(*(usep+19))
        g_Y[1]=0;
      else
        g_Y[0]=0;
    }
  }  //end of else
  return RIGHT;
}

/*******************************************************************
** function                :  Y001 output pulse HIGH
** function description    :  Y001 output pulse HIGH count
** input                   :  NO。
** transfer other function :
** return                  :  void
*******************************************************************/
void HET2_PWMP_Y0(void)
{
  if(cyc_union[0].cyc_num == 0)
  {
    if((HETDOUT & 0x00000001)==0)
      HETDOUT ^= 0x00000001;
    cyc_union[0].cyc_num = 127;//31;
    cyc_union[2].cyc_num = pwm_cyle[0];
  }
  else
  {
    cyc_union[0].cyc_num--;
    if(cyc_union[2].cyc_num == 0)
      return;
    else
      cyc_union[2].cyc_num--;
  }
}

/*******************************************************************
** function                :  Y001 output pulse HIGH
** function description    :  Y001 output pulse HIGH count
** input                   :  NO。
** transfer other function :
** return                  :  void
*******************************************************************/
void HET2_PWMH_Y0(void)
{
  if(cyc_union[2].cyc_num == 0)
  {
    if(HETDOUT & 0x00000001)
      HETDOUT ^= 0x00000001;
  }
}

/*******************************************************************
** function                :  Y001 output pulse manage
** function description    :  Y001 output pulse count manage
** input                   :  NO。
** transfer other function :
** return                  :  void
*******************************************************************/
void HET2_PTO_Y0(void)
{
  unsigned int use_unit;
  if(pto_str[0].count_ones == 1)
  {
    pto_str[0].count_ones = 0;
    use_unit = HET_L05_0.memory.data_word;
    use_unit >>= 5;
    g_SD50_firstref[0]=1;
  }
  else
  {
    use_unit = 0x100000;
    g_SD50_firstref[0]=0;
  }
  pto_pluse_add[0] += use_unit;    	   						            //处理Y0输出的脉冲总数
  pto_pluse_add[2] += use_unit;    	   						            //处理Y0输出的脉冲总数
  if(pto_count_flag.y0F)
  {
    if(g_SD50_firstref[0]==1)
    {
      pto_pluse_add[0] -= pto_count_now[0];
    }
    else
    {
      if (pto_count_now[0] <pulse_abs_num[0])
         {
           pto_pluse_add[0] -= (pto_count_now[0]+0x100000-pulse_abs_num[0]);
         }
      else
        pto_pluse_add[0] -= (pto_count_now[0]-pulse_abs_num[0]);
    }
    pto_count_now[0] = 0;
    pto_count_flag.y0F = 0;
  }
  if(pto_count_flag.addy0F)
  {
    pto_pluse_add[2] -= pto_count_now[2];
    pto_count_now[2] = 0;
    pto_count_flag.addy0F = 0;
  }
  *pulse_count0=pto_pluse_add[0];       					//处理Y0输出的脉冲总数
  *pulse_count2=pto_pluse_add[2];       					//处理Y1和Y0输出的脉冲总数

  if(plsy_zerof[0] == 1)
    return;

  if(plsy_zerof[0] == 0)
  {
    if(pulse_cyc_num[0] == 0)
    {
      if(pto_str[0].op_type == 1)
      {
        HET_L03_0.memory.control_word &= 0xFFEFFFFF;
        HET_L00_0.memory.data_word = 0;
        if(HETDOUT & 0x00000001)
          HETDOUT ^= 0x00000001;
        pto_str[0].out_mode=3;
        //g_SM[82]=0;
        if(g_SM[63] == 1)
          add_intr(18);

        if(g_SM[86]==1)//g_SM[86]==1)
        {
          plsy_init_y0(0);
        }


	
        if(g_pos[0].posOpType!=0)
        {
          g_SD80_reffing = 1;
/*         if(g_SD80_reffing==1)
          {
           g_SM82_reset = 1;
           g_SD50_reffing[0] = 1;  //zhao add for SD80 ref
          }
         else
          {
*/
/*          if(g_pos[0].aspect == 0)
          {
            *((int*)&g_SD[80]) -= (*(int *)(&g_SD[50])-g_pos[0].lastTotalPLS);
          }
          else
          {
            *((int*)&g_SD[80]) += (*(int *)(&g_SD[50])-g_pos[0].lastTotalPLS);
          }
          g_pos[0].lastTotalPLS=*(int *)(&g_SD[50]);
          //g_SM[82]=0;   //move by zhao
          //}
*/
          g_pos[0].posOpType=0;
          g_pos[0].PositionFrq_flag = 0;
        }


      }
      else if(pto_str[0].op_type == 3)
      {
        plsr_change_y0();
      }
    }
    else
      pulse_cyc_num[0]--;
  }


}

/*******************************************************************
** function                :  Y001 output pulse HIGH
** function description    :  Y001 output pulse HIGH count
** input                   :  NO。
** transfer other function :
** return                  :  void
*******************************************************************/
void HET2_PWMP_Y1(void)
{
  if(cyc_union[1].cyc_num == 0)
  {
    if((HETDOUT & 0x00000010) == 0)
      HETDOUT ^= 0x00000010;
    cyc_union[1].cyc_num = 127;//31;
    cyc_union[3].cyc_num = pwm_cyle[1];
  }
  else
  {
    cyc_union[1].cyc_num--;
    if(cyc_union[3].cyc_num == 0)
      return;
    else
      cyc_union[3].cyc_num--;
  }
}

/*******************************************************************
** function                :  Y001 output pulse HIGH
** function description    :  Y001 output pulse HIGH count
** input                   :  NO。
** transfer other function :
** return                  :  void
*******************************************************************/
void HET2_PWMH_Y1(void)
{
  if(cyc_union[3].cyc_num == 0)
  {
    if(HETDOUT & 0x00000010)
      HETDOUT ^= 0x00000010;
  }
}

/*******************************************************************
** function                :  Y001 output pulse manage
** function description    :  Y001 output pulse count manage
** input                   :  NO。
** transfer other function :
** return                  :  void
*******************************************************************/
void HET2_PTO_Y1(void)
{
/*  unsigned int use_unit;

  if(pto_str[1].count_ones == 1)
  {
    pto_str[1].count_ones = 0;
    use_unit = HET_L11_0.memory.data_word;
    use_unit >>= 5;
  }
  else
  {
    use_unit = 0x100000;
  }
  pto_pluse_add[1] += use_unit;    	   						            //处理Y0输出的脉冲总数
  pto_pluse_add[2] += use_unit;    	   						            //处理Y0输出的脉冲总数
  if(pto_count_flag.y1F)
  {
    pto_pluse_add[1] -= pto_count_now[1];
    pto_count_now[1] = 0;
    pto_count_flag.y1F = 0;
  }
  if(pto_count_flag.addy1F)
  {
    pto_pluse_add[2] -= pto_count_now[3];
    pto_count_now[3] = 0;
    pto_count_flag.addy1F = 0;
  }
  *pulse_count1=pto_pluse_add[1];       					//处理Y0输出的脉冲总数
  *pulse_count2=pto_pluse_add[2];       					//处理Y1和Y0输出的脉冲总数
*/
  unsigned int use_unit;
  if(pto_str[1].count_ones == 1)
  {
    pto_str[1].count_ones = 0;
    use_unit = HET_L11_0.memory.data_word;
    use_unit >>= 5;
    g_SD50_firstref[1]=1;
  }
  else
  {
    use_unit = 0x100000;
    g_SD50_firstref[1]=0;
  }
  pto_pluse_add[1] += use_unit;    	   						            //处理Y1输出的脉冲总数
  pto_pluse_add[2] += use_unit;    	   						            //处理Y0/Y1输出的脉冲总数
  if(pto_count_flag.y1F)
  {
    if(g_SD50_firstref[1]==1)
    {
      pto_pluse_add[1] -= pto_count_now[1];
    }
    else
    {
      if (pto_count_now[1] <pulse_abs_num[1])
         {
           pto_pluse_add[1] -= (pto_count_now[1]+0x100000-pulse_abs_num[1]);
         }
      else
        pto_pluse_add[1] -= (pto_count_now[1]-pulse_abs_num[1]);
    }
    pto_count_now[1] = 0;
    pto_count_flag.y1F = 0;
  }
  if(pto_count_flag.addy1F)
  {
    pto_pluse_add[2] -= pto_count_now[3];
    pto_count_now[3] = 0;
    pto_count_flag.addy1F = 0;
  }
  *pulse_count1=pto_pluse_add[1];       					//处理Y1输出的脉冲总数
  *pulse_count2=pto_pluse_add[2];       					//处理Y1和Y0输出的脉冲总数

  if(plsy_zerof[1] == 1)
    return;

  if(plsy_zerof[1] == 0)
  {
    if(pulse_cyc_num[1] == 0)
    {
      if(pto_str[1].op_type == 1)
      {
        HET_L09_0.memory.control_word &= 0xFFEFFFFF;
        HET_L06_0.memory.data_word = 0;
        if(HETDOUT & 0x00000010)
          HETDOUT ^= 0x00000010;
        pto_str[1].out_mode=3;
        //g_SM[83]=0;
        if(g_SM[64] == 1)
          add_intr(19);

        if(g_SM[87]==1)//g_SM[87]==1)
        {
          plsy_init_y1(0);
        }

	if(g_pos[1].posOpType!=0)
        {
          g_SD82_reffing = 1;
 /*        if(g_SD82_reffing==1)
          {
            g_SM83_reset = 1;
            g_SD50_reffing[1] = 1;  //zhao add for SD82 ref
          }
         else
          {
          }
          */
/*          if(g_pos[1].aspect == 0)
          {
            *((int*)&g_SD[82]) -= (*(int *)(&g_SD[52])-g_pos[1].lastTotalPLS);
          }
          else
          {
            *((int*)&g_SD[82]) += (*(int *)(&g_SD[52])-g_pos[1].lastTotalPLS);
          }
          g_pos[1].lastTotalPLS=*(int *)(&g_SD[52]);
          //g_SM[83]=0; //move by zhao
*/
          g_pos[1].posOpType=0;
          g_pos[1].PositionFrq_flag = 0;
        }
      }
      else if(pto_str[1].op_type == 3)
      {
        plsr_change_y1();
      }
    }
    else
      pulse_cyc_num[1]--;
  }

}

/************************************************************************
function: 高速输出上电初始化程序
description: 对上电需要初始化的操作处理
input   : no;
output  : no;
************************************************************************/
void pto_poweron_init(void)
{
  unsigned char *clearp;
  unsigned int i;
  HET_L00_0.memory.data_word = 0;                                                   //Y0 pulse  disable
  HET_L05_0.memory.control_word &= 0xFFFFFFFE;
  HET_L06_0.memory.data_word = 0;                                                     //Y1 pulse  disable
  HET_L11_0.memory.control_word &= 0xFFFFFFFE;
  //脉冲输出的结构的处理
  clearp=(unsigned char *)&pto_str[0];    		//清除Y0对应的数据结构
  for(i=0;i<sizeof(pto_str[0]);i++)
    *clearp++=0;
  clearp=(unsigned char *)&pto_str[1];    		//清除Y1对应的数据结构
  for(i=0;i<sizeof(pto_str[1]);i++)
    *clearp++=0;
  clearp=(unsigned char *)&plsr_attr[0];    		//清除Y0对应的PLSR数据结构
  for(i=0;i<sizeof(plsr_attr[0]);i++)
    *clearp++=0;
  clearp=(unsigned char *)&plsr_attr[1];    		//清除Y1对应的PLSR数据结构
  for(i=0;i<sizeof(plsr_attr[1]);i++)
    *clearp++=0;

  pulse_count0 = (unsigned int *)&g_SD[50];
  pulse_count1 = (unsigned int *)&g_SD[52];
  pulse_count2 = (unsigned int *)&g_SD[54];

  pwm_cyle[0] = 0;
  pwm_cyle[1] = 0;

  //特殊脉冲输出处理单元值初始化
  pto_count_now[0]=0;
  pto_count_now[1]=0;
  pto_count_now[2]=0;
  pto_count_now[3]=0;
}

/************************************************************************
function: 高速输出停机处理程序
description: 对上电需要初始化的操作处理
input   : no;
output  : no;
************************************************************************/
void pto_switchoff_init(void)
{
  unsigned char *clearp;
  unsigned int i;
  unsigned int use_unit;
  HET_L00_0.memory.data_word = 0;                                                   //Y0 pulse  disable
  HET_L05_0.memory.control_word &= 0xFFFFFFFE;
  HET_L06_0.memory.data_word = 0;                                                     //Y1 pulse  disable
  HET_L11_0.memory.control_word &= 0xFFFFFFFE;
  if((pto_str[0].op_type == 1)||(pto_str[0].op_type == 3))
  {
    use_unit = HET_L04_0.memory.data_word;
    use_unit >>= 5;
    if(pto_str[0].count_ones != 1)
    {
      use_unit -= pulse_abs_num[0];
      use_unit &= 0xFFFFF;
    }
    *pulse_count0 = pto_pluse_add[0]+use_unit;    	   						//处理Y0输出的脉冲总数
    *pulse_count2 = pto_pluse_add[2]+use_unit;       					    //处理Y1和Y0输出的脉冲总数
    if(pto_count_flag.y0F)
      *pulse_count0 -= pto_count_now[0];
    if(pto_count_flag.addy0F)
      *pulse_count2 -= pto_count_now[2];
    pto_pluse_add[0] = *pulse_count0;    	   						//处理Y0输出的脉冲总数
    pto_pluse_add[2] = *pulse_count2;       					    //处理Y1和Y0输出的脉冲总数
    HET_L04_0.memory.data_word = 0;
  }
  if((pto_str[1].op_type == 1)||(pto_str[1].op_type == 3))
  {
    use_unit = HET_L10_0.memory.data_word;
    use_unit >>= 5;
    if(pto_str[1].count_ones != 1)
    {
      use_unit -= pulse_abs_num[1];
      use_unit &= 0xFFFFF;
    }
    *pulse_count1 = pto_pluse_add[1]+use_unit;    	   					    //处理Y0输出的脉冲总数
    *pulse_count2 = pto_pluse_add[2]+use_unit;       					    //处理Y1和Y0输出的脉冲总数
    if(pto_count_flag.y0F)
      *pulse_count1-=pto_count_now[1];
    if(pto_count_flag.addy0F)
      *pulse_count2-=pto_count_now[3];
    pto_pluse_add[1] = *pulse_count1;    	   						//处理Y0输出的脉冲总数
    pto_pluse_add[2] = *pulse_count2;       					    //处理Y1和Y0输出的脉冲总数
    HET_L10_0.memory.data_word = 0;
  }
  clearp=(unsigned char *)&pto_str[0];    			//清除Y0对应的数据结构
  for(i=0;i<sizeof(pto_str[0]);i++)
    *clearp++=0;
  clearp=(unsigned char *)&plsr_attr[0];    			//清除Y0对应的PLSR数据结构
  for(i=0;i<sizeof(plsr_attr[0]);i++)
    *clearp++=0;
  clearp=(unsigned char *)&pto_str[1];    	//清除Y1对应的数据结构
  for(i=0;i<sizeof(pto_str[1]);i++)
    *clearp++=0;
  clearp=(unsigned char *)&plsr_attr[1];    	//清除Y1对应的PLSR数据结构
  for(i=0;i<sizeof(plsr_attr[1]);i++)
    *clearp++=0;

  //特殊脉冲输出处理单元值初始化
  pto_count_now[0]=0;
  pto_count_now[1]=0;
  pto_count_now[2]=0;
  pto_count_now[3]=0;
}

/************************************************************************
function: 处理SD写入时的脉冲个数
description:
input   : no;
output  : no;
************************************************************************/
void save_pto_dy0(void)
{
  __disable_interrupt();                               // Enable interrupts
  pto_pluse_add[0]=*pulse_count0;	
  if((pto_str[0].op_type == 1)||(pto_str[0].op_type == 3))
  {
    if((pto_str[0].out_mode == 1)||(pto_str[0].out_mode == 2))
    {
      pto_count_now[0]=HET_L04_0.memory.data_word;
      pto_count_now[0] >>= 5;
      pto_count_flag.y0F=1;
    }
  }
  __enable_interrupt();                               // Enable interrupts
}

/************************************************************************
function: 处理SD写入时的脉冲个数
description:
input   : no;
output  : no;
************************************************************************/
void save_pto_dy1(void)
{
  __disable_interrupt();                               // Enable interrupts
  pto_pluse_add[1]=*pulse_count1;	
  if((pto_str[1].op_type == 1)||(pto_str[1].op_type == 3))
  {
    if((pto_str[1].out_mode == 1)||(pto_str[1].out_mode == 2))
    {
      pto_count_now[1]=HET_L10_0.memory.data_word;
      pto_count_now[1] >>= 5;
      pto_count_flag.y1F=1;
    }
  }
  __enable_interrupt();                               // Enable interrupts
}
/************************************************************************
function: 处理SD写入时的脉冲个数
description:
input   : no;
output  : no;
************************************************************************/
void save_pto_dy0y1(void)
{
  __enable_interrupt();                               // Enable interrupts
  pto_pluse_add[2]=*pulse_count2;	
  if((pto_str[0].op_type == 1)||(pto_str[0].op_type == 3))
  {
    if((pto_str[0].out_mode == 1)||(pto_str[0].out_mode == 2))
    {
      pto_count_now[2]=HET_L04_0.memory.data_word;
      pto_count_now[2] >>= 5;
      pto_count_flag.addy0F=1;
    }
  }
  if((pto_str[1].op_type == 1)||(pto_str[1].op_type == 3))
  {
    if((pto_str[1].out_mode == 1)||(pto_str[1].out_mode == 2))
    {
      pto_count_now[3]=HET_L10_0.memory.data_word;
      pto_count_now[3] >>= 5;
      pto_count_flag.addy1F=1;
    }
  }
  __enable_interrupt();                               // Enable interrupts
}

/************************************************************************
function: 处理高速输出的状态灯
description:
input   : no;
output  : no;
************************************************************************/
void ref_y0y1_led(void)
{
 	if((pto_str[0].op_type==1)||(pto_str[0].op_type==2)||(pto_str[0].op_type==3))
        {
          if(Y0_RD)
            g_Y[0] = 1;
          else
            g_Y[0] = 0;
        }
        if((pto_str[1].op_type==1)||(pto_str[1].op_type==2)||(pto_str[1].op_type==3))
        {
          if(Y1_RD)
            g_Y[1] = 1;
          else
            g_Y[1] = 0;
        }
}




/************************************************************************
function: PLSB带基底速度加减脉冲输出指令;
description:对Y端口输出操作; PLSB 基底速度＋2 最高速度＋6 总脉冲数＋10 加减速时间＋16 输出端口 ＋20 total＋24
input   : EXC_ENV结构的变量指针;
output  : no;
************************************************************************/
unsigned char f_CI_PLSB(EXC_ENV *exc_env)
{

  unsigned char *usep,jude_unit,j,delta,protect;
  unsigned int  cmp_num_ok[2],cmp;
  unsigned short startSpeed,endSpeed,accTime;
  unsigned int outPulse;
  unsigned short Y;//输出端口
  unsigned char segments=60;

  usep=(*exc_env).pc_ucode;
  Y=*(usep+23);

  //能流有效处理
  if(PF(exc_env) && (g_SM[Y+80]==0))
  {
    if(pto_str[Y].out_mode==3)
    {
      g_Y[Y]=0;
      g_SM[Y+82] = 0;
      return RIGHT;
    }
    //判断指令是否在执行
    if(pto_str[Y].op_type==3)
    {
      //判断是否为本指令
      if(pto_str[Y].ucode_pc!=(unsigned int)usep)
        return RIGHT;
      //输出脉冲数量计算
      if(Y)
        plsr_init_y1(3);
      else
        plsr_init_y0(3);
    }
    //判断Y[Y]没有输出指令
    else if(pto_str[Y].op_type==0)
    {
      //取基底速度并进行有效性判断
      jude_unit=get_word(usep+2,&startSpeed,0,1);
      if(jude_unit)
        return jude_unit;
      //取最高速度并进行有效性判断
      jude_unit=get_word(usep+6,&endSpeed,0,1);
      if(jude_unit)
        return jude_unit;
      //取总脉冲数并进行有效性判断
      jude_unit=get_dword(usep+10,&outPulse,0,1);
      if(jude_unit)
        return jude_unit;
      //取加减速时间并进行有效性判断
      jude_unit=get_word(usep+16,&accTime,0,1);
      if(jude_unit)
        return jude_unit;

      cmp_num_ok[0] = 0;
      delta=58;

      if(startSpeed > 20000 || startSpeed > endSpeed)
        return ERR_OPERAND_VAL;
      if(endSpeed > 20000 || endSpeed < 10)
        return ERR_OPERAND_VAL;
      if(accTime > 10000)
        return ERR_OPERAND_VAL;
      if((outPulse<5)||outPulse >999999)
        return ERR_OPERAND_VAL;


      //dynamic number of segments
      cmp=startSpeed*accTime;
      if(cmp>60000)
        segments=60;
      else if(cmp>40000)
        segments=40;
      else if(cmp>20000)
        segments=20;
      else if(cmp>10000)
        segments=10;
      else if(cmp>5000)
        segments=5;
      else
        segments=4;

      delta=segments-2;

      for(j=0;j<segments-1;j++)
      {
        plsr_attr[Y].freq_array[j] = (unsigned short)(startSpeed+(endSpeed-startSpeed)*(j+1)/segments);
        plsr_attr[Y].pulse_array[j] = (unsigned int)(plsr_attr[Y].freq_array[j]*accTime/(segments*1000));
        if(plsr_attr[Y].pulse_array[j]<1 || plsr_attr[Y].pulse_array[j]>outPulse)
        {
          plsr_attr[Y].pulse_array[j]=1;
          delta=123;
        }
        cmp_num_ok[0] += plsr_attr[Y].pulse_array[j];
        cmp_num_ok[0] += plsr_attr[Y].pulse_array[j];
        plsr_attr[Y].freq_array[j] = (unsigned int)(3000000/(float)plsr_attr[Y].freq_array[j]+0.5);

        protect=j;

        //delta check
        if((cmp_num_ok[0]+1)*2>outPulse||delta==123)
        {
          delta=j;
          j=segments-2;
        }
      }

      //mirror copy
      for(j=0;j<(delta+1);j++)
      {
        plsr_attr[Y].pulse_array[(delta+1)*2-j]=plsr_attr[Y].pulse_array[j];
        plsr_attr[Y].freq_array[(delta+1)*2-j] = plsr_attr[Y].freq_array[j];
      }

      plsr_attr[Y].freq_array[delta+1] = (unsigned short)(startSpeed+(endSpeed-startSpeed)*(protect+2)/segments);
      plsr_attr[Y].freq_array[delta+1] = (unsigned int)(3000000/(float)plsr_attr[Y].freq_array[delta+1]+0.5);
      plsr_attr[Y].pulse_array[delta+1] = outPulse-cmp_num_ok[0];
      if(plsr_attr[Y].pulse_array[delta+1]<1 || plsr_attr[Y].pulse_array[delta+1]>outPulse)
      {
        plsr_attr[Y].pulse_array[delta+1]=1;
      }

      g_plsrGrades[Y]=(delta+1)*2;



      plsr_attr[Y].out_freq_now = plsr_attr[Y].freq_array[0];
      plsr_attr[Y].rec_num = 0;
      g_SM[Y+82] = 1;
      pto_str[Y].ucode_pc = (unsigned int)usep;

      //初始化处理
      if(Y)
      {
        if(HETDOUT & 0x00000010)
          HETDOUT ^= 0x00000010;
        plsr_init_y1(1);
      }
      else
      {
        if(HETDOUT & 0x00000001)
          HETDOUT ^= 0x00000001;
        plsr_init_y0(1);
      }
    }
  }
  //能流无效处理
  else
  {
    if(pto_str[Y].op_type == 3)
    {
      if(pto_str[Y].ucode_pc != (unsigned int)usep)
        return RIGHT;
      //置对应的监控位为0
      g_SM[Y+82] = 0;
      if(Y)
        plsr_init_y1(0);
      else
        plsr_init_y0(0);
    }
    if(pto_str[Y].ucode_pc == (unsigned int)usep)
    {
      if(Y)
        g_Y[1]=0;
      else
        g_Y[0]=0;
    }
  }  //end of else
  return RIGHT;
}

