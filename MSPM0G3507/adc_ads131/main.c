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

#include "ADS131.h"

#define SAMPLE_NUM 512

void adc_cs_high(void) { DL_GPIO_setPins(GPIO_ADS_PORT, GPIO_ADS_PIN_CS_PIN); }
void adc_cs_low(void) { DL_GPIO_clearPins(GPIO_ADS_PORT, GPIO_ADS_PIN_CS_PIN); }
uint8_t adc_drdy_read(void)
{
    return DL_GPIO_readPins(GPIO_ADS_PORT, GPIO_ADS_PIN_DRDY_PIN) ==
           GPIO_ADS_PIN_DRDY_PIN;
}

ADS131_Handler adc_handler = {.ADC_CS_HIGH = adc_cs_high,
                              .ADC_CS_LOW = adc_cs_low,
                              .ADC_DRDY_Read = adc_drdy_read};
int32_t adcBuffer[SAMPLE_NUM], filterResult[SAMPLE_NUM];
volatile int64_t adcAver;

int main(void)
{
    volatile uint32_t data[2];
    volatile ADS131_OneSample adc_result;
    SYSCFG_DL_init();

    if (!ADS131_reset(&adc_handler))
        __BKPT(0);

    if (!ADS131_setClock(&adc_handler, 0xf, 0, ADS_POWER_HIGH_RES,
                         ADS_OSR_8192))
        __BKPT(0);

    ADS131_readRegister(&adc_handler, 0x03, (uint32_t *)data, 1);

    ADS131_readRegister(&adc_handler, 0x02, (uint32_t *)data, 1);

    if (!ADS131_setGain(&adc_handler, ADS_PGA_GAIN_1, ADS_PGA_GAIN_1,
                        ADS_PGA_GAIN_1, ADS_PGA_GAIN_1))
        __BKPT(0);
    ADS131_readRegister(&adc_handler, 0x04, (uint32_t *)data, 1);
    if (!ADS131_setFilterSetting(&adc_handler, ADS_FILTER_512))
        __BKPT(0);

    if (!ADS131_setChannel(&adc_handler, CH0_REG_ADD_OFFSET, 0, 1,
                           ADS_MUX_AINP_AINN))
        __BKPT(0);
    ADS131_readRegister(&adc_handler, 0x09, (uint32_t *)data, 1);

    if (!ADS131_setChannelOCAL(&adc_handler, CH0_REG_ADD_OFFSET, 1842))
        __BKPT(0);

    adc_result = ADS131_readADC(&adc_handler);

    while (1)
    {
        adcAver = 0;
        ADS131_readContinously(&adc_handler, 0b00000001, SAMPLE_NUM, adcBuffer);
        for (uint16_t i = 0; i < SAMPLE_NUM; i++)
        {
            adcAver += adcBuffer[i];
        }
        adcAver /= SAMPLE_NUM;
        // fir_low_pass_filter(adcBuffer, filterResult, SAMPLE_NUM);
        __BKPT(0);
    }
}
