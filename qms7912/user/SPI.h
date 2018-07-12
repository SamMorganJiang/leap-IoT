#ifndef __SPI_H__
#define __SPI_H__


void SPI_Init(void);								
void SPI_Write_Byte(int data1);
int SPI_Read_Byte(void);
void SPI_Write_Reg(int reg, int datae);
int SPI_Read_Reg(int reg);
void SPI_Write_Buffer(int reg, int *dataBuf, int len);


#endif/* __SPI_H__ */
