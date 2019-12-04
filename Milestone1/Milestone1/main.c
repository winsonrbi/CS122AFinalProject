/*
 * CS122ATarget.c
 *
 * Created: 11/7/2019 6:53:33 PM
 * Author : winso
 */

#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"
#include "io.c"
#include "timer.h"
#include "usart_ATmega1284.h"


unsigned int testCount = 0;
char testCountString[2];

//====START OF GLOBALS====
char combineString [8];
unsigned int randomNumber = 0;
unsigned int ammoBack = 0;
unsigned char currTarget = 0;
unsigned int timeLeft = 0;
unsigned int pinUp = 0;
unsigned int score = 0;
unsigned int wait = 0;
volatile unsigned char  previousPins = 0;
volatile unsigned char pins = 0;
unsigned int bullets = 6;
//====END OF GLOBALS=====
void commTranslate(unsigned char data){
	//LCD_DisplayString(1,data);
	if(data == 0xFF){
		GameOver();
	}
	else{
		bullets = data;
		if(bullets == 0){
			GameOver();
		}
	}
}
void hitCheck(unsigned char pins){
	//Used by PCINT Interrupt to check whether correct target was hit, compare Target to IR receiver
	//Returns true if correct target was hit
	//TODO: Fix if statement if not correct
	if(pinUp == 1){
		pinUp = 0;
		return;
	}
	if(pins == currTarget){
		if(ammoBack == 0){
			sendToGun(0x01);
			score++;
		}
		if(pinUp == 0){
			currTarget = 0xFF;
			PORTA = 0x00;
			PORTD = PORTD & 0xE3;
			pinUp = 1;
			score++;
		}		
		return;
	}
	else{
		return;
	}
}
ISR(PCINT1_vect){
	pins = (PINB & 0x07);
	hitCheck(pins);
}


void DisplayString( unsigned char column, const unsigned char* string) {
	unsigned char c = column;
	while(*string) {
		LCD_Cursor(c++);
		LCD_WriteData(*string++);
	}
}
enum timerStates{timerSM_init, timerSM_Countdown, timerSM_GameOver};
int timerSM(int timerState){
	unsigned char payload = 0x00;
	if(USART_HasReceived(0)){ //Message Received
		payload = USART_Receive(0);
		USART_Flush(0);
		commTranslate(payload);
		
	}
	switch(timerState){
		case timerSM_init:
		timeLeft = 60;
		timerState = timerSM_Countdown;
		break;

		case timerSM_Countdown:
		if(timeLeft > 0){
			timeLeft = timeLeft - 1;
			timerState = timerSM_Countdown;
		}
		else{
			timerState = timerSM_GameOver;
			GameOver();
		}
		break;

		case timerSM_GameOver:
		timerState = timerSM_init;
		break;
	}
	return timerState;
}

enum targetSelectStates {targetSelectSM_init, targetSelectSM_targetSelect, targetSelectSM_hitWait};
int targetSelectSM(int targetSelectState){
	unsigned char payload = 0x00;
	if(USART_HasReceived(0)){ //Message Received
		payload = USART_Receive(0);
		commTranslate(payload);
	}
	switch(targetSelectState){
		case targetSelectSM_init:
		score = 0;
		targetSelectState = targetSelectSM_targetSelect;
		break;

		case targetSelectSM_targetSelect:
		randTarget();
		targetSelectState = targetSelectSM_hitWait;
		break;

		case targetSelectSM_hitWait:
		++wait;
		if(wait == 10){ //2 Seconds is over, time to switch targets
			wait = 0;
			targetSelectState = targetSelectSM_targetSelect;
		}
		else{
			targetSelectState = targetSelectSM_hitWait;
		}
		break;
	}
	return targetSelectState;
}

enum LCDDisplayStates{LCDDisplaySM_update, LCDDisplaySM_GameOver};
int LCDDisplaySM(int LCDDisplayState){
	unsigned char payload = 0x00;
	if(USART_HasReceived(0)){ //Message Received
		payload = USART_Receive(0);
		USART_Flush(0);
		commTranslate(payload);
	}
	switch(LCDDisplayState){
		case LCDDisplaySM_update:
		sprintf(combineString,"Time: %02d        Score: %d Ammo:%d ",timeLeft,score,bullets);
		DisplayString(1,combineString);
		LCDDisplayState = LCDDisplaySM_update;
		
		break;

		default:
		LCDDisplayState = LCDDisplaySM_update;
	}
	return LCDDisplayState;
}

void sendToGun(unsigned char sendValue){
unsigned char payload = 0x00;
if(USART_HasReceived(0)){ //Message Received
	payload = USART_Receive(0);
	USART_Flush(0);
	commTranslate(payload);
}
	while(USART_IsSendReady(0) == 0); //Wait till send is ready then send value to gun.
	USART_Send(sendValue,0);
}

void randTarget(){
	//TODO FINISH LED COLORS AND OUTPUT
	randomNumber = rand()%3;
	ammoBack = rand()% 3;
	if(randomNumber == 0){
		if(ammoBack == 0){
			PORTA = 0x00;
			PORTA = 0X07;
			PORTD = PORTD & 0xE3;
		}
		else{
			//Green Light
			PORTA = 0x00;
			PORTA = 0x02;
			PORTD = PORTD & 0xE3;
		}
		currTarget = 0x01;
	}
	else if (randomNumber == 1){
		if(ammoBack == 0){
			//White Light
			PORTA = 0x00;
			PORTA = 0x38;
			PORTD = PORTD & 0xE3;
		}
		else{
			//Green Light
			PORTA = 0x00;
			PORTA = 0x10;
			PORTD = PORTD & 0xE3;
		}		
		currTarget = 0x02;
	}
	else{
		if(ammoBack == 0){
			//White Light
			PORTD = PORTD & 0xE3;
			PORTD = PORTD | 0x01C;
			PORTA = 0x00;
		}
		else{
			//Green Light
			PORTD = PORTD & 0xE3;
			PORTD = PORTD | 0x08;
			PORTA = 0x00;
		}
		currTarget = 0x04;
	}
	return;
}
enum commStates{commStateSM_init, commStateSM_loop};
int commStateSM(int commState){
	unsigned char payload = 0x00;
	switch(commState){
		case commStateSM_init:
		commState = commStateSM_loop;
		break;
		
		case commStateSM_loop:
		if(USART_HasReceived(0)){ //Message Received
			payload = USART_Receive(0);
			USART_Flush(0);
			commTranslate(payload);
		}
		commState = commStateSM_loop;
		break;
	}
	return commState;
}

void GameOver(){
	char gameOverString[64];
	sendToGun(0x02);
	sprintf(gameOverString,"Game Over,      Final Score: %d",score);
	LCD_DisplayString(1, gameOverString);
	while(1);
}


int main(void)
{

	(PCMSK1) |= (1<< PCINT8);
	(PCMSK1) |= (1<< PCINT9);
	(PCMSK1) |= (1<< PCINT10);
	(PCICR) |= (1<< PCIE1);
	sei();
	DDRD = 0xFF; PORTD = 0x00;
	DDRC = 0XFF; PORTC = 0x00;
	DDRB = 0x00; PORTB = 0xFF;
	DDRA = 0xFF; PORTA = 0x00;
	LCD_init();
	initUSART(0);
	USART_Flush(0);
	//====Task Scheduler Setup====
	static task task1;
	static task task2;
	static task task3;
	static task task4;

	task *tasks[] = {&task1,&task2,&task3,&task4};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	task1.state = timerSM_init;
	task1.period = 1000;
	task1.elapsedTime = task1.period;
	task1.TickFct = &timerSM;

	task2.state = targetSelectSM_init;
	task2.period = 200;
	task2.elapsedTime = task2.period;
	task2.TickFct = &targetSelectSM;

	task3.state = LCDDisplaySM_update;
	task3.period = 200;
	task3.elapsedTime = task3.period;
	task3.TickFct = &LCDDisplaySM;

	task4.state = commStateSM_init;
	task4.period = 50;
	task4.elapsedTime = task4.period;
	task4.TickFct = &commStateSM;
	
	TimerSet(50);
	TimerOn();
	unsigned int i;
	//====End of Task Scheduler Setup====
	//Send start game signal to gun
	sendToGun(0x03);
    while (1)
    {	
		/*
		while(USART_HasReceived(0)){
			LCD_DisplayString(1,USART_Receive(0));
			USART_Flush(0);
		}
		*/
		for( i = 0; i < numTasks; i++){
			if(tasks[i]->elapsedTime == tasks[i]->period){
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i] ->elapsedTime += 50;
		}
		while(!TimerFlag);
		TimerFlag = 0;
    }
}
