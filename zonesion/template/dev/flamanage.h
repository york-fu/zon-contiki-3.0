#ifndef __FLAMANAGE_H__
#define __FLAMANAGE_H__

void FlaFormat(void);

int FlaManageInit(void);

int FlaVPageRead(unsigned vpg, unsigned off, void *buf, int len);
int FlaVPageWrite(unsigned vpg, unsigned off, void *buf, int len);

void FlaUnMapvPage(int vpg);

#define DbgNul  0
#define DbgErr  1
#define DbgWan  2
#define DbgInf  3
#define DbgOutLv DbgInf
#define DebugInfo(lv, x...) do { \
        if (lv <= DbgOutLv) {    \
            char *DbgMsg[] = {    \
                NULL, "Err : ", "Wan : ", "Info : ", \
            };            \
            printf("\r\n%s %s %s() : ", DbgMsg[lv], MODULE_FLAG, __FUNCTION__); \
            printf(x);         \
        }          \
    } while (0)
#endif
