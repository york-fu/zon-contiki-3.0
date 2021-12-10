#ifndef __ZXBEE_H__
#define __ZXBEE_H__

void zxbeeBegin(void);
int zxbeeAdd(char* tag, char* val);
char* zxbeeEnd(void);
void zxbee_onrecv_fun(char *pkg, int len);

#endif