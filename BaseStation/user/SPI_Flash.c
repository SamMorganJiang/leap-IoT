/**
*	************************************************************************************
*									 模块性能介绍
*	1、全双工，三/四线同步传输
*	2、主从机操作
*	3、4级可编程主时钟频率
*	4、极性相位可编程的串行时钟
*	5、可选择数据传输方向
*	6、写冲突及接收溢出标志
*	7、带MCU中断的主模式模式冲突检测
*	8、带MCU中断的传输结束标志
*	************************************************************************************
*									 应用注意事项
*	1、SSIG=0&CPHA=0时，数据在SS为低被驱动；CPHA=1时，数据在SCK的前时钟沿驱动
*	2、接收为双BUFF，接收溢出发生在第二个数据传送完成前仍未清除之前接收数据产生的SPIF标
*	   志，故每次准备接收下一个数据前必须先清除SPIF，否则RXOV将置1，RXOV置1不会影响SPI接
*	   收
*	************************************************************************************
**/

#include "HC89F0650.h"
#include "intrins.h"
#include "SPI_Flash.h"

/***************************************************************************************
  * @说明  	SPI初始化
  *	@参数	无
  * @返回值 无
  * @注		无
***************************************************************************************/
void SPI_Flash_Init(void)
{
	P0M0 = 0xcc;	//25Q64IO输出为5V时需要降低推挽的电流
	P0M1 = 0x6c;

	SS_MAP   = 0xc0; 	//使能端
	SCK_MAP  = 0xc1;	//时钟端
	MOSI_MAP = 0xc2;	//输出口
	MISO_MAP = 0xc3;	//输入口

	SPDAT  = 0x00;
	SPSTAT = 0x00;
	SPCTL  = 0xd3;
}

/***************************************************************************************
  * @说明  	写一字节并读出此地址的状态
  *	@参数	dat：写入的数据
  * @返回值 读到的数据
  * @注		无
***************************************************************************************/
unsigned char SPI_ReadWriteByte(unsigned char dat)
{
	SPSTAT = 0xc0;
	SPDAT  = dat;
	while(!(SPSTAT&0x80));
	SPSTAT = 0xc0;

	return SPDAT;
}

/***************************************************************************************
  * @说明  	获取芯片ID
  *	@参数	无
  * @返回值 无
  * @注		无
***************************************************************************************/
void SPI_Flash_ReadID(unsigned char *id)
{
	CS = 0;
	SPI_ReadWriteByte(0x90);//发送读取ID命令	    
	SPI_ReadWriteByte(0x00);
	SPI_ReadWriteByte(0x00);
	SPI_ReadWriteByte(0x00);
	id[0] = SPI_ReadWriteByte(0xFF);
	id[1] = SPI_ReadWriteByte(0xFF);
	CS = 1;
}

/***************************************************************************************
  * @说明  	读SPI_FLASH状态寄存器
  *	@参数	无
  * @返回值 状态寄存器的值
  * @注		无
***************************************************************************************/
unsigned char SPI_Flash_ReadSR(void)   
{  
	unsigned char byte = 0;

	CS = 0;                            			//使能器件
	SPI_ReadWriteByte(W25X_ReadStatusReg);      //发送读取状态寄存器命令
	byte=SPI_ReadWriteByte(0Xff);               //读取一个字节
	CS = 1;                           	 		//取消片选

	return byte;
}

/***************************************************************************************
  * @说明  	写SPI_FLASH状态寄存器
  *	@参数	sr：写的状态
  * @返回值 状态寄存器的值
  * @注		只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写
***************************************************************************************/
void SPI_FLASH_Write_SR(unsigned char sr)
{
	CS = 0;                            		//使能器件
	SPI_ReadWriteByte(W25X_WriteStatusReg);	//发送写取状态寄存器命令
	SPI_ReadWriteByte(sr);          		//写入一个字节
	CS = 1;                       			//取消片选
}

/***************************************************************************************
  * @说明  	SPI_FLASH写使能
  *	@参数	无
  * @返回值 无
  * @注		无
***************************************************************************************/
void SPI_FLASH_Write_Enable(void)
{
	CS = 0;                         			//使能器件
    SPI_ReadWriteByte(W25X_WriteEnable);	    //发送写使能
	CS = 1;                         			//取消片选
}

/***************************************************************************************
  * @说明  	SPI_FLASH写禁止
  *	@参数	无
  * @返回值 无
  * @注		无
***************************************************************************************/
void SPI_FLASH_Write_Disable(void)
{
	CS = 0;                         			//使能器件
    SPI_ReadWriteByte(W25X_WriteDisable);	    //发送写禁止指令
	CS = 1;                         			//取消片选
}

/***************************************************************************************
  * @说明  	在指定地址开始读取指定长度的数据
  *	@参数	pBuffer：数据存储区
  *			ReadAddr:开始读取的地址(24bit)
  *			NumByteToRead:要读取的字节数(最大65535)
  * @返回值 无
  * @注		无
***************************************************************************************/
void SPI_Flash_Read(unsigned char* pBuffer,
					unsigned long ReadAddr,
					unsigned int NumByteToRead)
{
	unsigned int i;

	CS=0;                      				//使能器件
	SPI_ReadWriteByte(W25X_ReadData); 		//发送读取命令
	SPI_ReadWriteByte((unsigned char)((ReadAddr)>>16));  //发送24bit地址
	SPI_ReadWriteByte((unsigned char)((ReadAddr)>>8));
	SPI_ReadWriteByte((unsigned char)ReadAddr);

	for(i = 0; i < NumByteToRead; i++)
		pBuffer[i] = SPI_ReadWriteByte(0XFF);	//循环读数

	CS=1;									    //取消片选
}

/***************************************************************************************
  * @说明  	SPI在一页(0~65535)内写入少于256个字节的数
  *	@参数	pBuffer：数据存储区
  *			WriteAddr：开始写入的地址(24bit)
  *			NumByteToRead:要读取的字节数(最大65535)，该数不应该超过该页的剩余字节数
  * @返回值 无
  * @注		无
***************************************************************************************/
void SPI_Flash_Write_Page(unsigned char* pBuffer,
						unsigned long WriteAddr,
						unsigned int NumByteToWrite)
{
		unsigned int i;

		SPI_FLASH_Write_Enable();			//SET WEL
		CS=0;								//使能器件
		SPI_ReadWriteByte(W25X_PageProgram);						//发送写页命令
		SPI_ReadWriteByte((unsigned char)((WriteAddr)>>16));		//发送24bit地址
		SPI_ReadWriteByte((unsigned char)((WriteAddr)>>8));
		SPI_ReadWriteByte((unsigned char)WriteAddr);

		for(i = 0; i < NumByteToWrite; i++)
			SPI_ReadWriteByte(pBuffer[i]);	//循环写数

		CS=1;								//取消片选
		SPI_Flash_Wait_Busy();				//等待写入结束
}

/***************************************************************************************
  * @说明  	擦除整个芯片
  *	@参数	无
  * @返回值 无
  * @注		等待时间超长
***************************************************************************************/
void SPI_Flash_Erase_Chip(void)
{
	SPI_FLASH_Write_Enable();	//SET WEL
	SPI_Flash_Wait_Busy();
	CS=0;                   	//使能器件
	SPI_ReadWriteByte(W25X_ChipErase);//发送片擦除命令
	CS=1;                  		//取消片选
	SPI_Flash_Wait_Busy();		//等待芯片擦除结束
}

/***************************************************************************************
  * @说明  	等待空闲
  *	@参数	无
  * @返回值 无
  * @注		无
***************************************************************************************/
void SPI_Flash_Wait_Busy(void)
{
	while((SPI_Flash_ReadSR()&0x01) == 0x01);   // 等待BUSY位清空
}

/***************************************************************************************
  * @说明  	进入掉电模式
  *	@参数	无
  * @返回值 无
  * @注		无
***************************************************************************************/
void SPI_Flash_PowerDown(void)
{
	CS=0;                             //使能器件
	SPI_ReadWriteByte(W25X_PowerDown);//发送掉电命令
	CS=1;                             //取消片选

 	_nop_();
	_nop_();
	_nop_();
}

/***************************************************************************************
  * @说明  	唤醒
  *	@参数	无
  * @返回值 无
  * @注		无
***************************************************************************************/
void SPI_Flash_WAKEUP(void)
{
	CS=0;                                     //使能器件
	SPI_ReadWriteByte(W25X_ReleasePowerDown); //send W25X_PowerDown command 0xAB
	CS=1;                 		              //取消片选

	_nop_();
	_nop_();
	_nop_();
}

/***************************************************************************************
  * @实现效果	在25Q64写入数据后再次读取出来
***************************************************************************************/
void SPI_Flash_Test(void)
{
	unsigned char hello[]="hello holychip\r";
	unsigned char buf[16];									//用于存储读取数据
	unsigned char id[2];									//用于存储ID号

	SPI_Flash_Init();                       //SPI控制器初始化
	SPI_FLASH_Write_Enable();			    //SPI写使能
	SPI_Flash_ReadID(id); 				    //读取ID号

	SPI_Flash_Write_Page(hello, 1000, 16);	//SPI写入数据
	SPI_Flash_Read(buf, 1000, 16);			//SPI读出数据

	_nop_();
}