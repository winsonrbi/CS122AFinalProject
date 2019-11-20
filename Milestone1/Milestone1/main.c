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
unsigned int currTarget = 0;
unsigned int timeLeft = 0;
unsigned int score = 0;
unsigned int wait = 0;	
volatile unsigned char  previousPins = 0;
volatile unsigned char pins = 0;
//====END OF GLOBALS=====

/*
ISR(PCINT1_vect){
	previousPins = pins;
	pins = (PINB & 0x03);
	if(pins == 0x01){
		score++;
	}
}
*/
	
void DisplayString( unsigned char column, const unsigned char* string) {
	unsigned char c = column;
	while(*string) {
		LCD_Cursor(c++);
		LCD_WriteData(*string++);
	}
}
enum timerStates{timerSM_init, timerSM_Countdown, timerSM_GameOver};
int timerSM(int timerState){
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
	switch(targetSelectState){
		case targetSelectSM_init:
		score = 0;
		targetSelectState = targetSelectSM_targetSelect;
		break;
		
		case targetSelectSM_targetSelect:
		//randTarget();
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
	switch(LCDDisplayState){
		case LCDDisplaySM_update:
		//TODO: Display on Screen the score and Time Left us itoa()
		sprintf(combineString,"Time: %d        Score: %d",timeLeft,score);
		DisplayString(1,combineString);		
		LCDDisplayState = LCDDisplaySM_update;
		
		break;
		
		default:
		LCDDisplayState = LCDDisplaySM_update;
	}
	return LCDDisplayState;
}

void sendToGun(unsigned char sendValue){
	while(USART_IsSendReady(0) == 0); //Wait till send is ready then send value to gun.
	USART_Send(sendValue,0);
}
unsigned int hitCheck(){
	//Used by PCINT Interrupt to check whether correct target was hit, compare Target to IR receiver
	//Returns true if correct target was hit
	return 1;
	
}
void randTarget(){
	randomNumber = rand()%3;
	ammoBack = rand()% 3;
	if(randomNumber == 0){
		PORTA = 0x01;
	}
	else if (randomNumber == 1){
		PORTA = 0x02;
	}
	else{
		PORTA = 0x04;
	}
	return;
}
void GameOver(){
	//TODO: Send Game Over Signal to Gun
	//task1.state =  timerSM_init;
	//task2.state = targetSelectSM_init;
	//task3.state = LCDDisplaySM_update;
	//TODO: Display Game Over and Score
	return;
}

int main(void)
{	
	
	(PCMSK1) |= (1<< PCINT8);
	(PCICR) |= (1<< PCIE1);
	sei();
	DDRD = 0xFF; PORTD = 0x00;
	DDRC = 0XFF; PORTC = 0x00;
	DDRB = 0x00; PORTB = 0xFF;
	DDRA = 0xFF; PORTA = 0x00;
	LCD_init();
	initUSART(0);
	//====Task Scheduler Setup====
	static task task1;
	static task task2;
	static task task3;
	
	task *tasks[] = {&task1, &task2, &task3};
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
	
	TimerSet(200);
	TimerOn();
	unsigned int i;
	//====End of Task Scheduler Setup====
    while (1) 
    {
		for( i = 0; i < numTasks; i++){
			if(tasks[i]->elapsedTime == tasks[i]->period){
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i] ->elapsedTime += 200;
		}
		PORTB = PORTA;
		while(!TimerFlag);
		TimerFlag = 0;		
    }
}

