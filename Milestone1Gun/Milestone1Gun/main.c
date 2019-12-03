/*
 * Milestone1Gun.c
 *
 * Created: 11/19/2019 7:50:41 PM
 * Author : winso
 */

#include <avr/io.h>
#include "scheduler.h"
#include "io.c"
#include "timer.h"
#include "usart_ATmega1284.h"
#include "nokia5110.c"

unsigned char tempB = 0x00;
unsigned char tempA = 0x00;
unsigned int coolDownCount = 2;
unsigned int bulletCount = 6;
unsigned int gameStart = 1;
void sendToTarget(unsigned char data){
unsigned char payload = 0x00;
if(USART_HasReceived(0)){ //Message Received
	payload = USART_Receive(0);
	USART_Flush(0);
	commTranslate(payload);
}
	if(USART_IsSendReady(0)){
		USART_Send(data,0);
		while(USART_HasTransmitted(0) == 0){
			PORTB = 0xFF;
		}
	}
}

void gameOver(){
	//Display Score and Game Over Screen
	sendToTarget(0xFF);
	while(1);
}
void commTranslate(unsigned char data){
	if(data == "0x01"){
		//Hit a special target, add 3 bullets to bulletCount
		if((bulletCount + 3) > 6){
			bulletCount = 6;
		}
		else{
			bulletCount = bulletCount + 3;
		}
		return;
	}
	if(data == "0x02"){
		//Time is up
		gameOver();
	}
	if(data == "0x03"){
		//Start Game
		gameStart = 1;
	}
}
enum shootStates {shootStateSM_init, shootStateSM_wait, shootStateSM_shoot, shootStateSM_cooldown, shootStateSM_waitRelease};
int shootStateSM(int shootState){
unsigned char payload = 0x00;
if(USART_HasReceived(0)){ //Message Received
	payload = USART_Receive(0);
	USART_Flush(0);
	commTranslate(payload);
}
	switch(shootState){
		case shootStateSM_init:
			if(gameStart == 0){
				shootState = shootStateSM_init;
			}
			else{
				shootState = shootStateSM_wait;
			}
			break;
		case shootStateSM_wait:
			if(tempA){
				if(bulletCount > 0){
					bulletCount = bulletCount - 1;
					sendToTarget(bulletCount);
					shootState = shootStateSM_shoot;
				}
				if(bulletCount == 0){
					sendToTarget(bulletCount);
					gameOver();
				}
			}
			else{
				if(bulletCount == 0){
					gameOver();
				}
				sendToTarget(bulletCount);
				shootState = shootStateSM_wait;
			}
			break;
		case shootStateSM_shoot:
			tempB = 0x01;
			shootState = shootStateSM_cooldown;
			break;
		case shootStateSM_cooldown:
			if(coolDownCount  == 0){
				tempB = 0x00;
				coolDownCount = 2;
				shootState = shootStateSM_waitRelease;
			}
			else{
				tempB = 0x01;
				coolDownCount = coolDownCount - 1;
				shootState = shootStateSM_cooldown;
			}
			break;
		case shootStateSM_waitRelease:
			if(tempA == 0x01){
				shootState = shootStateSM_waitRelease;
			}
			else{
				shootState = shootStateSM_wait;
			}
			break;

		default:
			shootState = shootStateSM_init;
	}
	return shootState;
}

enum displayStates{displaySM_init, displaySM_update};

int displaySM(int displayState){
unsigned char payload = 0x00;
	if(USART_HasReceived(0)){ //Message Received
		payload = USART_Receive(0);
		USART_Flush(0);
		commTranslate(payload);
	}
	switch(displayState){
		case displaySM_init:
			if(gameStart == 0){
				//display game start screen
			}
			else{
				displayState = displaySM_update;
			}
			break;
		case displaySM_update:
			//display bullets
			displayState = displaySM_update;
			break;
	}
	return displayState;
}

//commStates are in charge of communication from target
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

int main(void)
{
	DDRB = 0xFF; PORTB = 0x00;
	//PORTC FOR Nokia 5110 LCD
	DDRA = 0x00; PORTA = 0xFF;
	initUSART(0);
	USART_Flush(0);
	static task task1;
	static task task2;
	static task task3;

	task *tasks[] = {&task1,&task2,&task3};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	task1.state = commStateSM_init;
	task1.period = 50;
	task1.elapsedTime = task1.period;
	task1.TickFct = &commStateSM;

	task2.state = shootStateSM_init;
	task2.period = 100;
	task2.elapsedTime = task2.period;
	task2.TickFct = &shootStateSM;

	task3.state = displaySM_init;
	task3.period = 50;
	task3.elapsedTime = task3.period;
	task3.TickFct = &displaySM;
	
	TimerSet(50);
	TimerOn();
	unsigned int i;
	//====End of Task Scheduler Setup====
	while (1)
	{
		tempA = PINA & 0x01;
		for( i = 0; i < numTasks; i++){
			if(tasks[i]->elapsedTime == tasks[i]->period){
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i] ->elapsedTime += 50;
			PORTB = tempB;
		}
	while(!TimerFlag);
	TimerFlag = 0;
	}
}
