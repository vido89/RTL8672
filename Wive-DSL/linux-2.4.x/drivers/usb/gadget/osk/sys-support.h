
//
// sys-support.h
//

#ifndef OTG_SYS_SUPPORT_H
#define OTG_SYS_SUPPORT_H

#include "global.h"
#include "xlinux.h"
#include "board.h"

#define __devinit
#define __exit

#undef VERBOSE
#undef DEBUGOTGON
//#define VERBOSE
//#define DEBUGOTGON
#ifdef DEBUGOTGON
#define DEBUGOTG(args...) xprintf(args)
#else
#define	DEBUGOTG(args...)
#endif

#define CONFIG_KERNELVERSION "2.6.19.7"
#define CONFIG_USB_GADGET_DUALSPEED 1

#ifdef __CHECKER__
#define __bitwise__ __attribute__((bitwise))
#else
#define __bitwise__
#endif
typedef unsigned __bitwise__ gfp_t;

#ifdef __CHECKER__
#define __bitwise __bitwise__
#else
#define __bitwise
#endif

typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

typedef __signed__ long __s64;
typedef unsigned long __u64;

#define uint8_t unsigned char
#define uint32_t unsigned int
#define uint16_t unsigned short
#define int32_t int
#define int16_t short
#define int8_t char

typedef __u16 __bitwise __le16;
typedef __u16 __bitwise __be16;
typedef __u32 __bitwise __le32;
typedef __u32 __bitwise __be32;

#define atomic_t  unsigned int
#define spinlock_t int

#define GFP_ATOMIC	0

#define writeb(b,addr)	((*(volatile unsigned char *)(addr)) = ((unsigned char)(b)))
#define writew(b,addr)	((*(volatile unsigned short *)(addr)) = ((unsigned short)(b)))
#define writel(b,addr)	((*(volatile unsigned long *)(addr)) = ((unsigned long)(b)))

#define readb(addr)		((*(volatile unsigned char *)(addr)))
#define readw(addr)		((*(volatile unsigned short *)(addr)))
#define readl(addr)		((*(volatile unsigned int *)(addr)))

//#if defined(_LITTLE_ENDIAN_)

#define htons(x) \
		((unsigned short)((((unsigned short)(x) & 0x00FFU) << 8) | \
			(((unsigned short)(x) & 0xFF00U) >> 8)))	

#define htonl(x) \
		((unsigned long)((((unsigned long)(x) & 0x000000FFU) << 24) | \
			(((unsigned long)(x) & 0x0000FF00U) <<  8) | \
			(((unsigned long)(x) & 0x00FF0000U) >>  8) | \
			(((unsigned long)(x) & 0xFF000000U) >> 24)))

#define ntohs(x) htons(x)
#define ntohl(x) htonl(x)

#define __force

#define le16_to_cpu(x) ntohs(x)
#define le32_to_cpu(x) ntohl(x)
#define le16_to_cpup(x) ntohs(*(x))
#define le32_to_cpup(x) ntohl(*(x))

#define cpu_to_le16(x) htons(x)
#define cpu_to_le32(x) htonl(x)

#define ___constant_swab16(x) \
	((__u16)( \
		(((__u16)(x) & (__u16)0x00ffU) << 8) | \
		(((__u16)(x) & (__u16)0xff00U) >> 8) ))
#define ___constant_swab32(x) \
	((__u32)( \
		(((__u32)(x) & (__u32)0x000000ffUL) << 24) | \
		(((__u32)(x) & (__u32)0x0000ff00UL) <<  8) | \
		(((__u32)(x) & (__u32)0x00ff0000UL) >>  8) | \
		(((__u32)(x) & (__u32)0xff000000UL) >> 24) ))

#define __constant_cpu_to_le32(x) ((__force __le32)___constant_swab32((x)))
#define __constant_le32_to_cpu(x) ___constant_swab32((__force __u32)(__le32)(x))
#define __constant_cpu_to_le16(x) ((__force __le16)___constant_swab16((x)))
#define __constant_le16_to_cpu(x) ___constant_swab16((__force __u16)(__le16)(x))


//#else 
#if 0
#define htons(x) (x)
#define htonl(x) (x)
//#define ntohs(x) htons(x)
//#define ntohl(x) htonl(x)

#define le16_to_cpu(x) (x)
#define le32_to_cpu(x) (x)
#define le16_to_cpup(x) (*(x))
#define le32_to_cpup(x) (*(x))

#define cpu_to_le16(x) (le16_to_cpu(x))
#define cpu_to_le32(x) (le32_to_cpu(x))
#define be16_to_cpu(x) ntohs(x)
#define cpu_to_be16(x) htons(x)

 #define __constant_cpu_to_le16(x)	cpu_to_le16(x)
#define __constant_cpu_to_le32(x)	cpu_to_le32(x)
#endif


/*
 * What is the most efficient way of loading/storing an unaligned value?
 *
 * That is the subject of this file.  Efficiency here is defined as
 * minimum code size with minimum register usage for the common cases.
 * It is currently not believed that long longs are common, so we
 * trade efficiency for the chars, shorts and longs against the long
 * longs.
 *
 * Current stats with gcc 2.7.2.2 for these functions:
 *
 *	ptrsize	get:	code	regs	put:	code	regs
 *	1		1	1		1	2
 *	2		3	2		3	2
 *	4		7	3		7	3
 *	8		20	6		16	6
 *
 * gcc 2.95.1 seems to code differently:
 *
 *	ptrsize	get:	code	regs	put:	code	regs
 *	1		1	1		1	2
 *	2		3	2		3	2
 *	4		7	4		7	4
 *	8		19	8		15	6
 *
 * which may or may not be more efficient (depending upon whether
 * you can afford the extra registers).  Hopefully the gcc 2.95
 * is inteligent enough to decide if it is better to use the
 * extra register, but evidence so far seems to suggest otherwise.
 *
 * Unfortunately, gcc is not able to optimise the high word
 * out of long long >> 32, or the low word from long long << 32
 */

#define __get_unaligned_2(__p)					\
	(__p[0] << 8 | __p[1] )

#define __get_unaligned_4(__p)					\
	(__p[0] <<24 | __p[1] <<16 | __p[2] << 8 | __p[3] )

#define get_unaligned(ptr)					\
	({							\
		__typeof__(*(ptr)) __v;				\
		__u8 *__p = (__u8 *)(ptr);			\
		switch (sizeof(*(ptr))) {			\
		case 1:	__v = *(ptr);			break;	\
		case 2: __v = __get_unaligned_2(__p);	break;	\
		case 4: __v = __get_unaligned_4(__p);	break;	\
		case 8: {					\
				unsigned int __v1, __v2;	\
				__v2 = __get_unaligned_4((__p)); \
				__v1 = __get_unaligned_4((__p+4));	\
				__v = ((unsigned long long)__v2 << 32 | __v1);	\
			}					\
			break;					\
		default:					\
			break;					\
		}						\
		__v;						\
	})

static inline void __put_unaligned_2(__u32 __v, register __u8 *__p)
{
	*__p++ = __v>>8;
	*__p++ = __v;
}

static inline void __put_unaligned_4(__u32 __v, register __u8 *__p)
{
	__put_unaligned_2(__v >>16, __p );
	__put_unaligned_2(__v , __p +2);
}

static inline void __put_unaligned_8(const unsigned long long __v, register __u8 *__p)
{
	/*
	 * tradeoff: 8 bytes of stack for all unaligned puts (2
	 * instructions), or an extra register in the long long
	 * case - go for the extra register.
	 */
	__put_unaligned_4(__v >> 32, __p);
	__put_unaligned_4(__v, __p+4);
}

#define __put_unaligned_bad_length()

/*
 * Try to store an unaligned value as efficiently as possible.
 */
#define put_unaligned(val,ptr)					\
	({							\
		switch (sizeof(*(ptr))) {			\
		case 1:						\
			*(ptr) = (val);				\
			break;					\
		case 2: __put_unaligned_2((val),(__u8 *)(ptr));	\
			break;					\
		case 4:	__put_unaligned_4((val),(__u8 *)(ptr));	\
			break;					\
		case 8:	__put_unaligned_8((val),(__u8 *)(ptr)); \
			break;					\
		default:						\
			 __put_unaligned_bad_length();		\ 
			break;					\
		}						\
		(void) 0;					\
	})


static int utf8_to_utf16le(const char *s, __le16 *cp, unsigned len)
{
	int	count = 0;
	u8	c;
	u16	uchar;

	/* this insists on correct encodings, though not minimal ones.
	 * BUT it currently rejects legit 4-byte UTF-8 code points,
	 * which need surrogate pairs.  (Unicode 3.1 can use them.)
	 */
	while (len != 0 && (c = (u8) *s++) != 0) {
//		if (unlikely(c & 0x80)) {
		if(c&0x80){
			// 2-byte sequence:
			// 00000yyyyyxxxxxx = 110yyyyy 10xxxxxx
			if ((c & 0xe0) == 0xc0) {
				uchar = (c & 0x1f) << 6;

				c = (u8) *s++;
				if ((c & 0xc0) != 0xc0)
					goto fail;
				c &= 0x3f;
				uchar |= c;

			// 3-byte sequence (most CJKV characters):
			// zzzzyyyyyyxxxxxx = 1110zzzz 10yyyyyy 10xxxxxx
			} else if ((c & 0xf0) == 0xe0) {
				uchar = (c & 0x0f) << 12;

				c = (u8) *s++;
				if ((c & 0xc0) != 0xc0)
					goto fail;
				c &= 0x3f;
				uchar |= c << 6;

				c = (u8) *s++;
				if ((c & 0xc0) != 0xc0)
					goto fail;
				c &= 0x3f;
				uchar |= c;

				/* no bogus surrogates */
				if (0xd800 <= uchar && uchar <= 0xdfff)
					goto fail;

			// 4-byte sequence (surrogate pairs, currently rare):
			// 11101110wwwwzzzzyy + 110111yyyyxxxxxx
			//     = 11110uuu 10uuzzzz 10yyyyyy 10xxxxxx
			// (uuuuu = wwww + 1)
			// FIXME accept the surrogate code points (only)

			} else
				goto fail;
		} else
			uchar = c;
		put_unaligned (cpu_to_le16 (uchar), cp++);
		count++;
		len--;
	}
	return count;
fail:
	return -1;
}

#ifndef min
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif

//#endif	// _BIG_ENDIAN_

//#define GIMR		0xb8003000
//#define GISR		0xb8003004
//#define IRR0         0xB8003008
//#define IRR1         0xB800300c
//#define IRR2         0xB8003010
//#define IRR3         0xB8003014

typedef int irqreturn_t;
#define IRQ_NONE	(0)
#define IRQ_HANDLED	(1)
#define IRQ_RETVAL(x)	((x) != 0)

struct device {
	struct device 	*parent;
	unsigned		is_registered:1;
	struct device_driver *driver;	/* which driver has allocated this
					   device */
	char	bus_id[20];	/* position on parent bus */
#if 0
	struct klist		klist_children;
	struct klist_node	knode_parent;		/* node in sibling list */
	struct klist_node	knode_driver;
	struct klist_node	knode_bus;
	struct kobject kobj;
	struct device_attribute uevent_attr;
	struct device_attribute *devt_attr;

	struct semaphore	sem;	/* semaphore to synchronize calls to
					 * its driver.
					 */

	struct bus_type	* bus;		/* type of bus device is on */
	void		*platform_data;	/* Platform specific data, device
					   core doesn't touch it */
	void		*firmware_data; /* Firmware specific data (e.g. ACPI,
					   BIOS data),reserved for device core*/
	struct dev_pm_info	power;

	u64		*dma_mask;	/* dma mask (if dma'able device) */
	u64		coherent_dma_mask;/* Like dma_mask, but for
					     alloc_coherent mappings as
					     not all hardware supports
					     64 bit addresses for consistent
					     allocations such descriptors. */

	struct list_head	dma_pools;	/* dma pools (if dma'ble) */

	struct dma_coherent_mem	*dma_mem; /* internal for coherent mem
					     override */

	/* class_device migration path */
	struct list_head	node;
	struct class		*class;		/* optional*/
	dev_t			devt;		/* dev_t, creates the sysfs "dev" */
	struct attribute_group	**groups;	/* optional groups */
#endif
	void		*driver_data;	/* data private to the driver */
	void	(*release)(struct device * dev);
};

/*
 * These error numbers are intended to be MIPS ABI compatible
 */
#define	EPERM		 1	/* Operation not permitted */
#define	ENOENT		 2	/* No such file or directory */
#define	ESRCH		 3	/* No such process */
#define	EINTR		 4	/* Interrupted system call */
#define	EIO		 5	/* I/O error */
#define	ENXIO		 6	/* No such device or address */
#define	E2BIG		 7	/* Arg list too long */
#define	ENOEXEC		 8	/* Exec format error */
#define	EBADF		 9	/* Bad file number */
#define	ECHILD		10	/* No child processes */
#define	EAGAIN		11	/* Try again */
//#define	ENOMEM		12	/* Out of memory */
#define	EACCES		13	/* Permission denied */
//#define	EFAULT		14	/* Bad address */
#define	ENOTBLK		15	/* Block device required */
#define	EBUSY		16	/* Device or resource busy */
#define	EEXIST		17	/* File exists */
#define	EXDEV		18	/* Cross-device link */
#define	ENODEV		19	/* No such device */
#define	ENOTDIR		20	/* Not a directory */
#define	EISDIR		21	/* Is a directory */
//#define	EINVAL		22	/* Invalid argument */
#define	ENFILE		23	/* File table overflow */
#define	EMFILE		24	/* Too many open files */
#define	ENOTTY		25	/* Not a typewriter */
#define	ETXTBSY		26	/* Text file busy */
#define	EFBIG		27	/* File too large */
#define	ENOSPC		28	/* No space left on device */
#define	ESPIPE		29	/* Illegal seek */
#define	EROFS		30	/* Read-only file system */
#define	EMLINK		31	/* Too many links */
#define	EPIPE		32	/* Broken pipe */
#define	EDOM		33	/* Math argument out of domain of func */
#define	ERANGE		34	/* Math result not representable */
#define	ENOMSG		35	/* No message of desired type */
#define	EIDRM		36	/* Identifier removed */
#define	ECHRNG		37	/* Channel number out of range */
#define	EL2NSYNC	38	/* Level 2 not synchronized */
#define	EL3HLT		39	/* Level 3 halted */
#define	EL3RST		40	/* Level 3 reset */
#define	ELNRNG		41	/* Link number out of range */
#define	EUNATCH		42	/* Protocol driver not attached */
#define	ENOCSI		43	/* No CSI structure available */
#define	EL2HLT		44	/* Level 2 halted */
#define	EDEADLK		45	/* Resource deadlock would occur */
#define	ENOLCK		46	/* No record locks available */
#define	EBADE		50	/* Invalid exchange */
#define	EBADR		51	/* Invalid request descriptor */
#define	EXFULL		52	/* Exchange full */
#define	ENOANO		53	/* No anode */
#define	EBADRQC		54	/* Invalid request code */
#define	EBADSLT		55	/* Invalid slot */
#define	EDEADLOCK	56	/* File locking deadlock error */
#define	EBFONT		59	/* Bad font file format */
#define	ENOSTR		60	/* Device not a stream */
#define	ENODATA		61	/* No data available */
#define	ETIME		62	/* Timer expired */
#define	ENOSR		63	/* Out of streams resources */
#define	ENONET		64	/* Machine is not on the network */
#define	ENOPKG		65	/* Package not installed */
#define	EREMOTE		66	/* Object is remote */
#define	ENOLINK		67	/* Link has been severed */
#define	EADV		68	/* Advertise error */
#define	ESRMNT		69	/* Srmount error */
#define	ECOMM		70	/* Communication error on send */
#define	EPROTO		71	/* Protocol error */
#define	EDOTDOT		73	/* RFS specific error */
#define	EMULTIHOP	74	/* Multihop attempted */
#define	EBADMSG		77	/* Not a data message */
#define	ENAMETOOLONG	78	/* File name too long */
#define	EOVERFLOW	79	/* Value too large for defined data type */
#define	ENOTUNIQ	80	/* Name not unique on network */
#define	EBADFD		81	/* File descriptor in bad state */
#define	EREMCHG		82	/* Remote address changed */
#define	ELIBACC		83	/* Can not access a needed shared library */
#define	ELIBBAD		84	/* Accessing a corrupted shared library */
#define	ELIBSCN		85	/* .lib section in a.out corrupted */
#define	ELIBMAX		86	/* Attempting to link in too many shared libraries */
#define	ELIBEXEC	87	/* Cannot exec a shared library directly */
#define	EILSEQ		88	/* Illegal byte sequence */
#define	ENOSYS		89	/* Function not implemented */
#define	ELOOP		90	/* Too many symbolic links encountered */
#define	ERESTART	91	/* Interrupted system call should be restarted */
#define	ESTRPIPE	92	/* Streams pipe error */
#define	ENOTEMPTY	93	/* Directory not empty */
#define	EUSERS		94	/* Too many users */
#define	ENOTSOCK	95	/* Socket operation on non-socket */
#define	EDESTADDRREQ	96	/* Destination address required */
#define	EMSGSIZE	97	/* Message too long */
#define	EPROTOTYPE	98	/* Protocol wrong type for socket */
#define	ENOPROTOOPT	99	/* Protocol not available */
#define	EPROTONOSUPPORT	120	/* Protocol not supported */
#define	ESOCKTNOSUPPORT	121	/* Socket type not supported */
//#define	EOPNOTSUPP	122	/* Operation not supported on transport endpoint */
#define	EPFNOSUPPORT	123	/* Protocol family not supported */
#define	EAFNOSUPPORT	124	/* Address family not supported by protocol */
#define	EADDRINUSE	125	/* Address already in use */
#define	EADDRNOTAVAIL	126	/* Cannot assign requested address */
#define	ENETDOWN	127	/* Network is down */
#define	ENETUNREACH	128	/* Network is unreachable */
#define	ENETRESET	129	/* Network dropped connection because of reset */
#define	ECONNABORTED	130	/* Software caused connection abort */
#define	ECONNRESET	131	/* Connection reset by peer */
#define	ENOBUFS		132	/* No buffer space available */
#define	EISCONN		133	/* Transport endpoint is already connected */
#define	ENOTCONN	134	/* Transport endpoint is not connected */
#define	EUCLEAN		135	/* Structure needs cleaning */
#define	ENOTNAM		137	/* Not a XENIX named type file */
#define	ENAVAIL		138	/* No XENIX semaphores available */
#define	EISNAM		139	/* Is a named type file */
#define	EREMOTEIO	140	/* Remote I/O error */
#define EINIT		141	/* Reserved */
#define EREMDEV		142	/* Error 142 */
#define	ESHUTDOWN	143	/* Cannot send after transport endpoint shutdown */
#define	ETOOMANYREFS	144	/* Too many references: cannot splice */
#define	ETIMEDOUT	145	/* Connection timed out */
#define	ECONNREFUSED	146	/* Connection refused */
#define	EHOSTDOWN	147	/* Host is down */
#define	EHOSTUNREACH	148	/* No route to host */
#define	EWOULDBLOCK	EAGAIN	/* Operation would block */
#define	EALREADY	149	/* Operation already in progress */
#define	EINPROGRESS	150	/* Operation now in progress */
#define	ESTALE		151	/* Stale NFS file handle */
#define ECANCELED	158	/* AIO operation canceled */

#ifndef __offsetof
#define __offsetof(type, field) ((size_t)(&((type *)0)->field))
#endif

#ifndef offsetof
#define offsetof(type, field) __offsetof(type, field)
#endif

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
        const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
        (type *)( (char *)__mptr - offsetof(type,member) );})

/*-------------------------------------------------------------------------*/

/*
 * STANDARD DESCRIPTORS ... as returned by GET_DESCRIPTOR, or
 * (rarely) accepted by SET_DESCRIPTOR.
 *
 * Note that all multi-byte values here are encoded in little endian
 * byte order "on the wire".  But when exposed through Linux-USB APIs,
 * they've been converted to cpu byte order.
 */

/*
 * Descriptor types ... USB 2.0 spec table 9.5
 */
#define USB_DT_DEVICE			0x01
#define USB_DT_CONFIG			0x02
#define USB_DT_STRING			0x03
#define USB_DT_INTERFACE		0x04
#define USB_DT_ENDPOINT			0x05
#define USB_DT_DEVICE_QUALIFIER		0x06
#define USB_DT_OTHER_SPEED_CONFIG	0x07
#define USB_DT_INTERFACE_POWER		0x08
/* these are from a minor usb 2.0 revision (ECN) */
#define USB_DT_OTG			0x09
#define USB_DT_DEBUG			0x0a
#define USB_DT_INTERFACE_ASSOCIATION	0x0b
/* these are from the Wireless USB spec */
#define USB_DT_SECURITY			0x0c
#define USB_DT_KEY			0x0d
#define USB_DT_ENCRYPTION_TYPE		0x0e
#define USB_DT_BOS			0x0f
#define USB_DT_DEVICE_CAPABILITY	0x10
#define USB_DT_WIRELESS_ENDPOINT_COMP	0x11
#define USB_DT_WIRE_ADAPTER		0x21
#define USB_DT_RPIPE			0x22

/* conventional codes for class-specific descriptors */
#define USB_DT_CS_DEVICE		0x21
#define USB_DT_CS_CONFIG		0x22
#define USB_DT_CS_STRING		0x23
#define USB_DT_CS_INTERFACE		0x24
#define USB_DT_CS_ENDPOINT		0x25

/*
 * Device and/or Interface Class codes
 * as found in bDeviceClass or bInterfaceClass
 * and defined by www.usb.org documents
 */
#define USB_CLASS_PER_INTERFACE		0	/* for DeviceClass */
#define USB_CLASS_AUDIO			1
#define USB_CLASS_COMM			2
#define USB_CLASS_HID			3
#define USB_CLASS_PHYSICAL		5
#define USB_CLASS_STILL_IMAGE		6
#define USB_CLASS_PRINTER		7
#define USB_CLASS_MASS_STORAGE		8
#define USB_CLASS_HUB			9
#define USB_CLASS_CDC_DATA		0x0a
#define USB_CLASS_CSCID			0x0b	/* chip+ smart card */
#define USB_CLASS_CONTENT_SEC		0x0d	/* content security */
#define USB_CLASS_VIDEO			0x0e
#define USB_CLASS_WIRELESS_CONTROLLER	0xe0
#define USB_CLASS_APP_SPEC		0xfe
#define USB_CLASS_VENDOR_SPEC		0xff


/*-------------------------------------------------------------------------*/


#define USB_DT_ENDPOINT_SIZE		7
#define USB_DT_ENDPOINT_AUDIO_SIZE	9	/* Audio extension */


/*
 * Endpoints
 */
#define USB_ENDPOINT_NUMBER_MASK	0x0f	/* in bEndpointAddress */
#define USB_ENDPOINT_DIR_MASK		0x80

#define USB_ENDPOINT_XFERTYPE_MASK	0x03	/* in bmAttributes */
#define USB_ENDPOINT_XFER_CONTROL	0
#define USB_ENDPOINT_XFER_ISOC		1
#define USB_ENDPOINT_XFER_BULK		2
#define USB_ENDPOINT_XFER_INT		3
#define USB_ENDPOINT_MAX_ADJUSTABLE	0x80


/*-------------------------------------------------------------------------*/


#define ATOMIC_INIT(i)    { (i) }
#define atomic_set(v,i)	((v)->counter = (i))

/*-------------------------------------------------------------------------*/

/* CONTROL REQUEST SUPPORT */

/*
 * USB directions
 *
 * This bit flag is used in endpoint descriptors' bEndpointAddress field.
 * It's also one of three fields in control requests bRequestType.
 */
#define USB_DIR_OUT			0		/* to device */
#define USB_DIR_IN			0x80		/* to host */

/*
 * USB types, the second of three bRequestType fields
 */
#define USB_TYPE_MASK			(0x03 << 5)
#define USB_TYPE_STANDARD		(0x00 << 5)
#define USB_TYPE_CLASS			(0x01 << 5)
#define USB_TYPE_VENDOR			(0x02 << 5)
#define USB_TYPE_RESERVED		(0x03 << 5)

/*
 * USB recipients, the third of three bRequestType fields
 */
#define USB_RECIP_MASK			0x1f
#define USB_RECIP_DEVICE		0x00
#define USB_RECIP_INTERFACE		0x01
#define USB_RECIP_ENDPOINT		0x02
#define USB_RECIP_OTHER			0x03
/* From Wireless USB 1.0 */
#define USB_RECIP_PORT 			0x04
#define USB_RECIP_RPIPE 		0x05

/*
 * Standard requests, for the bRequest field of a SETUP packet.
 *
 * These are qualified by the bRequestType field, so that for example
 * TYPE_CLASS or TYPE_VENDOR specific feature flags could be retrieved
 * by a GET_STATUS request.
 */
#define USB_REQ_GET_STATUS		0x00
#define USB_REQ_CLEAR_FEATURE		0x01
#define USB_REQ_SET_FEATURE		0x03
#define USB_REQ_SET_ADDRESS		0x05
#define USB_REQ_GET_DESCRIPTOR		0x06
#define USB_REQ_SET_DESCRIPTOR		0x07
#define USB_REQ_GET_CONFIGURATION	0x08
#define USB_REQ_SET_CONFIGURATION	0x09
#define USB_REQ_GET_INTERFACE		0x0A
#define USB_REQ_SET_INTERFACE		0x0B
#define USB_REQ_SYNCH_FRAME		0x0C

#define USB_REQ_SET_ENCRYPTION		0x0D	/* Wireless USB */
#define USB_REQ_GET_ENCRYPTION		0x0E
#define USB_REQ_RPIPE_ABORT		0x0E
#define USB_REQ_SET_HANDSHAKE		0x0F
#define USB_REQ_RPIPE_RESET		0x0F
#define USB_REQ_GET_HANDSHAKE		0x10
#define USB_REQ_SET_CONNECTION		0x11
#define USB_REQ_SET_SECURITY_DATA	0x12
#define USB_REQ_GET_SECURITY_DATA	0x13
#define USB_REQ_SET_WUSB_DATA		0x14
#define USB_REQ_LOOPBACK_DATA_WRITE	0x15
#define USB_REQ_LOOPBACK_DATA_READ	0x16
#define USB_REQ_SET_INTERFACE_DS	0x17

/*
 * USB feature flags are written using USB_REQ_{CLEAR,SET}_FEATURE, and
 * are read as a bit array returned by USB_REQ_GET_STATUS.  (So there
 * are at most sixteen features of each type.)
 */
#define USB_DEVICE_SELF_POWERED		0	/* (read only) */
#define USB_DEVICE_REMOTE_WAKEUP	1	/* dev may initiate wakeup */
#define USB_DEVICE_TEST_MODE		2	/* (wired high speed only) */
#define USB_DEVICE_BATTERY		2	/* (wireless) */
#define USB_DEVICE_B_HNP_ENABLE		3	/* (otg) dev may initiate HNP */
#define USB_DEVICE_WUSB_DEVICE		3	/* (wireless)*/
#define USB_DEVICE_A_HNP_SUPPORT	4	/* (otg) RH port supports HNP */
#define USB_DEVICE_A_ALT_HNP_SUPPORT	5	/* (otg) other RH port does */
#define USB_DEVICE_DEBUG_MODE		6	/* (special devices only) */

#define USB_ENDPOINT_HALT		0	/* IN/OUT will STALL */


/*-------------------------------------------------------------------------*/
// #include <linux/prefetch.h>
static inline void prefetch(const void *x) {;}

/**
 * list_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
 * list_for_each_entry	-	iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define list_for_each_entry(pos, head, member)				\
	for (pos = list_entry((head)->next, typeof(*pos), member);	\
	     prefetch(pos->member.next), &pos->member != (head); 	\
	     pos = list_entry(pos->member.next, typeof(*pos), member))

#endif
