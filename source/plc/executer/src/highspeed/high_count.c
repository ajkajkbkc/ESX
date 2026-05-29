
#include <stdio.h>
#include <string.h>
#include "high_count.h"
#include "bsp_gpio.h"
#include "fsl_debug_console.h"
#include "plc_element.h"
#include "plc_parseaddr.h"
#include "kalyke_opts.h"

static const char *TAG = "high_count";
static const bool logFlag = false;
unsigned char   hc_outF;                 //为1:标志成立对中断输出的处理；为0：不处理

/*========================================================================
                     high_counter hardware drive
=========================================================================*/
/************************************************************************
function:     The counters from X000 input are drived ;
description:  The function drive or cancel the counters's hardware;
input:        counter_nonum: opposite counter serial number;
              init_onoff: 1:drive the counter's hardware
                          2:cancel the counter's hardware
output:       no;
************************************************************************/
void hc_X0_init(int counter_no, unsigned char init_onoff)
{
    unsigned int use_unit;
    if(init_onoff == 1)                 					              //init hardware
    {
        //GIOPOL1 |= 0x08;                                                //X000 rising confige
        //GIOENA1 |= 0x08;                                                //enable X0 rise input interrupt
        bsp_kalyke_enable_X_interrupt(0, kGPIO_IntRisingEdge);
        if(counter_no == HCOUNTER244)
        {
            //GIOPOL1 |= 0x10;                                            //X002 rising confige
            //GIOENA1 |= 0x10;                                            //enable X2 rise input interrupt
            bsp_kalyke_enable_X_interrupt(2, kGPIO_IntRisingEdge);
        }
        else if(counter_no == HCOUNTER246)
        {
            //GIOPOL1 |= 0x10;                                            //X002 rising confige
            //GIOENA1 |= 0x10;                                            //enable X2 rise input interrupt
            bsp_kalyke_enable_X_interrupt(2, kGPIO_IntRisingEdge);
            //use_unit = HET_L12_0.memory.control_word & 0xFFFFFF8E;
            //use_unit |= 0x31;
            //HET_L12_0.memory.control_word = use_unit;                   //enable X6 rise and fall input interrupt
            bsp_kalyke_enable_X_interrupt(6, kGPIO_IntRisingOrFallingEdge);
        }
    }
    else if(init_onoff == 0)                        			        //cancel hardware
    {
        //GIOENA1 &= 0xFFFFFFF7;                                          //disable X0 rise input interrupt
        bsp_kalyke_disable_X_interrupt(0);
        if(counter_no == HCOUNTER244)
        {
            //GIOENA1 &= 0xFFFFFFEF;                                      //disable X2 rise input interrupt
            bsp_kalyke_disable_X_interrupt(2);
        }
        else if(counter_no == HCOUNTER246)
        {
            //GIOENA1 &= 0xFFFFFFEF;                                      //disable X2 rise input interrupt
            //HET_L12_0.memory.control_word &= 0xFFFFFFFE;                //disable X6 rise and fall input interrupt
            bsp_kalyke_disable_X_interrupt(2);
            bsp_kalyke_disable_X_interrupt(6);
        }
    }
}

/************************************************************************
function:     The counters from X001 input are drived ;
description:  The function drive or cancel the counters's hardware;
input:        init_onoff: 1:drive the counter's hardware
                          2:cancel the counter's hardware
output:       no;
************************************************************************/
void hc_X1_init(unsigned char init_onoff)
{
    unsigned int use_unit;
    if(init_onoff == 1) // Enable X1 rise input interrupt
    {
        bsp_kalyke_enable_X_interrupt(1, kGPIO_IntRisingEdge);
    }
    else if(init_onoff == 0) //Disable X1 rise input interrupt
    {
        bsp_kalyke_disable_X_interrupt(1);
    }
}

/************************************************************************
function:     The counters from X002 input are drived ;
description:  The function drive or cancel the counters's hardware;
input:        init_onoff: 1:drive the counter's hardware
                          2:cancel the counter's hardware
output:       no;
************************************************************************/
void hc_X2_init(unsigned char init_onoff)
{
    if(init_onoff == 1)                 					              //init hardware
    {
        //GIOPOL1 |= 0x10;                                                //X002 rising confige
        //GIOENA1 |= 0x10;                                                //enable X2 rise input interrupt
        bsp_kalyke_enable_X_interrupt(2, kGPIO_IntRisingEdge);
    }
    else if(init_onoff == 0)                        		              //cancel hardware
    {
        //GIOENA1 &= 0xFFFFFFEF;                                          //disable X2 rise input interrupt
        bsp_kalyke_disable_X_interrupt(2);
    }
}

/************************************************************************
function:     The counters from X003 input are drived ;
description:  The function drive or cancel the counters's hardware;
input:        counter_nonum: opposite counter serial number;
              init_onoff: 1:drive the counter's hardware
                          2:cancel the counter's hardware
output:       no;
************************************************************************/
void hc_X3_init(int counter_no, unsigned char init_onoff)
{
    unsigned int use_unit;
    if(init_onoff == 1)                 					              //init hardware
    {
        //GIOPOL1 |= 0x40;                                                //X003 rising confige
        //GIOENA1 |= 0x40;                                                //enable X003 rise input interrupt
        bsp_kalyke_enable_X_interrupt(3, kGPIO_IntRisingEdge);
        if(counter_no == HCOUNTER245)
        {
            //GIOPOL1 |= 0x20;                                            //X005 rising confige
            //GIOENA1 |= 0x20;                                            //enable X5 rise input interrupt
            bsp_kalyke_enable_X_interrupt(5, kGPIO_IntRisingEdge);
        }
        else if(counter_no == HCOUNTER247)
        {
            //GIOPOL1 |= 0x20;                                            //X005 rising confige
            //GIOENA1 |= 0x20;                                            //enable X005 rise input interrupt
            bsp_kalyke_enable_X_interrupt(5, kGPIO_IntRisingEdge);
            //use_unit = HET_L13_0.memory.control_word & 0xFFFFFF8E;
            //use_unit |= 0x31;
            //HET_L13_0.memory.control_word = use_unit;                   //enable X007 rise and fall input interrupt
            bsp_kalyke_enable_X_interrupt(7, kGPIO_IntRisingOrFallingEdge);
        }
    }
    else if(init_onoff == 0)                        			          //cancel hardware
    {
        //GIOENA1 &= 0xFFFFFFBF;                                          //disable X003 rise input interrupt
        bsp_kalyke_disable_X_interrupt(3);
        if(counter_no == HCOUNTER245)
        {
            //GIOENA1 &= 0xFFFFFFDF;                                      //disable X005 rise input interrupt
            bsp_kalyke_disable_X_interrupt(5);
        }
        else if(counter_no == HCOUNTER247)
        {
            //GIOENA1 &= 0xFFFFFFDF;                                      //disable X005 rise input interrupt
            //HET_L13_0.memory.control_word &= 0xFFFFFFFE;                //disable X007 rise and fall input interrupt
            bsp_kalyke_disable_X_interrupt(5);
            bsp_kalyke_disable_X_interrupt(7);
        }
    }
}

/************************************************************************
function:     The counters from X004 input are drived ;
description:  The function drive or cancel the counters's hardware;
input:        init_onoff: 1:drive the counter's hardware
                          2:cancel the counter's hardware
output:       no;
************************************************************************/
void hc_X4_init(unsigned char init_onoff)
{
    unsigned int use_unit;
    if(init_onoff == 1)                 					              //init hardware
    {
        //use_unit = HET_L15_0.memory.control_word & 0xFFFFFF8E;
        //use_unit |= 0x21;
        //HET_L15_0.memory.control_word = use_unit;                       //enable X4 rise input interrupt
        bsp_kalyke_enable_X_interrupt(4, kGPIO_IntRisingEdge);
    }
    else if(init_onoff == 0)                        		              //cancel hardware
    {
        //HET_L15_0.memory.control_word &= 0xFFFFFFFE;                    //disable X4 rise input interrupt
        bsp_kalyke_disable_X_interrupt(4);
    }
}

/************************************************************************
function:     The counters from X005 input are drived ;
description:  The function drive or cancel the counters's hardware;
input:        init_onoff: 1:drive the counter's hardware
                          2:cancel the counter's hardware
output:       no;
************************************************************************/
void hc_X5_init(unsigned char init_onoff)
{
    if(init_onoff == 1)                 					              //init hardware
    {
        //GIOPOL1 |= 0x20;                                                //X005 rising confige
        //GIOENA1 |= 0x20;                                                //enable X5 rise input interrupt
        bsp_kalyke_enable_X_interrupt(5, kGPIO_IntRisingEdge);
    }
    else if(init_onoff == 0)                        		              //cancel hardware
    {
        //GIOENA1 &= 0xFFFFFFDF;                                          //disable X5 rise input interrupt
        bsp_kalyke_disable_X_interrupt(5);
    }
}

/************************************************************************
function:     The counters from X000 and x001 input are drived ;
description:  The function drive or cancel the counters's hardware;
input:        counter_nonum: opposite counter serial number;
              init_onoff: 1:drive the counter's hardware
                          2:cancel the counter's hardware
output:       no;
************************************************************************/
void hc_X0X1_init(int counter_no, unsigned char init_onoff)
{
    unsigned int use_unit;
    if(init_onoff == 1)                 					              //init hardware
    {
        //GIOPOL1 |= 0x08;                                                //X000 rising confige
        //GIOENA1 |= 0x08;                                                //enable X0 rise input interrupt
        bsp_kalyke_enable_X_interrupt(0, kGPIO_IntRisingEdge);
        //use_unit = HET_L14_0.memory.control_word & 0xFFFFFF8E;
        //use_unit |= 0x21;
        //HET_L14_0.memory.control_word = use_unit;                       //enable X1 rise input interrupt
        bsp_kalyke_enable_X_interrupt(1, kGPIO_IntRisingEdge);
        if(counter_no == 247)
        {
            //GIOPOL1 |= 0x10;                                            //X002 rising confige
            //GIOENA1 |= 0x10;                                            //enable X2 rise input interrupt
            bsp_kalyke_enable_X_interrupt(2, kGPIO_IntRisingEdge);
        }
        else if(counter_no == 249)
        {
            //GIOPOL1 |= 0x10;                                            //X002 rising confige
            //GIOENA1 |= 0x10;                                            //enable X2 rise input interrupt
            bsp_kalyke_enable_X_interrupt(2, kGPIO_IntRisingEdge);
            //use_unit = HET_L12_0.memory.control_word & 0xFFFFFF8E;
            //use_unit |= 0x31;
            //HET_L12_0.memory.control_word = use_unit;                   //enable X6 rise and fall input interrupt
            bsp_kalyke_enable_X_interrupt(6, kGPIO_IntRisingOrFallingEdge);
        }
    }
    else if(init_onoff == 0)                        			        //cancel hardware
    {
        //GIOENA1 &= 0xFFFFFFF7;
        //HET_L14_0.memory.control_word &= 0xFFFFFFFE;
        bsp_kalyke_disable_X_interrupt(0);
        bsp_kalyke_disable_X_interrupt(1);

        if(counter_no == 247)
        {
            //GIOENA1 &= 0xFFFFFFEF;                                      //disable X2 rise input interrupt
            bsp_kalyke_disable_X_interrupt(2);
        }
        else if(counter_no == 249)
        {
            //GIOENA1 &= 0xFFFFFFEF;                                      //disable X2 rise input interrupt
            //HET_L12_0.memory.control_word &= 0xFFFFFFFE;                //disable X6 rise and fall input interrupt
            bsp_kalyke_disable_X_interrupt(2);
            bsp_kalyke_disable_X_interrupt(6);
        }
    }
}

/************************************************************************
function:     The counters from X003 and x004 input are drived ;
description:  The function drive or cancel the counters's hardware;
input:        counter_nonum: opposite counter serial number;
              init_onoff: 1:drive the counter's hardware
                          2:cancel the counter's hardware
output:       no;
************************************************************************/
void hc_X3X4_init(int counter_no, unsigned char init_onoff)
{
    unsigned int use_unit;
    if(init_onoff == 1)                 					              //init hardware
    {
        //GIOPOL1 |= 0x40;                                                //X003 rising confige
        //GIOENA1 |= 0x40;                                                //enable X003 rise input interrupt
        bsp_kalyke_enable_X_interrupt(3, kGPIO_IntRisingEdge);
        //use_unit = HET_L15_0.memory.control_word & 0xFFFFFF8E;
        //use_unit |= 0x21;
        //HET_L15_0.memory.control_word = use_unit;                       //enable X004 rise input interrupt
        bsp_kalyke_enable_X_interrupt(4, kGPIO_IntRisingEdge);
        if(counter_no == HCOUNTER245)
        {
            //GIOPOL1 |= 0x20;                                            //X005 rising confige
            //GIOENA1 |= 0x20;                                            //enable X5 rise input interrupt
            bsp_kalyke_enable_X_interrupt(5, kGPIO_IntRisingEdge);
        }
        else if(counter_no == HCOUNTER247)
        {
            //GIOPOL1 |= 0x20;                                            //X005 rising confige
            //GIOENA1 |= 0x20;                                            //enable X005 rise input interrupt
            bsp_kalyke_enable_X_interrupt(5, kGPIO_IntRisingEdge);
            //use_unit = HET_L13_0.memory.control_word & 0xFFFFFF8E;
            //use_unit |= 0x31;
            //HET_L13_0.memory.control_word = use_unit;                   //enable X7 rise and fall input interrupt
            bsp_kalyke_enable_X_interrupt(7, kGPIO_IntRisingOrFallingEdge);
        }
    }
    else if(init_onoff == 0)                        			          //cancel hardware
    {
        //GIOENA1 &= 0xFFFFFFBF;                                          //disable X003 rise input interrupt
        //HET_L15_0.memory.control_word &= 0xFFFFFFFE;                    //disable X004 rise input interrupt
        bsp_kalyke_disable_X_interrupt(3);
        bsp_kalyke_disable_X_interrupt(4);
        if(counter_no == HCOUNTER245)
        {
            //GIOENA1 &= 0xFFFFFFDF;                                      //disable X005 rise input interrupt
            bsp_kalyke_disable_X_interrupt(5);
        }
        else if(counter_no == HCOUNTER247)
        {
            //GIOENA1 &= 0xFFFFFFDF;                                      //disable X005 rise input interrupt
            //HET_L13_0.memory.control_word &= 0xFFFFFFFE;                //disable X7 rise and fall input interrupt
            bsp_kalyke_disable_X_interrupt(5);
            bsp_kalyke_disable_X_interrupt(7);
        }
    }
}

/************************************************************************
function:     The counters from X000 and x001 input are drived ;
description:  The function drive or cancel the counters's hardware;
input:        counter_nonum: opposite counter serial number;
              init_onoff: 1:drive the counter's hardware
                          2:cancel the counter's hardware
output:       no;
************************************************************************/
void hc_AB0_init(int counter_no, unsigned char init_onoff)
{
    unsigned int use_unit;
    //GIOENA1 &= 0xFFFFFFF7;
    if(init_onoff == 1)                 					              //init hardware
    {
        //GIOFLG1 &= 0xFFFFFFEF;			 //清除以前累计的X0中断的标志，解决切换到四倍频时减去一个的现象
        //GIOENA1 |= 0x08;                             //enable X0 interrupt if fourfold frequence counter
        //GIOPOL1 |= 0x08;                                                //X000 rising confige
        bsp_kalyke_enable_X_interrupt(0, kGPIO_IntRisingEdge);
        //use_unit = HET_L14_0.memory.control_word & 0xFFFFFF8E;
        //use_unit |= 0x31;
        //HET_L14_0.memory.control_word = use_unit;                       //enable X1 rise and fall input interrupt
        bsp_kalyke_enable_X_interrupt(1, kGPIO_IntRisingEdge);
        if(counter_no == HCOUNTER260)
        {
            //GIOPOL1 |= 0x10;                                            //X002 rising confige
            //GIOENA1 |= 0x10;                                            //enable X2 rise input interrupt
            bsp_kalyke_enable_X_interrupt(2, kGPIO_IntRisingEdge);
        }
        else if(counter_no == HCOUNTER262)
        {
            //GIOPOL1 |= 0x10;                                            //X002 rising confige
            //GIOENA1 |= 0x10;                                            //enable X2 rise input interrupt
            bsp_kalyke_enable_X_interrupt(2, kGPIO_IntRisingEdge);
            //use_unit = HET_L12_0.memory.control_word & 0xFFFFFF8E;
            //use_unit |= 0x31;
            //HET_L12_0.memory.control_word = use_unit;                   //enable X6 rise and fall input interrupt
            bsp_kalyke_enable_X_interrupt(6, kGPIO_IntRisingOrFallingEdge);
        }
    }
    else if(init_onoff == 0)                        			        //cancel hardware
    {
        //GIOENA1 &= 0xFFFFFFF7; //disable X0 interrupt if fourfold frequence counter
        bsp_kalyke_disable_X_interrupt(0);
        //HET_L14_0.memory.control_word &= 0xFFFFFFFE;
        bsp_kalyke_disable_X_interrupt(1);
        if(counter_no == HCOUNTER260)
        {
            //GIOENA1 &= 0xFFFFFFEF;                                      //disable X2 rise input interrupt
            bsp_kalyke_disable_X_interrupt(2);
        }
        else if(counter_no == HCOUNTER262)
        {
            //GIOENA1 &= 0xFFFFFFEF;                                      //disable X2 rise input interrupt
            //HET_L12_0.memory.control_word &= 0xFFFFFFFE;                //disable X6 rise and fall input interrupt
            bsp_kalyke_disable_X_interrupt(2);
            bsp_kalyke_disable_X_interrupt(6);
        }
    }
}

/************************************************************************
function:     The counters from X003 and x004 input are drived ;
description:  The function drive or cancel the counters's hardware;
input:        counter_nonum: opposite counter serial number;
              init_onoff: 1:drive the counter's hardware
                          2:cancel the counter's hardware
output:       no;
************************************************************************/
void hc_AB1_init(int counter_no, unsigned char init_onoff)
{
    unsigned int use_unit;
    //GIOENA1 &= 0xFFFFFFBF;                                          //disable X003 rise input interrupt
    if(init_onoff == 1)                 					              //init hardware
    {
        //GIOPOL1 |= 0x40;                                                //X003 rising confige
        bsp_kalyke_enable_X_interrupt(3, kGPIO_IntRisingEdge);
        //use_unit = HET_L15_0.memory.control_word & 0xFFFFFF8E;
        //use_unit |= 0x31;
        //HET_L15_0.memory.control_word = use_unit;                       //enable X004 rise input interrupt
        bsp_kalyke_enable_X_interrupt(4, kGPIO_IntRisingEdge);
        if(counter_no == HCOUNTER261)
        {
            //GIOPOL1 |= 0x20;                                            //X005 rising confige
            //GIOENA1 |= 0x20;                                            //enable X5 rise input interrupt
            bsp_kalyke_enable_X_interrupt(5, kGPIO_IntRisingEdge);
        }
        else if(counter_no == HCOUNTER263)
        {
            //GIOPOL1 |= 0x20;                                            //X005 rising confige
            //GIOENA1 |= 0x20;                                            //enable X005 rise input interrupt
            bsp_kalyke_enable_X_interrupt(5, kGPIO_IntRisingEdge);
            //use_unit = HET_L13_0.memory.control_word & 0xFFFFFF8E;
            //use_unit |= 0x31;
            //HET_L13_0.memory.control_word = use_unit;                   //enable X7 rise and fall input interrupt
            bsp_kalyke_enable_X_interrupt(7, kGPIO_IntRisingOrFallingEdge);
        }
    }
    else if(init_onoff == 0)                        			          //cancel hardware
    {
        bsp_kalyke_disable_X_interrupt(3);
        //HET_L15_0.memory.control_word &= 0xFFFFFFFE;                    //disable X004 rise input interrupt
        bsp_kalyke_disable_X_interrupt(4);
        if(counter_no == HCOUNTER261)
        {
            //GIOENA1 &= 0xFFFFFFDF;                                      //disable X005 rise input interrupt
            bsp_kalyke_disable_X_interrupt(5);
        }
        else if(counter_no == HCOUNTER263)
        {
            //GIOENA1 &= 0xFFFFFFDF;                                      //disable X005 rise input interrupt
            //HET_L13_0.memory.control_word &= 0xFFFFFFFE;                //disable X7 rise and fall input interrupt
            bsp_kalyke_disable_X_interrupt(5);
            bsp_kalyke_disable_X_interrupt(7);
        }
    }
}

/************************************************************************
function:     Cancel pulse capture function;
description:  make the X000-X007 normal input io;
input:        input_num: cancel number ;
op_type:  0, cancel interrupt; 1, capture interrupt; 2,user interrupt;
output:       no;
************************************************************************/
void cancel_capture(unsigned char input_num, unsigned char op_type)
{
    unsigned int use_unit;
    switch(input_num)
    {
    case 0:
    {
        if(op_type == 0)
        {
            //GIOENA1 &= 0xFFFFFFF7;                                          //disable X000  input interrupt
            bsp_kalyke_disable_X_interrupt(0);
        }
        else if(op_type == 1)
        {
            //GIOPOL1 |= 0x08;                                                //X000 rising confige
            //GIOENA1 |= 0x08;                                                //enable X0 rise input interrupt
            bsp_kalyke_enable_X_interrupt(0, kGPIO_IntRisingEdge);
        }
        else if(op_type == 2)
        {
            #if 0 // LiXianyu 20191217
            if(intr_num_save[0] == 0)
            {
                GIOPOL1 |= 0x08;                                                //X000 rising confige
                GIOENA1 |= 0x08;                                                //enable X0 rise input interrupt
            }
            else if(intr_num_save[0] == 10)
            {
                GIOPOL1 &= 0xFFFFFFF7;                                          //X000 falling confige
                GIOENA1 |= 0x08;                                                //enable X0 rise input interrupt
            }
            #endif
        }
    }
    break;

    case 1:
    {
        if(op_type == 0)
        {
            //HET_L14_0.memory.control_word &= 0xFFFFFFFE;                        //disable X001  input interrupt
            bsp_kalyke_disable_X_interrupt(1);
        }
        else if(op_type == 1)
        {
            /*
            use_unit = HET_L14_0.memory.control_word & 0xFFFFFF8E;
            use_unit |= 0x21;
            HET_L14_0.memory.control_word = use_unit;                           //enable X1 rise input interrupt
            */
            bsp_kalyke_enable_X_interrupt(1, kGPIO_IntRisingEdge);
        }
        else if(op_type == 2)
        {
            #if 0 // LiXianyu 20191217
            if(intr_num_save[1] == 1)
            {
                use_unit = HET_L14_0.memory.control_word & 0xFFFFFF8E;
                use_unit |= 0x21;
                HET_L14_0.memory.control_word = use_unit;                       //enable X1 rise input interrupt
            }
            else if(intr_num_save[1] == 11)
            {
                use_unit = HET_L14_0.memory.control_word & 0xFFFFFF8E;
                use_unit |= 0x11;
                HET_L14_0.memory.control_word = use_unit;                       //enable X1 falling input interrupt
            }
            #endif
        }
    }
    break;

    case 2:
    {
        if(op_type == 0)
        {
            //GIOENA1 &= 0xFFFFFFEF;                                              //disable X002  input interrupt
            bsp_kalyke_disable_X_interrupt(2);
        }
        else if(op_type == 1)
        {
            //GIOPOL1 |= 0x10;                                                    //X002 rising confige
            //GIOENA1 |= 0x10;                                                    //enable X2 rise input interrupt
            bsp_kalyke_enable_X_interrupt(2, kGPIO_IntRisingEdge);
        }
        else if(op_type == 2)
        {
            #if 0 // LiXianyu 20191217
            if(intr_num_save[2] == 2)
            {
                GIOPOL1 |= 0x10;                                                //X002 rising confige
                GIOENA1 |= 0x10;                                                //enable X2 rise input interrupt
            }
            else if(intr_num_save[2] == 12)
            {
                GIOPOL1 &= 0xFFFFFFEF;                                           //X002 falling confige
                GIOENA1 |= 0x10;                                                 //enable X2 rise input interrupt
            }
            #endif
        }
    }
    break;

    case 3:
    {
        if(op_type == 0)
        {
            //GIOENA1 &= 0xFFFFFFBF;                                              //disable X003  input interrupt
            bsp_kalyke_disable_X_interrupt(3);
        }
        else if(op_type == 1)
        {
            //GIOPOL1 |= 0x40;                                                    //X003 rising confige
            //GIOENA1 |= 0x40;                                                    //enable X3 rise input interrupt
            bsp_kalyke_enable_X_interrupt(3, kGPIO_IntRisingEdge);
        }
        else if(op_type == 2)
        {
            #if 0 // LiXianyu 20191217
            if(intr_num_save[3] == 3)
            {
                GIOPOL1 |= 0x40;                                                //X003 rising confige
                GIOENA1 |= 0x40;                                                //enable X3 rise input interrupt
            }
            else if(intr_num_save[3] == 13)
            {
                GIOPOL1 &= 0xFFFFFFBF;                                           //X003 falling confige
                GIOENA1 |= 0x40;                                                 //enable X3 rise input interrupt
            }
            #endif
        }
    }
    break;

    case 4:
    {
        if(op_type == 0)
        {
            //HET_L15_0.memory.control_word &= 0xFFFFFFFE;                    //disable X4 rise input interrupt
            bsp_kalyke_disable_X_interrupt(4);
        }
        else if(op_type == 1)
        {
            //use_unit = HET_L15_0.memory.control_word & 0xFFFFFF8E;
            //use_unit |= 0x21;
            //HET_L15_0.memory.control_word = use_unit;                       //enable X4 rise input interrupt
            bsp_kalyke_enable_X_interrupt(4, kGPIO_IntRisingEdge);
        }
        else if(op_type == 2)
        {
            #if 0 // LiXianyu 20191217
            if(intr_num_save[4] == 4)
            {
                use_unit = HET_L15_0.memory.control_word & 0xFFFFFF8E;
                use_unit |= 0x21;
                HET_L15_0.memory.control_word = use_unit;                       //enable X4 rise input interrupt
            }
            else if(intr_num_save[4] == 14)
            {
                use_unit = HET_L15_0.memory.control_word & 0xFFFFFF8E;
                use_unit |= 0x11;
                HET_L15_0.memory.control_word = use_unit;                       //enable X4 rise input interrupt
            }
            #endif
        }
    }
    break;

    case 5:
    {
        if(op_type == 0)
        {
            //GIOENA1 &= 0xFFFFFFDF;                                          //disable X5 rise input interrupt
            bsp_kalyke_disable_X_interrupt(5);
        }
        else if(op_type == 1)
        {
            //GIOPOL1 |= 0x20;                                                //X005 rising confige
            //GIOENA1 |= 0x20;                                                //enable X5 rise input interrupt
            bsp_kalyke_enable_X_interrupt(5, kGPIO_IntRisingEdge);
        }
        else if(op_type == 2)
        {
        #if 0 // LiXianyu 20191217
            if(intr_num_save[5] == 5)
            {
                GIOPOL1 |= 0x20;                                                //X005 rising confige
                GIOENA1 |= 0x20;                                                //enable X5 rise input interrupt
            }
            else if(intr_num_save[5] == 15)
            {
                GIOPOL1 &= 0xFFFFFFDF;                                          //X005 falling confige
                GIOENA1 |= 0x20;                                                //enable X5 rise input interrupt
            }
        #endif
        }
    }
    break;

    case 6:
    {
        if(op_type == 0)
        {
            //HET_L12_0.memory.control_word &= 0xFFFFFFFE;                    //disable X6 rise input interrupt
            bsp_kalyke_disable_X_interrupt(6);
        }
        else if(op_type == 1)
        {
            //use_unit = HET_L12_0.memory.control_word & 0xFFFFFF8E;
            //use_unit |= 0x21;
            //HET_L12_0.memory.control_word = use_unit;                       //enable X6 rise input interrupt
            bsp_kalyke_enable_X_interrupt(6, kGPIO_IntRisingEdge);
        }
        else if(op_type == 2)
        {
        #if 0 // LiXianyu 20191217
            if(intr_num_save[6] == 6)
            {
                use_unit = HET_L12_0.memory.control_word & 0xFFFFFF8E;
                use_unit |= 0x21;
                HET_L12_0.memory.control_word = use_unit;                   //enable X6 riseing input interrupt
            }
            else if(intr_num_save[6] == 16)
            {
                use_unit = HET_L12_0.memory.control_word & 0xFFFFFF8E;
                use_unit |= 0x11;
                HET_L12_0.memory.control_word = use_unit;                   //enable X6 falling input interrupt
            }
        #endif
        }
        else if(op_type == 3)
        {
            bsp_kalyke_enable_X_interrupt(6, kGPIO_IntRisingEdge);//enable X6 rise input interrupt
        }
    }
    break;

    case 7:
    {
        if(op_type == 0)
        {
            //HET_L13_0.memory.control_word &= 0xFFFFFFFE;                    //disable X7 rise input interrupt
            bsp_kalyke_disable_X_interrupt(7);
        }
        else if(op_type == 1)
        {
            //use_unit = HET_L13_0.memory.control_word & 0xFFFFFF8E;
            //use_unit |= 0x21;
            //HET_L13_0.memory.control_word = use_unit;                       //enable X7 rise input interrupt
            bsp_kalyke_enable_X_interrupt(7, kGPIO_IntRisingEdge);
        }
        else if(op_type == 2)
        {
        #if 0 // LiXianyu 20191217
            if(intr_num_save[7] == 7)
            {
                use_unit = HET_L13_0.memory.control_word & 0xFFFFFF8E;
                use_unit |= 0x21;
                HET_L13_0.memory.control_word = use_unit;                   //enable X7 riseing input interrupt
            }
            else if(intr_num_save[7] == 17)
            {
                use_unit = HET_L13_0.memory.control_word & 0xFFFFFF8E;
                use_unit |= 0x11;
                HET_L13_0.memory.control_word = use_unit;                   //enable X7 falling input interrupt
            }
        #endif
        }
        else if(op_type == 3)
        {
            //use_unit = HET_L13_0.memory.control_word & 0xFFFFFF8E;
            //use_unit |= 0x31;
            //HET_L13_0.memory.control_word = use_unit;                       //enable X7 rise input interrupt
            bsp_kalyke_enable_X_interrupt(7, kGPIO_IntRisingEdge);
        }
    }
    break;

    default:
        break;
    }
}

#if 0 // LiXianyu 20191216
/************************************************************************
                pulse number test for X000--X005
************************************************************************/
/************************************************************************
function: pulse number test for X000;
description:
input   :op_type；0.config the port for normal's io;
                  1.init the hardware for SPD count;
                  2.count the SPD'S parameter;
output  : no;
************************************************************************/
void spd_x0(unsigned char op_type)
{
    unsigned int  cmp_long_num;
    unsigned int  spd_time_rd;
    if(op_type == 0)          						                  //config the hardware normal io;
    {
        GIOENA1 &= 0xFFFFFFF7;                                          //disable X0 rise input interrupt
    }
    else if(op_type == 1)     						                  //init the hardware for SPD begin
    {
        GIOPOL1 |= 0x08;                                                //X000 rising confige
        GIOENA1 |= 0x08;                                                //enable X0 rise input interrupt
    }
    else if(op_type == 2)     						                  //count the number for SPD
    {
        __disable_interrupt();                                          // disable interrupts
        spd_time_rd = RTICNTR;
        spd_time_rd >>= 11;
        if(spd_time_save[0] == 0)
        {
            spd_time_rec[0] = spd_time_rd;
            *spd_count[0] = 0;      					                  //save the record time of RTI
            *(spd_count[0] + 1) = 0;
            *(spd_count[0] + 2) = 0;
            spd_time_savet[0] = 0;
            __enable_interrupt();                                            // Enable interrupts
            return;
        }

        cmp_long_num = spd_time_rd - spd_time_rec[0];
        spd_time_rec[0] = spd_time_rd;
        spd_time_savet[0] += (cmp_long_num & 0x1FFFFF);
        if(spd_time_savet[0] >= spd_time_save[0])
        {
            spd_time_rec[0] = spd_time_rd;
            spd_time_savet[0] = 0;
            *spd_count[0] = *(spd_count[0] + 1);      	              //pulse number save
            *(spd_count[0] + 1) = 0;
            *(spd_count[0] + 2) = 0;
            __enable_interrupt();                                            // Enable interrupts
        }
        else
        {
            __enable_interrupt();                                            // Enable interrupts
            *(spd_count[0] + 2) = (unsigned short)((spd_time_save[0] - spd_time_savet[0]) / 1000);
        }
    }
}

/************************************************************************
function: pulse number test for X001;
description:
input   :op_type；0.config the port for normal's io;
                  1.init the hardware for SPD count;
                  2.count the SPD'S parameter;
output  : no;
************************************************************************/
void spd_x1(unsigned char op_type)
{
    unsigned int  cmp_long_num;
    unsigned int  use_unit;
    unsigned int  spd_time_rd;
    if(op_type == 0)          						                    //config the hardware normal io;
    {
        HET_L14_0.memory.control_word &= 0xFFFFFFFE;
    }
    else if(op_type == 1)     						                    //init the hardware for SPD begin
    {
        use_unit = HET_L14_0.memory.control_word & 0xFFFFFF8E;
        use_unit |= 0x21;
        HET_L14_0.memory.control_word = use_unit;                       //enable X1 rise input interrupt
    }
    else if(op_type == 2)     						                    //count the number for SPD
    {
        __disable_interrupt();                                          // disable interrupts
        spd_time_rd = RTICNTR;
        spd_time_rd >>= 11;
        if(spd_time_save[1] == 0)
        {
            spd_time_rec[1] = spd_time_rd;
            *spd_count[1] = 0;      					                  //save the record time of RTI
            *(spd_count[1] + 1) = 0;
            *(spd_count[1] + 2) = 0;
            spd_time_savet[1] = 0;
            __enable_interrupt();                                            // Enable interrupts
            return;
        }
        cmp_long_num = spd_time_rd - spd_time_rec[1];
        spd_time_rec[1] = spd_time_rd;
        spd_time_savet[1] += (cmp_long_num & 0x1FFFFF);
        if(spd_time_savet[1] >= spd_time_save[1])
        {
            spd_time_rec[1] = spd_time_rd;
            spd_time_savet[1] = 0;
            *spd_count[1] = *(spd_count[1] + 1);      	              //pulse number save
            *(spd_count[1] + 1) = 0;
            *(spd_count[1] + 2) = 0;
            __enable_interrupt();                                           // Enable interrupts
        }
        else
        {
            __enable_interrupt();                                           // Enable interrupts
            *(spd_count[1] + 2) = (unsigned short)((spd_time_save[1] - spd_time_savet[1]) / 1000);
        }
    }
}

/************************************************************************
function: pulse number test for X002;
description:
input   :op_type；0.config the port for normal's io;
                  1.init the hardware for SPD count;
                  2.count the SPD'S parameter;
output  : no;
************************************************************************/
void spd_x2(unsigned char op_type)
{
    unsigned int  cmp_long_num;
    unsigned int  spd_time_rd;
    if(op_type == 0)          						                    //config the hardware normal io;
    {
        GIOENA1 &= 0xFFFFFFEF;                                          //disable X2 rise input interrupt
    }
    else if(op_type == 1)     						                    //init the hardware for SPD begin
    {
        GIOPOL1 |= 0x10;                                                //X002 rising confige
        GIOENA1 |= 0x10;                                                //enable X2 rise input interrupt
    }
    else if(op_type == 2)     						                    //count the number for SPD
    {
        __disable_interrupt();                                          // disable interrupts
        spd_time_rd = RTICNTR;
        spd_time_rd >>= 11;
        if(spd_time_save[2] == 0)
        {
            spd_time_rec[2] = spd_time_rd;
            *spd_count[2] = 0;      					                  //save the record time of RTI
            *(spd_count[2] + 1) = 0;
            *(spd_count[2] + 2) = 0;
            spd_time_savet[2] = 0;
            __enable_interrupt();                                            // Enable interrupts
            return;
        }

        cmp_long_num = spd_time_rd - spd_time_rec[2];
        spd_time_rec[2] = spd_time_rd;
        spd_time_savet[2] += (cmp_long_num & 0x1FFFFF);
        if(spd_time_savet[2] >= spd_time_save[2])
        {
            spd_time_rec[2] = spd_time_rd;
            spd_time_savet[2] = 0;
            *spd_count[2] = *(spd_count[2] + 1);      	              //pulse number save
            *(spd_count[2] + 1) = 0;
            *(spd_count[2] + 2) = 0;
            __enable_interrupt();                                           // Enable interrupts
        }
        else
        {
            __enable_interrupt();                                           // Enable interrupts
            *(spd_count[2] + 2) = (unsigned short)((spd_time_save[2] - spd_time_savet[2]) / 1000);
        }
    }
}

/************************************************************************
function: pulse number test for X003;
description:
input   :op_type；0.config the port for normal's io;
                  1.init the hardware for SPD count;
                  2.count the SPD'S parameter;
output  : no;
************************************************************************/
void spd_x3(unsigned char op_type)
{
    unsigned int  cmp_long_num;
    unsigned int  spd_time_rd;
    if(op_type == 0)          						                    //config the hardware normal io;
    {
        GIOENA1 &= 0xFFFFFFBF;                                          //disable X003 rise input interrupt
    }
    else if(op_type == 1)     						                    //init the hardware for SPD begin
    {
        GIOPOL1 |= 0x40;                                                //X003 rising confige
        GIOENA1 |= 0x40;                                                //enable X003 rise input interrupt
    }
    else if(op_type == 2)     						                    //count the number for SPD
    {
        __disable_interrupt();                                          // disable interrupts
        spd_time_rd = RTICNTR;
        spd_time_rd >>= 11;
        if(spd_time_save[3] == 0)
        {
            spd_time_rec[3] = spd_time_rd;
            *spd_count[3] = 0;      					                    //save the record time of RTI
            *(spd_count[3] + 1) = 0;
            *(spd_count[3] + 2) = 0;
            spd_time_savet[3] = 0;
            __enable_interrupt();                                            // Enable interrupts
            return;
        }
        cmp_long_num = spd_time_rd - spd_time_rec[3];
        spd_time_rec[3] = spd_time_rd;
        spd_time_savet[3] += (cmp_long_num & 0x1FFFFF);
        if(spd_time_savet[3] >= spd_time_save[3])
        {
            spd_time_rec[3] = spd_time_rd;
            spd_time_savet[3] = 0;
            *spd_count[3] = *(spd_count[3] + 1);      	              //pulse number save
            *(spd_count[3] + 1) = 0;
            *(spd_count[3] + 2) = 0;
            __enable_interrupt();                                           // Enable interrupts
        }
        else
        {
            __enable_interrupt();                                           // Enable interrupts
            *(spd_count[3] + 2) = (unsigned short)((spd_time_save[3] - spd_time_savet[3]) / 1000);
        }
    }
}

/************************************************************************
function: pulse number test for X004;
description:
input   :op_type；0.config the port for normal's io;
                  1.init the hardware for SPD count;
                  2.count the SPD'S parameter;
output  : no;
************************************************************************/
void spd_x4(unsigned char op_type)
{
    unsigned int  cmp_long_num;
    unsigned int  use_unit;
    unsigned int  spd_time_rd;
    if(op_type == 0)          						                    //config the hardware normal io;
    {
        HET_L15_0.memory.control_word &= 0xFFFFFFFE;                    //disable X4 rise input interrupt
    }
    else if(op_type == 1)     						                    //init the hardware for SPD begin
    {
        use_unit = HET_L15_0.memory.control_word & 0xFFFFFF8E;
        use_unit |= 0x21;
        HET_L15_0.memory.control_word = use_unit;                       //enable X4 rise input interrupt
    }
    else if(op_type == 2)     						                    //count the number for SPD
    {
        __disable_interrupt();                                          // disable interrupts
        spd_time_rd = RTICNTR;
        spd_time_rd >>= 11;
        if(spd_time_save[4] == 0)
        {
            spd_time_rec[4] = RTICNTR_bit.CNTR20_0;
            *spd_count[4] = 0;      					                    //save the record time of RTI
            *(spd_count[4] + 1) = 0;
            *(spd_count[4] + 2) = 0;
            spd_time_savet[4] = 0;
            __enable_interrupt();                                            // Enable interrupts
            return;
        }

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
            __enable_interrupt();                                           // Enable interrupts
        }
        else
        {
            __enable_interrupt();                                           // Enable interrupts
            *(spd_count[4] + 2) = (unsigned short)((spd_time_save[4] - spd_time_savet[4]) / 1000);
        }
    }
}


/************************************************************************
function: pulse number test for X005;
description:
input   :op_type；0.config the port for normal's io;
                  1.init the hardware for SPD count;
                  2.count the SPD'S parameter;
output  : no;
************************************************************************/
void spd_x5(unsigned char op_type)
{
    unsigned int  cmp_long_num;
    unsigned int  spd_time_rd;
    if(op_type == 0)          						                    //config the hardware normal io;
    {
        GIOENA1 &= 0xFFFFFFDF;                                          //disable X5 rise input interrupt
    }
    else if(op_type == 1)     						                    //init the hardware for SPD begin
    {
        GIOPOL1 |= 0x20;                                                //X005 rising confige
        GIOENA1 |= 0x20;                                                //enable X5 rise input interrupt
    }
    else if(op_type == 2)     						                    //count the number for SPD
    {
        __disable_interrupt();                                          // disable interrupts
        spd_time_rd = RTICNTR;
        spd_time_rd >>= 11;
        if(spd_time_save[5] == 0)
        {
            spd_time_rec[5] = spd_time_rd;
            *spd_count[5] = 0;      					                    //save the record time of RTI
            *(spd_count[5] + 1) = 0;
            *(spd_count[5] + 2) = 0;
            spd_time_savet[5] = 0;
            __enable_interrupt();                                            // Enable interrupts
            return;
        }
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
            __enable_interrupt();                                           // Enable interrupts
        }
        else
        {
            __enable_interrupt();
            *(spd_count[5] + 2) = (unsigned short)((spd_time_save[5] - spd_time_savet[5]) / 1000);
        }                                           // Enable interrupts
    }
}

/************************************************************************
function: get the user's control unit point;
description: ;
input   : pucode--system user code save room;
          idisp--user's control unit excursion;
output  : return the successful;
************************************************************************/
unsigned char get_word_point(unsigned char *pucode, unsigned short idisp)
{
    unsigned short deco_num[2];
    deco_num[0] = *(unsigned short *)(pucode + 10);
    deco_num[1] = *(unsigned short *)(pucode + 12);
    if(deco_num[0] == 0x1100)      					//jude the D user's element
    {
        if((deco_num[1] + idisp) >= D_RANG)
            return ERR_ELEM_RANG;
        get_word_addr = (unsigned short *)&g_D[deco_num[1] + idisp];
        return RIGHT;
    }
    else if(deco_num[0] == 0x1200)      				//jude the Z user's element
    {
        if((deco_num[1] + idisp) >= Z_RANG)
            return ERR_ELEM_RANG;
        get_word_addr = (unsigned short *)&g_Z[deco_num[1] + idisp];
        return RIGHT;
    }
    else if(deco_num[0] == 0x1600)      				//jude the V user's element
    {
        if((deco_num[1] + idisp) >= V_RANG)
            return ERR_ELEM_RANG;
        get_word_addr = (unsigned short *)&g_V[g_pou_sp][deco_num[1] + idisp];
        return RIGHT;
    }
    else if(deco_num[0] == 0x1300)      				//jude the SD user's element
    {
        if((deco_num[1] + idisp) >= SD_RANG)
            return ERR_ELEM_RANG;
        get_word_addr = (unsigned short *)&g_SD[deco_num[1] + idisp];
        return RIGHT;
    }
    else if(deco_num[0] == 0x1400)      				//jude the C user's element
    {
        deco_num[1] += idisp;
        if(deco_num[1] < 200)							//16 bits counter
        {
            get_word_addr = (unsigned short *)&g_C_CV16[deco_num[1]];
            return RIGHT;
        }
        else if(deco_num[1] < 236)      				//32 bits counter
        {
            get_word_addr = (unsigned short *)&g_C_CV32[deco_num[1]];
            return RIGHT;
        }
        else if(deco_num[1] < 256) 					//32 bits high-counter
        {
            get_word_addr = (unsigned short *)&g_C_HCD32[deco_num[1]];
            return RIGHT;
        }
        return ERR_ELEM_RANG;
    }
    else if(deco_num[0] == 0x1500)      				//jude the T user's element
    {
        if((deco_num[1] + idisp) >= T_RANG)
            return ERR_ELEM_RANG;
        get_word_addr = (unsigned short *)&g_T_CV[deco_num[1] + idisp];
        return RIGHT;
    }

    else if(*(pucode + 10) == 0x32)      				//jude the D of variable user's element
    {
        if((*(pucode + 11)) > 15)
            return ERR_ELEM_RANG;
        if((deco_num[1] + idisp + g_Z[*(pucode + 11)]) >= D_RANG)
            return ERR_ELEM_RANG;
        get_word_addr = (unsigned short *)&g_D[deco_num[1] + idisp + g_Z[*(pucode + 11)]];
        return RIGHT;
    }
    else if(*(pucode + 10) == 0x33)      				//jude the C of variable user's element
    {
        if((*(pucode + 11)) > 15)
            return ERR_ELEM_RANG;
        deco_num[1] = deco_num[1] + idisp + g_Z[*(pucode + 11)];
        if(deco_num[1] < 200)							//16 bits counter
        {
            get_word_addr = (unsigned short *)&g_C_CV16[deco_num[1] + idisp + g_Z[*(pucode + 11)]];
            return RIGHT;
        }
        else if(deco_num[1] < 236)      				//32 bits counter
        {
            get_word_addr = (unsigned short *)&g_C_CV32[deco_num[1] + idisp + g_Z[*(pucode + 11)]];
            return RIGHT;
        }
        else if(deco_num[1] < 256) 					//32 bits high-counter
        {
            get_word_addr = (unsigned short *)&g_C_HCD32[deco_num[1] + idisp + g_Z[*(pucode + 11)]];
            return RIGHT;
        }
        return ERR_ELEM_RANG;
    }
    else if(*(pucode + 10) == 0x34)      				//jude the T of variable user's element
    {
        if((*(pucode + 11)) > 15)
            return ERR_ELEM_RANG;
        if((deco_num[1] + idisp + g_Z[*(pucode + 11)]) >= TV_RANG)
            return ERR_ELEM_RANG;
        get_word_addr = (unsigned short *)&g_T_CV[deco_num[1] + idisp + g_Z[*(pucode + 11)]];
        return RIGHT;
    }
    else if(*(pucode + 10) == 0x3C)
    {
        if((deco_num[1] + idisp + g_Z[*(pucode + 11)]) >= V_RANG)
            return ERR_ELEM_RANG;
        get_word_addr = (unsigned short *)&g_V[g_pou_sp][deco_num[1] + idisp + g_Z[*(pucode + 11)]];
        return RIGHT;
    }
    return ERR_ELEM_TYPE;
}
#endif
/************************************************************************
function: the bit operation for high_speed instruction ;
description: set the bit's operation of  high_speed instruction
             from  the user's element  ;
input   : unsigned char optype,unsigned int opnum,unsigned char *parray;
output  : no;
************************************************************************/
unsigned char parse_ucode_bit(unsigned char *optype, unsigned short *opnum, unsigned char *parray)
{// See plc_parseaddr.h
    if(*parray == ADDR_Y)      						          //operation for Y element
    {
        *optype = 1;
    }
    else if(*parray == ADDR_M)   						          //operation for M element
    {
        *optype = 2;
    }
    else if(*parray == ADDR_S)   						          //operation for S element
    {
        *optype = 3;
    }
    #if 0
    else if((*(unsigned short *)parray == 0) || (*(unsigned short *)parray == 1))
    {
        *optype = 5;                                        //operation for I010--I060 element
    }
    else if(*(unsigned short *)parray == 0x300)
    {
        if(*(parray - 3) != *(parray + 3))                  //operation for high_counter self
            return 0;
        *optype = 5;
    }
    #endif
    else
    {
        return 0;
    }
    *opnum = *(unsigned short *)(parray + 1);
    return 1;
}

/************************************************************************
function: dispose the instruction's record of the same high_counter
description:;
input   :
output  :
************************************************************************/
static void hccmd_combinate(void)
{
    unsigned char i, j, count;
    for(i = 0; i < 6; i++)
    {
        count = 0;
        for(j = 0; j < 6; j++)
        {
            if(hc_cmd_copy[j].cmdno == 0xFF)
            {
                continue;
            }
            if(use_hc_attr[i].hcno == (hc_cmd_copy[j].hcno))
            {
                hc_array[i][count] = j;                         //save the instruction record
                count++;
            }
        }
        hc_cmdnum[i] = count;                                   //save the number of instruction
    }
}

/************************************************************************
function: high_counter instruction add
description:;
input   : struct cmd_copy_str hccmd_now---high_counter's instruction property；
output  :
************************************************************************/
void hc_cmd_save(struct cmd_copy_str * hc_cmd_now)
{
    unsigned char i;
    for(i = 0; i < 6; i++)
    {
        if(hc_cmd_now->cmdno == hc_cmd_copy[i].cmdno)                //Judge the instruction is existent
        {
            hc_cmd_copy[i].cmptype = hc_cmd_now->cmptype;            //Reflash the parament of instruction
            hccmd_combinate();
            return;
        }
    }
    if(hccmdall >= 6)                                               //Judge  the number of instruction
    {
        return;
    }
    for(i = 0; i < 6; i++)
    {
        if(hc_cmd_copy[i].cmdno == 0xFF) //找到一个还没用的
        {
            if (logFlag) LOGV(TAG, "%s: Got a cmdno", __func__);
            hc_cmd_now->opstate = 0;
            //hc_cmd_copy[i] = hc_cmd_now;
            memcpy(&hc_cmd_copy[i], hc_cmd_now, sizeof(struct cmd_copy_str));
            hccmdall++;
            hccmd_combinate();                                     //Reflash the parament of instruction about the same instruction
            return;
        }
    }
}

/************************************************************************
function: high_counter instruction sub
description:;
input   : struct cmd_copy_str hccmd_now---high_counter's instruction property；
output  :
************************************************************************/
void hc_cmd_del(struct cmd_copy_str *hc_cmd_now)
{
    unsigned char i, j, *parray;
    for(i = 0; i < 6; i++)                                                      //Judge the instruction is existent
    {
        if(hc_cmd_now->cmdno == hc_cmd_copy[i].cmdno)
        {
            parray = (unsigned char *)&hc_cmd_copy[i];
            for(j = 0; j < sizeof(hc_cmd_copy[0]); j++)                         //Clear the record of the same high_count;
            {
                *parray++ = 0;
            }
            hc_cmd_copy[i].cmdno = 0xFF;
            hccmdall--;
            hccmd_combinate();                                                  //Reflash the parament of instruction about the same instruction
            return;
        }
    }
}

/************************************************************************
function: read DHSP and DHST's table property
description:
input   read_flag; 1:read the DHSP parament,0:read the DHST paramnet
		struct cmd_copy_str *use_copy point the change parament
output  :
************************************************************************/
void read_dhsp_dhst_num(unsigned char read_flag, struct cmd_copy_str *use_copy)
{
    int *usep, *usepa;
    unsigned short useno; // D元件起始值。例如D100，则useno为100

    useno = hc_cmd_table.startaddr + hc_cmd_table.curpoint * 4;
    //g_SD[184] = hc_cmd_table.curpoint + 1;
    SET_SD_ELEMENT_VALUE(SD184, hc_cmd_table.curpoint + 1);
    //usep = (int *)&g_D[useno];
    usep = (int *)&gtv_PlcElement.msp_DElement[useno];
    //usepa = (int *)&g_SD[182];
    usepa = (int *)&gtv_PlcElement.msp_SDElement[SD182];
    if (logFlag) LOGD(TAG, "*usep = %d, *usepa = %d, useno = %u", *usep, *usepa, useno);
    if(read_flag)//Read DHSP current parament
    {
        use_copy->cmptype.dhsp.cmppoint = *usep++;
        *usepa = use_copy->cmptype.dhsp.cmppoint;
        //*(int *)&g_SD[180] = *usep;
        *(int *)&gtv_PlcElement.msp_SDElement[SD180] = *usep;
        use_copy->cmptype.dhsp.movnum = *usep;
    }
    else //Read DHST current parament
    {
        //use_copy->cmptype.dhst.cmppoint = *usep++;
        use_copy->cmptype.dhst.cmppoint = *usep;
        *usepa = use_copy->cmptype.dhst.cmppoint;
        //use_copy->cmptype.dhst.opnum = g_D[useno + 2];
        use_copy->cmptype.dhst.opnum = GET_D_ELEMENT_VALUE(useno + 2);
        //use_copy->cmptype.dhst.opdo = g_D[useno + 3];
        use_copy->cmptype.dhst.opdo = GET_D_ELEMENT_VALUE(useno + 3);
    }
}

/************************************************************************
function: read the DHSP and DHST's table parament
description:
input   read_flag 1:read the DHSP's parament, 0:read the DHST's parament
		struct cmd_copy_str *use_copy
output  :
************************************************************************/
void ref_dhsp_dhst_num(unsigned char read_flag, struct cmd_copy_str *use_copy)
{
    int *usep, *usepa;
    unsigned short useno;
    hc_cmd_table.curpoint++;
    if(hc_cmd_table.curpoint == hc_cmd_table.pointall)                  //Table's record is finish
    {
        //g_SM[185] = 1;
        plc_set_bit_element_value(SM_ELEMENT, SM185, 1);
        hc_cmd_table.curpoint = 0;
    }
    useno = hc_cmd_table.startaddr + hc_cmd_table.curpoint * 4;
    //g_SD[184] = hc_cmd_table.curpoint + 1;
    SET_SD_ELEMENT_VALUE(SD184, hc_cmd_table.curpoint + 1);
    //usep = (int *)&g_D[useno];
    usep = (int *)&gtv_PlcElement.msp_DElement[useno];
    //usepa = (int *)&g_SD[182];
    usepa = (int *)&gtv_PlcElement.msp_SDElement[SD182];
    if(read_flag)                                                       //Read the DHSP current parament
    {
        use_copy->cmptype.dhsp.cmppoint = *usep++;
        *usepa = use_copy->cmptype.dhsp.cmppoint;
        //*(int *)&g_SD[180] = *usep;
        *(int *)&gtv_PlcElement.msp_SDElement[SD180] = *usep;
        use_copy->cmptype.dhsp.movnum = *usep;
        if(hc_cmd_table.curpoint == (hc_cmd_table.pointall - 1))
        {
            if(*usep == 0)
            {
                hc_cmd_del(use_copy);
            }
        }
    }
    else                                                                //Read the DHST current parament
    {
        //use_copy->cmptype.dhst.cmppoint = *usep++;
        use_copy->cmptype.dhst.cmppoint = *usep;
        *usepa = use_copy->cmptype.dhst.cmppoint;
        //use_copy->cmptype.dhst.opnum = g_D[useno + 2];
        use_copy->cmptype.dhst.opnum = GET_D_ELEMENT_VALUE(useno + 2);
        //use_copy->cmptype.dhst.opdo = g_D[useno + 3];
        use_copy->cmptype.dhst.opdo = GET_D_ELEMENT_VALUE(useno + 3);
    }
}

/************************************************************************
function: parse the high_counter's operation
description: parse the parament to do the action
output  :
************************************************************************/
#if 1
void hccmd_act(unsigned char hc_act_no)
{
    unsigned char opstate, array;
    unsigned short useno;
    if(hc_buf[hc_act_no].op_mode != 4)
    {
        array = hc_buf[hc_act_no].op_act;
        if (logFlag) LOGV(TAG, "%s: array = %u", __func__, array);
        if(hc_cmd_copy[array].cmdno == 0xFF)
        {
            return;
        }
        if(array > 5)
        {
            return;
        }
        if(hc_cmd_copy[array].cmdtype == 1)                                 //DHSCS's instruction doing;
        {
            if(hc_cmd_copy[array].cmptype.dhscs.optype == 1)           		//set Y element
            {
                //if(g_Y[hc_cmd_copy[array].cmptype.dhscs.opnum])
                if (plc_get_bit_element_value(Y_ELEMENT, hc_cmd_copy[array].cmptype.dhscs.opnum))
                {
                    return;
                }
                //g_Y[hc_cmd_copy[array].cmptype.dhscs.opnum] = 1;
                plc_set_bit_element_value(Y_ELEMENT, hc_cmd_copy[array].cmptype.dhscs.opnum, 1);
                hccmd_output(hc_cmd_copy[array].cmptype.dhscs.opnum, 1); 	//call the io's output
            }
            else if(hc_cmd_copy[array].cmptype.dhscs.optype == 2)      		//set M element
            {
                //g_M[hc_cmd_copy[array].cmptype.dhscs.opnum] = 1;
                plc_set_bit_element_value(M_ELEMENT, hc_cmd_copy[array].cmptype.dhscs.opnum, 1);
            }
            else if(hc_cmd_copy[array].cmptype.dhscs.optype == 3)      		//set S element
            {
                //g_S[hc_cmd_copy[array].cmptype.dhscs.opnum] = 1;
                plc_set_bit_element_value(S_ELEMENT, hc_cmd_copy[array].cmptype.dhscs.opnum, 1);
            }
            #if 0 // LiXianyu 20191222
            else if(hc_cmd_copy[array].cmptype.dhscs.optype == 5)      		//call the high_counter's interrupt;
            {
                if((g_SM[65] == 1) && g_intrpt_en)
                {
                    add_intr(hc_cmd_copy[array].cmptype.dhscs.opnum);
                }
            }
            #endif
        }  //end of "if(hc_cmd_copy[array].cmdtype==1)"
        else if(hc_cmd_copy[array].cmdtype == 2)                            //DHSCR's instruction doing;
        {
            if(hc_cmd_copy[array].cmptype.dhscr.optype == 1)   				//clear Y element
            {
                //if(!g_Y[hc_cmd_copy[array].cmptype.dhscr.opnum])
                if(!plc_get_bit_element_value(Y_ELEMENT, hc_cmd_copy[array].cmptype.dhscr.opnum))
                {
                    return;
                }
                //g_Y[hc_cmd_copy[array].cmptype.dhscr.opnum] = 0;
                plc_set_bit_element_value(Y_ELEMENT, hc_cmd_copy[array].cmptype.dhscr.opnum, 0);
                hccmd_output(hc_cmd_copy[array].cmptype.dhscr.opnum, 0);  	//call the io's output
            }
            else if(hc_cmd_copy[array].cmptype.dhscr.optype == 2)   			//clear M element
            {
                //g_M[hc_cmd_copy[array].cmptype.dhscr.opnum] = 0;
                plc_set_bit_element_value(M_ELEMENT, hc_cmd_copy[array].cmptype.dhscr.opnum, 0);
            }
            else if(hc_cmd_copy[array].cmptype.dhscr.optype == 3)   			//clear S element
            {
                //g_S[hc_cmd_copy[array].cmptype.dhscr.opnum] = 0;
                plc_set_bit_element_value(S_ELEMENT, hc_cmd_copy[array].cmptype.dhscr.opnum, 0);
            }
            else if(hc_cmd_copy[array].cmptype.dhscr.optype == 5)   			//clear high_counter self
            {
                //g_C[hc_cmd_copy[array].hcno + 236] = 0;
                plc_set_bit_element_value(C_ELEMENT, hc_cmd_copy[array].hcno + 236, 0);
            }
        } //end of "else if(hc_cmd_copy[array].cmdtype==2)"
        else if(hc_cmd_copy[array].cmdtype == 3)                            //DHSZ's instruction doing;
        {
            opstate = hc_buf[hc_act_no].op_mode;

            if(opstate == 1)                                                //less the first parament
            {
                if(hc_cmd_copy[array].cmptype.dhsz.optype == 1)   			//clear Y element
                {
                    //if((g_Y[hc_cmd_copy[array].cmptype.dhsz.opnum] == 1) && 
                    //   (g_Y[hc_cmd_copy[array].cmptype.dhsz.opnum + 1] == 0)&& 
                    //   (g_Y[hc_cmd_copy[array].cmptype.dhsz.opnum + 2] == 0))
                    if((plc_get_bit_element_value(Y_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum)   == 1) && 
                       (plc_get_bit_element_value(Y_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum+1) == 0) && 
                       (plc_get_bit_element_value(Y_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum+2) == 0))
                    {
                        return;
                    }
                    //g_Y[hc_cmd_copy[array].cmptype.dhsz.opnum] = 1;
                    //g_Y[hc_cmd_copy[array].cmptype.dhsz.opnum + 1] = 0;
                    //g_Y[hc_cmd_copy[array].cmptype.dhsz.opnum + 2] = 0;
                    plc_set_bit_element_value(Y_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum, 1);
                    plc_set_bit_element_value(Y_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum+1, 0);
                    plc_set_bit_element_value(Y_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum+2, 0);
                    hccmd_output(hc_cmd_copy[array].cmptype.dhsz.opnum, 1); //call the io's output
                    hccmd_output(hc_cmd_copy[array].cmptype.dhsz.opnum+1, 0);
                    hccmd_output(hc_cmd_copy[array].cmptype.dhsz.opnum+2, 0);
                }
                else if(hc_cmd_copy[array].cmptype.dhsz.optype == 2)   		//clear M element
                {
                    //g_M[hc_cmd_copy[array].cmptype.dhsz.opnum] = 1;
                    //g_M[hc_cmd_copy[array].cmptype.dhsz.opnum + 1] = 0;
                    //g_M[hc_cmd_copy[array].cmptype.dhsz.opnum + 2] = 0;
                    plc_set_bit_element_value(M_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum, 1);
                    plc_set_bit_element_value(M_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum+1, 0);
                    plc_set_bit_element_value(M_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum+2, 0);
                }
                else if(hc_cmd_copy[array].cmptype.dhsz.optype == 3)   		//clear S element
                {
                    //g_S[hc_cmd_copy[array].cmptype.dhsz.opnum] = 1;
                    //g_S[hc_cmd_copy[array].cmptype.dhsz.opnum + 1] = 0;
                    //g_S[hc_cmd_copy[array].cmptype.dhsz.opnum + 2] = 0;
                    plc_set_bit_element_value(S_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum, 1);
                    plc_set_bit_element_value(S_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum+1, 0);
                    plc_set_bit_element_value(S_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum+2, 0);
                }
            }  //end of "if(opstate==1)"
            else if(opstate == 2)                                           //great the first parament and less the second parament
            {
                if(hc_cmd_copy[array].cmptype.dhsz.optype == 1)   			//clear Y element
                {
                    //if((g_Y[hc_cmd_copy[array].cmptype.dhsz.opnum] == 0) && 
                    //   (g_Y[hc_cmd_copy[array].cmptype.dhsz.opnum + 1] == 1)&&
                    //   (g_Y[hc_cmd_copy[array].cmptype.dhsz.opnum + 2] == 0))
                    if (plc_get_bit_element_value(Y_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum)   == 0 &&
                        plc_get_bit_element_value(Y_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum+1) == 1 &&
                        plc_get_bit_element_value(Y_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum+2) == 0)
                    {
                        return;
                    }
                    //g_Y[hc_cmd_copy[array].cmptype.dhsz.opnum] = 0;
                    //g_Y[hc_cmd_copy[array].cmptype.dhsz.opnum + 1] = 1;
                    //g_Y[hc_cmd_copy[array].cmptype.dhsz.opnum + 2] = 0;
                    plc_set_bit_element_value(Y_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum, 0);
                    plc_set_bit_element_value(Y_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum+1, 1);
                    plc_set_bit_element_value(Y_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum+2, 0);
                    hccmd_output(hc_cmd_copy[array].cmptype.dhsz.opnum, 0); //call the io's output
                    hccmd_output(hc_cmd_copy[array].cmptype.dhsz.opnum+1, 1);
                    hccmd_output(hc_cmd_copy[array].cmptype.dhsz.opnum+2, 0);
                }
                else if(hc_cmd_copy[array].cmptype.dhsz.optype == 2)   		//clear M element
                {
                    //g_M[hc_cmd_copy[array].cmptype.dhsz.opnum] = 0;
                    //g_M[hc_cmd_copy[array].cmptype.dhsz.opnum + 1] = 1;
                    //g_M[hc_cmd_copy[array].cmptype.dhsz.opnum + 2] = 0;
                    plc_set_bit_element_value(M_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum, 0);
                    plc_set_bit_element_value(M_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum+1, 1);
                    plc_set_bit_element_value(M_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum+2, 0);
                }
                else if(hc_cmd_copy[array].cmptype.dhsz.optype == 3)   		//clear S element
                {
                    //g_S[hc_cmd_copy[array].cmptype.dhsz.opnum] = 0;
                    //g_S[hc_cmd_copy[array].cmptype.dhsz.opnum + 1] = 1;
                    //g_S[hc_cmd_copy[array].cmptype.dhsz.opnum + 2] = 0;
                    plc_set_bit_element_value(S_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum, 0);
                    plc_set_bit_element_value(S_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum+1, 1);
                    plc_set_bit_element_value(S_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum+2, 0);
                }
            }  //end of "else if(opstate==2)"
            else if(opstate == 3)                                           //great the second parament
            {
                if(hc_cmd_copy[array].cmptype.dhsz.optype == 1)   			//clear Y element
                {
                    //if((g_Y[hc_cmd_copy[array].cmptype.dhsz.opnum] == 0) &&
                    //   (g_Y[hc_cmd_copy[array].cmptype.dhsz.opnum + 1] == 0)&& 
                    //   (g_Y[hc_cmd_copy[array].cmptype.dhsz.opnum + 2] == 1))
                    if((plc_get_bit_element_value(Y_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum) == 0) && 
                       (plc_get_bit_element_value(Y_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum+1) == 0)&& 
                       (plc_get_bit_element_value(Y_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum+2) == 1))
                    {
                        return;
                    }
                    //g_Y[hc_cmd_copy[array].cmptype.dhsz.opnum] = 0;
                    //g_Y[hc_cmd_copy[array].cmptype.dhsz.opnum + 1] = 0;
                    //g_Y[hc_cmd_copy[array].cmptype.dhsz.opnum + 2] = 1;
                    plc_set_bit_element_value(Y_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum, 0);
                    plc_set_bit_element_value(Y_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum+1, 0);
                    plc_set_bit_element_value(Y_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum+2, 1);
                    hccmd_output(hc_cmd_copy[array].cmptype.dhsz.opnum, 0);
                    hccmd_output(hc_cmd_copy[array].cmptype.dhsz.opnum+1, 0);
                    hccmd_output(hc_cmd_copy[array].cmptype.dhsz.opnum+2, 1);
                }
                else if(hc_cmd_copy[array].cmptype.dhsz.optype == 2)   		//clear M element
                {
                    //g_M[hc_cmd_copy[array].cmptype.dhsz.opnum] = 0;
                    //g_M[hc_cmd_copy[array].cmptype.dhsz.opnum + 1] = 0;
                    //g_M[hc_cmd_copy[array].cmptype.dhsz.opnum + 2] = 1;
                    plc_set_bit_element_value(M_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum, 0);
                    plc_set_bit_element_value(M_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum+1, 0);
                    plc_set_bit_element_value(M_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum+2, 1);
                }

                else if(hc_cmd_copy[array].cmptype.dhsz.optype == 3)   		//clear S element
                {
                    //g_S[hc_cmd_copy[array].cmptype.dhsz.opnum] = 0;
                    //g_S[hc_cmd_copy[array].cmptype.dhsz.opnum + 1] = 0;
                    //g_S[hc_cmd_copy[array].cmptype.dhsz.opnum + 2] = 1;
                    plc_set_bit_element_value(S_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum, 0);
                    plc_set_bit_element_value(S_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum+1, 0);
                    plc_set_bit_element_value(S_ELEMENT, hc_cmd_copy[array].cmptype.dhsz.opnum+2, 1);
                }
            }  //end of "else if(opstate==3)"
        }
    }
    else                                                                     //DHST or DHSP's instruction doing;
    {
        useno = hc_cmd_table.startaddr + hc_buf[hc_act_no].table_num * 4;
        //if(g_D[useno + 2] < 255)
        if(GET_D_ELEMENT_VALUE(useno + 2) < 255)
        {
            //if(g_D[useno + 3] == 1)
            if(GET_D_ELEMENT_VALUE(useno + 3) == 1)
            {
                //g_Y[g_D[useno + 2]] = 1;
                plc_set_bit_element_value(Y_ELEMENT, GET_D_ELEMENT_VALUE(useno + 2), 1);
                hccmd_output(GET_D_ELEMENT_VALUE(useno + 2), 1);                            //call the io's output
            }
            else if(GET_D_ELEMENT_VALUE(useno + 3) == 0)
            {
                //g_Y[g_D[useno + 2]] = 0;
                plc_set_bit_element_value(Y_ELEMENT, GET_D_ELEMENT_VALUE(useno + 2), 0);
                hccmd_output(GET_D_ELEMENT_VALUE(useno + 2), 0);                            //call the io's output
            }
        }
    }  //end of "else if(hc_cmd_copy[j].cmdtype==4)"
}
#endif

/************************************************************************
function: high_counter action record doing
description:
output  :
************************************************************************/
void hccmd_buf_act(void)
{
    unsigned char save_startp, save_endp, i;
    hc_outF = 0;
    save_startp = start_hcp;
    save_endp = end_hcp;
    if (logFlag) LOGW(TAG, "%s: save_startp = %u, save_endp = %u", __func__, save_startp, save_endp);
    if(save_startp == save_endp)
    {
        return;
    }
    if(save_endp > save_startp)
    {
        for(i = save_startp; i < save_endp; i++)
        {
            hccmd_act(i);
        }
    }
    else
    {
        for(i = save_startp; i < 19; i++)
        {
            hccmd_act(i);
        }
        for(i = 0; i < save_endp; i++)
        {
            hccmd_act(i);
        }
    }
    start_hcp = save_endp;
}

/************************************************************************
function:    high_counter'S action record doing;
description:
input:       count_now, high_counter number; array, doing serial number
output  :
************************************************************************/
void hccmd_in_out(int count_now, unsigned char array)
{
    unsigned char i, j, g;
    g = hc_cmdnum[array];
    for(i = 0; i < g; i++)
    {
        j = hc_array[array][i];
        if(hc_cmd_copy[j].cmdtype == 3)          //DHSZ指令的处理
        {
            if(hc_cmd_copy[j].cmptype.cmp.cmppoint1 > count_now)
            {
                if(hc_cmd_copy[j].opstate != 1)
                {
                    hc_cmd_copy[j].opstate = 1;
                    hc_buf[end_hcp].op_mode = 1;
                    hc_buf[end_hcp].op_act = j;
                    hc_outF = 1;
                    end_hcp++;
                    if(end_hcp >= 19)
                        end_hcp = 0;
                    if(end_hcp == start_hcp)
                    {
                        start_hcp++;
                        if(start_hcp >= 19)
                            start_hcp = 0;
                    }
                }
            }
            else if(hc_cmd_copy[j].cmptype.cmp.cmppoint2 < count_now)
            {
                if(hc_cmd_copy[j].opstate != 3)
                {
                    hc_cmd_copy[j].opstate = 3;
                    hc_buf[end_hcp].op_mode = 3;
                    hc_buf[end_hcp].op_act = j;
                    hc_outF = 1;
                    end_hcp++;
                    if(end_hcp >= 19)
                        end_hcp = 0;
                    if(end_hcp == start_hcp)
                    {
                        start_hcp++;
                        if(start_hcp >= 19)
                            start_hcp = 0;
                    }
                }
            }
            else
            {
                if(hc_cmd_copy[j].opstate != 2)
                {
                    hc_cmd_copy[j].opstate = 2;
                    hc_buf[end_hcp].op_mode = 2;
                    hc_buf[end_hcp].op_act = j;
                    hc_outF = 1;
                    end_hcp++;
                    if(end_hcp >= 19)
                        end_hcp = 0;
                    if(end_hcp == start_hcp)
                    {
                        start_hcp++;
                        if(start_hcp >= 19)
                            start_hcp = 0;
                    }
                }
            }
        }
        else if((hc_cmd_copy[j].cmdtype == 1) || (hc_cmd_copy[j].cmdtype == 2))
        {
            if(hc_cmd_copy[j].cmptype.cmp.cmppoint1 == count_now)
            {
                hc_buf[end_hcp].op_act = j;
                hc_buf[end_hcp].op_mode = 0;
                hc_outF = 1;
                end_hcp++;
                if(end_hcp >= 19)
                    end_hcp = 0;
                if(end_hcp == start_hcp)
                {
                    start_hcp++;
                    if(start_hcp >= 19)
                        start_hcp = 0;
                }
            }
        }
        else if(hc_cmd_copy[j].cmdtype == 4)
        {
            if(hc_cmd_copy[j].cmptype.cmp.cmppoint1 == count_now)
            {
                hc_buf[end_hcp].table_num = hc_cmd_table.curpoint;
                hc_buf[end_hcp].op_mode = 4;
                hc_outF = 1;
                end_hcp++;
                if(end_hcp >= 19)
                    end_hcp = 0;
                if(end_hcp == start_hcp)
                {
                    start_hcp++;
                    if(start_hcp >= 19)
                        start_hcp = 0;
                }
                ref_dhsp_dhst_num(0, &hc_cmd_copy[j]);
            }
        }
        else if(hc_cmd_copy[j].cmdtype == 5)
        {
            if(hc_cmd_copy[j].cmptype.cmp.cmppoint1 == count_now)
                ref_dhsp_dhst_num(1, &hc_cmd_copy[j]);
        }
    }
}

/************************************************************************
function:    (C236,C242,C244)high_counter'S action record doing;
description:
input:       count_now, high_counter number; array, doing serial number
output  :
************************************************************************/
void hc01cmd_in_out(int count_now, unsigned char flag)
{
    if (logFlag) LOGV(TAG, "%s: count_now = %d", __func__, count_now);
    unsigned char i, g, *cmparray;
    if(flag == 0)
    {
        g = hc_cmdnum[0];
        cmparray = &hc_array[0][0];
    }
    else if(flag == 1)
    {
        g = hc_cmdnum[1];
        cmparray = &hc_array[1][0];
    }
    if (logFlag) LOGD(TAG, "g = %u", g);
    
    for(i = 0; i < g; i++)
    {
        if (logFlag) LOGI(TAG, "*(cmparray + %u) = %u", i, *(cmparray + i));
        switch(*(cmparray + i))
        {
        case 0:
        {
            if(hc_cmd_copy[0].cmdtype == 3)          //DHSZ指令的处理
            {
                if(hc_cmd_copy[0].cmptype.cmp.cmppoint1 > count_now)
                {
                    if(hc_cmd_copy[0].opstate != 1)
                    {
                        hc_cmd_copy[0].opstate = 1;
                        hc_buf[end_hcp].op_mode = 1;
                        hc_buf[end_hcp].op_act = 0;
                        hc_outF = 1;
                        end_hcp++;
                        if(end_hcp >= 19)
                            end_hcp = 0;
                        if(end_hcp == start_hcp)
                        {
                            start_hcp++;
                            if(start_hcp >= 19)
                                start_hcp = 0;
                        }
                    }
                }
                else if(hc_cmd_copy[0].cmptype.cmp.cmppoint2 < count_now)
                {
                    if(hc_cmd_copy[0].opstate != 3)
                    {
                        hc_cmd_copy[0].opstate = 3;
                        hc_buf[end_hcp].op_mode = 3;
                        hc_buf[end_hcp].op_act = 0;
                        hc_outF = 1;
                        end_hcp++;
                        if(end_hcp >= 19)
                            end_hcp = 0;
                        if(end_hcp == start_hcp)
                        {
                            start_hcp++;
                            if(start_hcp >= 19)
                                start_hcp = 0;
                        }
                    }
                }
                else
                {
                    if(hc_cmd_copy[0].opstate != 2)
                    {
                        hc_cmd_copy[0].opstate = 2;
                        hc_buf[end_hcp].op_mode = 2;
                        hc_buf[end_hcp].op_act = 0;
                        hc_outF = 1;
                        end_hcp++;
                        if(end_hcp >= 19)
                            end_hcp = 0;
                        if(end_hcp == start_hcp)
                        {
                            start_hcp++;
                            if(start_hcp >= 19)
                                start_hcp = 0;
                        }
                    }  //end of "if(hc_cmd_copy[0].cmptype.cmp.cmppoint1<=count_now)"
                }
            }
            else if((hc_cmd_copy[0].cmdtype == 1) || (hc_cmd_copy[0].cmdtype == 2))
            {
                if(hc_cmd_copy[0].cmptype.cmp.cmppoint1 == count_now)
                {
                    if (logFlag) LOGD(TAG, "hc_cmd_copy[0].cmptype.cmp.cmppoint1 == count_now");
                    hc_buf[end_hcp].op_act = 0;
                    hc_buf[end_hcp].op_mode = 0;
                    hc_outF = 1;
                    end_hcp++;
                    if(end_hcp >= 19)
                        end_hcp = 0;
                    if(end_hcp == start_hcp)
                    {
                        start_hcp++;
                        if(start_hcp >= 19)
                            start_hcp = 0;
                    }
                }
            }
            else if(hc_cmd_copy[0].cmdtype == 4)
            {
                if(hc_cmd_copy[0].cmptype.cmp.cmppoint1 == count_now)
                {
                    hc_buf[end_hcp].table_num = hc_cmd_table.curpoint;
                    hc_buf[end_hcp].op_mode = 4;
                    hc_outF = 1;
                    end_hcp++;
                    if(end_hcp >= 19)
                        end_hcp = 0;
                    if(end_hcp == start_hcp)
                    {
                        start_hcp++;
                        if(start_hcp >= 19)
                            start_hcp = 0;
                    }
                    ref_dhsp_dhst_num(0, &hc_cmd_copy[0]);
                }
            }
            else if(hc_cmd_copy[0].cmdtype == 5)
            {
                if(hc_cmd_copy[0].cmptype.cmp.cmppoint1 == count_now)
                    ref_dhsp_dhst_num(1, &hc_cmd_copy[0]);
            }
        }
        break;
        case 1:
        {
            if(hc_cmd_copy[1].cmdtype == 3)          //DHSZ指令的处理
            {
                if(hc_cmd_copy[1].cmptype.cmp.cmppoint1 > count_now)
                {
                    if(hc_cmd_copy[1].opstate != 1)
                    {
                        hc_cmd_copy[1].opstate = 1;
                        hc_buf[end_hcp].op_mode = 1;
                        hc_buf[end_hcp].op_act = 1;
                        hc_outF = 1;
                        end_hcp++;
                        if(end_hcp >= 19)
                            end_hcp = 0;
                        if(end_hcp == start_hcp)
                        {
                            start_hcp++;
                            if(start_hcp >= 19)
                                start_hcp = 0;
                        }
                    }
                }
                else if(hc_cmd_copy[1].cmptype.cmp.cmppoint2 < count_now)
                {
                    if(hc_cmd_copy[1].opstate != 3)
                    {
                        hc_cmd_copy[1].opstate = 3;
                        hc_buf[end_hcp].op_mode = 3;
                        hc_buf[end_hcp].op_act = 1;
                        hc_outF = 1;
                        end_hcp++;
                        if(end_hcp >= 19)
                            end_hcp = 0;
                        if(end_hcp == start_hcp)
                        {
                            start_hcp++;
                            if(start_hcp >= 19)
                                start_hcp = 0;
                        }
                    }
                }
                else
                {
                    if(hc_cmd_copy[1].opstate != 2)
                    {
                        hc_cmd_copy[1].opstate = 2;
                        hc_buf[end_hcp].op_mode = 2;
                        hc_buf[end_hcp].op_act = 1;
                        hc_outF = 1;
                        end_hcp++;
                        if(end_hcp >= 19)
                            end_hcp = 0;
                        if(end_hcp == start_hcp)
                        {
                            start_hcp++;
                            if(start_hcp >= 19)
                                start_hcp = 0;
                        }
                    }  //end of "if(hc_cmd_copy[0].cmptype.cmp.cmppoint1<=count_now)"
                }
            }
            else if((hc_cmd_copy[1].cmdtype == 1) || (hc_cmd_copy[1].cmdtype == 2))
            {
                if(hc_cmd_copy[1].cmptype.cmp.cmppoint1 == count_now)
                {
                    hc_buf[end_hcp].op_act = 1;
                    hc_buf[end_hcp].op_mode = 0;
                    hc_outF = 1;
                    end_hcp++;
                    if(end_hcp >= 19)
                        end_hcp = 0;
                    if(end_hcp == start_hcp)
                    {
                        start_hcp++;
                        if(start_hcp >= 19)
                            start_hcp = 0;
                    }
                }
            }
            else if(hc_cmd_copy[1].cmdtype == 4)
            {
                if(hc_cmd_copy[1].cmptype.cmp.cmppoint1 == count_now)
                {
                    hc_buf[end_hcp].table_num = hc_cmd_table.curpoint;
                    hc_buf[end_hcp].op_mode = 4;
                    hc_outF = 1;
                    end_hcp++;
                    if(end_hcp >= 19)
                        end_hcp = 0;
                    if(end_hcp == start_hcp)
                    {
                        start_hcp++;
                        if(start_hcp >= 19)
                            start_hcp = 0;
                    }
                    ref_dhsp_dhst_num(0, &hc_cmd_copy[1]);
                }
            }
            else if(hc_cmd_copy[1].cmdtype == 5)
            {
                if(hc_cmd_copy[1].cmptype.cmp.cmppoint1 == count_now)
                    ref_dhsp_dhst_num(1, &hc_cmd_copy[1]);
            }
        }
        break;
        case 2:
        {
            if(hc_cmd_copy[2].cmdtype == 3)          //DHSZ指令的处理
            {
                if(hc_cmd_copy[2].cmptype.cmp.cmppoint1 > count_now)
                {
                    if(hc_cmd_copy[2].opstate != 1)
                    {
                        hc_cmd_copy[2].opstate = 1;
                        hc_buf[end_hcp].op_mode = 1;
                        hc_buf[end_hcp].op_act = 2;
                        hc_outF = 1;
                        end_hcp++;
                        if(end_hcp >= 19)
                            end_hcp = 0;
                        if(end_hcp == start_hcp)
                        {
                            start_hcp++;
                            if(start_hcp >= 19)
                                start_hcp = 0;
                        }
                    }
                }
                else if(hc_cmd_copy[2].cmptype.cmp.cmppoint2 < count_now)
                {
                    if(hc_cmd_copy[2].opstate != 3)
                    {
                        hc_cmd_copy[2].opstate = 3;
                        hc_buf[end_hcp].op_mode = 3;
                        hc_buf[end_hcp].op_act = 2;
                        hc_outF = 1;
                        end_hcp++;
                        if(end_hcp >= 19)
                            end_hcp = 0;
                        if(end_hcp == start_hcp)
                        {
                            start_hcp++;
                            if(start_hcp >= 19)
                                start_hcp = 0;
                        }
                    }
                }
                else
                {
                    if(hc_cmd_copy[2].opstate != 2)
                    {
                        hc_cmd_copy[2].opstate = 2;
                        hc_buf[end_hcp].op_mode = 2;
                        hc_buf[end_hcp].op_act = 2;
                        hc_outF = 1;
                        end_hcp++;
                        if(end_hcp >= 19)
                            end_hcp = 0;
                        if(end_hcp == start_hcp)
                        {
                            start_hcp++;
                            if(start_hcp >= 19)
                                start_hcp = 0;
                        }
                    }  //end of "if(hc_cmd_copy[0].cmptype.cmp.cmppoint1<=count_now)"
                }
            }
            else if((hc_cmd_copy[2].cmdtype == 1) || (hc_cmd_copy[2].cmdtype == 2))
            {
                if(hc_cmd_copy[2].cmptype.cmp.cmppoint1 == count_now)
                {
                    hc_buf[end_hcp].op_act = 2;
                    hc_buf[end_hcp].op_mode = 0;
                    hc_outF = 1;
                    end_hcp++;
                    if(end_hcp >= 19)
                        end_hcp = 0;
                    if(end_hcp == start_hcp)
                    {
                        start_hcp++;
                        if(start_hcp >= 19)
                            start_hcp = 0;
                    }
                }
            }
            else if(hc_cmd_copy[2].cmdtype == 4)
            {
                if(hc_cmd_copy[2].cmptype.cmp.cmppoint1 == count_now)
                {
                    hc_buf[end_hcp].table_num = hc_cmd_table.curpoint;
                    hc_buf[end_hcp].op_mode = 4;
                    hc_outF = 1;
                    end_hcp++;
                    if(end_hcp >= 19)
                        end_hcp = 0;
                    if(end_hcp == start_hcp)
                    {
                        start_hcp++;
                        if(start_hcp >= 19)
                            start_hcp = 0;
                    }
                    ref_dhsp_dhst_num(0, &hc_cmd_copy[2]);
                }
            }
            else if(hc_cmd_copy[2].cmdtype == 5)
            {
                if(hc_cmd_copy[2].cmptype.cmp.cmppoint1 == count_now)
                    ref_dhsp_dhst_num(1, &hc_cmd_copy[2]);
            }
        }
        break;
        case 3:
        {
            if(hc_cmd_copy[3].cmdtype == 3)          //DHSZ指令的处理
            {
                if(hc_cmd_copy[3].cmptype.cmp.cmppoint1 > count_now)
                {
                    if(hc_cmd_copy[3].opstate != 1)
                    {
                        hc_cmd_copy[3].opstate = 1;
                        hc_buf[end_hcp].op_mode = 1;
                        hc_buf[end_hcp].op_act = 3;
                        hc_outF = 1;
                        end_hcp++;
                        if(end_hcp >= 19)
                            end_hcp = 0;
                        if(end_hcp == start_hcp)
                        {
                            start_hcp++;
                            if(start_hcp >= 19)
                                start_hcp = 0;
                        }
                    }
                }
                else if(hc_cmd_copy[3].cmptype.cmp.cmppoint2 < count_now)
                {
                    if(hc_cmd_copy[3].opstate != 3)
                    {
                        hc_cmd_copy[3].opstate = 3;
                        hc_buf[end_hcp].op_mode = 3;
                        hc_buf[end_hcp].op_act = 3;
                        hc_outF = 1;
                        end_hcp++;
                        if(end_hcp >= 19)
                            end_hcp = 0;
                        if(end_hcp == start_hcp)
                        {
                            start_hcp++;
                            if(start_hcp >= 19)
                                start_hcp = 0;
                        }
                    }
                }
                else
                {
                    if(hc_cmd_copy[3].opstate != 2)
                    {
                        hc_cmd_copy[3].opstate = 2;
                        hc_buf[end_hcp].op_mode = 2;
                        hc_buf[end_hcp].op_act = 3;
                        hc_outF = 1;
                        end_hcp++;
                        if(end_hcp >= 19)
                            end_hcp = 0;
                        if(end_hcp == start_hcp)
                        {
                            start_hcp++;
                            if(start_hcp >= 19)
                                start_hcp = 0;
                        }
                    }  //end of "if(hc_cmd_copy[0].cmptype.cmp.cmppoint1<=count_now)"
                }
            }
            else if((hc_cmd_copy[3].cmdtype == 1) || (hc_cmd_copy[3].cmdtype == 2))
            {
                if(hc_cmd_copy[3].cmptype.cmp.cmppoint1 == count_now)
                {
                    hc_buf[end_hcp].op_act = 3;
                    hc_buf[end_hcp].op_mode = 0;
                    hc_outF = 1;
                    end_hcp++;
                    if(end_hcp >= 19)
                        end_hcp = 0;
                    if(end_hcp == start_hcp)
                    {
                        start_hcp++;
                        if(start_hcp >= 19)
                            start_hcp = 0;
                    }
                }
            }
            else if(hc_cmd_copy[3].cmdtype == 4)
            {
                if(hc_cmd_copy[3].cmptype.cmp.cmppoint1 == count_now)
                {
                    hc_buf[end_hcp].table_num = hc_cmd_table.curpoint;
                    hc_buf[end_hcp].op_mode = 4;
                    hc_outF = 1;
                    end_hcp++;
                    if(end_hcp >= 19)
                        end_hcp = 0;
                    if(end_hcp == start_hcp)
                    {
                        start_hcp++;
                        if(start_hcp >= 19)
                            start_hcp = 0;
                    }
                    ref_dhsp_dhst_num(0, &hc_cmd_copy[3]);
                }
            }
            else if(hc_cmd_copy[3].cmdtype == 5)
            {
                if(hc_cmd_copy[3].cmptype.cmp.cmppoint1 == count_now)
                    ref_dhsp_dhst_num(1, &hc_cmd_copy[3]);
            }
        }
        break;
        case 4:
        {
            if(hc_cmd_copy[4].cmdtype == 3)          //DHSZ指令的处理
            {
                if(hc_cmd_copy[4].cmptype.cmp.cmppoint1 > count_now)
                {
                    if(hc_cmd_copy[4].opstate != 1)
                    {
                        hc_cmd_copy[4].opstate = 1;
                        hc_buf[end_hcp].op_mode = 1;
                        hc_buf[end_hcp].op_act = 4;
                        hc_outF = 1;
                        end_hcp++;
                        if(end_hcp >= 19)
                            end_hcp = 0;
                        if(end_hcp == start_hcp)
                        {
                            start_hcp++;
                            if(start_hcp >= 19)
                                start_hcp = 0;
                        }
                    }
                }
                else if(hc_cmd_copy[4].cmptype.cmp.cmppoint2 < count_now)
                {
                    if(hc_cmd_copy[4].opstate != 3)
                    {
                        hc_cmd_copy[4].opstate = 3;
                        hc_buf[end_hcp].op_mode = 3;
                        hc_buf[end_hcp].op_act = 4;
                        hc_outF = 1;
                        end_hcp++;
                        if(end_hcp >= 19)
                            end_hcp = 0;
                        if(end_hcp == start_hcp)
                        {
                            start_hcp++;
                            if(start_hcp >= 19)
                                start_hcp = 0;
                        }
                    }
                }
                else
                {
                    if(hc_cmd_copy[4].opstate != 2)
                    {
                        hc_cmd_copy[4].opstate = 2;
                        hc_buf[end_hcp].op_mode = 2;
                        hc_buf[end_hcp].op_act = 4;
                        hc_outF = 1;
                        end_hcp++;
                        if(end_hcp >= 19)
                            end_hcp = 0;
                        if(end_hcp == start_hcp)
                        {
                            start_hcp++;
                            if(start_hcp >= 19)
                                start_hcp = 0;
                        }
                    }  //end of "if(hc_cmd_copy[0].cmptype.cmp.cmppoint1<=count_now)"
                }
            }
            else if((hc_cmd_copy[4].cmdtype == 1) || (hc_cmd_copy[4].cmdtype == 2))
            {
                if(hc_cmd_copy[4].cmptype.cmp.cmppoint1 == count_now)
                {
                    hc_buf[end_hcp].op_act = 4;
                    hc_buf[end_hcp].op_mode = 0;
                    hc_outF = 1;
                    end_hcp++;
                    if(end_hcp >= 19)
                        end_hcp = 0;
                    if(end_hcp == start_hcp)
                    {
                        start_hcp++;
                        if(start_hcp >= 19)
                            start_hcp = 0;
                    }
                }
            }
            else if(hc_cmd_copy[4].cmdtype == 4)
            {
                if(hc_cmd_copy[4].cmptype.cmp.cmppoint1 == count_now)
                {
                    hc_buf[end_hcp].table_num = hc_cmd_table.curpoint;
                    hc_buf[end_hcp].op_mode = 4;
                    hc_outF = 1;
                    end_hcp++;
                    if(end_hcp >= 19)
                        end_hcp = 0;
                    if(end_hcp == start_hcp)
                    {
                        start_hcp++;
                        if(start_hcp >= 19)
                            start_hcp = 0;
                    }
                    ref_dhsp_dhst_num(0, &hc_cmd_copy[4]);
                }
            }
            else if(hc_cmd_copy[4].cmdtype == 5)
            {
                if(hc_cmd_copy[4].cmptype.cmp.cmppoint1 == count_now)
                    ref_dhsp_dhst_num(1, &hc_cmd_copy[4]);
            }
        }
        break;
        case 5:
        {
            if(hc_cmd_copy[5].cmdtype == 3)          //DHSZ指令的处理
            {
                if(hc_cmd_copy[5].cmptype.cmp.cmppoint1 > count_now)
                {
                    if(hc_cmd_copy[5].opstate != 1)
                    {
                        hc_cmd_copy[5].opstate = 1;
                        hc_buf[end_hcp].op_mode = 1;
                        hc_buf[end_hcp].op_act = 5;
                        hc_outF = 1;
                        end_hcp++;
                        if(end_hcp >= 19)
                            end_hcp = 0;
                        if(end_hcp == start_hcp)
                        {
                            start_hcp++;
                            if(start_hcp >= 19)
                                start_hcp = 0;
                        }
                    }
                }
                else if(hc_cmd_copy[5].cmptype.cmp.cmppoint2 < count_now)
                {
                    if(hc_cmd_copy[5].opstate != 3)
                    {
                        hc_cmd_copy[5].opstate = 3;
                        hc_buf[end_hcp].op_mode = 3;
                        hc_buf[end_hcp].op_act = 5;
                        hc_outF = 1;
                        end_hcp++;
                        if(end_hcp >= 19)
                            end_hcp = 0;
                        if(end_hcp == start_hcp)
                        {
                            start_hcp++;
                            if(start_hcp >= 19)
                                start_hcp = 0;
                        }
                    }
                }
                else
                {
                    if(hc_cmd_copy[5].opstate != 2)
                    {
                        hc_cmd_copy[5].opstate = 2;
                        hc_buf[end_hcp].op_mode = 2;
                        hc_buf[end_hcp].op_act = 5;
                        hc_outF = 1;
                        end_hcp++;
                        if(end_hcp >= 19)
                            end_hcp = 0;
                        if(end_hcp == start_hcp)
                        {
                            start_hcp++;
                            if(start_hcp >= 19)
                                start_hcp = 0;
                        }
                    }  //end of "if(hc_cmd_copy[0].cmptype.cmp.cmppoint1<=count_now)"
                }
            }
            else if((hc_cmd_copy[5].cmdtype == 1) || (hc_cmd_copy[5].cmdtype == 2))
            {
                if(hc_cmd_copy[5].cmptype.cmp.cmppoint1 == count_now)
                {
                    hc_buf[end_hcp].op_act = 5;
                    hc_buf[end_hcp].op_mode = 0;
                    hc_outF = 1;
                    end_hcp++;
                    if(end_hcp >= 19)
                        end_hcp = 0;
                    if(end_hcp == start_hcp)
                    {
                        start_hcp++;
                        if(start_hcp >= 19)
                            start_hcp = 0;
                    }
                }
            }
            else if(hc_cmd_copy[5].cmdtype == 4)
            {
                if(hc_cmd_copy[5].cmptype.cmp.cmppoint1 == count_now)
                {
                    hc_buf[end_hcp].table_num = hc_cmd_table.curpoint;
                    hc_buf[end_hcp].op_mode = 4;
                    hc_outF = 1;
                    end_hcp++;
                    if(end_hcp >= 19)
                        end_hcp = 0;
                    if(end_hcp == start_hcp)
                    {
                        start_hcp++;
                        if(start_hcp >= 19)
                            start_hcp = 0;
                    }
                    ref_dhsp_dhst_num(0, &hc_cmd_copy[5]);
                }
            }
            else if(hc_cmd_copy[5].cmdtype == 5)
            {
                if(hc_cmd_copy[5].cmptype.cmp.cmppoint1 == count_now)
                    ref_dhsp_dhst_num(1, &hc_cmd_copy[5]);
            }
        }
        break;
        default:
            break;
        }
    }
}

/************************************************************************
function:脉冲捕捉硬件初始化
description:
input   : no;
output  : no;
************************************************************************/
void pulse_capture_init(void)
{
    cancel_capture(0, 1);
    cancel_capture(1, 1);
    cancel_capture(2, 1);
    cancel_capture(3, 1);
    cancel_capture(4, 1);
    cancel_capture(5, 1);
    cancel_capture(6, 1);
    cancel_capture(7, 1);
}

/************************************************************************
function: 高速计数指令的上电初始化程序
description: 对上电需要初始化的操作处理
input   : no;
output  : no;
************************************************************************/
void highcount_poweron_init(void)
{
#if (KALYKE_HIGH_SPEED_IO == 0)
    unsigned char *clearp, i, j;
    unsigned int k;
    hccmdall = 0;           //高速指令的总个数清0
    //清除高速属性结构描述
    for(i = 0; i < 6; i++)
    {
        clearp = (unsigned char *)&use_hc_attr[i];
        for(j = 0; j < sizeof(use_hc_attr[i]); j++)
            *clearp++ = 0;
    }
    uint32_t cmdCopySize = sizeof(hc_cmd_copy[0]);
    if (logFlag) LOGV(TAG, "cmdCopySize = %u", cmdCopySize);
    //清除指令保存结构
    for(i = 0; i < 6; i++)
    {
        clearp = (unsigned char *)&hc_cmd_copy[i];
        for(j = 0; j < cmdCopySize; j++)
        {
            *clearp++ = 0;
        }
        hc_array[0][i] = 0;
        hc_array[1][i] = 0;
        hc_array[2][i] = 0;
        hc_array[3][i] = 0;
        hc_array[4][i] = 0;
        hc_array[5][i] = 0;
        hc_cmdnum[i] = 0;

        hc_cmd_copy[i].cmdno = 0xFF;
    }
    //高速命令缓冲的处理
    for(i = 0; i < 19; i++)
    {
        hc_buf[i].op_mode = 0;
        hc_buf[i].op_act = 0;
    }
    start_hcp = 0;
    end_hcp = 0;
    hc_outF = 0;
    //清除表格记录
#if 0
    clearp = (unsigned char *)&hc_cmd_table;
    for(j = 0; j < sizeof(hc_cmd_table); j++)
    {
        *clearp++ = 0;
    }
#else
    memset(&hc_cmd_table, 0, sizeof(hc_cmd_table));
    hc_cmd_table.cmdno = 0xFF;
#endif
    //SPD操作的相关初始化
    spd_flag.BYTE = 0;
    for(i = 0; i < 6; i++)
    {
        spd_time_save[i] = 0;
    }
    //外部中断的相关初始化
    intr_flag.BYTE = 0;
    for(i = 0; i < 8; i++)
    {
        intr_num_save[i] = 0;
    }
    catch_flag.BYTE = 0;

#if 0 // LiXianyu 20191217
    //HOUR指令处理参数
    for(k = 0; k <= 255; k++)
        hour_str[k].up_state = 0;
    //从EEPROM中读出高速数值
#endif
#endif
}

/************************************************************************
function: 高速计数器停机处理程序
description: 对上电需要初始化的操作处理
input   : no;
output  : no;
************************************************************************/
#if 0 // LiXianyu 20191216
void highcount_switchoff_init(void)
{
    GIOENA1 &= 0xFFFFFF0F;                                          //disable X0,X2,X3,X5rise input interrupt
    HET_L14_0.memory.control_word &= 0xFFFFFFFE;                    //X001
    HET_L13_0.memory.control_word &= 0xFFFFFFFE;                    //disable X7 rise and fall input interrupt
    HET_L15_0.memory.control_word &= 0xFFFFFFFE;                    //disable X4 rise input interrupt
    cancel_capture(0, 0);
    cancel_capture(1, 0);
    cancel_capture(2, 0);
    cancel_capture(3, 0);
    cancel_capture(4, 0);
    cancel_capture(5, 0);
    cancel_capture(6, 0);
    cancel_capture(7, 0);
    highcount_poweron_init();
}
#else
void highcount_poweron_deinit(void)
{
    LOGV(TAG, "Enter %s()", __func__);
    cancel_capture(0, 0);
    cancel_capture(1, 0);
    cancel_capture(2, 0);
    cancel_capture(3, 0);
    cancel_capture(4, 0);
    cancel_capture(5, 0);
    cancel_capture(6, 0);
    cancel_capture(7, 0);
}
#endif

