#include "AD9833.h"

#include "stdint.h"
#include "string.h"
#include "ti/driverlib/dl_spi.h"
#include "ti_msp_dl_config.h"

#ifdef __cplusplus
#include <cstdint>
#include <cstring>
#endif

uint16_t FRQLW = 0;    // MSB of Frequency Tuning Word
uint16_t FRQHW = 0;    // LSB of Frequency Tuning Word
uint32_t phaseVal = 0; // Phase Tuning Value
uint8_t WKNOWN = 0;    // Flag Variable

user_ad9833_t now_ad9833 = USER_AD9833_MAX;

GPIO_Regs *ad_fnc_ports[USER_AD9833_MAX] = {GPIO_DDS_PORT, GPIO_DDS_PORT};
uint32_t ad_fnc_pins[USER_AD9833_MAX] = {GPIO_DDS_PIN_0_PIN,
                                         GPIO_DDS_PIN_1_PIN};

#define AD9833_SET(x)                                                          \
    (x) ? DL_GPIO_setPins(ad_fnc_ports[now_ad9833], ad_fnc_pins[now_ad9833])   \
        : DL_GPIO_clearPins(ad_fnc_ports[now_ad9833], ad_fnc_pins[now_ad9833])

void AD9833_choose(user_ad9833_t x)
{
    now_ad9833 = x;
}

void AD9833_Init(uint16_t WaveType, float FRQ, float Phase)
{
    AD9833_SetWave(WaveType);       // Type Of Wave
    AD9833_SetWaveData(FRQ, Phase); // Frequency & Phase Set
    return;
}

void writeSPI(uint16_t word)
{
    AD9833_SET(0);
    // DL_SPI_transmitDataBlocking16(SPI_DDS_INST, TxData);
    DL_SPI_transmitDataBlocking8(SPI_DDS_INST, (word >> 8) & 0xff);
    DL_SPI_transmitDataBlocking8(SPI_DDS_INST, word & 0xFF);
    // DL_SPI_transmitData16(SPI_DDS_INST, TxData);
    while (DL_SPI_isBusy(SPI_DDS_INST))
        ;
    AD9833_SET(1);
}

void AD9833_SetWave(uint16_t Wave)
{
    switch (Wave)
    {
    case 0:
        writeSPI(0x2000); // Value for Sinusoidal Wave
        WKNOWN = 0;
        break;
    case 1:
        writeSPI(0x2028); // Value for Square Wave
        WKNOWN = 1;
        break;
    case 2:
        writeSPI(0x2002); // Value for Triangle Wave
        WKNOWN = 2;
        break;
    default:
        break;
    }
}

void AD9833_SetWaveData(float Frequency, float Phase)
{
    long freq = 0;
    uint16_t final_ctrl_word = 0x0000;
    // ---------- Tuning Word for Phase ( 0 - 360 Degree )
    if (Phase < 0)
        Phase = 0; // Changing Phase Value to Positive
    if (Phase > 360)
        Phase = 360; // Maximum value For Phase (In Degree)
    phaseVal =
        ((uint16_t)(Phase * (4096.0f / 360.0f))) & 0x0FFF; // Mask to 12 bits
    phaseVal |= 0XC000;
    // ---------- Tuning word for Frequency
    freq = (uint32_t)(((Frequency * 268435456.0) / FMCLK) + 1); // Tuning Word
    FRQHW = (uint32_t)((freq & 0xFFFC000) >> 14);               // FREQ MSB
    FRQLW = (uint32_t)(freq & 0x3FFF);                          // FREQ LSB
    FRQLW |= 0x4000;
    FRQHW |= 0x4000;
    // ------------------------------------------------ Writing DATA
    writeSPI(0x2100); // enable 16bit words and set reset bit
    writeSPI(FRQLW);
    writeSPI(FRQHW);
    writeSPI(phaseVal);
    switch (WKNOWN)
    {
    case 0:
        final_ctrl_word = 0x2000;
        break; // Sinusoidal
    case 1:
        final_ctrl_word = 0x2028;
        break; // Square
    case 2:
        final_ctrl_word = 0x2002;
        break; // Triangle
    default:
        final_ctrl_word = 0x2000; // Default to Sinusoidal
    }
    writeSPI(final_ctrl_word); // clear reset bit
    AD9833_SetWave(WKNOWN);
    return;
}
