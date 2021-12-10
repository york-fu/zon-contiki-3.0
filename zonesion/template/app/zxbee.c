#include "contiki.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "zxbee.h"

#define BUFSIZE 128
static char wbuf[BUFSIZE];

int z_process_command_call(char *tag, char *val);

void zxbeeBegin(void)
{
  wbuf[0] = '{';
  wbuf[1] = '\0';
}
int zxbeeAdd(char* tag, char* val)
{
  int offset = strlen(wbuf);
  sprintf(&wbuf[offset],"%s=%s,", tag, val);
  return 0;
}
char* zxbeeEnd(void)
{
  int offset = strlen(wbuf);
  wbuf[offset-1] = '}';
  wbuf[offset] = '\0';
  if (offset > 2) return wbuf;
  return NULL;
}

static int _process_command_call(char* ptag, char* pval, char* obuf)
{
  int ret = -1;

  return ret;
}
void zxbee_onrecv_fun(char *pkg, int len)
{
   char *p;
  char *ptag = NULL;
  char *pval = NULL;
  //int len = strlen(pkg);
  char *pwbuf = wbuf+1;
  
  
  if (pkg[0] != '{' || pkg[len-1] != '}') return;
  
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
      /*if (process_command_call != NULL) */{
        int ret;
        ret = _process_command_call(ptag, pval, pwbuf);
        if (ret < 0) {
           ret = z_process_command_call(ptag, pval);
        }
      }
    }
  } while (p != NULL);
 
}

