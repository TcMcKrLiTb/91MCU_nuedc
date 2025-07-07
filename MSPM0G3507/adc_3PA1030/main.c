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

#define SAMPLE_NUMS 20

volatile uint32_t adc_buffer[SAMPLE_NUMS];
volatile bool adc_flag = false;

int main(void)
{
    SYSCFG_DL_init();

    NVIC_EnableIRQ(DMA_INT_IRQn);

    adc_flag = false;
    DL_DMA_setSrcAddr(DMA, DMA_CH0_CHAN_ID,
                      (uint32_t)&(GPIO_3PA_PORT->DIN3_0));
    DL_DMA_setDestAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t)&adc_buffer[0]);
    DL_DMA_setTransferSize(DMA, DMA_CH0_CHAN_ID, SAMPLE_NUMS);
    DL_DMA_enableChannel(DMA, DMA_CH0_CHAN_ID);

    DL_TimerA_startCounter(PWM_0_INST);

    while (adc_flag == false)
        ;

    __BKPT();

    while (1)
    {
    }
}

void DMA_IRQHandler(void)
{
    switch (DL_DMA_getPendingInterrupt(DMA))
    {
    case DL_DMA_EVENT_IIDX_DMACH0:
        DL_TimerA_stopCounter(PWM_0_INST);
        adc_flag = true;
        DL_DMA_clearInterruptStatus(DMA, DL_DMA_INTERRUPT_CHANNEL0);
        break;
    default:
        break;
    }
}
