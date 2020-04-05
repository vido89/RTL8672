
/* ioctls
   0xf1 is 'fl'ash							*/
#define FLIOGET_ID		0xf100
#define FLIO_ERASE		0xf101
#define FLIO_READ		0xf102
#define FLIO_WRITE		0xf103
#define FLIO_RELOADMAP		0xf104
#define FLIOGET_EXTRABAD	0xf105
#define FLIOGET_RESERVEDLEFT	0xf106
#define FLIO_TESTWRITEFAIL	0xf107
#define FLIO_TESTERASEFAIL	0xf108
#define FLIOGET_INFO		0xf109

#define FLIOSET_XVERINATMEL	0xf10a//20010322//off
#define FLIOGET_XVERINATMEL	0xf10b//20010322//off

#define FLIOGET_COUNT		0xf10c//20010327
#define FLIOSET_WRITESTAT	0xf10d//20010403
#define FLIOGET_WRITESTAT	0xf10e//20010403

#define FLIOSET_BEGINWRITENAND	0xf10f//20010410//hg120
#define FLIOSET_ENDWRITENAND	0xf110//20010410//hg120

#define FLIO_XBOOTROM		0xf111//20010417
#define FLIO_FREEFLASHMAP	0xf112//jmt:20011114
#define FLIOGET_ORIGPPID	0xf113//jmt:20011119
#define FLIOGET_VCCLOW		0xf114//jmt:20011221
#define FLIOSET_RO		0xf115//jmt:20020319
#define FLIOSET_RW		0xf116//jmt:20010319

struct fl_count
{
	unsigned long long count;//20010327
};

struct fl_io
{
	unsigned int sector;//64M
	char buf528[528];
};

struct fl_info
{
	//jmt: note: flash_size isn't correct before "mkflash"
	unsigned short flash_spare_size;
	unsigned short flash_sector_per_block;
	unsigned short flash_pageshift_per_block;
	unsigned short flash_k_per_block;
	unsigned short flash_start_block; /* skip kernel area */
	unsigned int flash_start_sector; /* 64M,skip kernel area */
	unsigned short flash_max_block;
	unsigned int flash_max_sector;//64M
	unsigned int flash_free_sector;//64M
	unsigned int flash_bad_sector; /* 64M,bad block area */
	unsigned int flash_reserved_sector; /* 64M,reserved blocks/sectors */
	unsigned int flash_super_sector; /* 64M,flash_super blocks/sectors */
};

