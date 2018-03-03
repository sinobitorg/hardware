#include "MicroBit.h"

MicroBit uBit;

/******************指令表/Instructions**********************/
#define SYS_DIS 0x00                //关闭系统震荡/Turn off system shock
#define SYS_EN  0x01                //打开系统震荡/Turn on  system shock
#define LED_OFF 0x02                //关闭LED显示/Turn off LED display
#define LED_ON  0x03                //打开LED显示/Turn on LED display
#define BLINK_OFF   0x08            //关闭闪烁   /Close blink
#define BLINK_ON    0x09            //打开闪烁   /Open blink            
#define SLAVE_MODE  0x10            //从模式     /Slave mode
#define RC_MASTER_MODE  0x18        //内部RC时钟 /Use internal clock 
#define COM_OPTION  0x24            //24:16COM，NMOS。C：COM，PMOS。
#define PWM_DUTY    0xAF            //PWM亮度控制 /PWM Brightness Control
/****************引脚定义 I/O definition**********************/

DigitalOut HT_CS(P0_16);
DigitalOut HT_RD(P0_22);
DigitalOut HT_WR(P0_23);
DigitalOut HT_DAT(P0_21);

static unsigned char show[24] = {
  0b00011111, 0b10000000,
  0b00100000, 0b01000000,
  0b01001011, 0b00100000,
  0b10000000, 0b00010000,
  0b10100010, 0b01010000,
  0b10101110, 0b00010000,
  0b10000111, 0b01010000,
  0b10100100, 0b01010000,
  0b10000000, 0b00010000,
  0b01001101, 0b00100000,
  0b00100000, 0b01000000,
  0b00011111, 0b10000000
};

static unsigned char com[12] = {0x00,0x04,0x08,0x0C,0x10,0x14,0x18,0x1C,0x20,0x24,0x28,0x2C};

void HT1632C_Write(unsigned char Data,unsigned char cnt)      //MCU向HT1632C写数据函数，高位在前/MCU writes the data to ht1632c, and the high position is in front
{
    unsigned char i;
    for(i=0; i<cnt; i++) {
        HT_WR=0;
        if(Data&0x80)
            HT_DAT=1;
        else
            HT_DAT=0;
        Data<<=1;
        HT_WR=1;
    }
}
void HT1632C_Write_CMD(unsigned char cmd)                     //MCU向HT1632c写命令/MCU writes commands to ht1632c
{
    HT_CS=0;
    HT1632C_Write(0x80,3);                                    //ID:100
    HT1632C_Write(cmd,9);
    HT_CS=1;
}

void HT1632C_Write_DAT(unsigned char Addr,const unsigned char data[],unsigned char num)
{
    unsigned char d;
    unsigned char i;
    HT_CS=0;
    HT1632C_Write(0xa0,3);                                    //ID:101
    HT1632C_Write(Addr<<1,7);

    for(d=data[2*(num-1)], i=0; i<8; i++) {
        HT_WR=0;
        if(d&0x80)
            HT_DAT=1;
        else
            HT_DAT=0;
        d<<=1;
        HT_WR=1;
    }

    for(d=data[2*num-1], i=0; i<4; i++) {
        HT_WR=0;
        if(d&0x80)
            HT_DAT=1;
        else
            HT_DAT=0;
        d<<=1;
        HT_WR=1;
    }

    HT_CS=1;
}

void HT1632C_clr(void)  //清屏函数/Clear function
{
    unsigned char i;
    HT_CS=0;
    HT1632C_Write(0xa0,3);
    HT1632C_Write(0x00,7);
    for(i=0; i<48; i++)
        HT1632C_Write(0,8);
    HT_CS=1;
}
void HT1632C_Init(void)                 //HT1632C初始化函数/HT1632C Init Function
{
    HT_CS=1;
    HT_WR=1;
    HT_DAT=1;
    HT1632C_Write_CMD(SYS_DIS);         //关闭HT1632C内部RC时钟/Close the HT1632C internal clock
    HT1632C_Write_CMD(COM_OPTION);      //选择HT1632C工作模式/Select HT1632C work mode
    HT1632C_Write_CMD(RC_MASTER_MODE);  //选择内部RC时钟/Select internal clock
    HT1632C_Write_CMD(SYS_EN);          //打开HT1632C内部RC时钟/Open the HT1632C internal clock
    HT1632C_Write_CMD(PWM_DUTY);        //PWM亮度设置/Init the PWM Brightness
    HT1632C_Write_CMD(BLINK_OFF);       //关闭闪烁   /Close blink
    HT1632C_Write_CMD(LED_ON);          //打开LED显示/ Turn on LED display
}


void HT1632C_Read_DATA(unsigned char Addr)
{
    unsigned char i;
    HT_CS=0;
    HT1632C_Write(0xc0,3);                                    //ID:101
    HT1632C_Write(Addr<<1,7);
    for(i=0; i<12; i++) {
        HT_RD=0;
        asm("nop");
        HT_RD=1;
    }
    HT_CS=1;
}

int main()
{
    HT1632C_Init();
    HT1632C_clr();
    uBit.init();
    for(int i=0; i<12; i++) {
        HT1632C_Write_DAT(com[i],show,i+1);
    }
    wait(0.5);
    while(1) {
        for(int i=0; i<12; i++) {
            HT1632C_Read_DATA(com[i]);
        }
    }
}

