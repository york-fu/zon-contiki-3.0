#include <contiki.h>
#include <stdio.h>
#include "../dev/lis3dh.h"


void acc_step_mode(void)
{
  LIS3DH_init();
  
  LIS3DH_SetODR(LIS3DH_ODR_25Hz); //25hz
  
  LIS3DH_SetAxis(LIS3DH_X_ENABLE|LIS3DH_Y_ENABLE|LIS3DH_Z_ENABLE);
  LIS3DH_SetMode(LIS3DH_NORMAL);
  
  LIS3DH_SetBLE(LIS3DH_BLE_MSB);
  
  LIS3DH_FIFOModeEnable(LIS3DH_FIFO_STREAM_MODE);
  
  LIS3DH_SetFullScale(LIS3DH_FULLSCALE_8);
}