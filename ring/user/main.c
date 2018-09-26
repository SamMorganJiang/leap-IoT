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
unsigned char data key_short_flag = 0;
unsigned char data ket_long_flag = 0;

unsigned int rtc_count = 0;
unsigned int t0_count = 0;
unsigned int flag_trx = 0;

static u8 tx_cnt;
static u8 rx_cnt;

void RCT_Init(void);
void UART_Init(void);
void System_init(void);
void IO_Init(void);
void EXTI_Init(void);
void Timer_Init(void);
void Timer_enable(unsigned int timer, unsigned int enable);
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
	IIC_Init();                         //IIC��ʼ��
	EXTI_Init();                        //�жϳ�ʼ��
	Timer_Init();                       //��ʱ����ʼ��
	//UART_Init();
	//ES1 = 1;	 						//�������ж�
	EA = 1;								//ʹ�����ж�

	if(qmaX981_get_chip_id()){
		qma6981_initialize();
	}

	BLE_Init();
	LED1 = 1;
	LED2 = 1;
	tx_cnt = 500; //txcnt=0 is for rx only application
	rx_cnt = 0; //rxcnt=0 is for tx only application
	while(1){
		/*
		if (KEY == 1) {
			Delay_ms(30);
			if (KEY == 1 && flag_trx == 0) {     //tx mode
				LED2 = 1;
				flag_trx == 1;
				tx_cnt = 500; //txcnt=0 is for rx only application
				rx_cnt = 0; //rxcnt=0 is for tx only application
				goto TX_MODE;
			}
			if (KEY == 1 && flag_trx == 1) {     //rx mode
				LED2 = 0;
				flag_trx == 0;
				tx_cnt = 0; //txcnt=0 is for rx only application
				rx_cnt = 500; //rxcnt=0 is for tx only application
				goto RX_MODE;
			}
		}
*/
TX_MODE:
		if(rtc_count == 0) {
			ERTC=0; 							//disable RTC�ж�
			step_num = qmaX981_step_c_read_stepCounter();
			BLE_Set_rxtxcnt(tx_cnt, rx_cnt);
			BLE_TRX(step_num);

			//delay to set ble tx interval
			Delay_ms(100);
			ERTC=1; 							//enable RTC�ж�
			// add by yangzhiqiang enter power done mode
			PCON |= 0x03;
			// yangzhiqiang
			LED1 = 0;
		}
		continue;

//RX_MODE:
		//LED2 = 1;
		//BLE_Set_rxtxcnt(tx_cnt, rx_cnt);
		//BLE_TRX();
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

/***************************************************************************************
  * @˵��  	INT2-7�жϷ�����
  *	@����	��
  * @����ֵ ��
  * @ע		��
***************************************************************************************/
void INT2_7_Rpt() interrupt INT2_7_VECTOR
{
	if(PINTF0&0x10){
		PINTF0 &=~ 0x10;			//���INT4�жϱ�־λ
		Timer_enable(0, 1);
		IE |= 0x02;
	}
}

void EXTI_Init(void)
{
	//PITS1  &= 0xFC;      //EINT4�͵�ƽ����
	PITS1  |= 0x01;      //EINT4�½��ش���
	PINTE0 |= 0x10;      //EINT4ʹ��
	IE1    |= 0x20;		 //���ж�
}

/***************************************************************************************
  * @˵��  	T0�жϷ�����
  *	@����	��
  * @����ֵ ��
  * @ע		��
***************************************************************************************/
void TIMER0_Rpt(void) interrupt TIMER0_VECTOR
{
	TCON &=~ 0x20;						//���T0�жϱ�־λ
	t0_count++;

	if(t0_count == 2 && KEY == 1)       //�̰������ж� 100ms
		key_short_flag = 1;
	if(t0_count == 6 && KEY != 1 && key_short_flag){    //�̰�ȷ�� 300ms ����
		key_short_flag = 0;
		t0_count == 0;
		LED1 = ~LED1;
		IE &= 0xFD;
		//Timer_enable(0, 0);
	}
	if(t0_count == 20 && KEY == 1){     //�����ж�
		key_short_flag = 0;
		t0_count == 0;
		LED2 = ~LED2;
		IE &= 0xFD;
		//Timer_enable(0, 0);
	}
}

/***************************************************************************************
  * @˵��  	Timer��ʼ��
  *	@����	��
  * @����ֵ ��
  * @ע		��
***************************************************************************************/
void Timer_Init(void)
{
	TCON1 = 0x00;						//Tx0��ʱ��ʱ��ΪFosc
	TMOD = 0x00;						//16λ��װ�ض�ʱ��/������

	//Tim0����ʱ�� 	= (65536 - 0xFD64) * (1 / (Fosc /Timer��Ƶϵ��))
	//				= 666 / (8000000 / 12)
	//				= 1 ms

	//��ʱ1ms
	//���Ƴ�ֵ 	= 65536 - ((1/1000) / (1/(Fosc / Timer��Ƶϵ��)))
	//		   	= 65536 - ((1/1000) / (1/(8000000 / 12)))
	//			= 65536 - 666
	//			= 0xFD64
	//TH0 = 0xFD;
	//TL0 = 0x64;							//T0��ʱʱ��1ms
	TH0 = 0X7D;
	TL0 = 0x88;                         //T0��ʱʱ��50ms
	IE |= 0x02;							//��T0�ж�
	TCON &= 0xE0;						//�ر�ʹ��T0
	//TCON |= 0x10;
}

void Timer_enable(unsigned int timer, unsigned int enable)
{
	if(timer){
		if(enable)
			TCON |= 0x40;                       //ʹ��T1
		else
			TCON &= 0xB0;
	} else {
		if(enable)
			TCON |= 0x10;						//ʹ��T0
		else
			TCON &= 0xE0;
	}
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
	rtc_count++;
	RTCC &= ~0x80;					   //??3y?D??����????
	if (rtc_count >= 3) {
		LED1 = 1;
		rtc_count = 0;
		ERTC = 0; 							//disable RTC�ж�
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
	ERTC = 0;							//disable RTC�ж�
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
