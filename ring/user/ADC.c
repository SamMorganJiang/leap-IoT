#include "HC89F0650.h"
#include "Uart.h"
#include "SPI.h"
#include "I2C.h"
#include "ADC.h"
#include "Test_Prom.h"
#include "Timer.h"
#include <HC_DEFINE.H>

sbit ADC1 = P2^5;   //Test Add 1.2V
sbit ADC_Contr2 = P3^3;  //Test ADC Test Contrl
sbit ADC_Contr1 = P3^4;  
sbit ADC_Contr3 = P5^5;
sbit ADC_test_bit1 = P1^5;
sbit ADC_test_bit2 = P1^6;
sbit ADC_test_bit3 = P1^7;
sbit ADC_test_bit4 = P5^3;



unsigned int  ADC_sum = 0;
unsigned int  ADC_Report = 0;

void ADC_TestInit(void)
{
	P2M2 = 0x33;			//P22口设置为模拟输入
  //	P5M1 = 0x33;			//P53口设置为模拟输入
	P1M2 = 0xD3;
	P1M3 = 0xDD;
	P5M1 = 0xD3;
	//P5M2 = 0x33;
	P3M1 = 0x03;
	P3M2 = 0x30;
	
	ADCC0 |= 0x80;						//打开ADC转换电源
	ADCC1 |= 0x0D;						//选择外部通道10
	ADCC2 = 0x13;						//转换结果12位数据，高八位置于ADCRH寄存器，ADC时钟8分频
	
}

u8 data data8_1;
u8 data data8_2;
u8 data data8_3;
u8 data data8_4;

void Data_Covent(unsigned int data ADC_data)
{
//unsigned int data ADC_data ;
unsigned int data ADC_data2;
	
ADC_data2 = ADC_data & 0x000f;
data8_1 = ADC_data2;
ADC_data2 = (ADC_data >> 4);
ADC_data2 = ADC_data2 & 0x000f;
data8_2 = (u8)ADC_data2;
ADC_data2 = (ADC_data >> 8);
ADC_data2 = ADC_data2 & 0x000f;
data8_3 = (u8)ADC_data2;
ADC_data2 = (ADC_data >> 12);
ADC_data2 = ADC_data2 & 0x000f;
data8_4 = (u8)ADC_data2;	
}

void Data_Out(u8 data data_out)
{
	switch(data_out)
	{
	case 0x00:
	ADC_test_bit1 = 0;
	ADC_test_bit2 = 0;
	ADC_test_bit3 = 0;
	ADC_test_bit4 = 0;
  break;	
	case 0x01:
	ADC_test_bit1 = 1;
	ADC_test_bit2 = 0;
	ADC_test_bit3 = 0;
	ADC_test_bit4 = 0;
	break;
	case 0x02:
	ADC_test_bit1 = 0;
	ADC_test_bit2 = 1;
	ADC_test_bit3 = 0;
	ADC_test_bit4 = 0;
	break;	
	case 0x03:
	ADC_test_bit1 = 1;
	ADC_test_bit2 = 1;
	ADC_test_bit3 = 0;
	ADC_test_bit4 = 0;
	break;		
	case 0x04:
	ADC_test_bit1 = 0;
	ADC_test_bit2 = 0;
	ADC_test_bit3 = 1;
	ADC_test_bit4 = 0;
	break;
	case 0x05:
	ADC_test_bit1 = 1;
	ADC_test_bit2 = 0;
	ADC_test_bit3 = 1;
	ADC_test_bit4 = 0;
	break;
	case 0x06:
	ADC_test_bit1 = 0;
	ADC_test_bit2 = 1;
	ADC_test_bit3 = 1;
	ADC_test_bit4 = 0;
	break;
	case 0x07:
	ADC_test_bit1 = 1;
	ADC_test_bit2 = 1;
	ADC_test_bit3 = 1;
	ADC_test_bit4 = 0;
	break;
	case 0x08:
	ADC_test_bit1 = 0;
	ADC_test_bit2 = 0;
	ADC_test_bit3 = 0;
	ADC_test_bit4 = 1;
	break;
	case 0x09:
	ADC_test_bit1 = 1;
	ADC_test_bit2 = 0;
	ADC_test_bit3 = 0;
	ADC_test_bit4 = 1;
	break;
	case 0x0A:
	ADC_test_bit1 = 0;
	ADC_test_bit2 = 1;
	ADC_test_bit3 = 0;
	ADC_test_bit4 = 1;
	break;
	case 0x0B:
	ADC_test_bit1 = 1;
	ADC_test_bit2 = 1;
	ADC_test_bit3 = 0;
	ADC_test_bit4 = 1;
	break;
	case 0x0C:
	ADC_test_bit1 = 0;
	ADC_test_bit2 = 0;
	ADC_test_bit3 = 1;
	ADC_test_bit4 = 1;
	break;
	case 0x0D:
	ADC_test_bit1 = 1;
	ADC_test_bit2 = 0;
	ADC_test_bit3 = 1;
	ADC_test_bit4 = 1;
	break;
	case 0x0E:
	ADC_test_bit1 = 0;
	ADC_test_bit2 = 1;
	ADC_test_bit3 = 1;
	ADC_test_bit4 = 1;
	break;
	case 0x0F:
	ADC_test_bit1 = 1;
	ADC_test_bit2 = 1;
	ADC_test_bit3 = 1;
	ADC_test_bit4 = 1;
	break;	
	}	
}

void ADC_DATA_OUT(void)
{
	if((ADC_Contr1 == 0)&&(ADC_Contr2 == 0))
	{
		Data_Out(data8_1);
		delay(20);
	}
	if((ADC_Contr1 == 0)&&(ADC_Contr2 == 1))
	{
		Data_Out(data8_2);
		delay(20);
	}
	if((ADC_Contr1 == 1)&&(ADC_Contr2 == 0))
	{
		Data_Out(data8_3);
		delay(20);
	}
	if((ADC_Contr1 == 1)&&(ADC_Contr2 == 1))
	{
		Data_Out(data8_4);
		delay(20);
	}
}

unsigned int ADC_GetResault(void)
{
	ADCC0 |= 0x40;						//启动ADC转换
	while(!(ADCC0&0x20));				//等待ADC转换结束
	ADCC0 &=~ 0x20;						//清除标志位
	return ADCR;						//返回ADC的值
}

/*

void GPIO_TestInPinInit(void)
{

	P5M2 = 0x33;			//P55,P55口设置为模拟输入
}

void GPIO_TestOutPinInit(void)
{
	
	P5M2 = 0x99;			//P55,P55口设置为模拟输入
}*/

void ADC_MCUTest(void)
{
	int i;
//  float valute[10];
	unsigned int  ADC_[10] = {0};
	ADC_sum = 0;

	for(i = 0;i<10;i++)
  {
  ADC_[i] = ADC_GetResault();
  ADC_sum += ADC_[i];
  delay(20);
  }
	ADC_Report = ADC_GetResault();//int) ADC_sum/10;
	
	Data_Covent(ADC_Report);
  ADC_DATA_OUT();
}
