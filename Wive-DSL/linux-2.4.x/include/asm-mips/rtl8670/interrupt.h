//#define CPU_CLOCK_RATE 14318180		/* CPU clock @ 14 MHz */
//#define CPU_CLOCK_RATE 20000000		/* CPU clock @ 20 MHz */
#if defined(CONFIG_RTL8670)
#define CPU_CLOCK_RATE 150000000		/* CPU clock @ 150 MHz */
#elif defined(CONFIG_RTL8672)
#define CPU_CLOCK_RATE 175000000
#else // 8671
#define CPU_CLOCK_RATE 35328000		//tylo/* CPU clock @ 150 MHz */
#endif
//#define CPU_CLOCK_RATE 25000000		/* CPU clock @ 20 MHz */


