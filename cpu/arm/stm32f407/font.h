#ifndef __FONT_H__
#define __FONT_H__

struct typFNT_GB24                // ������ģ���ݽṹ
{
       char* Index;              // ������������
       char Msk[72];                      // ����������
};
struct typFNT_GB16                // ������ģ���ݽṹ
{
       char* Index;              // ������������
       char Msk[32];                      // ����������
};
extern const unsigned char nAsciiDot8x16[];
extern const unsigned char nAsciiDot12x24[];
extern const struct typFNT_GB24 GB_24[];
extern const struct typFNT_GB16 GB_16[];
extern int GB_24_SIZE;
extern int GB_16_SIZE;
#endif
