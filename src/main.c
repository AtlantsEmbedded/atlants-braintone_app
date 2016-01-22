/**
 * @file main.c
 * @author Frederic Simard (fred.simard@atlantsembedded.com)
 * @date 6/11/2015
 * @brief This program implements a simple neurofeedback loop between the occipital
 * alpha band power and an audio pitch, emitted by a piezo buzzer.
 * 
 * It starts with a short training period to set the frame of reference and then
 * the test runs for a duration that is set in the xml file.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include <ctype.h>
#include <math.h>
#include <signal.h>

#include <wiringPi.h>
#include <softTone.h>
#include "app_signal.h"
#include "buzzer_lib.h"
#include "feature_processing.h"
#include "ipc_status_comm.h"
#include "feature_input.h"
#include "xml.h"

/*defines the frequency scale*/
#define NB_STEPS 100
#define NB_PLAYERS 1
#define PLAYER_1 0

/*function prototypes*/
static inline void print_banner();
inline char *which_config(int argc, char **argv);
char task_running = 0x01;

/*default xml file path/name*/
#define CONFIG_NAME "config/braintone_app_config.xml"

/**
 * main(int argc, char *argv[])
 * @brief main program function
 * @param argc
 * @param argv - optional alternative xml path/name
 * @return 0 for success, -1 for error
 */
int main(int argc, char *argv[])
{	
	/*freq index*/
	double cpu_time_used;
	double running_avg = 0;
	double adjusted_sample = 0;
	clock_t start, end;
	feature_input_t feature_input[NB_PLAYERS];
	ipc_comm_t ipc_comm[NB_PLAYERS];
	feat_proc_t feature_proc[NB_PLAYERS];
	
	/*configuration structure*/
	appconfig_t* app_config;
	
	/*Set up ctrl c signal handler*/
	(void)signal(SIGINT, ctrl_c_handler);

	/*Show program banner on stdout*/
	print_banner();
	
	/*read the xml*/
	app_config = xml_initialize(which_config(argc, argv));
	
	/*setup the buzzer*/
	setup_buzzer_lib();
	
	/*configure the feature input*/
	feature_input[PLAYER_1].shm_key=7804;
	feature_input[PLAYER_1].sem_key=1234;
	init_feature_input(app_config->feature_source, &(feature_input[PLAYER_1]));
	
	/*configure the inter-process communication channel*/
	ipc_comm[PLAYER_1].sem_key=1234;
	ipc_comm_init(&(ipc_comm[PLAYER_1]));
	
	/*set beep mode*/
	set_beep_mode(50, 0, 500);

	/*if required, wait for eeg hardware to be present*/
	if(app_config->eeg_hardware_required){
		if(!ipc_wait_for_harware(&(ipc_comm[PLAYER_1]))){
			exit(0);
		}
	}
	
	/*stop beep mode*/
	turn_off_beeper();
	
	/*initialize feature processing*/
	feature_proc[PLAYER_1].nb_train_samples = app_config->training_set_size;
	feature_proc[PLAYER_1].feature_input = &(feature_input[PLAYER_1]);
	init_feat_processing(&(feature_proc[PLAYER_1]));
		
	/*start training*/	
	train_feat_processing(&(feature_proc[PLAYER_1]));
	
	/*little pause between training and testing*/	
	printf("About to start task\n");
	fflush(stdout);	
	sleep(3);	
		
	start = clock();
		
	/*run the test*/
	while(task_running){
	
		/*get a normalized sample*/
		get_normalized_sample(&(feature_proc[PLAYER_1]));
		
		/*adjust the sample value to the pitch scale*/
		adjusted_sample = ((float)feature_proc[PLAYER_1].sample*100/3.5);
		/*compute the running average, using the defined kernel*/
		running_avg += (adjusted_sample-running_avg)/app_config->avg_kernel;
		
		/*update buzzer state*/
		set_buzzer_state(running_avg);
		
		/*show sample value on console*/
		printf("sample value: %i\n",(int)running_avg);
		
		/*get current time*/
		end = clock();
		cpu_time_used = ((double) (end - start)) / (double)CLOCKS_PER_SEC * 100;
		
		/*check if one of the stop conditions is met*/
		if(app_config->test_duration < cpu_time_used){
			task_running = 0x00;
		}
		
	}
	
	printf("Finished\n");
	
	/*clean up app*/	
	ipc_comm_cleanup(&(ipc_comm[PLAYER_1]));
	clean_up_feat_processing(&(feature_proc[PLAYER_1]));
	
	exit(0);
}


/**
 * print_banner()
 * @brief Prints app banner
 */
static inline void print_banner()
{
	printf("\nBrain Tone - EEG audio Neurofeedback\n\n");
	printf("Frederic Simard (fred.simard@atlantsembedded.com)\n");
	printf("------------------------------------------\n");
}



/**
 * which_config(int argc, char **argv)
 * @brief return which config to use
 * @param argc
 * @param argv
 * @return string of config
 */
inline char *which_config(int argc, char **argv)
{

	if (argc == 2) {
		return argv[1];
	} else {
		return CONFIG_NAME;
	}
}

