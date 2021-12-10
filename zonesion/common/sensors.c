/*********************************************************************************************
*�ļ�: sensors.c
*����: zonesion   2015.12.18
*˵��: ���ش������������
*�޸�:
*ע��:
*********************************************************************************************/
#include <contiki.h>
#include <dev/htu21d.h>
#include <dev/bh1750.h>
#include "system_stm32f4xx.h"
/*********************************************************************************************
 ���������
*********************************************************************************************/
PROCESS(sensors, "sensors");
/*********************************************************************************************
 ����ȫ�ֱ���
*********************************************************************************************/
float gLight, gTemp, gHumi;
uint16_t gADC1, gADC2, gADC3, gADC4;
process_event_t process_event_key, _evt_check_key;
static struct etimer sensors_timer;
/*********************************************************************************************
 ��������
*********************************************************************************************/
extern void pm25_read_byte(char c);
void uart3_putc(unsigned char x);
void uart3_init(unsigned int bpr);
void uart3_set_input();
void adc_init(void);
void KeySetCallback(void (*f)(int k));
uint16_t adc_get(uint8_t ch);

/*********************************************************************************************
* ����: PROCESS_THREAD()
* ����: �������������
* ����: sensors, ev, data
* ����: ��
* �޸�:
* ע��:
*********************************************************************************************/
PROCESS_THREAD(sensors, ev, data)
{
  PROCESS_BEGIN();

  uart3_init(9600);
  uart3_set_input(pm25_read_byte);

  BH1750GPIOInit();                                             //����I2C��ʼ��
  HTU21DGPIOInit();                                             //��ʪ��I2C��ʼ��
  adc_init();                                                    //adc��ʼ��

  process_event_key = process_alloc_event();
  _evt_check_key = process_alloc_event();

  htu21d_reset();

  bh1750_write_cmd(0x01);                                       //power on
  etimer_set(&sensors_timer, 10*CLOCK_SECOND/1000);
  PROCESS_YIELD();
  PROCESS_WAIT_UNTIL(ev == PROCESS_EVENT_TIMER);

  bh1750_write_cmd(0x07);                                       //reset
  etimer_set(&sensors_timer, 10*CLOCK_SECOND/1000);
  PROCESS_YIELD();
  PROCESS_WAIT_UNTIL(ev == PROCESS_EVENT_TIMER);

  bh1750_write_cmd(0x10);                                       //H- resolution mode
  etimer_set(&sensors_timer, 180*CLOCK_SECOND/1000);

  while (1) {
    static unsigned int tick = 0;
    PROCESS_YIELD();
    if(etimer_expired(&sensors_timer)) {
      /* �������� */
      if (tick % 10 == 0) {
        gLight = bh1750_get();
      }
      /* ��ʪ�ȴ��� */
      if (tick % 10 == 1) {
        htu21d_mesure_t();
      }
      if (tick % 10 == 2) {
          gTemp = htu21d_t();
      }
      if (tick % 10 == 3) {
        htu21d_mesure_h();
      }
      if (tick % 10 == 4) {
        gHumi = htu21d_h();
      }
      /* adc ���� */
      if (tick % 10 == 5) {
         gADC1 = adc_get(1);
      }
      if (tick % 10 == 6) {
        gADC2 = adc_get(2);
      }
      if (tick % 10 == 7) {
        gADC3 = adc_get(3);
      }
      if (tick % 10 == 8) {
        gADC4 = adc_get(4);
      }
      etimer_set(&sensors_timer, 100*CLOCK_SECOND/1000);
      tick ++;
    }
  }
  PROCESS_END();
}