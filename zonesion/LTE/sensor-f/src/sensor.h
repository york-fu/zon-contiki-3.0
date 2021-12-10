#ifndef __SENSOR_H__
#define __SENSOR_H__

#define NODE_NAME "611"
extern void sensorInit(void);
extern void sensorLinkOn(void);
extern void sensorUpdate(void);
extern void sensorCheck(void);
extern void sensorControl(uint8_t cmd);
#endif