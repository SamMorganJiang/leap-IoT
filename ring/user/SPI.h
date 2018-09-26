#ifndef _SPI_H_
#define _SPI_H_

void SPI_Init(void);
void SPI_Write_Reg(u8 reg, u8 datae);
u8 SPI_Read_Reg(u8 reg);
void SPI_Write_Buffer(u8 reg, u8 *dataBuf, u8 len);
void SPI_Read_Buffer(u8 reg, u8 *dataBuf, u8 len);
u8 mcu_get_irq(void);

#endif

