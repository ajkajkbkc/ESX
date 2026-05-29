
#ifndef _HIGH_COUNT_H
#define _HIGH_COUNT_H

/////////////////////////////////////////////////////////////////////////
/*高速计数器设计的变量定义*/
/////////////////////////////////////////////////////////////////////////
/***************所有高速计数器******************/
extern unsigned char hccmdall; //用户程序中描述高速指令的数量；
extern struct
{
    unsigned char counterno;                            //当前在使用高速计数属性的高速计数器号
    unsigned char hcno;
    int           out_cmp_num;                          //触点输出的比较点
    union
    {
        unsigned char    BYTE;
        struct
        {
            unsigned char initF:  1;             //高速计数器硬件已初始化标志；
            unsigned char outhcF: 1;             //高速计数器OUT指令有效标志
            unsigned char dhszF:  1;             //高速计数器DHSZ命令初次加入标志。
            unsigned char :       5;
        } BIT;
    } bit_use;
} use_hc_attr[6];

extern unsigned char hc_cmdnum[6];                                  //对本计数器使用高速指令的数据
extern unsigned char hc_array[6][6];                                //用来存放该计数器的相关指令

//高速指令比较缓冲单元
extern struct
{
    unsigned char op_mode;                              //0:对应除DHSCS,DHSCR指令;1:对应DHSZ的1段;2:对应DHSZ的2段;3:对应DHSZ的3段；
    //4:对应除DHST
    unsigned char op_act;                               //对应高速指令的处理;
    unsigned char table_num;			                //表格比较的记录号
} hc_buf[19];

//高速指令动作缓冲的指针
extern unsigned char  start_hcp;                 //指令开始的指针
extern unsigned char  end_hcp;                   //指令结束的指针
//描述DHST或DHSP内表格结构的变量
extern struct
{
    unsigned char   cmdno;                        //高速指令在用户程序中的号
    unsigned char   curpoint;                     //当前操作的表格记录号；
    unsigned char   pointall;                     //表格总的记录数；
    unsigned short  startaddr;                    //存储表格的起始地址；
} hc_cmd_table;

//#pragma section EXSRAM_VAR             //片外RAM定义，段起始地址为0x0200000
//高速指令的数据备份结构
extern struct cmd_copy_str
{
    unsigned char cmdno;                         //高速指令在用户程序中的序号。
    unsigned char cmdtype;                       //高速指令的具体类型01：DHSCS；02：DHSCR；03：DHSZ；04：DHST；05：DHSP。
    unsigned char hcno;                          //操作的高速计数器号(0-19)。
    unsigned char opstate;                       //为0：指令没执行；
    union
    {
        struct
        {
            int cmppoint1;                  //比较数据1
            int cmppoint2;                  //比较数据2
        } cmp;
        struct
        {
            int cmppoint;                  //DHSCS指令要比较的long型的数据
            unsigned char optype;           //DHSCS指令要操作的类型01：Y元件；02：M元件；03：S元件；04：保留；05：中断标号。
            unsigned short  opnum;            //操作软元件的序号。
        } dhscs;                               //DHSCS操作数据存储。
        struct
        {
            int cmppoint;                  //DHSCR指令要比较的long型的数据
            unsigned char optype;           //DHSCR指令要操作的类型01：Y元件；02：M元件；03：S元件；04：保留；05：高速计数器本身。
            unsigned short  opnum;            //操作软元件的序号。
        } dhscr;                               //DHSCR操作数据存储。
        struct
        {
            int cmppoint1;                 //DHSZ指令要比较的long型的数据
            int cmppoint2;                 //DHSZ指令要比较的long型的数据
            unsigned char optype;           //DHSZ指令要操作的类型01：Y元件；02：M元件；03：S元件；04：保留；。
            unsigned short  opnum;            //操作软元件的序号。
        } dhsz;                  				 //DHSZ操作数据存储。
        struct
        {
            int cmppoint;                  //DHSZT指令要比较的long型的数据
            unsigned short opnum;             //Y软元件的操作号
            unsigned short opdo;              //Y软元件的操作0：复位；1：置位。
        } dhst;                                //DHST操作数据存储。
        struct
        {
            int cmppoint;                  //DHSP指令要比较的long型的数据
            int movnum;                    //DHSP指令要传送到D8132（D8133）中
        } dhsp;                                //DHSP操作数据存储。
    } cmptype;
} hc_cmd_copy[6];

/////////////////////////////////////////////////////////////////////////
/*SPD设计的变量定义*/
/////////////////////////////////////////////////////////////////////////
extern union
{
    unsigned char BYTE;
    struct
    {
        unsigned char spd_x0F: 1;       //对X000的测量脉冲密度；
        unsigned char spd_x1F: 1;       //对X001的测量脉冲密度；
        unsigned char spd_x2F: 1;       //对X002的测量脉冲密度；
        unsigned char spd_x3F: 1;       //对X003的测量脉冲密度；
        unsigned char spd_x4F: 1;       //对X004的测量脉冲密度；
        unsigned char spd_x5F: 1;       //对X005的测量脉冲密度；
        unsigned char        : 2;       //保留；
    } BIT;
} spd_flag;
extern unsigned short           *spd_count[6];                   //保存脉冲密度计数值；
extern unsigned int    spd_time_save[6];                //SPD要测试的时间；
extern unsigned int    spd_time_savet[6];               //SPD already finished time；
extern unsigned int    spd_time_rec[6];                 //保存SPD启动的时间
//extern unsigned short  *get_word_addr;                  //在取地址函数中的全局变量

/////////////////////////////////////////////////////////////////////////
/*外部中断设计的变量定义*/
/////////////////////////////////////////////////////////////////////////
extern union
{
    unsigned char BYTE;
    struct
    {
        unsigned char intr_x0F: 1;       //外部中断0已初始化标志；
        unsigned char intr_x1F: 1;       //外部中断1已初始化标志；
        unsigned char intr_x2F: 1;       //外部中断2已初始化标志；
        unsigned char intr_x3F: 1;       //外部中断3已初始化标志；
        unsigned char intr_x4F: 1;       //外部中断4已初始化标志；
        unsigned char intr_x5F: 1;       //外部中断5已初始化标志；
        unsigned char intr_x6F: 1;       //外部中断6已初始化标志；
        unsigned char intr_x7F: 1;       //外部中断7已初始化标志；
    } BIT;
} intr_flag;
extern unsigned char intr_num_save[8];             //要保存的中断号；

/////////////////////////////////////////////////////////////////////////
/*脉冲捕捉禁止标志*/
/////////////////////////////////////////////////////////////////////////
extern union
{
    unsigned char BYTE;
    struct
    {
        unsigned char catch_x0F: 1;       //脉冲捕捉中断0已禁止标志；
        unsigned char catch_x1F: 1;       //脉冲捕捉中断1已禁止标志；
        unsigned char catch_x2F: 1;       //脉冲捕捉中断2已禁止标志；
        unsigned char catch_x3F: 1;       //脉冲捕捉中断3已禁止标志；
        unsigned char catch_x4F: 1;       //脉冲捕捉中断4已禁止标志；
        unsigned char catch_x5F: 1;       //脉冲捕捉中断5已禁止标志；
        unsigned char catch_x6F: 1;       //脉冲捕捉中断6已禁止标志；
        unsigned char catch_x7F: 1;       //脉冲捕捉中断7已禁止标志；
    } BIT;
} catch_flag;

struct HOUR_ATTR
{
    unsigned char 	up_state;	   //上次的状态
    unsigned int  	up_second;     //上次的时间
};
extern struct HOUR_ATTR  hour_str[256];


//extern unsigned char get_word(unsigned char *ptr, unsigned short *dtr, unsigned short idisp, unsigned short len);
//extern unsigned char get_dword(unsigned char *ptr, unsigned int *dtr, unsigned short idisp, unsigned short len);
extern unsigned char add_intr(unsigned char num);
extern void hccmd_output(unsigned char yValue, unsigned char on_off);

extern void highcount_poweron_deinit(void);
extern void highcount_poweron_init(void);
//extern unsigned char f_CI_HCNT(plc_run_power_flow_st *ltp_RunEnv);

#endif

