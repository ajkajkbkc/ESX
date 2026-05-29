一．简单指令翻译(小段格式)：
#define SI_LD_X     (0x10)          2字节(X值小于255)
    eg:     LD X1
    ucode:  0x01  0x10    
#define SI_LD_Y     (0x11)			2字节
#define SI_LD_C     (0x12)          2字节
#define SI_LD_T     (0x13)          2字节
#define SI_LD_SM    (0x14)          2字节
#define SI_LD_M     (0x15)          2字节
#define SI_LD_S     (0x16)          2字节
#define SI_LD_LM    (0x17)          2字节
#define SI_LD_X_EXT     (0x18)          4字节
    eg:     LD X257
    ucode:  0x01        0x18        (0x16 0x00)
            (257&0x0f)  (功能码)    (257>>4)
            子变量                   母变量
#define SI_LD_Y_EXT     (0x19)          4字节
#define SI_LD_C_EXT     (0x1A)          4字节
#define SI_LD_T_EXT     (0x1B)          4字节
#define SI_LD_SM_EXT    (0x1C)          4字节
#define SI_LD_M_EXT     (0x1D)          4字节
#define SI_LD_S_EXT     (0x1E)          4字节

#define SI_LDI_X     (0x20)          2字节(X值小于255)
    eg:     LDI X1
    ucode:  0x01  0x20
#define SI_LDI_Y     (0x21)          2字节
#define SI_LDI_C     (0x22)          2字节
#define SI_LDI_T     (0x23)          2字节
#define SI_LDI_SM    (0x24)          2字节
#define SI_LDI_M     (0x25)          2字节
#define SI_LDI_S     (0x26)          2字节
#define SI_LDI_LM    (0x27)          2字节
#define SI_LDI_X_EXT     (0x28)          4字节
    eg:     LDI X257
    ucode:  0x01        0x28        (0x00 0x16)
            (257&0x0f)  (功能码)    (257>>4)
            子变量                   母变量
#define SI_LDI_Y_EXT     (0x29)          4字节
#define SI_LDI_C_EXT     (0x2A)          4字节
#define SI_LDI_T_EXT     (0x2B)          4字节
#define SI_LDI_SM_EXT    (0x2C)          4字节
#define SI_LDI_M_EXT     (0x2D)          4字节
#define SI_LDI_S_EXT     (0x2E)          4字节

#define SI_AND_X     (0x30)          2字节(X值小于255)
	eg:     AND X1
	ucode:  0x01  0x30   
#define SI_AND_Y     (0x31)          2字节
#define SI_AND_C     (0x32)          2字节
#define SI_AND_T     (0x33)          2字节
#define SI_AND_SM    (0x34)          2字节
#define SI_AND_M     (0x35)          2字节
#define SI_AND_S     (0x36)          2字节
#define SI_AND_LM    (0x37)          2字节
#define SI_AND_X_EXT     (0x38)          4字节
    eg:     AND X257
    ucode:  0x01        0x38        (0x00 0x16)
            (257&0x0f)  (功能码)    (257>>4)
            子变量                   母变量
#define SI_AND_Y_EXT     (0x39)          4字节
#define SI_AND_C_EXT     (0x3A)          4字节
#define SI_AND_T_EXT     (0x3B)          4字节
#define SI_AND_SM_EXT    (0x3C)          4字节
#define SI_AND_M_EXT     (0x3D)          4字节
#define SI_AND_S_EXT     (0x3E)          4字节

#define SI_ANI_X     (0x40)          2字节(X值小于255)
	eg:     ANI X1
	ucode:  0x01  0x40   
#define SI_ANI_Y     (0x41)          2字节
#define SI_ANI_C     (0x42)          2字节
#define SI_ANI_T     (0x43)          2字节
#define SI_ANI_SM    (0x44)          2字节
#define SI_ANI_M     (0x45)          2字节
#define SI_ANI_S     (0x46)          2字节
#define SI_ANI_LM    (0x47)          2字节
#define SI_ANI_X_EXT     (0x48)          4字节
    eg:     ANI X257
    ucode:  0x01        0x48        (0x00 0x16)
            (257&0x0f)  (功能码)    (257>>4)
            子变量                   母变量
#define SI_ANI_Y_EXT     (0x49)          4字节
#define SI_ANI_C_EXT     (0x4A)          4字节
#define SI_ANI_T_EXT     (0x4B)          4字节
#define SI_ANI_SM_EXT    (0x4C)          4字节
#define SI_ANI_M_EXT     (0x4D)          4字节
#define SI_ANI_S_EXT     (0x4E)          4字节

#define SI_OR_X     (0x50)          2字节(X值小于255)
	eg:     OR X1
	ucode:  0x01  0x50   
#define SI_OR_Y     (0x51)          2字节
#define SI_OR_C     (0x52)          2字节
#define SI_OR_T     (0x53)          2字节
#define SI_OR_SM    (0x54)          2字节
#define SI_OR_M     (0x55)          2字节
#define SI_OR_S     (0x56)          2字节
#define SI_OR_LM    (0x57)          2字节
#define SI_OR_X_EXT     (0x58)          4字节
    eg:     OR X257
    ucode:  0x01        0x58        (0x00 0x16)
            (257&0x0f)  (功能码)    (257>>4)
            子变量                   母变量
#define SI_OR_Y_EXT     (0x59)          4字节
#define SI_OR_C_EXT     (0x5A)          4字节
#define SI_OR_T_EXT     (0x5B)          4字节
#define SI_OR_SM_EXT    (0x5C)          4字节
#define SI_OR_M_EXT     (0x5D)          4字节
#define SI_OR_S_EXT     (0x5E)          4字节

#define SI_ORI_X     (0x60)          2字节(X值小于255)
	eg:     ORI X1
	ucode:  0x01  0x60   
#define SI_ORI_Y     (0x61)          2字节
#define SI_ORI_C     (0x62)          2字节
#define SI_ORI_T     (0x63)          2字节
#define SI_ORI_SM    (0x64)          2字节
#define SI_ORI_M     (0x65)          2字节
#define SI_ORI_S     (0x66)          2字节
#define SI_ORI_LM    (0x67)          2字节
#define SI_ORI_X_EXT     (0x68)          4字节
    eg:     ORI X257
    ucode:  0x01        0x68        (0x00 0x16)
            (257&0x0f)  (功能码)    (257>>4)
            子变量                   母变量
#define SI_ORI_Y_EXT     (0x69)          4字节
#define SI_ORI_C_EXT     (0x6A)          4字节
#define SI_ORI_T_EXT     (0x6B)          4字节
#define SI_ORI_SM_EXT    (0x6C)          4字节
#define SI_ORI_M_EXT     (0x6D)          4字节
#define SI_ORI_S_EXT     (0x6E)          4字节

#define SI_OUT_Y     (0x71)          2字节(Y值小于255)
	eg:     OUT Y1
	ucode:  0x01  0x71 
#define SI_OUT_SM    (0x74)          2字节
#define SI_OUT_M     (0x75)          2字节
#define SI_OUT_LM    (0x77)          2字节
#define SI_OUT_Y_EXT     (0x79)          4字节
    eg:     OUT Y257
    ucode:  0x01        0x79        (0x00 0x16)
            (257&0x0f)  (功能码)    (257>>4)
            子变量                   母变量
#define SI_OUT_SM_EXT    (0x7C)          4字节
#define SI_OUT_M_EXT     (0x7D)          4字节

#define SI_SET_Y     (0x81)          2字节(Y值小于255)
	eg:     SET Y1
	ucode:  0x01  0x81
#define SI_SET_SM    (0x84)          2字节
#define SI_SET_M     (0x85)          2字节
#define SI_SET_LM    (0x87)          2字节
#define SI_SET_Y_EXT     (0x89)          4字节
    eg:     SET Y257
    ucode:  0x01        0x89        (0x00 0x16)
            (257&0x0f)  (功能码)    (257>>4)
            子变量                   母变量
#define SI_SET_SM_EXT    (0x8C)          4字节
#define SI_SET_M_EXT     (0x8D)          4字节

#define SI_RST_Y     (0x91)          2字节(Y值小于255)
	eg:     RST Y1
	ucode:  0x01  0x91
#define SI_RST_SM    (0x94)          2字节
#define SI_RST_M     (0x95)          2字节
#define SI_RST_LM    (0x97)          2字节
#define SI_RST_Y_EXT     (0x99)          4字节
    eg:     RST Y257
    ucode:  0x01        0x99        (0x00 0x16)
            (257&0x0f)  (功能码)    (257>>4)
            子变量                   母变量
#define SI_RST_SM_EXT    (0x9C)          4字节
#define SI_RST_M_EXT     (0x9D)          4字节

#define SI_INV     (0xa0)          2字节
    eg:     INV
    ucode:  0x00 0xa0
#define SI_ANB 	   (0xa1)          2字节
    eg:     ANB
    ucode:  0x00 0xa1
#define SI_ORB	   (0xa2)          2字节
    eg:     ORB
    ucode:  0x00 0xa2
#define SI_MPS	   (0xA3)          2字节
    eg:     MPS
    ucode:  0x00 0xa3
#define SI_MRD     (0xA4)          2字节
    eg:     MRD
    ucode:  0x00 0xa4
#define SI_MPP     (0xA5)          2字节
    eg:     MPP
    ucode:  0x00 0xa5
#define SI_NOP     (0xA6)          2字节
    eg:     NOP
    ucode:  0x00 0xa6
#define SI_EU      (0xA7)               4字节
    eg:     EU
    ucode:  0x00 0xa7   (分配的沿号,unsigned short 小段格式)
#define SI_ED      (0xA8)               4字节
    eg:     ED
    ucode:  0x00 0xa8   (分配的沿号,unsigned short 小段格式)

#define LDP X,Y,C,T,SM,M,S    (0xB0)        6字节            (小段格式)                (小段格式)
    eg:     LDP  X     :(0x00寻址X)     (0xB0功能码)    (0xXXXX 两字节元件编号)     (0xXXXX 两字节沿号)
              LDP  Y     :(0x01寻址Y)    (0xB0功能码)    (0xXXXX 两字节元件编号)     (0xXXXX 两字节沿号)
              LDP  C     :(0x02寻址C)    (0xB0功能码)    (0xXXXX 两字节元件编号)     (0xXXXX 两字节沿号)
              LDP  T     :(0x03寻址T)     (0xB0功能码)    (0xXXXX 两字节元件编号)     (0xXXXX 两字节沿号)
              LDP  SM  :(0x04寻址SM)  (0xB0功能码)    (0xXXXX 两字节元件编号)     (0xXXXX 两字节沿号)
              LDP  M    :(0x05寻址M)    (0xB0功能码)    (0xXXXX 两字节元件编号)     (0xXXXX 两字节沿号)
              LDP  S     :(0x06寻址S)     (0xB0功能码)    (0xXXXX 两字节元件编号)     (0xXXXX 两字节沿号)
#define LDF    X,Y,C,T,SM,M,S       (0xB1)  6字节

#define ANDP X,Y,C,T,SM,M,S     (0xB2)  6字节
#define ANDF X,Y,C,T,SM,M,S     (0xB3)  6字节

#define ORP X,Y,C,T,SM,M,S        (0xB4)  6字节
#define ORF X,Y,C,T,SM,M,S        (0xB5）6字节

#define PLP Y,SM,M               (0xB6)  6字节   
    eg:      PLP  Y      :(0x01寻址Y)     (0xB5功能码)    (0xXXXX 两字节元件编号)     (0xXXXX 两字节沿号)
              PLP  SM    :(0x04寻址SM)    (0xB5功能码)    (0xXXXX 两字节元件编号)     (0xXXXX 两字节沿号)
              PLP  M      :(0x05寻址M)     (0xB5功能码)    (0xXXXX 两字节元件编号)     (0xXXXX 两字节沿号)
              PLP  LM    :(0x07寻址LM)    (0xB5功能码)    (0xXXXX 两字节元件编号)     (0xXXXX 两字节沿号)        
   
#define PLF Y,SM,M,LM           (0xB7)  6字节    
    eg:     OUTF  Y     :(0x01寻址Y)     (0xBE功能码)    (0xXXXX 两字节元件编号)     (0xXXXX 两字节沿号)
            OUTF  SM    :(0x04寻址SM)    (0xBE功能码)    (0xXXXX 两字节元件编号)     (0xXXXX 两字节沿号)
            OUTF  M     :(0x05寻址M)     (0xBE功能码)    (0xXXXX 两字节元件编号)     (0xXXXX 两字节沿号)
