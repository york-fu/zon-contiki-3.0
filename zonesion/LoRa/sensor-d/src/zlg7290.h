/*********************************************************************************************
* �ļ���zlg7290.h
* ���ߣ�zonesion
* ˵����zlg7290ͷ�ļ�
* �޸ģ�Chenkm 2017.01.10 �޸Ĵ����ʽ�����Ӵ���ע�ͺ��ļ�˵��
* ע�ͣ�
*********************************************************************************************/
#ifndef __ZLG7290_H__
#define __ZLG7290_H__

/*********************************************************************************************
* �궨��
*********************************************************************************************/
#define	   ZLG7290ADDR          0x70		                    //ZLG7290��IIC��ַ
#define	   SYETEM_REG           0x00		                    //ϵͳ�Ĵ���
#define	   KEY_REG              0x01		                    //��ֵ�Ĵ���
#define	   REPEATCNT_REG        0x02		                    //���������Ĵ���
#define	   FUNCTIONKEY          0x03		                    //���ܼ��Ĵ���
#define	   CMDBUF0              0x07		                    //�������0
#define	   CMDBUF1              0x08		                    //�������1
#define	   FALSH                0x0C		                    //��˸���ƼĴ���
#define	   SCANNUM              0x0D		                    //ɨ��λ���Ĵ���
#define	   DPRAM0               0x10		                    //��ʾ����Ĵ���0
#define	   DPRAM1               0x11		                    //��ʾ����Ĵ���1
#define	   DPRAM2               0x12		                    //��ʾ����Ĵ���2
#define	   DPRAM3               0x13		                    //��ʾ����Ĵ���3
#define	   DPRAM4               0x14		                    //��ʾ����Ĵ���4
#define	   DPRAM5               0x15		                    //��ʾ����Ĵ���5
#define	   DPRAM6               0x16		                    //��ʾ����Ĵ���6
#define	   DPRAM7               0x17		                    //��ʾ����Ĵ���7

#define	   UP                   0x01		                    //��
#define	   LEFT                 0x02		                    //��
#define	   DOWN                 0x03		                    //��
#define	   RIGHT                0x04		                    //��
#define	   CENTER               0x05		                    //��


/*********************************************************************************************
* �ⲿԭ�ͺ���
*********************************************************************************************/
void  zlg7290_init(void);
unsigned char zlg7290_read_reg(unsigned char cmd);
void zlg7290_write_data(unsigned char cmd, unsigned char data);

void zlg7290_download(unsigned char addr, unsigned dp, unsigned char flash, unsigned char dat);
void segment_display(unsigned int num);
unsigned char zlg7290_get_keyval(void);
void display_off(void);
#endif //__ZLG7290_H__