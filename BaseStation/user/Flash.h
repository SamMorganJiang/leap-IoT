#ifndef _FLASH_H_
#define _FLASH_H_

void Flash_EraseBlock(unsigned int Address);		//��������
void FLASH_ProgramWord(unsigned int Address,unsigned char datt);	//д����һ�����ݵ�FLASH����
void Flash_ReadArr(unsigned int Address,unsigned char Length,unsigned char *SaveArr);	//��FLASH�����ȡ���ⳤ�ȵ�����
void Flash_RestWithReadOption(void);				//��λ�ض�OPTION
void Flash_RestWithoutReadOption(void);	   			//��λ���ض�OPTION
unsigned int Flash_HeadArr(unsigned int Address);	//���㵱ǰ��ַ�������׵�ַ

#endif