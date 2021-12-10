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
void iic_init(void);
void iic_start(void);
void iic_stop(void);
void iic_send_ack(int ack);
int iic_recv_ack(void);
unsigned char iic_write_byte(unsigned char data);
unsigned char iic_read_byte(unsigned char ack);
int iic_read_buf(char addr, char r, char *buf, int len);
int iic_write_buf(char addr, char r, char *buf, int len);

#endif 