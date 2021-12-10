#ifndef _WIFI_CFG_H_
#define _WIFI_CFG_H_

#define SL_SEC_TYPE_OPEN                                (0)
#define SL_SEC_TYPE_WEP                                 (1)
#define SL_SEC_TYPE_WPA_WPA2                            (2)

//WiFi名称和密码
#define Z_SSID_NAME           "AndroidAP"                       /* AP SSID */
#define Z_SECURITY_TYPE       SL_SEC_TYPE_OPEN                  //SL_SEC_TYPE_OPEN               /* Security type (OPEN or WEP or WPA*/
#define Z_SECURITY_KEY        "12345678"                       /* Password of the secured AP */

//网关IP地址
#define IP_ADDR               "192.168.43.1"                      //网关ip192.168.43.1
#define GW_PORT               7003
#define LO_PORT               7004
#endif