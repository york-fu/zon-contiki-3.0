/*********************************************************************************************
* �ļ���sensor.h
* ���ߣ�Xuzhy 2018.5.16
* ˵����sensorͷ�ļ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
#ifndef SENSOR_H
#define SENSOR_H
/*********************************************************************************************
* �궨��
*********************************************************************************************/
#define NODE_NAME "602"
extern void sensorInit(void);
extern void sensorLinkOn(void);
extern void sensorUpdate(void);
extern void sensorCheck(void);
extern void sensorControl(uint8_t cmd);
#endif
