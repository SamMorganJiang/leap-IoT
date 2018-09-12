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

unsigned char data bufdat = 0;				//用于存放和接受数据
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

	System_init();	   					//系统初始化
	IO_Init();							//IO初始化
	RCT_Init();							//RTC初始化
	SPI_Init();                         //SPI初始化
	IIC_Init();
	//UART_Init();
	//ES1 = 1;	 						//开串口中断

	while(1) {
		if (KEY == 1) {
			Delay_ms(30);
			if (KEY == 1) {
				LED1 = ~LED1;
				LED2 = ~LED2;
			}
		}
	}

	ERTC=0;								//disable RTC中断
	EA = 1;								//使能总中断

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
			ERTC=0; 							//disable RTC中断
			step_num = qmaX981_step_c_read_stepCounter();
			//BLE_Set_rxtxcnt(100, 20);
			BLE_Set_rxtxcnt(500, 0);
			BLE_TRX();

			//delay to set ble tx interval
			Delay_ms(100);
			ERTC=1; 							//enable RTC中断
			// add by yangzhiqiang enter power done mode
			PCON |= 0x03;
			// yangzhiqiang
		}
	}
}

/***************************************************************************************
  * @说明  	延时函数
  * @参数  	timdlay ：延时设定值
  *			取值范围: 1 - 65536.
  * @返回值 无
  * @注		粗略延时
***************************************************************************************/
void delay_by_ms(unsigned int timdlay)
{
	unsigned char j;
	for(;timdlay>0;timdlay--)
	for(j=100;j>0;j--);
}

/***************************************************************************************
  * @说明  	系统初始化
  *	@参数	无
  * @返回值 无
  * @注		关闭看门狗以及时钟初始化
***************************************************************************************/
void System_init(void)
{
	WDTCCR = 0x00;						//关闭看门狗

	while((CLKCON&0x20)!=0x20);			//等待内部高频晶振起振
	CLKDIV = 0x02;						//CPU时钟2分频，确保在进行RC32分频时CPU时钟小于16M
	CLKSWR &=~ 0x10;					//切换为内部高频时钟
	while((CLKSWR&0x40)!=0x00);			//等待内部高频切换完成
	CLKDIV = 0x04;						//OSC时钟4分频
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

	P1M3 = 0xCC;                        //LED1、2推挽输出( IO 驱动电流 Low Drive Mode)
	LED1 = LED2 = 0;

	P3M2 &= 0xF0;
	P3M2 |= 0x05;                       //KEY 带下拉输入(SMT)
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
  * @?μ?÷  	RTC?D??·t??oˉêy
  *	@2?êy	?T
  * @·μ???μ ?T
  * @×￠		?T
***************************************************************************************/
void Rtc_Irt()interrupt RTC_VECTOR
{
//	P0_1 =! P0_1; 					   //P01·-×a
	rtc_count++;
	RTCC &= ~0x80;					   //??3y?D??±ê????
	if(rtc_count >= 5)
	{
		rtc_count = 0;
		ERTC=0; 							//disable RTC中断
		PCON &= (~0x03);
	}
}

/***************************************************************************************
  * @?μ?÷  	RTC3?ê??ˉ
  *	@2?êy	?T
  * @·μ???μ ?T
  * @×￠		ê1?üRTC?D??ò??°RTCê?3?￡??D??ê±??1s
***************************************************************************************/
void RCT_Init(void)
{
	CLKCON |= 0x04;						//使能外部晶振
	while((CLKCON&0x40)!=0x40);			//等待外部低频晶振起振
	//RTCC = 0x13;				   		//RTC中断时间1s,允许RTC时钟输出
	RTCC = 0x1f;				   		//RTC中断时间8s,允许RTC时钟输出
	RTCO_MAP = 0x03;					//RCT时钟输出映射P0.3口
}
