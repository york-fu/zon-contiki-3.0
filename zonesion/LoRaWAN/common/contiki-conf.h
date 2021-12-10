/*
 * Copyright (c) 2012, Thingsquare, http://www.thingsquare.com/.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#define CCIF
#define CLIF

#include <stdint.h>
/* These names are deprecated, use C99 names. */
typedef uint8_t u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef int8_t s8_t;
typedef int16_t s16_t;
typedef int32_t s32_t;

typedef unsigned int clock_time_t;
typedef unsigned int uip_stats_t;

#define AUTOSTART_ENABLE        1

#define CLOCK_CONF_SECOND       1000

#define CONFIG_RADIO_TYPE       8
#define CONFIG_DEV_TYPE         2

/*LoRa�����ʶ*/
#define LoRaWAN_DEFAULT_DATARATE        DR_5            //DR_0,DR_1,DR_2,DR_3,DR_4,DR_5
#define LoRaWAN_DEFAULT_CHMASK          {0x0001,0,0,0,0,0}
#define LoRaWAN_ADR                     0               //0:disable, 1:enable
#define LoRaWAN_DEFAULT_CLASS           2               //0,class a, 2 class c
#define LoRaWAN_JOINEUI                 {0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7}
#define LoRaWAN_KEY                     {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}
#define LoRaWAN_AKEY                    1 //Ĭ��ʹ��������Կ�����㷨��ÿ���ڵ����Կ��һ����������簲ȫ��


/*---------------------------------------------------------------------------*/
/* ContikiMAC configuration options.
 *
 * These are typically configured on a per-platform basis.
 */

/* CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION specifies if ContikiMAC
   should optimize for the phase of neighbors. The phase optimization
   may reduce power consumption but is not compatible with all timer
   settings and is therefore off by default. */
#ifndef CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION
#define CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION 0
#endif /* CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION */


#endif /* CONTIKI_DEFAULT_CONF_H */
