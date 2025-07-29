#include "ADS131.h"

#include "stdarg.h"
#include "stdio.h"

#define system_delay_us(m) delay_cycles((m * (CPUCLK_FREQ / 1000000)))
#define system_delay_ms(m) delay_cycles((m * (CPUCLK_FREQ / 1000)))

/// @brief write a word (default length is 24) to ADS131
void ADS131_write_word(uint32_t data)
{
    while (DL_SPI_isBusy(SPI_ADS_INST))
        ;
    DL_SPI_transmitDataBlocking8(SPI_ADS_INST, (data >> 16) & 0xff);
    DL_SPI_transmitDataBlocking8(SPI_ADS_INST, (data >> 8) & 0xff);
    DL_SPI_transmitDataBlocking8(SPI_ADS_INST, data & 0xff);
    while (DL_SPI_isBusy(SPI_ADS_INST))
        ;
}

/// @brief read a word (default length is 24) from ADS131
uint32_t ADS131_read_word(uint32_t data)
{
    uint8_t datbuf[3] = {0x00, 0x00, 0x00};
    // firstly clear out the RXFIFO
    while (!DL_SPI_isRXFIFOEmpty(SPI_ADS_INST))
    {
        DL_SPI_receiveDataBlocking8(SPI_ADS_INST);
    }
    // transmit data or just generate sufficient sclk to recieve data
    DL_SPI_transmitDataBlocking8(SPI_ADS_INST, (data >> 16) & 0xff);
    DL_SPI_transmitDataBlocking8(SPI_ADS_INST, (data >> 8) & 0xff);
    DL_SPI_transmitDataBlocking8(SPI_ADS_INST, data & 0xff);
    while (DL_SPI_isBusy(SPI_ADS_INST))
        ;
    for (uint8_t i = 0; i < 3 && !DL_SPI_isRXFIFOEmpty(SPI_ADS_INST); i++)
    {
        datbuf[i] = DL_SPI_receiveDataBlocking8(SPI_ADS_INST);
    }
    return ((uint32_t)datbuf[0] << 16) | ((uint32_t)datbuf[1] << 8) | datbuf[2];
}

/// @brief reset the ADS131
bool ADS131_reset(ADS131_Handler *handler)
{
    volatile uint32_t dat;
    system_delay_us(T_REGACQ);
    // write a completed command frame
    handler->ADC_CS_LOW();
    dat = (uint32_t)ADS_RESET << 8;
    // command word
    ADS131_write_word(dat);
    // CRC (all zero)
    ADS131_write_word(0x00000000);
    // complete all frame
    ADS131_write_word(0x00000000);
    ADS131_write_word(0x00000000);
    ADS131_write_word(0x00000000);
    ADS131_write_word(0x00000000);
    handler->ADC_CS_HIGH();
    // wait for tREGACQ
    system_delay_us(T_REGACQ);
    handler->ADC_CS_LOW();
    // check if reset is completed
    dat = ADS131_read_word(0x00000000);
    // complete all frame
    ADS131_write_word(0x00000000);
    ADS131_write_word(0x00000000);
    ADS131_write_word(0x00000000);
    ADS131_write_word(0x00000000);
    ADS131_write_word(0x00000000);
    handler->ADC_CS_HIGH();
    if (dat == 0xff2400)
        return true;
    return false;
}

/// @brief send command to ADS131 (STANDBY, WAKEUP, LOCK, UNLOCK)
void ADS131_sendCommand(ADS131_Handler *handler, ADS131_Command_t command)
{
    uint32_t dat;
    system_delay_us(T_REGACQ);
    handler->ADC_CS_LOW();
    dat = (uint32_t)command << 8;
    // command word
    ADS131_write_word(dat);
    // CRC (all zero)
    ADS131_write_word(0x00000000);
    // complete all frame
    ADS131_write_word(0x00000000);
    ADS131_write_word(0x00000000);
    ADS131_write_word(0x00000000);
    ADS131_write_word(0x00000000);
    handler->ADC_CS_HIGH();
}

/// @brief read multiple
void ADS131_readRegister(ADS131_Handler *handler, uint8_t address,
                         uint32_t *datBuf, uint8_t dataSize)
{
    volatile uint32_t dat;
    dat = ADS_RREG | ((address & 0x3f) << 7) | (dataSize - 1);
    dat <<= 8;
    system_delay_us(T_REGACQ);
    handler->ADC_CS_LOW();
    // command word
    ADS131_write_word(dat);
    // CRC (all zero)
    ADS131_write_word(0x00000000);
    // complete all frame
    ADS131_write_word(0x00000000);
    ADS131_write_word(0x00000000);
    ADS131_write_word(0x00000000);
    ADS131_write_word(0x00000000);
    handler->ADC_CS_HIGH();
    // wait for tREGACQ
    system_delay_us(T_REGACQ);
    if (dataSize == 1)
    {
        handler->ADC_CS_LOW();
        // only one data don't have a ACK
        datBuf[0] = ADS131_read_word(0x00000000);
        // complete all frame
        for (uint8_t i = 0; i < 5; i++)
            ADS131_read_word(0x00000000);
        handler->ADC_CS_HIGH();
    }
    else
    {
        handler->ADC_CS_LOW();
        // firstly read in ACK word
        dat = ADS131_read_word(0x00000000);
        // read in multiple datas in a loop
        for (uint8_t i = 0; i < dataSize; i++)
        {
            datBuf[i] = ADS131_read_word(0x00000000);
        }
        handler->ADC_CS_HIGH();
    }
}

/// @brief write multiple word to registers, datBuf don't need to aligned
bool ADS131_writeRegister(ADS131_Handler *handler, uint8_t address,
                          uint32_t *datBuf, uint8_t dataSize)
{
    volatile uint32_t dat, dat1;
    dat = ADS_WREG | ((address & 0x3f) << 7) | (dataSize - 1);
    dat <<= 8;
    system_delay_us(T_REGACQ);
    handler->ADC_CS_LOW();
    // command word
    ADS131_write_word(dat);
    // begin to send all the data in a row
    for (uint8_t i = 0; i < dataSize; i++)
    {
        // MSB aligned
        ADS131_write_word(datBuf[i] << 8);
    }
    handler->ADC_CS_HIGH();
    // wait for tREGACQ
    system_delay_us(T_REGACQ);

    handler->ADC_CS_LOW();
    // check the ACK
    dat1 = ADS131_read_word(0x00000000);
    // complete all frame
    for (uint8_t i = 0; i < 5; i++)
        ADS131_read_word(0x00000000);
    handler->ADC_CS_HIGH();
    if ((dat & 0xdfffff) == dat1)
        return true;
    return false;
}

/// @brief set the CLOCK Register of ADS131
/// @param ch_en 4bit represent CH3, CH2, CH1, CH0, 1 means enabled, for
/// example: 0xf means all enabled
/// @param tbm Modulator oversampling ratio 64 selection (turbo mode)
/// 0b = OSR set by bits 4:2 (that is, OSR[2:0])
/// 1b = OSR of 64 is selected
bool ADS131_setClock(ADS131_Handler *handler, uint8_t ch_en, uint8_t tbm,
                     ADS131_Power_t power, ADS131_OSR_t osr)
{
    uint32_t dat;
    dat = (((uint32_t)ch_en & 0x0f) << 8) | ((tbm & 0x01) << 5) | (osr << 2) |
          (power);
    return ADS131_writeRegister(handler, 0x03, &dat, 1);
}

/// @brief set the GAIN Register of ADS131
bool ADS131_setGain(ADS131_Handler *handler, ADS131_PGAGain_t Gain0,
                    ADS131_PGAGain_t Gain1, ADS131_PGAGain_t Gain2,
                    ADS131_PGAGain_t Gain3)
{
    uint32_t dat;
    dat =
        ((uint32_t)Gain3 << 12) | ((uint32_t)Gain2 << 8) | (Gain1 << 4) | Gain0;
    return ADS131_writeRegister(handler, 0x04, &dat, 1);
}

/// @brief set CHx_CFG Register
/// @param filter_enable DC block filter 0 = Disabled 1 = controlled by DCBLOCK
bool ADS131_setChannel(ADS131_Handler *handler, uint8_t channel_offset,
                       uint16_t phase, uint8_t filter_enable,
                       ADS131_Channel_MUX_t mux)
{
    uint32_t dat;
    uint8_t address;
    address = channel_offset + 0;
    dat = (phase << 6) | ((filter_enable ^ 0x1) << 2) | mux;
    return ADS131_writeRegister(handler, address, &dat, 1);
}

/// @brief set CHx_OCAL Register (offset calibration, in a word, make 0mV is 0)
/// @param ocal offset calibration value
bool ADS131_setChannelOCAL(ADS131_Handler *handler, uint8_t channel_offset,
                           uint32_t ocal)
{
    uint32_t dat[2];
    uint8_t address;
    address = channel_offset + 0x01;
    ocal &= 0xffffff;
    dat[0] = (ocal >> 8) & 0xffff;
    dat[1] = (ocal & 0xff) << 8;
    return ADS131_writeRegister(handler, address, dat, 2);
}

/// @brief set CHx_GCAL Register (gain calibration, in a word, make x2 is x2)
/// @param ocal offset calibration value
bool ADS131_setChannelGCAL(ADS131_Handler *handler, uint8_t channel_offset,
                           uint32_t gcal)
{
    uint32_t dat[2];
    uint8_t address;
    address = channel_offset + 0x03;
    dat[0] = (gcal >> 8) & 0xffff;
    dat[1] = (gcal & 0xff) << 8;
    return ADS131_writeRegister(handler, address, dat, 2);
}

/// @brief set DCBLOCK in THRSHLD_LSB Register
bool ADS131_setFilterSetting(ADS131_Handler *handler,
                             ADS131_FilterSetting_t DCBLOCK)
{
    uint32_t dat;
    ADS131_readRegister(handler, 0x08, &dat, 1);
    dat = (dat & 0xfff0) | (DCBLOCK & 0x0f);
    return ADS131_writeRegister(handler, 0x08, &dat, 1);
}

/// @brief do one sample
ADS131_OneSample ADS131_readADC(ADS131_Handler *handler)
{
    ADS131_OneSample res;
    uint32_t result[4];
    while (handler->ADC_DRDY_Read())
        ;
    handler->ADC_CS_LOW();
    for (int i = 0; i < 6; i++)
        ADS131_read_word(0);
    handler->ADC_CS_HIGH();
    // wait for tREGACQ
    system_delay_us(T_REGACQ);
    handler->ADC_CS_LOW();
    system_delay_us(10);
    // first word is status register
    res.status = ADS131_read_word(0x00000000);
    // rest 4 words are adc result
    result[0] = ADS131_read_word(0x00000000);
    result[1] = ADS131_read_word(0x00000000);
    result[2] = ADS131_read_word(0x00000000);
    result[3] = ADS131_read_word(0x00000000);
    // complete all frame
    ADS131_read_word(0x00000000);
    handler->ADC_CS_HIGH();
    // uint24_t -> int32_t
    res.ch0 = ((int32_t)(result[0] << 8)) >> 8;
    res.ch1 = ((int32_t)(result[1] << 8)) >> 8;
    res.ch2 = ((int32_t)(result[2] << 8)) >> 8;
    res.ch3 = ((int32_t)(result[3] << 8)) >> 8;
    return res;
}

/// @brief read adc continously
/// @param channel_mask 4bit LSB aligned, 1 means enable, 0x0f means all four
/// channels enabled
void ADS131_readContinously(ADS131_Handler *handler, uint8_t channelMask,
                            uint32_t bufSize, ...)
{
    va_list args;
    int32_t *bufList[4] = {NULL, NULL, NULL, NULL};
    uint32_t result[4];
    // multiple param
    va_start(args, bufSize);
    for (uint8_t i = 0, j = 1; i < 4; i++)
    {
        // make buffer according to channel mask
        if (channelMask & j)
            bufList[i] = va_arg(args, int32_t *);
        j <<= 1;
    }
    va_end(args);
    // according to manual, readin two data firstly to correct time sequence
    ADS131_readADC(handler);
    ADS131_readADC(handler);
    for (uint16_t i = 0; i < bufSize; i++)
    {
        while (handler->ADC_DRDY_Read())
            ;
        handler->ADC_CS_LOW();
        for (uint8_t j = 0; j < 6; j++)
            ADS131_read_word(0);
        handler->ADC_CS_HIGH();
        // wait for tREGACQ
        system_delay_us(T_REGACQ);
        handler->ADC_CS_LOW();
        // first word is status register
        ADS131_read_word(0x00000000);
        // rest 4 words are adc result
        result[0] = ADS131_read_word(0x00000000);
        result[1] = ADS131_read_word(0x00000000);
        result[2] = ADS131_read_word(0x00000000);
        result[3] = ADS131_read_word(0x00000000);
        // complete all frame
        ADS131_read_word(0x00000000);
        handler->ADC_CS_HIGH();
        // uint24_t -> int32_t
        for (uint8_t j = 0, k = 1; j < 4; j++)
        {
            // make buffer according to channel mask
            if (bufList[j])
                bufList[j][i] = ((int32_t)(result[j] << 8)) >> 8;
            k <<= 1;
        }
    }
}
