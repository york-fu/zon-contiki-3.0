#include "arm_math.h"
#include "arm_const_structs.h"

/*
 * 频率分辨率 0.390625Hz 范围 0 - 12.5 Hz
 *
 */
#define N       64      //采样个数
#define Fs      10      //采样频率
#define F_P     (((float)Fs)/N)



int stepcounting(float32_t* test_f32)
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
  
  float max = 0; 
  uint32_t mi = 0;
  
  for (i=0; i<N/2; i++) {
    float a = testOutput[i];
    if (i == 0) a = testOutput[i]/(N);
    else a = testOutput[i]/(N/2);
      
    //printf(" f  %f, \t A %f\r\n", i*F_P, a);
    
    if (i != 0 && a > max && i*F_P <= 5.4f) {
        mi = i;
        max = a;
    }
  }
  if (max > 1.5) {
      uint32_t sc = 0;
      sc = (int)(mi * F_P * (1.0/Fs)*N);
      if (sc >= 3) {
       return sc;
      }
  }
  return 0;
}