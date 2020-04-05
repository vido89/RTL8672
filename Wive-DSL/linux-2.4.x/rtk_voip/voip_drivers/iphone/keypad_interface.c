#include <linux/types.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include "rtk_voip.h"
#include "type.h"
#include "voip_params.h"
#include "voip_control.h"
#include "keypad_map.h"

#define ENABLE_KEYPAD_DEBOUNCING

#define TIMER_INTERVAL_PERIOD		10	/* 10ms */
#define NOARML_PRESS_KEY_PERIOD		30	/* 30ms */
#define LONG_PRESS_KEY_PERIOD		500	/* 500ms */

static void debug_keypad_signal_target( wkey_t wkey );
extern unsigned char iphone_hook_detect();

//#define NUM_OF_KEYPAD_QUEUE		( 5 + 1 )	/* queue size: 5 */

static pid_t sig_handler_pid = 0;
static wkey_t key_buffering;
static unsigned char key_w = 0, key_r = 0;
static unsigned char key_hook_status = 2;	/* 0: on-hook, 1: off-hook */

#if 0	// test only
static unsigned char hook_status = 0;	// TODO: it should read from hardware
#endif

static int GetHookStatus( unsigned char *pStatus )
{
	unsigned char status;

	status = ( iphone_hook_detect() ? 1 : 0 );
	
	if( pStatus )
		*pStatus = status;
	
	if( status == key_hook_status )
		return 0;	/* unchange */
	
	key_hook_status = status;
	
	return 1;	/* changed!! */
}

void do_keypad_opt_ctl( void *pUser, unsigned int len )
{
	TstKeypadCtl keypadCtl;
	
	if( len > sizeof( TstKeypadCtl ) ) {
		len = sizeof( TstKeypadCtl );
		printk( "Keypad: Truncate user specified length\n" );
	}
	
	copy_from_user( &keypadCtl, pUser, len );
	
	switch( keypadCtl.General.cmd ) {
	case KEYPAD_CMD_SET_TARGET:
		sig_handler_pid = keypadCtl.SetTarget.pid;
		break;
		
	case KEYPAD_CMD_SIG_DEBUG:
		debug_keypad_signal_target( keypadCtl.SignalDebug.wkey );
		break;
	
	case KEYPAD_CMD_READ_KEY:
		if( key_w != key_r ) {
			keypadCtl.ReadKey.wkey = key_buffering;
			keypadCtl.ReadKey.validKey = 1;
			
			key_r = ( key_r + 1 ) & 0x01;
		} else
			keypadCtl.ReadKey.validKey = 0;
			
		copy_to_user( pUser, &keypadCtl.ReadKey, sizeof( keypadCtl.ReadKey ) );
		break;
		
	case KEYPAD_CMD_HOOK_STATUS:
		//keypadCtl.HookStatus.status = hook_status;
		GetHookStatus( &keypadCtl.HookStatus.status );
		copy_to_user( pUser, &keypadCtl.HookStatus, sizeof( keypadCtl.HookStatus ) );
		break;
	
	default:
		printk( "Keypad: this cmd not support\n" );
	}
}

static void do_keypad_signal_target( wkey_t wkey )
{
#if 0	// test only
	if( wkey == KEY_HOOK )
		hook_status = ( hook_status ? 0 : 1 );	// xor 
#endif

	if( key_w == key_r ) {
	
		key_buffering = wkey;
		
		key_w = ( key_w + 1 ) & 0x01;
	}

	/* signal to user space */
	if( sig_handler_pid ) {
		/*
		 * Value of third parameter can be 0 or 1:
		 *  0: SI_USER		(sent by kill, sigsend, raise)
		 *  1: SI_KERNEL	(sent by the kernel from somewhere)
		 */
		kill_proc( sig_handler_pid, SIGUSR1, 0 );
	} else
		printk( "Keypad: no target pid?\n" );
}

static void debug_keypad_signal_target( wkey_t wkey )
{
	do_keypad_signal_target( wkey );
}

#ifndef ENABLE_KEYPAD_DEBOUNCING
void keypad_signal_target( const keypad_dev_t *keypad_data_pool )
{
	if( keypad_data_pool ->flags == 0 )
		return;
	
	do_keypad_signal_target( keypad_data_pool ->data_string );
	
	keypad_data_pool ->flags = 0;
}
#endif /* !ENABLE_KEYPAD_DEBOUNCING */

#ifdef ENABLE_KEYPAD_DEBOUNCING

#define NORMAL_PRESS_KEY_COUNT		( NOARML_PRESS_KEY_PERIOD / TIMER_INTERVAL_PERIOD )
#define LONG_PRESS_KEY_COUNT		( LONG_PRESS_KEY_PERIOD / TIMER_INTERVAL_PERIOD )

enum {
	DB_STATE_EMPTY,
	DB_STATE_PRESS,
	DB_STATE_CHECK_PRESS,
};

static unsigned char db_key = 0;
static unsigned int db_count = 0;
static unsigned int db_state = DB_STATE_EMPTY;

void keypad_polling_signal_target( const keypad_dev_t *keypad_data_pool )
{
	/* During scan whole keys, it calls this function if a key is pressed. */
	if( keypad_data_pool ->flags == 0 )
		return;
		
	switch( db_state ) {
	case DB_STATE_EMPTY:	/* a fresh key */
	case DB_STATE_PRESS:	/* press more than one key? */
label_press_a_new_key:
		db_key = keypad_data_pool ->data_string;
		db_count = 1;
		db_state = DB_STATE_PRESS;
		break;
	
	case DB_STATE_CHECK_PRESS:	/* still press this key? */
		if( db_key != keypad_data_pool ->data_string )
			goto label_press_a_new_key;
		
		/* ok. still press */	
		db_count ++;
		db_state = DB_STATE_PRESS;
		break;
	}
	
	keypad_data_pool ->flags = 0;
}

void keypad_polling_scan_done( void )
{
	/* Call this function, once it scans whole keys. */
	/* i.e. It is executed in period of TIMER_INTERVAL_PERIOD. */
	switch( db_state ) {
	case DB_STATE_PRESS:
		if( db_count == NORMAL_PRESS_KEY_COUNT )	/* normal press key */
			do_keypad_signal_target( db_key );
		else if( db_count == LONG_PRESS_KEY_COUNT )	/* long press key */
			do_keypad_signal_target( 0x8000 | ( wkey_t )db_key );
			
		db_state = DB_STATE_CHECK_PRESS;
		break;
		
	case DB_STATE_CHECK_PRESS:
		db_key = 0;
		db_count = 0;
		db_state = DB_STATE_EMPTY;
		break;
	}
	
	/* hook status */
	if( GetHookStatus( NULL ) ) {
		do_keypad_signal_target( KEY_HOOK );
	}
}
#endif /* ENABLE_KEYPAD_DEBOUNCING */

