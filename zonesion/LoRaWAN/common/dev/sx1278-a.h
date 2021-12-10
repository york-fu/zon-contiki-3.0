#ifndef __SX1278_A_H__
#define __SX1278_A_H__

#include "sx1278.h"

typedef struct {
  int fp; //��Ƶ
  int pv; //���书��
  int sf; //��Ƶ����
  int cr; //��Ч���ݱ�
  int ps; //ǰ���볤��
  int bw; //����
  int hop; //��Ƶ
  int hop_tab[HOP_TAB_SIZE];
#if WITH_xLab
  int id;
#endif
}sx1278_config_t;

extern sx1278_config_t  sx1278Config;

void LoraSetFP(int fp);
void LoraSetPV(int pv);
void LoraSetSF(int sf);
void LoraSetCR(int cr);
void LoraSetBW(int bw);
void LoraSetPS(int ps);
void LoraSetHOP(int hop);
void LoraSetHOPTAB(int *tab);
#if WITH_xLab              
void LoraSetID(int id);
int LoraGetID(void);
#endif

#endif
