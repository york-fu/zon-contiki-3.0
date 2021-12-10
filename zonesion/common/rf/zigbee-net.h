#ifndef __ZIGBEE_NET_H__
#define __ZIGBEE_NET_H__

char* zigbee_mac();
unsigned int zigbee_panid();
unsigned int zigbee_channel();
extern int zigbee_type();
void zigbee_send(char *str);
void zigbee_set_onrecv(void (*f)(char *, int));

#endif