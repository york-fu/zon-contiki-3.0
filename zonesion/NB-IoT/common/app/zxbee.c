/*********************************************************************************************
* �ļ���zxbee.c
* ���ߣ�xuzhy 
* ˵����
*       
*       
*      
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
#include "contiki.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stimer.h>
#include "zxbee.h"
#include "zxbee-inf.h"
#include "sensor.h"

static char wbuf[128];
/*********************************************************************************************
* ���ƣ�ZXBeeBegin
* ���ܣ�ZXBeeָ��ϳɿ�ʼ
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void ZXBeeBegin(void)
{
  wbuf[0] = '{';
  wbuf[1] = '\0';
  

  char buf[16];
  sprintf(buf, "%d%d%s", CONFIG_RADIO_TYPE, CONFIG_DEV_TYPE, NODE_NAME);
  ZXBeeAdd("TYPE", buf);
      
}
/*********************************************************************************************
* ���ƣ�ZXBeeAdd
* ���ܣ����һ�����ݵ�zxbeeָ��
* ������tag����������ʶ
        val��������ֵ
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
int ZXBeeAdd(char* tag, char* val)
{
  int offset = strlen(wbuf);
  sprintf(&wbuf[offset],"%s=%s,", tag, val);
  return 0;
}
/*********************************************************************************************
* ���ƣ�ZXBeeEnd
* ���ܣ�zxbeeָ����ӽ���
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
char* ZXBeeEnd(void)
{
  int offset = strlen(wbuf);
  wbuf[offset-1] = '}';
  wbuf[offset] = '\0';
  if (offset > 12) return wbuf;
  return NULL;
}

/*********************************************************************************************
* ���ƣ�ZXBeeDecodePackage
* ���ܣ�zxbeeָ�����
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
char* ZXBeeDecodePackage(char *pkg, int len)
{
   char *p;
  char *ptag = NULL;
  char *pval = NULL;

  if (pkg[0] != '{' || pkg[len-1] != '}') return NULL;
  
  ZXBeeBegin();

  pkg[len-1] = 0;
  p = pkg+1;
  do {
    ptag = p;
    p = strchr(p, '=');
    if (p != NULL) {
      *p++ = 0;
      pval = p;
      p = strchr(p, ',');
      if (p != NULL) *p++ = 0;
      int ret;
      ret = ZXBeeSysCommandProc(ptag, pval);
      if (ret < 0) {
#ifndef NB_IoT_Serial
        ret = ZXBeeUserProcess(ptag, pval);
#endif
      }
    }
  } while (p != NULL);
  p = ZXBeeEnd();

  return p;
}
