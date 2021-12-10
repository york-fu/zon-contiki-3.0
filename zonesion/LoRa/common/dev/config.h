#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "stm32f10x.h"  
#include "stm32f10x_flash.h"                     //flash操作接口文件（在库文件中），必须要包含  

u16 STMFLASH_ReadHalfWord(u32 faddr);		  //读出半字  
void STMFLASH_WriteLenByte(u32 WriteAddr,u32 DataToWrite,u16 Len);	//指定地址开始写入指定长度的数据
u32 STMFLASH_ReadLenByte(u32 ReadAddr,u16 Len);						//指定地址开始读取指定长度数据
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);		//从指定地址开始写入指定长度的数据
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);   		//从指定地址开始读出指定长度的数据
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