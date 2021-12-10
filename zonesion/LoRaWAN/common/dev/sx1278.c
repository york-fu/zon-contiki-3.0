/*********************************************************************************************
* 文件：sx1278.c
* 作者：xuzhy
* 说明：
* 修改：
* 注释：
*
*********************************************************************************************/
#include <string.h>
#include "sx1278-io.h"
#include "sx1278.h"


#define SX1276ReadBuffer sx1278RegRead
#define SX1276WriteBuffer sx1278RegWrite

static char recv_buf[256];
static int recv_len = 0;
static int lora_rssi;

char *sx1278RecvBuf()
{
  return recv_buf;
}
int sx1278RecvLength()
{
  return recv_len;
}
int sx1278Rssi()
{
  return lora_rssi;
}

static void (*evt_callback)(SX1278_EVT_T evt);

static int hop = 0;                     //跳频开关
static int hop_idx = 0;
static int hop_tab[HOP_TAB_SIZE] = {    //跳频表
	432,434,436,438,440,442,444,446,448,450
};
static int __fp = 433;                  //基频

/*
static const unsigned char BaseFreq[][3]=		//载波频率
{		{0x69,0x00,0x00},//420MHz
		{0x69,0x40,0x00},//421MHz
		{0x69,0x80,0x00},//422MHz
		{0x69,0xc0,0x00},//423MHz
		{0x6a,0x00,0x00},//424MHz
		{0x6a,0x40,0x00},//425MHz
		{0x6a,0x80,0x00},//426MHz
		{0x6a,0xc0,0x00},//427MHz
		{0x6b,0x00,0x00},//428MHz
		{0x6b,0x40,0x00},//429MHz
		{0x6b,0x80,0x00},//430MHz
		{0x6b,0xc0,0x00},//431MHz
		{0x6c,0x00,0x00},//432MHz
		{0x6c,0x40,0x00},//433MHz
		{0x6c,0x80,0x00},//434MHz
		{0x6c,0xc0,0x00},//435MHz
		{0x6d,0x00,0x00},//436MHz
		{0x6d,0x40,0x00},//437MHz
		{0x6d,0x80,0x00},//438MHz
		{0x6d,0xc0,0x00},//439MHz
		{0x6e,0x00,0x00},//440MHz
		{0x6e,0x40,0x00},//441MHz
		{0x6e,0x80,0x00},//442MHz
		{0x6e,0xc0,0x00},//443MHz
		{0x6f,0x00,0x00},//444MHz
		{0x6f,0x40,0x00},//445MHz
		{0x6f,0x80,0x00},//446MHz
		{0x6f,0xc0,0x00},//447MHz
		{0x70,0x00,0x00},//448MHz
		{0x70,0x40,0x00},//449MHz
		{0x70,0x80,0x00},//450MHz
		{0x70,0xc0,0x00},//451MHz
		{0x71,0x00,0x00},//452MHz
		{0x71,0x40,0x00},//453MHz
		{0x71,0x80,0x00},//454MHz
		{0x71,0xc0,0x00},//455MHz
		{0x72,0x00,0x00},//456MHz
		{0x72,0x40,0x00},//457MHz
		{0x72,0x80,0x00},//458MHz
		{0x72,0xc0,0x00},//459MHz
		{0x73,0x00,0x00},//460MHz
		{0x73,0x40,0x00},//461MHz
		{0x73,0x80,0x00},//462MHz
		{0x73,0xc0,0x00},//463MHz
		{0x74,0x00,0x00},//464MHz
		{0x74,0x40,0x00},//465MHz
		{0x74,0x80,0x00},//466MHz
		{0x74,0xc0,0x00},//467MHz
		{0x75,0x00,0x00},//468MHz
		{0x75,0x40,0x00},//469MHz
		{0x75,0x80,0x00},//470MHz
};
*/
/*********************************************************************************************************
** Function			: 
** Description	:
** Input Param	:
** Output Param	:
** Global Var		:
** Caller				: 
**
** Author				: 
** Date					: 
**-------------------------------------------------------------------------------------------------------
** Change				:
** Date					:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
static void  SX1276LoRaSetOpMode(RFMode_SET opMode) {
	unsigned char opModePrev;
	opModePrev = SX1276ReadBuffer(REG_LR_OPMODE);
	opModePrev &= 0xf8;
	opModePrev |= (unsigned char) opMode;
	SX1276WriteBuffer( REG_LR_OPMODE, opModePrev);
}

/*********************************************************************************************************
** Function	: 
** Description	:
** Input Param	:
** Output Param	:
** Global Var	:
** Caller	: 
**
** Author	: 
** Date		: 
**-------------------------------------------------------------------------------------------------------
** Change	:
** Date		:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/

static void  SX1276LoRaFsk(Debugging_fsk_ook opMode) {
	unsigned char opModePrev;
	opModePrev = SX1276ReadBuffer(REG_LR_OPMODE);
	opModePrev &= 0x7F;
	opModePrev |= (unsigned char) opMode;
	SX1276WriteBuffer( REG_LR_OPMODE, opModePrev);
}

/*********************************************************************************************************
** Function			: 
** Description	:
** Input Param	:
** Output Param	:
** Global Var		:
** Caller				: 
**
** Author				: 
** Date					: 
**-------------------------------------------------------------------------------------------------------
** Change				:
** Date					:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
static void  SX1276LoRaSetRFFrequency(unsigned int bf) {
    unsigned int val = (unsigned int)(1000000*bf / 61.035162f); //(434000000.0f / 0x6c8000)//7110656
	SX1276WriteBuffer( REG_LR_FRFMSB, (val >> 16)&0xff /*BaseFreq[bf-420][0]*/);//0x04???????????
	SX1276WriteBuffer( REG_LR_FRFMID, (val >> 8)&0xff /*BaseFreq[bf-420][1]*/);//0x07???????????
	SX1276WriteBuffer( REG_LR_FRFLSB, (val >> 0)&0xff /*BaseFreq[bf-420][2]*/);//0x00???????????
}
/*********************************************************************************************************
** Function			: 
** Description	:
** Input Param	:
** Output Param	:
** Global Var		:
** Caller				: 
**
** Author				: 
** Date					: 
**-------------------------------------------------------------------------------------------------------
** Change				:
** Date					:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
void  SX1276LoRaSetRFPower(unsigned char power) {
	/*if (power <=15) {
		SX1276WriteBuffer( REG_LR_PADAC, 0x84);
		SX1276WriteBuffer( REG_LR_PACONFIG, 0x70|(power & 0x0F));
	} else */if (power <= 17){
		power -= 2;
		SX1276WriteBuffer( REG_LR_PADAC, 0x84);
		SX1276WriteBuffer( REG_LR_PACONFIG, 0xf0|(power & 0x0F));
	} else { 
		power -= 5;
		SX1276WriteBuffer( REG_LR_PADAC, 0x87);
		SX1276WriteBuffer( REG_LR_PACONFIG, 0xf0|(power & 0x0F));
	}
}

/*********************************************************************************************************
** Function			: 
** Description	:
** Input Param	:
** Output Param	:
** Global Var		:
** Caller				: 
**
** Author				: 
** Date					: 
**-------------------------------------------------------------------------------------------------------
** Change				:
** Date					:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
static void  SX1276LoRaSetNbTrigPeaks(unsigned char value) {
	unsigned char RECVER_DAT;
	RECVER_DAT = SX1276ReadBuffer(0x31);
	RECVER_DAT = (RECVER_DAT & 0xF8) | value;
	SX1276WriteBuffer(0x31, RECVER_DAT);
}
/*********************************************************************************************************
** Function			: 
** Description	:
** Input Param	:
** Output Param	:
** Global Var		:
** Caller				: 
**
** Author				: 
** Date					: 
**-------------------------------------------------------------------------------------------------------
** Change				:
** Date					:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
void  SX1276LoRaSetSpreadingFactor(unsigned char factor) {
	unsigned char RECVER_DAT;
	SX1276LoRaSetNbTrigPeaks(3);
	RECVER_DAT = SX1276ReadBuffer( REG_LR_MODEMCONFIG2);
	RECVER_DAT = (RECVER_DAT & RFLR_MODEMCONFIG2_SF_MASK) | (factor << 4);
	SX1276WriteBuffer( REG_LR_MODEMCONFIG2, RECVER_DAT);
}
/*********************************************************************************************************
** Function			: 
** Description	:
** Input Param	:
** Output Param	:
** Global Var		:
** Caller				: 
**
** Author				: 
** Date					: 
**-------------------------------------------------------------------------------------------------------
** Change				:
** Date					:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
void  SX1276LoRaSetErrorCoding(unsigned char value) {
	unsigned char RECVER_DAT;
	RECVER_DAT = SX1276ReadBuffer( REG_LR_MODEMCONFIG1);
	RECVER_DAT = (RECVER_DAT & RFLR_MODEMCONFIG1_CODINGRATE_MASK)
			| (value << 1);
	SX1276WriteBuffer( REG_LR_MODEMCONFIG1, RECVER_DAT);
}

/*********************************************************************************************************
** Function			: 
** Description	:
** Input Param	:
** Output Param	:
** Global Var		:
** Caller				: 
**
** Author				: 
** Date					: 
**-------------------------------------------------------------------------------------------------------
** Change				:
** Date					:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
void  SX1276LoRaSetPreambleLength(unsigned short value) {
	SX1276WriteBuffer( REG_LR_PREAMBLEMSB, value>>8);
	SX1276WriteBuffer( REG_LR_PREAMBLELSB, value);
}
/*********************************************************************************************************
** Function			: 
** Description	:
** Input Param	:
** Output Param	:
** Global Var		:
** Caller				: 
**
** Author				: 
** Date					: 
**-------------------------------------------------------------------------------------------------------
** Change				:
** Date					:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
void  SX1276LoRaSetPacketCrcOn(int enable) {
	unsigned char RECVER_DAT;
	RECVER_DAT = SX1276ReadBuffer( REG_LR_MODEMCONFIG2);
	RECVER_DAT = (RECVER_DAT & RFLR_MODEMCONFIG2_RXPAYLOADCRC_MASK)	| (enable << 2);
	SX1276WriteBuffer( REG_LR_MODEMCONFIG2, RECVER_DAT);
}
/*********************************************************************************************************
** Function			: 
** Description	:
** Input Param	:
** Output Param	:
** Global Var		:
** Caller				: 
**
** Author				: 
** Date					: 
**-------------------------------------------------------------------------------------------------------
** Change				:
** Date					:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
void  SX1276LoRaSetSignalBandwidth(unsigned char bw) {
	unsigned char RECVER_DAT;
	RECVER_DAT = SX1276ReadBuffer( REG_LR_MODEMCONFIG1);
	RECVER_DAT = (RECVER_DAT & RFLR_MODEMCONFIG1_BW_MASK) | (bw << 4);
	SX1276WriteBuffer( REG_LR_MODEMCONFIG1, RECVER_DAT);
// LoRaSettings.SignalBw = bw;
}

/*********************************************************************************************************
** Function			: 
** Description	:
** Input Param	:
** Output Param	:
** Global Var		:
** Caller				: 
**
** Author				: 
** Date					: 
**-------------------------------------------------------------------------------------------------------
** Change				:
** Date					:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
static void  SX1276LoRaSetImplicitHeaderOn(int enable) {
	unsigned char RECVER_DAT;
	RECVER_DAT = SX1276ReadBuffer( REG_LR_MODEMCONFIG1);
	RECVER_DAT = (RECVER_DAT & RFLR_MODEMCONFIG1_IMPLICITHEADER_MASK)
			| (enable);
	SX1276WriteBuffer( REG_LR_MODEMCONFIG1, RECVER_DAT);
}


/*********************************************************************************************************
** Function			: 
** Description	:
** Input Param	:
** Output Param	:
** Global Var		:
** Caller				: 
**
** Author				: 
** Date					: 
**-------------------------------------------------------------------------------------------------------
** Change				:
** Date					:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
static void  SX1276LoRaSetSymbTimeout(unsigned int value) {
	unsigned char RECVER_DAT[2];
	RECVER_DAT[0] = SX1276ReadBuffer( REG_LR_MODEMCONFIG2);
	RECVER_DAT[1] = SX1276ReadBuffer( REG_LR_SYMBTIMEOUTLSB);
	RECVER_DAT[0] = (RECVER_DAT[0] & RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK)
			| ((value >> 8) & ~RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK);
	RECVER_DAT[1] = value & 0xFF;
	SX1276WriteBuffer( REG_LR_MODEMCONFIG2, RECVER_DAT[0]);
	SX1276WriteBuffer( REG_LR_SYMBTIMEOUTLSB, RECVER_DAT[1]);
}
/*********************************************************************************************************
** Function			: 
** Description	:
** Input Param	:
** Output Param	:
** Global Var		:
** Caller				: 
**
** Author				: 
** Date					: 
**-------------------------------------------------------------------------------------------------------
** Change				:
** Date					:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
static void  SX1276LoRaSetMobileNode(int enable) {
	unsigned char RECVER_DAT;
	RECVER_DAT = SX1276ReadBuffer( REG_LR_MODEMCONFIG3);
	RECVER_DAT = (RECVER_DAT & RFLR_MODEMCONFIG3_MOBILE_NODE_MASK)
			| (enable << 3);
	SX1276WriteBuffer( REG_LR_MODEMCONFIG3, RECVER_DAT);
}

/*********************************************************************************************************
** Function			: 
** Description	:
** Input Param	:
** Output Param	:
** Global Var		:
** Caller				: 
**
** Author				: 
** Date					: 
**-------------------------------------------------------------------------------------------------------
** Change				:
** Date					:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
void  sx1278SendPacket(unsigned char *pkt,unsigned char len) {
	
        char bufreg = 0x80;
        
	//SX1276WriteBuffer( REG_LR_IRQFLAGS, 0xff);//清除所有中断
	
	SX1276LoRaSetOpMode(Stdby_mode);

	
	//SX1276WriteBuffer(REG_LR_IRQFLAGSMASK, IRQN_TXD_Value);	//打开发送中断
	SX1276WriteBuffer( REG_LR_PAYLOADLENGTH, len);	 //最大数据包
	SX1276WriteBuffer( REG_LR_FIFOTXBASEADDR, 0);
	SX1276WriteBuffer( REG_LR_FIFOADDRPTR, 0);
	
        sx1278_cs(0);
	sx1278BufWrite(&bufreg, 1);
	sx1278BufWrite((char*)pkt, len);
        sx1278_cs(1);
	
	SX1276WriteBuffer(REG_LR_DIOMAPPING1, 0x50);
	SX1276WriteBuffer(REG_LR_DIOMAPPING2, 0x00);
	SX1276LoRaSetOpMode(Transmitter_mode);
}
/*********************************************************************************************************
** Function			: 
** Description	:
** Input Param	:
** Output Param	:
** Global Var		:
** Caller				: 
**
** Author				: 
** Date					: 
**-------------------------------------------------------------------------------------------------------
** Change				:
** Date					:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
void sx1278HopSet(int f)
{
  hop = f;
  if (f == 0) {
	SX1276WriteBuffer( REG_LR_HOPPERIOD, 0);	//不跳频
  } else {
	SX1276WriteBuffer( REG_LR_HOPPERIOD, 30);
	hop_idx = 0;
  }
}
void sx1278HopTabSet(int *tab)
{
  memcpy(&hop_tab, tab, sizeof hop_tab);
}
void sx1278SetFP(int f)
{
  __fp = f;
  SX1276LoRaSetRFFrequency(f);
}

void SX1276CheckCAD(void)
{
  SX1276LoRaSetOpMode(Stdby_mode);
  SX1276WriteBuffer(REG_LR_IRQFLAGS, 0xff); //清irq
  SX1276WriteBuffer( REG_LR_DIOMAPPING1, 0X02<<6); //检测cad
  SX1276WriteBuffer( REG_LR_DIOMAPPING2, 0X00);
  
  SX1276LoRaSetOpMode(CAD_mode);
  
  //SX1276ReadBuffer(REG_LR_IRQFLAGS); //???
}

void sx1278Init(void)
{
  
  	SX1276LoRaSetOpMode(Sleep_mode);  			//设置睡眠模式0x01
	SX1276LoRaFsk(LORA_mode);	      			//设置扩频模式，只能在睡眠模式下修改
	SX1276LoRaSetOpMode(Stdby_mode);   		        //设置为待机模式
	
        //SX1276WriteBuffer( REG_LR_DIOMAPPING1, GPIO_VARE_1);	//DIO0映射
	//SX1276WriteBuffer( REG_LR_DIOMAPPING1, GPIO_VARE_1);	//DIO2映射
	//SX1276WriteBuffer( REG_LR_DIOMAPPING2, GPIO_VARE_2);	//DIO4映射
        
	SX1276LoRaSetRFFrequency(__fp);		//基频频率
	SX1276LoRaSetRFPower(15);		//发射功率
	SX1276LoRaSetSpreadingFactor(12);	//扩频因子设置  6:64,7:128,8:256,9:512,10:1024,11:2048,12:4096
	SX1276LoRaSetErrorCoding(1);		//有效数据比    1:4/5,2:4/6,3:4/7,4:4/8
	SX1276LoRaSetPreambleLength(15);		//前导长度 +4.25 Symbles
	SX1276LoRaSetSignalBandwidth(9);	//设置扩频带宽
        
	SX1276LoRaSetImplicitHeaderOn(0);			//同步头显性模式
	//SX1276LoRaSetPayloadLength(0xff);			//数据区长度
        //SX1276WriteBuffer( REG_LR_PAYLOADLENGTH, 255);	
        
	SX1276LoRaSetSymbTimeout(50);				//超时阀值
	SX1276LoRaSetMobileNode(1); 	                        //低数据优化   DE = 1
        SX1276LoRaSetPacketCrcOn(1);			        //CRC 校验打开
        
        SX1276WriteBuffer( REG_LR_HOPPERIOD, 0);	//不跳频
        
	SX1276WriteBuffer(REG_LR_IRQFLAGSMASK, 0); 
}
void sx1278SetEvtCall(void (*callback)(SX1278_EVT_T evt))
{
  evt_callback = callback;
}

void sx1278Irq(void)
{
  int RF_EX0_STATUS = SX1276ReadBuffer( REG_LR_IRQFLAGS);
  

  SX1276WriteBuffer( REG_LR_IRQFLAGS, 0xff);//clear irq
   
  if (hop && (RF_EX0_STATUS & 0x02)) { //跳频模式下频率切换
    SX1276LoRaSetRFFrequency(hop_tab[hop_idx++]);
   
    if (hop_idx == HOP_TAB_SIZE) {
      hop_idx = 0;
    }
  } 
  if ((RF_EX0_STATUS & 0x04)) { //cad
    /*处理CAD信息*/
    hop_idx = 0;
    if (RF_EX0_STATUS & 0x01) {
      SX1276WriteBuffer( REG_LR_DIOMAPPING1, 0x00); //dio0 rx done dio1 rx timeout
      SX1276LoRaSetOpMode(receive_single);
      if (evt_callback != NULL) {
        evt_callback(CADDone);
      }
    } else {
      
      SX1276LoRaSetOpMode(Stdby_mode);
      if (evt_callback != NULL) {
        evt_callback(CADTimeout);
      }
    }
  }
  
  if ((RF_EX0_STATUS & 0x80)) { // rx timeout
    //lora_debug("Lora rx timeout!\r\n");
    SX1276LoRaSetOpMode(Stdby_mode);
    if (hop!=0) {
      SX1276LoRaSetRFFrequency(__fp); 
    }
    if (evt_callback != NULL) {
        evt_callback(RxTimeout);
    }
  }
  
  if (RF_EX0_STATUS  & 0x40) { //rx done
    int rlen;
    char r = 0x00;
    SX1276WriteBuffer(REG_LR_FIFOADDRPTR, 0x00);
    rlen = SX1276ReadBuffer(REG_LR_NBRXBYTES);
    
    sx1278_cs(0);
    sx1278BufWrite(&r, 1);
    sx1278BufRead(recv_buf, rlen);
    sx1278_cs(1);
    		
    if (RF_EX0_STATUS & 0x20){
      //lora_debug("rx crc error\r\n");
      rlen = 0;
    } 
					
    lora_rssi = -137 + SX1276ReadBuffer(REG_LR_PKTRSSIVALUE);
					
    if (rlen > 0) {	
      
    }
    recv_len = rlen;
    
    SX1276LoRaSetOpMode(Stdby_mode);
    if (hop != 0) {
      SX1276LoRaSetRFFrequency(__fp); 
    }
    if (evt_callback != NULL) {
        evt_callback(RxDone);
    }
    recv_len = 0;
  }
  
 
  if ((RF_EX0_STATUS &0x08)) { //tx done

    SX1276LoRaSetOpMode(Stdby_mode);
    if (hop != 0) {
      SX1276LoRaSetRFFrequency(__fp); 
    }

    if (evt_callback != NULL) {
        evt_callback(TxDone);
    }
  }
  /*
  static int ls = -1;
  if (ls != RF_EX0_STATUS) {
    printf("irq %02X\r\n", RF_EX0_STATUS);
    ls = RF_EX0_STATUS;
  }*/
}
