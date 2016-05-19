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

#include <stats.h>

#define NB_CHANNELS_USED 2
#define NB_PACKETS_DROPPED 3

/*navigation in feature vector*/
#define FEAT_IDX_START 4
#define FEAT_IDX_SECOND 5
#define FEAT_IDX_END 7
#define CHANNEL_WIDTH 55
#define SECOND_CHANNEL_OFFSET 3*CHANNEL_WIDTH


void get_peak_from_channels(double *max_left, double *max_right, double *feature_array);
void get_mean_from_channels(double *mean_left, double *mean_right, double *feature_array);

/**
 * int init_feat_processing(feat_proc_t* feature_proc)
 * @brief initialize the feature processing 
 * @param feature_proc, pointer to feature processing
 * @return EXIT_SUCCESS, EXIT_FAILURE
 */
int init_feat_processing(feat_proc_t * feature_proc __attribute__ ((unused)))
{

	return EXIT_SUCCESS;
}

/**
 * void train_feat_processing(feat_proc_t* feature_proc)
 * @brief train the feature processing, by recording a series of samples
 * @param feature_proc, pointer to feature processing
 * @return EXIT_SUCCESS, EXIT_FAILURE
 */
void train_feat_processing(feat_proc_t * feature_proc)
{

	int i = 0;

	/*pointers to the feature array */
	frame_info_t *frame_info;
	double *feature_array;

	double *training_set = (double *)malloc(feature_proc->nb_train_samples * NB_CHANNELS_USED * sizeof(double));

	if (training_set == NULL) {
		printf("Training_set malloc() may have failed OR is NULL\n - may function not as intented\n");
	}

	double mean_left = 0.0;
	double mean_right = 0.0;

	/*drop first NB_PACKETS_DROPPED packets to prevent errors */
	/*(empirical observation, should be fixed in data_interface in a later release) */
	for (i = 0; i < NB_PACKETS_DROPPED; i++) {
		REQUEST_FEAT_FC(feature_proc->feature_input);
		WAIT_FEAT_FC(feature_proc->feature_input);
	}

	/*start acquisition */
	i = 0;
	while (i < feature_proc->nb_train_samples) {

		/*log the next sequence of samples */
		REQUEST_FEAT_FC(feature_proc->feature_input);
		WAIT_FEAT_FC(feature_proc->feature_input);

		/*get reference on current frame info */
		frame_info = GET_FRAME_INFO_FC(feature_proc->feature_input);
		/*get reference on current feature array */
		feature_array = GET_FVECT_INFO_FC(feature_proc->feature_input);

		/*check if there is an eye blink in the sample */
		if (!frame_info->eye_blink_detected) {
			/*parse feature array to find peak values around 10Hz */
			get_mean_from_channels(&mean_left, &mean_right, feature_array);

			/*pick the two alpha wave samples */
			training_set[i * 2] = mean_left;
			training_set[i * 2 + 1] = mean_right;

			if (i % 5 == 0) {
				printf("training progress: %.1f\n",
				       (float)i / (float)feature_proc->nb_train_samples * 100);
				fflush(stdout);
			}
			i++;
		} else {
			printf("Frame invalid: ");
			if (frame_info->eye_blink_detected) {
				printf("Eye blink detected\n");
			}
		}
	}

	/*Show the training set on console */
	printf("left\tright\n");
	for (i = 0; i < feature_proc->nb_train_samples; i++) {
		printf("[%i]:\t%lf\t%lf\n", i, training_set[i * 2], training_set[i * 2 + 1]);
	}

	/*extract the training set parameters: */
	/* -compute the mean */
	printf("Computing the mean\n");
	stat_mean(training_set, feature_proc->mean, feature_proc->nb_train_samples, 2);
	printf("mean[%i]:\t%lf\t%lf\n", i, feature_proc->mean[0], feature_proc->mean[1]);

	/* -compute the standard deviation */
	printf("Computing the std\n");
	stat_std(training_set, feature_proc->mean, feature_proc->std_dev, feature_proc->nb_train_samples, 2);
	printf("std[%i]:\t%lf\t%lf\n", i, feature_proc->std_dev[0], feature_proc->std_dev[1]);
	fflush(stdout);

	printf("Training completed\n");
	free(training_set);

}

/**
 * int get_normalized_sample(feat_proc_t* feature_proc)
 * 
 * @brief parse and z-score the newly acquired sample
 * @param feature_proc, pointer to feature processing
 * @return EXIT_SUCCESS, EXIT_FAILURE
 */
int get_normalized_sample(feat_proc_t * feature_proc)
{

	/*pointers to the feature array */
	frame_info_t *frame_info;
	double *feature_array;
	double features[2];
	char frame_valid = 0x00;
	double mean_left = 0;
	double mean_right = 0;

	/*make sure to return a valid sample */
	while (!frame_valid) {

		/*request and... */
		REQUEST_FEAT_FC(feature_proc->feature_input);
		/*wait for a sample */
		WAIT_FEAT_FC(feature_proc->feature_input);

		/*get reference on current frame info */
		frame_info = GET_FRAME_INFO_FC(feature_proc->feature_input);
		/*get reference on current feature array */
		feature_array = GET_FVECT_INFO_FC(feature_proc->feature_input);

		if (!frame_info->eye_blink_detected) {
			/*parse feature array to find peak values around 10Hz */
			get_mean_from_channels(&mean_left, &mean_right, feature_array);

			/*get the samples */
			features[0] = (mean_left - feature_proc->mean[0]) / feature_proc->std_dev[0];
			features[1] = (mean_right - feature_proc->mean[1]) / feature_proc->std_dev[1];

			/*get the normalized average */
			feature_proc->sample = (features[0] + features[1]) / 2;

			if(abs(feature_proc->sample)>7){
				printf("Frame invalid: ");
				printf("Exceeding tolerances\n");
			}else{
				frame_valid = 0x01;
			}
			

		} else {
			printf("Frame invalid: ");
			if (frame_info->eye_blink_detected) {
				printf("Eye blink detected\n");
			}
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
void get_peak_from_channels(double *max_left, double *max_right, double *feature_array)
{

	int k = 0;

	/*read beginning of feature range */
	*max_left = feature_array[FEAT_IDX_START];
	*max_right = feature_array[FEAT_IDX_START + SECOND_CHANNEL_OFFSET];

	/*find max within the range for left and right */
	for (k = FEAT_IDX_SECOND; k < FEAT_IDX_END; k++) {
		if (feature_array[k] > *max_left) {
			*max_left = feature_array[k];
		}

		if (feature_array[k + SECOND_CHANNEL_OFFSET] > *max_right) {
			*max_right = feature_array[k + SECOND_CHANNEL_OFFSET];
		}
	}
}



/**
 * void get_mean_from_channels(double *mean_left, double *mean_right, double *feature_array)
 * @brief parse newly acquired sample to return the mean value within the defined range
 * @param mean_left(out), mean value left channel
 * @param mean_right(out), mean value right channel
 * @param feature_array, array of features to be parsed
 * @return EXIT_SUCCESS, EXIT_FAILURE
 */
void get_mean_from_channels(double *mean_left, double *mean_right, double *feature_array)
{

	int k = 0;

	/*read beginning of feature range */
	*mean_left = feature_array[FEAT_IDX_START];
	*mean_right = feature_array[FEAT_IDX_START + SECOND_CHANNEL_OFFSET];

	/*find max within the range for left and right */
	for (k = FEAT_IDX_SECOND; k < FEAT_IDX_END; k++) {
		*mean_left += feature_array[k];
		*mean_right += feature_array[k + SECOND_CHANNEL_OFFSET];
	}
}


/**
 * int clean_up_feat_processing(feat_proc_t* feature_proc)
 * @brief clean up the service
 * @param feature_proc, pointer to feature processing
 * @return EXIT_SUCCESS, EXIT_FAILURE
 */
int clean_up_feat_processing(feat_proc_t * feature_proc __attribute__ ((unused)))
{

	return EXIT_SUCCESS;
}

