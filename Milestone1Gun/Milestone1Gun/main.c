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

unsigned char tempB = 0x00;
unsigned char tempD = 0x00;
unsigned int coolDownCount = 2;
enum shootStates {shootStateSM_init, shootStateSM_wait, shootStateSM_shoot, shootStateSM_cooldown, shootStateSM_waitRelease};
int shootStateSM(int shootState){
	switch(shootState){
		case shootStateSM_init:
			shootState = shootStateSM_wait;
			break;
		case shootStateSM_wait:
			if(tempD){
				shootState = shootStateSM_shoot;
			}
			else{
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
			if(tempD == 0x01){
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
int main(void)
{
    /* Replace with your application code */
	DDRB = 0xFF; PORTB = 0x00;
	DDRD = 0x00; PORTD = 0xFF;
	static task task1;
		
	task *tasks[] = {&task1,};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
		
	task1.state = shootStateSM_init;
	task1.period = 100;
	task1.elapsedTime = task1.period;
	task1.TickFct = &shootStateSM;	
	TimerSet(100);
	TimerOn();
	unsigned int i;
	//====End of Task Scheduler Setup====
	while (1)	
	{
		tempD = PIND & 0x01;
		for( i = 0; i < numTasks; i++){
			if(tasks[i]->elapsedTime == tasks[i]->period){
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i] ->elapsedTime += 100;
		}	
		PORTB = tempB;
	while(!TimerFlag);
	TimerFlag = 0;
	}
}

