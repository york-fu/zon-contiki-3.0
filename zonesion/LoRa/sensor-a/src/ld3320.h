#ifndef __LD3320_H__
#define __LD3320_H__
/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include <stm32f10x.h>
char ld3320_read(void);
void ld3320_init();
void ld3320_add(char *s);
void ld3320_addrs(char *s,char *r);
void ld3320_addrx(char *s,unsigned char x);
void ld3320_clean(void);
void ld3320_reload(void);
void ld3320_debug(unsigned char cmd);

#endif
