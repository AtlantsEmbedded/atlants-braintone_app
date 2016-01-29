
#ifndef FAKE_FEAT_GENERATOR_H
#define FAKE_FEAT_GENERATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "feature_input.h"
#include "feature_structure.h"

int fake_feat_gen_init(void *param);
int fake_feat_gen_request(void *param);
int fake_feat_gen_wait_for_request_completed(void *param);
frame_info_t* fake_feat_gen_frame_info_ref(void *param);
double* fake_feat_gen_feature_array_ref(void *param);
int fake_feat_gen_cleanup(void *param);

#endif
