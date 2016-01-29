
#ifndef FEATURE_INPUT_H
#define FEATURE_INPUT_H

#include "feature_structure.h"

#define INIT_FEAT_INPUT_FC(param) \
		_INIT_FEAT_INPUT_FC(param)
		
#define REQUEST_FEAT_FC(param) \
		_REQUEST_FEAT_FC(param)
		
#define WAIT_FEAT_FC(param) \
		_WAIT_FEAT_FC(param)
		
#define GET_FRAME_INFO_FC(param) \
		_GET_FRAME_INFO_FC(param)
		
#define GET_FVECT_INFO_FC(param) \
		_GET_FVECT_INFO_FC(param)
		
#define TERMINATE_FEAT_INPUT_FC() \
		_TERMINATE_FEAT_INPUT_FC(NULL)
		
typedef int (*functionPtr_t) (void *);
functionPtr_t _INIT_FEAT_INPUT_FC;
functionPtr_t _REQUEST_FEAT_FC;
functionPtr_t _WAIT_FEAT_FC;
functionPtr_t _TERMINATE_FEAT_INPUT_FC;

typedef frame_info_t* (*get_frame_ptr_t) (void *);
typedef double* (*get_fvect_ptr_t) (void *);

get_frame_ptr_t _GET_FRAME_INFO_FC;
get_fvect_ptr_t _GET_FVECT_INFO_FC;

//frame_info_t* shm_get_frame_info_ref(void *param);
//double* shm_get_feature_array_ref(void *param);


typedef struct feature_input_s{
	
	/*options to be set for initialization*/
	int shm_key;
	int sem_key;
	
	/*filled during initialization*/
	int shmid; /*id of the shared memory array*/
	char* shm_buf; /*pointer to the beginning of the shared buffer*/
	int semid; /*id of semaphore set*/
	struct sembuf *sops; /* pointer to operations to perform */
	
	int current_page; /*identification of the current page*/
	
	int nb_features; /*number of single features*/
	int page_size; /*size of a single page*/
	int buffer_depth; /*nomber of page in the buffer*/
	
}feature_input_t;

int init_feature_input(char input_type, feature_input_t* feature_input);


#endif
