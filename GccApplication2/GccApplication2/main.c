/*
 * GccApplication2.c
 *
 * Created: 11/26/2019 8:37:33 PM
 * Author : winso
 */ 

#include <avr/io.h>


int main(void)
{
    /* Replace with your application code */
    while (1) 
    {
		DDRD = 0xFF; PORTD = 0x00;
		PORTD = 0xFF;
    }
}

