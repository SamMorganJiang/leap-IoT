#include "HC89F0650.h"
#include "Uart.h"
		
/*

void Uart_Test_Receive(void)
{
		//if(bufflag)
		//{
			SBUF = 0x03;//bufdat;				//��������
			while(!(SCON&0x02));
			SCON &=~ 0x02;				//�����ж������жϱ�־λ��0
			bufflag = 0;				//��־λ��0
			SCON |= 0x10; 				//�����ڽ���  
			ES1 = 1;	 				//�������ж�
		//}
}

void Usart_Ipt() interrupt UART1_VECTOR
{
	if(SCON&0x01)
	{
		SCON &=~ 0x01;   				//�յ����ݺ�������ձ�־
		bufdat = SBUF;					//��������
		bufflag = 1;
		SCON &=~ 0x10; 					//�ش��ڽ���  
		ES1 = 0;	 					//�ش����ж�
	}
}

*/

