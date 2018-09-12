#define	ALLOCATE_EXTERN
#include "HC89F0650.h"
#include "intrins.h"
#include "Uart.h"
#include "SPI.h"
#include "I2C.h"
#include "QMA6981.h"
#include "MG127.h"
#include "I2C_SLAVE.h"
#include "Test_Prom.h"

sbit LED1 = P1^7;
sbit LED2 = P1^6;
sbit KEY  = P3^4;

unsigned char data bufdat = 0;				//���ڴ�źͽ�������
unsigned char data bufflag = 0;
unsigned char data str[4];

unsigned int rtc_count = 0;
void RCT_Init(void);
void UART_Init(void);
void System_init(void);
void IO_Init();
void delay_by_ms(unsigned int timdlay);

void Delay_us(u32 delayCnt)
{
	u32 temp0 =0;

	for(temp0 =0; temp0 < delayCnt;temp0++) {
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
	}
}

void Delay_ms(u32 delayCnt)
{
	u32 temp0 =0;

	for(temp0 =0; temp0 < delayCnt;temp0++) {
        Delay_us(1132);
	}
}

void main(void)
{
	unsigned short step_num = 0;

	System_init();	   					//ϵͳ��ʼ��
	IO_Init();							//IO��ʼ��
	RCT_Init();							//RTC��ʼ��
	SPI_Init();                         //SPI��ʼ��
	IIC_Init();
	//UART_Init();
	//ES1 = 1;	 						//�������ж�

	while(1) {
		if (KEY == 1) {
			Delay_ms(30);
			if (KEY == 1) {
				LED1 = ~LED1;
				LED2 = ~LED2;
			}
		}
	}

	ERTC=0;								//disable RTC�ж�
	EA = 1;								//ʹ�����ж�

	if(qmaX981_get_chip_id()) {
		qma6981_initialize();
	}
	BLE_Init();

	while(1) {
		//////ble rtx api
		//txcnt=100; //txcnt=0 is for rx only application
		//rxcnt=20; //rxcnt=0 is for tx only application
		//qma6981_read_raw_xyz();
		if(rtc_count == 0) {
			ERTC=0; 							//disable RTC�ж�
			step_num = qmaX981_step_c_read_stepCounter();
			//BLE_Set_rxtxcnt(100, 20);
			BLE_Set_rxtxcnt(500, 0);
			BLE_TRX();

			//delay to set ble tx interval
			Delay_ms(100);
			ERTC=1; 							//enable RTC�ж�
			// add by yangzhiqiang enter power done mode
			PCON |= 0x03;
			// yangzhiqiang
		}
	}
}

/***************************************************************************************
  * @˵��  	��ʱ����
  * @����  	timdlay ����ʱ�趨ֵ
  *			ȡֵ��Χ: 1 - 65536.
  * @����ֵ ��
  * @ע		������ʱ
***************************************************************************************/
void delay_by_ms(unsigned int timdlay)
{
	unsigned char j;
	for(;timdlay>0;timdlay--)
	for(j=100;j>0;j--);
}

/***************************************************************************************
  * @˵��  	ϵͳ��ʼ��
  *	@����	��
  * @����ֵ ��
  * @ע		�رտ��Ź��Լ�ʱ�ӳ�ʼ��
***************************************************************************************/
void System_init(void)
{
	WDTCCR = 0x00;						//�رտ��Ź�

	while((CLKCON&0x20)!=0x20);			//�ȴ��ڲ���Ƶ��������
	CLKDIV = 0x02;						//CPUʱ��2��Ƶ��ȷ���ڽ���RC32��ƵʱCPUʱ��С��16M
	CLKSWR &=~ 0x10;					//�л�Ϊ�ڲ���Ƶʱ��
	while((CLKSWR&0x40)!=0x00);			//�ȴ��ڲ���Ƶ�л����
	CLKDIV = 0x04;						//OSCʱ��4��Ƶ
}

void Usart_Ipt() interrupt UART1_VECTOR
{
	if (SCON&0x01) {
		SCON &=~ 0x01;   				//?????,??????
		bufdat = SBUF & 0x0f;			//????(???4?)
		bufflag = 1;
		SCON &=~ 0x10; 					//?????
		ES1 = 0;	 					//?????
	}
}

/***************************************************************************************
 * @??  	?????
 * @??	?
 * @???  ?
 * @?		????????????
 ***************************************************************************************/

void SendChar(unsigned char Char)
{
	SBUF = Char;
	while(!(SCON&0x02));
	SCON &=~ 0x02;				//????????????0
}

void SendString(unsigned char *p)
{
	while(*p) {
		SendChar(*p);
		p++;
	}
}

unsigned char* CharToHexStr(unsigned char n)
{
	unsigned char *pstr;

	pstr = str;
	*pstr = n / 16 + '0';
	if (*pstr > '9')
		*pstr += 39;

	*++pstr = n % 16 + '0';
	if (*pstr > '9')
		*pstr += 39;

	*(++pstr) = '\0';
	return str;
}

unsigned char* CharToString(unsigned char n)
{
	unsigned char *pstr;

	pstr = str;
	if (n > 99) {
		*pstr++ = n/100 + '0';
		n %= 100;
		*pstr++ = n/10 + '0';
		n %= 10;
		*pstr = n + '0';
	} else if (n > 9) {
		*pstr++ = n/10 + '0';
		n %= 10;
		*pstr = n + '0';
	} else {
		*pstr = n + '0';
	}

	*(++pstr) = '\0';
	return str;
}

/***************************************************************************************
 * @??  	IO???
 * @??	?
 * @???  ?
 * @?		P00????,P01????
 ***************************************************************************************/
void IO_Init(void)
{
	P5M2 = 0x00;

	P1M3 = 0xCC;                        //LED1��2�������( IO �������� Low Drive Mode)
	LED1 = LED2 = 0;

	P3M2 &= 0xF0;
	P3M2 |= 0x05;                       //KEY ����������(SMT)
}

/***************************************************************************************
 * @??  	UART???
 * @??	?
 * @???  ?
 * @?		8?????,???1200
 ***************************************************************************************/
void UART_Init(void)
{
	P1M3 = 0x82;
	TXD_MAP = 0x0F;						//TXD??P17
	RXD_MAP = 0x0E;						//RXD??P16

	SBRTH = 0xFE5F/256;
	SBRTL = 0xFE5F%256;
	SCON2 = 0x12;						//?????
	SCON |= 0X10; 						//?????
}

/***************************************************************************************
  * @?��?��  	RTC?D??��t??o����y
  *	@2?��y	?T
  * @����???�� ?T
  * @����		?T
***************************************************************************************/
void Rtc_Irt()interrupt RTC_VECTOR
{
//	P0_1 =! P0_1; 					   //P01��-��a
	rtc_count++;
	RTCC &= ~0x80;					   //??3y?D??����????
	if(rtc_count >= 5)
	{
		rtc_count = 0;
		ERTC=0; 							//disable RTC�ж�
		PCON &= (~0x03);
	}
}

/***************************************************************************************
  * @?��?��  	RTC3?��??��
  *	@2?��y	?T
  * @����???�� ?T
  * @����		��1?��RTC?D??��??��RTC��?3?��??D??����??1s
***************************************************************************************/
void RCT_Init(void)
{
	CLKCON |= 0x04;						//ʹ���ⲿ����
	while((CLKCON&0x40)!=0x40);			//�ȴ��ⲿ��Ƶ��������
	//RTCC = 0x13;				   		//RTC�ж�ʱ��1s,����RTCʱ�����
	RTCC = 0x1f;				   		//RTC�ж�ʱ��8s,����RTCʱ�����
	RTCO_MAP = 0x03;					//RCTʱ�����ӳ��P0.3��
}
