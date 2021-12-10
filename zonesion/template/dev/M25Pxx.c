/*********************************************************************************************
*M25P16容量为2M bytes 
*被划分为32个扇区（Sectors），
*每个扇区有256页，
*每页有256个字节，即每个扇区有64k，
*每次可擦除一个扇区或者擦除整个flash
**********************************************************************************************/
#include "M25pxx.h"
#include <sys/clock.h>
#define M25Pxx_delay_us(x) clock_delay_us(x);
#define M25Pxx_CS_L()    {GPIO_ResetBits(GPIOA, GPIO_Pin_4);}
#define M25Pxx_CS_H()    {GPIO_SetBits(GPIOA, GPIO_Pin_4);}
/*********************************************************************************************
* 名称: M25Pxx__ReadWriteByte()
* 功能: M25Pxx读写一个字节
* 参数: TxData：向M25Pxx写入的数据
* 返回: 从M25Pxx读出的数据
* 修改:
* 注释: 
**********************************************************************************************/
#define M25Pxx__ReadWriteByte  SPI1_ReadWriteByte
/*********************************************************************************************
* 名称: M25Pxx_init()
* 功能: 初始化M25Pxx
* 参数: 
* 返回: 
* 修改:
* 注释: 
**********************************************************************************************/
void M25Pxx_init(void){
  GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);         //使能GPIOA时钟
  
  GPIO_InitStructure.GPIO_Pin =GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;                 //输出
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                //推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;            //100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;                  //上拉
  GPIO_Init(GPIOA, &GPIO_InitStructure);                        //初始化
  M25Pxx_CS_H();                                                //片选不选中
  SPI1_Init();                                                  //spi1初始化
 // SPI1_SetSpeed(SPI_BaudRatePrescaler_2);                   //设置为42M时钟，高速模式

}

/*********************************************************************************************
* 名称: M25Pxx_Read_ID()
* 功能: 读Flash的三个字节的标识码
* 参数: 
* 返回: 
* 修改:
* 注释: 
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
* 名称: M25Pxx_WaitReady()
* 功能: 等待M25Pxx完成数据写入
* 参数: 
* 返回: 
* 修改:
* 注释: 
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
* 名称: M25Pxx_ReadBuffer()
* 功能: 读M25Pxx数据到缓冲区
* 参数: pBuffer: 存放从M25Pxx读出来的数据;
        ReadAddr: 被读的M25Pxx地址;
        ReadNum读数据的个数.      
* 返回: 
* 修改:
* 注释: 
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
* 名称: M25Pxx_WREN()
* 功能: M25Pxx写使能
* 参数: 无
* 返回: 
* 修改:
* 注释: 
**********************************************************************************************/
void M25Pxx_WREN(void){
  M25Pxx_CS_L();
  M25Pxx__ReadWriteByte(WREN);
  M25Pxx_CS_H();
} 
/*********************************************************************************************
* 名称: M25Pxx_WRDI()
* 功能: M25Pxx写失能（写保护）
* 参数: 无
* 返回: 
* 修改:
* 注释: 
**********************************************************************************************/
void M25Pxx_WRDI(void){
  M25Pxx_CS_L();
  M25Pxx__ReadWriteByte(WRDI);
  M25Pxx_CS_H();
}

/*********************************************************************************************
* 名称: M25Pxx_BlukErase()
* 功能: M25Pxx擦除整块
* 参数: 无
* 返回: 
* 修改:
* 注释: 
**********************************************************************************************/
void M25Pxx_BlukErase(void){
  
  M25Pxx_WREN();                                                //M25Pxx写使能
  M25Pxx_CS_L();
  M25Pxx__ReadWriteByte(BE);
  M25Pxx_CS_H();
  
  M25Pxx_WRDI();                                                //M25Pxx写使能(写保护)
  M25Pxx_WaitReady();                                           //等待M25Pxx完成数据写入            
  
}
/*********************************************************************************************
* 名称: M25Pxx_SectorErase()
* 功能: M25Pxx擦除扇区
* 参数: SectorAddr：扇区首地址
* 返回: 
* 修改:
* 注释: 
**********************************************************************************************/
void M25Pxx_SectorErase(unsigned int SectorAddr){
  
  M25Pxx_WREN();                                                //M25Pxx写使能
  M25Pxx_CS_L();
  M25Pxx__ReadWriteByte(SE);
  M25Pxx__ReadWriteByte((SectorAddr & 0xFF0000) >> 16);         //接下来发送3字节地址
  M25Pxx__ReadWriteByte((SectorAddr & 0xFF00) >> 8);
  M25Pxx__ReadWriteByte(SectorAddr & 0xFF);
  M25Pxx_CS_H();
  M25Pxx_WRDI();                                                //M25Pxx写使能(写保护)
  M25Pxx_WaitReady();                                           //等待M25Pxx完成数据写入  
  
} 

/*********************************************************************************************
* 名称: M25Pxx_WritePage()
* 功能:写一页数据到SPI_Flash
* 参数:pBuffer: 存放写入M25Pxx的数据
       WriteAddr: 指定写入M25Pxx的地址
       WriteNum: 写数据的个数.
* 返回: 
* 修改:
* 注释: 
**********************************************************************************************/
void M25Pxx_WritePage(unsigned char* pBuffer, unsigned int WriteAddr, unsigned int WriteNum)
{
 
  M25Pxx_WREN();                                                //M25Pxx写使能
  //M25Pxx_delay_us(1);
  M25Pxx_CS_L();
  M25Pxx__ReadWriteByte(WRITE);                                 //写指令
  M25Pxx__ReadWriteByte((WriteAddr & 0xFF0000) >> 16);          //接下来发送3字节地址
  M25Pxx__ReadWriteByte((WriteAddr & 0xFF00) >> 8);
  M25Pxx__ReadWriteByte(WriteAddr & 0xFF);
  while(WriteNum--)
  {
    M25Pxx__ReadWriteByte(*pBuffer);
    pBuffer++;
  }
  M25Pxx_CS_H();
  M25Pxx_WRDI();                                                //M25Pxx写使能(写保护)
  M25Pxx_WaitReady();                                           //等待M25Pxx完成数据写入  
 
} 

/*********************************************************************************************
* 名称: M25Pxx_WriteBuffer()
* 功能:通用写数据到M25Pxx
* 参数:pBuffer: 存放写入M25Pxx的数据
       WriteAddr: 指定写入M25Pxx的地址
       WriteNum: 写数据的个数.
* 返回: 
* 修改:
* 注释: 
**********************************************************************************************/
void M25Pxx_WriteBuffer(unsigned char* pBuffer, unsigned int WriteAddr, unsigned int WriteNum)   {   
  unsigned char NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;   
   
  Addr = WriteAddr % M25Pxx_PageSize;   
  count = M25Pxx_PageSize - Addr;   
  NumOfPage =  WriteNum / M25Pxx_PageSize;   
  NumOfSingle = WriteNum % M25Pxx_PageSize;   
   
  if (Addr == 0)                                                //如果WriteAddr的低字节为0,即是页的整数倍
  {   
    if (NumOfPage == 0)                                         // WriteNum < M25Pxx_PageSize,直接页写入
    {   
      M25Pxx_WritePage(pBuffer, WriteAddr, WriteNum);   
    }   
    else                                                        // WriteNum > M25Pxx_PageSize,先页写入,最后写入不足一页的
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
  else                                                          //如果WriteAddr的低字节不为0,即不是页的整数倍
  {   
    if (NumOfPage == 0)                                         // WriteNum <M25Pxx_PAGESIZE
    {   
      if (NumOfSingle > count)                                  // 如果WriteNum大于地址低字节的剩余空间
      {   
        temp = NumOfSingle - count;                             //则先写完地址低字节的剩余空间
   
        M25Pxx_WritePage(pBuffer, WriteAddr, count);   
        WriteAddr +=  count;   
        pBuffer += count;   
   
        M25Pxx_WritePage(pBuffer, WriteAddr, temp);             //再写下一页
      }   
      else                                                      // 如果WriteNum小于地址低字节的剩余空间,直接写入
      {   
        M25Pxx_WritePage(pBuffer, WriteAddr, WriteNum);   
      }   
    }   
    else                                                        // WriteNum > M25Pxx_PageSize
    {   
      WriteNum -= count;   
      NumOfPage =  WriteNum / M25Pxx_PageSize;   
      NumOfSingle = WriteNum % M25Pxx_PageSize;   
   
      M25Pxx_WritePage(pBuffer, WriteAddr, count);             //先写完地址低字节的剩余空间
      WriteAddr +=  count;   
      pBuffer += count;   
   
      while (NumOfPage--)                                     //再进行页写入
      {   
        M25Pxx_WritePage(pBuffer, WriteAddr,M25Pxx_PageSize);   
        WriteAddr +=  M25Pxx_PageSize;   
        pBuffer += M25Pxx_PageSize;   
      }   
   
      if (NumOfSingle != 0)                                     //如果有不足一页,最后写入不足一页的
      {   
        M25Pxx_WritePage(pBuffer, WriteAddr, NumOfSingle);   
      }   
    }   
  }
}