#ifndef __SX1278_IO_H__
#define __SX1278_IO_H__


void sx1278_IO_Init(void);
void sx1278RegWrite(int reg, int val);
int sx1278RegRead(int reg);
void sx1278BufWrite(char *buf, int len);
int sx1278BufRead(char *buf, int len);
void sx1278_reset();
void sx1278_cs(int s);

#endif