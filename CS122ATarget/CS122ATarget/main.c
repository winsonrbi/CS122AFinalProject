/*
 * CS122ATarget.c
 *
 * Created: 11/7/2019 6:53:33 PM
 * Author : winso
 */ 

#include <avr/io.h>
#include "scheduler.h"
#include "io.c"
#include "timer.h"
#include "usart_ATmega1284.h"

//====START OF GLOBALS====
unsigned int currTarget = 0;
unsigned int timeLeft = 0;
unsigned int score = 0;
unsigned int wait = 0;
//====END OF GLOBALS=====

void sendToGun(unsigned char sendValue){
	while(USART_IsSendReady(0) == 0); //Wait till send is ready then send value to gun.
	USART_Send(sendValue,0);
}
unsigned int hitCheck(){
	//Used by PCINT Interrupt to check whether correct target was hit, compare Target to IR receiver
	//Returns true if correct target was hit
	
}
void randTarget(){
	//TODO: determines currTarget and then turns on the LED light so user knows which is the target
	
}
void GameOver(){
	//TODO: Send Game Over Signal to Gun
	timerState =  timerSM_init;
	LCDDisplayState = LCDDisplaySM_init;
	targetSelectState = targetSelectSM_init;
	
	//TODO: Display Game Over and Score
	return;
}
enum timerStates{timerSM_init, timerSM_Countdown, timerSM_GameOver}timerState;
int timerSM(int timerState){
	switch(timerState){
		case timerSM_init:
			timerState = timerSM_Countdown;
			timeLeft = 60;
			break;
			
		case timerSM_Countdown:
			if(timeLeft > 0){
				--timeLeft;
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
}

enum targetSelectStates {targetSelectSM_init, targetSelectSM_targetSelect, targetSelectSM_hitWait} targetSelectState;	
int targetSelectSM(int targetSelectState){
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
	
}

enum LCDDisplayStates{LCDDisplaySM_update, LCDDisplaySM_GameOver} LCDDisplayState; 
int LCDDisplaySM(int LCDDisplayState){
	switch(LCDDisplayState){
		case LCDDisplaySM_update:
			//TODO: Display on Screen the score and Time Left us itoa()
			break;
		default: 
			LCDDisplayState = LCDDisplaySM_update;			
	}
}

int main(void)
{	
	DDRD = 0xFF; PORTD = 0x00;
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
	task1.TickFct  &timerSM;
	
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
	unsigned short i;
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
    }
}

