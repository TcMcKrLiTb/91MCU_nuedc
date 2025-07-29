#ifndef __ADS131_H__
#define __ADS131_H__

#include "ti_msp_dl_config.h"

/// The interface operates in SPI mode 1 where CPOL = 0 and CPHA = 1.

// Register default acquisition time
#define T_REGACQ 5
// Register Channle Reg Offset, From Address Offset Start,
// Following are CFG, OCAL_MSB, OCAL_LSB, GCAL_MSB, GCAL_LSB
#define CH0_REG_ADD_OFFSET 0x09
#define CH1_REG_ADD_OFFSET 0x0e
#define CH2_REG_ADD_OFFSET 0x13
#define CH3_REG_ADD_OFFSET 0x18

typedef struct ADS131_OneSample_t
{
    uint16_t status;
    int32_t ch0;
    int32_t ch1;
    int32_t ch2;
    int32_t ch3;
} ADS131_OneSample;

typedef enum
{
    ADS_NULL = 0x0000,
    ADS_RESET = 0x0011,
    ADS_STANDBY = 0x0022,
    ADS_WAKEUP = 0x0033,
    ADS_LOCK = 0x0555,
    ADS_UNLOCK = 0x0655,
    ADS_RREG = 0xA000,
    ADS_WREG = 0x6000,
} ADS131_Command_t;

typedef enum
{
    ADS_POWER_VERY_LOW = 0x0,
    ADS_POWER_LOW = 0x1,
    ADS_POWER_HIGH_RES = 0x3,
} ADS131_Power_t;

typedef enum
{
    // ADS_OSR_64 = 0x0,
    ADS_OSR_128 = 0x0,
    ADS_OSR_256 = 0x1,
    ADS_OSR_512 = 0x2,
    ADS_OSR_1024 = 0x3,
    ADS_OSR_2048 = 0x4,
    ADS_OSR_4096 = 0x5,
    ADS_OSR_8192 = 0x6,
    ADS_OSR_16384 = 0x7,
} ADS131_OSR_t;

typedef enum
{
    ADS_PGA_GAIN_1 = 0x0,
    ADS_PGA_GAIN_2 = 0x1,
    ADS_PGA_GAIN_4 = 0x2,
    ADS_PGA_GAIN_8 = 0x3,
    ADS_PGA_GAIN_16 = 0x4,
    ADS_PGA_GAIN_32 = 0x5,
    ADS_PGA_GAIN_64 = 0x6,
    ADS_PGA_GAIN_128 = 0x7
} ADS131_PGAGain_t;

typedef enum
{
    ADS_MUX_AINP_AINN = 0x0,
    ADS_MUX_INPUTS_SHORTED = 0x1,
    ADS_MUX_POSITIVE_TEST = 0x2,
    ADS_MUX_NEGATIVE_TEST = 0x3,
} ADS131_Channel_MUX_t;

typedef enum
{
    ADS_FILTER_DIS = 0x0,
    ADS_FILTER_4,
    ADS_FILTER_8,
    ADS_FILTER_16,
    ADS_FILTER_32,
    ADS_FILTER_64,
    ADS_FILTER_128,
    ADS_FILTER_256,
    ADS_FILTER_512,
    ADS_FILTER_1024,
    ADS_FILTER_2048,
    ADS_FILTER_4096,
    ADS_FILTER_8192,
    ADS_FILTER_16384,
    ADS_FILTER_32768,
    ADS_FILTER_65536,
} ADS131_FilterSetting_t;

typedef struct ADS131_Handler_t
{
    void (*ADC_CS_HIGH)(void);
    void (*ADC_CS_LOW)(void);
    uint8_t (*ADC_DRDY_Read)(void);
    ADS131_OneSample oneSample;
} ADS131_Handler;

bool ADS131_reset(ADS131_Handler *handler);
void ADS131_sendCommand(ADS131_Handler *handler, ADS131_Command_t command);
void ADS131_readRegister(ADS131_Handler *handler, uint8_t address,
                         uint32_t *datBuf, uint8_t dataSize);
bool ADS131_writeRegister(ADS131_Handler *handler, uint8_t address,
                          uint32_t *datBuf, uint8_t dataSize);
bool ADS131_setClock(ADS131_Handler *handler, uint8_t ch_en, uint8_t tbm,
                     ADS131_Power_t power, ADS131_OSR_t osr);
bool ADS131_setGain(ADS131_Handler *handler, ADS131_PGAGain_t Gain0,
                    ADS131_PGAGain_t Gain1, ADS131_PGAGain_t Gain2,
                    ADS131_PGAGain_t Gain3);
bool ADS131_setChannel(ADS131_Handler *handler, uint8_t channel_offset,
                       uint16_t phase, uint8_t filter_enable,
                       ADS131_Channel_MUX_t mux);
bool ADS131_setChannelOCAL(ADS131_Handler *handler, uint8_t channel_offset,
                           uint32_t ocal);
bool ADS131_setChannelGCAL(ADS131_Handler *handler, uint8_t channel_offset,
                           uint32_t gcal);
bool ADS131_setFilterSetting(ADS131_Handler *handler,
                             ADS131_FilterSetting_t DCBLOCK);
ADS131_OneSample ADS131_readADC(ADS131_Handler *handler);
void ADS131_readContinously(ADS131_Handler *handler, uint8_t channelMask,
                            uint32_t bufSize, ...);

#endif
