/**
  ******************************************************************************
  * @file    :MG127.c
  * @author  :MG Team
  * @version :V1.0
  * @date
  * @brief
  ******************************************************************************
***/

/* Includes ------------------------------------------------------------------*/
//#include "Includes.h"
#include "HC89F0650.h"
#include "SPI.h"
#include "MG127.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;


/* Private macro -------------------------------------------------------------*/
#define LEN_BLE_ADDR 6

/* Private variables ---------------------------------------------------------*/
unsigned short tick;
u8 txcnt = 0;
u8 rxcnt = 0;
unsigned char rx_buf[39]; //include header(2B)+mac(6B)+data(max31B), for rx application
unsigned char iot_buf[39];

//BLE ADV_data, maxlen=31
#define LEN_DATA 30
u8 adv_data[LEN_DATA] = {0x02,0x01,0x04,
			0x1a,0xff,0x4c,0x00,0x02,0x15,
			0xfd,0xa5,0x06,0x93,0xa4,0xe2,
			0x4f,0xb1,0xaf,0xcf,0xc6,0xeb,
			0x07,0x64,0x78,0x25, 0x27,0x32,
			0x52,0xa8, 0xCA};

#if 0
#define LEN_DATA 30
code u8 adv_data[LEN_DATA] = {
				0x02,0x01,0x06,
				0x03,0x02,0xf0,0xff,
				0x16,0xff,0x48,0x43,0x2d,0x30,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
				//0x05,0x12,0x06,0x00,0x0c,0x00,
				//0x02,0x0a,0x04,
				//0x06,0x09,0x48,0x43,0x2d,0x30,0x38
				};
#endif

void BLE_Set_rxtxcnt(u8 tx_cnt, u8 rx_cnt)
{
	txcnt = tx_cnt;		//txcnt=0 is for rx only application
	rxcnt = rx_cnt;		//rxcnt=0 is for tx only application
}

/* Private function prototypes -----------------------------------------------*/
void BLE_Do_Cal(void);


/*******************************************************************************
* Function   :     	BLE_Mode_Sleep
* Parameter  :     	void
* Returns    :     	void
* Description:
* Note:      : 		BLE enter sleep mode. current: 3ua
*******************************************************************************/
void BLE_Mode_Sleep(void)
{
    u8	temp0[4] = {0x02, 0xff, 0xff, 0xff};

    //temp0[0] = 0x02;
    //temp0[1] = 0xff;
    //temp0[2] = 0xff;
    //temp0[3] = 0xff;

    SPI_Write_Buffer(SLEEP_WAKEUP,temp0,4);
}

/*******************************************************************************
* Function   :     	BLE_Mode_Wakeup
* Parameter  :     	void
* Returns    :     	void
* Description:
* Note:      : 		BLE reg:0x00--0x1f. write operation must or 0x20
*******************************************************************************/
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
    SPI_Write_Reg(0x3d, 0x1e);

    BLE_Do_Cal();
    SPI_Write_Reg(0x50, 0x56);
    BLE_Mode_Sleep();
}


void BLE_Mode_PwrDn(void)
{
    unsigned char temp[2] = {0x81, 0x02};

    SPI_Write_Reg(0X50, 0x51);
    SPI_Write_Reg(0X20, 0x78); //pwr down

    SPI_Write_Reg(0X50, 0x53);
    SPI_Write_Reg(0x3d, 0x18);
    SPI_Write_Reg(0X35, 0x01);  //tm

    //temp[0] = 0x81;
    //temp[1] = 0x02;
    SPI_Write_Buffer(0x13, temp, 2);
    SPI_Write_Reg(0X3e, 0xa0);

    SPI_Write_Reg(0X50, 0x56);
}


/*******************************************************************************
* Function   :     	BLE_Set_StartTime
* Parameter  :     	u32
* Returns    :     	void
* Description:
* Note:      :
*******************************************************************************/
void BLE_Set_StartTime(u32 htime)
{
    u8 temp0[3];

    temp0[0] = htime & 0xFF;
    temp0[1] = (htime>>8) & 0xFF;
    temp0[2] = (htime>>16) & 0xFF;

    SPI_Write_Buffer(START_TIME,temp0,3);
}


/*******************************************************************************
* Function   :     	BLE_Set_TimeOut
* Parameter  :     	u32 data_us
* Returns    :     	void
* Description:      TX/RX timeout .unit:us
* Note:      :
*******************************************************************************/
void BLE_Set_TimeOut(u32 data_us)
{
    u8 temp0[3];

    temp0[0] = data_us & 0xff;
    temp0[1] = (data_us >> 8) & 0xff;
    temp0[2] = (data_us >> 16) & 0xff;

    SPI_Write_Buffer(TIMEOUT, temp0, 3);
}

#if 0
/*******************************************************************************
* Function   :     	BLE_Set_Xtal
* Parameter  :     	u8
* Returns    :     	void
* Description:
* Note:      :
*******************************************************************************/
void BLE_Set_Xtal(u8 on_flag)
{
    SPI_Write_Reg(0x50, 0x53);
    SPI_Write_Reg(0x3D, 0x18|(on_flag<<2));
    SPI_Write_Reg(0x50, 0x56);
}
#endif

static void BLE_Get_Pdu(u8 *ptr, u8 *len)
{
    u8 hdr_type;
    u8 len_tmp;
    u8 bank_buf[6];

    SPI_Read_Buffer(ADV_HDR_RX, bank_buf, 2);

    *len = bank_buf[1] + 2;
    ptr[0] = bank_buf[0];
    ptr[1] = bank_buf[1];

    hdr_type = bank_buf[0] & 0xF;
    len_tmp = bank_buf[1];
    switch(hdr_type){
        case ADV_IND:  //advA+0~31
        case ADV_NONCONN_IND:
        case ADV_SCAN_IND:
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


#define TXGAIN_DEF 0x12

void BLE_Do_Cal()  //calibration
{
    u8 data_buf[2];

    SPI_Write_Reg(0x3F, 0x03);
    do{
        data_buf[0] = SPI_Read_Reg(0x1F);
    }while(data_buf[0]&0x03);

    SPI_Write_Reg(0x3F, 0x03);
    do{
        data_buf[0] = SPI_Read_Reg(0x1F);
    }while(data_buf[0]&0x03);

    //////////////////////////////////////////////////
    SPI_Write_Reg(0x35,0x01); //testm for tx/temp
    SPI_Write_Reg(0x32,0xA0);
    SPI_Write_Reg(0x2a,0x04);
    SPI_Write_Reg(0x2a,0x00);

    SPI_Write_Reg(0x32,0x88);
    data_buf[0] = 0x01;
    data_buf[1] = 0x21;
    SPI_Write_Buffer(0x13, data_buf, 2);
    data_buf[0] = 0x01;
    data_buf[1] = 0x20;
    SPI_Write_Buffer(0x13, data_buf, 2);
    SPI_Write_Reg(0x35,0x00);  //exist testm
    ////////////////////////////////////////////////////

}

/*******************************************************************************
* Function   :     	BLE_Init
* Parameter  :     	void
* Returns    :     	void
* Description:      power on .BLE must initnal reg .
* Note:      : 		delay 30ms .
*******************************************************************************/
void BLE_Init(void)
{
    u8 status;
    u8 data_buf[4];
    u8 ble_Addr[6];


    SPI_Write_Reg(0x50, 0x51);
    SPI_Write_Reg(0x50, 0x53);
    SPI_Write_Reg(0x35, 0x00);
	  SPI_Write_Reg(0x36, 0x8c);//Fabe
    SPI_Write_Reg(0x3D, 0x18);
    SPI_Write_Reg(0x50, 0x51);

    do{
        SPI_Write_Reg(0x50, 0x53);

        data_buf[0] = 0;
        data_buf[1] = 0;
        data_buf[2] = 1;
        SPI_Write_Buffer(0x00, data_buf, 3);

        SPI_Write_Reg(0x36, 0x8e);
        SPI_Write_Reg(0x37, 0x88);
        SPI_Write_Reg(0x38, 0x88);
        SPI_Write_Reg(0x39, 0x8e);

        SPI_Write_Reg(0x50, 0x51);

        SPI_Read_Reg(0x1e);

        status = SPI_Read_Reg(CHIP_OK);
    }while(status != 0x80);

#if 1 //debug
    //read chip version
	//status = SPI_Read_Reg(0x1e);
    //Uart_Send_String("chip version=");
//    Uart_Send_Byte(status);
    //Uart_Send_String("\r\n");
#endif

    SPI_Write_Reg(0X20, 0x78);//power down,tx, for hot reset
    SPI_Write_Reg(0X26, 0x06);//1Mbps
    SPI_Write_Reg(0X20, 0x7a);//power up

    SPI_Write_Reg(0x50, 0x56);

    BLE_Mode_Sleep();

#if 1 //debug
    //read BLE address. BLE MAC Address
    SPI_Read_Buffer(0x08, ble_Addr, 6);

    //Uart_Send_String("BleAddr=");
    //Uart_Send_Byte(ble_Addr[5]);
    //Uart_Send_Byte(ble_Addr[4]);
    //Uart_Send_Byte(ble_Addr[3]);
    //Uart_Send_Byte(ble_Addr[2]);
    //Uart_Send_Byte(ble_Addr[1]);
    //Uart_Send_Byte(ble_Addr[0]);
    //Uart_Send_String("\r\n");
#endif


    SPI_Write_Reg(0x50, 0x53);

    data_buf[0] = 0x7f;
    data_buf[1] = 0x80; //xocc
    SPI_Write_Buffer(0x14,data_buf,2);

    //set BLE TX Power
    data_buf[0] = 0x02;
    data_buf[1] = BLE_TX_POWER;
    SPI_Write_Buffer(0x0f,data_buf,2);

    data_buf[1] = SPI_Read_Reg(0x08);  //txgain
    if(0 == data_buf[1]){
      data_buf[1] = TXGAIN_DFF;
    }
    data_buf[0] = 0xc0;
    data_buf[2] = 0x2D; //rx
    SPI_Write_Buffer(0x4, data_buf, 3);


    data_buf[0] = 0x80; //rx
    data_buf[1] = 0x00;
    SPI_Write_Buffer(0x0C, data_buf, 2);

    data_buf[0] = 0x81;
    data_buf[1] = 0x22;
    SPI_Write_Buffer(0x13, data_buf, 2);

    SPI_Write_Reg(0X21, 0x02);
    SPI_Write_Reg(0x3C, 0x30);
    SPI_Write_Reg(0x3E, 0x30);

    data_buf[0] = 0x38;
    data_buf[1] = 0x0F;
    data_buf[2] = 0x00; //gc
    SPI_Write_Buffer(0x02, data_buf, 3);

    data_buf[0] = 0x80;
    data_buf[1] = 0x70; //gain
    data_buf[2] = 0x21;
    data_buf[3] = 0x40; //rx
    SPI_Write_Buffer(0x0b, data_buf, 4);

    SPI_Write_Reg(0x29, 0x71); //gain

    data_buf[0] = 0x10;
    data_buf[1] = 0x02;
    SPI_Write_Buffer(0xA, data_buf, 2);

    data_buf[0] = 0x02;
    data_buf[1] = 0x12;
    SPI_Write_Buffer(0xD, data_buf, 2);

    data_buf[0] = 0x01;
    data_buf[1] = 0x07;
    SPI_Write_Buffer(0xE, data_buf, 2);

    SPI_Write_Reg(0x50, 0x56);
}

//2480MHz carrier, for xtal capacitor tuning
//call this function after BLE_Init() in main()
void Carrier(void)
{
    unsigned long delay=0x14000;
    u8 data_buf[2] = {0xc0, 0x00};

    SPI_Write_Reg(0x50, 0x51);
    SPI_Write_Reg(0x50, 0x53);

    SPI_Write_Reg(0x3d, 0x1e);
    while(delay--);

    SPI_Write_Reg(0x50, 0x56);
    SPI_Write_Reg(0x20,0x0);

    SPI_Write_Reg(0x50, 0x53);
    SPI_Write_Reg(0x36, 0x8c);  //
    SPI_Write_Reg(0x27,0x0F);
    SPI_Write_Buffer(0x4, data_buf, 2);
    data_buf[0] = 0x24;
    data_buf[1] = 0x2e;
    SPI_Write_Buffer(0x11, data_buf, 2);
    SPI_Write_Reg(0x50, 0x51);

    SPI_Write_Reg(0x21,0x00);
    SPI_Write_Reg(0x25,0x50);
    SPI_Write_Reg(0x24,0x00);

    SPI_Write_Reg(0x20,0x0e);
    SPI_Write_Reg(0x26,0x96); //for cont wave
    SPI_Write_Reg(0xe3,0xff);

    SPI_Write_Reg(0x50, 0x53);
    SPI_Write_Reg(0x36, 0x8e); //

    SPI_Write_Reg(0x50, 0x51);
    SPI_Write_Reg(0x26, 0x06);
    SPI_Write_Reg(0x26, 0x96);

    SPI_Write_Reg(0x50, 0x53);

    while(1){};
}

//16MHz output by IRQ pin, for xtal capacitor tuning
void XOClockOutput(void)
{
    u8 data_buf[3];

    SPI_Write_Reg(0x50, 0x51);
    SPI_Write_Reg(0x50, 0x53);

    SPI_Write_Reg(0x35,0x01);
    data_buf[0] = 0x93;
    data_buf[1] = 0x00;
    SPI_Write_Buffer(0x14, data_buf, 2);
    SPI_Write_Reg(0x3e,0x30);
    SPI_Write_Reg(0x31,0x00);
    SPI_Write_Reg(0x3b,0x48);
    data_buf[0] = 0x00;
    data_buf[1] = 0x00;
    data_buf[2] = 0x15;
    SPI_Write_Buffer(0x12, data_buf, 3);

    while(1){};
}

/*******************************************************************************
* Function   :     	BLE_TRX
* Parameter  :     	txcnt, rxcnt
* Returns    :     	void
* Description:      Beacon data .process .
* Note:      :
*******************************************************************************/
sbit LED1 = P1^7;
sbit LED2 = P1^6;

void BLE_TRX(unsigned short value)
{
    u8 status = 0;
    u8 ch = 37;
    u8 data_buf[2];
    u8 tmp_cnt = txcnt+rxcnt;
    u8 len_pdu = 0;
    u8 loop = 0;

    if(tmp_cnt == 0) return;

    BLE_Mode_PwrUp();

#if 1  //if adv_data no change, can move this block to the end of BLE_Init()
    //set BLE TX default channel:37.38.39
    SPI_Write_Reg(CH_NO|0X20, ch);

    //BLT FIFO write adv_data . max len:31 byte
	adv_data[5] = (u8)((value >> 8) & 0xFF);
	adv_data[6] = (u8)(value & 0xFF);
    SPI_Write_Buffer(W_TX_PAYLOAD, adv_data, LEN_DATA);

    //PDU TYPE: 2  non-connectable undirected advertising . tx add:random address
    //set BLT PDU length:adv_data+6 mac adress.
    data_buf[0] = 0x02;
    data_buf[1] = LEN_DATA+LEN_BLE_ADDR;
    SPI_Write_Buffer(ADV_HDR_TX, data_buf, 2);

    //clear all interrupt
    data_buf[0] = 0xFF;
    data_buf[1] = 0x80;
    SPI_Write_Buffer(INT_FLAG, data_buf, 2);
#endif

    BLE_Mode_Wakeup();

    BLE_Set_TimeOut(BLE_RX_TIMEOUT);
    tick = BLE_GUARD_TIME;

    while(1)
    {
        //BLE IRQ LOW
        if(mcu_get_irq())		//(IRQ == 0) //(!BLE_IRQ_GET())
        {
            //clear interrupt flag
            status = SPI_Read_Reg(INT_FLAG);
            SPI_Write_Reg(INT_FLAG|0X20, status);
            //Uart_Send_Byte(status); //debug

            if(INT_TYPE_WAKEUP & status)//wakeup
            {
                if(txcnt > 0){
                    txcnt --;
                    SPI_Write_Reg(MODE_TYPE|0X20, RADIO_MODE_ADV_TX);
                    BLE_Set_StartTime(BLE_START_TIME);
                }else if(rxcnt > 0){
                    rxcnt --;
                    SPI_Write_Reg(MODE_TYPE|0X20, RADIO_MODE_ADV_RX);
                    BLE_Set_StartTime(BLE_START_TIME);
                }
                continue; //goto while(1)
            }

            BLE_Mode_Sleep();

            if(INT_TYPE_PDU_OK & status){ //only happen in rx application, no need porting in tx only application
                //LED_RED_ON(); //debug
                BLE_Get_Pdu(rx_buf, &len_pdu);
#if 1 //debug
                //Uart_Send_String("\r\nRX: ");
                if(rx_buf[8] == 0x02 && rx_buf[9] == 0x01 && rx_buf[10] == 0x04){
                    LED1 = 1;
                    for(loop=0; loop<len_pdu; loop++)
                        iot_buf[loop] = rx_buf[loop];
                }else
                    LED1 = 0;
#endif
            }else if(INT_TYPE_TX_START & status){ //only happen in tx application
                //LED_GREEN_ON(); //debug
            }

            if(INT_TYPE_SLEEP & status)//sleep
            {
                //LED_GREEN_OFF(); //debug
                //LED_RED_OFF();  //debug
                tick = BLE_GUARD_TIME;

                //BLE channel
                if (++ch > 39){
                    ch = 37;
                }
                SPI_Write_Reg(CH_NO|0X20, ch);

                tmp_cnt --;
                if(tmp_cnt == 0){
                    BLE_Mode_PwrDn();
                    break; //exit from while(1)
                }
                else
                    BLE_Mode_Wakeup();
            }

        }
        else if(tick == 0){ //robustness, in case no int
            BLE_Mode_Sleep();
        }

    }
}
