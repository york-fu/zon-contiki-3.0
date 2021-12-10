/*********************************************************************************************
*M25P16����Ϊ2M bytes 
*������Ϊ32��������Sectors����
*ÿ��������256ҳ��
*ÿҳ��256���ֽڣ���ÿ��������64k��
*ÿ�οɲ���һ���������߲�������flash
**********************************************************************************************/
#include "M25pxx.h"
#include <sys/clock.h>
#define M25Pxx_delay_us(x) clock_delay_us(x);
#define M25Pxx_CS_L()    {GPIO_ResetBits(GPIOA, GPIO_Pin_4);}
#define M25Pxx_CS_H()    {GPIO_SetBits(GPIOA, GPIO_Pin_4);}
/*********************************************************************************************
* ����: M25Pxx__ReadWriteByte()
* ����: M25Pxx��дһ���ֽ�
* ����: TxData����M25Pxxд�������
* ����: ��M25Pxx����������
* �޸�:
* ע��: 
**********************************************************************************************/
#define M25Pxx__ReadWriteByte  SPI1_ReadWriteByte
/*********************************************************************************************
* ����: M25Pxx_init()
* ����: ��ʼ��M25Pxx
* ����: 
* ����: 
* �޸�:
* ע��: 
**********************************************************************************************/
void M25Pxx_init(void){
  GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);         //ʹ��GPIOAʱ��
  
  GPIO_InitStructure.GPIO_Pin =GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;                 //���
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                //�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;            //100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;                  //����
  GPIO_Init(GPIOA, &GPIO_InitStructure);                        //��ʼ��
  M25Pxx_CS_H();                                                //Ƭѡ��ѡ��
  SPI1_Init();                                                  //spi1��ʼ��
 // SPI1_SetSpeed(SPI_BaudRatePrescaler_2);                   //����Ϊ42Mʱ�ӣ�����ģʽ

}

/*********************************************************************************************
* ����: M25Pxx_Read_ID()
* ����: ��Flash�������ֽڵı�ʶ��
* ����: 
* ����: 
* �޸�:
* ע��: 
**********************************************************************************************/

unsigned int M25Pxx_Read_ID(void)
{
  unsigned char temp0,temp1,temp2;
  unsigned int id;
  M25Pxx_CS_L();
  M25Pxx__ReadWriteByte(RDID);
  temp0 = M25Pxx__ReadWriteByte(Dummy_Byte);
  temp1 = M25Pxx__ReadWriteByte(Dummy_Byte);
  temp2 = M25Pxx__ReadWriteByte(Dummy_Byte);
  
  M25Pxx_CS_H();
  id = temp0<<16 | temp1 << 8 | temp2;
  return id;
} 
/*********************************************************************************************
* ����: M25Pxx_WaitReady()
* ����: �ȴ�M25Pxx�������д��
* ����: 
* ����: 
* �޸�:
* ע��: 
**********************************************************************************************/
void M25Pxx_WaitReady(void){
  unsigned char state;
  do{
    M25Pxx_CS_L();
    M25Pxx__ReadWriteByte(RDSR);
  
    state = M25Pxx__ReadWriteByte(Dummy_Byte);
  
    M25Pxx_CS_H();
  }while(state & 0x01);
} 
/*********************************************************************************************
* ����: M25Pxx_ReadBuffer()
* ����: ��M25Pxx���ݵ�������
* ����: pBuffer: ��Ŵ�M25Pxx������������;
        ReadAddr: ������M25Pxx��ַ;
        ReadNum�����ݵĸ���.      
* ����: 
* �޸�:
* ע��: 
**********************************************************************************************/
void M25Pxx_ReadBuffer(unsigned char * pBuffer, unsigned int ReadAddr, unsigned int ReadNum){

  M25Pxx_CS_L();
  M25Pxx__ReadWriteByte(READ);
  M25Pxx__ReadWriteByte((ReadAddr & 0xFF0000) >> 16);
  M25Pxx__ReadWriteByte((ReadAddr & 0xFF00) >> 8);
  M25Pxx__ReadWriteByte(ReadAddr & 0xFF);
  while(ReadNum--){
    *pBuffer = M25Pxx__ReadWriteByte(Dummy_Byte);
    pBuffer++;
  }
  M25Pxx_CS_H();
}


/*********************************************************************************************
* ����: M25Pxx_WREN()
* ����: M25Pxxдʹ��
* ����: ��
* ����: 
* �޸�:
* ע��: 
**********************************************************************************************/
void M25Pxx_WREN(void){
  M25Pxx_CS_L();
  M25Pxx__ReadWriteByte(WREN);
  M25Pxx_CS_H();
} 
/*********************************************************************************************
* ����: M25Pxx_WRDI()
* ����: M25Pxxдʧ�ܣ�д������
* ����: ��
* ����: 
* �޸�:
* ע��: 
**********************************************************************************************/
void M25Pxx_WRDI(void){
  M25Pxx_CS_L();
  M25Pxx__ReadWriteByte(WRDI);
  M25Pxx_CS_H();
}

/*********************************************************************************************
* ����: M25Pxx_BlukErase()
* ����: M25Pxx��������
* ����: ��
* ����: 
* �޸�:
* ע��: 
**********************************************************************************************/
void M25Pxx_BlukErase(void){
  
  M25Pxx_WREN();                                                //M25Pxxдʹ��
  M25Pxx_CS_L();
  M25Pxx__ReadWriteByte(BE);
  M25Pxx_CS_H();
  
  M25Pxx_WRDI();                                                //M25Pxxдʹ��(д����)
  M25Pxx_WaitReady();                                           //�ȴ�M25Pxx�������д��            
  
}
/*********************************************************************************************
* ����: M25Pxx_SectorErase()
* ����: M25Pxx��������
* ����: SectorAddr�������׵�ַ
* ����: 
* �޸�:
* ע��: 
**********************************************************************************************/
void M25Pxx_SectorErase(unsigned int SectorAddr){
  
  M25Pxx_WREN();                                                //M25Pxxдʹ��
  M25Pxx_CS_L();
  M25Pxx__ReadWriteByte(SE);
  M25Pxx__ReadWriteByte((SectorAddr & 0xFF0000) >> 16);         //����������3�ֽڵ�ַ
  M25Pxx__ReadWriteByte((SectorAddr & 0xFF00) >> 8);
  M25Pxx__ReadWriteByte(SectorAddr & 0xFF);
  M25Pxx_CS_H();
  M25Pxx_WRDI();                                                //M25Pxxдʹ��(д����)
  M25Pxx_WaitReady();                                           //�ȴ�M25Pxx�������д��  
  
} 

/*********************************************************************************************
* ����: M25Pxx_WritePage()
* ����:дһҳ���ݵ�SPI_Flash
* ����:pBuffer: ���д��M25Pxx������
       WriteAddr: ָ��д��M25Pxx�ĵ�ַ
       WriteNum: д���ݵĸ���.
* ����: 
* �޸�:
* ע��: 
**********************************************************************************************/
void M25Pxx_WritePage(unsigned char* pBuffer, unsigned int WriteAddr, unsigned int WriteNum)
{
 
  M25Pxx_WREN();                                                //M25Pxxдʹ��
  //M25Pxx_delay_us(1);
  M25Pxx_CS_L();
  M25Pxx__ReadWriteByte(WRITE);                                 //дָ��
  M25Pxx__ReadWriteByte((WriteAddr & 0xFF0000) >> 16);          //����������3�ֽڵ�ַ
  M25Pxx__ReadWriteByte((WriteAddr & 0xFF00) >> 8);
  M25Pxx__ReadWriteByte(WriteAddr & 0xFF);
  while(WriteNum--)
  {
    M25Pxx__ReadWriteByte(*pBuffer);
    pBuffer++;
  }
  M25Pxx_CS_H();
  M25Pxx_WRDI();                                                //M25Pxxдʹ��(д����)
  M25Pxx_WaitReady();                                           //�ȴ�M25Pxx�������д��  
 
} 

/*********************************************************************************************
* ����: M25Pxx_WriteBuffer()
* ����:ͨ��д���ݵ�M25Pxx
* ����:pBuffer: ���д��M25Pxx������
       WriteAddr: ָ��д��M25Pxx�ĵ�ַ
       WriteNum: д���ݵĸ���.
* ����: 
* �޸�:
* ע��: 
**********************************************************************************************/
void M25Pxx_WriteBuffer(unsigned char* pBuffer, unsigned int WriteAddr, unsigned int WriteNum)   {   
  unsigned char NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;   
   
  Addr = WriteAddr % M25Pxx_PageSize;   
  count = M25Pxx_PageSize - Addr;   
  NumOfPage =  WriteNum / M25Pxx_PageSize;   
  NumOfSingle = WriteNum % M25Pxx_PageSize;   
   
  if (Addr == 0)                                                //���WriteAddr�ĵ��ֽ�Ϊ0,����ҳ��������
  {   
    if (NumOfPage == 0)                                         // WriteNum < M25Pxx_PageSize,ֱ��ҳд��
    {   
      M25Pxx_WritePage(pBuffer, WriteAddr, WriteNum);   
    }   
    else                                                        // WriteNum > M25Pxx_PageSize,��ҳд��,���д�벻��һҳ��
    {   
      while (NumOfPage--)   
      {   
        M25Pxx_WritePage(pBuffer, WriteAddr, M25Pxx_PageSize);   
        WriteAddr +=  M25Pxx_PageSize;   
        pBuffer += M25Pxx_PageSize;   
      }   
   
      M25Pxx_WritePage(pBuffer, WriteAddr, NumOfSingle);   
    }   
  }   
  else                                                          //���WriteAddr�ĵ��ֽڲ�Ϊ0,������ҳ��������
  {   
    if (NumOfPage == 0)                                         // WriteNum <M25Pxx_PAGESIZE
    {   
      if (NumOfSingle > count)                                  // ���WriteNum���ڵ�ַ���ֽڵ�ʣ��ռ�
      {   
        temp = NumOfSingle - count;                             //����д���ַ���ֽڵ�ʣ��ռ�
   
        M25Pxx_WritePage(pBuffer, WriteAddr, count);   
        WriteAddr +=  count;   
        pBuffer += count;   
   
        M25Pxx_WritePage(pBuffer, WriteAddr, temp);             //��д��һҳ
      }   
      else                                                      // ���WriteNumС�ڵ�ַ���ֽڵ�ʣ��ռ�,ֱ��д��
      {   
        M25Pxx_WritePage(pBuffer, WriteAddr, WriteNum);   
      }   
    }   
    else                                                        // WriteNum > M25Pxx_PageSize
    {   
      WriteNum -= count;   
      NumOfPage =  WriteNum / M25Pxx_PageSize;   
      NumOfSingle = WriteNum % M25Pxx_PageSize;   
   
      M25Pxx_WritePage(pBuffer, WriteAddr, count);             //��д���ַ���ֽڵ�ʣ��ռ�
      WriteAddr +=  count;   
      pBuffer += count;   
   
      while (NumOfPage--)                                     //�ٽ���ҳд��
      {   
        M25Pxx_WritePage(pBuffer, WriteAddr,M25Pxx_PageSize);   
        WriteAddr +=  M25Pxx_PageSize;   
        pBuffer += M25Pxx_PageSize;   
      }   
   
      if (NumOfSingle != 0)                                     //����в���һҳ,���д�벻��һҳ��
      {   
        M25Pxx_WritePage(pBuffer, WriteAddr, NumOfSingle);   
      }   
    }   
  }
}