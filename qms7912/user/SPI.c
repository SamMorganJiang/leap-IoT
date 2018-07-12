#include "HC89F0650.h"
#include "intrins.h"
#include "SPI.h"
#include "Timer.h"
#include "MG127.h"
#include "Uart.h"

sbit	CSN=P0^0;										//P00 CS
sbit	SCK=P0^1;										//P03 SCK
sbit    IRQ=P0^5;                   					//P05 IRQ
sbit	SI=P0^2;										//P02 SI
sbit	SO=P0^2;	


/* Private macro -------------------------------------------------------------*/
#define LEN_BLE_ADDR 6


/* Private variables ---------------------------------------------------------*/
unsigned short  tick;

//unsigned char  rx_buf[39]; //include header(2B)+mac(6B)+(max31B), for rx application

//BLE ADV_, maxlen=31
#define LEN_ 31

code int adv_[31] = {0x02,0x01,0x04, 0x1a,0xff,0x4c,0x00,2,0x15, 0xfd};

int BLE_IRQ_GET(void)
{
	return IRQ ? 1 : 0;
}

void SPI_Init(void)
{
    P0M0 = 0xcc;	
}

void SPI_Write_Byte(int data1) 
{ 
    int data i;
    P0M1 = 0x6c;
    for(i = 0; i < 8; i++) {
        SCK = 0;
        if(data1 & 0x80)
            SI = 1;
        else
            SI = 0;

        SCK = 1;
        data1 <<= 1;
    }
    SCK = 0;
    SI = 1;
} 

int SPI_Read_Byte(void) 
{ 
    int ret = 0;
    int i = 0;

    P0M1 = 0xc6;	
    for(i = 0; i < 8; i++) {
        SCK = 0;
        SCK = 1;

        ret <<= 1;
        if(SI != 0)
            ret |= 1;
    }
    P0M1 = 0x6c;
    SCK = 0;
    return ret;
} 

void SPI_Write_Reg(int reg, int datae)
{ 
    CSN = 0;

    SPI_Write_Byte(reg);
    SPI_Write_Byte(datae);

    CSN = 1;
} 

int SPI_Read_Reg(int reg) 
{ 
    int data temp0=0;

    CSN = 0;

    SPI_Write_Byte(reg);
    temp0 = SPI_Read_Byte();

    CSN = 1;
    return temp0;
} 

void SPI_Write_Buffer(int reg, int *dataBuf, int len) 
{ 
    int data temp0=0;

    CSN = 0;

    delay(5);
    SPI_Write_Byte(reg|0x20);

    for(temp0 = 0; temp0 < len; temp0++) {
        SPI_Write_Byte(*dataBuf);
        dataBuf++;
    }

    delay(5);

    CSN = 1;
} 

void SPI_Read_Buffer(int reg, int *dataBuf, int len) 
{ 
    int data temp0=0;

    CSN = 0;
    delay(5);

    SPI_Write_Byte(reg);

    for(temp0 = 0; temp0 < len; temp0++)
        *(dataBuf+temp0) = SPI_Read_Byte();

    delay(5);

    CSN = 1;
}

void MG127_Init(void)
{
    unsigned char  status;
	char i;
    int  _buf[4];
    int  ble_Addr[6];

	IRQ = 1;
    SPI_Write_Reg(0x50, 0x51);
    SPI_Write_Reg(0x50, 0x53);
    SPI_Write_Reg(0x35, 0x00);
    SPI_Write_Reg(0x3D, 0x18);
    SPI_Write_Reg(0x50, 0x51);

    do {
        SPI_Write_Reg(0x50, 0x53);

        _buf[0] = 0;
        _buf[1] = 0;
        _buf[2] = 1;
        SPI_Write_Buffer(0x00, _buf, 3);

        SPI_Write_Reg(0x36, 0x8e);
        SPI_Write_Reg(0x37, 0x88);
        SPI_Write_Reg(0x38, 0x88);
        SPI_Write_Reg(0x39, 0x8e);
        SPI_Write_Reg(0x50, 0x51);
        status = SPI_Read_Reg(0x1e);
#if 0
		SendString("<--- debug1 ");
		SendChar(status);
		SendString(" ---> ");
#endif
        status = SPI_Read_Reg(CHIP_OK);
#if 0
		SendString("<--- debug2 ");
		SendChar(status);
		SendString(" --->\n");
#endif
    } while(status != 0x80);

#if 1
    //read c=hip version
    status=SPI_Read_Reg(0x1e);
    SendString("chip version=");
    SendString(CharToHexStr(status));	//SendChar(status);
    SendString("\r\n");
#endif

    SPI_Write_Reg(0X20, 0x78);//power down,tx, for hot reset
    SPI_Write_Reg(0X26, 0x06);//1Mbps
    SPI_Write_Reg(0X20, 0x7a);//power up
    SPI_Write_Reg(0x50, 0x56);
    BLE_Mode_Sleep();

#if 1
    //read BLE address. BLE MAC Address
    SPI_Read_Buffer(0x08, ble_Addr, 6);
    SendString("BleAddr=");
	for(i = 5; i >= 0; i--) {
		SendString(CharToHexStr(ble_Addr[i]));//SendChar(ble_Addr[i]);
		SendString(" ");
    }
    SendString("\r\n");
#endif

    SPI_Write_Reg(0x50, 0x53);

    _buf[0] = 0xff;
    _buf[1] = 0x80; //xocc
    SPI_Write_Buffer(0x14,_buf,2);

    //set BLE TX Power
    _buf[0] = 0x02;
    _buf[1] = BLE_TX_POWER;
    SPI_Write_Buffer(0x0f,_buf,2);

    _buf[1] = SPI_Read_Reg(0x08);  //txgain
    if(0 == _buf[1])
        _buf[1] = TXGAIN_DFF;
    _buf[0] = 0xc0;
    _buf[2] = 0x1D; // 1E, 20161212
    SPI_Write_Buffer(0x4, _buf, 3);


    _buf[0] = 0;
    _buf[1] = 0x00; //default08  20161212
    SPI_Write_Buffer(0x0C, _buf, 2);

    _buf[0] = 0x81;
    _buf[1] = 0x22;
    SPI_Write_Buffer(0x13, _buf, 2);

    SPI_Write_Reg(0X21, 0x02);
    SPI_Write_Reg(0x3C, 0x30);
    SPI_Write_Reg(0x3E, 0x30);

    _buf[0] = 0x10;
    _buf[1] = 0x02;
    SPI_Write_Buffer(0xA, _buf, 2);

    _buf[0] = 0x02;
    _buf[1] = 0x12;
    SPI_Write_Buffer(0xD, _buf, 2);

    _buf[0] = 0x01;
    _buf[1] = 0x07;
    SPI_Write_Buffer(0xE, _buf, 2);

    SPI_Write_Reg(0x50, 0x56);
}

void BLE_Mode_Sleep(void)
{
    int	temp0[4] = {0x02, 0xff, 0xff, 0xff};
    SPI_Write_Buffer(SLEEP_WAKEUP,temp0,4);
}

void BLE_Mode_Wakeup(void)
{
    SPI_Write_Reg(SLEEP_WAKEUP|0x20, 0x01);
}

void BLE_Mode_PwrUp(void)
{
    SPI_Write_Reg(0X50, 0x51);
    SPI_Write_Reg(0X20, 0x7a); //pwr up

    SPI_Write_Reg(0x50, 0x53);
    SPI_Write_Reg(0X35, 0x00);
    BLE_Do_Cal();
}

void BLE_Mode_PwrDn(void)
{
    int  temp[2] = {0x81, 0x02};

    SPI_Write_Reg(0X50, 0x51);
    SPI_Write_Reg(0X20, 0x78); //pwr down

    SPI_Write_Reg(0X50, 0x53);
    SPI_Write_Reg(0X35, 0x01);  //tm

    //temp[0] = 0x81;
    //temp[1] = 0x02;
    SPI_Write_Buffer(0x13, temp, 2);
    SPI_Write_Reg(0X3e, 0xa0);

    SPI_Write_Reg(0X50, 0x56);
}

void BLE_Set_StartTime(int  htime)
{
    int  temp0[3];

    temp0[0] = htime & 0xFF;
    temp0[1] = (htime>>8) & 0xFF;
    temp0[2] = (htime>>16) & 0xFF;

    SPI_Write_Buffer(START_TIME,temp0,3);
}
void BLE_Set_TimeOut(int _us)
{
    int temp0[3];

    temp0[0] = _us & 0xff;
    temp0[1] = (_us >> 8) & 0xff;
    temp0[2] = (_us >> 16) & 0xff;

    SPI_Write_Buffer(TIMEOUT, temp0, 3);
}

void BLE_Set_Xtal(int on_flag)
{
    SPI_Write_Reg(0x50, 0x53);
    SPI_Write_Reg(0x3D, 0x18|(on_flag<<2));
    SPI_Write_Reg(0x50, 0x56);
}

static void BLE_Get_Pdu(int *ptr, int *len)
{
    int hdr_type;
    int len_tmp;
    int bank_buf[6];

    SPI_Read_Buffer(ADV_HDR_RX, bank_buf, 2);

    *len = bank_buf[1] + 2;
    ptr[0] = bank_buf[0];
    ptr[1] = bank_buf[1];

    hdr_type = bank_buf[0] & 0xF;
    len_tmp = bank_buf[1];
    switch(hdr_type){
        case ADV_IND:  //advA+0~31
        case ADV_NONCONN_IND:
            //case ADV_SCAN_IND:
            //case ADV_SCAN_RSP:
            SPI_Read_Buffer(INITA_RX, &ptr[2], LEN_BLE_ADDR);  //INITA
            len_tmp -= LEN_BLE_ADDR;
            break;
            /*
            case ADV_DIRECT_IND:  //advA+InitA
            if(len_tmp == 12){
            SPI_Read_Buffer(ADVA_RX, &ptr[2], LEN_BLE_ADDR);  //advA
            len_tmp -= LEN_BLE_ADDR;
            SPI_Read_Buffer(INITA_RX, &ptr[2+LEN_BLE_ADDR], LEN_BLE_ADDR);  //INITA
            len_tmp -= LEN_BLE_ADDR;
            }
            len_tmp = 0;
            break;
            case ADV_SCAN_REQ:  //scanA + advA
            if(len_tmp == 12){
            SPI_Read_Buffer(INITA_RX, &ptr[2], LEN_BLE_ADDR);  //INITA
            len_tmp -= LEN_BLE_ADDR;
            SPI_Read_Buffer(ADVA_RX, &ptr[2+LEN_BLE_ADDR], LEN_BLE_ADDR);  //advA
            len_tmp -= LEN_BLE_ADDR;
            }
            len_tmp = 0;
            break;
            case ADV_CONN_REQ:  //InitA + advA + LL(22B)
            if(len_tmp == 34){
            SPI_Read_Buffer(INITA_RX, &ptr[2], LEN_BLE_ADDR);  //INITA
            SPI_Read_Buffer(ADVA_RX, &ptr[2+LEN_BLE_ADDR], LEN_BLE_ADDR);  //advA
            SPI_Read_Buffer(R_RX_PAYLOAD, &ptr[2+LEN_BLE_ADDR+LEN_BLE_ADDR], 22);
            }
            len_tmp = 0;
            break;
            */
        default:
            len_tmp = 0;
            break;
    }

    if(len_tmp > 0){
        SPI_Read_Buffer(R_RX_PAYLOAD, &ptr[2+LEN_BLE_ADDR], len_tmp);
    }
}


#define TXGAIN_DFF 0x15

void BLE_Do_Cal()  //calibration
{
    int _buf[2];

    SPI_Write_Reg(0x3F, 0x03);
    do {
        _buf[0] = SPI_Read_Reg(0x1F);
    } while(_buf[0]&0x03);

    SPI_Write_Reg(0x3F, 0x03);
    do {
        _buf[0] = SPI_Read_Reg(0x1F);
    } while(_buf[0]&0x03);

    //////////////////////////////////////////////////
    SPI_Write_Reg(0x35,0x01); //testm for tx/temp
    SPI_Write_Reg(0x32,0xA0);
    SPI_Write_Reg(0x2a,0x04);
    SPI_Write_Reg(0x2a,0x00);

    SPI_Write_Reg(0x32,0x88);
    _buf[0] = 0x01;
    _buf[1] = 0x21;
    SPI_Write_Buffer(0x13, _buf, 2);
    _buf[0] = 0x01;
    _buf[1] = 0x20;
    SPI_Write_Buffer(0x13, _buf, 2);
    SPI_Write_Reg(0x35,0x00);  //exist testm
    ////////////////////////////////////////////////////
    SPI_Write_Reg(0x50, 0x56);
}

//2480MHz carrier, for xtal capacitor tuning
//call this function after BLE_Init() in main()
void Carrier(void)
{
    unsigned long delay=0x14000;

    SPI_Write_Reg(0x50, 0x51);
    SPI_Write_Reg(0x50, 0x53);

    SPI_Write_Reg(0x3d, 0x1e);
    while(delay--);

    SPI_Write_Reg(0x50, 0x56);
    SPI_Write_Reg(0x20,0x0);

    SPI_Write_Reg(0x50, 0x53);	
    SPI_Write_Reg(0x36, 0x8c);  //
    SPI_Write_Reg(0x50, 0x51);

    SPI_Write_Reg(0x21,0x00);
    SPI_Write_Reg(0x25,0x50);
    SPI_Write_Reg(0x24,0x00);

    SPI_Write_Reg(0x20,0x0e);
    SPI_Write_Reg(0x26,0x96); //for cont wave
    SPI_Write_Reg(0xe3,0xff); 	

    SPI_Write_Reg(0x50, 0x53);
    SPI_Write_Reg(0x36, 0x8e); //

    SPI_Write_Reg(0x3d, 0x18);

    while(1){};
}

void XOClockOutput(void)
{
    int _buf[3];

    SPI_Write_Reg(0x50, 0x51);
    SPI_Write_Reg(0x50, 0x53);

    SPI_Write_Reg(0x35,0x01);
    _buf[0] = 0x93;
    _buf[1] = 0x00;
    SPI_Write_Buffer(0x14, _buf, 2);
    SPI_Write_Reg(0x3e,0x30);
    SPI_Write_Reg(0x31,0x00);
    SPI_Write_Reg(0x3b,0x48);
    _buf[0] = 0x00;
    _buf[1] = 0x00;
    _buf[2] = 0x15;
    SPI_Write_Buffer(0x12, _buf, 3);

    while(1){};
}

/*******************************************************************************
 * Function   :     	BLE_TRX
 * Parameter  :     	txcnt, rxcnt
 * Returns    :     	void
 * Description:      Beacon  .process .
 * Note:      :
 *******************************************************************************/
void BLE_TRX()
{
    int status = 0;
    int ch = 37;
    int _buf[2];
	int rx_buf[12];
    int tmp_cnt = txcnt+rxcnt;
    int len_pdu = 0;
    int loop = 0;

    if(tmp_cnt == 0)
		return;

    BLE_Set_Xtal(1);
    BLE_Mode_PwrUp();

#if 1  //if adv_ no change, can move this block to the end of BLE_Init()
    //set BLE TX default channel:37.38.39
    SPI_Write_Reg(CH_NO|0X20, ch);

    //BLT FIFO write adv_ . max len:31 byte
    SPI_Write_Buffer(W_TX_PAYLOAD, adv_, LEN_);

    //PDU TYPE: 2  non-connectable undirected advertising . tx add:random address
    //set BLT PDU length:adv_+6 mac adress.
    _buf[0] = 0x02;
    _buf[1] = LEN_+LEN_BLE_ADDR;
    SPI_Write_Buffer(ADV_HDR_TX, _buf, 2);

    //clear all interrupt
    _buf[0] = 0xFF;
    _buf[1] = 0x80;
    SPI_Write_Buffer(INT_FLAG, _buf, 2);
#endif

    BLE_Mode_Wakeup();

    BLE_Set_TimeOut(BLE_RX_TIMEOUT);
    tick = BLE_GUARD_TIME;

	while(1) {
		//BLE IRQ LOW
        if (!BLE_IRQ_GET()) {
			SendString("<--- IRQ LOW --->\n");
            //clear interrupt flag
            status = SPI_Read_Reg(INT_FLAG);
            SPI_Write_Reg(INT_FLAG|0X20, status);
            //Uart_Send_Byte(status); //debug

            if (INT_TYPE_WAKEUP & status) {//wakeup
				SendString("<--- ble irq wakeup --->\n");
                if (txcnt > 0) {
                    txcnt --;
                    SPI_Write_Reg(MODE_TYPE|0X20, RADIO_MODE_ADV_TX);
                    BLE_Set_StartTime(BLE_START_TIME);
                } else if (rxcnt > 0) {
                    rxcnt --;
                    SPI_Write_Reg(MODE_TYPE|0X20, RADIO_MODE_ADV_RX);
                    BLE_Set_StartTime(BLE_START_TIME);
                }
                continue; //goto while(1)
            }

			SendString("<--- ble enter sleep --->\n");
            BLE_Mode_Sleep();

            if (INT_TYPE_PDU_OK & status) { //only happen in rx application, no need porting in tx only application
				SendString("<--- INT_TYPE_PDU_OK --->\n");
                BLE_Get_Pdu(rx_buf, &len_pdu);
            } else if (INT_TYPE_TX_START & status){ //only happen in tx application
                //  LED_GREEN_ON(); //debug
            }

            if (INT_TYPE_SLEEP & status) {//sleep
				SendString("<--- INT_TYPE_SLEEP --->\n");
                tick = BLE_GUARD_TIME;

                //BLE channel
                if (++ch > 39)
                    ch = 37;
                SPI_Write_Reg(CH_NO|0X20, ch);

                tmp_cnt --;
                if (tmp_cnt == 0) {
                    BLE_Set_Xtal(0);
                    BLE_Mode_PwrDn();
                    break; //exit from while(1)
                }
                else
                    BLE_Mode_Wakeup();
            }
        }
        else if (tick == 0)//robustness, in case no int
            BLE_Mode_Sleep();
    }
}

void FT_TxGain_Init(void)
{
    unsigned long delay = 0x30;
    int data_buf[2] = {0xc0, 0x00};
    SPI_Write_Reg(0x50, 0x51);
    SPI_Write_Reg(0x50, 0x53);
    SPI_Write_Reg(0x3d, 0x1e);
    while(delay--);
    SPI_Write_Reg(0x50, 0x56);
    SPI_Write_Reg(0x20,0x0);
    SPI_Write_Reg(0x50, 0x53);
    SPI_Write_Reg(0x36, 0x8c);  
    SPI_Write_Reg(0x27,0x0F);
    SPI_Write_Buffer(0x4, data_buf, 2);
    SPI_Write_Reg(0x50, 0x51);
    SPI_Write_Reg(0x21,0x00);
    SPI_Write_Reg(0x25,0x50);  //freq = 2400+80 M
    SPI_Write_Reg(0x24,0x00);
    SPI_Write_Reg(0x20,0x0e);
    SPI_Write_Reg(0x26,0x96); 
    SPI_Write_Reg(0xe3,0xff);
    SPI_Write_Reg(0x50, 0x53);
    SPI_Write_Reg(0x36, 0x8e); 
    SPI_Write_Reg(0x50, 0x51);
    SPI_Write_Reg(0x26, 0x06);
    SPI_Write_Reg(0x26, 0x96); 
    SPI_Write_Reg(0x50, 0x53);
}

void FT_TxFtest_Prom(int BLE_Test_Case,int BLE_Test_Ruslt)
{
    int data_buf[2] = {0xc0, 0x00};
    while(1) {
        switch(BLE_Test_Case) {
            case 8:
                data_buf[1] = 8;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
            case 9:
                data_buf[1] = 9;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
            case 10:
                data_buf[1] = 10;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
            case 11:
                data_buf[1] = 11;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
            case 12:
                data_buf[1] = 12;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
            case 13:
                data_buf[1] = 13;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
            case 14:
                data_buf[1] = 14;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
            case 15:
                data_buf[1] = 15;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
            case 16:
                data_buf[1] = 16;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
            case 17:
                data_buf[1] = 17;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
            case 18:
                data_buf[1] = 18;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
            case 19:
                data_buf[1] = 19;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
            case 20:
                data_buf[1] = 20;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
            case 21:
                data_buf[1] = 21;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
            case 22:
                data_buf[1] = 22;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
            case 23:
                data_buf[1] = 23;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
            case 24:
                data_buf[1] = 24;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
            case 25:
                data_buf[1] = 25;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
            case 26:
                data_buf[1] = 26;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
            case 27:
                data_buf[1] = 27;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
            case 28:
                data_buf[1] = 28;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
            case 29:
                data_buf[1] = 29;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
            case 30:
                data_buf[1] = 30;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
            case 31:
                data_buf[1] = 31;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
            case 32:
                data_buf[1] = 32;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
            default:
                data_buf[1] = 8;
                SPI_Write_Buffer(0x4, data_buf, 2);
                break;
        }
    }
}
