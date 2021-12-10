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

/*LoRa网络标识*/
#define LoRa_NET_ID             0x32    // 应用组ID：0x01~0xFE
#define LoRa_PS                 15      // 前导码长度：4---100
#define LoRa_PV                 15      // 发射功率：0---20
#define LoRa_HOP                0       // 跳频开关：0--1
#define LoRa_HOPTAB             {431,435,431,435,431,435,431,435,431,435}  // 跳频表
/* 基频*/
#define LoRa_FP                 433     // 基频

/* */
#define LoRa_SF                 8      // 扩频因子：6--12
#define LoRa_CR                 1      // 编码率：1--4对应：4/5、4/6、4/7、4/8
#define LoRa_BW                 5      // 带宽：0--9

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
