#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "stm32f10x.h"  
#include "stm32f10x_flash.h"                     //flash�����ӿ��ļ����ڿ��ļ��У�������Ҫ����  

u16 STMFLASH_ReadHalfWord(u32 faddr);		  //��������  
void STMFLASH_WriteLenByte(u32 WriteAddr,u32 DataToWrite,u16 Len);	//ָ����ַ��ʼд��ָ�����ȵ�����
u32 STMFLASH_ReadLenByte(u32 ReadAddr,u16 Len);						//ָ����ַ��ʼ��ȡָ����������
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);		//��ָ����ַ��ʼд��ָ�����ȵ�����
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);   		//��ָ����ַ��ʼ����ָ�����ȵ�����
void config_save_fp(u16 data);
int config_read_fp();
void config_save_pv(u16 data);
int config_read_pv();
void config_save_sf(u16 data);
int config_read_sf();
void config_save_cr(u16 data);
int config_read_cr();
void config_save_bw(u16 data);
int config_read_bw();
void config_save_ps(u16 data);
int config_read_ps();
void config_save_hop(u16 data);
int config_read_hop();
void config_save_id(u16 data);
int config_read_id();
int config_init(void);

void config_save(void);

#endif