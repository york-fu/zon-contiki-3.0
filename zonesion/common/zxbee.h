#ifndef __ZXBEE_H__
#define __ZXBEE_H__

void zxbeeBegin(void);
int zxbeeAdd(char* tag, char* val);
char* zxbeeEnd(void);

#endif