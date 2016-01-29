#ifndef XML_H
#define XML_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ezxml.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>

#define SHM_INPUT 1    
#define FAKE_INPUT 2

#define COMMAND_LINE_OUTPUT 1  
#define WIRING_OUTPUT 2  

#define MAX_CHAR_FIELD_LENGTH 18

typedef struct appconfig_s {
	
	char debug;
	
	/*feature source config*/
	char feature_source;
	
	/*feature vect config*/
	int nb_channels;
	int window_width;
	int buffer_depth;
	char timeseries;
	char fft;
	char power_alpha;
	char power_beta;
	char power_gamma;
	
	/*Hardware status*/
	char eeg_hardware_required;
	
	/*exp related variables*/
	int training_set_size;
	double test_duration;
	double avg_kernel;
	
} appconfig_t;

appconfig_t *xml_initialize(char *filename);
appconfig_t *get_appconfig();
void set_appconfig(appconfig_t *config_obj);


#endif
