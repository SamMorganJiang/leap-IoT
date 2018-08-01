#include "HC89F0650.h"
#include "Uart.h"
		
/*

void Uart_Test_Receive(void)
{
		//if(bufflag)
		//{
			SBUF = 0x03;//bufdat;				//发送数据
			while(!(SCON&0x02));
			SCON &=~ 0x02;				//发送中断请求中断标志位清0
			bufflag = 0;				//标志位置0
			SCON |= 0x10; 				//开串口接收  
			ES1 = 1;	 				//开串口中断
		//}
}

void Usart_Ipt() interrupt UART1_VECTOR
{
	if(SCON&0x01)
	{
		SCON &=~ 0x01;   				//收到数据后，清除接收标志
		bufdat = SBUF;					//接收数据
		bufflag = 1;
		SCON &=~ 0x10; 					//关串口接收  
		ES1 = 0;	 					//关串口中断
	}
}

*/

