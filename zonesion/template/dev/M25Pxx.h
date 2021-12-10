#ifndef _M25Pxx_H
#define _M25Pxx_H
#include "stm32f4xx.h"
#include "spi.h"
#define SPI_SDI    PB_IDR_IDR4
#define WREN  0x06                                              //Set Write Enable Latch
#define WRDI  0x04                                              //Reset Write Enable Latch
#define RDID  0x9F                                              //Read Identification
#define RDSR  0x05                                              //Read Status Register
#define WRSR  0x01                                              //Write Status Register
#define READ   0x03                                             //Read data from memory
#define FAST_READ  0x0B                                         //Read data from memory
#define WRITE  0x02                                             //Program Data into memory
#define SE  0xD8                                                //Erase one sector in memory
#define BE  0xC7                                                //Erase all memory
#define Dummy_Byte  0xA5                                        //哑读字节
#define M25Pxx_PageSize   256                                   //每页的大小

extern void M25Pxx_init(void);
extern void M25Pxx_ReadBuffer(unsigned char * pBuffer, unsigned int ReadAddr, unsigned int ReadNum);
extern void M25Pxx_BlukErase(void);
extern void M25Pxx_SectorErase(unsigned int SectorAddr);
extern void M25Pxx_WritePage(unsigned char* pBuffer, unsigned int WriteAddr, unsigned int WriteNum);
extern void M25Pxx_WriteBuffer(unsigned char* pBuffer, unsigned int WriteAddr, unsigned int WriteNum);
unsigned int M25Pxx_Read_ID(void);


#endif