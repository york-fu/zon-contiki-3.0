/*********************************************************************************************
* �ļ���stepcounting.c
* ���ߣ�zonesion
* ˵���������������
* �޸ģ�Meixin 2017.05.31 ������ע��
* ע�ͣ�
*********************************************************************************************/

/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include "arm_math.h"
#include "arm_const_structs.h"
#include <contiki.h>
#include <stdio.h>
#include "stdlib.h"
#include "arm_math.h"

void mpu9250_init(void);
int mpu9250_accel(float *x, float *y, float *z);

static long total_step_cnt = 0;                                       //�ܲ�������
/*********************************************************************************************
* ����
*********************************************************************************************/
PROCESS(stepcount, "stepcount"); 
//Ƶ�ʷֱ��� 0.390625Hz ��Χ 0 - 12.5 Hz
//�������� 6.4��

#define N       64                                              //��������
#define Fs      10                                              //����Ƶ��
#define F_P     (((float)Fs)/N)

int stepGet(void)
{
  return total_step_cnt;
}
/*********************************************************************************************
* ���ƣ�int stepcounting(float32_t* test_f32)
* ���ܣ��������㺯��
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
int stepcounting(float32_t* test_f32)
{
  uint32_t ifftFlag = 0;                                        //����Ҷ��任��־λ
  uint32_t doBitReverse = 1;                                    //��ת��־λ
  float32_t testOutput[N/2];                                    //�������
  uint32_t i; 
  arm_cfft_f32(&arm_cfft_sR_f32_len64, test_f32, ifftFlag, doBitReverse);//����Ҷ�任
  arm_cmplx_mag_f32(test_f32, testOutput, N/2);  
  float max = 0; 
  uint32_t mi = 0;  
  for (i=0; i<N/2; i++) {
    float a = testOutput[i];
    if (i == 0) a = testOutput[i]/(N);
    else a = testOutput[i]/(N/2);
    if (i != 0 && a > max && i*F_P <= 5.4f) {
        mi = i;
        max = a;
    }
  }
  if (max > 1.5) {
      int sc = 0;
      sc = (int)(mi * F_P * (1.0/Fs)*N);
      if (sc >= 3 && sc < 30) {
       return sc;
      }
  }
  return 0;
}

/*********************************************************************************************
* ���ƣ�PROCESS_THREAD(test, ev, data)
* ����: ������
* ������test -- ��ʾ��ǰ�¼������ǲ����¼�
*       ev -- ��ʶ�������¼���data -- �����¼�����ʱ��õ������Ϣ�����¼���������Ը����̴��ݵ�����
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
PROCESS_THREAD(stepcount, ev, data)
{
  static struct etimer t_acc;
  static float acc_input[64*2];
  static uint32_t acc_len = 0;
  static int step_cnt = 0;                                             //��������

  PROCESS_BEGIN();                                                      //���̿�ʼ 
  mpu9250_init();                                                       //MPU9250��ʼ�� 
  etimer_set(&t_acc, CLOCK_SECOND/10);                                  //��ʱ���¼�����
  while (1) {
    PROCESS_YIELD();                                                    //����������ȣ��ó�CPUʹ��Ȩ
    if (etimer_expired(&t_acc)) {
      etimer_set(&t_acc, CLOCK_SECOND/10);
      static char tick = 9;
      if (++tick == 10) {
        tick = 0;
           /* �Ʋ����ݴ洢*/
          if (step_cnt > 10) {
            //char buf[16];
            total_step_cnt += step_cnt;
            //sprintf(buf, "{A0=%d}",total_step_cnt);                     //��ȡ����Ĳ��������ո�ʽ����д�ɴ�
            //ble_write(buf);                                             //����д��buf
            //A0 = total_step_cnt;
            //printf(buf);
            step_cnt = 0;                                               //��������
          }
      }
      float x, y, z;
      mpu9250_accel(&x, &y, &z);                                        //��ȡxyz���������
      float a = sqrt(x*x + y*y + z*z);                                 //��ƽ����
      acc_input[acc_len*2] = a;
      acc_input[acc_len*2+1] = 0;
      acc_len ++;
      if (acc_len == 64) acc_len = 0;
        if (acc_len == 0) {           
          step_cnt += stepcounting(acc_input);          //�ܲ����ۼ�
        }
    }
  }
  PROCESS_END();                                                //���̽���
}