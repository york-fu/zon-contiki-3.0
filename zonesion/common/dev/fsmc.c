/*********************************************************************************************
* �ļ���main.c
* ���ߣ�Lixm 2017.01.20
* ˵��������LED�ƿ��Ƴ���
* �޸ģ�Chenkm 2017.02.16 ���ע��
* ע�ͣ�
*
*********************************************************************************************/
/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include "stm32f4xx.h"

/*********************************************************************************************
* �궨��
*********************************************************************************************/
#define ST7789  0

/* zonesion zxbeepluse �ӷ�*/
/*  fsmc io
     PD12 -- RS // lcd  A17
#if ST7789
     PG2 --- RS // A12  //2.4��С���� or  PD13 --- A18
#endif

     PD7 -- CS // lcd  ��ǰ��·PD13��Ƭѡ���ܣ���Ҫ����PD7�� NE1����
  ---------------------
     PD4 --- NOE
     PD5 --- NWE
     PD14 -- D0
     PD15 -- D1
     PD0 --- D2
     PD1 --- D3

     PE7 --- D4
     PE8     D5
      .      .
      .      .
     PE15 -- D12
     PD8 --- D13
     PD9 --- D14
     PD10 -- D15
 */
 
 /*********************************************************************************************
* ����:fsmc_init()
* ����:fsmc��ʼ��
* ����:��
* ����:��
* �޸�:
* ע��:
*********************************************************************************************/
void fsmc_init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
  FSMC_NORSRAMTimingInitTypeDef  readWriteTiming;
  FSMC_NORSRAMTimingInitTypeDef  writeTiming;

  RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);          //ʹ��FSMCʱ��
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE|RCC_AHB1Periph_GPIOG, ENABLE);//ʹ��GPIOʱ��



  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_14 | GPIO_Pin_15 |
                                GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_8 | GPIO_Pin_9 |
                                GPIO_Pin_10| GPIO_Pin_7;        //ѡ����Ӧ������
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;                  //����ģʽ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;            //����ٶ�
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                //�������
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;              //��������
  GPIO_Init(GPIOD, &GPIO_InitStructure);                        //������������ʼ��(PD)

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10
    |                           GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14|
                                GPIO_Pin_15;                    //ѡ����Ӧ������
  GPIO_Init(GPIOE, &GPIO_InitStructure);                        //������������ʼ��(PE)

#if ST7789
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;                   //ѡ����Ӧ������
   GPIO_Init(GPIOD, &GPIO_InitStructure);

   GPIO_PinAFConfig(GPIOD,GPIO_PinSource13,GPIO_AF_FSMC);
#else
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;                   //ѡ����Ӧ������
   GPIO_Init(GPIOD, &GPIO_InitStructure);

   GPIO_PinAFConfig(GPIOD,GPIO_PinSource12,GPIO_AF_FSMC);
#endif

   //�������ã����������Ÿ���ΪFSMC
  //GPIO_PinAFConfig(GPIOD,GPIO_PinSource12,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource7,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource4,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource5,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource14,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource15,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource0,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource1,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource8,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource9,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource10,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource7,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource8,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource9,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource10,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource11,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource12,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource13,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource14,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource15,GPIO_AF_FSMC);
  //д����
  readWriteTiming.FSMC_AddressSetupTime = 0XF;	                //��ַ����ʱ��Ϊ16��HCLK 1/168M=6ns*16=96ns
  readWriteTiming.FSMC_AddressHoldTime = 0;	                //��ַ����ʱ�� ģʽAδ�õ�
  readWriteTiming.FSMC_DataSetupTime = 60;		        //���ݱ���ʱ��Ϊ60��HCLK =6*60=360ns
  readWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
  readWriteTiming.FSMC_CLKDivision = 0x00;
  readWriteTiming.FSMC_DataLatency = 0x00;
  readWriteTiming.FSMC_AccessMode = FSMC_AccessMode_A;	        //ģʽA
 //������
  writeTiming.FSMC_AddressSetupTime =15;	                //��ַ����ʱ��Ϊ9��HCLK =54ns
  writeTiming.FSMC_AddressHoldTime = 0;	                        //��ַ����ʱ��
  writeTiming.FSMC_DataSetupTime = 15;		                //���ݱ���ʱ��Ϊ6ns*9��HCLK=54ns
  writeTiming.FSMC_BusTurnAroundDuration = 0x00;
  writeTiming.FSMC_CLKDivision = 0x00;
  writeTiming.FSMC_DataLatency = 0x00;
  writeTiming.FSMC_AccessMode = FSMC_AccessMode_A;	        //ģʽA
 //����FSMC
  FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;    //ʹ��NE1
  FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable; //���������ݵ�ַ
  FSMC_NORSRAMInitStructure.FSMC_MemoryType =FSMC_MemoryType_SRAM;// FSMC_MemoryType_SRAM;  //SRAM
  FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;//�洢�����16bit
  FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode =FSMC_BurstAccessMode_Disable;// FSMC_BurstAccessMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
  FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait=FSMC_AsynchronousWait_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
  FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;	//�洢��дʹ��
  FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable; //��дʹ�ò�ͬʱ��
  FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &readWriteTiming; //��ʱ��
  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &writeTiming;  //дʱ��

  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);                 //��ʼ��fsmc����
  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);                 // ʹ��bank1,sram1
}
