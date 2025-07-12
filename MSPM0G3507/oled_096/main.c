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

#include "stdio.h"

#define OLED_SPI 1
#define OLED_IIC 2
#define TEST_MODE OLED_IIC

#if TEST_MODE == OLED_SPI
#include "oled_hardware_spi/oled.h"
#include "oled_hardware_spi/gui.h"
#else
#include "oled_hardware_iic/oled.h"
#include "oled_hardware_iic/gui.h"
#endif

uint16_t fpsCounter = 0, fpsNum = 0;

int main(void)
{
    char tempStr[10];
    uint8_t strLen = 0;
    SYSCFG_DL_init();

    NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);
    fpsCounter = 0;
    DL_Timer_startCounter(TIMER_0_INST);

#if TEST_MODE == OLED_SPI
    OLED_Init();
    OLED_Clear(0);
    OLED_Display();
    while (1)
    {
        GUI_ShowChar(0, 0, 'C', 8, 1);
        strLen = snprintf(tempStr, 10, "fps:%d", fpsNum);
        strLen++;
        GUI_ShowString(128 - 6 * strLen, 0, (uint8_t*)tempStr, 8, 1);
        OLED_Display();
        fpsCounter++;
    }
#else
    OLED_Init();
    OLED_Clear(0);
    OLED_Display();
#endif

    while (1)
    {
        GUI_ShowChar(0, 0, 'C', 8, 1);
        strLen = snprintf(tempStr, 10, "fps:%d", fpsNum);
        strLen++;
        GUI_ShowString(128 - 6 * strLen, 0, (uint8_t*)tempStr, 8, 1);
        OLED_Display();
        fpsCounter++;
    }
}

void TIMER_0_INST_IRQHandler(void)
{
    switch (DL_TimerG_getPendingInterrupt(TIMER_0_INST)) {
        case DL_TIMER_IIDX_ZERO:
            fpsNum = fpsCounter;
            fpsCounter = 0;
            break;
        default:
            break;
    }
}
