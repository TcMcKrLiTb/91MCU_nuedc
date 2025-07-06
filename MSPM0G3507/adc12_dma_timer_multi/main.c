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

// when using FIFO mode sample num need to be multiple of 12 / channel_num
#define SAMPLE_NUM 600
#define FIFO_NUM (SAMPLE_NUM)

volatile uint16_t adcSamples[SAMPLE_NUM << 1];
volatile bool adc_flag = 0;

int main(void)
{
    SYSCFG_DL_init();
    NVIC_EnableIRQ(ADC12_0_INST_INT_IRQN);

    while (1)
    {
        adc_flag = false;
        DL_DMA_setSrcAddr(DMA, DMA_CH0_CHAN_ID,
                          (uint32_t)DL_ADC12_getFIFOAddress(ADC12_0_INST));
        DL_DMA_setDestAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t)&adcSamples[0]);
        DL_DMA_setTransferSize(DMA, DMA_CH0_CHAN_ID, FIFO_NUM);
        DL_DMA_enableChannel(DMA, DMA_CH0_CHAN_ID);
        DL_TimerA_startCounter(TIMER_ADC_INST);

        while (false == adc_flag)
            ;

        __BKPT();
    }
}

void ADC12_0_INST_IRQHandler(void)
{
    switch (DL_ADC12_getPendingInterrupt(ADC12_0_INST))
    {
    case DL_ADC12_IIDX_DMA_DONE:
        DL_TimerA_stopCounter(TIMER_ADC_INST);
        adc_flag = true;
        break;
    default:
        break;
    }
}
