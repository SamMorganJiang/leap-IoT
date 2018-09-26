#include "HC89F0650.h"
#include "Uart.h"
#include "I2C_SLAVE.h"
#include "Timer.h"

#define	DEVICEADDSLAVE	0x24  				//器件地址
#define	STA	0x20		  				//起始位
#define	STO 0x10		  				//停止位
#define	SI	0x08 		  				//IIC串行中断标志
#define	AA	0X04		  				//应答标志
#define CLR 0x00              //SI清零

sbit SCL_S=P1^6;
sbit SDA_S=P1^5;

void IIC_Slave_Init(void)
{
	P1M2 = 0x23;
	P1M3 = 0x32;
	SCL_MAP = 0x31;
	SDA_MAP = 0x30;
	IICCON = 0x44;
  IICADR = DEVICEADDSLAVE;	
}

void IIC_Slave_Recive_Byte(unsigned char data add,unsigned char data dat)
{
	IICCON &=~ SI;
	while((IICCON != 0x60));
	while((IICCON != 0x68));
	IICCON &=~ SI;
	add = IICDAT;
	while((IICCON != 0x80));
	IICCON &=~ SI;
	dat = IICDAT;
}


unsigned char IIC_Slave_Read_Byte(unsigned char data add)
{
	unsigned char data dat;

	IICCON &=~ SI;
	IICCON |= STA; 			//启动
	while((!(IICCON&SI))&&(IICSTA!=0x08));
	IICCON &=~ STA;
	IICCON &=~ SI;
	IICDAT = DEVICEADDSLAVE;		//写入写指令
	while(IICSTA!=0x18);
	IICCON &=~ SI;
	IICDAT = add;			//写入读地址
	while(IICSTA!=0x28);
	IICCON &=~ SI;
	IICCON |= STA; 			//启动
	while((!(IICCON&SI))&&(IICSTA!=0x08));
	IICCON &=~ STA;
	IICCON &=~ SI;
	IICDAT = DEVICEADDSLAVE|0x01;//写入读指令
	while(IICSTA!=0x40);
	IICCON &=~ SI;
	while(IICSTA!=0x58); 
	dat = IICDAT;			//读入数据
	IICCON &=~ SI;
	IICCON |= STO;
	return	dat;
}