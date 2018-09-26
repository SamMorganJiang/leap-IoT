#ifndef _EXT_FLASH_H_
#define _EXT_FLASH_H_

sbit	CS    = P0^0;										//P00����ΪCS
sbit	SCK   = P0^1;										//P03����ΪSCK
sbit	MOSI  = P0^2;										//P02����ΪSI
sbit	MISO  = P0^3;										//P01����ΪSO

/************************************ָ���********************************************/
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

void SPI_Flash_Init(void);								//SPI��ʼ��
void SPI_Flash_WAKEUP(void);							//����
void SPI_Flash_PowerDown(void);							//����
void SPI_Flash_Wait_Busy(void);							//�ȴ�����
void SPI_Flash_Erase_Chip(void);						//��������оƬ
void SPI_FLASH_Write_Disable(void);						//SPI_FLASHд��ֹ
void SPI_FLASH_Write_Enable(void);						//SPI_FLASHдʹ��
void SPI_FLASH_Write_SR(unsigned char sr);				//дSPI_FLASH״̬�Ĵ���
unsigned char SPI_Flash_ReadSR(void);					//��SPI_FLASH״̬�Ĵ���
void SPI_Flash_ReadID(unsigned char *id);				//��ȡоƬID��
void SPI_Flash_Read(unsigned char* pBuffer,unsigned long ReadAddr,unsigned int NumByteToRead);			//��ȡ�����ֽ�����
void SPI_Flash_Write_Page(unsigned char* pBuffer,unsigned long WriteAddr,unsigned int NumByteToWrite);	//дС��256�ֽڵ�����

#endif