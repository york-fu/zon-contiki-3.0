/****************************************************************  
*Function:  STM32F103ϵ���ڲ�Flash��д����  
*Author:    ValerianFan  
*Date:      2014/04/09  
*E-Mail:    fanwenjingnihao@163.com  
*Other:     
****************************************************************/  
  
  
#include "stm32f10x.h"  
#include "stm32f10x_flash.h"                     //flash�����ӿ��ļ����ڿ��ļ��У�������Ҫ����  
#include "config.h"
#include "sx1278-a.h"

//FLASH��ʼ��ַ
#define STM32_FLASH_BASE 0x08000000 	//STM32 FLASH����ʼ��ַ
#define STM32_FLASH_SIZE 128 	 		//��ѡSTM32��FLASH������С(��λΪK)

//FLASH ��������ʼ��ַ
#define ADDR_FLASH_SECTOR_0     ((u32)0x08000000) 	//����0��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_1     ((u32)0x08004000) 	//����1��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_2     ((u32)0x08008000) 	//����2��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_3     ((u32)0x0800C000) 	//����3��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_4     ((u32)0x08010000) 	//����4��ʼ��ַ, 64 Kbytes 

//��ȡָ����ַ�İ���(16λ����)
//faddr:����ַ(�˵�ַ����Ϊ2�ı���!!)
//����ֵ:��Ӧ����.
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr; 
} 
//������д��
//WriteAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��   
void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)   
{ 			 		 
	u16 i;
	for(i=0;i<NumToWrite;i++)
	{
		FLASH_ProgramHalfWord(WriteAddr,pBuffer[i]);
	    WriteAddr+=2;//��ַ����2.
	}  
} 
//��ָ����ַ��ʼд��ָ�����ȵ�����
//WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ2�ı���!!)
//pBuffer:����ָ��
//NumToWrite:����(16λ)��(����Ҫд���16λ���ݵĸ���.)
#define STM_SECTOR_SIZE 1024 //�ֽ�	 
u16 STMFLASH_BUF[STM_SECTOR_SIZE/2];//�����2K�ֽ�
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)	
{
	u32 secpos;	   //������ַ
	u16 secoff;	   //������ƫ�Ƶ�ַ(16λ�ּ���)
	u16 secremain; //������ʣ���ַ(16λ�ּ���)	   
 	u16 i;    
	u32 offaddr;   //ȥ��0X08000000��ĵ�ַ
	if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return;//�Ƿ���ַ
	FLASH_Unlock();						//����
	offaddr=WriteAddr-STM32_FLASH_BASE;		//ʵ��ƫ�Ƶ�ַ.
	secpos=offaddr/STM_SECTOR_SIZE;			//������ַ  0~127 for STM32F103RBT6
	secoff=(offaddr%STM_SECTOR_SIZE)/2;		//�������ڵ�ƫ��(2���ֽ�Ϊ������λ.)
	secremain=STM_SECTOR_SIZE/2-secoff;		//����ʣ��ռ��С   
	if(NumToWrite<=secremain)secremain=NumToWrite;//�����ڸ�������Χ
	while(1) 
	{	
		STMFLASH_Read(secpos*STM_SECTOR_SIZE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//������������������
		for(i=0;i<secremain;i++)//У������
		{
			if(STMFLASH_BUF[secoff+i]!=0XFFFF)break;//��Ҫ����  	  
		}
		if(i<secremain)//��Ҫ����
		{
			FLASH_ErasePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//�����������
			for(i=0;i<secremain;i++)//����
			{
				STMFLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//д����������  
		}else STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������. 				   
		if(NumToWrite==secremain)break;//д�������
		else//д��δ����
		{
			secpos++;				//������ַ��1
			secoff=0;				//ƫ��λ��Ϊ0 	 
		   	pBuffer+=secremain;  	//ָ��ƫ��
			WriteAddr+=secremain;	//д��ַƫ��	   
		   	NumToWrite-=secremain;	//�ֽ�(16λ)���ݼ�
			if(NumToWrite>(STM_SECTOR_SIZE/2))secremain=STM_SECTOR_SIZE/2;//��һ����������д����
			else secremain=NumToWrite;//��һ����������д����
		}	 
	};	
	FLASH_Lock();//����
}

//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)   	
{
	u16 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//��ȡ2���ֽ�.
		ReadAddr+=2;//ƫ��2���ֽ�.	
	}
}  
  
void config_save_fp(u16 data)
{
  STMFLASH_Write(ADDR_FLASH_SECTOR_3+20,&data,1);
}

int config_read_fp()
{
  u16 data = 0;
  STMFLASH_Read(ADDR_FLASH_SECTOR_3+20, &data, 1);
  return data;
}

void config_save_pv(u16 data)
{
  STMFLASH_Write(ADDR_FLASH_SECTOR_3+30,&data,1);
}

int config_read_pv()
{
  u16 data = 0;
  STMFLASH_Read(ADDR_FLASH_SECTOR_3+30, &data, 1);
  return data;
}

void config_save_sf(u16 data)
{
  STMFLASH_Write(ADDR_FLASH_SECTOR_3+40,&data,1);
}

int config_read_sf()
{
  u16 data = 0;
  STMFLASH_Read(ADDR_FLASH_SECTOR_3+40, &data, 1);
  return data;
}

void config_save_cr(u16 data)
{
  STMFLASH_Write(ADDR_FLASH_SECTOR_3+50,&data,1);
}

int config_read_cr()
{
  u16 data = 0;
  STMFLASH_Read(ADDR_FLASH_SECTOR_3+50, &data, 1);
  return data;
}

void config_save_bw(u16 data)
{
  STMFLASH_Write(ADDR_FLASH_SECTOR_3+60,&data,1);
}

int config_read_bw()
{
  u16 data = 0;
  STMFLASH_Read(ADDR_FLASH_SECTOR_3+60, &data, 1);
  return data;
}

void config_save_ps(u16 data)
{
  STMFLASH_Write(ADDR_FLASH_SECTOR_3+70,&data,1);
}

int config_read_ps()
{
  u16 data = 0;
  STMFLASH_Read(ADDR_FLASH_SECTOR_3+70, &data, 1);
  return data;
}

void config_save_hop(u16 data)
{
  STMFLASH_Write(ADDR_FLASH_SECTOR_3+80,&data,1);
}

int config_read_hop()
{
  u16 data = 0;
  STMFLASH_Read(ADDR_FLASH_SECTOR_3+80, &data, 1);
  return data;
}

void config_save_id(u16 data)
{
  STMFLASH_Write(ADDR_FLASH_SECTOR_3+90,&data,1);
}

int config_read_id()
{
  u16 data = 0;
  STMFLASH_Read(ADDR_FLASH_SECTOR_3+90, &data, 1);
  return data;
}


/*********************************************************************************************
* ���ƣ�config_init(void)
* ���ܣ����ó�ʼ������
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
extern sx1278_config_t  sx1278Config ;
int config_init(void)
{
  /*u16 flag = 0;
  STMFLASH_Read(ADDR_FLASH_SECTOR_3+10,&flag, 1);
  if (flag != 0x5a) {
    flag = 0x5a;
    STMFLASH_Write(ADDR_FLASH_SECTOR_3+10,&flag,1);
    return 0;
  } else {
    return 1;
  }*/
  u32 flag = 0;
  u32 off = (sizeof(sx1278Config)+3)&0xfffffffC;
  
  STMFLASH_Read(ADDR_FLASH_SECTOR_3+off, (u16*)&flag, 2);
  if (flag != 0xa5a5a5a5) {
    //Ĭ�ϲ��ı�
  } else {
    STMFLASH_Read(ADDR_FLASH_SECTOR_3, (u16*)&sx1278Config, (sizeof(sx1278Config)+1)>>1);
  }
  return 1;
}

void config_save(void)
{
  u32 flag = 0;
  u32 off = (sizeof(sx1278Config)+3)&0xfffffffC;
  
  STMFLASH_Write(ADDR_FLASH_SECTOR_3, (u16*)&sx1278Config, (sizeof(sx1278Config)+1)>>1);
  
  STMFLASH_Read(ADDR_FLASH_SECTOR_3+off, (u16*)&flag, 2);
  if (flag != 0xa5a5a5a5) {
    flag =  0xa5a5a5a5;
    STMFLASH_Write(ADDR_FLASH_SECTOR_3+off, (u16*)&flag, 2);
  } else {
    
  }
  
}
