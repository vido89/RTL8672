#include "spi_flash.h"
#include <linux/mtd/map.h>
#include <linux/mtd/gen_probe.h>
#include <linux/spinlock.h>

//#define SPI_DEBUG
//#define MTD_SPI_SUZAKU_DEBUG

//set system clock and spi control registers according chip type
unsigned int MHZ, SYSCLK;
extern unsigned int SFCR, SFCSR, SFDR;
#define CHIPVERMASK (0xFFF00000)
#define IC8672	    (0xFFF00000)
#define IC8671B     (0xFFE00000)
#define IC0315      (0xFFD00000)
#define IC6166      (0xFFC00000)
#define SCCR   	     0xb8003200

/* SPI Flash Controller */
unsigned char ICver=0;
unsigned int SFCR=0;
unsigned int SFCSR=0;
unsigned int SFDR=0;

// patch for flashsize > 4MiB
#if defined(CONFIG_16M_FLASH) || defined(CONFIG_8M_FLASH)
#define FLASH_START_ADDR 0xbd000000
#else
#define FLASH_START_ADDR 0xbfc00000
#endif

#define LENGTH(i)       SPI_LENGTH(i)
#define CS(i)           SPI_CS(i)
#define RD_ORDER(i)     SPI_RD_ORDER(i)
#define WR_ORDER(i)     SPI_WR_ORDER(i)
#define READY(i)        SPI_READY(i)
#define CLK_DIV(i)      SPI_CLK_DIV(i)
#define RD_MODE(i)      SPI_RD_MODE(i)
#define SFSIZE(i)       SPI_SFSIZE(i)
#define TCS(i)          SPI_TCS(i)
#define RD_OPT(i)       SPI_RD_OPT(i)

/*
 * SPI Flash Info
 */
struct spi_flash_type   spi_flash_info[2];

/*
 * SPI Flash Info
 */
const struct spi_flash_db   spi_flash_known[] =
{
   {0x01, 0x02,   1}, /* Spansion */
   {0xC2, 0x20,   0}, /* MXIC */
   {0xC2, 0x5E,   0}, /* MXIC high performance */
   {0x1C, 0x31,   0}, /* EON */
   {0x8C, 0x20,   0}, /* F25L016A ESMT*/
   {0xEF, 0x30,   0}, /* W25X16 Winbond*/
   {0x1F, 0x46,   0}, /* AT26DF161 ATMEL*/
   {0xBF, 0x25,   0}, /* 25VF016B-50-4c-s2AF SST*/
   {0x1F, 0x47,   0}, /* AT25DF321 ATMEL*/
   {0x01, 0x20,   0}, /* S25FL128P */
   {0x9F, 0x00,   0}, /* GD25Q16SCP */
   {0xC8, 0x40,   0},  /*gigadevice GD25Q16 */
};

//type of SPI flash we support
static const struct spi_flash_info flash_tables[13] = {

	{
		mfr_id: SPANSION,
		dev_id: SPI,
		name: "spansion",
		DeviceSize: SIZE_2MiB,
		EraseSize: SIZE_64KiB,
	},
// Support for MX2fL series flash 
	{
		mfr_id: 0xC2,
		dev_id: 0x20,
		name: "mxic",
		DeviceSize: 0x200000,
		EraseSize: 4096,
	},
// Support EON Flash
	{
		mfr_id: 0x1C,
		dev_id: 0x31,
		name: "EON",
		DeviceSize: 0x200000,
		EraseSize: 4096,
	},	
// Support for MX high performace series flash 
	{
		mfr_id: 0xC2,
		dev_id: 0x5E,
		name: "mxic",
		DeviceSize: 0x400000,
		EraseSize: 4096,
	},
// Support ATMEL Flash
	{
		mfr_id: 0x1F,
		dev_id: 0x46,
		name: "ATMEL",
		DeviceSize: 0x200000,
		EraseSize: 4096,
	},	
// Support for Winbond series flash 
	{
		mfr_id: 0xEF,
		dev_id: 0x30,
		name: "Winbond",
		DeviceSize: 0x200000,
		EraseSize: 4096,
	},
// Support for ESMT series flash 
	{
		mfr_id: 0x8C,
		dev_id: 0x20,
		name: "A-Link",
		DeviceSize: 0x200000,
		EraseSize: 4096,
	},
// Support for SST series flash 
	{
		mfr_id: 0xBF,
		dev_id: 0x25,
		name: "A-Link",
		DeviceSize: 0x200000,
		EraseSize: 4096,
	},
// Support ATMEL Flash
        {
                mfr_id: 0x1F,
                dev_id: 0x47,
                name: "ATMEL",
                DeviceSize: 0x400000,
                EraseSize: 4096,
        },
// Support for  S25FL128P flash
        {
                mfr_id: 0x01,
                dev_id: 0x20,
                name: "S25FL128P",
                DeviceSize: 0x400000,
                EraseSize: 4096,
        },
// Support for GD25Q16SCP series flash 
	{
		mfr_id: 0x9F,
		dev_id: 0x00,
		name: "GD25Q16SCP",
		DeviceSize: 0x200000,
		EraseSize: 4096,
	},
// Support for Unknow series flash 
	{
		mfr_id: 0xFF,
		dev_id: 0xFF,
		name: "Unknow SPI",
		DeviceSize: 0x200000,
		EraseSize: 4096,
	},
//Support for Gigadevice GD25Q16
	{
	   	mfr_id: 0xC8,
	   	dev_id: 0x40,
	   	name:  "Gigadevice",
	   	DeviceSize: SIZE_2MiB,
	   	EraseSize: 4096,
	},	
};

/*
 * SPI Flash APIs
 */

/*
 * This function shall be called when switching from MMIO to PIO mode
 */
void spi_pio_init(void)
{
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(0) | READY(1);

   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(0) | READY(1);

   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
}

void spi_pio_init_ready(void)
{
   spi_ready();
}

void spi_pio_toggle1(void)
{
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
}
void spi_pio_toggle2(void)
{
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);
}

void spi_read(unsigned int chip, unsigned int address, unsigned int *data_out)
{
   /* De-Select Chip */
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

   /* RDSR Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);

   *(volatile unsigned int *) SFDR = 0x05 << 24;

   while (1)
   {
      unsigned int status;

      status = *(volatile unsigned int *) SFDR;

      /* RDSR Command */
      if ( (status & 0x01000000) == 0x00000000)
      {
         break;
      }
   }

   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* READ Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);

   *(volatile unsigned int *) SFDR = (0x03 << 24) | (address & 0xFFFFFF);

   /* Read Data Out */
   *data_out = *(volatile unsigned int *) SFDR;

   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
}

 void spi_erase_chip(unsigned int chip)
{
   /* De-select Chip */
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* RDSR Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
   *(volatile unsigned int *) SFDR = 0x05 << 24;

   while (1)
   {
      /* RDSR Command */
      if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
      {
         break;
      }
   }

   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* WREN Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
   *(volatile unsigned int *) SFDR = 0x06 << 24;
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* BE Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
   *(volatile unsigned int *) SFDR = (0xC7 << 24);
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
}

 void spi_ready(void)
{
    while(1)
    {
	if ( (*(volatile unsigned int *) SFCSR) & READY(1))
	    break;
    }
}

void spi_cp_probe(void)
{
   unsigned int cnt, i;
   unsigned int temp;

   for (cnt = 0; cnt < 2; cnt++)
   {
      /* Here set the default setting */
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);

      /* One More Toggle (May not Necessary) */
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);

      /* RDID Command */
      *(volatile unsigned int *) SFDR = 0x9F << 24;
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+cnt) | READY(1);
      temp = *(volatile unsigned int *) SFDR;

      spi_flash_info[cnt].maker_id = (temp >> 24) & 0xFF;
      spi_flash_info[cnt].type_id = (temp >> 16) & 0xFF;
      spi_flash_info[cnt].capacity_id = (temp >> 8) & 0xFF;
      spi_ready();

      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

      /* Iterate Each Maker ID/Type ID Pair */
      for (i = 0; i < sizeof(spi_flash_known) / sizeof(struct spi_flash_db); i++)
      {
         if ( (spi_flash_info[cnt].maker_id == spi_flash_known[i].maker_id) &&
              (spi_flash_info[cnt].type_id == spi_flash_known[i].type_id) )
         {
            spi_flash_info[cnt].device_size = (unsigned char)((signed char)spi_flash_info[cnt].capacity_id + spi_flash_known[i].size_shift);
         }
	  else
	     spi_flash_info[cnt].device_size = (unsigned char)((signed char)spi_flash_info[cnt].capacity_id); 
      }

      spi_flash_info[cnt].sector_cnt = 1 << (spi_flash_info[cnt].device_size - 16);
   }
   for(i=0;i<2;i++){
	printk("maker:%x  type:%x  sector_cnt:%d\n",spi_flash_info[i].maker_id,spi_flash_info[i].type_id,spi_flash_info[i].sector_cnt);
   }
}


#if defined(MTD_SPI_SUZAKU_DEBUG)
#define KDEBUG(args...) printk(args)
#else
#define KDEBUG(args...)
#endif

#define write32(a, v)       __raw_writel(v, a)
#define read32(a)           __raw_readl(a)

/* Proto-type declarations */
static u8 spi_read_status(void);
static void spi_set_cs(u32);

#define SPI_ERASING 1

static int spi_state = 0;
static spinlock_t spi_mutex = SPIN_LOCK_UNLOCKED;

/**
 * select which cs (chip select) line to activate
 */
inline static void spi_set_cs(u32 cs)
{
}

 static u32 spi_copy_to_dram(const u32 from, const u32 to, const u32 size)
{
	memcpy(to,from|FLASH_START_ADDR ,size);
    return 0;
}




static u32 do_spi_read(u32 from, u32 to, u32 size)
{
	u32 ret;
        if (from>0x10000)
            size=(size<=1024)?size:1024;
        else
            size=(size<=4096)?size:4096;
	ret = spi_copy_to_dram(from, to, size);
	spi_pio_init();

   return ret;
}


static u32 do_spi_write(u32 from, u32 to, u32 size)
{
   unsigned int temp;
	unsigned int  remain;
   unsigned int cur_addr;
   unsigned int cur_size ,flash_addr;
   unsigned int cnt;
   unsigned int next_page_addr;
 
   cur_addr = from;
   flash_addr = to;
   cur_size = size;

#ifdef SPI_DEBUG
	printk("\r\n do_spi_write : from :[%x] to:[%x], size:[%x]  ", from, to, size);
#endif

   	   spi_pio_init();
      next_page_addr = ((flash_addr >> 8) +1) << 8;

      while (cur_size > 0)
      {
	   /* De-select Chip */
	   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
	  
         /* WREN Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
         *(volatile unsigned int *) SFDR = 0x06 << 24;
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
         /* PP Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);
         *(volatile unsigned int *) SFDR = (0x02 << 24) | (flash_addr & 0xFFFFFF);

	   while (flash_addr != next_page_addr)
	   {
		remain = (cur_size > 4)?4:cur_size;		
		temp = *((int*)cur_addr);
		
            spi_ready();
			
            *(volatile unsigned int *) SFCSR = LENGTH(remain-1) | CS(1) | READY(1);                     
            *(volatile unsigned int *) SFDR = temp;
		
		cur_size -= remain;
		cur_addr += remain;
		flash_addr+=remain;
		
            if (cur_size == 0)
               break;;
	   }
		next_page_addr = flash_addr + 256;
         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

         /* RDSR Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
         *(volatile unsigned int *) SFDR = 0x05 << 24;

         cnt = 0;
         while (1)
         {
            unsigned int status = *(volatile unsigned int *) SFDR;

            /* RDSR Command */
            if ((status & 0x01000000) == 0x00000000)
            {
                break;
            }

            if (cnt > 200000)
            {
            		return -EINVAL;
            }
            cnt++;
         }

         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
      }
    return 0;
}




/*Notice !!!
 * To comply current design, the erase function will implement sector erase
*/
static int do_spi_erase(u32 addr)
{
	int chip=0;

#ifdef SPI_DEBUG
	printk("\r\n do_spi_erase : [%x] ", addr);
#endif
	spi_pio_init();
	
      /* WREN Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);

      *(volatile unsigned int *) SFDR = 0x06 << 24;
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

      /* SE Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = (0x20 << 24) | addr;
//	  *(volatile unsigned int *) SFDR = (0xD8 << 24) | addr;
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

      /* RDSR Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = 0x05 << 24;

      while (1)
      {
         /* RDSR Command */
         if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
         {
            break;
         }
      }

      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
    return 0;
}
/*
 The Block Erase function
*/
static int do_spi_block_erase(u32 addr)
{
	int chip=0;

#ifdef SPI_DEBUG
	printk("\r\n do_spi_block_erase : [%x] ", addr);
#endif
	spi_pio_init();
	
      /* WREN Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);

      *(volatile unsigned int *) SFDR = 0x06 << 24;
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

      /* SE Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = (0xD8 << 24) | addr;
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

      /* RDSR Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = 0x05 << 24;

      while (1)
      {
         /* RDSR Command */
         if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
         {
            break;
         }
      }

      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

    return 0;
}

static u32 do_spi_block_write(u32 from, u32 to, u32 size)
{
	unsigned char *ptr;

	//don't support write through 1st block
	if ((to < SIZE_64KiB) && ((to+size) > SIZE_64KiB))
		return -EINVAL;
	if (to < SIZE_64KiB)
	{
		ptr = kmalloc(SIZE_64KiB,GFP_KERNEL );
		if (!ptr)
			return -EINVAL;
		memcpy(ptr,FLASH_START_ADDR, SIZE_64KiB);
		do_spi_block_erase(0); // erase 1 sector
		memcpy(ptr+to,from , size);
		do_spi_write(ptr, 0 , SIZE_64KiB);
		kfree(ptr);
		return  0 ;
	}
	else 
		return do_spi_write(from , to, size);
}

static struct spi_chip_info *spi_suzaku_setup(struct map_info *map)
{
	struct spi_chip_info *chip_info;

	chip_info = kmalloc(sizeof(*chip_info), GFP_KERNEL);
	if (!chip_info) {
		printk(KERN_WARNING "Failed to allocate memory for MTD device\n");
		return NULL;
	}

	memset(chip_info, 0, sizeof(struct spi_chip_info));

	return chip_info;
}
static void spi_suzaku_destroy(struct spi_chip_info *chip_info)
{
	printk("spi destroy!\n");
}

//tylo, for 8671b, test IC version
void checkICverSPI(void){
       unsigned int sccr, ICver;

       sccr=*(volatile unsigned int*)SCCR;
       if ((sccr&CHIPVERMASK)==IC8671B)
       {//ic8671B(6085 with packet processor)
               ICver = IC8671B;
       }
       else if ((sccr&CHIPVERMASK)==IC0315)
       {//ic0315 //adjust clock frequency
               ICver = IC0315;
       }
       else if(( sccr & CHIPVERMASK) == IC8672)
       {//ic8672  Ver A & B
               ICver =IC8672;
       }
       else if(( sccr & CHIPVERMASK) == IC6166)
       {//6166  
               ICver =IC6166;
       }
       else
       {//unknown chip type, regard as IC6166
               ICver =IC6166;
       }

       switch (ICver)
       {
       case IC8671B://6085
               MHZ=175;
               SFCR = 0xB8001200;
               SFCSR = 0xB8001208;
               SFDR = 0xB800120C;
               break;
       case IC0315:
       case IC6166:
               MHZ=180;
               SFCR = 0xB8001200;
               SFCSR = 0xB8001208;
               SFDR = 0xB800120C;
               break;
       case IC8672://6028
       default:
               MHZ=175;
               SFCR = 0xB8001200;
               SFCSR= 0xB8001204;
               SFDR = 0xB8001208;
       }
       SYSCLK = MHZ*1000*1000;
}

struct spi_chip_info *spi_probe_flash_chip(struct map_info *map, struct chip_probe *cp)
{
	int i;
	struct spi_chip_info *chip_info = NULL;

	checkICverSPI();
	spi_pio_init();

	*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR & 0x1fffffff;
	*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR |SPI_CLK_DIV(2);
	*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR  &(~(1<<26));

	spi_cp_probe();

	for (i=0; i < (sizeof(flash_tables)/sizeof(struct spi_flash_info)); i++) {
		printk("maker ID %x  type ID  %x",spi_flash_info[0].maker_id ,spi_flash_info[0].type_id );
		if ( (spi_flash_info[0].maker_id == spi_flash_known[i].maker_id) &&
              		(spi_flash_info[0].type_id == spi_flash_known[i].type_id) ) {
			chip_info = spi_suzaku_setup(map);
			if (chip_info) {
				chip_info->flash      = &flash_tables[i];
				if (spi_flash_info[0].maker_id == 0xC2){
					printk("\r\nMXIC matched!!");
					chip_info->flash->DeviceSize = 1 << spi_flash_info[0].capacity_id;
				}else if(spi_flash_info[0].maker_id == 0x1C ){					
					chip_info->flash->DeviceSize = 1 << spi_flash_info[0].capacity_id;
					printk("\r\nEON matched!!\n");
				}else if(spi_flash_info[0].maker_id == 0x1F ){					
					chip_info->flash->DeviceSize = 1 << spi_flash_info[0].capacity_id;
					printk("\r\nATMEL matched!!\n");
				}else if(spi_flash_info[0].maker_id == 0xEF ){					
					chip_info->flash->DeviceSize = 1 << spi_flash_info[0].capacity_id;
					printk("\r\nWinbond matched!!\n");
				}else if(spi_flash_info[0].maker_id == 0xBF ){					
					chip_info->flash->DeviceSize = 1 << spi_flash_info[0].capacity_id;
					printk("\r\nSST matched!!\n");
				}else if(spi_flash_info[0].maker_id == 0x8C ){					
					chip_info->flash->DeviceSize = 1 << spi_flash_info[0].capacity_id;
					printk("\r\nESMT matched!!\n");
				}else {
					spi_flash_info[0].maker_id=0xFF;
					chip_info->flash->DeviceSize = 1 << spi_flash_info[0].capacity_id;
					printk("\r\nFlash type not found use defaults\n");
				}
				
				chip_info->destroy    = spi_suzaku_destroy;

				chip_info->read       = do_spi_read;
				
				if (flash_tables[i].EraseSize == 4096) //sector or block erase
				{
					chip_info->erase      = do_spi_erase;
					chip_info->write      = do_spi_write;
				}
				else
				{
					chip_info->erase      = do_spi_block_erase;
					chip_info->write      = do_spi_block_write;
				}
			}		
			printk("get SPI chip driver!\n");			
			return chip_info;
		}
		else{
			printk("Can not match SPI chip driver [%d] !\n",i);
		}
		if(i == (sizeof(flash_tables)/sizeof(struct spi_flash_info))-1)
		{
			chip_info = spi_suzaku_setup(map);
			if (chip_info) {
				chip_info->flash      = &flash_tables[i];
				chip_info->flash->DeviceSize = 1 << spi_flash_info[0].capacity_id;
				printk("\r\nUnknow SPI flash!!\n");
				
				chip_info->destroy    = spi_suzaku_destroy;

				chip_info->read       = do_spi_read;
				
				if (flash_tables[i].EraseSize == 4096) //sector or block erase
				{
					chip_info->erase      = do_spi_erase;
					chip_info->write      = do_spi_write;
				}
				else
				{
					chip_info->erase      = do_spi_block_erase;
					chip_info->write      = do_spi_block_write;
				}
			}
			printk("get unknow SPI chip driver!\n");
		    return chip_info;
		}
			
	}
    return NULL;
}
EXPORT_SYMBOL(spi_probe_flash_chip);
