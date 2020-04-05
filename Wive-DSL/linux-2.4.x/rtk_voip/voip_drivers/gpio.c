/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : GPIO Driver
* Abstract : 
* Author : 
*/

#include "gpio.h"

/*==================== FOR RTL8186 ==================*/

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186

/**************************** GPIO Driver **************************************/

enum GPIO_FUNC	
{
	GPIO_FUNC_DIRECTION,
	GPIO_FUNC_DATA,
	GPIO_FUNC_INTERRUPT_STATUS,
	GPIO_FUNC_INTERRUPT_ENABLE,
	GPIO_FUNC_MAX,
};


//******************************************* Direction

static uint32 regGpioDirectionRead[] =
{
	GPABDIR, 	/* Port A */
	GPABDIR, 	/* Port B */
	GPCDDIR, 	/* Port C */
	GPCDDIR, 	/* Port D */
	GPEFDIR,  	/* Port E */
	GPEFDIR,	/* Port F */
	GPGDIR,		/* Port G */
};

static uint32 bitStartGpioDirectionRead[] =
{
	0, 			/* Port A */
	16, 			/* Port B */
	0,  			/* Port C */
	16, 			/* Port D */
	0, 			/* Port E */
	0, 			/* Port F */	// according to modified Spec. 
	0, 			/* Port G */
};

static uint32 regGpioDirectionWrite[] =
{
	GPABDIR, 	/* Port A */
	GPABDIR, 	/* Port B */
	GPCDDIR, 	/* Port C */
	GPCDDIR, 	/* Port D */
	GPEFDIR,  	/* Port E */
	GPEFDIR,	/* Port F */
	GPGDIR,		/* Port G */
};

static uint32 bitStartGpioDirectionWrite[] =
{
	0, 			/* Port A */
	0, 			/* Port B */
	0,  			/* Port C */
	0, 			/* Port D */  
	0, 			/* Port E */
	0, 			/* Port F */
	0, 			/* Port G */
};

//******************************************* Data

static uint32 regGpioDataRead[] =
{
	GPABDATA, 	/* Port A */
	GPABDATA, 	/* Port B */
	GPCDDATA, 	/* Port C */
	GPCDDATA,  	/* Port D */
	GPEFDATA,  	/* Port E */
	GPEFDATA,	/* Port F */
	GPGDATA,	/* Port G */
};

static uint32 bitStartGpioDataRead[] =
{
	0, 			/* Port A */
	16, 			/* Port B */
	0,  			/* Port C */
	16, 			/* Port D */
	0, 			/* Port E */
	16, 			/* Port F */
	0, 			/* Port G */
};

static uint32 regGpioDataWrite[] =
{
	GPABDATA, 	/* Port A */
	GPABDATA, 	/* Port B */
	GPCDDATA, 	/* Port C */
	GPCDDATA,  	/* Port D */
	GPEFDATA,  	/* Port E */
	GPEFDATA,	/* Port F */
	GPGDATA,	/* Port G */
};

static uint32 bitStartGpioDataWrite[] =
{
	0, 			/* Port A */
	0, 			/* Port B */
	0,  			/* Port C */
	0, 			/* Port D */
	0, 			/* Port E */
	0, 			/* Port F */
	0, 			/* Port G */
};

//******************************************* ISR

static uint32 regGpioInterruptStatusRead[] =
{
	GPABISR, 	/* Port A */
	GPABISR, 	/* Port B */
	GPCDISR, 	/* Port C */
	GPCDISR,  	/* Port D */
	GPEFISR,  	/* Port E */
	GPEFISR,	/* Port F */
	GPGISR,		/* Port G */
};

static uint32 bitStartGpioInterruptStatusRead[] =
{
	0, 			/* Port A */
	16, 			/* Port B */
	0,  			/* Port C */
	16, 			/* Port D */
	0, 			/* Port E */
	16, 			/* Port F */
	0, 			/* Port G */
};

static uint32 regGpioInterruptStatusWrite[] =
{
	GPABISR, 	/* Port A */
	GPABISR, 	/* Port B */
	GPCDISR, 	/* Port C */
	GPCDISR,  	/* Port D */
	GPEFISR,  	/* Port E */
	GPEFISR,		/* Port F */
	GPGISR,		/* Port G */
};

static uint32 bitStartGpioInterruptStatusWrite[] =
{
	0, 			/* Port A */
	0, 			/* Port B */
	0,  			/* Port C */
	0, 			/* Port D */
	0, 			/* Port E */
	0, 			/* Port F */
	0, 			/* Port G */
};

//******************************************* IMR

static uint32 regGpioInterruptEnableRead[] =
{
	GPABIMR,	/* Port A */
	GPABIMR,	/* Port B */
	GPCDIMR, 	/* Port C */
	GPCDIMR,	/* Port D */
	GPEFIMR,		/* Port E */
	GPEFIMR,		/* Port F */
	GPGIMR,		/* Port G */
};

static uint32 bitStartGpioInterruptEnableRead[] =
{
	0, 			/* Port A */
	16,  			/* Port B */
	0,			/* Port C */
	16, 			/* Port D */
	0,  			/* Port E */
	16, 			/* Port F */
	0,  			/* Port G */
};

static uint32 regGpioInterruptEnableWrite[] =
{
	GPABIMR,	/* Port A */
	GPABIMR,	/* Port B */
	GPCDIMR, 	/* Port C */
	GPCDIMR,	/* Port D */
	GPEFIMR,		/* Port E */
	GPEFIMR,		/* Port F */
	GPGIMR,		/* Port G */
};

static uint32 bitStartGpioInterruptEnableWrite[] =
{
	0, 			/* Port A */
	0,  			/* Port B */
	0,			/* Port C */
	0, 			/* Port D */
	0,  			/* Port E */
	0, 			/* Port F */
	0,  			/* Port G */
};

/*
@func int32 | _getGpio | abstract GPIO registers 
@parm enum GPIO_FUNC | func | control/data/interrupt register
@parm enum GPIO_PORT | port | GPIO port
@parm uint32 | pin | pin number
@rvalue uint32 | value
@comm
This function is for internal use only. You don't need to care what register address of GPIO is.
This function abstracts these information.
*/
static uint32 _getGpio( enum GPIO_FUNC func, enum GPIO_PORT port, uint32 pin )
{
	//assert( port < GPIO_PORT_MAX );		
	//assert( func < GPIO_FUNC_MAX );
	//assert( pin < 8 );	

#if _GPIO_DEBUG_ >= 4	
	printk("[%s():%d] func=%d port=%d pin=%d\n", __FUNCTION__, __LINE__, func, port, pin );
#endif

	switch( func )
	{
				
		case GPIO_FUNC_DIRECTION:
#if _GPIO_DEBUG_ >= 5
			printk("[%s():%d] regGpioDirectionRead[port]=0x%08x  bitStartGpioDirectionRead[port]=%d\n", __FUNCTION__, __LINE__, regGpioDirectionRead[port], bitStartGpioDirectionRead[port] );
#endif
			if ( REG32(regGpioDirectionRead[port]) & ( (uint32)1 << (pin+bitStartGpioDirectionRead[port]) ) )
				return 1;
			else
				return 0;
			break;
			
		case GPIO_FUNC_DATA:
#if _GPIO_DEBUG_ >= 5
			printk("[%s():%d] regGpioDataRead[port]=0x%08x  bitStartGpioDataRead[port]=%d\n", __FUNCTION__, __LINE__, regGpioDataRead[port], bitStartGpioDataRead[port] );
#endif
			if ( REG32(regGpioDataRead[port]) & ( (uint32)1 << (pin+bitStartGpioDataRead[port]) ) )
				return 1;
			else
				return 0;
			break;
			
		case GPIO_FUNC_INTERRUPT_ENABLE:
#if _GPIO_DEBUG_ >= 5
			printk("[%s():%d] regGpioInterruptEnableRead[port]=0x%08x  bitStartGpioInterruptEnableRead[port]=%d\n", __FUNCTION__, __LINE__, regGpioInterruptEnableRead[port], bitStartGpioInterruptEnableRead[port] );
#endif
			if ( REG32(regGpioInterruptEnableRead[port]) & ( (uint32)1 <<(pin+bitStartGpioInterruptEnableRead[port]) ))
				return 1;
			else
				return 0;
			break;

		case GPIO_FUNC_INTERRUPT_STATUS:
#if _GPIO_DEBUG_ >= 5
			printk("[%s():%d] regGpioInterruptStatusRead[port]=0x%08x  bitStartGpioInterruptEnableRead[port]=%d\n", __FUNCTION__, __LINE__, regGpioInterruptStatusRead[port], bitStartGpioInterruptStatusRead[port] );
#endif
			if ( REG32(regGpioInterruptStatusRead[port]) & ( (uint32)1 << (pin+bitStartGpioInterruptStatusRead[port]) ) )
				return 1;
			else
				return 0;
			break;
			
		case GPIO_FUNC_MAX:
			//assert( 0 );
			break;
	}
	return 0xffffffff;
}


/*
@func int32 | _setGpio | abstract GPIO registers 
@parm enum GPIO_FUNC | func | control/data/interrupt register
@parm enum GPIO_PORT | port | GPIO port
@parm uint32 | pin | pin number
@parm uint32 | data | value
@rvalue NONE
@comm
This function is for internal use only. You don't need to care what register address of GPIO is.
This function abstracts these information.
*/
static void _setGpio( enum GPIO_FUNC func, enum GPIO_PORT port, uint32 pin, uint32 data )
{
	//assert( port < GPIO_PORT_MAX );
	//assert( func < GPIO_FUNC_MAX );
	//assert( pin < 8 );

#if _GPIO_DEBUG_ >= 4
	printk("[%s():%d] func=%d port=%d pin=%d data=%d\n", __FUNCTION__, __LINE__, func, port, pin, data );
#endif

	switch( func )
	{
		
		case GPIO_FUNC_DIRECTION:
#if _GPIO_DEBUG_ >= 5
			printk("[%s():%d] regGpioDirectionWrite[port]=0x%08x  bitStartGpioDirectionWrite[port]=%d\n", __FUNCTION__, __LINE__, regGpioDirectionWrite[port], bitStartGpioDirectionWrite[port] );
#endif
			if ( data )
				REG32(regGpioDirectionWrite[port]) |= (uint32)1 << (pin+bitStartGpioDirectionWrite[port]);
			else
				REG32(regGpioDirectionWrite[port]) &= ~((uint32)1 << (pin+bitStartGpioDirectionWrite[port]));
			break;

		case GPIO_FUNC_DATA:
#if _GPIO_DEBUG_ >= 5
			printk("[%s():%d] regGpioDataWrite[port]=0x%08x  bitStartGpioDataWrite[port]=%d\n", __FUNCTION__, __LINE__, regGpioDataWrite[port], bitStartGpioDataWrite[port] );
#endif
		#if _chiminer_gpiod_fixed
			if ( data )
				REG32(regGpioDataWrite[port]) = (REG32(regGpioDataWrite[port])>>16) | ((uint32)1 << (pin+bitStartGpioDataWrite[port]));
			else
				REG32(regGpioDataWrite[port]) = (REG32(regGpioDataWrite[port])>>16) & (~((uint32)1 << (pin+bitStartGpioDataWrite[port])));
			break;
		#else
			if ( data )
				REG32(regGpioDataWrite[port]) |= (uint32)1 << (pin+bitStartGpioDataWrite[port]);
			else
				REG32(regGpioDataWrite[port]) &= ~((uint32)1 << (pin+bitStartGpioDataWrite[port]));
			break;
		#endif
		case GPIO_FUNC_INTERRUPT_ENABLE:
#if _GPIO_DEBUG_ >= 5
			printk("[%s():%d] regGpioInterruptEnableWrite[port]=0x%08x  bitStartGpioInterruptEnableWrite[port]=%d\n", __FUNCTION__, __LINE__, regGpioInterruptEnableWrite[port], bitStartGpioInterruptEnableWrite[port] );
#endif
			if (data)
				REG32(regGpioInterruptEnableWrite[port]) |= (uint32)1 << (pin+bitStartGpioInterruptEnableWrite[port]);
			else
				REG32(regGpioInterruptEnableWrite[port]) &= ~((uint32)1 << (pin+bitStartGpioInterruptEnableWrite[port]));
			break;

		case GPIO_FUNC_INTERRUPT_STATUS:
#if _GPIO_DEBUG_ >= 5
			printk("[%s():%d] regGpioInterruptStatusWrite[port]=0x%08x  bitStartGpioInterruptStatusWrite[port]=%d\n", __FUNCTION__, __LINE__, regGpioInterruptStatusWrite[port], bitStartGpioInterruptStatusWrite[port] );
#endif
			if ( data )
				REG32(regGpioInterruptStatusWrite[port]) |= (uint32)1 << (pin+bitStartGpioInterruptStatusWrite[port]);
			else
				REG32(regGpioInterruptStatusWrite[port]) &= ~((uint32)1 << (pin+bitStartGpioInterruptStatusWrite[port]));
			break;

		case GPIO_FUNC_MAX:
			//assert( 0 );
			break;
	}
}


/*************************************************************************************/
/*
@func int32 | _rtl8186_initGpioPin | Initiate a specifed GPIO port.
@parm uint32 | gpioId | The GPIO port that will be configured
@parm enum GPIO_PERIPHERAL | dedicate | Dedicated peripheral type
@parm enum GPIO_DIRECTION | direction | Data direction, in or out
@parm enum GPIO_INTERRUPT_TYPE | interruptEnable | Interrupt mode
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
This function is used to initialize GPIO port.
*/
int32 _rtl8186_initGpioPin( uint32 gpioId, enum GPIO_DIRECTION direction, 
                                           enum GPIO_INTERRUPT_TYPE interruptEnable )
 /* Note: In RTL8186, parm "dedicate" is not make sense. It can be any value because of nothing happen. */                                          
{
	uint32 port = GPIO_PORT( gpioId );
	uint32 pin = GPIO_PIN( gpioId );

	if ( port >= GPIO_PORT_MAX ) return FAILED;
	//if ( pin >= 8 ) return FAILED;	/* This judgment does not suit to RTL8186. GPIO PIN definition is not the same as RTL865x. */

		
	_setGpio( GPIO_FUNC_DIRECTION, port, pin, direction );

	_setGpio( GPIO_FUNC_INTERRUPT_ENABLE, port, pin, interruptEnable );
	

	return SUCCESS;
}


/*
@func int32 | _rtl8186_getGpioDataBit | Get the bit value of a specified GPIO ID.
@parm uint32 | gpioId | GPIO ID
@parm uint32* | data | Pointer to store return value
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
int32 _rtl8186_getGpioDataBit( uint32 gpioId, uint32* pData )
{
	uint32 port = GPIO_PORT( gpioId );
	uint32 pin = GPIO_PIN( gpioId );

	if ( port >= GPIO_PORT_MAX ) return FAILED;
	//if ( pin >= 8 ) return FAILED;
	if ( pData == NULL ) return FAILED;

	*pData = _getGpio( GPIO_FUNC_DATA, port, pin );
#if _GPIO_DEBUG_ >= 3
	printk("[%s():%d] (port=%d,pin=%d)=%d\n", __FUNCTION__, __LINE__, port, pin, *pData );
#endif


	return SUCCESS;
}


/*
@func int32 | _rtl8186_setGpioDataBit | Set the bit value of a specified GPIO ID.
@parm uint32 | gpioId | GPIO ID
@parm uint32 | data | Data to write
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
int32 _rtl8186_setGpioDataBit( uint32 gpioId, uint32 data )
{
	uint32 port = GPIO_PORT( gpioId );
	uint32 pin = GPIO_PIN( gpioId );

	if ( port >= GPIO_PORT_MAX ) return FAILED;
	//if ( pin >= 8 ) return FAILED;
#if 0
	if ( _getGpio( GPIO_FUNC_DIRECTION, port, pin ) == GPIO_DIR_IN ) return FAILED; /* read only */  
#endif

#if _GPIO_DEBUG_ >= 3
	printk("[%s():%d] (port=%d,pin=%d)=%d\n", __FUNCTION__, __LINE__, port, pin, data );
#endif
	_setGpio( GPIO_FUNC_DATA, port, pin, data );


	return SUCCESS;
}


#endif//CONFIG_RTK_VOIP_DRIVERS_PCM8186


/*==================== FOR RTL865x ==================*/

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651


#include "asicRegs.h"
#include "assert.h"

/*
 * 0 -- less debug information
 * 5 -- more debug information
 */
#define _GPIO_DEBUG_ 0

enum GPIO_FUNC
{
	GPIO_FUNC_DEDICATE,
	GPIO_FUNC_DEDICATE_PERIPHERAL_TYPE,
	GPIO_FUNC_DIRECTION,
	GPIO_FUNC_DATA,
	GPIO_FUNC_INTERRUPT_STATUS,
	GPIO_FUNC_INTERRUPT_ENABLE,
	GPIO_FUNC_MAX,
};


static uint32 regGpioControl[] = 
{
	PABCCNR, /* Port A */
	PABCCNR, /* Port B */
	PABCCNR, /* Port C */
	PDEPTCR, /* Port D */
	PDEPTCR, /* Port E */
	PFGHICNR,/* Port F */
	PFGHICNR,/* Port G */
	PFGHICNR,/* Port H */
	PFGHICNR,/* Port I */
};

static uint32 bitStartGpioControl[] =
{
	24, /* Port A */
	16, /* Port B */
	8,  /* Port C */
	24, /* Port D */
	16, /* Port E */
	24, /* Port F */
	16, /* Port G */
	8,  /* Port H */
	0,  /* Port I */
};

static uint32 regGpioDedicatePeripheralType[] = 
{
	PABCPTCR, /* Port A */
	PABCPTCR, /* Port B */
	PABCPTCR, /* Port C */
	PDEPTCR,  /* Port D */
	PDEPTCR,  /* Port E */
	0,        /* Port F */
	0,        /* Port G */
	0,        /* Port H */
	0,        /* Port I */
};

static uint32 bitStartGpioDedicatePeripheralType[] =
{
	24, /* Port A */
	16, /* Port B */
	8,  /* Port C */
	24, /* Port D */
	16, /* Port E */
	0,  /* Port F */
	0,  /* Port G */
	0,  /* Port H */
	0,  /* Port I */
};

static uint32 regGpioDirection[] =
{
	PABCDIR, /* Port A */
	PABCDIR, /* Port B */
	PABCDIR, /* Port C */
	PDEDIR,  /* Port D */
	PDEDIR,  /* Port E */
	PFGHIDIR,/* Port F */
	PFGHIDIR,/* Port G */
	PFGHIDIR,/* Port H */
	PFGHIDIR,/* Port I */
};

static uint32 bitStartGpioDirection[] =
{
	24, /* Port A */
	16, /* Port B */
	8,  /* Port C */
	24, /* Port D */
	16, /* Port E */
	24, /* Port F */
	16, /* Port G */
	8,  /* Port H */
	0,  /* Port I */
};

static uint32 regGpioData[] =
{
	PABCDAT, /* Port A */
	PABCDAT, /* Port B */
	PABCDAT, /* Port C */
	PDEDAT,  /* Port D */
	PDEDAT,  /* Port E */
	PFGHIDAT,/* Port F */
	PFGHIDAT,/* Port G */
	PFGHIDAT,/* Port H */
	PFGHIDAT,/* Port I */
};

static uint32 bitStartGpioData[] =
{
	24, /* Port A */
	16, /* Port B */
	8,  /* Port C */
	24, /* Port D */
	16, /* Port E */
	24, /* Port F */
	16, /* Port G */
	8,  /* Port H */
	0,  /* Port I */
};

static uint32 regGpioInterruptStatus[] =
{
	PABCISR, /* Port A */
	PABCISR, /* Port B */
	PABCISR, /* Port C */
	PDEISR,  /* Port D */
	PDEISR,  /* Port E */
	PFGHIISR,/* Port F */
	PFGHIISR,/* Port G */
	PFGHIISR,/* Port H */
	PFGHIISR,/* Port I */
};

static uint32 bitStartGpioInterruptStatus[] =
{
	24, /* Port A */
	16, /* Port B */
	8,  /* Port C */
	24, /* Port D */
	16, /* Port E */
	24, /* Port F */
	16, /* Port G */
	8,  /* Port H */
	0,  /* Port I */
};

static uint32 regGpioInterruptEnable[] =
{
	PABIMR,/* Port A */
	PABIMR,/* Port B */
	PCIMR, /* Port C */
	PDEIMR,/* Port D */
	PDEIMR,/* Port E */
	PFGIMR,/* Port F */
	PFGIMR,/* Port G */
	PHIIMR,/* Port H */
	PHIIMR,/* Port I */
};

static uint32 bitStartGpioInterruptEnable[] =
{
	16, /* Port A */
	0,  /* Port B */
	16, /* Port C */
	16, /* Port D */
	0,  /* Port E */
	16, /* Port F */
	0,  /* Port G */
	16, /* Port H */
	0,  /* Port I */
};



/*
@func int32 | _getGpio | abstract GPIO registers 
@parm enum GPIO_FUNC | func | control/data/interrupt register
@parm enum GPIO_PORT | port | GPIO port
@parm uint32 | pin | pin number
@rvalue uint32 | value
@comm
This function is for internal use only. You don't need to care what register address of GPIO is.
This function abstracts these information.
*/
static uint32 _getGpio( enum GPIO_FUNC func, enum GPIO_PORT port, uint32 pin )
{
	assert( port < GPIO_PORT_MAX );
	assert( func < GPIO_FUNC_MAX );
	assert( pin < 8 );

#if _GPIO_DEBUG_ >= 4
	rtlglue_printf("[%s():%d] func=%d port=%d pin=%d\n", __FUNCTION__, __LINE__, func, port, pin );
#endif

	switch( func )
	{
		case GPIO_FUNC_DEDICATE:
#if _GPIO_DEBUG_ >= 5
			rtlglue_printf("[%s():%d] regGpioControl[port]=0x%08x  bitStartGpioControl[port]=%d\n", __FUNCTION__, __LINE__, regGpioControl[port], bitStartGpioControl[port] );
#endif
			if ( REG32(regGpioControl[port]) & ( (uint32)1 << (pin+bitStartGpioControl[port]) ) )
				return 1;
			else
				return 0;
			break;
			
		case GPIO_FUNC_DEDICATE_PERIPHERAL_TYPE:
			assert( port <= GPIO_PORT_E );

#if _GPIO_DEBUG_ >= 5
			rtlglue_printf("[%s():%d] regGpioDedicatePeripheralType[port]=0x%08x  bitStartGpioDedicatePeripheralType[port]=%d\n", __FUNCTION__, __LINE__, regGpioDedicatePeripheralType[port], bitStartGpioDedicatePeripheralType[port] );
#endif
			if ( REG32(regGpioDedicatePeripheralType[port]) & ( (uint32)1 << (pin+bitStartGpioDedicatePeripheralType[port]) ) )
				return 1;
			else
				return 0;
			break;
			
		case GPIO_FUNC_DIRECTION:
#if _GPIO_DEBUG_ >= 5
			rtlglue_printf("[%s():%d] regGpioDirection[port]=0x%08x  bitStartGpioDirection[port]=%d\n", __FUNCTION__, __LINE__, regGpioDirection[port], bitStartGpioDirection[port] );
#endif
			if ( REG32(regGpioDirection[port]) & ( (uint32)1 << (pin+bitStartGpioDirection[port]) ) )
				return 1;
			else
				return 0;
			break;
			
		case GPIO_FUNC_DATA:
#if _GPIO_DEBUG_ >= 5
			rtlglue_printf("[%s():%d] regGpioData[port]=0x%08x  bitStartGpioData[port]=%d\n", __FUNCTION__, __LINE__, regGpioData[port], bitStartGpioData[port] );
#endif
			if ( REG32(regGpioData[port]) & ( (uint32)1 << (pin+bitStartGpioData[port]) ) )
				return 1;
			else
				return 0;
			break;
			
		case GPIO_FUNC_INTERRUPT_ENABLE:
#if _GPIO_DEBUG_ >= 5
			rtlglue_printf("[%s():%d] regGpioInterruptEnable[port]=0x%08x  bitStartGpioInterruptEnable[port]=%d\n", __FUNCTION__, __LINE__, regGpioInterruptEnable[port], bitStartGpioInterruptEnable[port] );
#endif
			return ( REG32(regGpioInterruptEnable[port]) >> (pin*2+bitStartGpioInterruptEnable[port]) ) & (uint32)0x3;
			break;

		case GPIO_FUNC_INTERRUPT_STATUS:
#if _GPIO_DEBUG_ >= 5
			rtlglue_printf("[%s():%d] regGpioInterruptStatus[port]=0x%08x  bitStartGpioInterruptEnable[port]=%d\n", __FUNCTION__, __LINE__, regGpioInterruptStatus[port], bitStartGpioInterruptStatus[port] );
#endif
			if ( REG32(regGpioInterruptStatus[port]) & ( (uint32)1 << (pin+bitStartGpioInterruptStatus[port]) ) )
				return 1;
			else
				return 0;
			break;
			
		case GPIO_FUNC_MAX:
			assert( 0 );
			break;
	}
	return 0xffffffff;
}


/*
@func int32 | _setGpio | abstract GPIO registers 
@parm enum GPIO_FUNC | func | control/data/interrupt register
@parm enum GPIO_PORT | port | GPIO port
@parm uint32 | pin | pin number
@parm uint32 | data | value
@rvalue NONE
@comm
This function is for internal use only. You don't need to care what register address of GPIO is.
This function abstracts these information.
*/
static void _setGpio( enum GPIO_FUNC func, enum GPIO_PORT port, uint32 pin, uint32 data )
{
	assert( port < GPIO_PORT_MAX );
	assert( func < GPIO_FUNC_MAX );
	assert( pin < 8 );
	
#if _GPIO_DEBUG_ >= 4
	rtlglue_printf("[%s():%d] func=%d port=%d pin=%d data=%d\n", __FUNCTION__, __LINE__, func, port, pin, data );
#endif

	switch( func )
	{
		case GPIO_FUNC_DEDICATE:
#if _GPIO_DEBUG_ >= 5
			rtlglue_printf("[%s():%d] regGpioControl[port]=0x%08x  bitStartGpioControl[port]=%d\n", __FUNCTION__, __LINE__, regGpioControl[port], bitStartGpioControl[port] );
#endif
			if ( data )
				REG32(regGpioControl[port]) |= (uint32)1 << (pin+bitStartGpioControl[port]);
			else
				REG32(regGpioControl[port]) &= ~((uint32)1 << (pin+bitStartGpioControl[port]));
			break;
			
		case GPIO_FUNC_DEDICATE_PERIPHERAL_TYPE:
#if _GPIO_DEBUG_ >= 5
			rtlglue_printf("[%s():%d] regGpioDedicatePeripheralType[port]=0x%08x  bitStartGpioDedicatePeripheralType[port]=%d\n", __FUNCTION__, __LINE__, regGpioDedicatePeripheralType[port], bitStartGpioDedicatePeripheralType[port] );
#endif
			assert( port <= GPIO_PORT_E );
			if ( data )
				REG32(regGpioDedicatePeripheralType[port]) |= (uint32)1 << (pin+bitStartGpioDedicatePeripheralType[port]);
			else
				REG32(regGpioDedicatePeripheralType[port]) &= ~((uint32)1 << (pin+bitStartGpioDedicatePeripheralType[port]));
			break;
			
		case GPIO_FUNC_DIRECTION:
#if _GPIO_DEBUG_ >= 5
			rtlglue_printf("[%s():%d] regGpioDirection[port]=0x%08x  bitStartGpioDirection[port]=%d\n", __FUNCTION__, __LINE__, regGpioDirection[port], bitStartGpioDirection[port] );
#endif
			if ( data )
				REG32(regGpioDirection[port]) |= (uint32)1 << (pin+bitStartGpioDirection[port]);
			else
				REG32(regGpioDirection[port]) &= ~((uint32)1 << (pin+bitStartGpioDirection[port]));
			break;

		case GPIO_FUNC_DATA:
#if _GPIO_DEBUG_ >= 5
			rtlglue_printf("[%s():%d] regGpioData[port]=0x%08x  bitStartGpioData[port]=%d\n", __FUNCTION__, __LINE__, regGpioData[port], bitStartGpioData[port] );
#endif
			if ( data )
				REG32(regGpioData[port]) |= (uint32)1 << (pin+bitStartGpioData[port]);
			else
				REG32(regGpioData[port]) &= ~((uint32)1 << (pin+bitStartGpioData[port]));
			break;
			
		case GPIO_FUNC_INTERRUPT_ENABLE:
#if _GPIO_DEBUG_ >= 5
			rtlglue_printf("[%s():%d] regGpioInterruptEnable[port]=0x%08x  bitStartGpioInterruptEnable[port]=%d\n", __FUNCTION__, __LINE__, regGpioInterruptEnable[port], bitStartGpioInterruptEnable[port] );
#endif
			REG32(regGpioInterruptEnable[port]) &= ~((uint32)0x3 << (pin*2+bitStartGpioInterruptEnable[port]));
			REG32(regGpioInterruptEnable[port]) |= (uint32)data << (pin*2+bitStartGpioInterruptEnable[port]);
			break;

		case GPIO_FUNC_INTERRUPT_STATUS:
#if _GPIO_DEBUG_ >= 5
			rtlglue_printf("[%s():%d] regGpioInterruptStatus[port]=0x%08x  bitStartGpioInterruptStatus[port]=%d\n", __FUNCTION__, __LINE__, regGpioInterruptStatus[port], bitStartGpioInterruptStatus[port] );
#endif
			if ( data )
				REG32(regGpioInterruptStatus[port]) |= (uint32)1 << (pin+bitStartGpioInterruptStatus[port]);
			else
				REG32(regGpioInterruptStatus[port]) &= ~((uint32)1 << (pin+bitStartGpioInterruptStatus[port]));
			break;

		case GPIO_FUNC_MAX:
			assert( 0 );
			break;
	}
}


/*
@func int32 | _rtl865x_initGpioPin | Initiate a specifed GPIO port.
@parm uint32 | gpioId | The GPIO port that will be configured
@parm enum GPIO_PERIPHERAL | dedicate | Dedicated peripheral type
@parm enum GPIO_DIRECTION | direction | Data direction, in or out
@parm enum GPIO_INTERRUPT_TYPE | interruptEnable | Interrupt mode
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
This function is used to initialize GPIO port.
*/
int32 _rtl865x_initGpioPin( uint32 gpioId, enum GPIO_PERIPHERAL dedicate, 
                                           enum GPIO_DIRECTION direction, 
                                           enum GPIO_INTERRUPT_TYPE interruptEnable )
{
	uint32 port = GPIO_PORT( gpioId );
	uint32 pin = GPIO_PIN( gpioId );

	if ( port >= GPIO_PORT_MAX ) return FAILED;
	if ( pin >= 8 ) return FAILED;

	switch( dedicate )
	{
		case GPIO_PERI_GPIO:
			_setGpio( GPIO_FUNC_DEDICATE, port, pin, 0 );

			if ( port == GPIO_PORT_D || port == GPIO_PORT_E ) /*change Mii interface IO as GPIO pins*/
				REG32( MISCCR ) |= P5_LINK_PCMCIA << P5_LINK_OFFSET;

			break;
		case GPIO_PERI_TYPE0:
			if( port > GPIO_PORT_E ) return FAILED;
			_setGpio( GPIO_FUNC_DEDICATE, port, pin, 1 );
			_setGpio( GPIO_FUNC_DEDICATE_PERIPHERAL_TYPE, port, pin, 0 );
			break;
		case GPIO_PERI_TYPE1:
			if( port > GPIO_PORT_E ) return FAILED;
			_setGpio( GPIO_FUNC_DEDICATE, port, pin, 1 );
			_setGpio( GPIO_FUNC_DEDICATE_PERIPHERAL_TYPE, port, pin, 1 );
			break;
	}
	
	_setGpio( GPIO_FUNC_DIRECTION, port, pin, direction );

	_setGpio( GPIO_FUNC_INTERRUPT_ENABLE, port, pin, interruptEnable );

	return SUCCESS;
}


/*
@func int32 | _rtl865x_getGpioDataBit | Get the bit value of a specified GPIO ID.
@parm uint32 | gpioId | GPIO ID
@parm uint32* | data | Pointer to store return value
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
int32 _rtl865x_getGpioDataBit( uint32 gpioId, uint32* pData )
{
	uint32 port = GPIO_PORT( gpioId );
	uint32 pin = GPIO_PIN( gpioId );

	if ( port >= GPIO_PORT_MAX ) return FAILED;
	if ( pin >= 8 ) return FAILED;
	if ( pData == NULL ) return FAILED;

	*pData = _getGpio( GPIO_FUNC_DATA, port, pin );
#if _GPIO_DEBUG_ >= 3
	rtlglue_printf("[%s():%d] (port=%d,pin=%d)=%d\n", __FUNCTION__, __LINE__, port, pin, *pData );
#endif

	return SUCCESS;
}


/*
@func int32 | _rtl865x_setGpioDataBit | Set the bit value of a specified GPIO ID.
@parm uint32 | gpioId | GPIO ID
@parm uint32 | data | Data to write
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
int32 _rtl865x_setGpioDataBit( uint32 gpioId, uint32 data )
{
	uint32 port = GPIO_PORT( gpioId );
	uint32 pin = GPIO_PIN( gpioId );

	if ( port >= GPIO_PORT_MAX ) return FAILED;
	if ( pin >= 8 ) return FAILED;
#if 0
	if ( _getGpio( GPIO_FUNC_DIRECTION, port, pin ) == GPIO_DIR_IN ) return FAILED; /* read only */
#endif

#if _GPIO_DEBUG_ >= 3
	rtlglue_printf("[%s():%d] (port=%d,pin=%d)=%d\n", __FUNCTION__, __LINE__, port, pin, data );
#endif
	_setGpio( GPIO_FUNC_DATA, port, pin, data );

	return SUCCESS;
}


#if 0
/*
@func int32 | _rtl865x_fetchGpioInterruptStatus | Fetch the interrupt status of a specified GPIO ID.
@parm uint32 | gpioId | GPIO ID
@parm uint32* | data | Pointer to store interrupt status
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
int32 _rtl865x_fetchGpioInterruptStatus( uint32 gpioId, uint32* pStatus )
{
	uint32 port = GPIO_PORT( gpioId );
	uint32 pin = GPIO_PIN( gpioId );

	if ( port >= GPIO_PORT_MAX ) return FAILED;
	if ( pin >= 8 ) return FAILED;
	if ( pStatus == NULL ) return FAILED;

	*pStatus = _getGpio( GPIO_FUNC_INTERRUPT_STATUS, port, pin );

	return SUCCESS;
}


/*
@func int32 | _rtl865x_fetchGpioInterruptStatus | Clear the interrupt status of a specified GPIO ID.
@parm uint32 | gpioId | GPIO ID
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
int32 _rtl865x_clearGpioInterruptStatus( uint32 gpioId )
{
	uint32 port = GPIO_PORT( gpioId );
	uint32 pin = GPIO_PIN( gpioId );

	if ( port >= GPIO_PORT_MAX ) return FAILED;
	if ( pin >= 8 ) return FAILED;

	_setGpio( GPIO_FUNC_INTERRUPT_STATUS, port, pin, 1 );

	return SUCCESS;
}
#endif

#if 0/* not implemented */
int32 _rtl865x_allocGpioBit( uint32* gpioId );
int32 _rtl865x_releaseGpioBit( uint32 gpioId );
#endif




#endif//CONFIG_RTK_VOIP_DRIVERS_PCM8651

/*==================== FOR RTL867x ==================*/
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8671

enum GPIO_FUNC	
{
	GPIO_FUNC_DIRECTION,
	GPIO_FUNC_DATA,
	GPIO_FUNC_INTERRUPT_STATUS,
	GPIO_FUNC_INTERRUPT_ENABLE,
	GPIO_FUNC_MAX,
};


//******************************************* Direction
#ifndef USE_8671_GPIO_API
static uint32 regGpioDirectionRead[] =
{
	GPADIR, 	/* Port A */
	GPBDIR, 	/* Port B */
};

static uint32 bitStartGpioDirectionRead[] =
{
	16, 			/* Port A */
	16, 			/* Port B */
};

static uint32 regGpioDirectionWrite[] =
{
	GPADIR, 	/* Port A */
	GPBDIR, 	/* Port B */
};

static uint32 bitStartGpioDirectionWrite[] =
{
	16, 			/* Port A */
	16, 			/* Port B */
};

//******************************************* Data

static uint32 regGpioDataRead[] =
{
	GPADATA, 	/* Port A */
	GPBDATA, 	/* Port B */
};

static uint32 bitStartGpioDataRead[] =
{
	16, 			/* Port A */
	16, 			/* Port B */
};

static uint32 regGpioDataWrite[] =
{
	GPADATA, 	/* Port A */
	GPBDATA, 	/* Port B */
};

static uint32 bitStartGpioDataWrite[] =
{
	16, 			/* Port A */
	16, 			/* Port B */
};

//******************************************* ISR

static uint32 regGpioInterruptStatusRead[] =
{
	GPAISR, 	/* Port A */
	GPBISR, 	/* Port B */
};

static uint32 bitStartGpioInterruptStatusRead[] =
{
	0, 			/* Port A */
	0, 			/* Port B */
};

static uint32 regGpioInterruptStatusWrite[] =
{
	GPAISR, 	/* Port A */
	GPBISR, 	/* Port B */
};

static uint32 bitStartGpioInterruptStatusWrite[] =
{
	0, 			/* Port A */
	0, 			/* Port B */
};

//******************************************* IMR

static uint32 regGpioInterruptEnableRead[] =
{
	GPAIMR,	/* Port A */
	GPBIMR,	/* Port B */
};

static uint32 bitStartGpioInterruptEnableRead[] =
{
	0, 			/* Port A */
	0,  			/* Port B */
};

static uint32 regGpioInterruptEnableWrite[] =
{
	GPAIMR,	/* Port A */
	GPBIMR,	/* Port B */
};

static uint32 bitStartGpioInterruptEnableWrite[] =
{
	0, 			/* Port A */
	0,  		/* Port B */
};
#endif
/*
@func int32 | _getGpio | abstract GPIO registers 
@parm enum GPIO_FUNC | func | control/data/interrupt register
@parm enum GPIO_PORT | port | GPIO port
@parm uint32 | pin | pin number
@rvalue uint32 | value
@comm
This function is for internal use only. You don't need to care what register address of GPIO is.
This function abstracts these information.
*/
static uint32 _getGpio( enum GPIO_FUNC func, enum GPIO_PORT port, uint32 pin )
{
	//assert( port < GPIO_PORT_MAX );		
	//assert( func < GPIO_FUNC_MAX );
	//assert( pin < 8 );	
#ifdef USE_8671_GPIO_API
	switch(func)
	{	
		case GPIO_FUNC_DATA:	// to do GPIOset and GPIOconfig
			return gpioRead(pin);			
		default:
			// do not support these function at 8671 gpio.c
			break;
	}

#else
#if _GPIO_DEBUG_ >= 4	
	printk("[%s():%d] func=%d port=%d pin=%d\n", __FUNCTION__, __LINE__, func, port, pin );
#endif

	switch( func )
	{
				
		case GPIO_FUNC_DIRECTION:
#if _GPIO_DEBUG_ >= 5
			printk("[%s():%d] regGpioDirectionRead[port]=0x%08x  bitStartGpioDirectionRead[port]=%d\n", __FUNCTION__, __LINE__, regGpioDirectionRead[port], bitStartGpioDirectionRead[port] );
#endif
			if ( REG32(regGpioDirectionRead[port]) & ( (uint32)1 << (pin+bitStartGpioDirectionRead[port]) ) )
				return 1;
			else
				return 0;
			break;
			
		case GPIO_FUNC_DATA:
#if _GPIO_DEBUG_ >= 5
			printk("[%s():%d] regGpioDataRead[port]=0x%08x  bitStartGpioDataRead[port]=%d\n", __FUNCTION__, __LINE__, regGpioDataRead[port], bitStartGpioDataRead[port] );
#endif
			if ( REG32(regGpioDataRead[port]) & ( (uint32)1 << (pin+bitStartGpioDataRead[port]) ) )
				return 1;
			else
				return 0;
			break;
			
		case GPIO_FUNC_INTERRUPT_ENABLE:
#if _GPIO_DEBUG_ >= 5
			printk("[%s():%d] regGpioInterruptEnableRead[port]=0x%08x  bitStartGpioInterruptEnableRead[port]=%d\n", __FUNCTION__, __LINE__, regGpioInterruptEnableRead[port], bitStartGpioInterruptEnableRead[port] );
#endif
			if ( REG32(regGpioInterruptEnableRead[port]) & ( (uint32)1 <<(pin+bitStartGpioInterruptEnableRead[port]) ))
				return 1;
			else
				return 0;
			break;

		case GPIO_FUNC_INTERRUPT_STATUS:
#if _GPIO_DEBUG_ >= 5
			printk("[%s():%d] regGpioInterruptStatusRead[port]=0x%08x  bitStartGpioInterruptEnableRead[port]=%d\n", __FUNCTION__, __LINE__, regGpioInterruptStatusRead[port], bitStartGpioInterruptStatusRead[port] );
#endif
			if ( REG32(regGpioInterruptStatusRead[port]) & ( (uint32)1 << (pin+bitStartGpioInterruptStatusRead[port]) ) )
				return 1;
			else
				return 0;
			break;
			
		case GPIO_FUNC_MAX:
			//assert( 0 );
			break;
	}
#endif	
	return 0xffffffff;
}




/*
@func int32 | _setGpio | abstract GPIO registers 
@parm enum GPIO_FUNC | func | control/data/interrupt register
@parm enum GPIO_PORT | port | GPIO port
@parm uint32 | pin | pin number
@parm uint32 | data | value
@rvalue NONE
@comm
This function is for internal use only. You don't need to care what register address of GPIO is.
This function abstracts these information.
*/
static void _setGpio( enum GPIO_FUNC func, enum GPIO_PORT port, uint32 pin, uint32 data )
{
	//assert( port < GPIO_PORT_MAX );
	//assert( func < GPIO_FUNC_MAX );
	//assert( pin < 8 );

#ifdef USE_8671_GPIO_API
	switch(func)
	{
		case GPIO_FUNC_DIRECTION: // to do GPIOconfig
				gpioConfig(pin, data);
			break;
		case GPIO_FUNC_DATA:	// to do GPIOset and GPIOconfig
			if (data)
				gpioSet(port);
			else
				gpioClear(port);
			break;
		default:
			// do not support these function at 8671 gpio.c
			break;
	}

#else
#if _GPIO_DEBUG_ >= 4
	printk("[%s():%d] func=%d port=%d pin=%d data=%d\n", __FUNCTION__, __LINE__, func, port, pin, data );
#endif

	switch( func )
	{
		
		case GPIO_FUNC_DIRECTION:
#if _GPIO_DEBUG_ >= 5
			printk("[%s():%d] regGpioDirectionWrite[port]=0x%08x  bitStartGpioDirectionWrite[port]=%d\n", __FUNCTION__, __LINE__, regGpioDirectionWrite[port], bitStartGpioDirectionWrite[port] );
#endif
			if ( data )
				REG32(regGpioDirectionWrite[port]) |= (uint32)1 << (pin+bitStartGpioDirectionWrite[port]);
			else
				REG32(regGpioDirectionWrite[port]) &= ~((uint32)1 << (pin+bitStartGpioDirectionWrite[port]));
			break;

		case GPIO_FUNC_DATA:
#if _GPIO_DEBUG_ >= 5
			printk("[%s():%d] regGpioDataWrite[port]=0x%08x  bitStartGpioDataWrite[port]=%d\n", __FUNCTION__, __LINE__, regGpioDataWrite[port], bitStartGpioDataWrite[port] );
#endif
			if ( data )
				REG32(regGpioDataWrite[port]) |= (uint32)1 << (pin+bitStartGpioDataWrite[port]);
			else
				REG32(regGpioDataWrite[port]) &= ~((uint32)1 << (pin+bitStartGpioDataWrite[port]));
			break;
		
		case GPIO_FUNC_INTERRUPT_ENABLE:
#if _GPIO_DEBUG_ >= 5
			printk("[%s():%d] regGpioInterruptEnableWrite[port]=0x%08x  bitStartGpioInterruptEnableWrite[port]=%d\n", __FUNCTION__, __LINE__, regGpioInterruptEnableWrite[port], bitStartGpioInterruptEnableWrite[port] );
#endif
			if (data)
				REG32(regGpioInterruptEnableWrite[port]) |= (uint32)1 << (14-pin*2+bitStartGpioInterruptEnableWrite[port]);
			else
				REG32(regGpioInterruptEnableWrite[port]) &= ~((uint32)1 << (14-pin*2+bitStartGpioInterruptEnableWrite[port]));
			break;

		case GPIO_FUNC_INTERRUPT_STATUS:
#if _GPIO_DEBUG_ >= 5
			printk("[%s():%d] regGpioInterruptStatusWrite[port]=0x%08x  bitStartGpioInterruptStatusWrite[port]=%d\n", __FUNCTION__, __LINE__, regGpioInterruptStatusWrite[port], bitStartGpioInterruptStatusWrite[port] );
#endif
			if ( data )
				REG32(regGpioInterruptStatusWrite[port]) |= (uint32)1 << (pin+bitStartGpioInterruptStatusWrite[port]);
			else
				REG32(regGpioInterruptStatusWrite[port]) &= ~((uint32)1 << (pin+bitStartGpioInterruptStatusWrite[port]));
			break;

		case GPIO_FUNC_MAX:
			//assert( 0 );
			break;
	}
#endif	
}


/*************************************************************************************/
/*
@func int32 | _rtl867x_initGpioPin | Initiate a specifed GPIO port.
@parm uint32 | gpioId | The GPIO port that will be configured
@parm enum GPIO_PERIPHERAL | dedicate | Dedicated peripheral type
@parm enum GPIO_DIRECTION | direction | Data direction, in or out
@parm enum GPIO_INTERRUPT_TYPE | interruptEnable | Interrupt mode
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
This function is used to initialize GPIO port.
*/
int32 _rtl867x_initGpioPin( uint32 gpioId, enum GPIO_DIRECTION direction, 
                                           enum GPIO_INTERRUPT_TYPE interruptEnable )
 /* Note: In rtl867x, parm "dedicate" is not make sense. It can be any value because of nothing happen. */                                          
{
#ifdef USE_8671_GPIO_API
	_setGpio(GPIO_FUNC_DIRECTION, gpioId, gpioId,  direction );
#else 
	uint32 port = GPIO_PORT( gpioId );
	uint32 pin = GPIO_PIN( gpioId );

	if ( port >= GPIO_PORT_MAX ) return FAILED;
	//if ( pin >= 8 ) return FAILED;	/* This judgment does not suit to rtl867x. GPIO PIN definition is not the same as RTL865x. */

		
	_setGpio( GPIO_FUNC_DIRECTION, port, pin, direction );

	_setGpio( GPIO_FUNC_INTERRUPT_ENABLE, port, pin, interruptEnable );
#endif	

	return SUCCESS;
}


/*
@func int32 | _rtl867x_getGpioDataBit | Get the bit value of a specified GPIO ID.
@parm uint32 | gpioId | GPIO ID
@parm uint32* | data | Pointer to store return value
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
int32 _rtl867x_getGpioDataBit( uint32 gpioId, uint32* pData )
{

#ifdef USE_8671_GPIO_API
	if ( pData == NULL ) return FAILED;
	*pData = _getGpio( GPIO_FUNC_DATA, gpioId, gpioId );
	//printk("\r\n_rtl867x_getGpioDataBit : gpioId :%d, data:%d",gpioId, *pData);
#else
	uint32 port = GPIO_PORT( gpioId );
	uint32 pin = GPIO_PIN( gpioId );

	if ( port >= GPIO_PORT_MAX ) return FAILED;
	//if ( pin >= 8 ) return FAILED;
	if ( pData == NULL ) return FAILED;

	*pData = _getGpio( GPIO_FUNC_DATA, port, pin );
#if _GPIO_DEBUG_ >= 3
	printk("[%s():%d] (port=%d,pin=%d)=%d\n", __FUNCTION__, __LINE__, port, pin, *pData );
#endif
#endif

	return SUCCESS;
}


/*
@func int32 | _rtl867x_setGpioDataBit | Set the bit value of a specified GPIO ID.
@parm uint32 | gpioId | GPIO ID
@parm uint32 | data | Data to write
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
int32 _rtl867x_setGpioDataBit( uint32 gpioId, uint32 data )
{

#ifdef USE_8671_GPIO_API
	//printk("\r\nSet : gpioId :%d, data:%d",gpioId, data);
	_setGpio( GPIO_FUNC_DATA,gpioId,gpioId, data );

#else
	uint32 port = GPIO_PORT( gpioId );
	uint32 pin = GPIO_PIN( gpioId );

	if ( port >= GPIO_PORT_MAX ) return FAILED;
	//if ( pin >= 8 ) return FAILED;
#if 0
	if ( _getGpio( GPIO_FUNC_DIRECTION, port, pin ) == GPIO_DIR_IN ) return FAILED; /* read only */  
#endif

#if _GPIO_DEBUG_ >= 3
	printk("[%s():%d] (port=%d,pin=%d)=%d\n", __FUNCTION__, __LINE__, port, pin, data );
#endif
	_setGpio( GPIO_FUNC_DATA, port, pin, data );

#endif
	return SUCCESS;
}

#endif//CONFIG_RTK_VOIP_DRIVERS_PCM8671
