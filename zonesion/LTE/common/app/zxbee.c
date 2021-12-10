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

static char wbuf[128];

void on_command_begin();
void on_command_end();
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
  if (offset > 2) return wbuf;
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
  on_command_begin();           //Ϊ֧�ֲ���绰�ͷ��Ͷ��Ŷ�����zxbee�ӿ�
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
        ret = ZXBeeUserProcess(ptag, pval);
      }
    }
  } while (p != NULL);
  on_command_begin();
  p = ZXBeeEnd();

  return p;
}
