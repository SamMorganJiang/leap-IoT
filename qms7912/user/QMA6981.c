#include "I2C.h"
#include "QMA6981.h"
#include "Timer.h"
#include "Uart.h"

void qma6981_initialize(void)
{	
    IIC_Send_Byte(0x11, 0x80);
    IIC_Send_Byte(0x36, 0xb6);
    delay(5);
    IIC_Send_Byte(0x36, 0x00);
    IIC_Send_Byte(0x11, 0x80);
    IIC_Send_Byte(0x0f, 0x04);//QMAX981_RANGE_8G
    IIC_Send_Byte(0x10, 0x2a);
    IIC_Send_Byte(0x12, 0x8f);
    IIC_Send_Byte(0x13, 0x10);
    IIC_Send_Byte(0x14, 0x14);
    IIC_Send_Byte(0x15, 0x10);	
    IIC_Send_Byte(0x16, 0x0c);
    IIC_Send_Byte(0x32, 0x01);//0x02
    IIC_Send_Byte(0x27, 0x60);//QMA6981_OFFSET
    IIC_Send_Byte(0x28, 0x60);
    IIC_Send_Byte(0x29, 0x60);
    delay(1);
}

unsigned char qmaX981_get_chip_id(void)
{
    unsigned char  ret;
    unsigned char  chip_id ;

    IIC_Send_Byte(0x11,0x80);
    delay(2);
    chip_id = (unsigned char)IIC_Read_Byte(0x00);
    SendChar(chip_id);
    if((chip_id >= 0xb0) && (chip_id <= 0xb6))
        ret = 1;
    else
        ret = 0;

    return ret;
}

void qma6981_read_raw_xyz(unsigned char *acc)
{
    unsigned char  buf[6] = {0};		
    unsigned char  i;	

    buf[0] = IIC_Read_Byte(0x01);	
    buf[1] = IIC_Read_Byte(0x02);
    buf[2] = IIC_Read_Byte(0x03);
    buf[3] = IIC_Read_Byte(0x04);
    buf[4] = IIC_Read_Byte(0x05);
    buf[5] = IIC_Read_Byte(0x06);
#if 0
	SendString("<--- debug (");
	for (i = 0; i < 6; i++) {
		SendChar(buf[i]);
		SendString(" ");
	}
	SendString(") --->\n");
#endif
    acc[0]  = (short)((buf[1]<<2) |( buf[0]>>6));
    acc[1]  = (short)((buf[3]<<2) |( buf[2]>>6));
    acc[2]  = (short)((buf[5]<<2) |( buf[4]>>6));

    for(i=0; i<3; i++) {
        if ( acc[i] == 0x0200 )
            acc[i]= -512;
        else if ( acc[i] & 0x0200 ) {
            acc[i] -= 0x1;
            acc[i] = ~acc[i];
            acc[i] &= 0x01ff;
            acc[i] = -acc[i];
        }
        acc[i] -= 0x60;
    }

    acc[0] = (int)(acc[0]*9807)/(1024);//8G 1024 //4G 2048
    acc[1] = (int)(acc[1]*9807)/(1024);
    acc[2] = (int)(acc[2]*9807)/(1024);
}

void qmaX981_reset_sc(void)
{
    unsigned char  value_13;
    value_13= IIC_Read_Byte(0x13);
    IIC_Send_Byte(0x13,0x80);
    IIC_Send_Byte(0x13,value_13);
}

void qmaX981_step_c_read_stepCounter(unsigned char  result)
{
    unsigned char  buf[2];

    buf[0] = IIC_Read_Byte(0x07);
    buf[1] = IIC_Read_Byte(0x08);
    result = (unsigned char) (buf[1] << 8) | buf[0];
}
