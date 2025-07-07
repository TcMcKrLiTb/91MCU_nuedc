/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ti_msp_dl_config.h"

#include "arm_const_structs.h"
#include "arm_math.h"

#define NUM_SAMPLES 256
#define IFFTFLAG 0
#define BITREVERSE 1

static uint32_t adcBuffer[NUM_SAMPLES] = {
    0,      4091,   8021,   11639,  14810,  17421,  19391,  20671,  21246,
    21137,  20397,  19109,  17378,  15325,  13078,  10767,  8513,   6421,
    4577,   3040,   1839,   976,    424,    128,    16,     0,      -16,
    -128,   -424,   -976,   -1839,  -3040,  -4577,  -6421,  -8513,  -10767,
    -13078, -15325, -17378, -19109, -20397, -21137, -21246, -20671, -19391,
    -17421, -14810, -11639, -8021,  -4091,  0,      4091,   8021,   11639,
    14810,  17421,  19391,  20671,  21246,  21137,  20397,  19109,  17378,
    15325,  13078,  10767,  8513,   6421,   4577,   3040,   1839,   976,
    424,    128,    16,     0,      -16,    -128,   -424,   -976,   -1839,
    -3040,  -4577,  -6421,  -8513,  -10767, -13078, -15325, -17378, -19109,
    -20397, -21137, -21246, -20671, -19391, -17421, -14810, -11639, -8021,
    -4091,  0,      4091,   8021,   11639,  14810,  17421,  19391,  20671,
    21246,  21137,  20397,  19109,  17378,  15325,  13078,  10767,  8513,
    6421,   4577,   3040,   1839,   976,    424,    128,    16,     0,
    -16,    -128,   -424,   -976,   -1839,  -3040,  -4577,  -6421,  -8513,
    -10767, -13078, -15325, -17378, -19109, -20397, -21137, -21246, -20671,
    -19391, -17421, -14810, -11639, -8021,  -4091,  0,      4091,   8021,
    11639,  14810,  17421,  19391,  20671,  21246,  21137,  20397,  19109,
    17378,  15325,  13078,  10767,  8513,   6421,   4577,   3040,   1839,
    976,    424,    128,    16,     0,      -16,    -128,   -424,   -976,
    -1839,  -3040,  -4577,  -6421,  -8513,  -10767, -13078, -15325, -17378,
    -19109, -20397, -21137, -21246, -20671, -19391, -17421, -14810, -11639,
    -8021,  -4091,  0,      4091,   8021,   11639,  14810,  17421,  19391,
    20671,  21246,  21137,  20397,  19109,  17378,  15325,  13078,  10767,
    8513,   6421,   4577,   3040,   1839,   976,    424,    128,    16,
    0,      -16,    -128,   -424,   -976,   -1839,  -3040,  -4577,  -6421,
    -8513,  -10767, -13078, -15325, -17378, -19109, -20397, -21137, -21246,
    -20671, -19391, -17421, -14810, -11639, -8021,  -4091,  0,      4091,
    8021,   11639,  14810,  17421};

volatile int16_t FFTOutput[NUM_SAMPLES * 2];
volatile uint32_t FFTmaxValue;
volatile uint32_t FFTmaxFreqIndex;

int main(void)
{
    SYSCFG_DL_init();
    arm_cfft_q15(&arm_cfft_sR_q15_len256, (q15_t *)adcBuffer, IFFTFLAG,
                 BITREVERSE);
    arm_cmplx_mag_q15((q15_t *)adcBuffer, (q15_t *)FFTOutput, NUM_SAMPLES);
    arm_max_q15((q15_t *)FFTOutput, NUM_SAMPLES, (q15_t *)&FFTmaxValue,
                (uint32_t *)&FFTmaxFreqIndex);

    __BKPT(0);
    while (1)
    {
    }
}
