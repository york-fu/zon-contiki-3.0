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
#include "contiki-conf.h"
#include "string.h"
#include "stdio.h"
//FLASH��ʼ��ַ
#define STM32_FLASH_BASE 0x08000000 	//STM32 FLASH����ʼ��ַ
#define STM32_FLASH_SIZE 128 	 		//��ѡSTM32��FLASH������С(��λΪK)

//FLASH ��������ʼ��ַ
#define ADDR_FLASH_SECTOR_0     ((u32)0x08000000) 	//����0��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_1     ((u32)0x08004000) 	//����1��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_2     ((u32)0x08008000) 	//����2��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_3     ((u32)0x0800C000) 	//����3��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_4     ((u32)0x08010000) 	//����4��ʼ��ַ, 64 Kbytes 


nb_config_t  nbConfig = {
  .id = AID,
  .key = AKEY,
  .ip = CONFIG_ZHIYUN_IP,
  .port = CONFIG_ZHIYUN_PORT,
  .mode = 1
};

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

void config_save_id(char *data)
{
  STMFLASH_Write(ADDR_FLASH_SECTOR_3+200,(u16 *)data,(strlen(data)+1));
}

void config_read_id()
{
  char buf[33];
  char n = 0;
  STMFLASH_Read(ADDR_FLASH_SECTOR_3+200, (u16 *)buf, 16);
  while((buf[n] >= '.') && (buf[n] <= 'z') && (n < 16)){
  nbConfig.id[n] = buf[n];
  n++;
  }
  if(n > 0)
  nbConfig.id[n] = 0;
}



void config_save_key(char *data)
{
  STMFLASH_Write(ADDR_FLASH_SECTOR_3+300,(u16 *)data,(strlen(data)+1));
}

void config_read_key()
{
  char buf[130];
  char n = 0;
  STMFLASH_Read(ADDR_FLASH_SECTOR_3+300, (u16 *)buf, 64);
  while((buf[n] >= '.') && (buf[n] <= 'z') && (n < 64)){
  nbConfig.key[n] = buf[n];
  n++;
  }
  if(n > 0)
  nbConfig.key[n] = 0;
}

void config_save_ip(char *data)
{
  STMFLASH_Write(ADDR_FLASH_SECTOR_3+400,(u16 *)data,(strlen(data)+1));
}

void config_read_ip()
{
  char buf[130];
  char n = 0;
  STMFLASH_Read(ADDR_FLASH_SECTOR_3+400, (u16 *)buf, 64);
  while((buf[n] >= '.') && (buf[n] <= 'z') && (n < 64)){
  nbConfig.ip[n] = buf[n];
  n++;
}
  if(n > 0)
  nbConfig.ip[n] = 0;

}

/*********************************************************************************************
* ���ƣ�config_init(void)
* ���ܣ����ó�ʼ������
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
int config_init(void)
{
  u32 flag = 0;
  u32 off = (sizeof(nbConfig)+3)&0xfffffffC;
  
  STMFLASH_Read(ADDR_FLASH_SECTOR_3+off, (u16*)&flag, 2);
  if (flag != 0xa4a4a4a4) {
    //Ĭ�ϲ��ı�
  } else {
    STMFLASH_Read(ADDR_FLASH_SECTOR_3, (u16*)&nbConfig, (sizeof(nbConfig)+1)>>1);
  }
  return 0;
}


/*********************************************************************************************
* ���ƣ�
* ���ܣ�
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void config_save(void)
{
  u32 flag = 0;
  u32 off = (sizeof(nbConfig)+3)&0xfffffffC;
  
  STMFLASH_Write(ADDR_FLASH_SECTOR_3, (u16*)&nbConfig, (sizeof(nbConfig)+1)>>1);
  
  STMFLASH_Read(ADDR_FLASH_SECTOR_3+off, (u16*)&flag, 2);
  if (flag != 0xa4a4a4a4) {
    flag =  0xa4a4a4a4;
    STMFLASH_Write(ADDR_FLASH_SECTOR_3+off, (u16*)&flag, 2);
  } else {
    
  }
}