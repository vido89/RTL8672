#include <linux/types.h>
#include <asm/uaccess.h>
#include "rtk_voip.h"
#include "type.h"
#include "voip_params.h"
#include "voip_control.h"
#include "lcm.h"

#define LCM_WIDTH		DISPLAY_CHARACTER	/* 16 */
#define LCM_HEIGHT		2

static void lcm_draw_text( int x, int y, unsigned char *pszText, int len );
static void lcm_move_cursor_position( int x, int y );

void do_lcm_opt_ctl( void *pUser, unsigned int len )
{
	TstLcmCtl lcmCtl;
	
	if( len > sizeof( TstLcmCtl ) ) {
		len = sizeof( TstLcmCtl );
		printk( "LCM: Truncate user specified length\n" );
	}
	
	copy_from_user( &lcmCtl, pUser, len );

	switch( lcmCtl.General.cmd ) {
	case LCM_CMD_DISPLAY_ON_OFF:
		Display_on_off( lcmCtl.DisplayOnOff.bDisplayOnOff, 
						lcmCtl.DisplayOnOff.bCursorOnOff,
						lcmCtl.DisplayOnOff.bCursorBlink, 0 );
		break;
	
	case LCM_CMD_MOVE_CURSOR_POS:
		lcm_move_cursor_position( lcmCtl.MoveCursorPosition.x, 
								  lcmCtl.MoveCursorPosition.y );
		break;
		
	case LCM_CMD_DRAW_TEXT:
		lcm_draw_text( lcmCtl.DrawText.x, lcmCtl.DrawText.y,
					   lcmCtl.DrawText.szText, lcmCtl.DrawText.len );
		break;
		
	default:
		printk( "LCD not support\n" );
		break;
	}
}

static void lcm_draw_text( int x, int y, unsigned char *pszText, int len )
{
	int abs_x;
	
	if( y < 0 || y >= LCM_HEIGHT || x >= LCM_WIDTH || len <= 0 )
		return;

	if( x < 0 ) {
		
		abs_x = x * ( -1 );
		
		if( abs_x > len )	/* too left */
			return;

		x = 0;
		pszText += abs_x;
		len -= abs_x;
	}
	
	if( x + len > LCM_WIDTH )	/* too long */
		len = LCM_WIDTH - x;
		
	/*
	 * Now, it normalize to 
	 *  0 <= x < LCM_WIDTH
	 *  0 <= y < LCM_HEIGHT
	 *  1 <= x + len < LCM_WIDTH
	 */
	if( y == 0 )
		Set_DD_RAM_address( 0x00 + x, 0 );
	else
		Set_DD_RAM_address( 0x40 + x, 0 );
	
	while( len -- )
		Write_Data_to_RAM( *pszText ++, 0 );
}

static void lcm_move_cursor_position( int x, int y )
{
	if( y < 0 || y >= LCM_HEIGHT || x < 0 || x >= LCM_WIDTH )
		return;
	
	/* set AC address */
	if( y == 0 )
		Set_DD_RAM_address( 0x00 + x, 0 );
	else
		Set_DD_RAM_address( 0x40 + x, 0 );
}

