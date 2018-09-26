#ifndef _EXT_FLASH_H_
#define _EXT_FLASH_H_

sbit	CS    = P0^0;										//P00定义为CS
sbit	SCK   = P0^1;										//P03定义为SCK
sbit	MOSI  = P0^2;										//P02定义为SI
sbit	MISO  = P0^3;										//P01定义为SO

/************************************指令表********************************************/
#define W25X_WriteEnable		0x06
#define W25X_WriteDisable		0x04
#define W25X_ReadStatusReg		0x05
#define W25X_WriteStatusReg		0x01
#define W25X_ReadData			0x03
#define W25X_FastReadData		0x0B
#define W25X_FastReadDual		0x3B
#define W25X_PageProgram		0x02
#define W25X_BlockErase			0xD8
#define W25X_SectorErase		0x20
#define W25X_ChipErase			0xC7
#define W25X_PowerDown			0xB9
#define W25X_ReleasePowerDown	0xAB
#define W25X_DeviceID			0xAB
#define W25X_ManufactDeviceID	0x90
#define W25X_JedecDeviceID		0x9F

void SPI_Flash_Init(void);								//SPI初始化
void SPI_Flash_WAKEUP(void);							//唤醒
void SPI_Flash_PowerDown(void);							//休眠
void SPI_Flash_Wait_Busy(void);							//等待空闲
void SPI_Flash_Erase_Chip(void);						//擦除整个芯片
void SPI_FLASH_Write_Disable(void);						//SPI_FLASH写禁止
void SPI_FLASH_Write_Enable(void);						//SPI_FLASH写使能
void SPI_FLASH_Write_SR(unsigned char sr);				//写SPI_FLASH状态寄存器
unsigned char SPI_Flash_ReadSR(void);					//读SPI_FLASH状态寄存器
void SPI_Flash_ReadID(unsigned char *id);				//读取芯片ID号
void SPI_Flash_Read(unsigned char* pBuffer,unsigned long ReadAddr,unsigned int NumByteToRead);			//读取任意字节数据
void SPI_Flash_Write_Page(unsigned char* pBuffer,unsigned long WriteAddr,unsigned int NumByteToWrite);	//写小于256字节的数据

#endif