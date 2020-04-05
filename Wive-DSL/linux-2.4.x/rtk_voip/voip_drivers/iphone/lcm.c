//It's something wrong about AC increment by 1 in 4-bit interface.
//So some trick is included in Write_Data_to_RAM function.

#include <linux/kernel.h>
#include <linux/string.h>
#include "lcm.h"
#include "base_gpio.h"
#ifdef __kernel_used__
#include <linux/delay.h>	//udelay(),mdelay()
#endif
void LCM_Software_Init(void)
{
	unsigned int i;
	init_lcm_gpio(0);
	//delay 15 ms
	#ifdef __kernel_used__
	mdelay(15);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000000;i++);
	#endif
	write_lcm_IR(0x30,NOT_READ_FLAG,1);//Function_set
	//delay 4.1 ms
	#ifdef __kernel_used__
	mdelay(5);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<500000;i++);
	#endif
	write_lcm_IR(0x30,NOT_READ_FLAG,1);//Function_set
	//delay 100us
	#ifdef __kernel_used__
	udelay(100);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<40000;i++);
	#endif
	write_lcm_IR(0x30,NOT_READ_FLAG,1);//Function_set
	//delay 100us
	#ifdef __kernel_used__
	udelay(100);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<40000;i++);
	#endif
	Function_set(0,1);//set interface to 4-bit
	Function_set(0,0);//set number of lines and character font
	Display_on_off(0,0,0,0);//display off ,cursor off and no blink
	Clear_Display(0);//clear display
	Entry_mode_set(1,0);//entry mode set
	Display_on_off(1,0,0,0);//display on ,cursor off and no blink
	
	return;
}

//bit_interface: 0-> 4-bit data length, 1-> 8-bit data_length.	
void Clear_Display(unsigned char bit_interface)
{
	write_lcm_IR(0x01,READ_FLAG,bit_interface);
	return;
}

void Return_Home(unsigned char bit_interface)
{
	write_lcm_IR(0x02,READ_FLAG,bit_interface);
	return;
}

//address_shift: 0->decrement DD_RAM address by 1, 1->increment DD_RAM address by 1.
void Entry_mode_set(unsigned char address_shift,unsigned char bit_interface)
{
	if (address_shift > 1) {
		printk("Wrong LCM address decrement or increment\n");
		return;
	}		
	write_lcm_IR(0x04|(address_shift<<1),READ_FLAG,bit_interface);
	return;
}

//display: 0->display off, 1->display on.
//cursor: 0->cursor don't display, 1->cursor display.
//blink: 0->cursor don't blink, 1->cursor blink.
void Display_on_off(unsigned char display,unsigned char cursor,unsigned char blink,unsigned char bit_interface)
{
	if ((display > 1) || (cursor > 1) || (blink > 1)) {
		printk("Wrong input to Display_on_off\n");
		return;
	}	
	write_lcm_IR(0x08|(display<<2)|(cursor<<1)|blink,READ_FLAG,bit_interface);	
	return;
}

//data_length: 0->data sent or received in 4-bit length, 1->data sent or received in 8-bit length.
void Function_set(unsigned char data_length,unsigned char bit_interface)
{
	if (data_length > 1) {
		printk("Wrong data interface\n");
		return;
	}
	write_lcm_IR(0x28|(data_length<<4),READ_FLAG,bit_interface);	
	return;
}

void Set_CG_RAM_address(unsigned char CG_address,unsigned char bit_interface)
{
	write_lcm_IR(0x40|CG_address,READ_FLAG,bit_interface);
	return;
}

void Set_DD_RAM_address(unsigned char DD_address,unsigned char bit_interface)
{
	write_lcm_IR(0x80|DD_address,READ_FLAG,bit_interface);
	return;
}

void Write_Data_to_RAM(char character,unsigned char bit_interface)
{
	write_lcm_DR(character,READ_FLAG,bit_interface);
	Set_DD_RAM_address(((Read_BF_AC()&0x7F)-1),0);
	return;
}

void Write_Data_to_CGRAM(char character,unsigned char bit_interface)
{
	//printk("bef Read_BF_AC()=%x\n",Read_BF_AC());
	write_lcm_DR(character,READ_FLAG,bit_interface);
	
	//Set_CG_RAM_address((Read_BF_AC()-1),0);
	return;
}

unsigned char Read_BF_AC(void)
{
	return read_lcm_IR();
}

unsigned char Read_Data_from_RAM(void)
{
	return read_lcm_DR();
}

static unsigned char check_string_number(char *strings)
{
	int i=0;
	while (strings[i++] != NULL);
	return (i-1);
}	
//line: 1-> line 1, 2-> line2. 
void Write_string_to_LCM(char *strings,unsigned char start,unsigned char end,unsigned char line)
{
	int i=0;
	
	#if 0
	if (check_string_number(strings) > DISPLAY_CHARACTER)  {
		printk("The string is over the display\n");
		return;
	}
	#endif
	if (end < start) {
		printk("Wrong parameter input\n");
		return;
	}
		
	if (start == 0 && end == 0) {
		if (line == 1)
			Set_DD_RAM_address(0,0);
		else if (line == 2)
			Set_DD_RAM_address(0x40,0);
		else
			printk("Wrong LCM line number\n");
		while (strings[i] != NULL) 
			Write_Data_to_RAM(strings[i++],0);			
	} else {
		if (line == 1)
			Set_DD_RAM_address(start,0);
		else if (line == 2)
			Set_DD_RAM_address(0x40|start,0);
		else
			printk("Wrong LCM line number\n");
		while (strings[i] != NULL) {
			Write_Data_to_RAM(strings[i++],0);
			if (i == end+1)
				break;
		}		
	}		
	return;	
}

void make_character(unsigned char number_char, lcm_pixel_data_t *data)
{
	if (number_char < 0 || number_char > 6) {
		printk("Over CGRAM address\n");
		return;
	}
	
	Set_CG_RAM_address((0x40+8*number_char),0);
	Write_Data_to_CGRAM(data->row0,0);
	Set_CG_RAM_address((0x40+8*number_char+1),0);
	Write_Data_to_CGRAM(data->row1,0);
	Set_CG_RAM_address((0x40+8*number_char+2),0);
	Write_Data_to_CGRAM(data->row2,0);
	Set_CG_RAM_address((0x40+8*number_char+3),0);
	Write_Data_to_CGRAM(data->row3,0);
	Set_CG_RAM_address((0x40+8*number_char+4),0);
	Write_Data_to_CGRAM(data->row4,0);
	Set_CG_RAM_address((0x40+8*number_char+5),0);
	Write_Data_to_CGRAM(data->row5,0);
	Set_CG_RAM_address((0x40+8*number_char+6),0);
	Write_Data_to_CGRAM(data->row6,0);
	Set_CG_RAM_address((0x40+8*number_char+7),0);
	Write_Data_to_CGRAM(data->row7,0);	 

	return;
}	
				