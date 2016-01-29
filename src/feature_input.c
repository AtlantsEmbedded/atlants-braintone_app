/**
 * @file feature_input.c
 * @author Frederic Simard (fred.simard@atlantsembedded.com)
 * @date Jan 2016
 * @brief Sets the virtual interface to read features from.
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "feature_input.h"
#include "fake_feature_generator.h"
#include "shm_rd_buf.h"
#include "xml.h"

/**
 * int init_feature_input(char input_type)
 * @brief Setup function pointers for the data input
 * @param input_type, string identifying the type of input to init
 * @return EXIT_FAILURE for unknown type, EXIT_SUCCESS for known/success
 */
int init_feature_input(char input_type, feature_input_t* feature_input){

	/*default values*/
	_INIT_FEAT_INPUT_FC = NULL;
	_REQUEST_FEAT_FC = NULL;
	_WAIT_FEAT_FC = NULL;
	_TERMINATE_FEAT_INPUT_FC = NULL;

	/*shared memory interface*/
	if(input_type == SHM_INPUT) {
		
		printf("Input source: SHM\n");
		_INIT_FEAT_INPUT_FC = &shm_rd_init;
		_REQUEST_FEAT_FC = &shm_rd_request;
		_WAIT_FEAT_FC = &shm_rd_wait_for_request_completed;
		_GET_FRAME_INFO_FC = &shm_get_frame_info_ref;
		_GET_FVECT_INFO_FC = &shm_get_feature_array_ref;
		_TERMINATE_FEAT_INPUT_FC = &shm_rd_cleanup;
	}
	/*fake input interface*/
	else if(input_type == FAKE_INPUT){
		printf("Input source: FAKE\n");
		_INIT_FEAT_INPUT_FC = &fake_feat_gen_init;
		_REQUEST_FEAT_FC = &fake_feat_gen_request;
		_WAIT_FEAT_FC = &fake_feat_gen_wait_for_request_completed;
		_GET_FRAME_INFO_FC = &fake_feat_gen_frame_info_ref;
		_GET_FVECT_INFO_FC = &fake_feat_gen_feature_array_ref;
		_TERMINATE_FEAT_INPUT_FC = &fake_feat_gen_cleanup;
	}
	else{
		fprintf(stderr, "Unknown input type\n");
		return EXIT_FAILURE;
	}
	return INIT_FEAT_INPUT_FC(feature_input);
	
}

