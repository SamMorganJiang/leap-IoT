#define	ALLOCATE_EXTERN
#include <stdio.h>
#include <stdlib.h>
#include "HC89F0650.h"
#include <HC_DEFINE.H>
#include "Uart.h"
#include "SPI.h"
#include "I2C.h"
#include "QMA6981.h"
#include "MG127.h"
#include "I2C_SLAVE.h"
#include "Test_Prom.h"

//I2C
unsigned char data i2c_buf[3]=0;				//用于存放读写数据
unsigned char data step_data=0;					//用于存放读写数据
//SPI
//unsigned char hello[]="hello QST\r";
unsigned char data id[2];									//用于存储ID号
unsigned char data SPI_buf[16];								//用于存储读取数据
unsigned char data QMS7912_start_Flag;
//UART
unsigned char data bufdat = 0;				//用于存放和接受数据
unsigned char data bufflag = 0;		
int data res;
unsigned char data slave_add;
unsigned char data slave_dat;
unsigned char data str[4];

int txcnt = 0;
int rxcnt = 0;
int BLE_Test_Case;
int BLE_Test_Ruslt;				

void System_init(void);					//系统初始化
void IO_Init(void);						//IO初始化
void UART_Init(void);					//UART初始化
void SendChar(unsigned char Char);
void SendString(unsigned char *p);
unsigned char* CharToString(unsigned char n);
unsigned char* CharToHexStr(unsigned char n);

/***************************************************************************************
 * @实现效果	UART接收到上位机发送的8位数据后，把接收的数据再次发送给上位机
 * @注意		波特率1200，8位数据，无校验位		
 ***************************************************************************************/
void main(void)
{
    System_init();	   					//系统初始化
    IO_Init();							//IO初始化
    UART_Init();						//UART初始化
    IIC_Init();            				//IIC初始化
    SPI_Init();            				//SPI初始化
    ES1 = 1;	 						//开串口中断
    EA = 1;								//使能总中断

    qma6981_initialize();
    res = qmaX981_get_chip_id();
    if (res == 1)
        SendString("QMAX981_Read_ChipID_Susscess!\n");

    MG127_Init();
    SendString("MG127_Init_Susscess!\n");

    QMS7912_start_Flag = 4;

    while(1) {
        if (bufflag) {
            SBUF = bufdat;				//发送数据
            QMS7912_start_Flag = bufdat;
            while(!(SCON&0x02));
            SCON &=~ 0x02;				//发送中断请求中断标志位清0
            bufflag = 0;				//标志位置0
            SCON |= 0x10; 				//开串口接收  
			ES1 = 1;	 				//开串口中断
        }

        if (QMS7912_start_Flag == 1) {//BLE 发送接收测试
            txcnt=3; //txcnt=0 is for rx only application
            rxcnt=6; //rxcnt=0 is for tx only application
            BLE_TRX();
        }

        if (QMS7912_start_Flag == 2) {//BLE FT相关测试
            //FT_TxGain_Init();
            IIC_Slave_Init(); 
            while(1) {
                res = qmaX981_get_chip_id();//IIC_Slave_Recive_Byte(slave_add,slave_dat);
            }
        }

        if (QMS7912_start_Flag == 3) {//MCU ADC相关测试
            ADC_TestInit();
            GPIO_TestPinInit();
            ADC_MCUTest();
        }

        if (QMS7912_start_Flag == 4) {//加速度原始数据
            qma6981_read_raw_xyz(&i2c_buf);
            SendString("QMAX981_Read_Rawdata[X ");
			SendString(CharToHexStr(i2c_buf[0]));            //SendChar(i2c_buf[0]);
            SendString(",Y ");
			SendString(CharToHexStr(i2c_buf[1]));            //SendChar(i2c_buf[1]);
            SendString(",Z ");
			SendString(CharToHexStr(i2c_buf[2]));            //SendChar(i2c_buf[2]);
            SendString("]\n");
        }

        if (QMS7912_start_Flag == 5) {//加速度计计步器
            qmaX981_step_c_read_stepCounter(step_data);
            SendString("QMAX981_Read_Stepdata[");
            SendString(CharToHexStr(step_data));			//SendChar(step_data);
            SendString("]\n");
        }
    }
}

/***************************************************************************************
 * @说明  	UART中断服务函数
 * @参数	无
 * @返回值  无
 * @注		无
 ***************************************************************************************/
void Usart_Ipt() interrupt UART1_VECTOR
{
    if (SCON&0x01) {
        SCON &=~ 0x01;   				//收到数据后，清除接收标志
        bufdat = SBUF & 0x0f;			//接收数据（只取低4位）
        bufflag = 1;
        SCON &=~ 0x10; 					//关串口接收  		 
        ES1 = 0;	 					//关串口中断
    }
}

/***************************************************************************************
 * @说明  	系统初始化
 * @参数	无
 * @返回值  无
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

void SendChar(unsigned char Char)
{	
    SBUF = Char;
    while(!(SCON&0x02));
    SCON &=~ 0x02;				//发送中断请求中断标志位清0
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
 * @说明  	IO初始化
 * @参数	无
 * @返回值  无
 * @注		P00上拉输入，P01推挽输出
 ***************************************************************************************/
void IO_Init(void)
{
    //P0M0=0x82; 							//P00上拉输入，P01推挽输出
}

/***************************************************************************************
 * @说明  	UART初始化
 * @参数	无
 * @返回值  无
 * @注		8位数据发送，波特率1200
 ***************************************************************************************/
void UART_Init(void)
{
    P1M3 = 0x82;
    TXD_MAP = 0x0F;						//TXD映射P17
    RXD_MAP = 0x0E;						//RXD映射P16

    SBRTH = 0xFE5F/256;
    SBRTL = 0xFE5F%256;
    SCON2 = 0x12;						//独立波特率
    SCON |= 0X10; 						//开串口接收  
}
