#include "contiki.h"
#include "stdint.h"
#include "string.h"
#include "stdio.h"
//#include "utilities.h"
#include "LoRaMac.h"
#include "Commissioning.h"
//#include "utilities.h"
#include "dev/leds.h"
#include "stm32f10x.h"
#include "config.h"

PROCESS(lorawan, "lorawan");

#define ACTIVE_REGION                           LORAMAC_REGION_CN470

//#define LORAWAN_DEFAULT_DATARATE                DR_5
//static uint16_t ChannelMask[6] = {0x0001,0,0,0,0,0};
//static uint8_t sADREnable = 0;

//static uint8_t reqClassType = CLASS_C;
        
//static uint8_t AppEui[] = {0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7};
//lora mac app key
//static uint8_t AppKey[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};//LORAWAN_APPLICATION_KEY; l



#pragma pack(push) //保存对齐状态
#pragma pack(4)
lorawan_cfg_t lorawan_cfg;
#pragma pack(pop) 



#if 0
#define debug(...) do{printf(" %u: ", clock_time());printf(__VA_ARGS__);}while(0) 
#define debug_e(...) do{printf(__VA_ARGS__);}while(0)
#else
#define debug(...)  do{}while(0)
#define debug_e(...) do{}while(0)
#endif

#define DEBUG_TM 0
#if DEBUG_TM
uint32_t sendTm, sendConfTm;
#endif

static int DeviceState = 0;

static LoRaMacPrimitives_t LoRaMacPrimitives;

static uint32_t lorawanReqACKTm = 0;
static uint8_t lorawanACTTimeout = 0;

static process_event_t ev_rx;
static process_event_t ev_tx;

#define NB_RETRIALS       1 //重发次数
#define REQ_ACK_TIME      120
#define REJOIN_TIME       300 //重连


static uint8_t rxPort = 0;
static uint8_t rxLen = 0;
static uint8_t rxBuffer[254];

static uint8_t txPort = 0;
static uint8_t txLen = 0;
static uint8_t txBuffer[254];

static void (*(__onPortCall[224]))(uint8_t *dat, int len);

int32_t rand1( void );

uint8_t *getDeviceEUI(void)
{
  #define UUID            ((unsigned int *)0x1FFFF7E8)
  static uint8_t eui[8] = {0};
  if (eui[0] == 0){
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
    uint32_t c = CRC_CalcBlockCRC(UUID, 3);
    eui[4] = (c>>24)&0xff;
    eui[5] = (c>>16)&0xff;
    eui[6] = (c>>8)&0xff;
    eui[7] = c&0xff;
    
    eui[0] = 'L';  // 'L'
    eui[1] = 'W';  //'W'
    eui[2] = 0x00;  //  00
    eui[3] = 0x00;  //  00
  }
  return eui;
  #undef UUID
}

uint8_t *getRootKey(void)
{
  static uint8_t key[16];

  if (lorawan_cfg.aKey != 0){
    memcpy(key, lorawan_cfg.JoinEUI, 8);
    memcpy(key+8, getDeviceEUI(), 8);
    return key;
  } else {
    return lorawan_cfg.Key;
  }
}


int loraWanRegisterOnPortCall(int port, void (*fun)(uint8_t*, int))
{
  if (port<0 || port > 224) return -1;
  __onPortCall[port-1] = fun;
  return 0;
}

void loraWanSendData(int AppPort, uint8_t *AppData, int AppDataSize)
{
  if (DeviceState == 4 && txLen == 0 && AppDataSize<254) {
    memcpy(txBuffer, AppData, AppDataSize);
    txPort = AppPort;
    txLen = AppDataSize;
    process_post(&lorawan, ev_tx, NULL);
  }
}

static int __loraWanSendData(int AppPort, uint8_t *AppData, int AppDataSize)
{
    McpsReq_t mcpsReq;
    LoRaMacTxInfo_t txInfo;

    if( LoRaMacQueryTxPossible( AppDataSize, &txInfo ) != LORAMAC_STATUS_OK )
    {
        // Send empty frame in order to flush MAC commands
        mcpsReq.Type = MCPS_UNCONFIRMED;
        mcpsReq.Req.Unconfirmed.fBuffer = NULL;
        mcpsReq.Req.Unconfirmed.fBufferSize = 0;
        mcpsReq.Req.Unconfirmed.Datarate = lorawan_cfg.DefaultDataRate;
        AppDataSize = 0;
    }
    else
    {    
         if( lorawanReqACKTm  < REQ_ACK_TIME )
        {
          debug("send UNCONFIRMED msg\r\n");
            mcpsReq.Type = MCPS_UNCONFIRMED;
            mcpsReq.Req.Unconfirmed.fPort = AppPort;
            mcpsReq.Req.Unconfirmed.fBuffer = AppData;
            mcpsReq.Req.Unconfirmed.fBufferSize = AppDataSize;
            mcpsReq.Req.Unconfirmed.Datarate = lorawan_cfg.DefaultDataRate;
        }
        else
        {
           debug("send CONFIRMED msg\r\n");
            mcpsReq.Type = MCPS_CONFIRMED;
            mcpsReq.Req.Confirmed.fPort = AppPort;
            mcpsReq.Req.Confirmed.fBuffer = AppData;
            mcpsReq.Req.Confirmed.fBufferSize = AppDataSize;
            mcpsReq.Req.Confirmed.NbTrials = NB_RETRIALS;
            mcpsReq.Req.Confirmed.Datarate = lorawan_cfg.DefaultDataRate;
        }
    }
    if( LoRaMacMcpsRequest( &mcpsReq ) == LORAMAC_STATUS_OK )
    {
#if DEBUG_TM
      sendTm = clock_time();
#endif
      return AppDataSize;
    } else{
      return -1;
    }
}
/*!
 * \brief   MLME-Indication event function
 *
 * \param   [IN] mlmeIndication - Pointer to the indication structure.
 */
static void MlmeIndication( MlmeIndication_t *mlmeIndication )
{
}
/*!
 * \brief   MLME-Confirm event function
 *
 * \param   [IN] mlmeConfirm - Pointer to the confirm structure,
 *               containing confirm attributes.
 */
static void MlmeConfirm( MlmeConfirm_t *mlmeConfirm )
{
  switch( mlmeConfirm->MlmeRequest )
  {
      case MLME_JOIN:
      {
          if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
          {
            debug("join...ok\r\n");
            DeviceState = 4;
          } else {
            DeviceState = 3;
          }
      }
  }
}
/*!
 * \brief   MCPS-Indication event function
 *
 * \param   [IN] mcpsIndication - Pointer to the indication structure,
 *               containing indication attributes.
 */
static void McpsIndication( McpsIndication_t *mcpsIndication )
{
    if( mcpsIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK )
    {
        return;
    }

    switch( mcpsIndication->McpsIndication )
    {
        case MCPS_UNCONFIRMED:
        {
         debug("recv unconfirmed msg\r\n");
          lorawanReqACKTm = 0;
            break;
        }
        case MCPS_CONFIRMED:
        {
          lorawanReqACKTm = 0;
          debug("recv confirmed msg\r\n");
            break;
        }
        case MCPS_PROPRIETARY:
        {
            break;
        }
        case MCPS_MULTICAST:
        {
            break;
        }
        default:
            break;
    }
    if( mcpsIndication->FramePending == true )
    {
        // The server signals that it has pending data to be sent.
        // We schedule an uplink as soon as possible to flush the server.
       debug(" mac req send dat\r\n");
       
    }
    // Check Buffer
    // Check BufferSize
    // Check Rssi
    // Check Snr
    // Check RxSlot
    if( mcpsIndication->RxData == true )
    {
      memcpy(rxBuffer, mcpsIndication->Buffer, mcpsIndication->BufferSize);
      rxLen = mcpsIndication->BufferSize;
      rxPort = mcpsIndication->Port;
    }
    process_post(&lorawan, ev_rx, NULL);
}

/*!
 * \brief   MCPS-Confirm event function
 *
 * \param   [IN] mcpsConfirm - Pointer to the confirm structure,
 *               containing confirm attributes.
 */
static void McpsConfirm( McpsConfirm_t *mcpsConfirm )
{
  //debug("mcpsConfirm %u\r\n",  mcpsConfirm->Status);
#if DEBUG_TM
  sendConfTm = clock_time();
  printf("sendtm: %u  finsh tm: %u, dt: %u\r\n", sendTm, sendConfTm, sendConfTm-sendTm);
#endif
  if (mcpsConfirm->Status == LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT ||
     mcpsConfirm->Status == LORAMAC_EVENT_INFO_STATUS_RX1_TIMEOUT) {
    lorawanACTTimeout = 1;
    debug("call recv timeout\r\n");
  }
   if( mcpsConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
    {
        switch( mcpsConfirm->McpsRequest )
        {
            case MCPS_UNCONFIRMED:
            {
                // Check Datarate
                // Check TxPower
              debug("call unconfirmed\r\n");
                break;
            }
            case MCPS_CONFIRMED:
            {
                // Check Datarate
                // Check TxPower
                // Check AckReceived
                // Check NbTrials
              debug("call confirmed \r\n");
              //lorawanReqACKTm = 0;
                break;
            }
            case MCPS_PROPRIETARY:
            {
                break;
            }
            default:
                break;
        }
    }  
}

static void devInit(void)
{
    MibRequestConfirm_t mibReq;
                
                
    LoRaMacPrimitives.MacMcpsConfirm = McpsConfirm;
    LoRaMacPrimitives.MacMcpsIndication = McpsIndication;
    LoRaMacPrimitives.MacMlmeConfirm = MlmeConfirm;
    LoRaMacPrimitives.MacMlmeIndication = MlmeIndication;

    LoRaMacInitialization( &LoRaMacPrimitives, NULL/*callback*/, ACTIVE_REGION );



    mibReq.Type = MIB_ADR;
    mibReq.Param.AdrEnable = lorawan_cfg.ADREnable;
    LoRaMacMibSetRequestConfirm( &mibReq );

    mibReq.Type = MIB_PUBLIC_NETWORK;
    mibReq.Param.EnablePublicNetwork = LORAWAN_PUBLIC_NETWORK;
    LoRaMacMibSetRequestConfirm( &mibReq );
    
        /*设置默认使用的信道*/
    mibReq.Type = MIB_CHANNELS_DEFAULT_MASK;
    mibReq.Param.ChannelsMask = lorawan_cfg.ChannelMask;
    LoRaMacMibSetRequestConfirm( &mibReq );
    

    mibReq.Type = MIB_DEVICE_CLASS;     
    mibReq.Param.Class = CLASS_A;
    LoRaMacMibSetRequestConfirm( &mibReq );
            
    DeviceState = 1;
    
    memset(__onPortCall, 0, sizeof __onPortCall);
}

static void devJoin(void)
{
  MlmeReq_t mlmeReq;
  
  mlmeReq.Type = MLME_JOIN;

  mlmeReq.Req.Join.DevEui = getDeviceEUI();
  mlmeReq.Req.Join.AppEui = lorawan_cfg.JoinEUI;
  mlmeReq.Req.Join.AppKey = getRootKey(); //lorawan_cfg.Key;
  mlmeReq.Req.Join.Datarate = lorawan_cfg.DefaultDataRate;

  if( LoRaMacMlmeRequest( &mlmeReq ) == LORAMAC_STATUS_OK )
  {
       DeviceState = 2;
  }
  else
  {
      DeviceState = 3;
  }
}

PROCESS_THREAD(lorawan, ev, data)
{
  static struct etimer timer;
  static uint32_t tick = 0;

  uint8_t pkey[] = LoRaWAN_KEY;
  uint8_t pjoin[] = LoRaWAN_JOINEUI;
  uint16_t pmask[] = LoRaWAN_DEFAULT_CHMASK;
  
  PROCESS_BEGIN();
  
  ev_tx = process_alloc_event();
  ev_rx = process_alloc_event();
  
  memcpy(lorawan_cfg.Key, pkey, 16);
  memcpy(lorawan_cfg.JoinEUI,pjoin, 8);
  lorawan_cfg.ClassType = LoRaWAN_DEFAULT_CLASS;
  lorawan_cfg.ADREnable   = LoRaWAN_ADR;
  memcpy(lorawan_cfg.ChannelMask, pmask, 12);
  lorawan_cfg.DefaultDataRate  = LoRaWAN_DEFAULT_DATARATE;
  lorawan_cfg.aKey =   LoRaWAN_AKEY;
  
  config_init();
  
  devInit();
  DeviceState = 1;

  etimer_set(&timer, CLOCK_SECOND+rand1()%3000);
  while (1) {
    PROCESS_WAIT_EVENT();
    if (ev == ev_rx) {
      if (rxLen > 0) {
        //debug(" rx (%u) >>> ", rxPort);
        //for (int i=0; i<rxLen; i++) {
        //  debug_e("%02X ", rxBuffer[i]);
        //}
        //debug_e("\r\n");
        if (rxPort>0 && rxPort<=240){
          if (__onPortCall[rxPort-1] != NULL) {
            __onPortCall[rxPort-1](rxBuffer, rxLen);
          }
        }
        rxLen = 0;
      }
     
    }
    if (ev == ev_tx) {
      if (txLen > 0) {
        //debug(" tx (%u) <<< ", txPort);
        //for (int i=0; i<txLen; i++) {
        //  debug_e("%02X ", txBuffer[i]);
        //}
        //debug_e("\r\n");
        __loraWanSendData(txPort, txBuffer, txLen);
        //debug(" tx end: %d\r\n", r);
        
        txLen = 0;
      } 
    }
    
    if (etimer_expired(&timer)) {
      etimer_set(&timer, CLOCK_SECOND);
      tick += 1;
      if (DeviceState == 0) {
        devInit();
        DeviceState = 1;
      } 
      if (DeviceState == 1) {
        debug("join...\r\n");
          devJoin();
      }
      if (DeviceState == 2) {  //joining
      }
      if (DeviceState == 3) {  //join fail
        DeviceState = 1; //rejoin
        debug("join fail...\r\n");
      }
      if (DeviceState == 4) { //joined
        lorawanReqACKTm += 1;
        //debug("lorawanReqACKTm %u, to %u\r\n", lorawanReqACKTm, lorawanACTTimeout);
        if (lorawanReqACKTm > REQ_ACK_TIME && lorawanACTTimeout != 0) {
          static uint8_t f = 0;
          if (f) leds_on(1);
          else leds_off(1);
          f = !f;
          if (lorawanReqACKTm > REJOIN_TIME) {
            DeviceState = 0;
            lorawanACTTimeout = 0;
            lorawanACTTimeout = 0;
            __set_FAULTMASK(1);
            NVIC_SystemReset();
          }
        } else {
          leds_on(1);
          lorawanACTTimeout = 0;
        }
        
        {
           MibRequestConfirm_t mibReq;
           mibReq.Type = MIB_DEVICE_CLASS;
           LoRaMacMibGetRequestConfirm(&mibReq); 
           if (mibReq.Param.Class != lorawan_cfg.ClassType) {
              mibReq.Param.Class = (DeviceClass_t)lorawan_cfg.ClassType;
              LoRaMacMibSetRequestConfirm( &mibReq );
           }
        }
      } else {
        static uint8_t f = 0;
        if (f) leds_on(1);
        else leds_off(1);
        f = !f;
      }
    }
  } 
  PROCESS_END();
}
