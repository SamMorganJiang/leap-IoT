#include "HC89F0650.h"
#include "Uart.h"
#include "I2C_SLAVE.h"
#include "Timer.h"

#define	DEVICEADDSLAVE	0x24  				//������ַ
#define	STA	0x20		  				//��ʼλ
#define	STO 0x10		  				//ֹͣλ
#define	SI	0x08 		  				//IIC�����жϱ�־
#define	AA	0X04		  				//Ӧ���־
#define CLR 0x00              //SI����

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
	IICCON |= STA; 			//����
	while((!(IICCON&SI))&&(IICSTA!=0x08));
	IICCON &=~ STA;
	IICCON &=~ SI;
	IICDAT = DEVICEADDSLAVE;		//д��дָ��
	while(IICSTA!=0x18);
	IICCON &=~ SI;
	IICDAT = add;			//д�����ַ
	while(IICSTA!=0x28);
	IICCON &=~ SI;
	IICCON |= STA; 			//����
	while((!(IICCON&SI))&&(IICSTA!=0x08));
	IICCON &=~ STA;
	IICCON &=~ SI;
	IICDAT = DEVICEADDSLAVE|0x01;//д���ָ��
	while(IICSTA!=0x40);
	IICCON &=~ SI;
	while(IICSTA!=0x58); 
	dat = IICDAT;			//��������
	IICCON &=~ SI;
	IICCON |= STO;
	return	dat;
}