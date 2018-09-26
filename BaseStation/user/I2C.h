#ifndef __I2C_H__
#define __I2C_H__


void IIC_Init(void);					//硬件IIC初始化
void IIC_Send_Byte(unsigned char add,unsigned char dat);	//IIC发送一个字节的数据
unsigned char IIC_Read_Byte(unsigned char add);				//IIC读取一个字节的数据


#endif/* __I2C_H__ */