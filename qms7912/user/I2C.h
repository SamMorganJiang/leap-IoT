#ifndef __I2C_H__
#define __I2C_H__


void IIC_Init(void);					//Ӳ��IIC��ʼ��
void IIC_Send_Byte(unsigned char add,unsigned char dat);	//IIC����һ���ֽڵ�����
unsigned char IIC_Read_Byte(unsigned char add);				//IIC��ȡһ���ֽڵ�����


#endif/* __I2C_H__ */