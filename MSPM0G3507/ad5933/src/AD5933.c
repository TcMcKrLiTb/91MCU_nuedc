#include "AD5933.h"

#include "stdlib.h"

/* Counters for TX length and bytes sent */
uint32_t gTxLen, gTxCount;
/* Data received from Target */
uint8_t *gRxPacket;
/* Counters for TX length and bytes sent */
uint32_t gRxLen, gRxCount;
/* Indicates status of I2C */
enum I2cControllerStatus
{
    I2C_STATUS_IDLE = 0,
    I2C_STATUS_TX_STARTED,
    I2C_STATUS_TX_INPROGRESS,
    I2C_STATUS_TX_COMPLETE,
    I2C_STATUS_RX_STARTED,
    I2C_STATUS_RX_INPROGRESS,
    I2C_STATUS_RX_COMPLETE,
    I2C_STATUS_ERROR,
} gI2cControllerStatus;

static const unsigned long clockSpeed = 16776000;

bool AD5933_Init(void)
{
    NVIC_EnableIRQ(I2C_INST_INT_IRQN);
    return true;
}

bool AD5933_GetByte(uint8_t address, uint8_t *value)
{
    uint8_t txBuf[2] = {ADDR_PTR, address};
    gTxLen = sizeof(txBuf);
    // fill the fifo
    DL_I2C_fillControllerTXFIFO(I2C_INST, txBuf, gTxLen);
    // start TX
    gI2cControllerStatus = I2C_STATUS_TX_STARTED;
    // wait for IDLE
    while (
        !(DL_I2C_getControllerStatus(I2C_INST) & DL_I2C_CONTROLLER_STATUS_IDLE))
        ;
    DL_I2C_startControllerTransfer(I2C_INST, AD5933_ADDR,
                                   DL_I2C_CONTROLLER_DIRECTION_TX, gTxLen);
    // wait for completion or error
    while ((gI2cControllerStatus != I2C_STATUS_TX_COMPLETE) &&
           (gI2cControllerStatus != I2C_STATUS_ERROR))
    {
        __WFE();
    }
    // wait for idle
    while (DL_I2C_getControllerStatus(I2C_INST) &
           DL_I2C_CONTROLLER_STATUS_BUSY_BUS)
        ;
    if (DL_I2C_getControllerStatus(I2C_INST) & DL_I2C_CONTROLLER_STATUS_ERROR)
        return false;
    // a choosable delay
    delay_cycles(10);
    // send read request
    gRxLen = 1;
    gRxCount = 0;
    gRxPacket = value;
    gI2cControllerStatus = I2C_STATUS_RX_STARTED;
    DL_I2C_startControllerTransfer(I2C_INST, AD5933_ADDR,
                                   DL_I2C_CONTROLLER_DIRECTION_RX, gRxLen);
    // wait for completion
    while (gI2cControllerStatus != I2C_STATUS_RX_COMPLETE)
    {
        __WFE();
    }
    while (DL_I2C_getControllerStatus(I2C_INST) &
           DL_I2C_CONTROLLER_STATUS_BUSY_BUS)
        ;
    // check for error
    if (DL_I2C_getControllerStatus(I2C_INST) & DL_I2C_CONTROLLER_STATUS_ERROR)
        return false;
    return true;
}

bool AD5933_SendByte(uint8_t address, uint8_t value)
{
    uint8_t txBuf[2] = {address, value};
    gTxLen = sizeof(txBuf);
    DL_I2C_fillControllerTXFIFO(I2C_INST, txBuf, gTxLen);
    DL_I2C_disableInterrupt(I2C_INST,
                            DL_I2C_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER);
    gI2cControllerStatus = I2C_STATUS_TX_STARTED;
    while (
        !(DL_I2C_getControllerStatus(I2C_INST) & DL_I2C_CONTROLLER_STATUS_IDLE))
        ;
    DL_I2C_startControllerTransfer(I2C_INST, AD5933_ADDR,
                                   DL_I2C_CONTROLLER_DIRECTION_TX, gTxLen);
    while ((gI2cControllerStatus != I2C_STATUS_TX_COMPLETE) &&
           (gI2cControllerStatus != I2C_STATUS_ERROR))
    {
        __WFE();
    }
    while (DL_I2C_getControllerStatus(I2C_INST) &
           DL_I2C_CONTROLLER_STATUS_BUSY_BUS)
        ;
    if (DL_I2C_getControllerStatus(I2C_INST) & DL_I2C_CONTROLLER_STATUS_ERROR)
        return false;
    // a choosable delay
    delay_cycles(10);
    return true;
}

bool AD5933_SetControlMode(uint8_t mode)
{
    uint8_t val;
    if (!AD5933_GetByte(CTRL_REG1, &val))
        return false;
    // Wipe out the top 4 bits...mode bits are bits 5 through 8.
    val &= 0x0F;
    // Set the top 4 bits appropriately
    val |= mode;
    // Write back to the register
    return AD5933_SendByte(CTRL_REG1, val);
}

bool AD5933_Reset(void)
{
    // Get the current value of the control register
    uint8_t val;
    if (!AD5933_GetByte(CTRL_REG2, &val))
        return false;
    // Set bit D4 for restart
    val |= CTRL_RESET;
    // Send byte back
    return AD5933_SendByte(CTRL_REG2, val);
}

bool AD5933_EnableTemperature(uint8_t enable)
{
    // If enable, set temp measure bits. If disable, reset to no operation.
    if (enable == TEMP_MEASURE)
    {
        return AD5933_SetControlMode(CTRL_TEMP_MEASURE);
    }
    else
    {
        return AD5933_SetControlMode(CTRL_NO_OPERATION);
    }
}

float AD5933_GetTemperature(void)
{
    // Set temperature mode
    if (AD5933_EnableTemperature(TEMP_MEASURE))
    {
        // Wait for a valid temperature to be ready
        while ((AD5933_ReadStatusRegister() & STATUS_TEMP_VALID) !=
               STATUS_TEMP_VALID)
            ;

        // Read raw temperature from temperature registers
        uint8_t rawTemp[2];
        if (AD5933_GetByte(TEMP_DATA_1, &rawTemp[0]) &&
            AD5933_GetByte(TEMP_DATA_2, &rawTemp[1]))
        {
            // Combine raw temperature bytes into an interger. The ADC
            // returns a 14-bit 2's C value where the 14th bit is a sign
            // bit. As such, we only need to keep the bottom 13 bits.
            int rawTempVal = (rawTemp[0] << 8 | rawTemp[1]) & 0x1FFF;

            // Convert into celcius using the formula given in the
            // datasheet. There is a different formula depending on the sign
            // bit, which is the 5th bit of the byte in TEMP_DATA_1.
            if ((rawTemp[0] & (1 << 5)) == 0)
            {
                return rawTempVal / 32.0;
            }
            else
            {
                return (rawTempVal - 16384) / 32.0;
            }
        }
    }
    return -1;
}

bool AD5933_SetClockSource(uint8_t source)
{
    // Determine what source was selected and set it appropriately
    switch (source)
    {
    case CLOCK_EXTERNAL:
        return AD5933_SendByte(CTRL_REG2, CTRL_CLOCK_EXTERNAL);
    case CLOCK_INTERNAL:
        return AD5933_SendByte(CTRL_REG2, CTRL_CLOCK_INTERNAL);
    default:
        return false;
    }
}

bool AD5933_SetInternalClock(uint8_t internal)
{
    // This function is mainly a wrapper for setClockSource()
    if (internal)
        return AD5933_SetClockSource(CLOCK_INTERNAL);
    else
        return AD5933_SetClockSource(CLOCK_EXTERNAL);
}

bool AD5933_setSettlingCycles(int32_t time)
{
    int cycles;
    uint8_t settleTime[2], rsTime[2], val;

    settleTime[0] = time & 0xFF;        // LSB - 8B
    settleTime[1] = (time >> 8) & 0xFF; // MSB - 8A

    cycles = (settleTime[0] | (settleTime[1] & 0x1));
    val = (uint8_t)((settleTime[1] & 0x7) >> 1);
    if ((cycles > 0x1FF) || !(val == 0 || val == 1 || val == 3))
    {
        return false;
    }
    if (AD5933_SendByte(NUM_SCYCLES_1, settleTime[1]) &&
        (AD5933_SendByte(NUM_SCYCLES_2, settleTime[0])))
    {
        // Reading values which wrote above
        if (AD5933_GetByte(NUM_SCYCLES_1, &rsTime[1]) &&
            AD5933_GetByte(NUM_SCYCLES_2, &rsTime[0]))
        {
            // checking settling time which send and then read both are same or
            // not
            if ((settleTime[0] == rsTime[0]) && (settleTime[1] == rsTime[1]))
            {
                return true;
            }
        }
    }
    return false;
}

bool AD5933_SetStartFrequency(uint32_t start)
{
    // Page 24 of the Datasheet gives the following formula to represent the
    // start frequency.
    // TODO: Precompute for better performance if we want to keep this constant.
    int32_t freqHex = (start / (clockSpeed / 4.0)) * pow(2, 27);
    if (freqHex > 0xFFFFFF)
    {
        return false; // overflow
    }

    // freqHex should be a 24-bit value. We need to break it up into 3 bytes.
    uint8_t highByte = (freqHex >> 16) & 0xFF;
    uint8_t midByte = (freqHex >> 8) & 0xFF;
    uint8_t lowByte = freqHex & 0xFF;

    // Attempt sending all three bytes
    return AD5933_SendByte(START_FREQ_1, highByte) &&
           AD5933_SendByte(START_FREQ_2, midByte) &&
           AD5933_SendByte(START_FREQ_3, lowByte);
}

bool AD5933_SetIncrementFrequency(uint32_t increment)
{
    // Page 25 of the Datasheet gives the following formula to represent the
    // increment frequency.
    // TODO: Precompute for better performance if we want to keep this constant.
    int32_t freqHex = (increment / (clockSpeed / 4.0)) * pow(2, 27);
    if (freqHex > 0xFFFFFF)
    {
        return false; // overflow
    }

    // freqHex should be a 24-bit value. We need to break it up into 3 bytes.
    uint8_t highByte = (freqHex >> 16) & 0xFF;
    uint8_t midByte = (freqHex >> 8) & 0xFF;
    uint8_t lowByte = freqHex & 0xFF;

    // Attempt sending all three bytes
    return AD5933_SendByte(INC_FREQ_1, highByte) &&
           AD5933_SendByte(INC_FREQ_2, midByte) &&
           AD5933_SendByte(INC_FREQ_3, lowByte);
}

bool AD5933_SetNumberIncrements(uint32_t num)
{
    // Check that the number sent in is valid.
    if (num > 511)
    {
        return false;
    }
    // Divide the 9-bit integer into 2 bytes.
    uint8_t highByte = (num >> 8) & 0xFF;
    uint8_t lowByte = num & 0xFF;
    // Write to register.
    return AD5933_SendByte(NUM_INC_1, highByte) &&
           AD5933_SendByte(NUM_INC_2, lowByte);
}

bool AD5933_SetPGAGain(uint8_t gain)
{
    // Get the current value of the control register
    uint8_t val;
    if (!AD5933_GetByte(CTRL_REG1, &val))
        return false;

    // Clear out the bottom bit, D8, which is the PGA gain set bit
    val &= 0xFE;

    // Determine what gain factor was selected
    if (gain == PGA_GAIN_X1 || gain == 1)
    {
        // Set PGA gain to x1 in CTRL_REG1
        val |= PGA_GAIN_X1;
        return AD5933_SendByte(CTRL_REG1, val);
    }
    else if (gain == PGA_GAIN_X5 || gain == 5)
    {
        // Set PGA gain to x5 in CTRL_REG1
        val |= PGA_GAIN_X5;
        return AD5933_SendByte(CTRL_REG1, val);
    }
    else
    {
        return false;
    }
}

uint8_t AD5933_ReadRegister(uint8_t reg)
{
    // Read status register and return it's value. If fail, return 0xFF.
    uint8_t val;
    if (AD5933_GetByte(reg, &val))
    {
        return val;
    }
    else
    {
        return STATUS_ERROR;
    }
}

bool AD5933_SetRange(uint8_t range)
{
    uint8_t val;
    // Get the current value of the control register
    if (!AD5933_GetByte(CTRL_REG1, &val))
    {
        return false;
    }
    // Clear out the bottom bit, D9 and D10, which is the output voltage range
    // set bit
    val &= 0xF9;
    // Determine what output voltage range was selected
    switch (range)
    {
    case CTRL_OUTPUT_RANGE_2:
        // Set output voltage range to 1.0 V p-p typical in CTRL_REG1
        val |= CTRL_OUTPUT_RANGE_2;
        break;

    case CTRL_OUTPUT_RANGE_3:
        // Set output voltage range to 400 mV p-p typical in CTRL_REG1
        val |= CTRL_OUTPUT_RANGE_3;
        break;

    case CTRL_OUTPUT_RANGE_4:
        // Set output voltage range to 200 mV p-p typical in CTRL_REG1
        val |= CTRL_OUTPUT_RANGE_4;
        break;

    default:
        // Set output voltage range to 200 mV p-p typical in CTRL_REG1
        val |= CTRL_OUTPUT_RANGE_1;
        break;
    }

    // Write to register
    return AD5933_SendByte(CTRL_REG1, val);
}

uint8_t AD5933_ReadStatusRegister(void)
{
    volatile uint8_t status = AD5933_ReadRegister(STATUS_REG);
    return status;
}

int32_t AD5933_ReadControlRegister(void)
{
    return ((AD5933_ReadRegister(CTRL_REG1) << 8) |
            AD5933_ReadRegister(CTRL_REG2)) &
           0xFFFF;
}

bool AD5933_GetComplexData(int32_t *real, int32_t *imag)
{
    // Wait for a measurement to be available
    while ((AD5933_ReadStatusRegister() & STATUS_DATA_VALID) !=
           STATUS_DATA_VALID)
        ;
    // Read the four data registers.
    /// todo: Do this faster with a block read
    uint8_t realComp[2];
    uint8_t imagComp[2];
    if (AD5933_GetByte(REAL_DATA_1, &realComp[0]) &&
        AD5933_GetByte(REAL_DATA_2, &realComp[1]) &&
        AD5933_GetByte(IMAG_DATA_1, &imagComp[0]) &&
        AD5933_GetByte(IMAG_DATA_2, &imagComp[1]))
    {
        // Combine the two separate bytes into a single 16-bit value and store
        // them at the locations specified.
        *real = (int16_t)(((realComp[0] << 8) | realComp[1]) & 0xFFFF);
        *imag = (int16_t)(((imagComp[0] << 8) | imagComp[1]) & 0xFFFF);

        return true;
    }
    else
    {
        *real = -1;
        *imag = -1;
        return false;
    }
}

bool AD5933_SetPowerMode(uint8_t level)
{
    // Make the appropriate switch. TODO: Does no operation even do anything?
    switch (level)
    {
    case POWER_ON:
        return AD5933_SetControlMode(CTRL_NO_OPERATION);
    case POWER_STANDBY:
        return AD5933_SetControlMode(CTRL_STANDBY_MODE);
    case POWER_DOWN:
        return AD5933_SetControlMode(CTRL_POWER_DOWN_MODE);
    default:
        return false;
    }
}

bool AD5933_FrequencySweep(int32_t *real, int32_t *imag, int32_t n)
{
    // Begin by issuing a sequence of commands
    // If the commands aren't taking hold, add a brief delay
    if (!(AD5933_SetPowerMode(POWER_STANDBY) &&          // place in standby
          AD5933_SetControlMode(CTRL_INIT_START_FREQ) && // init start freq
          AD5933_SetControlMode(
              CTRL_START_FREQ_SWEEP))) // begin frequency sweep
    {
        return false;
    }

    // Perform the sweep. Make sure we don't exceed n.
    int i = 0;
    while ((AD5933_ReadStatusRegister() & STATUS_SWEEP_DONE) !=
           STATUS_SWEEP_DONE)
    {
        // Make sure we aren't exceeding the bounds of our buffer
        if (i >= n)
        {
            return false;
        }
        // Get the data for this frequency point and store it in the array
        if (!AD5933_GetComplexData(&real[i], &imag[i]))
        {
            return false;
        }
        // Increment the frequency and our index.
        i++;
        AD5933_SetControlMode(CTRL_INCREMENT_FREQ);
    }

    // Put into standby
    return AD5933_SetPowerMode(POWER_STANDBY);
}

bool AD5933_Calibrate(float *gain, int32_t *phase, int32_t ref, int32_t n)
{
    // We need arrays to hold the real and imaginary values temporarily
    int32_t *real = (int32_t *)malloc(sizeof(int32_t) * n);
    int32_t *imag = (int32_t *)malloc(sizeof(int32_t) * n);

    // Perform the frequency sweep
    if (!AD5933_FrequencySweep(real, imag, n))
    {
        free(real);
        free(imag);
        return false;
    }
    // For each point in the sweep, calculate the gain factor and phase
    for (int i = 0; i < n; i++)
    {
        gain[i] = (double)(1.0 / ref) / sqrt(pow(real[i], 2) + pow(imag[i], 2));
        // TODO: phase
    }
    free(real);
    free(imag);
    return true;
}

bool AD5933_Calibrate2(float *gain, int32_t *phase, int32_t *real,
                       int32_t *imag, int32_t ref, int32_t n)
{
    // Perform the frequency sweep
    if (!AD5933_FrequencySweep(real, imag, n))
    {
        return false;
    }
    // For each point in the sweep, calculate the gain factor and phase
    for (int i = 0; i < n; i++)
    {
        gain[i] = (double)(1.0 / ref) / sqrt(pow(real[i], 2) + pow(imag[i], 2));
        // TODO: phase
    }
    return true;
}

void I2C_INST_IRQHandler(void)
{
    switch (DL_I2C_getPendingInterrupt(I2C_INST))
    {
    case DL_I2C_IIDX_CONTROLLER_RX_DONE:
        gI2cControllerStatus = I2C_STATUS_RX_COMPLETE;
        break;
    case DL_I2C_IIDX_CONTROLLER_TX_DONE:
        DL_I2C_disableInterrupt(I2C_INST,
                                DL_I2C_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER);
        gI2cControllerStatus = I2C_STATUS_TX_COMPLETE;
        break;
    case DL_I2C_IIDX_CONTROLLER_RXFIFO_TRIGGER:
        gI2cControllerStatus = I2C_STATUS_RX_INPROGRESS;
        /* Receive all bytes from target */
        while (DL_I2C_isControllerRXFIFOEmpty(I2C_INST) != true) {
            if (gRxCount < gRxLen) {
                gRxPacket[gRxCount++] =
                    DL_I2C_receiveControllerData(I2C_INST);
            } else {
                /* Ignore and remove from FIFO if the buffer is full */
                DL_I2C_receiveControllerData(I2C_INST);
            }
        }
        break;
        /* Not used for this example */
    case DL_I2C_IIDX_CONTROLLER_ARBITRATION_LOST:
    case DL_I2C_IIDX_CONTROLLER_NACK:
        if ((gI2cControllerStatus == I2C_STATUS_RX_STARTED) ||
            (gI2cControllerStatus == I2C_STATUS_TX_STARTED))
        {
            /* NACK interrupt if I2C Target is disconnected */
            gI2cControllerStatus = I2C_STATUS_ERROR;
        }
    case DL_I2C_IIDX_CONTROLLER_RXFIFO_FULL:
    case DL_I2C_IIDX_CONTROLLER_TXFIFO_EMPTY:
    case DL_I2C_IIDX_CONTROLLER_START:
    case DL_I2C_IIDX_CONTROLLER_STOP:
    case DL_I2C_IIDX_CONTROLLER_EVENT1_DMA_DONE:
    case DL_I2C_IIDX_CONTROLLER_EVENT2_DMA_DONE:
    default:
        break;
    }
}
