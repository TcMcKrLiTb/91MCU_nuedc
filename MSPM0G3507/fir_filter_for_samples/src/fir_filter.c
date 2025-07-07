#include "fir_filter.h"

#ifdef __cplusplus
#include <cstdio>
#include <cstring>
#endif

#include "string.h"
#include "stdio.h"

#define FILTER_ORDER 139
#define OFFSET 69
#define FIXED_POINT_PRECISION 16

static int32_t filterCoeff[FILTER_ORDER] = {
    170, 25, 26, 26, 26, 25, 24, 21, 18, 13, 8, 2, -5, -12, -21, -30, -40, -50, -61, -72, -83, -93, -104,
-114, -123, -131, -139, -144, -148, -150, -151, -148, -144, -136, -126, -113, -97, -77, -55, -29, 0,
33, 68, 106, 147, 191, 236, 284, 334, 386, 438, 491, 545, 598, 651, 703, 754, 803, 850, 894, 935, 974,
1008, 1039, 1065, 1087, 1104, 1116, 1124, 1126, 1124, 1116, 1104, 1087, 1065, 1039, 1008, 974, 935, 894,
850, 803, 754, 703, 651, 598, 545, 491, 438, 386, 334, 284, 236, 191, 147, 106, 68, 33, 0, -29, -55,
-77, -97, -113, -126, -136, -144, -148, -151, -150, -148, -144, -139, -131, -123, -114, -104, -93, -83,
-72, -61, -50, -40, -30, -21, -12, -5, 2, 8, 13, 18, 21, 24, 25, 26, 26, 26, 25, 170
};

const DL_MathACL_operationConfig gMpyConfig = {
    .opType      = DL_MATHACL_OP_TYPE_MAC,
    .opSign      = DL_MATHACL_OPSIGN_SIGNED,
    .iterations  = 0,
    .scaleFactor = 0,
    .qType       = DL_MATHACL_Q_TYPE_Q16};

void fir_low_pass_filter(float *source, float *result, uint16_t buff_size)
{
    uint32_t gDelayLine[FILTER_ORDER];
    uint32_t gResult = 0;
    memset(gDelayLine, 0, sizeof(gDelayLine));
    /* Configure MathACL for Multiply */
    DL_MathACL_configOperation(MATHACL, &gMpyConfig, 0, 0 );
    for (int i = 0; i < buff_size + FILTER_ORDER; i++)
	{
        memmove(&gDelayLine[1], gDelayLine, sizeof(gDelayLine) - sizeof(gDelayLine[0]));
        gDelayLine[0] = (uint16_t)source[i % buff_size];
        for (int i = 0; i < FILTER_ORDER; i++){
            /* Set Operand One last */
            DL_MathACL_setOperandTwo(MATHACL, filterCoeff[i]);
            DL_MathACL_setOperandOne(MATHACL, gDelayLine[i]);
            DL_MathACL_waitForOperation(MATHACL);
        }
        gResult = DL_MathACL_getResultOne(MATHACL);
		result[(i + buff_size - OFFSET) % buff_size] = (float)gResult * 2;
        /* Clear Results Registers */
        DL_MathACL_clearResults(MATHACL);
	}
}

