#ifndef __GPS_H
#define __GPS_H 

extern unsigned char start_cmd[];       //��ʼ��������
extern unsigned char gps_flag[];                                         //ֹͣ����ָ��
extern unsigned char GPS_cmd[200];       //��λָ��

extern unsigned int hh = 0;
extern unsigned int mm = 0;
extern unsigned int ss = 0;
extern double Lat_v,Lon_v;
extern int Lat_deg,Lon_deg;
extern double Lat_mi,Lon_mi;
extern unsigned char sn;
extern unsigned char ew;

int gps_get(char *lat, char *lng);

#endif
