/**
 * @file shm_rd_buf.c
 * @author Frederic Simard, Atlants Embedded (frederic.simard.1@outlook.com)
 * @brief This file implements the shared memory feature input system.
 *        This service drive all the input stream, opening buffers on request
 * 		  and providing a blocking call to wait for the news sample
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "feature_structure.h"
#include "fake_feature_generator.h"
#include "feature_input.h"

#define SAMPLE_LENGTH 220

extern double randn();

/**
 * int fake_feat_gen_init(void *param)
 * @brief init fake feature generator memory
 * @param reference to the feature input
 * @return EXIT_FAILURE/EXIT_SUCCESS
 */
int fake_feat_gen_init(void *param){
	
	feature_input_t* pfeature_input = param;
	pfeature_input->shm_buf = malloc(pfeature_input->page_size);
	/*nothing to do*/
	return EXIT_SUCCESS;
}

/**
 * int fake_feat_gen_request()
 * @brief request a new sample (do nothing)
 * @param reference to the feature input
 * @return EXIT_FAILURE/EXIT_SUCCESS
 */
int fake_feat_gen_request(void *param __attribute__((unused))){
	
	return EXIT_SUCCESS;
}

/**
 * int fake_feat_gen_wait_for_request_completed(void *param)
 * @brief return a random sample, and sleep a bit to simulate a delay
 * @param reference to the feature input
 * @return EXIT_FAILURE/EXIT_SUCCESS
 */
int fake_feat_gen_wait_for_request_completed(void *param __attribute__((unused))){
	
	/*wait to simulate a delay*/
	usleep(500000);
	
	return EXIT_SUCCESS;
}


/**
 * double* fake_feat_gen_feature_array_ref(void *param)
 * @brief get a handle on the current frame info
 * @param reference to the feature input
 * @return pointer to frame info
 */
frame_info_t* fake_feat_gen_frame_info_ref(void *param){
	
	feature_input_t* pfeature_input = param;
	frame_info_t* frame_info = (frame_info_t*)pfeature_input->shm_buf;
	
	/*randomly generate 10% of blinks*/
	if((double)rand()/(double)RAND_MAX < 0.1){
		frame_info->eye_blink_detected = 0x01;
	}else{
		frame_info->eye_blink_detected = 0x00;
	}
	
	return frame_info;
}


/**
 * double* fake_feat_gen_feature_array_ref(void *param)
 * @brief get a handle on the current feature array
 * @param reference to the feature input
 * @return pointer to feature array
 */
double* fake_feat_gen_feature_array_ref(void *param){
	
	int i=0;
	feature_input_t* pfeature_input = param;
	double* feature_array = (double*)&(pfeature_input->shm_buf[sizeof(frame_info_t)]);
	
	/*file the buffer with random values*/
	for(i=0;i<pfeature_input->nb_features;i++){
		feature_array[i] = (double)rand()/(double)RAND_MAX;
	}

	return feature_array;
}


/**
 * int fake_feat_gen_cleanup(void *param)
 * @brief cleanup fake feature generator memory
 * @param reference to the feature input
 * @return EXIT_FAILURE/EXIT_SUCCESS
 */
int fake_feat_gen_cleanup(void *param){
	
	feature_input_t* pfeature_input = param;
	
	free(pfeature_input->shm_buf);
	
	return EXIT_SUCCESS;
}



