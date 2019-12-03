/*
 * NokiaLCD.c
 *
 * Created: 12/2/2019 11:00:01 PM
 * Author : winso
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include "nokia5110.c"

int main(void)
{
	DDRC = 0xFF; PORTC = 0x00;
	nokia_lcd_init();
	nokia_lcd_clear();
	nokia_lcd_write_string("Test",2);
	nokia_lcd_render();
    while (1) 
    {
    }
}

