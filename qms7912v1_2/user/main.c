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
	System_init();	   					//系统初始化
	IO_Init();							//IO初始化
	SPI_Init();            //SPI初始化
	IIC_Init();
	//ES1 = 1;	 						//开串口中断
	EA = 1;								//使能总中断

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

void IO_Init()
{
	P5M2 = 0x00;
}
