/*********************************************************************************************
* �ļ���iic.h
* ���ߣ�zonesion
* ˵����iicͷ�ļ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
#ifndef _IIC_H_
#define _IIC_H_

/*********************************************************************************************
* �ⲿԭ�ͺ���
*********************************************************************************************/
void iic_delay_us(unsigned int i);
void iic2_init(void);
void iic2_start(void);
void iic2_stop(void);
void iic2_send_ack(int ack);
int iic2_recv_ack(void);
unsigned char iic2_write_byte(unsigned char data);
unsigned char iic2_read_byte(unsigned char ack);

int iic2_read_buf(char addr, char r, char *buf, int len);
int iic2_write_buf(char addr, char r, char *buf, int len);

#endif 