/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /usr/local/dslrepos/uClinux-dist/user/boa/src/LINUX/rtl_flashdrv_mmu.c,v 1.14 2008/11/26 04:13:40 kaohj Exp $
*
* Abstract: Flash driver source code.
*
* $Author: kaohj $
*
* $Log: rtl_flashdrv_mmu.c,v $
* Revision 1.14  2008/11/26 04:13:40  kaohj
* modify flash dump command
*
* Revision 1.13  2008/10/03 12:55:40  kaohj
* fix native compilation error
*
* Revision 1.12  2008/05/08 13:37:11  eric
* fix upgrade fail at 2.6
*
* Revision 1.11  2008/04/25 09:00:38  jenny
* web upgrade firmware write flash determination
*
* Revision 1.10  2008/02/20 14:43:00  eric
*
* Add support for spansion 16MB flash
*
* Revision 1.9  2008/01/17 05:29:17  yachang
* remove dump mtd info
*
* Revision 1.8  2008/01/16 13:53:48  yachang
* patch for linux 2.6
*
* Revision 1.7  2007/05/15 12:45:40  star
* no message
*
* Revision 1.6  2007/01/05 16:35:37  kaohj
* fix flashdrv_read() bug for reading size more than 4096
*
* Revision 1.5  2006/10/03 14:09:30  kaohj
* add for configuration server and related
*
* Revision 1.4  2006/07/26 05:27:39  masonyu
* Modify mtd_start for write flash
*
* Revision 1.3  2006/07/24 06:06:57  masonyu
* FTP client for upload Image
*
* Revision 1.2  2005/12/27 06:00:24  kaohj
* support for native build
*
* Revision 1.1  2005/12/07 11:46:11  kaohj
* mmu support
*
* Revision 1.1  2005/08/29 08:16:20  kaohj
* port from goahead to boa
*
* Revision 1.1.1.1  2005/08/15 13:56:31  hjen
* initial import int CVS
*
* Revision 1.2  2005/07/12 08:38:23  kaohj
* support auto-detect flash
*
* Revision 1.1  2003/10/01 09:08:42  dicktam
* *** empty log message ***
*
* Revision 1.3  2003/06/23 11:11:22  elvis
* change include path of  rtl_types.h
*
* Revision 1.2  2003/05/20 08:52:39  elvis
* change the include path of "rtl_types.h"
*
* Revision 1.1  2003/04/29 14:17:04  orlando
* flashdrv module initial check-in (used by cfgmgr), ported from srcroot.
*
* Revision 1.5  2003/03/13 10:49:24  cfliu
* no message
*
* Revision 1.4  2003/03/04 13:33:52  danwu
* no message
*
* Revision 1.3  2002/11/07 12:04:12  henryCho
* Add flashdrv_test().
*
* Revision 1.2  2002/07/30 04:33:28  danwu
* Add flashdrv_updateImg for boot.
*
* Revision 1.1  2002/07/19 08:37:58  danwu
* Create file.
*
*
* ---------------------------------------------------------------
*/

#include <stdio.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../uClibc/include/linux/autoconf.h"
#endif
#include <linux/version.h>
   
//andrew, maybe we can merge into one, by replace MEMWRITE/MEMREAD with write/read
#if (LINUX_VERSION_CODE < 0x00020600)  // linux 2.4?

#define KERNEL_VERSION_24 1

#ifdef EMBED
#include <linux/mtd/mtd.h>
#endif

#else // linux 2.6

#ifdef EMBED
#include <mtd/mtd-user.h>
#endif

#endif

#include "rtl_types.h"
#include "rtl_board.h"
//#include "rtl_flashdrv.h"

#if 0
/* Manufacturers */
#define MANUFACTURER_MXIC	0x00C2

/* MXIC deivce id */
#define MX29LV800B		0x225B
#define MX29LV160AB		0x2249
#define MX29LV320AB		0x22A8

struct flash_desc {
	unsigned short mfr_id;
	unsigned short dev_id;
	uint32 size;
	uint32 block_num;
	const uint32 *blockOffset;
};

static struct flash_desc table[]= {
	{
		mfr_id: MANUFACTURER_MXIC,
		dev_id: MX29LV800B,
		size: 0x00100000,
		block_num: BLOCK_NUM_1M,
		blockOffset: blockOffset_1M
	},
	{
		mfr_id: MANUFACTURER_MXIC,
		dev_id: MX29LV160AB,
		size: 0x00200000,
		block_num: BLOCK_NUM_2M,
		blockOffset: blockOffset_2M
	},
	{
		mfr_id: MANUFACTURER_MXIC,
		dev_id: MX29LV320AB,
		size: 0x00400000,
		block_num: BLOCK_NUM_4M,
		blockOffset: blockOffset_4M
	}
};

static struct flash_desc *map;
#endif

//see drivers/mtd/mapsrtl865x_flash.c for partition details
#ifdef CONFIG_SPANSION_16M_FLASH
static uint32 mtd_end[6]={0, 0,0,0,0,0};
static uint32 mtd_start[6]={0, 0, 0, 0, 0, 0};
#else 
static uint32 mtd_end[2]={0, 0};
static uint32 mtd_start[2]={0, 0};
#endif

#if 0
uint32 flashdrv_init(flashdriver_obj_t * const drvObj_P)
{
    ASSERT_CSP( drvObj_P );
    if ( drvObj_P->blockBaseArrayCapacity < FLASH_NUM_OF_BLOCKS )
        return 1;
    
	drvObj_P->flashSize = FLASH_TOTAL_SIZE;
	drvObj_P->flashBaseAddress = FLASH_BASE;
	drvObj_P->blockNumber = FLASH_NUM_OF_BLOCKS;
	memcpy(drvObj_P->blockBaseArray_P, flashBlockOffset,
	             FLASH_NUM_OF_BLOCKS * sizeof(uint32));
    
    return 0;
}
#endif

int get_flash_index(void *src_addr)
{
    int idx=0;
    
    do {
    	if ((uint32)src_addr<mtd_end[idx])
    		return idx*2;
    	idx++;
    } while (idx<(sizeof(mtd_end)/sizeof(int)));
    return -1;
}

/*
static void dump_mtd_info(mtd_info_t *mtd) {
	printf("\nMTD Info\n");
	printf("\tType: %d\tFlags: %d\tSize: 0x%x\n", mtd->type, mtd->flags, mtd->size);
	printf("\tErasesize: 0x%x\tWritesize: 0x%x\tOOBSize: 0x%x\n", mtd->erasesize, mtd->writesize, mtd->oobsize);
}
*/

uint32 flashdrv_init()
{
#ifdef EMBED
    int	fd, i;
    char flashdev[32];
	mtd_info_t	mtd_info;
	
	for (i=0;i<sizeof(mtd_end)/sizeof(int);i++) {
		sprintf(flashdev, "/dev/mtd%d", i*2);
	    if ((fd = open(flashdev, O_RDONLY)) < 0) {
		    printf("ERROR: failed to open(%s), errno=%d\n",
			    flashdev, errno);
		    return 1;
	    }
	    if (ioctl(fd, MEMGETINFO, &mtd_info) < 0) {
		    printf("ERROR: ioctl(MEMGETINFO) failed, errno=%d\n",
			    errno);
		    return 1;
	    }

	    mtd_end[i] = (uint32)mtd_info.size;
	    if (i>0) mtd_end[i] += mtd_end[i-1];
	    
	    // Added by Mason Yu
	    if (i>0)
	    	mtd_start[i] = mtd_end[i-1];
	    else
	    	mtd_start[i] = 0x0;
	    
	    //printf("flash end %d: %08x\n", i, mtd_end[i]);
	    //printf("flash start %d: %08x\n", i, mtd_start[i]);
	    close(fd);
	}
#endif

	return 0;
}

#if 0
uint32 flashdrv_eraseBlock(void *startAddr_P)
{
    uint32                  blockNum;
    uint32                  count = 0;
    volatile uint16         *flash_cui;
    volatile uint32         *test_cui;
    
    ASSERT_CSP( startAddr_P );
    ASSERT_CSP( map );
    
    flash_cui = (volatile uint16 *) ( (uint32)startAddr_P - FLASH_BASE );
    
    /* Check if legal starting address */
    //for (blockNum = 0; blockNum < FLASH_NUM_OF_BLOCKS; blockNum++)
    for (blockNum = 0; blockNum < map->block_num; blockNum++)
        //if (flashBlockOffset[blockNum] == (uint32) flash_cui)
        if (map->blockOffset[blockNum] == (uint32) flash_cui)
            break;
    //if (blockNum >= FLASH_NUM_OF_BLOCKS)
    if (blockNum >= map->block_num)
        return 1;

    /* Start erase sequence */
    flash_cui = (volatile uint16 *) startAddr_P;
    
    *AM29LVXXX_COMMAND_ADDR1 = AM29LVXXX_COMMAND1;
    *AM29LVXXX_COMMAND_ADDR2 = AM29LVXXX_COMMAND2;
    *AM29LVXXX_COMMAND_ADDR1 = AM29LVXXX_SECTOR_ERASE_CMD1;
    *AM29LVXXX_COMMAND_ADDR1 = AM29LVXXX_COMMAND1;
    *AM29LVXXX_COMMAND_ADDR2 = AM29LVXXX_COMMAND2;
    *flash_cui = AM29LVXXX_SECTOR_ERASE_CMD2;
    
    /* Check if completed */
    //if ( blockNum < FLASH_NUM_OF_BLOCKS - 1 )
    if ( blockNum < map->block_num - 1 )
        //test_cui = (volatile uint32 *) (flashBlockOffset[blockNum + 1] 
        test_cui = (volatile uint32 *) (map->blockOffset[blockNum + 1] 
                            + FLASH_BASE - 4);
    else
        test_cui = (volatile uint32 *) (FLASH_BASE - 4);
    
    while ((uint32)test_cui >= (uint32)startAddr_P)
    {
        if (*test_cui == 0xFFFFFFFF)
            test_cui--;
        else
        {
            count++;
            if (count > 0x3fFFFF)
            {
              printf("! Block number %d erase \n", blockNum);
            	return 1;
            }
        }
    }
    
    return 0;
}
#endif

uint32 flashdrv_read (void *dstAddr_P, void *srcAddr_P, uint32 size)
{
#ifdef EMBED
	int idx;
	unsigned long thislen, thissrc, ofs;
	unsigned char *thisdst;
	//ASSERT_CSP( srcAddr_P );
	//ASSERT_CSP( dstAddr_P );
	
	if ( (idx=get_flash_index(srcAddr_P)) >=0 ) {
		int	fd;
		char flashdev[32];
		#if KERNEL_VERSION_24
		struct mtd_oob_buf arg;
		#endif
		
		sprintf(flashdev, "/dev/mtd%d", idx);
		if ((fd = open(flashdev, O_RDWR)) < 0) {
			printf("ERROR: failed to open(%s), errno=%d\n",
			    flashdev, errno);
			return 1;
		}
		
		thissrc = (uint32)srcAddr_P;
			thisdst = (unsigned char *)dstAddr_P;
			ofs = 0;
		while (size) {
			thissrc = (uint32)srcAddr_P+ofs;
			thisdst = (unsigned char *)dstAddr_P+ofs;
			// Kaohj
			// Note: the size (4096) depends on the mtd read function.
			//	drivers/mtd/maps/rtl865x_flash.c: rtl865x_map_copy_from()
			//	would check to fit the filesystem block size
			if  (size <= 4096) {
				thislen = size;
			}
			else {
				thislen = 4096;
				ofs += 4096;
			}
			
			#if KERNEL_VERSION_24
			arg.start=thissrc;
			arg.length=thislen;
			arg.ptr=thisdst;
			if (ioctl(fd, MEMREADDATA, &arg) < 0) {
				printf("ERROR: ioctl(MEMREADDATA) failed, errno=%d\n",
				    errno);
				return 1;
			}
			#else
			{
			/*int rv; 
			fprintf(stderr, "lseek(%d, %x) = ", fd, thissrc);
			rv = lseek(fd, thissrc, SEEK_SET);			
			fprintf(stderr, "%d\n", rv);
			fprintf(stderr, "read(%d, %x, %d) = ", fd, thisdst, thislen);
			rv = read(fd, thisdst, thislen);
			fprintf(stderr, "%d\n", rv);
			if (rv < 0) {
				printf("ERROR: read %u bytes from %x to %x failed, errno=%d\n",
					thislen, thissrc, thisdst, errno);
				return 1;
			}*/
			  thissrc = thissrc - mtd_start[idx/2];
			if ((lseek(fd, thissrc, SEEK_SET) < 0) ||(read(fd, thisdst, thislen) < 0)) {
				printf("ERROR: read %u bytes from %x to %x failed, errno=%d\n",
					thislen, thissrc, thisdst, errno);
		    return 1;
	    }
			}
			#endif
			size-=thislen;
	    }

		close(fd);
		return 0;
	} else
		return 1;
#else
	return 0;
#endif
}
#ifdef CONFIG_SPANSION_16M_FLASH
uint32 flashdrv_128K_write_1st_sector(void *dstAddr_P, void *srcAddr_P, uint32 size)
{
#ifdef EMBED
	int idx;
	char *buff = 0;

	if (size >= 0x20000)
	{
		printf("%s:only support writing 1st sector...",__FUNCTION__);
		goto error;
	}

	if (dstAddr_P < 0x20000)
		buff = malloc(0x20000);
	if (!buff)
	{
		printf("%s: can't allocate!!",__FUNCTION__);
		goto error;
	}
	if (flashdrv_read(buff, 0, 0x20000) !=0 )
	{	
		printf("%s: read fail!!", __FUNCTION__);
		goto error;
	}

	memcpy(buff + (int)dstAddr_P , srcAddr_P, size);
	
	{
        int	fd,i;
        char flashdev[32];
        struct erase_info_user arge;
        struct mtd_oob_buf argw;

		sprintf(flashdev, "/dev/mtd0");	//specical case, always 1st sector
	    if ((fd = open(flashdev, O_RDWR)) < 0) {
	    	    printf("ERROR: failed to open(%s), errno=%d\n",
			    flashdev, errno);
			goto error;
	    }
	    arge.start=(uint32)0;
	    arge.length=(uint32)0x20000;
	    if (ioctl(fd, MEMERASE, &arge) < 0) {
		    printf("ERROR: ioctl(MEMERASE) failed, errno=%d\n",
			    errno);
		    return 1;
	    }
		

	// printf("\r\n writing  %u bytes from %x to %x -- \r\n",
	//		0x20000, buff, 0);	
	    if ((lseek(fd, 0, SEEK_SET) < 0) ||(write(fd, buff , 0x20000) < 0)) {
		   printf("ERROR: write %u bytes from %x to %x failed, errno=%d\n",
			size, srcAddr_P, dstAddr_P, errno);
		   close(fd);    
		   
		   goto error;
	    }

	    close(fd);   
	free(buff);
        return 0;
    }

error:
	if (buff)	free(buff);
	return 1;
	
#else
	return 0;
#endif
}
#endif 

uint32 flashdrv_write(void *dstAddr_P, void *srcAddr_P, uint32 size)
{
#ifdef EMBED
  int idx;
    //ASSERT_CSP( srcAddr_P );
    //ASSERT_CSP( dstAddr_P );
    
    if ((idx=get_flash_index(dstAddr_P))>=0) {
        int	fd;
        char flashdev[32];
        struct erase_info_user arge;
        struct mtd_oob_buf argw;
	int writebyte;

		sprintf(flashdev, "/dev/mtd%d", idx);
		printf("write %s\n", flashdev);
	    if ((fd = open(flashdev, O_RDWR)) < 0) {
		    printf("ERROR: failed to open(%s), errno=%d\n",
			    flashdev, errno);
		    return 1;
	    }
	    
	    // Added by Mason Yu	   
	    dstAddr_P = dstAddr_P - mtd_start[idx/2];
	    	

#if 0	
	    argw.start=(uint32)dstAddr_P;
	    argw.length=(uint32)size;
	    argw.ptr=(unsigned char *)srcAddr_P;
	    if (ioctl(fd, MEMWRITEDATA, &argw) < 0) {
		    printf("ERROR: ioctl(MEMREADDATA) failed, errno=%d\n",
			    errno);
		    close(fd);    
		    return 1;
	    }
#else
	    //if ((lseek(fd, dstAddr_P, SEEK_SET) < 0) ||(write(fd, srcAddr_P, size) < 0)) {
	    if ((lseek(fd, dstAddr_P, SEEK_SET) < 0)) {
		   printf("ERROR: write %u bytes from %x to %x failed, errno=%d\n",
			size, srcAddr_P, dstAddr_P, errno);
		   close(fd);
		   return 1;
	    }
	    writebyte = write(fd, srcAddr_P, size);
#endif
	    
	    close(fd);    
	    return writebyte;
//       return 0;
    } else
        return 1;

#if 0	
    volatile uint16         *dstAddr;
    volatile uint16         *srcAddr;
    uint32                  len;
    uint32                  count = 0;

    //ASSERT_CSP( srcAddr_P );
    //ASSERT_CSP( dstAddr_P );
    //ASSERT_CSP( map );

    /* Check if legal range */
    if ( ((uint32) dstAddr_P < FLASH_BASE) || 
            //((uint32) ( (uint32)dstAddr_P + size ) > (FLASH_BASE + FLASH_TOTAL_SIZE)) )
            ((uint32) ( (uint32)dstAddr_P + size ) > (FLASH_BASE + map->size)) )
    {
        printf("flashdrv_write legal range fail! %08x %d\n ", (uint32)dstAddr_P, size);
        return 1;
    }
    
    dstAddr = (volatile uint16 *) dstAddr_P;
    srcAddr = (volatile uint16 *) srcAddr_P;
    len = size / 2 + size % 2;
    
    while (len)
    {
        /* Start program sequence */
        *AM29LVXXX_COMMAND_ADDR1 = AM29LVXXX_COMMAND1;
        *AM29LVXXX_COMMAND_ADDR2 = AM29LVXXX_COMMAND2;
        *AM29LVXXX_COMMAND_ADDR1 = AM29LVXXX_PROGRAM_CMD;
        *dstAddr = *srcAddr;
        
	    /* Check if completed */
	    count = 0;
	    while (1)
	    {
	        if ((uint16)*dstAddr == (uint16)*srcAddr)
	        {
	            dstAddr++;
	            srcAddr++;
	            break;
	        }
	        else
	        {
	            count++;
	            if (count > 0xFF)
                {
                    		printf("! Address %x write \n", (uint32) dstAddr);
	         	   	return 1;
	         	}
	        }
	    }
	    
        len--;
    }    
    
    return 0;
#endif    
#else
	return 0;
#endif
}

uint32 flashdrv_updateImg(void *srcAddr_P, void *dstAddr_P, uint32 size)
{
#ifdef EMBED
  int idx;
    //ASSERT_CSP( srcAddr_P );
    //ASSERT_CSP( dstAddr_P );
#if CONFIG_SPANSION_16M_FLASH
	struct mtd_info_user info;
#endif

    if ((idx=get_flash_index(dstAddr_P))>=0) {
        int	fd;
        char flashdev[32];
	int ret;
        struct erase_info_user arge;
        struct mtd_oob_buf argw;

		sprintf(flashdev, "/dev/mtd%d", idx);
	    if ((fd = open(flashdev, O_RDWR)) < 0) {
		    printf("ERROR: failed to open(%s), errno=%d\n",
			    flashdev, errno);
		    return 1;
	    }
#if CONFIG_SPANSION_16M_FLASH
	 if (ioctl(fd, MEMGETINFO, &info) < 0) {
		    printf("ERROR: ioctl(MEMGETINFO) failed, errno=%d\n",
			    errno);
		    return 1;
	    }
	// sector size is 128 K, spansion 16mb flash
	if ((info.erasesize == 0x20000) && (idx ==0))
		return flashdrv_128K_write_1st_sector(dstAddr_P, srcAddr_P, size);

#endif 
	
	    
	    // Added by Mason Yu	    
	    dstAddr_P = dstAddr_P - mtd_start[idx/2];
	    //printf("flashdrv_updateImg(1): dstAddr_P=0x%x mtd_start[%d/2]=0x%x\n", dstAddr_P, idx, mtd_start[idx/2] );
	    
	    arge.start=(uint32)dstAddr_P;
	    arge.length=(uint32)size;
	    if (ioctl(fd, MEMERASE, &arge) < 0) {
		    printf("ERROR: ioctl(MEMERASE) failed, errno=%d\n",
			    errno);
		    return 1;
	    }
	    close(fd);    
        /* Write blocks */
        // Added by Mason Yu        
	   dstAddr_P = dstAddr_P + mtd_start[idx/2];        
           //printf("flashdrv_updateImg(2):dstAddr_P=0x%x  mtd_start[%d/2]=0x%x\n", dstAddr_P, idx, mtd_start[idx/2] );
        
	    ret = flashdrv_write(dstAddr_P, srcAddr_P, size);
	   if (ret == size)
	        return 0;
	   else
	   	return 1;
        //return flashdrv_write(dstAddr_P, srcAddr_P, size);
    } else
        return 1;
#else
	return 0;
#endif
}

#if 0
void flashdrv_test(uint32 channel, int32 argc, int8 ** argv)
{
    void *  pStartAddr;
    uint32  count = 0;
    uint32  blockNum;
    uint32  startBlkNum;
    
    pStartAddr = (void *) 0x80000;
    
    /* Search the starting block */
    //for (blockNum = 0; blockNum < (FLASH_NUM_OF_BLOCKS - 1); blockNum++)
    for (blockNum = 0; blockNum < (map->block_num - 1); blockNum++)
        //if ( flashBlockOffset[blockNum + 1] > (uint32) pStartAddr )
        if ( map->blockOffset[blockNum + 1] > (uint32) pStartAddr )
            break;
    startBlkNum = blockNum;
    
    while (count++ < 1000)
    {
        /* Erase the blocks */
        //for (blockNum = startBlkNum; blockNum < (FLASH_NUM_OF_BLOCKS - 1); blockNum++)
        for (blockNum = startBlkNum; blockNum < (map->block_num - 1); blockNum++)
        {
            //if ( flashdrv_eraseBlock((void *) (flashBlockOffset[blockNum] + FLASH_BASE) ) )
            if ( flashdrv_eraseBlock((void *) (map->blockOffset[blockNum] + FLASH_BASE) ) )
            {
                printf("Erase block #%d fail!\n", blockNum);
                return;
            }
        }
            
        /* Write blocks */
        //for (blockNum = startBlkNum; blockNum < (FLASH_NUM_OF_BLOCKS - 1); blockNum++)
        for (blockNum = startBlkNum; blockNum < (map->block_num - 1); blockNum++)
        {
            //if ( flashdrv_write((void *) (flashBlockOffset[blockNum] + FLASH_BASE), 
            if ( flashdrv_write((void *) (map->blockOffset[blockNum] + FLASH_BASE), 
                                (void *) (0x80600000), 
                                0x10000) )
            {
                printf("Write block #%d fail!\n", blockNum);
                return;
            }
        }
        
        printf(".");
    }
    
    /* Clear */
    //for (blockNum = startBlkNum; blockNum < (FLASH_NUM_OF_BLOCKS - 1); blockNum++)
    for (blockNum = startBlkNum; blockNum < (map->block_num - 1); blockNum++)
    {
        /* Erase the block */
        //if ( flashdrv_eraseBlock((void *) (flashBlockOffset[blockNum] + FLASH_BASE) ) )
        if ( flashdrv_eraseBlock((void *) (map->blockOffset[blockNum] + FLASH_BASE) ) )
        {
            printf("Erase block #%d fail!\n", blockNum);
            return;
        }
    }
    
    printf("SUCCESS!\n");
}
#endif
