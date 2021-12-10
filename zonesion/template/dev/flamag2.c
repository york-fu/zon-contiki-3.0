/******************************* (C) Embest ***********************************
* File Name          : flamag2.c
* Author             : Embest 
* Date               : 2010-4-19
* Version            : 0.1
* Description        : flash页面管理另一个实现
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "flacfg2.h"
#include "flamanage.h"
#include "M25Pxx.h"

#define MODULE_FLAG     "FlaMag : "

/* 错误代码定义*/
#define ERR_NO_PAGE    	(-11)
#define ERR_ARGS       	(-12)

#define NOMAP     		((-1))
#define PAGE_DIRTY     	(0)



/* pgnum_t标识每一个页的逻辑页号和物理页号
 * 所有物理页号从1开始编号
 * 所有逻辑页号从1开始编号
 */
/* 所有块号从1开始编号 */
typedef unsigned char   bknum_t;
typedef unsigned short  pgnum_t;
typedef unsigned short  erasecnt_t;


#define ISInvalidVBlock(vbk) (vbk<=0||vbk>VBLOCKS)
#define ISInvalidPBlock(bk)	(bk<=0||bk>BLOCKS)
#define IsInvalidVPage(vpg)	((vpg)<=0||(vpg)>VPAGES)
#define IsInvalidPPage(ppg)	((ppg)<=0||(ppg)>PAGES)

#define _PPG2PADDR(ppg)		(((ppg)-1)*PAGE_SIZE)

#define PAGES_PER_PAGE		(PAGE_SIZE/sizeof(pgnum_t))
//#if 0
//#define DebugInfo(x, a...)   do{printf(a);printf("\r\n");}while(0)
//#define DbgInf   0
//#else
//#define DebugInfo(x, a...)  
//#define DbgInf   0
//#endif

#if 0
#define VPG2VBLK(vpg)	((vpg-1)/(VPAGES_PRE_VBLOCK>>1)%VBLOCKS+1)
#define VPG2PGIDX(vpg) 	((((vpg)-1)%(VPAGES_PRE_VBLOCK>>1)) + ((vpg>(VPAGES>>1))?(VPAGES_PRE_VBLOCK>>1):0))
#else
#define VPG2VBLK(vpg)	((vpg-1)%(VBLOCKS)+1)
#define VPG2PGIDX(vpg)	(((vpg)-1)/VBLOCKS)
#endif
#define VPG2BLKIDX(vpg)	((vpg-1)/VPAGES_PRE_VBLOCK)

#define BLKMAPOFF		(PAGE_SIZE-(sizeof(erasecnt_t)+sizeof(bknum_t)))
#define ERASECNTOFF		(PAGE_SIZE-(sizeof(erasecnt_t)))
//#if (sizeof(erasecnt_t) + sizeof(pgnum_t) + VPAGES_PRE_VBLOCK*sizeof(pgnum_t)) > (ESPECIAL_PAGES_BLOCK*PAGE_SIZE)
//#error "块管理空间大小不够"
//#endif

#define _GetMPage(ppg) 		((((((ppg-1)/PAGES_PRE_BLOCK+1)*PAGES_PRE_BLOCK)-ESPECIAL_PAGES_BLOCK)+((ppg-1)%PAGES_PRE_BLOCK)/PAGES_PER_PAGE) + 1)
#define _pPage2pAddr(ppg)	((ppg-1)*PAGE_SIZE)

#define _DirtyPhyBlk(blk)	_pBlkSetVBlk(blk, 0);
/* 作废指定物理页面*/
#define _DirtyPhyPage(x)    _pPageSetVAddr(x,0)

typedef struct {
    unsigned short eraseCnt;    //块擦除次数
    bknum_t  next;
} bkinfo_t;

struct {
	bknum_t Blk; //;	//物理块
	pgnum_t DirtyPages;
	pgnum_t FreePages;
	pgnum_t PgMapCash[VPAGES_PRE_VBLOCK];
} gBlkCashed;
static bknum_t BlkMap[VBLOCKS];
static bkinfo_t sBlocks[BLOCKS];
static bknum_t sFreeBlkList = NOMAP;

static void gDataInit(void)
{
	gBlkCashed.Blk = NOMAP;
	gBlkCashed.DirtyPages = 0;
	gBlkCashed.FreePages = 0;
	memset(gBlkCashed.PgMapCash, NOMAP, (sizeof(pgnum_t) * VPAGES_PRE_VBLOCK));	
	memset(BlkMap, NOMAP, sizeof BlkMap);
	memset(sBlocks, NOMAP, sizeof sBlocks);
	sFreeBlkList = NOMAP;
}
void _BlkSetEraseCnt(bknum_t blk, unsigned char cnt)
{
	int off = ERASECNTOFF;
	pgnum_t pg = blk*PAGES_PRE_BLOCK;
	FlaPageWrite(_pPage2pAddr(pg)+off, &cnt, sizeof (erasecnt_t));
}

static int _BuiltBlkCash(bknum_t blk)
{
	int ppg;
	int i, k;

	pgnum_t pgs[PAGES_PER_PAGE];

	if (blk == gBlkCashed.Blk) {
		return 0;
	}
	ppg = (blk*PAGES_PRE_BLOCK)-ESPECIAL_PAGES_BLOCK;
	
	gBlkCashed.Blk = NOMAP;
	gBlkCashed.DirtyPages = 0;
	gBlkCashed.FreePages = 0;
	memset(gBlkCashed.PgMapCash, NOMAP, (sizeof(pgnum_t) * VPAGES_PRE_VBLOCK));

	for (i=0; i<ESPECIAL_PAGES_BLOCK; i++) {		
		FlaPageRead((ppg+i)*PAGE_SIZE, (unsigned char *)pgs, sizeof pgs);	
		for (k=0; k<PAGES_PER_PAGE && ((i*PAGES_PER_PAGE)+k)<VPAGES_PRE_VBLOCK; k++) {
			if (!IsInvalidVPage(pgs[k])) {
				int phypg = (blk-1)*PAGES_PRE_BLOCK+i*PAGES_PER_PAGE+k+1;
				DebugInfo(DbgInf, "物理页[%u] ==>  虚拟页 [%u]", phypg, pgs[k]);
				gBlkCashed.PgMapCash[VPG2PGIDX(pgs[k])] = phypg;
			} else if (pgs[k] == (pgnum_t)NOMAP) {
			    gBlkCashed.FreePages++;
			} else { //if (pgs[k] == PAGE_DIRTY) {
				gBlkCashed.DirtyPages++;
			}
		}
	}
	gBlkCashed.Blk = blk;
    DebugInfo(DbgInf, "物理块[%u] 空闲页[%u] 脏页[%u]", 
		blk, gBlkCashed.FreePages, gBlkCashed.DirtyPages);

	return 1;
}

static int _GetFreePage(bknum_t blk) 
{
	//_BuiltBlkCash(blk);
	unsigned int pg;

	assert(blk == gBlkCashed.Blk);

	if (gBlkCashed.FreePages <= 0) {
		return ERR_NO_PAGE;
	}
	
	pg = ((blk-1) * PAGES_PRE_BLOCK + (VPAGES_PRE_VBLOCK - gBlkCashed.FreePages) + 1);
	gBlkCashed.FreePages--;

	DebugInfo(DbgInf, "从块 [%u] 中获取空闲页 [%u]", blk, pg);
	return (pgnum_t)pg;
}

static pgnum_t _pPageGetVAddr(pgnum_t ppg)
{
	pgnum_t mpg, vpg;
	int off;
	mpg = _GetMPage(ppg);
	off = ((ppg-1) % PAGES_PRE_BLOCK)%PAGES_PER_PAGE;
    FlaPageRead(_pPage2pAddr(mpg)+off*sizeof(pgnum_t), (unsigned char *)&(vpg), sizeof(pgnum_t));
	return vpg;	
}
static void _pPageSetVAddr(pgnum_t ppg, pgnum_t vpg)
{
	pgnum_t mpg;
	int off;
	mpg = _GetMPage(ppg);
	off = ((ppg-1) % PAGES_PRE_BLOCK)%PAGES_PER_PAGE;
	DebugInfo(DbgInf, "设置 物理页[%u] ==> 虚拟页[%u]", ppg, vpg);
    FlaPageWrite(_pPage2pAddr(mpg)+off*sizeof(pgnum_t), (unsigned char *)&vpg, sizeof(pgnum_t));
}
static void doPageMap(pgnum_t vpg, pgnum_t ppg)
{
//	pgnum_t oppg;
//
//	oppg = gBlkCashed.PgMapCash[VPG2PGIDX(vpg)];
//	if (!IsInvalidPPage(oppg)) {
//		_DirtyPhyPage(oppg); 	
//	}
	_pPageSetVAddr(ppg, vpg);
	gBlkCashed.PgMapCash[VPG2PGIDX(vpg)] = ppg;
}


//static bknum_t _pBlkGetVBlk(bknum_t pblk)
//{
//	pgnum_t pg;
//	bknum_t vblk;
//	int off = BLKMAPOFF;
//
//	pg = pblk*PAGES_PRE_BLOCK-1;
//	FlaPageRead((pg)*PAGE_SIZE+off, &vblk, sizeof(bknum_t));
//	return vblk;
//}


static void _pBlkSetVBlk(bknum_t pblk, bknum_t vblk)
{
	pgnum_t pg;
	int off = BLKMAPOFF;

	pg = pblk*PAGES_PRE_BLOCK-1;
	FlaPageWrite(pg*PAGE_SIZE+off, &vblk, sizeof(bknum_t));
}
static void doBlkMap(bknum_t vblk, bknum_t pblk)
{
	if (!ISInvalidPBlock(BlkMap[vblk-1])) {
	 	_DirtyPhyBlk(BlkMap[vblk-1]);	
	}
	_pBlkSetVBlk(pblk, vblk);
	BlkMap[vblk-1] = pblk;
}





static void _BlockInsertFreeLst(bknum_t blk)
{
    bknum_t b, v;

    if (ISInvalidPBlock(sFreeBlkList)) {
        sFreeBlkList = blk;
        sBlocks[blk-1].next = NOMAP;
        return;
    }
    v = sFreeBlkList;
    b = NOMAP;
    while (!ISInvalidPBlock(v)) {
        if (sBlocks[v-1].eraseCnt >= sBlocks[blk-1].eraseCnt)
            break;
        b = v;
        v = sBlocks[v-1].next;
    }
    sBlocks[blk-1].next = v;
    if (v == sFreeBlkList) 
        sFreeBlkList = blk;
    else
        sBlocks[b-1].next = blk;
}
static bknum_t _GetFreeBlk(void)
{
    bknum_t blk;
    if (ISInvalidPBlock(sFreeBlkList)) {
        DebugInfo(DbgErr, "没有空闲块可分配");
		while (1);
        //return NOMAP;
    }
    blk = sFreeBlkList;
    sFreeBlkList = sBlocks[sFreeBlkList-1].next;
    DebugInfo(DbgInf, "分配空闲块[%u]", blk);
 	sBlocks[blk-1].next = 0;
    return blk;
}

static void _BlkCopy(bknum_t dblk, bknum_t sblk)
{
	pgnum_t vpg;
	pgnum_t spg = (sblk-1)*PAGES_PRE_BLOCK;
	pgnum_t dpg = (dblk-1)*PAGES_PRE_BLOCK;
	pgnum_t duse = 1;
	unsigned char tmp[VPAGE_SIZE];
	int i;

	DebugInfo(DbgInf, "块复制 原[%u] ==>> 目的[%u]", sblk, dblk);
	for (i=1; i<=VPAGES_PRE_VBLOCK; i++) {
		vpg = _pPageGetVAddr(spg+i);
		if (!IsInvalidVPage(vpg)) {
			DebugInfo(DbgInf, "移动虚拟页[%u] : 原物理页[%u] --> 目的物理页[%u]", vpg, spg+i, dpg+duse);
			_pPageSetVAddr(dpg+duse, vpg);
			FlaPageRead(_pPage2pAddr(spg+i), tmp, VPAGE_SIZE);
			FlaPageWrite(_pPage2pAddr(dpg+duse), tmp, VPAGE_SIZE);
			duse++;
		}
	}				
}

static void _EraseBlk(bknum_t blk)
{
	FlaBlkErase(blk-1);
	sBlocks[blk-1].eraseCnt++;
	_BlkSetEraseCnt(blk, sBlocks[blk-1].eraseCnt);

}
/*=============================================================================
* Function		: FlaFormat
* Description	: 格式化flash存储器
* Input Para	: 
* Output Para	: 
* Return Value  : 
=============================================================================*/
void FlaFormat(void)
{
    int i;
    
    for (i=1; i<=BLOCKS; i++) 
        FlaBlkErase(i-1);
    FlaManageInit();
}

/*=============================================================================
* Function		: FlaManageInit
* Description	: 全局初始化，建立虚拟地址与物理地址的映射关系
* Input Para	: 
* Output Para	: 
* Return Value  : 
=============================================================================*/
int FlaManageInit(void)
{
	bknum_t bk;
	pgnum_t pg;

	int off = BLKMAPOFF ;
	unsigned char buf[sizeof(bknum_t)+sizeof(erasecnt_t)];
	bknum_t *pvbk = &buf[0];
	erasecnt_t __packed *pcnt = (erasecnt_t __packed *)&buf[sizeof(bknum_t)];

	gDataInit();

	for (bk=1; bk<=BLOCKS; bk++) {
		pg = bk*PAGES_PRE_BLOCK-1;
		FlaPageRead((pg)*PAGE_SIZE+off, (unsigned char *)&buf, sizeof(bknum_t)+sizeof(erasecnt_t));
		sBlocks[bk-1].eraseCnt = (*pcnt == (erasecnt_t)-1 ? 0 : *pcnt);
		if (ISInvalidVBlock(*pvbk)) {
			if (*pvbk != (bknum_t)NOMAP) {
				_EraseBlk(bk);
			}
			_BlockInsertFreeLst(bk);	
		} else {
			DebugInfo(DbgInf, "物理块[%u] ==> 虚拟块[%u]", bk, *pvbk);
			BlkMap[*pvbk-1] = bk; 
		}	
	}
        return 0;
}



void FlaUnMapvPage(int vpg)
{
	bknum_t vblk, pblk;
	pgnum_t ppg;

	if (IsInvalidVPage(vpg)) {
		DebugInfo(DbgErr, "无效虚拟地址 [%u]\r\n", vpg);
		return;
	}
	vblk = VPG2VBLK(vpg);
	pblk = BlkMap[vblk-1];

	if (ISInvalidPBlock(pblk)) {
		//没有映射块
		return;
	}
	_BuiltBlkCash(pblk);
	
	ppg = gBlkCashed.PgMapCash[VPG2PGIDX(vpg)];
	if (IsInvalidPPage(ppg)) {
		return;
	}
	_DirtyPhyPage(ppg);
	gBlkCashed.PgMapCash[VPG2PGIDX(vpg)] = 0;
	return;

}

/*=============================================================================
* Function		: FlaVPageRead
* Description	: 读虚拟页vpg，到buf
* Input Para	: 
* Output Para	: 
* Return Value  : 
=============================================================================*/
int FlaVPageRead(unsigned vpg, unsigned off, void *buf, int len)
{
	bknum_t vblk, pblk;
	pgnum_t ppg;

	if (IsInvalidVPage(vpg)) {
		DebugInfo(DbgErr, "无效虚拟地址 [%u]\r\n", vpg);
		return -1;
	}
	vblk = VPG2VBLK(vpg);
	pblk = BlkMap[vblk-1];

	if (ISInvalidPBlock(pblk)) {
		//需要映射块
#if 0
		pblk = _GetFreeBlk();
		if (ISInvalidPBlock(pblk)) {
			return -2;
		}
		doBlkMap(vblk, pblk);
#endif
		DebugInfo(DbgInf, "读 虚拟页[%u] 块没有映射\r\n", vpg);
		memset(buf, 0xff, len);
		return len;
	}
	_BuiltBlkCash(pblk);
	
	ppg = gBlkCashed.PgMapCash[VPG2PGIDX(vpg)];
	if (IsInvalidPPage(ppg)) {	
		//需要映射页面
#if 0
		ppg = _GetFreePage(gBlkCashed.Blk);
		if (ppg == ERR_NO_PAGE) {
			//需要新块
			bknum_t nblk;
			nblk = _GetFreeBlk();			
			if (ISInvalidPBlock(nblk)) {
				return -3;
			}
			_BlkCopy(nblk, pblk);
			doBlkMap(vblk, nblk);

			_EraseBlk(pblk);
			_BlockInsertFreeLst(pblk);

			_BuiltBlkCash(nblk);
			ppg = _GetFreePage(gBlkCashed.Blk);
			if (ppg == ERR_NO_PAGE) {
				DebugInfo(DbgErr, "页管理逻辑错误\r\n");	
				while (1);
			}
		}
		doPageMap(vpg, ppg);
#endif
		DebugInfo(DbgInf, "读 虚拟页[%u] 页没有映射\r\n", vpg);
		memset(buf, 0xff, len);
		return len;
	}
	DebugInfo(DbgInf, "读 虚拟页[%u] --> 物理块[%u] -> 物理页[%u]",vpg, pblk, ppg);
    FlaPageRead(_pPage2pAddr(ppg)+off, buf, len);
	return len;
}


static int _DataNeeNewPage(const char *s, const char *d, int len)
{
#if 0
    for (i=0; i<len; i++) {
        if ((s[i] & d[i]) != d[i])
            return 1;
    }
#endif
    return 1;
}
/*=============================================================================
* Function		: FlaVPageWrite
* Description	: 写buf的数据到虚拟页vpg
* Input Para	: 
* Output Para	: 
* Return Value  : 
=============================================================================*/
int FlaVPageWrite(unsigned vpg, unsigned off, void *buf, int len)
{
	bknum_t vblk, pblk;
	pgnum_t ppg, npg;
	
	char tmp[VPAGE_SIZE];

	if (IsInvalidVPage(vpg)) {
		DebugInfo(DbgErr, "无效虚拟地址 [%u]\r\n", vpg);
		return -1;
	}
	vblk = VPG2VBLK(vpg);
	pblk = BlkMap[vblk-1];

	if (ISInvalidPBlock(pblk)) {
		//需要映射块
		pblk = _GetFreeBlk();
		if (ISInvalidPBlock(pblk)) {
			return -2;
		}
		doBlkMap(vblk, pblk);		
	}
	_BuiltBlkCash(pblk);
	
	ppg = gBlkCashed.PgMapCash[VPG2PGIDX(vpg)];
	if (!IsInvalidPPage(ppg) /*&& len!=VPAGE_SIZE*/) {	
		//读取原来页面数据
		FlaPageRead(_pPage2pAddr(ppg), (unsigned char *)tmp,  VPAGE_SIZE);
		if (!_DataNeeNewPage(tmp+off, buf, len)) {
			FlaPageWrite(_pPage2pAddr(ppg)+off, buf,  len);
			return len;
		} 
		memcpy(tmp+off, buf, len);
		off = 0;
		buf = tmp;
		len = VPAGE_SIZE;
		_DirtyPhyPage(ppg);
	}
	//需要映射页面
	npg = _GetFreePage(gBlkCashed.Blk);
	if (IsInvalidPPage(npg)) {
			//需要新块
			bknum_t nblk;
			nblk = _GetFreeBlk();			
			if (ISInvalidPBlock(nblk)) {
				DebugInfo(DbgErr, "块管理逻辑错误\r\n");	
				while (1);
				//return -3;
			}
			doBlkMap(vblk, nblk);
			_BlkCopy(nblk, pblk);
			_EraseBlk(pblk);
			_BlockInsertFreeLst(pblk);			
			_BuiltBlkCash(nblk);
			npg = _GetFreePage(gBlkCashed.Blk);
			if (IsInvalidPPage(npg)) {
				DebugInfo(DbgErr, "页管理逻辑错误\r\n");	
				while (1);
			}
			pblk = nblk;
	}
	doPageMap(vpg, npg);
	DebugInfo(DbgInf, "写 虚拟页[%u] ->物理块[%u] --> 物理页[%u]", vpg, pblk, npg);
#if 0
{
	int i;
	printf("\r\n");
	for (i=off; i<256; i++)
		printf("%02X ", ((char *)buf)[i]);
}
#endif
    FlaPageWrite(_pPage2pAddr(npg)+off, buf, len);
	return len;
}


void FlaFragTidy02(void (*fun)(void))
{
	//printf("%s : Need update !\r\n", __FUNCTION__);
}

#define MORE_FUNCTION   1
#if MORE_FUNCTION
/*=============================================================================
* Function		: 
* Description	: 
* Input Para	: 
* Output Para	: 
* Return Value  : 
=============================================================================*/
void FlaBlockInfo(void)
{
	int i;
	
	unsigned use;

	printf("\r\n   块    使用页   空闲页   脏页  擦除次数");
	printf("\r\n-------------------------------------------");

	for (i=1; i<=BLOCKS; i++) {
		_BuiltBlkCash(i);
		use = VPAGES_PRE_VBLOCK - gBlkCashed.FreePages - gBlkCashed.DirtyPages;
		printf("\r\n  %-6u   %-6u  %-6u  %-6u  %-6u", 
			i, use, gBlkCashed.FreePages, gBlkCashed.DirtyPages, sBlocks[i-1].eraseCnt);
	}
	printf("\r\n");
}

void FlaPageInfo(void)
{    
	int i;
	unsigned free = 0, dirty = 0, valite = 0, total = 0;

	for (i=1; i<=BLOCKS; i++) {
		_BuiltBlkCash(i);
		
		free += gBlkCashed.FreePages;
		dirty += gBlkCashed.DirtyPages;
	}
	total = VPAGES_PRE_VBLOCK*BLOCKS;
	valite = total -free-dirty;
	printf("\r\n  使用页    空闲页     脏页     总页数");
	printf("\r\n-----------------------------------------");
	printf("\r\n   %-7u   %-7u   %-7u   %7u", valite, free, dirty, total);
	printf("\r\n");
}

/*
int command_fla(int argc, char *argv[])
{
    char pgbuf[256];
    int i;
  
    int pg, off, len;

//    if (init == 0) {
  //      FlaManageInit();
  //      init = 1;
 //   }

    if (argc==2 && strcmp(argv[1], "format") == 0) {
        FlaFormat();
    }else if (argc==5 && strcmp(argv[1], "write") == 0) {
        pg = atoi(argv[2]);
        off = atoi(argv[3]);
        len = str2dat(argv[4],pgbuf, 256); 
        bufdump("Write : ", pgbuf, len);
        FlaVPageWrite(pg, off, pgbuf, len);
	  //	fpage_write((pg-1)*32+off, pgbuf, len);
    }
    else if (argc==5 && strcmp(argv[1], "read") == 0) {
        pg = atoi(argv[2]);
        off = atoi(argv[3]);
        len = atoi(argv[4]);
        if (FlaVPageRead(pg, off, pgbuf, len) >= 0)
	//	if (fpage_read((pg-1)*32+off, pgbuf, len)>0)
            bufdump("Read : ", pgbuf, len);
    } else if (argc == 2 && strcmp(argv[1], "pginfo") == 0) {
        FlaPageInfo();
    } else if (argc == 2 && strcmp(argv[1], "blkinfo") == 0) {
        FlaBlockInfo();
    } else if (argc == 3 && strcmp(argv[1], "fragtidy") == 0) {
        //int v = atoi(argv[2]);
        //printf("整理块虚拟页[%d]\r\n", v);
      //  _FragTidyEx(v);
     //   _FragTidy01();
       //FlaFragTidy03();
     //  FlaFragTidy04(v);
        FlaFragTidy02(0);
    } else if (argc == 4 && strcmp(argv[1], "auto") == 0) {
        int i;
        int bpg = atoi(argv[2]);
        int epg = atoi(argv[3]);
        static int s = 0;
        s++;
#if 1
        for (i=bpg; i<=epg; i++) {
            memset (pgbuf, i+s, 256);
            if(FlaVPageWrite(i, 0, pgbuf, 256) < 0)
                break;
        }
#else
        srand(GenClock());
        for (i=bpg; i<=epg; i++) {
            memset (pgbuf, i+s, 256);
            if(FlaVPageWrite(1+(rand()%12000), 0, pgbuf, 256) < 0)
                break;
        }
#endif

    } else if (strcmp(argv[1], "bt") == 0){
        WBlockTest(argc-1, &argv[1]);
    } else if (strcmp(argv[1], "dl") == 0) {
        DealTest(argc-1, &argv[1]);
    } 
    else {
        printf(" format\r\n");
        printf(" write <pg> <off> <data>\r\n");
        printf(" read  <pg> <off> <len>\r\n");
        printf(" pginfo\r\n");
        printf(" blkinfo\r\n");
        printf(" fragtidy \r\n");
        printf(" auto <bpg> <epg>\r\n");
    }
}
*/
#endif
