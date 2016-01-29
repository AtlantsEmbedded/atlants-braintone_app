/**
 * @file shm_rd_buf.c
 * @author Frederic Simard, Atlants Embedded (frederic.simard.1@outlook.com)
 * @brief This file implements the shared memory feature input system.
 *        This service drive all the input stream, opening buffers on request
 * 		  and providing a blocking call to wait for the news sample
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "feature_structure.h"
#include "feature_input.h"
#include "shm_rd_buf.h"

/**
 * int shm_rd_init(void *param)
 * @brief Setups the shared memory input (memory and semaphores linkage)
 *        this function also post the write ready semaphore to begin data streaming
 * @param param, unused
 * @return EXIT_FAILURE for unknown type, EXIT_SUCCESS for known/success
 */
int shm_rd_init(void *param){
	
	int i;
	
	feature_input_t* pfeature_input = param;
	
    /*
     * initialise the shared memory array
     */
    if ((pfeature_input->shmid = shmget(pfeature_input->shm_key, pfeature_input->buffer_depth*pfeature_input->page_size, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        return EXIT_FAILURE;
    }
		
    /*
     * Now we attach it to our data space.
     */
    if ((pfeature_input->shm_buf = shmat(pfeature_input->shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        return EXIT_FAILURE;
    }
    
    /*
     * Access the semaphore array.
     */
	if ((pfeature_input->semid = semget(pfeature_input->sem_key, NB_SEM, IPC_CREAT | 0666)) == -1) {
		perror("semget failed\n");
		return EXIT_FAILURE;
    } 
	
	/*allocate the memory for the pointer to semaphore operations*/
	pfeature_input->sops = (struct sembuf *) malloc(sizeof(struct sembuf));
	
	/*set as if the current page was the last, such that the next page read will
	  be the first one*/
	pfeature_input->current_page = pfeature_input->buffer_depth;

	/*set all semaphores to 0*/
	for(i=0;i<4;i++){
		semctl(pfeature_input->semid, i, SETVAL, 0);
	}

	return EXIT_SUCCESS;
}


/**
 * int shm_rd_request(void *param)
 * @brief Open the buffers to catch a new sample
 * @param param, reference to the feature input struct
 * @return EXIT_FAILURE for unknown type, EXIT_SUCCESS for known/success
 */
int shm_rd_request(void *param){
	
	feature_input_t* pfeature_input = param;
	
	/*open the buffer string*/
	/*preprocessing opened*/
	pfeature_input->sops->sem_num = PREPROC_IN_READY;
	pfeature_input->sops->sem_op = 1; 
	pfeature_input->sops->sem_flg = IPC_NOWAIT;
	semop(pfeature_input->semid, pfeature_input->sops, 1);
	
	/*application opened*/
	pfeature_input->sops->sem_num = APP_IN_READY;
	pfeature_input->sops->sem_op = 1; 
	pfeature_input->sops->sem_flg = IPC_NOWAIT;
	semop(pfeature_input->semid, pfeature_input->sops, 1);
	
	return EXIT_SUCCESS;
}


/**
 * int shm_rd_request(void *param)
 * @brief Blocking call, until a sample has arrived
 * @param param, reference to the feature input struct
 * @return EXIT_FAILURE for unknown type, EXIT_SUCCESS for known/success
 */
int shm_rd_wait_for_request_completed(void *param){
	
	feature_input_t* pfeature_input = param;
	
	/*wait for features to be ready*/
	pfeature_input->sops->sem_num = PREPROC_OUT_READY; 
	pfeature_input->sops->sem_op = -1;
	pfeature_input->sops->sem_flg = 0;	
	
	if(semop(pfeature_input->semid, pfeature_input->sops, 1) != 0){
		return EXIT_FAILURE;
	}
	
	/*update page id*/
	pfeature_input->current_page += 1;
	pfeature_input->current_page %= pfeature_input->buffer_depth;
	return EXIT_SUCCESS;
	
}

/**
 * frame_info_t* shm_get_frame_info_ref(void *param)
 * @brief Call to get a reference to the frame info of the current page
 * @param param, reference to the feature input struct
 * @return references to the frame info
 */
frame_info_t* shm_get_frame_info_ref(void *param){
	
	feature_input_t* pfeature_input = param;
	/*compute offset of current page*/
	int offset = pfeature_input->current_page*pfeature_input->page_size;
	return (frame_info_t*)&(pfeature_input->shm_buf[offset]);
}

/**
 * frame_info_t* shm_get_frame_info_ref(void *param)
 * @brief Call to get a reference to the feature vector of the current page
 * @param param, reference to the feature input struct
 * @return reference to the feature vector
 */
double* shm_get_feature_array_ref(void *param){
	
	feature_input_t* pfeature_input = param;
	/*compute offset of current page and skip frame info*/
	int offset = pfeature_input->current_page*pfeature_input->page_size + sizeof(frame_info_t);
	return (double*)&(pfeature_input->shm_buf[offset]);
}


/**
 * int shm_rd_cleanup(void *param)
 * @brief Clean up shared memory and semaphore linkage
 * @param param, unused
 * @return EXIT_FAILURE for unknown type, EXIT_SUCCESS for known/success
 */
int shm_rd_cleanup(void *param){
	
	feature_input_t* pfeature_input = param;
	
	/* Detach the shared memory segment. */
	shmdt(pfeature_input->shm_buf);
	
	return EXIT_SUCCESS;
}
