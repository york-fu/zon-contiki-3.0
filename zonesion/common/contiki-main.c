/*********************************************************************************************
* �ļ���contiki-main.c
* ���ߣ�Lixm 2017.01.20
* ˵����
* �޸ģ�Chenkm 2017.02.16 ���ע��
* ע�ͣ�
*
*********************************************************************************************/
/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include <stdio.h>
#include <contiki.h>
#include <contiki-net.h>
#include <dev/leds.h>
#include "usart.h"
/*********************************************************************************************
* ȫ�ֱ���
*********************************************************************************************/
unsigned int idle_count = 0;

/*********************************************************************************************
* �ⲿԭ�ͺ���
*********************************************************************************************/
void relay_init(void);
void uart1_init(unsigned int bpr);
void debug_init(void (*f)(char));
void serial_shell_init(void);
/*********************************************************************************************
* ����:main()
* ����:������������
* ����:��
* ����:��
* �޸�:
* ע��:
*********************************************************************************************/
void main(void)
{
  clock_init();

  leds_init();
  leds_on(1);

  uart1_init(115200);
  debug_init(uart1_putc);                                       //���ڵ������
  int debug_input(char c);
  uart1_set_input(debug_input);
  serial_shell_init();
  printf("\r\nStarting ");
  printf(CONTIKI_VERSION_STRING);
  printf(" on STM32F40x\r\n");

  relay_init();

  process_start(&etimer_process, NULL);

  ctimer_init();

#ifdef WITH_II_802154
  netstack_init();
#endif
#if 0
  printf(" Net: ");
  printf("%s\r\n", NETSTACK_NETWORK.name);
  printf(" MAC: ");
  printf("%s\r\n", NETSTACK_MAC.name);
  printf(" RDC: ");
  printf("%s\r\n", NETSTACK_RDC.name);
  printf(" RADIO: \r\n");
  char *get_arch_rime_addr(void);
  char* mac = get_arch_rime_addr();
  printf("   MAC: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n",
         mac[0], mac[1], mac[2],mac[3],mac[4],mac[5],mac[6],mac[7]);
  printf("   PANID: %u\r\n", IEEE802154_CONF_PANID);
  printf("   CHANNEL: %u\r\n", RF_CONF_CHANNEL);
#endif

#if UIP_CONF_IPV6
  queuebuf_init();
  /* tcpip��ʼ���ƶ�������������ʼ���ɹ�֮��*/
  //process_start(&tcpip_process, NULL);
#endif /* UIP_CONF_IPV6 */

#if AUTOSTART_ENABLE
  autostart_start(autostart_processes);
#endif

  while(1) {
    do {
    } while(process_run() > 0);
    idle_count++;
    /* Idle! */
    /* Stop processor clock */
    /* asm("wfi"::); */
  }
}