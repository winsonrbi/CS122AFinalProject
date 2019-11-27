/*
 * BluetoothTest.c
 *
 * Created: 11/12/2019 7:06:19 PM
 * Author : winso
 */

#include <avr/io.h>
#include <string.h>
#include "timer.h"
#include "io.c""
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
	DDRC = 0XFF; PORTC = 0x00;
	LCD_init();
	LCD_DisplayString(1,"Test");
	TimerSet(1000);
	TimerOn();
	initUSART(0);
	unsigned int sent = 0;
    /* Replace with your application code */
	char char_array[] = { "AT\r\n" };
	char command1[] = {"AT+ROLE=1\r\n"};
	char command2[] = {"AT+CMODE=0\r\n"};
	char command3[] = {"AT+BIND=98d3,21,fc7fdf\r\n"};
    while (1)
    {
		if(sent == 0){
			sent = 1;
			sendString(char_array);
		}
		while(USART_HasReceived(0)){
			LCD_DisplayString(1,USART_Receive(0));
		}
		while(!TimerFlag);
		TimerFlag=0;
		//LCD_DisplayString(1,"AT");
		/*
		sendString(command1);
		while(!TimerFlag);
		TimerFlag=0;
		LCD_DisplayString(1,"AT+ROLE");
		sendString(command2);
		while(!TimerFlag);
		TimerFlag=0;
		LCD_DisplayString(1,"AT+CMODE");
		sendString(command3);
		while(!TimerFlag);
		TimerFlag=0;
		LCD_DisplayString(1,"AT+BIND");
		*/
		while(1);
		
    }
}
