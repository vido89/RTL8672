/*
**	zipflash.c
**	flash driver written by wangjian for ggv gp1288
*/


#include <linux/config.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mman.h>
#include <linux/ioctl.h>
#include <linux/fd.h>
#include <linux/init.h>
#include <linux/slab.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/byteorder.h>

#include "tsbref.h"
#include "tx39.h" 


#define MAJOR_NR FLASHDISK_MAJOR
#include <linux/blk.h>

///////////////////////// zip variable ////////////////////////////

#include "flash.h"

///////////////////////////////////////////////////////////////////

#define IS64M	0

#define RDBLK_SIZE_BITS		9
#define RDBLK_SIZE		(1<<RDBLK_SIZE_BITS) /* 512 */

#define	WRITE_MFIOOUT(i)	WRITE_TX39REG( i , OFF_IO_MFIOOUT)
#define	READ_MFIOOUT		(READ_TX39REG( OFF_IO_MFIOOUT))
#define	OR_MFIOOUT(i)		OR_TX39REG( i , OFF_IO_MFIOOUT)
#define	AND_MFIOOUT(i)		AND_TX39REG( i , OFF_IO_MFIOOUT)
#define MFIO_IN				(READ_TX39REG(OFF_IO_MFIOIN)) 
#define	FDPort				(*((volatile unsigned char *) 0xabffffff))	//BIU I/O Access to Card
#define HI					1
#define LOW					0
#define ENABLE				0
#define DISABLE				1
#define FlashReady			(READ_TX39REG(OFF_IO_MFIOIN) & IO_MFIOIN(FLASH_RB))
/*
#define FlashALE(hi)		(hi? OR_MFIOOUT(IO_MFIOOUT(FLASH_ALE)) : AND_MFIOOUT( ~IO_MFIOOUT(FLASH_ALE)))
#define FlashCLE(hi)		(hi? OR_MFIOOUT(IO_MFIOOUT(FLASH_CLE)) : AND_MFIOOUT( ~IO_MFIOOUT(FLASH_CLE)))
#define FlashCE(enable)		(enable ? OR_MFIOOUT(IO_MFIOOUT(FLASH_CE)) : AND_MFIOOUT( ~IO_MFIOOUT(FLASH_CE)))
#define FlashWP(hi)			(hi? OR_MFIOOUT(IO_MFIOOUT(FLASH_WP)) : AND_MFIOOUT( ~IO_MFIOOUT(FLASH_WP)))
*/

void __inline FlashALE(int hi)
{
	if(hi)
		OR_MFIOOUT(IO_MFIOOUT(FLASH_ALE));
	else
		AND_MFIOOUT( ~IO_MFIOOUT(FLASH_ALE));
}

void __inline FlashCLE(int hi)
{
	if(hi)
		OR_MFIOOUT(IO_MFIOOUT(FLASH_CLE));
	else
		AND_MFIOOUT( ~IO_MFIOOUT(FLASH_CLE));
}

void __inline FlashCE(int enable)
{
	if(enable)
		OR_MFIOOUT(IO_MFIOOUT(FLASH_CE));
	else
		AND_MFIOOUT( ~IO_MFIOOUT(FLASH_CE));
}

void __inline FlashWP(int hi)
{
	if(hi)
		OR_MFIOOUT(IO_MFIOOUT(FLASH_WP));
	else
		AND_MFIOOUT( ~IO_MFIOOUT(FLASH_WP));
}

void MFIO_INIT(void)
{
	OR_TX39REG(  0x00020018, OFF_BIU_MEM_CONFIG3);	// set BIU to I/O Access mode for WE ,RE pins
	AND_TX39REG(~0x000d0040, OFF_BIU_MEM_CONFIG3);	// for NAND Flash RB pin

	OR_TX39REG( 0x00000998 , OFF_IO_MFIOSEL);		// CE,  CLE, ALE, WP : output
	AND_TX39REG( 0xfffff9ff , OFF_IO_MFIOSEL);		// WE, RE : BIU normal pin function

	OR_TX39REG( 0x0f88 , OFF_IO_MFIODIREC);			// CE, WE, RE, CLE, ALE, WP : output
	AND_TX39REG( ~0x00000010 , OFF_IO_MFIODIREC);	// RB : input

	FlashALE(LOW);
	FlashCLE(LOW);
	FlashWP(LOW);	//vincent : 20010816
	FlashCE(DISABLE);
}

#define	FlashCmdReadA			(0x00)
#define	FlashCmdReadB			(0x01)
#define	FlashCmdReadC			(0x50)
#define	FlashCmdReadSpare		(0x50)
#define FlashCmdBlockEraseSetup	(0x60)
#define FlashCmdErase			(0xD0)
#define	FlashCmdReadStatus		(0x70)
#define	FlashCmdSeqDataIn		(0x80)
#define	FlashCmdPgm				(0x10)
#define	FlashCmdReadID			(0x90)
#define FlashCmdReset			(0xff)//jmt:20010319

static void FlashCmd(unsigned char cmd)
{
	int i;
	FlashCE(ENABLE); 		// chip enable
	FlashALE(LOW);		// 
	FlashCLE(HI);		// CLE hi
	FDPort = cmd;		// Write command
	FlashCLE(LOW);		// CLE low 
	for( i =0; i<6; i++);	//for delay twb
}

static void FlashAddr(unsigned char count, unsigned char *addr)
{
	int i;
	FlashCLE(LOW);		// CLE low 
	FlashALE(HI);		// ALE hi
	while (count --)
	{
		FDPort = *(addr++);	// Write addresses
	}
	FlashALE(LOW);		// ALE pull low
	for( i =0; i<6; i++);	//for delay twb
}

static void FlashRead(unsigned int count, unsigned char *buffer/*, char terminate*/)
{
	//	To get something from flash
	unsigned char *bufferp;
	
	bufferp = buffer;
	while( !FlashReady ); 	// wait for R/B ready
	for ( ; count > 0; count--)
	{
		*(bufferp++) = FDPort;
	}
}

static int GetNANDFlashID(void)//FlashReadID(unsigned char *id)
{
	int id=0;
	unsigned char *buffer;

	buffer=(unsigned char *)&id;
	FlashCmd( FlashCmdReadID );
	buffer[0] = 0;
	FlashAddr(1, buffer);
	FlashRead(2, buffer);
	id&=0x0000ffff;

	printk("id=%x(%x,%x,%x,%x)\n",id,buffer[0],buffer[1],buffer[2],buffer[3]);

	FlashCE(DISABLE); 		// chip disable
	return id;
}

static void NANDHardReset(void)
{
	FlashCmd(FlashCmdReset);
	while( !FlashReady ) ;  // wait for R/B ready
}

static int NANDReadSpare(unsigned int PageNo,unsigned char *buf)//jmt:20011106(64M),20020128:unsigned
{ 
	unsigned char addr[4];

	addr[0] = 0;					//offset
	addr[1] = PageNo & 0xff;		// A9 - A16
	addr[2] = (PageNo>>8) &0xff;	// A17 - A24
	addr[3] = (PageNo>>16) &0xff;	// A25
	FlashCmd(FlashCmdReadC);
	if (IS64M)
		FlashAddr(4, addr);
	else
		FlashAddr(3, addr);
	FlashRead(16, buf/*, 1*/);
	FlashCE(DISABLE); 				// chip disable
	return 1; 						//1=ok
}

/*
---------------------------------------------------------------------------
; NANDReadPage(unsigned int PageNo,unsigned char *Buf)//jmt:20010327,20020128:unsigned
;   Input:
;       PageNo          (1 page == 512 bytes)
;       Buf address
;   Output:
;       1 : successful ,= 0 fail
;   Destroy:
---------------------------------------------------------------------------
*/
static int NANDReadPage(unsigned int PageNo,unsigned char *buf)//jmt:20011106(64M),20020128:unsigned
{
	unsigned char addr[4];

	addr[0] = 0;					//offset
	addr[1] = PageNo & 0xff;		// A9 - A16
	addr[2] = (PageNo>>8) &0xff;	// A17 - A24
	addr[3] = (PageNo>>16) &0xff;	// A25
	FlashCmd(FlashCmdReadA);
	if (IS64M)
		FlashAddr(4, addr);
	else
		FlashAddr(3, addr);
	FlashRead(512, buf/*, 1*/);

	FlashCE(DISABLE); 				// chip disable
	return 1;						//1=ok
}

/*
---------------------------------------------------------------------------
; NANDReadPageWithSpare(unsigned int PageNo,unsigned char *Buf)//jmt:20010327,20020128
;   Input:
;       PageNo          (1 page == 512 bytes data + 16 bytes spare)
;       Buf address
;   Output:
;       1 : successful ,= 0 fail
;   Destroy:
---------------------------------------------------------------------------
*/
static int NANDReadPageWithSpare(unsigned int PageNo, unsigned char *buf)//jmt:20011106(64M),20020128
{
	unsigned char addr[4];

	addr[0] = 0;					//offset
	addr[1] = PageNo & 0xff;		// A9 - A16
	addr[2] = (PageNo>>8) &0xff;	// A17 - A24
	addr[3] = (PageNo>>16) &0xff;	// A25
	FlashCmd(FlashCmdReadA);
	if (IS64M)
		FlashAddr(4, addr);
	else
		FlashAddr(3, addr);
	FlashRead(528, buf);

	FlashCE(DISABLE); 				// chip disable
	return 1;						//1=ok
}

static void NANDReset(void)
{
	unsigned char addr[4];
	addr[0] = 0;
	addr[1] = 0;
	addr[2] = 0;
	addr[3] = 0;
	FlashCmd(FlashCmdReadA);
	if (IS64M)
		FlashAddr(4, addr);
	else
		FlashAddr(3, addr);
	FlashRead(1, addr);
	FlashCE(DISABLE); 		// chip disable
}

/*
---------------------------------------------------------------------------
; NANDWritePageWithSpare(unsigned int PageNo,unsigned char *Buf)//jmt:20020128
;   Input:
;       PageNo          (1 page == 512 bytes data + 16 bytes spare)
;       Buf address
;   Output:
;       1 : successful ,= 0 fail
;   Destroy:
---------------------------------------------------------------------------
*/
static int NANDWritePageWithSpare(unsigned int PageNo, unsigned char *buf)//jmt:20011106(64M),20020128
{
	unsigned char	buffer[4];
	int				count ;
	unsigned char	*pbuf;	
	
	pbuf	=buf;
	//count	= 528;
	count	= 512;
	buffer[0] = 0;
	buffer[1] = PageNo & 0xff;
	buffer[2] = (PageNo>>8) & 0xff;
	buffer[3] = (PageNo>>16) & 0xff;
	FlashWP(HI);	//vincent 2001/08/15
	FlashCmd( FlashCmdSeqDataIn );
	if (IS64M)
		FlashAddr( 4, buffer);
	else
		FlashAddr( 3, buffer);
    while (count --)
    {
            FDPort = *(pbuf++);
    }
    FlashCmd( FlashCmdPgm );
    while( !FlashReady );   // wait for R/B ready
	FlashWP(LOW); //vincent 2001/08/15
    FlashCmd( FlashCmdReadStatus );
    FlashRead(1, buffer/*, 1*/);
    if (buffer[0] & 1)
		return 0;//0=fail

	return 1;//1=success
}

int flash_write_clean=0;

//////////////////////////////////////////////////////////////////

static int xflash_write_page(unsigned int pageno,unsigned char *buf)
{
	if (!NANDWritePageWithSpare(pageno, buf))
	{
		printk("error NANDWritePageWithSpare(%d)\n",pageno);
		return 0; /* error */
	}

	return 1; /* ok */
}


static int xflash_read_page(unsigned int pageno,unsigned char *buf)
{
	if (NANDReadPage(pageno,buf) == 0)
	{
		return 0; /* 0=error */
	}

	return 1; /* 0=ok */
}

static int flash_read(unsigned char *buffer,unsigned long sector,unsigned long nr_sectors)
{
	unsigned int 	i;
	unsigned char 	*pbuffer=buffer;
	
	for (i=sector; i<sector+nr_sectors; i++)
	{
		if(xflash_read_page(i,pbuffer) == 0)
			return 0; /* error */
		pbuffer	+= 512;
	}
	return 1;
}


int flash_write(unsigned char *buffer,unsigned long sector,unsigned long nr_sectors)
{
	unsigned int 	i;
	unsigned char 	*pbuffer=buffer;
	
	for (i=sector; i<sector+nr_sectors; i++)
	{
		if(xflash_write_page(i,pbuffer) == 0)
			return 0; /* error */
		pbuffer	+= 512;
	}
	return 1;
}

#define NUM_FLASHDISKS	1

static unsigned long rd_length[NUM_FLASHDISKS];	/* Size of FLASH disks in bytes   */
static int rd_hardsec[NUM_FLASHDISKS];		/* Size of real blocks in bytes */
static int rd_blocksizes[NUM_FLASHDISKS];	/* Size of 1024 byte blocks :)  */
static int rd_kbsize[NUM_FLASHDISKS];		/* Size in blocks of 1024 bytes */

static void flash_request(void)//jmt:20011106(64M)
{
	unsigned int minor;
	unsigned long offset, len;

	struct request *req;//jmt:20010302
	unsigned int result=0;//jmt:20010302
	
repeat:
	INIT_REQUEST;
	req=CURRENT;

	spin_unlock_irq(&io_request_lock);
	
	minor = MINOR(req->rq_dev);
	if (minor >= NUM_FLASHDISKS) {
		result=0;//end_request(0); /* error */
		goto end_req;//goto repeat;
	}
	
	offset = req->sector << RDBLK_SIZE_BITS;
	len = req->current_nr_sectors << RDBLK_SIZE_BITS;

	if ((offset + len) > rd_length[minor]) {
		result=0;//end_request(0); /* error */
		goto end_req;//goto repeat;
	}

	if ((req->cmd != READ) && (req->cmd != WRITE)) {
		printk("FLASHDISK: bad command: %d\n", req->cmd);
		result=0;//end_request(0);
		goto end_req;//goto repeat;
	}

	if (req->cmd == READ) 
	{
		if(flash_read(req->buffer,req->sector,req->current_nr_sectors) == 0)
		{
			printk("?");
			result=0;		//end_request(0); /* error */
			goto end_req;	//goto repeat;
		}

	}
	else//WRITE:	
	{	
		if(flash_write(req->buffer,req->sector,req->current_nr_sectors) == 0)
		{
			printk("!");
			result=0;		//end_request(0); /* error */
			goto end_req;	//goto repeat;
		}
		
	}

	result=1;//1=ok
	
end_req:
	spin_lock_irq(&io_request_lock);
	end_request(result);
	goto repeat;
} 

/////***************** driver functions ****************************************************////////

static int flash_open(struct inode * inode, struct file * filp)
{
	if (DEVICE_NR(inode->i_rdev) >= NUM_FLASHDISKS)
		return -ENXIO;

	//MOD_INC_USE_COUNT;

	return 0;
}

static int flash_release(struct inode * inode, struct file * filp)
{
	struct super_block * sb;
	
	if (inode == NULL)
		return(-ENODEV);
	
	sb = get_super(inode->i_rdev);//>=2.2.10
	fsync_dev(inode->i_rdev);//>=2.2.10
	if (sb) invalidate_inodes(sb);//>=2.2.10
	invalidate_buffers(inode->i_rdev);//>=2.2.10

	//MOD_DEC_USE_COUNT;

	return 0;
}
unsigned char io528[528];
static int fl_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	struct super_block * sb;
	unsigned int minor;
	unsigned short block;
	unsigned int sector;//sector usage
	struct fl_io *flio=(struct fl_io *)arg;
	struct fl_info *flinfo=(struct fl_info *)arg;//20010316
	struct fl_info flinfox;
	int err;

	if (!inode || !inode->i_rdev) 	
		return -EINVAL;

	minor = MINOR(inode->i_rdev);

	switch (cmd)
	{
	
		case FLIO_READ:
			if (flio==NULL) return -EINVAL;
			err = verify_area(VERIFY_WRITE, flio, sizeof(*flio));
			if (err) return err;

			get_user(sector, &flio->sector);

			err=(NANDReadPageWithSpare(sector,io528) ? 0:1);

			copy_to_user(&flio->buf528, io528, sizeof(io528));
			return err;

		case FLIO_WRITE:
			if (flio==NULL) return -EINVAL;
			err = verify_area(VERIFY_WRITE, flio, sizeof(*flio));
			if (err) return err;

			get_user(sector, &flio->sector);
			copy_from_user(io528, &flio->buf528, sizeof(io528));

			err=(NANDWritePageWithSpare(sector,io528) ? 0:1);

			return err;
#if 0			
		case FLIOGET_INFO:
			if (flinfo==NULL) return -EINVAL;
			err = verify_area(VERIFY_WRITE, flinfo, sizeof(*flinfo));
			if (err) return err;

			//jmt: note: flash_size isn't correct before "mkflash"
			flinfox.flash_spare_size=16;
			flinfox.flash_sector_per_block=32;
			flinfox.flash_pageshift_per_block=5;
			flinfox.flash_k_per_block=16;
			flinfox.flash_start_block=0; 
			flinfox.flash_start_sector=0; 
			flinfox.flash_max_block=2048;
			flinfox.flash_max_sector=65536;
			flinfox.flash_free_sector=0xffffffff;
			flinfox.flash_bad_sector=0xfffffffe; 
			flinfox.flash_reserved_sector=0xfffffffd; 
			flinfox.flash_super_sector=0xffa5a5; 

			copy_to_user(flinfo, &flinfox, sizeof(flinfox));

			return 0;
#endif			
/*
		case FLIOGET_ORIGPPID: //jmt:20011119
		{
			int pid;
			struct task_struct *p;

			if (!arg)  return -EINVAL;
			get_user(pid, (int *) arg);
			for_each_task(p)
			{
				if(p->pid == pid)
				{
					return p->jmpid;
				}
			}
			return 0;
		}
*/
		case BLKFLSBUF:
			if (!capable(CAP_SYS_ADMIN)) return -EACCES;
			sb = get_super(inode->i_rdev);//>=2.2.10
			fsync_dev(inode->i_rdev);//>=2.2.10
			if (sb) invalidate_inodes(sb);//>=2.2.10
			invalidate_buffers(inode->i_rdev);//>=2.2.10
			break;

		case BLKGETSIZE:   /* Return device size */
			if (!arg)  return -EINVAL;
			printk("ioctl BLKGETSIZE size=%d minor=%d\n", rd_length[minor], minor);
			return put_user(rd_length[minor] >> RDBLK_SIZE_BITS, (long *) arg);

		case BLKSSZGET:	   /* Block size of media */
			if (!arg)  return -EINVAL;
			return put_user(rd_blocksizes[minor], (int *)arg);
			
		default:
			printk("Unsupport IOCTL in flashdrv %d\n", cmd);

	}
	
	return -1;
}

static struct block_device_operations fd_fops = {
	owner:					THIS_MODULE,
	open:					flash_open,
	release:				flash_release,
	ioctl:					fl_ioctl,
};

int __init fl_init(void)
{
	int	i;
	
////////////// coded by jmt /////////////
	MFIO_INIT();
	printk("\nfl_init(): ");

	if (register_blkdev(MAJOR_NR, "flashdisk", &fd_fops)) {
		printk("FLASHDISK: Could not get major %d", MAJOR_NR);
		return -EIO;
	}

	printk("NANDHardReset...");
	NANDHardReset();
	printk("ok\n");

	blk_init_queue(BLK_DEFAULT_QUEUE(MAJOR_NR), flash_request);

	for (i = 0; i < NUM_FLASHDISKS; i++) 
	{
		/* flash_size is given in kB */
		rd_length[i] = (32*1024*1024 >> 10 << BLOCK_SIZE_BITS);
		rd_hardsec[i] = RDBLK_SIZE;
		rd_blocksizes[i] = 1024;
		rd_kbsize[i] = (rd_length[i] >> BLOCK_SIZE_BITS);
		
		printk("i=%d len=%d sec=%d bsize=%d kbsize=%d\n", i, rd_length[i], rd_hardsec[i], rd_blocksizes[i], rd_kbsize[i]);
	}

	hardsect_size[MAJOR_NR] = rd_hardsec;   /* Size of the FLASH disk blocks */
	blksize_size[MAJOR_NR] = rd_blocksizes; /* Avoid set_blocksize() check */
	blk_size[MAJOR_NR] = rd_kbsize;	   /* Size of the FLASH disk in kB  */

	printk("FlashDisk v7k [XIP_ZIP_FLASH]: gp1288 build 20020716\n");
	
	return 0;
}

