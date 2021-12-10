/*********************************************************************************************
* �ļ���lte-ec20.c
* ���ߣ�xuzhy 2018.1.11
* ˵����lte gsm ģ������
*       
*       
*       
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
#include <contiki.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gsm.h"
#include "sys/ctimer.h"
#include "dev/leds.h"

#define DEBUG 1

#if DEBUG
#define Debug      printf
#define DebugAT    printf
#else
#define Debug(...)
#define DebugAT(...)
#endif

#define _RESPONSE_BUF_SIZE 1152  //atָ����ջ��泤��

PROCESS(gsm, "gsm");



/*gsm�ײ�ӿں���*/
extern void gsm_uart_write(char *buf, int len);
extern void gsm_inf_init(void);
extern int gsm_uart_read_ch(void);

/* �ϲ�tcp�ص�����*/
void on_gsm_tcp_open(int id, int err);
void on_gsm_tcp_recv(int id, int len, char *buf);
void on_gsm_tcp_close(int id);

/* gsm_info ����gsm��ǰ״̬��Ϣ*/
gsm_info_t gsm_info;

static int gsm_response_mode = 0;
static char  _response_buf[_RESPONSE_BUF_SIZE];
static int _response_offset = 0;


/* atָ������ṹ */
struct _at_request{
  void *req;
  void (*fun)(char * response);
  unsigned int timeout_tm;
  struct _at_request *next;
};

struct _at_request *current_at = NULL; //���浱ǰ����ִ�е�atָ��
struct _at_request * list_at = NULL;  //����atָ�������б�


static void check_sim_card(void);

/*********************************************************************************************
* ���ƣ�_request_at_1
* ���ܣ�����atָ������������������Ͷ���
* ������at ����Ҫִ�е�atָ��
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void _request_at_1(char *at)
{
  _request_at_3(at, 1000, NULL);
}

/*********************************************************************************************
* ���ƣ�_request_at_1
* ���ܣ�����atָ�����ʱ���ͽ������ص�����
* ������at����Ҫִ�е�atָ��
*       to��ָ��ִ�г�ʱʱ�䣬��λ ms
*       fun���������ص�����
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
int _request_at_3(char *at, unsigned int to, void (*fun)(char*))
{
  int len = strlen(at);
  char *p = malloc(len+1);
  if (p) strcpy(p, at);
  else return -1;
  struct _at_request *pa = malloc(sizeof (struct _at_request));
  if (pa) {
    pa->req = p;
    pa->next = NULL;
    pa->fun = fun;
    pa->timeout_tm = to; //��¼��ʱʱ��
    if (list_at == NULL) list_at = pa;
    else {
      struct _at_request *pl = list_at;
      while (pl->next != NULL) pl = pl->next;
      pl->next = pa; 
    }
  } else {
    free(p);
    return -1;
  }
  return 0;
}
static char HEX2I(char x)
{
  if (x>='0' && x<='9') return x - '0';
  else if (x>='a' && x<='f') return 10+(x-'a');
  else return 10+(x-'A');
}
/*********************************************************************************************
* ���ƣ�end_process_at
* ���ܣ�atָ�������ͷ��ڴ�
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
static void end_process_at(void)
{
  if (current_at != NULL) {
    free(current_at->req);
    free(current_at);
    current_at = NULL;
  }
}
////////////////////////////////////////////////////////////////////////////////

static void _rp_check_gsm(char *rsp)
{
  if (rsp == NULL) {
   //_request_at_3("at\r\n", 3000, _rp_check_gsm);
  } else if (memcmp(rsp, "OK", 2) == 0) {
    gsm_info.gsm_status = 1;
  }
}
/*********************************************************************************************
* ���ƣ�check_gsm
* ���ܣ����gsmģ�������Ƿ�����
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
static void check_gsm(void)
{
   _request_at_3("at\r\n", 3000, _rp_check_gsm);
}


static void _rs_check_sim_card(char *rsp)
{
  if (rsp == NULL) {
    return;
  } 
  else if (memcmp(rsp, "OK", 2)==0) {
     gsm_info.gsm_status = 2;
     gsm_info.simcard_status = 1;
  } 
  
}
/*********************************************************************************************
* ���ƣ�check_sim_card
* ���ܣ�����Ƿ��в���sim��
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void check_sim_card(void)
{
   _request_at_3("at+cfun=1\r\n", 5000, _rs_check_sim_card);
  //_request_at_3("at*tsimins?\r\n", 3000, _rs_check_sim_card);
}


static void _rs_register_network(char *rsp)
{
  if (rsp != NULL && memcmp(rsp, "+COPS:", 6) == 0) {
    //+COPS: 0,2,"46000"
    char *p = strchr(rsp, '\"');
    if (p != NULL) {
      p = p + 1;
      char *e = strchr(p, '\"');
      if (e) {
        *e = '\0';
        if (strlen(p) > 0) {
          strcpy(gsm_info.network, p);
        }
      }
    }
  }
}

/*********************************************************************************************
* ���ƣ�gsm_register_network
* ���ܣ���⵱ǰע�ᵽ������
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void gsm_register_network(void)
{
  
  //_request_at_3("at+cereg?\r\n", 5000, _rs_register_network);
  _request_at_3("at+cops?\r\n", 5000, _rs_register_network);
}



void _rs_gsm_call_accept(char *res)
{
  if (res == NULL) {
    Debug(" error : gsm mast not timeout whic ruan ata\r\n");
    return;
  }
  if (memcmp(res, "OK", 2) == 0) {
    gsm_info.phone_status = PHONE_ST_TALK;
  } 
  if (memcmp(res, "ERROR", 5) == 0) {
    gsm_info.phone_status = PHONE_ST_IDLE;
  }/*
  if (memcmp(res, "NO CARRIER", 10) == 0) {
    gsm_info.phone_status = PHONE_ST_IDLE;
  }*/
}
/*********************************************************************************************
* ���ƣ�gsm_call_accept
* ���ܣ������绰
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void gsm_call_accept(void)
{
  if (gsm_info.phone_status == PHONE_ST_RING) {
    _request_at_3("ata\r\n", 1000, _rs_gsm_call_accept);
  }
}


void _rs_gsm_call_hangup(char *rsp){
  gsm_info.phone_status = PHONE_ST_IDLE;
}
/*********************************************************************************************
* ���ƣ�gsm_call_hangup
* ���ܣ��Ҷϵ绰
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void gsm_call_hangup(void)
{
  _request_at_3("ath\r\n", 1000, _rs_gsm_call_hangup);
}


void _rs_gsm_request_call(char *rsp)
{
  if (rsp == NULL) {
    Debug("error : gsm must not timeout when run atd\r\n");
    return;
  }
  if (memcmp(rsp, "OK", 2) == 0) {
    gsm_info.phone_status = PHONE_ST_COUT;
  }
  if (memcmp(rsp, "ERROR", 5) == 0) {
    gsm_info.phone_status = PHONE_ST_IDLE;
  } /*
  if (memcmp(rsp, "NO CARRIER", 10) == 0) {
    gsm_info.phone_status = PHONE_ST_IDLE;
  }
  if (memcmp(rsp, "BUSY", 4) == 0) {
    gsm_info.phone_status = PHONE_ST_IDLE;
  }*/
}

/*********************************************************************************************
* ���ƣ�gsm_request_call
* ���ܣ�����
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
int gsm_request_call(char *pnum)
{
  char buf[32];
  sprintf(buf, "atd%s;\r\n", pnum);
  if (gsm_info.phone_status == PHONE_ST_IDLE) {
    strcpy(gsm_info.phone_number, pnum);
    _request_at_3(buf, 2000, _rs_gsm_request_call);
    return 1;
  }
  else return 0;
}

void _rs_gsm_get_current_call(char *rsp)
{
  if (memcmp(rsp, "OK", 2) == 0) {
  }
  if (memcmp(rsp, "+CLCC", 5) == 0) {
    //+CLCC: 1,0,0,0,0,"10010",129
    if (strlen(rsp)>12) {
      if (rsp[11] == '0') {
        gsm_info.phone_status = PHONE_ST_TALK;
      }
    }
  }
}

/*********************************************************************************************
* ���ƣ�gsm_get_current_call
* ���ܣ���ȡ��ǰͨ��״̬
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void gsm_get_current_call(void)
{
  _request_at_3("at+clcc\r\n", 1000, _rs_gsm_get_current_call);
}

/*********************************************************************************************
* ���ƣ�gsm_ppp_init
* ���ܣ��������ӳ�ʼ��
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void gsm_ppp_init(void)
{
  void gsm_ppp_stats();
  char *apn = "cmnet";
  char buf[32];
  if (memcmp(gsm_info.network, "46000", 5) == 0) {
    apn = "cmnet";
  }
  if (memcmp(gsm_info.network, "46001", 5) == 0) {
    apn = "3gnet";
  }
  if (memcmp(gsm_info.network, "46011", 5) == 0 ) { //�й����ţ�
    apn = "";
  }
  sprintf(buf, "at+qicsgp=1,1,\"%s\"\r\n", apn);
  _request_at_3(buf, 3000, NULL);
  
  gsm_info.ppp_status = 1;
  //_request_at_3("at+zpppstatus\r\n", 3000, NULL);
  //_request_at_3("at+qiact?\r\n", 3000, NULL);
  gsm_ppp_stats();
}

/*********************************************************************************************
* ���ƣ�gsm_ppp_stats
* ���ܣ����ppp����״̬
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void gsm_ppp_stats()
{
  _request_at_3("at+qiact=1\r\n", 3000, NULL);
  _request_at_3("at+qiact?\r\n", 3000, NULL);
}

void _rs_gsm_get_imei(char *rsp)
{
  if (rsp == NULL) {
    Debug("error: get imei not timeout\r\n");
    return;
  }
  if (memcmp(rsp, "+CGSN:", 6) == 0) { 
    strcpy(gsm_info.imei, &rsp[6]);
  }
}

/*********************************************************************************************
* ���ƣ�gsm_get_imei
* ���ܣ���ȡģ��imei
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void gsm_get_imei(void)
{
  _request_at_3("at+cgsn=1\r\n", 1000, _rs_gsm_get_imei);
}

static char short_message[72];
static void _rs_gsm_send_message(char *rsp)
{
  if (rsp == NULL) {
     Debug("error: send message timeout\r\n");
    return;
  }
  if (rsp[0] == '>')  {
    gsm_uart_write(short_message, strlen(short_message));
    char ch = 0x1A;
    gsm_uart_write(&ch, 1);
    DebugAT("\r\nGSM <<< %s\r\n", short_message);
  }
}

/*********************************************************************************************
* ���ƣ�gsm_send_message
* ���ܣ����Ͷ���Ϣ
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void gsm_send_message(char *num, char *msg)
{
  char buf[32];
  sprintf(buf, "at+cmgs=\"%s\"\r", num);
  sprintf(short_message, msg);
  _request_at_3(buf, 5000, _rs_gsm_send_message);
}

void _rs_gsm_query_creg(char *rsp)
{
  /* +CREG: 2, 1, "712C", "DF6A", 0 */
  if (rsp == NULL) return;
  if (memcmp(rsp, "+CREG:", 6) == 0) {
    if (rsp[7] != '2') {
      _request_at_3("at+creg=2\r\n", 1000, NULL);
      return;
    } else {
      if (strlen(rsp) > 14) {
        char *lac = &rsp[14];
        char *p = strchr(lac, '\"');
        *p = '\0';
        char *ci = p+4;
        p = strchr(ci, '\"');
        *p = '\0';
        char *act = p + 3;
        //printf(" got lac: %s, ci: %s, act:%s\r\n", lac, ci, act);
        strcpy(gsm_info.lac, lac);
        strcpy(gsm_info.ci, ci);
        strcpy(gsm_info.act, act);
      }
    }
  }
}
/*********************************************************************************************
* ���ƣ�gsm_query_creg
* ���ܣ���ѯ��ǰ����ע��״̬
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void gsm_query_creg(void)
{
  _request_at_3("at+creg?\r\n", 5000, _rs_gsm_query_creg);
}

static void _rs_gsm_query_cereg(char *rsp)
{
  if (rsp == NULL) {
    return;
  }
  if (memcmp(rsp, "+CEREG:", 7) == 0) {
    char *p = &rsp[7];
    char *pst = strchr(p, ',');
    if (pst != NULL) {
      pst = pst + 1;
      int st = atoi(pst);
      if (st == 1 || st == 5){
        gsm_info.ppp_status = 2;
      } else {
        gsm_info.ppp_status = 1;
      }
    }
    rsp[0] = '\0'; //
  }
}
void gsm_query_cereg(void)
{
  _request_at_3("at+cereg?\r\n", 5000, _rs_gsm_query_cereg);
}

static void _rs_gsm_query_cgatt(char *rsp)
{
    if (rsp == NULL) {
    return;
  }
  if (memcmp(rsp, "+CGATT:", 7) == 0) {
    char *p = &rsp[7];
    int st = atoi(p);
    if (st == 1) {
       gsm_info.ppp_status = 2;
    } else {
       gsm_info.ppp_status = 0;
    }
  }
}
void gsm_query_cgatt(void)
{
  _request_at_3("at+cgatt?\r\n", 5000, _rs_gsm_query_cgatt);
}

static void __on_ec20_at_port(char *rsp)
{
  //+QURCCFG: "urcport","usbat"
  if (memcmp(rsp, "+QURCCFG:", 9) == 0) {
    char *p = strchr(&rsp[9], ',');
    if (p != NULL) {
      if (memcmp(p+1, "\"uart1\"", 5) != 0) {
        /* ����Ĭ��atָ���ϱ�Ϊ���� */
        _request_at_3("at+qurccfg=\"urcport\",\"uart1\"\r\n", 1000, NULL);
      }
    }
  }
}
/*********************************************************************************************
* ���ƣ�ec20_at_port_init
* ���ܣ�ec20 ����at�ϱ�״̬Ϊ����
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void ec20_at_port_init(void)
{
  //_request_at_3("at+qurccfg=\"urcport\",\"uart1\"\r\n", 1000, NULL);
  _request_at_3("at+qurccfg?\r\n", 2000, __on_ec20_at_port);
}

static struct ctimer gsm_timer;

/*********************************************************************************************
* ���ƣ�e_gsm_timer_call
* ���ܣ���ʱ���ص�����������gsmģ��״̬
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
static void _gsm_timer_call(void *ptr)
{
  static int tick = 0;
  tick ++;
  if (gsm_info.gsm_status == 0) {
    if (tick % 3 == 0) {
      check_gsm();
    }
  }
  if (gsm_info.gsm_status == 1) {
    static int init = 0;
    if (init == 0) {
       _request_at_3("AT+NRB\r\n",5000, NULL);
       gsm_info.gsm_status = 0;
       init = 1;
       
    }else if (init == 1) {
      _request_at_3("at\r\n", 1000, NULL);
      _request_at_3("at\r\n", 1000, NULL);
      _request_at_3("at\r\n", 1000, NULL);
      _request_at_3("ate0\r\n", 1000, NULL);
      _request_at_3("ati\r\n", 1000, NULL);
      _request_at_3("at+cgmi\r\n", 1000, NULL);
      _request_at_3("at+cgmm\r\n", 1000, NULL);
      _request_at_3("AT+CGMR\r\n", 1000, NULL);
      _request_at_3("AT+NCONFIG?\r\n", 5000, NULL);

      init = 2;
    }
    if (init == 2 && tick % 5 == 0) {
      check_sim_card();
    }
  }
  if (gsm_info.gsm_status == 2) {
    static int f = 0;
    if (f == 0) {
       
      gsm_get_imei();

      _request_at_3("at+nband?\r\n", 1000, NULL);

      _request_at_3("at+nuestats\r\n", 3000, NULL);
      
      _request_at_3("at+cereg=2\r\n", 1000, NULL);
      //_request_at_3("at+cops=0,2\r\n", 1000, NULL);
      _request_at_3("at+cops=1,2,\"46011\"\r\n", 1000, NULL);
      
      _request_at_3("at+qregswt=1\r\n", 1000, NULL);
      
        f = 1;
    }
    if (f == 1) {
      _request_at_3("at+cgatt=1\r\n", 1000, NULL);
      f = 2;
    }
    if (tick % 5 == 1 && strlen(gsm_info.network) == 0) {
      //gsm_register_network();
    }
    if (gsm_info.ppp_status == 0 && tick % 5 == 0) {
      //gsm_info.ppp_status = 1;
      //gsm_query_cereg();
      gsm_query_cgatt();
      _request_at_3("at+csq\r\n", 1000, NULL);
    } 
    if (gsm_info.ppp_status == 0 && tick % 1 == 0) {
      static int flag = 0;
      if (flag == 0) {
        //leds_off(2);
        flag = 1;
      } else {
        //leds_on(2);
        flag = 0;
      }
    } else  if (gsm_info.ppp_status == 2 ){
      //leds_on(2);
    }
    
    if (strlen(gsm_info.network) > 0) {
   
    }
    if (tick % 60 == 15) {
      //gsm_query_creg();
      //_request_at_3("at+nuestats\r\n", 1000, NULL);
      //_request_at_3("at+cops?\r\n", 1000, NULL);
      //gsm_query_cgatt();
      //_request_at_3("at+cgatt?\r\n", 1000, NULL);
      //_request_at_3("at+nping=119.23.162.168\r\n", 3000, NULL);
    }
    if (gsm_info.phone_status == PHONE_ST_COUT) {
      gsm_get_current_call();
    }
  }
  if (gsm_info.gsm_status == 3) {
    //��sim��
    if (tick % 5 == 0) check_sim_card();
  }
  ctimer_reset(&gsm_timer);
}

/*********************************************************************************************
* ���ƣ�_poll_request
* ���ܣ�at������ѯ
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void _poll_request(void)
{
  if (current_at == NULL) {
    current_at = list_at;
    if (current_at != NULL) {
      DebugAT(" gsm <<< %s", (char*)current_at->req);
      if (strstr(current_at->req, "zipsend")) {
        DebugAT("\n");
      }
      
      current_at->timeout_tm = clock_time()+ current_at->timeout_tm;
      gsm_uart_write(current_at->req, strlen(current_at->req));
      list_at = list_at->next;
    }
  } else {
    unsigned int tm = clock_time();
    if (((int)(tm - current_at->timeout_tm)) > 0) {
      DebugAT(" gsm : timeout\r\n");
      if (current_at->fun != NULL) {
        current_at->fun(NULL);
      }
      end_process_at();
    } 
  }
}
/*********************************************************************************************
* ���ƣ�_poll_response
* ���ܣ�����at��Ӧ
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void _poll_response(void)
{
  int ch;
  ch = gsm_uart_read_ch();
  while (ch >= 0) {
    if (gsm_response_mode == 0) {
      if (_response_offset == 0 && ch == '>') {
        if (current_at != NULL && current_at->fun != NULL) {    
          current_at->fun(">");
        }
      } else
      if (ch != '\n') {
        _response_buf[_response_offset++] = ch;
      } else {
        if (_response_offset>0 && _response_buf[_response_offset-1] == '\r') {
          _response_buf[_response_offset-1] = '\0';
          _response_offset -= 1;
          if (_response_offset > 0) {
            DebugAT(" gsm >>> %s\r\n", _response_buf);
            if (current_at != NULL && current_at->fun != NULL) {
                current_at->fun(_response_buf);
            }
            if ((memcmp(_response_buf, "OK", 2) == 0) 
                ||(memcmp(_response_buf, "ERROR", 5) == 0)
                || (memcmp(_response_buf, "+CME ERROR", 10) == 0)
                ||(memcmp(_response_buf, "SEND OK", 7) == 0))
            {
                end_process_at();
            }
              /* unsolite message */
              if (memcmp(_response_buf, "+CSQ:", 5) == 0) {
                  char *p = &_response_buf[6];
                  char *e = strchr(p, ',');
                  *e = '\0';
                  int v = atoi(p);
                  if (v == 99) v = 0;
                  gsm_info.signal = v/6;
              }
              if (memcmp(_response_buf, "+CLIP: ", 7) == 0) {
                //+CLIP: "13135308483",161,"","",0
                gsm_info.phone_status = PHONE_ST_RING;
                char *p = &_response_buf[8];
                char *e = strchr(p, '\"');
                *e = '\0';
                strcpy(gsm_info.phone_number, p);
              }
              if (memcmp(_response_buf, "NO CARRIER", 10) == 0) {
                gsm_info.phone_status = PHONE_ST_IDLE;
              }
              if (memcmp(_response_buf, "BUSY", 4) == 0) {
                gsm_info.phone_status = PHONE_ST_IDLE;
              }
            
              //bc95 
              //+CEREG:1,8F25,8F65153,7
              if (memcmp(_response_buf, "+CEREG:", 7) == 0) {
                char *p = &_response_buf[7];
                int st = atoi(p);
          
                if (st == 1 || st==5) {
                  //gsm_info.ppp_status = 2;
                } else {
                  //gsm_info.ppp_status = 1;
                }
              }
            if (memcmp(_response_buf, "+NSONMI:", 8) == 0) {
              void bc95_on_udp_data_notify(int sid, int len);
              char *p = &_response_buf[8];
              int sid = atoi(p);
              p = strchr(p, ',');
              p += 1;
              int len = atoi(p);
              bc95_on_udp_data_notify(sid, len);
            }
            if (memcmp(_response_buf, "+NNMI", 5) == 0) {
              void bc95_coap_on_data_notify(char *p, int len);
              char *p = strchr(_response_buf, ',');
              if (p != NULL) {
                *p ++ = '\0';
                int len = atoi(&_response_buf[6]);
                for (int i=0; i<len; i++) {
                  p[i] = HEX2I(p[i*2])<<4 | HEX2I(p[i*2+1]);
                }
                bc95_coap_on_data_notify(p, len);
              } /*else {
                void bc95_coap_on_data_notify(void);
                bc95_coap_on_data_notify();
              }*/
            }
            _response_offset = 0;
          }
        }
      }
    }
    
    /**/
    ch = gsm_uart_read_ch();
  }
}
//////////////////////////////////////////////////////

#include "shell.h"
PROCESS(shell_gsm_process, "gsm shell process");
SHELL_COMMAND(gsm_command, "gsm", "gsm: gsm operator",
	      &shell_gsm_process);
PROCESS_THREAD(shell_gsm_process, ev, data)
{
  
  PROCESS_BEGIN();
  
  //printf("gsm data %s\r\n", data);
  char buf[64];
  if (strlen(data) > 0) {
    sprintf(buf, "%u\r\n", data);
    _request_at_3(buf, 1000, NULL);
  } else 
    gsm_send_message("13135308483", "abc");
  PROCESS_EXIT();
  
  PROCESS_END();
}

PROCESS_THREAD(gsm, ev, data)
{
  static struct etimer et;
  static int led2 = 0;
  static int led_t = 0;
  PROCESS_BEGIN();

  gsm_inf_init();
  
  //event_at_finish = process_alloc_event();
  
  etimer_set(&et, CLOCK_SECOND); // 1000 ms
  PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER); 
  
  //mytimer_set(&gsm_timer, _gsm_timer_call, NULL);
  //mytimer_add(&gsm_timer, 1000);
  ctimer_set(&gsm_timer, CLOCK_SECOND, _gsm_timer_call, NULL);
  shell_register_command(&gsm_command);
  while (1) {
    _poll_request();
    _poll_response();
    if (gsm_info.gsm_status == 0){led2 = 0;}
    else if (gsm_info.simcard_status != 1) {
      led_t += 1;
      if (led_t % 50 == 0) {
        led2 = !led2;
        led_t = 0;
      }
    }else if (gsm_info.ppp_status != 2) {
      led_t += 1;
      if (led_t % 16 == 0) {
        led2 = !led2;
        led_t = 0;
      }
    } else {
      led2 = 1;
    }
    if (led2 != 0) {
      leds_on(2);  
    } else {
      leds_off(2);
    }
    etimer_set(&et, CLOCK_SECOND/100);
    PROCESS_YIELD();
  }
  PROCESS_END();
}