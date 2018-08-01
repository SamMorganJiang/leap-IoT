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

void System_init(void);
void IO_Init();

void Delay_us(u32 delayCnt)
{
	u32 temp0 =0;
	for(temp0 =0; temp0 < delayCnt;temp0++)
	{
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
	for(temp0 =0; temp0 < delayCnt;temp0++)
	{
        Delay_us(1132);
	}
}


void main(void)
{
	unsigned short step_num = 0;
	System_init();	   					//ϵͳ��ʼ��
	IO_Init();							//IO��ʼ��
	SPI_Init();            //SPI��ʼ��
	IIC_Init();
	//ES1 = 1;	 						//�������ж�
	EA = 1;								//ʹ�����ж�

	if(qmaX981_get_chip_id())
	{
		qma6981_initialize();
	}
	BLE_Init();
	while(1)
	{
		//////ble rtx api
		//txcnt=100; //txcnt=0 is for rx only application
		//rxcnt=20; //rxcnt=0 is for tx only application
		//qma6981_read_raw_xyz();
		step_num = qmaX981_step_c_read_stepCounter();
		BLE_Set_rxtxcnt(100, 20);
		BLE_TRX();
		//Uart_Send_String("BLE trx done.\r\n");

		//delay to set ble tx interval
		Delay_ms(100);
	}
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

void IO_Init()
{
	P5M2 = 0x00;
}
