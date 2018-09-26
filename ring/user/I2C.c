#include "HC89F0650.h"
#include "Uart.h"
#include "I2C.h"
#include "Timer.h"

#define	DEVICEADD	0x24  				//器件地址
#define	STA	0x20		  				//起始位
#define	STO 0x10		  				//停止位
#define	SI	0x08 		  				//IIC串行中断标志
#define	AA	0X04		  				//应答标志

sbit SCL=P2^7;            				//模拟I2C时钟控制位
sbit SDA=P4^4;            				//模拟I2C数据传送位


void IIC_Init(void)
{
	P2M3 = 0xA3;
	P4M2 = 0x3A;
	SCL_MAP = 0x17;
	SDA_MAP = 0x24;
	IICCON = 0x40;   
}

void IIC_Send_Byte(unsigned char data add,unsigned char data dat)
{
	IICCON &=~ SI;
	IICCON |= STA; 			//启动
	while((!(IICCON&SI))&&(IICSTA!=0x08));
	IICCON &=~ STA;
	IICCON &=~ SI;
	IICDAT = DEVICEADD;
	while(IICSTA != 0x18);
	IICCON &=~ SI;
	IICDAT = add;
	while(IICSTA!=0x28);
	IICCON &=~ SI;
	IICDAT = dat;
	while(IICSTA!=0x28);
	IICCON &=~ SI;
	IICCON |= STO; 
}

unsigned char IIC_Read_Byte(unsigned char data add)
{
	unsigned char data dat;

	IICCON &=~ SI;
	IICCON |= STA; 			//启动
	while((!(IICCON&SI))&&(IICSTA!=0x08));
	IICCON &=~ STA;
	IICCON &=~ SI;
	IICDAT = DEVICEADD;		//写入写指令
	while(IICSTA!=0x18);
	IICCON &=~ SI;
	IICDAT = add;			//写入读地址
	while(IICSTA!=0x28);
	IICCON &=~ SI;
	IICCON |= STA; 			//启动
	while((!(IICCON&SI))&&(IICSTA!=0x08));
	IICCON &=~ STA;
	IICCON &=~ SI;
	IICDAT = DEVICEADD|0x01;//写入读指令
	while(IICSTA!=0x40);
	IICCON &=~ SI;
	while(IICSTA!=0x58); 
	dat = IICDAT;			//读入数据
	IICCON &=~ SI;
	IICCON |= STO;
	return	dat;
}


