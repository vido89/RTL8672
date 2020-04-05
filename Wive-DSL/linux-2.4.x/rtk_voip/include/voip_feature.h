#ifndef __VOIP_FEATURE_H
#define __VOIP_FEATURE_H

//*****************************************
//* VoIP Feature
//*****************************************

/* SLIC NUM ( bit 30-31 )*/
#define SLIC_NUM_1			0x00000000
#define SLIC_NUM_2			0x40000000
#define SLIC_NUM_3			0x80000000
#define SLIC_NUM_4			0xC0000000

#define SLIC_NUM_MASK			0xC0000000

/* DAA NUM ( bit 28-29 )*/
#define DAA_NUM_1			0x00000000
#define DAA_NUM_2			0x10000000
#define DAA_NUM_3			0x20000000
#define DAA_NUM_4			0x30000000

#define DAA_NUM_MASK			0x30000000

/* DAA Type ( bit 26-27 )*/
#define NO_DAA				0x00000000	//No DAA
#define REAL_DAA			0x04000000	//No Negotiation
#define REAL_DAA_NEGO			0x08000000	//Negotiation
#define VIRTUAL_DAA			0x0C000000	//Virtual DAA

#define DAA_TYPE_MASK			0x0C000000

/* Platform ( bit 24-25 )*/
#define PLATFORM_8186			0x00000000	//8186 SoC
#define PLATFORM_865x			0x01000000	//865x Soc
#define PLATFORM_867x			0x02000000	//867x Soc
#define PLATFORM_RESERVE2		0x03000000	//reserve

#define PLATFORM_MASK			0x03000000

/* Platform Type (8186) ( bit 22-23 )*/
#define PLATFORM_TYPE_8186V		0x00000000	//8186V 
#define PLATFORM_TYPE_8186PV		0x00400000	//8186PV
#define PLATFORM_TYPE3			0x00800000	//reserve
#define PLATFORM_TYPE4			0x00C00000	//reserve

/* Platform Type (865x) ( bit 22-23 )*/
#define PLATFORM_TYPE_8651		0x00000000	//8651
#define PLATFORM_TYPE_865xC		0x00400000	//865xC
#define PLATFORM_TYPE3			0x00800000	//reserve
#define PLATFORM_TYPE4			0x00C00000	//reserve

/* Platform Type (867x) ( bit 22-23 )*/
#define PLATFORM_TYPE_8671		0x00000000	//8671
#define PLATFORM_TYPE2			0x00400000	//reserve
#define PLATFORM_TYPE3			0x00800000	//reserve
#define PLATFORM_TYPE4			0x00C00000	//reserve

#define PLATFORM_TYPE_MASK		0x00C00000

/* IVR ( bit 21 )*/
#define IVR_SUPPORT			0x00200000
#define IVR_NOT_SUPPORT			0xFFDFFFFF

/* One ARM Router ( bit 20 )*/
#define ONE_ARM_ROUTER_SUPPORT		0x00100000
#define ONE_ARM_ROUTER_NOT_SUPPORT	0xFFEFFFFF

/* Codec ( bit 14-19 )*/
#define CODEC_G729_SUPPORT		0x00080000
#define CODEC_G729_NOT_SUPPORT		0xFFF7FFFF

#define CODEC_G723_SUPPORT		0x00040000
#define CODEC_G723_NOT_SUPPORT		0xFFFBFFFF

#define CODEC_G726_SUPPORT		0x00020000
#define CODEC_G726_NOT_SUPPORT		0xFFFDFFFF

#define CODEC_GSMFR_SUPPORT		0x00010000
#define CODEC_GSMFR_NOT_SUPPORT		0xFFFEFFFF

#define CODEC_AMR_SUPPORT		0x00008000
#define CODEC_AMR_NOT_SUPPORT		0xFFFF7FFF

#define CODEC_iLBC_SUPPORT		0x00004000
#define CODEC_iLBC_NOT_SUPPORT		0xFFFFBFFF

#define CODEC_T38_SUPPORT		0x00002000
#define CODEC_T38_NOT_SUPPORT		0xFFFFDFFF

/* VoIP MW ( bit 8 )*/
#define VOIP_MW_REALTEK			0x00000100
#define VOIP_MW_AUDIOCODES		0x00000000

#define VOIP_MW_MASK			0x00000100

/* Reserve ( bit 0-7, 9-12 )*/

/* SLIC NUM */
#if defined (CONFIG_RTK_VOIP_SLIC_NUM_1)
#define	RTK_VOIP_SLIC_FEATURE		SLIC_NUM_1
#elif defined (CONFIG_RTK_VOIP_SLIC_NUM_2)
#define	RTK_VOIP_SLIC_FEATURE		SLIC_NUM_2
#elif defined (CONFIG_RTK_VOIP_SLIC_NUM_3)
#define	RTK_VOIP_SLIC_FEATURE		SLIC_NUM_3
#elif defined (CONFIG_RTK_VOIP_SLIC_NUM_4)
#define	RTK_VOIP_SLIC_FEATURE		SLIC_NUM_4
#elif defined (CONFIG_RTK_VOIP_DRIVERS_IP_PHONE)
#define	RTK_VOIP_SLIC_FEATURE		SLIC_NUM_1
#endif

/* DAA NUM and Type*/
#if defined (CONFIG_RTK_VOIP_DRIVERS_DAA_SI3050) && !defined (CONFIG_RTK_SLIC_DAA_NEGOTIATION) && !defined (CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA)
#define RTK_VOIP_DAA_FEATURE		(DAA_NUM_1 | REAL_DAA)
#elif defined (CONFIG_RTK_VOIP_DRIVERS_DAA_SI3050) && (CONFIG_RTK_SLIC_DAA_NEGOTIATION)
#define RTK_VOIP_DAA_FEATURE		(DAA_NUM_1 | REAL_DAA_NEGO)
#elif defined (CONFIG_RTK_VOIP_DRIVERS_DAA_SI3050) && (CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA)
#define RTK_VOIP_DAA_FEATURE		(DAA_NUM_1 | VIRTUAL_DAA)
#else
#define RTK_VOIP_DAA_FEATURE		NO_DAA
#endif

#define RTK_VOIP_DAA_MASK	(DAA_NUM_MASK | DAA_TYPE_MASK)
				
/* Platform anf Platform Type */
#if defined (CONFIG_RTK_VOIP_DRIVERS_PCM8186) && (CONFIG_RTK_VOIP_DRIVERS_PCM8186V)
#define	RTK_VOIP_PLATFORM_FEATURE	(PLATFORM_8186 | PLATFORM_TYPE_8186V)
#elif defined (CONFIG_RTK_VOIP_DRIVERS_PCM8186) && (CONFIG_RTK_VOIP_DRIVERS_PCM8186PV)
#define	RTK_VOIP_PLATFORM_FEATURE	(PLATFORM_8186 | PLATFORM_TYPE_8186PV)
#elif defined (CONFIG_RTK_VOIP_DRIVERS_PCM8651)
#define	RTK_VOIP_PLATFORM_FEATURE	(PLATFORM_865x | PLATFORM_TYPE_8651)
#elif defined (CONFIG_RTK_VOIP_DRIVERS_PCM8671)
#define	RTK_VOIP_PLATFORM_FEATURE	(PLATFORM_867x | PLATFORM_TYPE_8671)
#elif defined (CONFIG_RTK_VOIP_DRIVERS_PCM865xC)
#define	RTK_VOIP_PLATFORM_FEATURE	(PLATFORM_865x | PLATFORM_TYPE_865xC)
#endif

#define RTK_VOIP_PLATFORM_MASK		(PLATFORM_MASK | PLATFORM_TYPE_MASK)

/* IVR */
#if defined (CONFIG_RTK_VOIP_IVR)
#define	RTK_VOIP_IVR_FEATURE		IVR_SUPPORT
#else
#define	RTK_VOIP_IVR_FEATURE		0x0
#endif

/* One ARM Router */
#if defined (CONFIG_RTK_VOIP_DRIVERS_8186V_ROUTER)
#define RTK_VOIP_ONE_ARM_ROUTER_FEATURE ONE_ARM_ROUTER_SUPPORT
#else
#define RTK_VOIP_ONE_ARM_ROUTER_FEATURE 0x0
#endif

/* G729 */
#if defined (CONFIG_RTK_VOIP_G729AB)
#define  RTK_VOIP_G729_FEATURE		CODEC_G729_SUPPORT
#else
#define  RTK_VOIP_G729_FEATURE		0x0
#endif

/* G723 */
#if defined (CONFIG_RTK_VOIP_G7231)
#define  RTK_VOIP_G723_FEATURE		CODEC_G723_SUPPORT
#else
#define  RTK_VOIP_G723_FEATURE		0x0
#endif

/* G726 */
#if defined (CONFIG_RTK_VOIP_G726)
#define  RTK_VOIP_G726_FEATURE		CODEC_G726_SUPPORT
#else
#define  RTK_VOIP_G726_FEATURE		0x0
#endif

/* GSM-FR */
#if defined (CONFIG_RTK_VOIP_GSMFR)
#define  RTK_VOIP_GSMFR_FEATURE		CODEC_GSMFR_SUPPORT
#else
#define  RTK_VOIP_GSMFR_FEATURE		0x0
#endif

/* AMR */
#if defined (CONFIG_RTK_VOIP_AMR)
#define  RTK_VOIP_AMR_FEATURE		CODEC_AMR_SUPPORT
#else
#define  RTK_VOIP_AMR_FEATURE		0x0
#endif

/* iLBC */
#if defined (CONFIG_RTK_VOIP_iLBC)
#define  RTK_VOIP_iLBC_FEATURE		CODEC_iLBC_SUPPORT
#else
#define  RTK_VOIP_iLBC_FEATURE		0x0
#endif

/* T.38 */
#if defined (CONFIG_RTK_VOIP_T38)
#define  RTK_VOIP_T38_FEATURE		CODEC_T38_SUPPORT
#else
#define  RTK_VOIP_T38_FEATURE		0x0
#endif

/* VoIP Middleware*/
#ifndef CONFIG_AUDIOCODES_VOIP
#define	RTK_VOIP_MW_FEATURE			VOIP_MW_REALTEK
#else
#define	RTK_VOIP_MW_FEATURE			VOIP_MW_AUDIOCODES
#endif

#define RTK_VOIP_CODEC_FEATURE	( \
								RTK_VOIP_G729_FEATURE	| \
								RTK_VOIP_G723_FEATURE	| \
								RTK_VOIP_G726_FEATURE	| \
								RTK_VOIP_GSMFR_FEATURE	| \
								RTK_VOIP_AMR_FEATURE	| \
								RTK_VOIP_iLBC_FEATURE 	| \
								RTK_VOIP_T38_FEATURE	\
								)

#define	RTK_VOIP_CODEC_MASK		( \
								CODEC_G729_SUPPORT	| \
								CODEC_G723_SUPPORT	| \
								CODEC_G726_SUPPORT	| \
								CODEC_GSMFR_SUPPORT	| \
								CODEC_AMR_SUPPORT	| \
								CODEC_iLBC_SUPPORT	| \
								CODEC_T38_SUPPORT	\
								)

/* 32 bit */
#define RTK_VOIP_FEATURE 	( \
							RTK_VOIP_SLIC_FEATURE			| \
							RTK_VOIP_DAA_FEATURE			| \
							RTK_VOIP_PLATFORM_FEATURE 		| \
							RTK_VOIP_IVR_FEATURE			| \
							RTK_VOIP_ONE_ARM_ROUTER_FEATURE	| \
							RTK_VOIP_CODEC_FEATURE 			| \
							RTK_VOIP_MW_FEATURE	\
							)

//------- feature macro -----------------------
#define RTK_VOIP_SLIC_NUM(f)		((((f) & SLIC_NUM_MASK) >> 30) + 1)
#define	RTK_VOIP_PLATFORM_CHECK(f)	(((f) & RTK_VOIP_PLATFORM_MASK) == RTK_VOIP_PLATFORM_FEATURE)
#define	RTK_VOIP_ROUTER_CHECK(f)	(((f) & ONE_ARM_ROUTER_SUPPORT) == RTK_VOIP_ONE_ARM_ROUTER_FEATURE)
#define	RTK_VOIP_SLIC_CHECK(f)		(((f) & SLIC_NUM_MASK) == RTK_VOIP_SLIC_FEATURE)
#define	RTK_VOIP_DAA_CHECK(f)		(((f) & RTK_VOIP_DAA_MASK) == RTK_VOIP_DAA_FEATURE)
#define	RTK_VOIP_CODEC_CHECK(f)		(((f) & RTK_VOIP_CODEC_MASK) == RTK_VOIP_CODEC_FEATURE)
#define	RTK_VOIP_MW_CHECK(f)		(((f) & VOIP_MW_REALTEK) == RTK_VOIP_MW_FEATURE)

#define SLIC_NUM				((RTK_VOIP_SLIC_FEATURE >> 30) + 1)
#define MAX_SLIC_NUM			((SLIC_NUM_4 >> 30) + 1)

#endif
