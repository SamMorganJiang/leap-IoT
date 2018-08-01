#ifndef __UART_H__
#define __UART_H__

void IO_Init(void);						//IO初始化
void UART_Init(void);					//UART初始化
void Uart_Test_Receive(void);  //UART测试
void SendString(unsigned char *p);
void SendChar(unsigned char Char);

#endif/* __UART_H__ */