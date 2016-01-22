/**
 * @file feature_processing.c
 * @author Frederic Simard (fred.simard@atlantsembedded.com)
 * @date Jan 2016
 * @brief This feature processing service use the highest peak around 10Hz as measurment.
 * It needs to be trained to form a reference frame and then it can be used to 
 * produce normalized sample.
 * 
 * The strategy is simple, it z-transform samples 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "feature_processing.h"
#include "feature_input.h"

#define NB_CHANNELS_USED 2
#define NB_PACKETS_DROPPED 3

/*navigation in feature vector*/
#define FEAT_IDX_START 4
#define FEAT_IDX_SECOND 5
#define FEAT_IDX_END 7
#define CHANNEL_WIDTH 55
#define SECOND_CHANNEL_OFFSET 3*CHANNEL_WIDTH

/*threshold to detect eye-blinks*/
#define EYE_BLINK_THRESHOLD 3.5

void get_peak_from_channels(double* max_left, double* max_right, double* feature_array);
void stat_mean(double *a, double *mean, int dim_i, int dim_j);
void stat_std(double *a, double *mean, double *std, int dim_i, int dim_j);

/**
 * int init_feat_processing(feat_proc_t* feature_proc)
 * @brief initialize the feature processing 
 * @param feature_proc, pointer to feature processing
 * @return EXIT_SUCCESS, EXIT_FAILURE
 */
int init_feat_processing(feat_proc_t* feature_proc){
	
	return EXIT_SUCCESS;
}


/**
 * void train_feat_processing(feat_proc_t* feature_proc)
 * @brief train the feature processing, by recording a series of samples
 * @param feature_proc, pointer to feature processing
 * @return EXIT_SUCCESS, EXIT_FAILURE
 */
void train_feat_processing(feat_proc_t* feature_proc){

	int i=0;
	double* feature_array = (double*)feature_proc->feature_input->shm_buf;
	double* training_set = (double*)malloc(feature_proc->nb_train_samples*NB_CHANNELS_USED*sizeof(double));
	double max_left = 0.0;
	double max_right = 0.0;
	
	/*drop first NB_PACKETS_DROPPED packets to prevent errors*/
	/*(empirical observation, should be fixed in data_interface in a later release)*/
	for(i=0;i<NB_PACKETS_DROPPED;i++){
		REQUEST_FEAT_FC(feature_proc->feature_input);
		WAIT_FEAT_FC(feature_proc->feature_input);
	}
	
	/*start acquisition*/
	for(i=0;i<feature_proc->nb_train_samples;i++){
		
		/*log the next sequence of samples*/
		REQUEST_FEAT_FC(feature_proc->feature_input);
		WAIT_FEAT_FC(feature_proc->feature_input);
		
		/*parse feature array to find peak values around 10Hz*/
		get_peak_from_channels(&max_left, &max_right, feature_array);	
		
		/*pick the two alpha wave samples*/
		training_set[i*2] = max_left;
		training_set[i*2+1] = max_right;
		
		if(i%5==0){
			printf("training progress: %.1f\n",(float)i/(float)feature_proc->nb_train_samples*100);
			fflush(stdout);
		}	
	}

	/*Show the training set on console*/
	printf("left\tright\n");
	for(i=0;i<feature_proc->nb_train_samples;i++){
		printf("[%i]:\t%lf\t%lf\n",i,training_set[i*2],training_set[i*2+1]);
	}
	
	/*extract the training set parameters:*/
	/* -compute the mean*/
	printf("Computing the mean\n");
	stat_mean(training_set, feature_proc->mean, feature_proc->nb_train_samples, 2);
	printf("mean[%i]:\t%lf\t%lf\n",i,feature_proc->mean[0],feature_proc->mean[1]);
	
	/* -compute the standard deviation*/
	printf("Computing the std\n");
	stat_std(training_set, feature_proc->mean, feature_proc->std_dev, feature_proc->nb_train_samples, 2);
	printf("std[%i]:\t%lf\t%lf\n",i,feature_proc->std_dev[0],feature_proc->std_dev[1]);
	fflush(stdout);	
	
	printf("Done\n");
	free(training_set);
	printf("and Done\n");
	
}

/**
 * int get_normalized_sample(feat_proc_t* feature_proc)
 * @brief parse and z-score the newly acquired sample
 * @param feature_proc, pointer to feature processing
 * @return EXIT_SUCCESS, EXIT_FAILURE
 */
int get_normalized_sample(feat_proc_t* feature_proc){
	
	double* feature_array = (double*)feature_proc->feature_input->shm_buf;
	double features[2];
	char eye_blink_detected = 0x01;
	double max_left=0;
	double max_right=0;
	
	/*make sure to return a valid sample*/	
	while(eye_blink_detected){	
		
		/*request and...*/
		REQUEST_FEAT_FC(feature_proc->feature_input);
		/*wait for a sample*/
		WAIT_FEAT_FC(feature_proc->feature_input);

		/*parse feature array to find peak values around 10Hz*/
		get_peak_from_channels(&max_left, &max_right, feature_array);
		
		/*get the samples*/
		features[0] = (max_left-feature_proc->mean[0])/feature_proc->std_dev[0];
		features[1] = (max_right-feature_proc->mean[1])/feature_proc->std_dev[1];
		
		/*get the normalized average*/
		feature_proc->sample = (features[0]+features[1])/2;
		
		/*hardcoded blink detection*/
		/*(will be moved to data preprocessing, future release)*/
		if(feature_proc->sample < EYE_BLINK_THRESHOLD){
			eye_blink_detected = 0x00;
		}else{
			printf("Eye blink detected!\n");
			fflush(stdout);
		}
	}
	
	return EXIT_SUCCESS;
}

/**
 * void get_peak_from_channels(double* max_left, double* max_right, double* feature_array)
 * @brief parse newly acquired sample to return the peak value within the defined range
 * @param max_left(out), peak value left channel
 * @param max_right(out), peak value right channel
 * @param feature_array, array of features to be parsed
 * @return EXIT_SUCCESS, EXIT_FAILURE
 */
void get_peak_from_channels(double* max_left, double* max_right, double* feature_array){
	
	int k=0;
	
	/*read beginning of feature range*/
	*max_left = feature_array[FEAT_IDX_START];
	*max_right = feature_array[FEAT_IDX_START+SECOND_CHANNEL_OFFSET];
	
	/*find max within the range for left and right*/
	for(k=FEAT_IDX_SECOND;k<FEAT_IDX_END;k++){
		if(feature_array[k]>*max_left){
			*max_left = feature_array[k];
		}
		
		if(feature_array[k+SECOND_CHANNEL_OFFSET]>*max_right){
			*max_right = feature_array[k+SECOND_CHANNEL_OFFSET];
		}
	}
}

/**
 * int clean_up_feat_processing(feat_proc_t* feature_proc)
 * @brief clean up the service
 * @param feature_proc, pointer to feature processing
 * @return EXIT_SUCCESS, EXIT_FAILURE
 */
int clean_up_feat_processing(feat_proc_t* feature_proc){
	
	return EXIT_SUCCESS;
}


/**
 * void stat_mean(double *a, double *mean, int dim_i, int dim_j)
 * @brief Function to compute the mean of a matrix
 * @param double *a , the matrix
 * @param double *mean (out), the vector that will contain the mean of each column of A
 * @param int dim_i, the number of rows in the matrix
 * @param int dim_j, the number of colomns in the matrix
 */
void stat_mean(double *A, double *mean, int dim_i, int dim_j){
	int i=0,j=0,n=0;  
   
	/*initialize the mean vector*/
	for (j=0;j<dim_j;j++){
            mean[j] = 0;
	}
   
	/*Sum of each column*/
	for(i=0;i<dim_i;i++){
		n = i*dim_j;
		for (j=0;j<dim_j;j++){
            mean[j] += A[j+n];
		}
	}

	/*Divide to get average*/
	for (j=0;j<dim_j;j++){
		mean[j] /= dim_i;
	} 
}

/**
 * void stat_mean(double *a, double *mean, int dim_i, int dim_j)
 * @brief Function to compute the std deviation vector of a matrix
 * @param double *A , the matrix
 * @param double *mean , the mean vector
 * @param double *std (out), the std vector
 * @param int dim_i, the number of rows in the matrix
 * @param int dim_j, the number of colomns in the matrix
 */
void stat_std(double *A, double *mean, double *std, int dim_i, int dim_j){
	int i=0,j=0,n=0;  
   
	/*initialize the std vector*/
	for (j=0;j<dim_j;j++){
        std[j] = 0;
	}
   
	/*Sum of each column*/
	for(i=0;i<dim_i;i++){
		n = i*dim_j;
		for (j=0;j<dim_j;j++){
             std[j] += (A[j+n]-mean[j])*(A[j+n]-mean[j]);
		}
	}

	/*Divide to get average*/
	for (j=0;j<dim_j;j++){
		std[j] /= (double)(dim_i-1);
		std[j] = sqrt(std[j]);
	} 
}
