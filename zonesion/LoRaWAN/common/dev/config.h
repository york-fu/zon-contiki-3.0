#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "stm32f10x.h"  
#include "stm32f10x_flash.h"                     //flash操作接口文件（在库文件中），必须要包含  

u16 STMFLASH_ReadHalfWord(u32 faddr);		  //读出半字  
void STMFLASH_WriteLenByte(u32 WriteAddr,u32 DataToWrite,u16 Len);	//指定地址开始写入指定长度的数据
u32 STMFLASH_ReadLenByte(u32 ReadAddr,u16 Len);						//指定地址开始读取指定长度数据
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);		//从指定地址开始写入指定长度的数据
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);   		//从指定地址开始读出指定长度的数据

int config_init(void);

void config_save(void);


typedef struct {
 
  uint8_t JoinEUI[8];
  uint8_t Key[16];
  
  uint16_t ChannelMask[6];
  uint8_t DefaultDataRate;
  uint8_t ADREnable;
  uint8_t ClassType;

  uint8_t aKey;
} lorawan_cfg_t;

extern lorawan_cfg_t lorawan_cfg;
#endif