#ifndef _FLASH_H_
#define _FLASH_H_

void Flash_EraseBlock(unsigned int Address);		//扇区擦除
void FLASH_ProgramWord(unsigned int Address,unsigned char datt);	//写入任一个数据到FLASH里面
void Flash_ReadArr(unsigned int Address,unsigned char Length,unsigned char *SaveArr);	//从FLASH里面读取任意长度的数据
void Flash_RestWithReadOption(void);				//复位重读OPTION
void Flash_RestWithoutReadOption(void);	   			//复位不重读OPTION
unsigned int Flash_HeadArr(unsigned int Address);	//计算当前地址的扇区首地址

#endif