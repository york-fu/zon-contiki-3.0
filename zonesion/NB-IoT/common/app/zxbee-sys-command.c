/*********************************************************************************************
* �ļ���zxbee-sys-command.c
* ���ߣ�xuzhy 2018.1.12
* ˵����lte ģ�鹫��zxbeeָ���ģ��
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
#include "gsm.h"
#include "usart.h"
#include "at.h"
#include "sensor.h"

static int action = 0; //
static char phone[16];
static char msg[128];




/*********************************************************************************************
* ���ƣ�on_command_begin
* ���ܣ�zxbeeָ���ʼ����
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void on_command_begin()
{
  action = 0;
}
/*********************************************************************************************
* ���ƣ�on_command_end
* ���ܣ�zxbeeָ�����ϵ���
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void on_command_end()
{
  if (action == 1) {
    /*����绰*/
    gsm_request_call(phone);
  }
  if (action == 2) {
    gsm_send_message(phone, msg);
  }
}
/*********************************************************************************************
* ���ƣ�zxbee_sys_process_command_call
* ���ܣ�lteģ�鹫��zxbeeָ���
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
int ZXBeeSysCommandProc(char* ptag, char* pval)
{
  int ret = -1;
  if (memcmp(ptag, "ECHO", 4) == 0) {
    ZXBeeAdd(ptag, pval);
    return 1;
  }
  if (memcmp(ptag, "TYPE", 4) == 0) {
    if (pval[0] == '?') {
#if !defined(NB_IoT_Serial)
      int radio_type = CONFIG_RADIO_TYPE; //lora
      int dev_type = CONFIG_DEV_TYPE; //lora ed
      char buf[16];
      ret = sprintf(buf, "%d%d%s", radio_type, dev_type, NODE_NAME);
      ZXBeeAdd("TYPE", buf);
      return 1;
#else
      at_notify_data("AT+TYPE?\r\n",strlen("AT+TYPE?\r\n"));
#endif
    }
  }

  if (memcmp(ptag, "Action", 6) == 0) {
    if (memcmp(pval, "Tel", 3) == 0) {
      action = 1;
    }
    if (memcmp(pval, "SendSMS", 3) == 0) {
      action = 2;
    }
    return 1;
  }
  if (memcmp(ptag, "Number", 6) == 0) {
    strcpy(phone, pval);
    return 1;
  }
  if (memcmp(ptag, "Content", 7) == 0) {
    strcpy(msg, pval);
    return 1;
  }
  return ret;
}



