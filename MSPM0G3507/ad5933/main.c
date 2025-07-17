/*
 * Copyright (c) 2024, Texas Instruments Incorporated
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

#include "stdio.h"
#include "string.h"

#include "AD5933.h"

#define system_delay_ms(m) delay_cycles((m * (CPUCLK_FREQ / 1000)))

#define START_FREQ (80000)
#define FREQ_INCR (1000)
#define NUM_INCR (40)
#define REF_RESIST (10000)

float gain[NUM_INCR + 1];
int phase[NUM_INCR + 1];

void frequencySweepEasy();

int main(void)
{
    SYSCFG_DL_init();

    printf("Hello\r\n");

    NVIC_EnableIRQ(I2C_INST_INT_IRQN);

    printf("AD5933 Test Started!\r\n");

    // Perform initial configuration. Fail if any one of these fail.
    if (!(AD5933_Reset() && AD5933_SetInternalClock(true) &&
          AD5933_SetStartFrequency(START_FREQ) &&
          AD5933_SetIncrementFrequency(FREQ_INCR) &&
          AD5933_SetNumberIncrements(NUM_INCR) &&
          AD5933_SetPGAGain(PGA_GAIN_X1)))
    {
        printf("FAILED in initialization!\r\n");
        while (true)
            ;
    }

    // Perform calibration sweep
    if (AD5933_Calibrate(gain, phase, REF_RESIST, NUM_INCR + 1))
        printf("Calibrated!\r\n");
    else
        printf("Calibration failed...\r\n");

    while (1)
    {
        frequencySweepEasy();
        system_delay_ms(5000);
    }
}

void frequencySweepEasy()
{
    // Create arrays to hold the data
    int real[NUM_INCR + 1], imag[NUM_INCR + 1];

    // Perform the frequency sweep
    if (AD5933_FrequencySweep(real, imag, NUM_INCR + 1))
    {
        // Print the frequency data
        int cfreq = START_FREQ / 1000;
        for (int i = 0; i < NUM_INCR + 1; i++, cfreq += FREQ_INCR / 1000)
        {
            // Print raw frequency data
            printf("%d", cfreq);
            printf(": R=");
            printf("%d", real[i]);
            printf("/I=");
            printf("%d", imag[i]);

            // Compute impedance
            float magnitude = sqrt(pow(real[i], 2) + pow(imag[i], 2));
            float impedance = 1 / (magnitude * gain[i]);
            printf("  |Z|=");
            printf("%f\r\n", impedance);
        }
        printf("Frequency sweep complete!\r\n");
    }
    else
    {
        printf("Frequency sweep failed...\r\n");
    }
}

int fputc(int c, FILE *stream)
{
    DL_UART_Main_transmitDataBlocking(UART_0_INST, c);
    return c;
}

int fputs(const char *restrict s, FILE *restrict stream)
{
    uint16_t i, len;
    len = strlen(s);
    for (i = 0; i < len; i++)
    {
        DL_UART_Main_transmitDataBlocking(UART_0_INST, s[i]);
    }
    return len;
}

int puts(const char *_ptr)
{
    int count = fputs(_ptr, stdout);
    count += fputs("\n", stdout);
    return count;
}
