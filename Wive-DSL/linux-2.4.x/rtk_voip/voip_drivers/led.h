#if 0		/* 0:V210 EV Board ,1:E_version*/
#define _V210_Ed_ 
#else
#define _V210_EV_BOARD_
#endif
/* =================== For V210 EV Board LED Control ======================== */
#define GPAB_DIR  *((volatile unsigned int *)0xbd010124)
#define GPAB_DATA  *((volatile unsigned int *)0xbd010120)
#define led_blinking_frequency 	5

/* led.c function prototype */
static struct timer_list fxs_led_timer;
void FXS_LED_STATE(unsigned int state);
static struct timer_list fxo_led_timer;
void FXO_LED_STATE(unsigned int state);
static struct timer_list sip_led_timer;
void SIP_LED_STATE(unsigned int state);
static struct timer_list fxs_one_led_timer;
void FXS_ONE_LED_STATE(unsigned int state);
void LED_Init(void);
/* ========================================================================== */