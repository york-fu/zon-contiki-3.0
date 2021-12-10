/******************************* (C) Embest ***********************************
* File Name          : flacfg.h
* Author             : Embest 
* Date               : 2010-3-3
* Version            : 0.1
* Description        : flash参数配置文件
******************************************************************************/

#ifndef __FLACFG_H__
#define __FLACFG_H__

#if 0
#define BLOCKS              64          // FLASH物理块的数量							
#define	PAGES				(BLOCKS*PAGES_PRE_BLOCK)
#define PAGES_PRE_BLOCK     256         // FLASH每块的物理页数量
#define PAGE_SIZE       	256         // FLASH页大小
#define BLOCK_SIZE  		(PAGE_SIZE*PAGES_PRE_BLOCK)  
            

/* 虚拟块个数*/
#define	VBLOCKS				63
#define VPAGES				(VBLOCKS*VPAGES_PRE_VBLOCK)
#define VPAGES_PRE_VBLOCK	254
#define VPAGE_SIZE			256
#define VBLOCK_SIZE			(VPAGE_SIZE*VPAGES_PRE_VBLOCK)
 
#define ESPECIAL_PAGES_BLOCK	(PAGES_PRE_BLOCK-VPAGES_PRE_VBLOCK)

#define FlaBlkErase(x) do { \
        printf("\r\n Info : FLA flash erase block [%d]", x);  \
        SPI_FLASH_SectorErase((x) * BLOCK_SIZE);   \
    } while(0)


extern void SPI_FLASH_SectorErase(unsigned SectorAddr);
extern void SPI_FLASH_BufferRead(void* pBuffer, unsigned ReadAddr, unsigned short NumByteToRead);
extern void SPI_FLASH_PageWrite(void* pBuffer, unsigned WriteAddr, unsigned short NumByteToWrite);

#define FlaPageRead(addr, buf, len)   SPI_FLASH_BufferRead(buf, addr, len)
#define FlaPageWrite(addr, buf, len)  \
    do {  \
        SPI_FLASH_PageWrite(buf, addr, len);      \
    }while(0)
#else 
/* m25p16 */
#define BLOCKS              32         // FLASH物理块的数量		
#define PAGES_PRE_BLOCK     256         // FLASH每块的物理页数量					
#define	PAGES				(BLOCKS*PAGES_PRE_BLOCK)
#define PAGE_SIZE           256         // FLASH页大小
#define BLOCK_SIZE  		(PAGE_SIZE*PAGES_PRE_BLOCK)  
            
/* 虚拟块个数*/
#define	VBLOCKS			30
#define VPAGES_PRE_VBLOCK	254
#define VPAGES				(VBLOCKS*VPAGES_PRE_VBLOCK)
#define VPAGE_SIZE		256
#define VBLOCK_SIZE			(VPAGE_SIZE*VPAGES_PRE_VBLOCK)
 
#define ESPECIAL_PAGES_BLOCK	(PAGES_PRE_BLOCK-VPAGES_PRE_VBLOCK)


extern void SPI_FLASH_SectorErase(unsigned SectorAddr);
extern void SPI_FLASH_BufferRead(void* pBuffer, unsigned ReadAddr, unsigned short NumByteToRead);
extern void SPI_FLASH_PageWrite(void* pBuffer, unsigned WriteAddr, unsigned short NumByteToWrite);

#define FlaPageRead(addr, buf, len)   M25Pxx_ReadBuffer(buf, addr, len)
#define FlaPageWrite(addr, buf, len)  M25Pxx_WritePage( buf,addr,  len)
#define FlaBlkErase(x) do { \
        printf("\r\n Info : FLA flash erase block [%d]", x+1);  \
        M25Pxx_SectorErase(x<<16);   \
    } while(0)

#endif
#endif
