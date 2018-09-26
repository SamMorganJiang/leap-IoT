#include "HC89F0650.h"
#include "Uart.h"
#include "I2C.h"
#include "Timer.h"
#include "string.h"

#define	DEVICEADD	0x24  				//������ַ
#define	STA	0x20		  				//��ʼλ
#define	STO 0x10		  				//ֹͣλ
#define	SI	0x08 		  				//IIC�����жϱ�־
#define	AA	0X04		  				//Ӧ���־

sbit SCL=P2^7;            				//ģ��I2Cʱ�ӿ���λ
sbit SDA=P4^4;            				//ģ��I2C���ݴ���λ


void IIC_Init(void)
{
	P2M3 = 0xA3;
	P4M2 = 0x3A;
	SCL_MAP = 0x17;
	SDA_MAP = 0x24;
	IICCON = 0x40;   
}

void IIC_Send_Byte(unsigned char data addr,
					unsigned char data val)
{
	IICCON &=~ SI;
	IICCON |= STA; 			//����
	while((!(IICCON&SI))&&(IICSTA!=0x08));
	IICCON &=~ STA;
	IICCON &=~ SI;
	IICDAT = DEVICEADD;
	while(IICSTA != 0x18);
	IICCON &=~ SI;
	IICDAT = addr;
	while(IICSTA!=0x28);
	IICCON &=~ SI;
	IICDAT = val;
	while(IICSTA!=0x28);
	IICCON &=~ SI;
	IICCON |= STO; 
}

void IIC_Send_nByte(unsigned char data addr,
					unsigned char data *write_buf,
					unsigned char data nbyte)
{

}

unsigned char IIC_Read_Byte(unsigned char data addr)
{
	unsigned char data val;

	IICCON &=~ SI;
	IICCON |= STA; 			//����
	while((!(IICCON&SI))&&(IICSTA!=0x08));
	IICCON &=~ STA;
	IICCON &=~ SI;
	IICDAT = DEVICEADD;		//д��дָ��
	while(IICSTA!=0x18);
	IICCON &=~ SI;
	IICDAT = addr;			//д�����ַ
	while(IICSTA!=0x28);
	IICCON &=~ SI;
	IICCON |= STA; 			//����
	while((!(IICCON&SI))&&(IICSTA!=0x08));
	IICCON &=~ STA;
	IICCON &=~ SI;
	IICDAT = DEVICEADD|0x01;//д���ָ��
	while(IICSTA!=0x40);
	IICCON &=~ SI;
	while(IICSTA!=0x58); 
	val = IICDAT;			//��������
	IICCON &=~ SI;
	IICCON |= STO;
	return val;
}


