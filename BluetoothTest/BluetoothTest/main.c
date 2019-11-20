/*
 * BluetoothTest.c
 *
 * Created: 11/12/2019 7:06:19 PM
 * Author : winso
 */ 

#include <avr/io.h>
#include <string.h>
#include "timer.h"
#include "usart_ATmega1284.h"
void sendString(char* val){
	if(USART_IsSendReady(0)){
		for( int i=0 ; i < strlen(val); i++){
			USART_Send(val[i],0);
		}
	}
}
unsigned char tempB = 0x00;
int main(void)
{	
	DDRD = 0xFF; PORTD = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	initUSART(0);
    /* Replace with your application code */
	char char_array[] = { "AT+NAME=GunBluetooth\r\n" }; 
    while (1) 
    {
	/*	
		if( USART_HasReceived(0)){
			tempB = 0xFF;
			sendString(char_array);
			USART_Flush(0);
		*/
	sendString(char_array);
		PORTB = tempB;
    }
}

