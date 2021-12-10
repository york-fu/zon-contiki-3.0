/*********************************************************************************************
* �ļ���lcd.c
* ���ߣ�zonesion 2017.02.17
* ˵����LCD��ʾ����ʵ����Ϣ����
* �޸ģ�Chenkm 2017.02.17 ���ע��
* ע�ͣ�
*
*********************************************************************************************/
/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include <stdio.h>
#include <contiki.h>
#include <dev/leds.h>
#include "rf/zigbee-net.h"
#include "ili93xx.h"
#include "zxbee.h"
#if (DEV_TYPE==3)
#include "wifi_cfg.h"
#endif

PROCESS(lcd, "lcd");

/*********************************************************************************************
* ȫ�ֱ���
*********************************************************************************************/
extern float gLight, gTemp, gHumi;
extern uint16_t gADC1, gADC2, gADC3, gADC4;
extern unsigned int PM1_0, PM2_5, PM10;
extern process_event_t process_event_key;

/*********************************************************************************************
* �ⲿԭ�ͺ���
*********************************************************************************************/
//static void page1(int);
static void page2(int);
extern void page3(int);
static struct etimer lcd_timer, led_timer, led3_timer;
static void (*pages[])(int f) = {page2, page3};
void fsmc_init(void);

/*********************************************************************************************
* ����:page1()
* ����:��һ����ʾ
* ����:int f -- >0��ʾ
* ����:��
* �޸�:
* ע��:
*********************************************************************************************/
void page1(int f)
{
  if (f) {
    LCDClear(0xffff);

    LCDDrawFnt24(4, 2, "����Ѷ���人���Ƽ����޹�˾", 0x0000, -1);

    LCDrawLineH(0, 320, 29, 0x07e0);

    LCDrawLineH(0, 320, 211, 0x07e0);

    LCDDrawFnt24(46, 213, "www.zonesion.com.cn", 0x0000, -1);
  }
}
/*********************************************************************************************
* ����:page2()
* ����:�ڶ�����ʾ
* ����:int f -- >0��ʾ
* ����:��
* �޸�:
* ע��:
*********************************************************************************************/
void page2(int f)
{
  if (f) {
    char buf[64];

    LCDClear(0xffff);

    LCDDrawFnt24(76, 2, "����������ƽ̨", 0x0000, -1);

    LCDrawLineH(0, 320, 29, 0x07e0);

#if (DEV_TYPE==1)
    char* t[] = {"", "Э����","·����","�ն˽ڵ�"};
    LCDDrawFnt16(10, 40, "  Э��ջ: ZigBee2007/PRO", 0xf800, -1);
    sprintf(buf,         "�����ʶ: %u", zigbee_panid());
    LCDDrawFnt16(10, 65, buf, 0xf800, -1);
    sprintf(buf,         "    �ŵ�: %u", zigbee_channel());
    LCDDrawFnt16(10, 90, buf, 0xf800, -1);
    sprintf(buf,         "�ڵ�����: %s", t[zigbee_type()+1]);
    LCDDrawFnt16(10, 115,  buf, 0xf800, -1);
    sprintf(buf,         "�ڵ��ַ: %s", zigbee_mac());
    LCDDrawFnt16(10, 140,  buf, 0xf800, -1);
#elif (DEV_TYPE==4)
    LCDDrawFnt16(10, 40, "  Э��ջ: BLE", 0xf800, -1);
    sprintf(buf,         "�ڵ��ַ: %s", zigbee_mac());
    LCDDrawFnt16(10, 140,  buf, 0xf800, -1);
    LCDrawLineH(0, 320, 211, 0x07e0);
#elif (DEV_TYPE==3)
    LCDDrawFnt16(10, 40, "  Э��ջ: WIFI", 0xf800, -1);
    sprintf(buf,         "    SSID: %s", Z_SSID_NAME);
    LCDDrawFnt16(10, 65, buf, 0xf800, -1);
    sprintf(buf,         " GW_PORT: %u", GW_PORT);
    LCDDrawFnt16(10, 90, buf, 0xf800, -1);
    sprintf(buf,         " LO_PORT: %u", LO_PORT);
    LCDDrawFnt16(10, 115,  buf, 0xf800, -1);
    sprintf(buf,         "�ڵ��ַ: %s", zigbee_mac());
    LCDDrawFnt16(10, 140,  buf, 0xf800, -1);
    LCDrawLineH(0, 320, 211, 0x07e0);
#endif
    LCDDrawFnt24(58, 213, "www.zhiyun360.com", 0x0000, -1);
  }
}
/*********************************************************************************************
* ����:PROCESS_THREAD()
* ����:���̲���
* ����:
* ����:��
* �޸�:
* ע��:
*********************************************************************************************/
PROCESS_THREAD(lcd, ev, data)
{
  PROCESS_BEGIN();
  BLInit();
  BLOnOff(1);
  fsmc_init();
  etimer_set(&lcd_timer, 50*CLOCK_SECOND/1000);
  PROCESS_WAIT_UNTIL(ev == PROCESS_EVENT_TIMER);
  ILI93xxInit();


  etimer_set(&lcd_timer, 1);
  while (1) {
    static unsigned int st = 4;
    static char dir = 1;

    if (etimer_expired(&lcd_timer) || ev==process_event_key) {
      static int tick = 0;
      static int cur_idx = 0;
      int page_delay = 5;
      if (tick % page_delay == 0 || ev==process_event_key) {
        cur_idx += 1;
        if (cur_idx >= (sizeof pages /sizeof pages[0])) cur_idx = 0;
        pages[cur_idx](1);
        tick = 1;
      } else if (tick % page_delay != 0) {
        pages[cur_idx](0);
        tick++;
      }
      etimer_set(&lcd_timer, CLOCK_SECOND);
    }
    /* ��ɫ�� */
    if (etimer_expired(&led3_timer)) {
      leds_off(st);
      if (dir) {
        st <<= 1;
        if (st > 0x10) st=0x08, dir=0;
      } else {
        st >>= 1;
        if (st < 0x04) st=0x08, dir=1;
      }
      leds_on(st);
      etimer_set(&led3_timer, CLOCK_SECOND/10);
    }
    /* ����ָʾ�� */
    if (etimer_expired(&led_timer)) {
      static unsigned char s[] = {1,  0,  1,  0,   1,   0,   0};
      static unsigned int t[] = {200,500, 100,100,100,  100, 2000};
      static int idx = 0;
      idx++;
      if (idx == 7) idx = 0;
      if (s[idx]) leds_on(1);
      else leds_off(1);
      etimer_set(&led_timer, t[idx]*CLOCK_SECOND/1000);
    }
    PROCESS_YIELD();
  }
  PROCESS_END();
}