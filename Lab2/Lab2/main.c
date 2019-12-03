/*	Author: winson Bi
 *  Partner(s) Name: Dylan Han
 *	Lab Section:
 *	Assignment: Lab 2  Exercise 1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include "timer.h"
#include "usart_ATmega1284.h"

/*RXD0 is mapped to PD0
 *TXD0 is mapped to PD1
 *RXD1 is mapped to PD2
 *TXD1 is mapped to PD3
 */
unsigned char tempA = 0x00;
unsigned char tempVal = 0x00;
enum slaveStates{slaveInitState} slaveState;
void slaveSM(){
	switch(slaveState){//Transitions
		case slaveInitState:
			tempVal = USART_Receive(0);
			if(tempVal == 0x00){
				tempA = 0x00;
				USART_Flush(0);
			}
			if(tempVal == 0x01){
				tempA = 0x01;
				USART_Flush(0);
			}
			slaveState = slaveInitState;
			break;

		default:
			slaveState = slaveInitState;
			break;
	}
}

enum leaderStates{leaderInitState,leaderLEDOn,leaderTransmissionWait,leaderLEDOff, leaderTransmissionWaitOff} leaderState;

void leaderSM(){
	switch(leaderState){//Transitions
		case leaderInitState:
			leaderState = leaderLEDOn;
			break;

		case leaderLEDOn:
			if(USART_IsSendReady(1) != 0){
				tempA = 0x01;
				USART_Send(0x01,1);
				if(USART_HasTransmitted(1) != 0){
					USART_Flush(1);
					leaderState = leaderLEDOff;
				}
				else{
					leaderState = leaderTransmissionWait;
				}
			}
			else{
				leaderState = leaderLEDOn;
			}

			break;

		case leaderTransmissionWait:
			if(USART_HasTransmitted(1) != 0){
				USART_Flush(1);
				leaderState = leaderLEDOff;
			}
			else{
				leaderState = leaderTransmissionWait;
			}
			break;

		case leaderLEDOff:
			if(USART_IsSendReady(1) != 0){
				tempA = 0x00;
				USART_Send(0x00,1);
				if(USART_HasTransmitted(1) != 0){
					USART_Flush(1);
					leaderState = leaderLEDOn;
				}
				else{
					leaderState= leaderTransmissionWaitOff;
				}
			}
			else{
				leaderState = leaderLEDOff;
			}
			break;
		case leaderTransmissionWaitOff:
			if(USART_HasTransmitted(1) != 0){
				USART_Flush(1);
				leaderState = leaderLEDOn;
			}
			else{
				leaderState = leaderTransmissionWaitOff;
			}
			break;

		default:
			leaderState = leaderInitState;
			break;
	}
}

int main(void) {
	//1 are outputs and 0 are in inputs
    /* Insert DDR and PORT initializations */
	DDRA = 0xFF; PORTA = 0x00; //OUTPUTS
    /* Insert your solution below */
	//Receive on USART0
	initUSART(0);
	USART_Flush(0);
	//Transmit on USART1
	initUSART(1);
	USART_Flush(1);

	TimerOn();
	TimerSet(100);
    while (1) {
	//TODO: ADD desired SM to run
	leaderSM();
	//slaveSM();
    PORTA = tempA;

	while(!TimerFlag){}
	TimerFlag = 0;
    }
    return 1;
}
