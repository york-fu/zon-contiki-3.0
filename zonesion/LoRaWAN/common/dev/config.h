#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "stm32f10x.h"  
#include "stm32f10x_flash.h"                     //flash�����ӿ��ļ����ڿ��ļ��У�������Ҫ����  

u16 STMFLASH_ReadHalfWord(u32 faddr);		  //��������  
void STMFLASH_WriteLenByte(u32 WriteAddr,u32 DataToWrite,u16 Len);	//ָ����ַ��ʼд��ָ�����ȵ�����
u32 STMFLASH_ReadLenByte(u32 ReadAddr,u16 Len);						//ָ����ַ��ʼ��ȡָ����������
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);		//��ָ����ַ��ʼд��ָ�����ȵ�����
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);   		//��ָ����ַ��ʼ����ָ�����ȵ�����

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