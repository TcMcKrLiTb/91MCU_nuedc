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

#define NUM_SAMPLES 1024
#define REAL_NUM_SAMPLES (NUM_SAMPLES + 8)
#define FIFO_NUM (REAL_NUM_SAMPLES >> 1)
#define IFFTFLAG 0
#define BITREVERSE 1

volatile int16_t adcBuffer[REAL_NUM_SAMPLES] = {};
volatile float FFTInput[NUM_SAMPLES << 1];
volatile float FFTOutput[NUM_SAMPLES];
volatile float IFFTOutput[NUM_SAMPLES];
volatile uint32_t FFTmaxValue;
volatile uint32_t FFTmaxFreqIndex;
arm_cfft_instance_f32 varInstCfftF32;
volatile bool adcflag;

int main(void)
{
    SYSCFG_DL_init();
    NVIC_EnableIRQ(ADC12_0_INST_INT_IRQN);
    while (1)
    {
        adcflag = false;
        DL_DMA_setSrcAddr(DMA, DMA_CH0_CHAN_ID,
                          (uint32_t)DL_ADC12_getFIFOAddress(ADC12_0_INST));
        DL_DMA_setDestAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t)&adcBuffer[0]);
        DL_DMA_setTransferSize(DMA, DMA_CH0_CHAN_ID, FIFO_NUM);
        DL_DMA_enableChannel(DMA, DMA_CH0_CHAN_ID);
        DL_ADC12_startConversion(ADC12_0_INST);
        while (false == adcflag)
        {
        }

        __BKPT(0);
        for (uint16_t i = 0; i < NUM_SAMPLES; i++)
        {
            FFTInput[i << 1] = (float)adcBuffer[i];
            FFTInput[(i << 1) + 1] = 0.0;
        }
        arm_cfft_init_f32(&varInstCfftF32, NUM_SAMPLES);

        arm_cfft_f32(&varInstCfftF32, (float32_t *)FFTInput, 0, 1);
        arm_cmplx_mag_f32((float32_t *)FFTInput, (float32_t *)FFTOutput,
                          NUM_SAMPLES);
        __BKPT(0);

        arm_cfft_init_f32(&varInstCfftF32, NUM_SAMPLES);
        arm_cfft_f32(&varInstCfftF32, (float32_t*)FFTInput, 1, 1);


        __BKPT(0);
    }
}

void ADC12_0_INST_IRQHandler(void)
{
    switch (DL_ADC12_getPendingInterrupt(ADC12_0_INST))
    {
    case DL_ADC12_IIDX_DMA_DONE:
        DL_ADC12_stopConversion(ADC12_0_INST);
        adcflag = 1;
        // clear interrupt flag
        DL_ADC12_clearInterruptStatus(ADC12_0_INST,
                                      DL_ADC12_INTERRUPT_DMA_DONE);
        break;
    default:
        break;
    }
    // DL_TimerA_startCounter(TIMER_ADC_INST);
}
