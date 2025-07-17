#ifndef __AD5933_H__
#define __AD5933_H__

#include "ti_msp_dl_config.h"
#include "stdint.h"

// Device address and address pointer
#define AD5933_ADDR     (0x0D)
#define ADDR_PTR        (0xB0)
// Control Register
#define CTRL_REG1       (0x80)
#define CTRL_REG2       (0x81)
// Start Frequency Register
#define START_FREQ_1    (0x82)
#define START_FREQ_2    (0x83)
#define START_FREQ_3    (0x84)
// Frequency increment register
#define INC_FREQ_1      (0x85)
#define INC_FREQ_2      (0x86)
#define INC_FREQ_3      (0x87)
// Number of increments register
#define NUM_INC_1       (0x88)
#define NUM_INC_2       (0x89)
// Number of settling time cycles register
#define NUM_SCYCLES_1   (0x8A)
#define NUM_SCYCLES_2   (0x8B)
// Status register
#define STATUS_REG      (0x8F)
// Temperature data register
#define TEMP_DATA_1     (0x92)
#define TEMP_DATA_2     (0x93)
// Real data register
#define REAL_DATA_1     (0x94)
#define REAL_DATA_2     (0x95)
// Imaginary data register
#define IMAG_DATA_1     (0x96)
#define IMAG_DATA_2     (0x97)

// Temperature measuring
#define TEMP_MEASURE    (CTRL_TEMP_MEASURE)
#define TEMP_NO_MEASURE (CTRL_NO_OPERATION)
// Clock sources
#define CLOCK_INTERNAL  (CTRL_CLOCK_INTERNAL)
#define CLOCK_EXTERNAL  (CTRL_CLOCK_EXTERNAL)
// PGA gain options
#define PGA_GAIN_X1     (CTRL_PGA_GAIN_X1)
#define PGA_GAIN_X5     (CTRL_PGA_GAIN_X5)
// Power modes
#define POWER_STANDBY   (CTRL_STANDBY_MODE)
#define POWER_DOWN      (CTRL_POWER_DOWN_MODE)
#define POWER_ON        (CTRL_NO_OPERATION)
// I2C result success/fail
#define I2C_RESULT_SUCCESS       (0)
#define I2C_RESULT_DATA_TOO_LONG (1)
#define I2C_RESULT_ADDR_NAK      (2)
#define I2C_RESULT_DATA_NAK      (3)
#define I2C_RESULT_OTHER_FAIL    (4)
// Control output voltage range options
#define CTRL_OUTPUT_RANGE_1		(0b00000000)
#define CTRL_OUTPUT_RANGE_2		(0b00000110)
#define CTRL_OUTPUT_RANGE_3		(0b00000100)
#define CTRL_OUTPUT_RANGE_4		(0b00000010)
// Control register options
#define CTRL_NO_OPERATION       (0b00000000)
#define CTRL_INIT_START_FREQ    (0b00010000)
#define CTRL_START_FREQ_SWEEP   (0b00100000)
#define CTRL_INCREMENT_FREQ     (0b00110000)
#define CTRL_REPEAT_FREQ        (0b01000000)
#define CTRL_TEMP_MEASURE       (0b10010000)
#define CTRL_POWER_DOWN_MODE    (0b10100000)
#define CTRL_STANDBY_MODE       (0b10110000)
#define CTRL_RESET              (0b00010000)
#define CTRL_CLOCK_EXTERNAL     (0b00001000)
#define CTRL_CLOCK_INTERNAL     (0b00000000)
#define CTRL_PGA_GAIN_X1        (0b00000001)
#define CTRL_PGA_GAIN_X5        (0b00000000)
// Status register options
#define STATUS_TEMP_VALID       (0x01)
#define STATUS_DATA_VALID       (0x02)
#define STATUS_SWEEP_DONE       (0x04)
#define STATUS_ERROR            (0xFF)
// Frequency sweep parameters
#define SWEEP_DELAY             (1)

bool AD5933_Reset(void);
bool AD5933_EnableTemperature(uint8_t enable);
float AD5933_GetTemperature(void);
bool AD5933_SetClockSource(uint8_t source);
bool AD5933_SetInternalClock(uint8_t internal);
bool AD5933_setSettlingCycles(int32_t time);
bool AD5933_SetStartFrequency(uint32_t start);
bool AD5933_SetIncrementFrequency(uint32_t increment);
bool AD5933_SetNumberIncrements(uint32_t num);
bool AD5933_SetPGAGain(uint8_t gain);
uint8_t AD5933_ReadRegister(uint8_t reg);
bool AD5933_SetRange(uint8_t range);
uint8_t AD5933_ReadStatusRegister(void);
int32_t AD5933_ReadControlRegister(void);
bool AD5933_GetComplexData(int32_t *real, int32_t *imag);
bool AD5933_SetPowerMode(uint8_t level);
bool AD5933_FrequencySweep(int32_t *real, int32_t *imag, int32_t n);
bool AD5933_Calibrate(float *gain, int32_t *phase, int32_t ref, int32_t n);
bool AD5933_Calibrate2(float *gain, int32_t *phase, int32_t *real, int32_t *imag, int32_t ref, int32_t n);

#endif
