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
unsigned char data i2c_buf[3]=0;				//���ڴ�Ŷ�д����
unsigned char data step_data=0;					//���ڴ�Ŷ�д����
//SPI
//unsigned char hello[]="hello QST\r";
unsigned char data id[2];									//���ڴ洢ID��
unsigned char data SPI_buf[16];								//���ڴ洢��ȡ����
unsigned char data QMS7912_start_Flag;
//UART
unsigned char data bufdat = 0;				//���ڴ�źͽ�������
unsigned char data bufflag = 0;		
int data res;
unsigned char data slave_add;
unsigned char data slave_dat;
unsigned char data str[4];

int txcnt = 0;
int rxcnt = 0;
int BLE_Test_Case;
int BLE_Test_Ruslt;				

void System_init(void);					//ϵͳ��ʼ��
void IO_Init(void);						//IO��ʼ��
void UART_Init(void);					//UART��ʼ��
void SendChar(unsigned char Char);
void SendString(unsigned char *p);
unsigned char* CharToString(unsigned char n);
unsigned char* CharToHexStr(unsigned char n);

/***************************************************************************************
 * @ʵ��Ч��	UART���յ���λ�����͵�8λ���ݺ󣬰ѽ��յ������ٴη��͸���λ��
 * @ע��		������1200��8λ���ݣ���У��λ		
 ***************************************************************************************/
void main(void)
{
    System_init();	   					//ϵͳ��ʼ��
    IO_Init();							//IO��ʼ��
    UART_Init();						//UART��ʼ��
    IIC_Init();            				//IIC��ʼ��
    SPI_Init();            				//SPI��ʼ��
    ES1 = 1;	 						//�������ж�
    EA = 1;								//ʹ�����ж�

    qma6981_initialize();
    res = qmaX981_get_chip_id();
    if (res == 1)
        SendString("QMAX981_Read_ChipID_Susscess!\n");

    MG127_Init();
    SendString("MG127_Init_Susscess!\n");

    QMS7912_start_Flag = 4;

    while(1) {
        if (bufflag) {
            SBUF = bufdat;				//��������
            QMS7912_start_Flag = bufdat;
            while(!(SCON&0x02));
            SCON &=~ 0x02;				//�����ж������жϱ�־λ��0
            bufflag = 0;				//��־λ��0
            SCON |= 0x10; 				//�����ڽ���  
			ES1 = 1;	 				//�������ж�
        }

        if (QMS7912_start_Flag == 1) {//BLE ���ͽ��ղ���
            txcnt=3; //txcnt=0 is for rx only application
            rxcnt=6; //rxcnt=0 is for tx only application
            BLE_TRX();
        }

        if (QMS7912_start_Flag == 2) {//BLE FT��ز���
            //FT_TxGain_Init();
            IIC_Slave_Init(); 
            while(1) {
                res = qmaX981_get_chip_id();//IIC_Slave_Recive_Byte(slave_add,slave_dat);
            }
        }

        if (QMS7912_start_Flag == 3) {//MCU ADC��ز���
            ADC_TestInit();
            GPIO_TestPinInit();
            ADC_MCUTest();
        }

        if (QMS7912_start_Flag == 4) {//���ٶ�ԭʼ����
            qma6981_read_raw_xyz(&i2c_buf);
            SendString("QMAX981_Read_Rawdata[X ");
			SendString(CharToHexStr(i2c_buf[0]));            //SendChar(i2c_buf[0]);
            SendString(",Y ");
			SendString(CharToHexStr(i2c_buf[1]));            //SendChar(i2c_buf[1]);
            SendString(",Z ");
			SendString(CharToHexStr(i2c_buf[2]));            //SendChar(i2c_buf[2]);
            SendString("]\n");
        }

        if (QMS7912_start_Flag == 5) {//���ٶȼƼƲ���
            qmaX981_step_c_read_stepCounter(step_data);
            SendString("QMAX981_Read_Stepdata[");
            SendString(CharToHexStr(step_data));			//SendChar(step_data);
            SendString("]\n");
        }
    }
}

/***************************************************************************************
 * @˵��  	UART�жϷ�����
 * @����	��
 * @����ֵ  ��
 * @ע		��
 ***************************************************************************************/
void Usart_Ipt() interrupt UART1_VECTOR
{
    if (SCON&0x01) {
        SCON &=~ 0x01;   				//�յ����ݺ�������ձ�־
        bufdat = SBUF & 0x0f;			//�������ݣ�ֻȡ��4λ��
        bufflag = 1;
        SCON &=~ 0x10; 					//�ش��ڽ���  		 
        ES1 = 0;	 					//�ش����ж�
    }
}

/***************************************************************************************
 * @˵��  	ϵͳ��ʼ��
 * @����	��
 * @����ֵ  ��
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

void SendChar(unsigned char Char)
{	
    SBUF = Char;
    while(!(SCON&0x02));
    SCON &=~ 0x02;				//�����ж������жϱ�־λ��0
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
 * @˵��  	IO��ʼ��
 * @����	��
 * @����ֵ  ��
 * @ע		P00�������룬P01�������
 ***************************************************************************************/
void IO_Init(void)
{
    //P0M0=0x82; 							//P00�������룬P01�������
}

/***************************************************************************************
 * @˵��  	UART��ʼ��
 * @����	��
 * @����ֵ  ��
 * @ע		8λ���ݷ��ͣ�������1200
 ***************************************************************************************/
void UART_Init(void)
{
    P1M3 = 0x82;
    TXD_MAP = 0x0F;						//TXDӳ��P17
    RXD_MAP = 0x0E;						//RXDӳ��P16

    SBRTH = 0xFE5F/256;
    SBRTL = 0xFE5F%256;
    SCON2 = 0x12;						//����������
    SCON |= 0X10; 						//�����ڽ���  
}
