
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <pthread.h>

#include "gpio_wrapper.h"


#define	START_DEMO 0


void setup_gpios(void){

	  /*setup the wiring pi*/
	  if (wiringPiSetup () == -1)
			exit (1) ;
	  
	  /*define the pins functions*/
	  pinMode(START_DEMO, INPUT);
	
	
}


/**
 * void wait_for_coin_insertion(void)
 * @brief blocking call that waits for coin insertion
 */
void wait_for_start_demo(void)
{
	  printf("Waiting to start\n"); 
	  fflush(stdout);
	  
	  /*wait for start button to be pressed*/
	  while (digitalRead(START_DEMO)==HIGH)
		delay(50);
		
	  /*wait for start button to be released*/
	  while (digitalRead(START_DEMO)==LOW)
		delay(50);
	  
	  /*inform user*/
	  printf("Starting!\n");
}
