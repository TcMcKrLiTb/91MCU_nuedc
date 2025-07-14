#ifndef __AD9833_H__
#define __AD9833_H__

#include "ti/driverlib/dl_gpio.h"
#include "ti_msp_dl_config.h"

#define FMCLK 25000000 // Master Clock On AD9833
enum WaveType
{
    SIN,
    SQR,
    TRI
}; // Wave Selection Enum

typedef enum
{
    USER_AD9833_0 = 0,
    USER_AD9833_1,
    USER_AD9833_MAX,
} user_ad9833_t;

void AD9833_choose(user_ad9833_t x);
void AD9833_SetWave(uint16_t Wave);                      // Sets Output Wave Type
void AD9833_SetWaveData(float Frequency, float Phase);   // Sets Wave Frequency & Phase
void AD9833_Init(uint16_t Wave, float FRQ, float Phase); // Initializing AD9833

#endif
