
#ifndef _PTO_H
#define _PTO_H
/////////////////////////////////////////////////////////////////////////

/*************PWM和PLSY指令的结构*************/
struct{
       unsigned char op_type;         //1:对应DPLSY输出；2:对应PWM输出
	 			                      //3:对应DPLSR输出；0:没指令操作
	   unsigned char out_mode;        //1:输出比较模式；2:输出IO翻转模式;0:输出停止模式
					                  //3:输出完成模式；
	   unsigned int ucode_pc;         //保存高速输出指令的地址
       unsigned char count_ones;      //PLSY和PLSR第一次计数
	   union{
			 struct{
			        unsigned short    out_freq;                 //输出最大的频率数据
			        unsigned short    change_time;              //加减速时间(4us为单位)；
			        unsigned int      out_pulse;                //要输出的脉冲总数
		     }plsr;

		     struct{
			        unsigned int      out_freq;                 //输出频率
			        unsigned int      out_pulse;                //输出频率数据
		     }plsy;
		
			 struct{
			        unsigned short   out_freq;                  //输出周期
					unsigned short   out_breadth;               //输出脉冲幅值
                    unsigned short   sm84;                      //时基
		     }pwm;
   	 }cmd_type;
}pto_str[2];              //pto_pwm_str[0]对应Y0输出；pto_pwm_str[1]对应Y1输出；

/****************PLSR指令的结构****************/
struct{
	   unsigned char        rec_num;                  //当前的记录号
	   unsigned int         freq_array[121];           //输出频率数组	
       unsigned int         pulse_array[121];          //当前输出频率数据
	   unsigned short       out_freq_now;			    //正要输出的脉冲频率	
}plsr_attr[2];            //PLSR指令的属性结构；0：对应Y0；1：对应Y1；

struct{
	   unsigned char        cyc_num;
       unsigned int         cmp_num;
}cyc_union[4];                          //在IO比较输出的比较设定点；cyc_union[0]对应TGRA的Y0输出
									    //cyc_union[1]对应TGRA的Y1输出;cyc_union[2]对应TGRB的Y0输出
									    //cyc_union[3]对应TGRB的Y1输出
unsigned char  pwm_cyle[2];             //PWM—IO翻转的标志
unsigned short pulse_cyc_num[2];
unsigned int   pulse_abs_num[2];
unsigned char  plsy_zerof[2];

//unsigned int  pto_io_cyc[4];            //做P20和P10翻转的数组保存

//extern function define;
extern unsigned char get_word(unsigned char *ptr, unsigned short *dtr, unsigned short idisp, unsigned short len);
extern unsigned char get_dword(unsigned char *ptr, unsigned int *dtr, unsigned short idisp, unsigned short len);
extern unsigned char add_intr(unsigned char num);

#endif



