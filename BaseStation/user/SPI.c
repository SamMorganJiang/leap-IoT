#include "HC89F0650.h"
#include "intrins.h"
#include "SPI.h"
#include "Timer.h"
#include "MG127.h"
#include "Uart.h"

sbit	CSN=P0^0;										//P00???CS
sbit	SCK=P0^1;										//P03???SCK
sbit	IRQ=P4^2;                   //P05???IRQ
sbit	SI=P0^2;										//P02???SI
sbit	SO=P0^2;	


void SPI_Init(void)
{
	P0M0=0xcc;	
	P0M2=0x33;
	P4M1 =0x80;
	CSN =1;
}

void SPI_Write_Byte(int data1) 
{ 
	int  i;
	P0M1=0x6c;
	for(i=0;i<8;i++)
	{
		SCK = 0;
		_nop_();
		_nop_();
		if(data1&0x80)
		{
			SI = 1;
		}
	 	else
		{
			SI = 0;
		}

		_nop_();
		_nop_();
		_nop_();
		_nop_();
		SCK = 1;
		data1<<=1;
		
   	}
	SCK = 0;
	SI = 1;
} 


u8 SPI_Read_Byte(void) 
{ 
	u8 ret = 0;
	u8 i = 0;
	
	 P0M1=0xc6;	
	for(i=0;i<8;i++)
	{
		SCK = 0;
		_nop_();
		_nop_();
		SCK = 1;
		ret<<=1;
		if(SI != 0)
		{
			ret|=1;
		}
	}
	P0M1=0x6c;	
	SCK = 0;
	return ret;
} 

 

void SPI_Write_Reg(u8 reg, u8 datae)
{ 
	CSN = 0;
	//delay(5);
	SPI_Write_Byte(reg);
	SPI_Write_Byte(datae);
  //delay(5);
	CSN = 1;
} 


u8 SPI_Read_Reg(u8 reg) 
{ 
	u8 data temp0=0;
	
    CSN = 0;
	//delay(5);
	SPI_Write_Byte(reg);
	temp0 = SPI_Read_Byte();
	//delay(5);
	CSN = 1;
	return temp0;
} 

void SPI_Write_Buffer(u8 reg, u8 *dataBuf, u8 len) 
{ 
	u8 data temp0=0;
	
    CSN = 0;
	
	//delay(5);
	SPI_Write_Byte(reg|0x20);

	for(temp0=0;temp0<len;temp0++)
	{
		SPI_Write_Byte(*dataBuf);
		dataBuf++;
	}
	
	//delay(5);

	CSN = 1;
} 

void SPI_Read_Buffer(u8 reg, u8 *dataBuf, u8 len) 
{ 
    u8  temp0=0;
	
    CSN = 0;
	//delay(5);
	
	SPI_Write_Byte(reg);

	for(temp0=0;temp0<len;temp0++)
	{
		//*(dataBuf+temp0)=SPI_Read_Byte();
		dataBuf[temp0]=SPI_Read_Byte();
	}
	//delay(5);

	CSN = 1;
}

u8 mcu_get_irq(void)
{
	if(IRQ == 0)
		return 1;
	else
		return 0;
}
