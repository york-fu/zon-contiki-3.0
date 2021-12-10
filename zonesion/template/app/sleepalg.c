
#include "arm_math.h"
#include "arm_const_structs.h"

/* 记录开始睡眠时间 */
unsigned int sleep_time = 0;
/* 记录深度随眠时长 单位s */
unsigned int sleep_deep = 0;

unsigned int turnover_cnt = 0;


static unsigned int sleepalg_cnt = 0;

void sleep_enter()
{
    sleep_time = 0;
    sleepalg_cnt = 0;
    turnover_cnt = 0;
}

void sleep_leave()
{
}

/*
 * 频率分辨率 0.390625Hz 范围 0 - 12.5 Hz
 *
 */
#define N       64      //采样个数
#define Fs      10      //采样频率
#define F_P     (((float)Fs)/N)


void sleepalg(float32_t* test_f32)
{
  uint32_t ifftFlag = 0;
  uint32_t doBitReverse = 1;
  float32_t testOutput[N/2];
  uint32_t i;
  
  arm_cfft_f32(&arm_cfft_sR_f32_len64, test_f32, ifftFlag, doBitReverse);

  /* Process the data through the Complex Magnitude Module for
  calculating the magnitude at each bin */
  arm_cmplx_mag_f32(test_f32, testOutput, N/2);
  //printf("\033[%d;%dH", (0), (0)) ;
  
  int sa = 0;
  float ma = 0;
  int mi = 0;
  for (i=0; i<N/2; i++) {
    float a = testOutput[i];
    if (i == 0) a = testOutput[i]/(N);
    else a = testOutput[i]/(N/2);
    if (i != 0) {
      if (a < 1) sa += 1;
      if (a > ma) {
        ma = a;
        mi = i;
      }
    }
  }
  if (sa >= N/2 - 5)  sleepalg_cnt += 1;
  else sleepalg_cnt = 0;
  
  if (sleepalg_cnt > 10*5) { //持续5分钟进入睡眠状态 
     sleep_time += 6; 
  }
  if (sleepalg_cnt > 10*20) { //持续20分钟，进入深度睡眠
      sleep_deep += 6;
  }
  if (sleep_time != 0 && sleepalg_cnt == 0 && mi >= 4 && ma >= 2) {
    turnover_cnt += 1;
  }
}