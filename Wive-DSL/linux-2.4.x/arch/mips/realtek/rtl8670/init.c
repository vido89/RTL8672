
#include <asm/io.h>
#include <linux/irq.h>
#include <asm/reboot.h>
#include <linux/config.h>
#include "gpio.h"

void lexra_setup()
{
	_machine_restart = 0xbfc00000;
	return;
}

