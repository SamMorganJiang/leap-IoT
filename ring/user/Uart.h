#ifndef __UART_H__
#define __UART_H__

void IO_Init(void);						//IO��ʼ��
void UART_Init(void);					//UART��ʼ��
void Uart_Test_Receive(void);  //UART����
void SendString(unsigned char *p);
void SendChar(unsigned char Char);

#endif/* __UART_H__ */