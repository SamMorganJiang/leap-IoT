#include "HC89F0650.h"
#include "Uart.h"
#include "SPI.h"
#include "I2C.h"
#include "ADC.h"
#include "Test_Prom.h"
#include "Timer.h"

sbit ADC1 = P2^1;   //Test Add 1.2V
sbit ADC_Contrl = P1^7;  //Test ADC Test Contrl
sbit BLE_test = P3^3;

unsigned int  ADC_[10] = {0};
unsigned int  ADC_sum = 0;
unsigned int  ADC_Report = 0;


void ADC_TestInit(void)
{
    P2M0 = 0x33;						//P00������Ϊģ������

    ADCC0 |= 0x80;						//��ADCת����Դ
    ADCC1 |= 0x09;						//ѡ���ⲿͨ��16
    ADCC2 = 0x13;						//ת�����12λ���ݣ��߰�λ����ADCRH�Ĵ�����ADCʱ��8��Ƶ
}

void GPIO_TestPinInit(void)
{
    P1M3 = 0x33;
}

void ADC_MCUTest(void)
{
    int i;

    for(i = 0; i < 10; i++) {
        if(ADC_Contrl = 1) {
            ADC_[i] = ADC_GetResault();
            ADC_sum += ADC_[i];
        }
    }

    ADC_Report = (int) ADC_sum/10;
}
