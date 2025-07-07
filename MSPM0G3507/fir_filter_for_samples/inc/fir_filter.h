#ifndef __FIR_FILTER_H__
#define __FIR_FILTER_H__

#include "ti_msp_dl_config.h"

void fir_low_pass_filter(float *source, float *result, uint16_t buff_size);

#endif
