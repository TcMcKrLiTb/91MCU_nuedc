#include "TPL0501.h"

// #define CHIP_NAME                 "Texas Instruments TPL0501"        /**<
// chip name */ #define MANUFACTURER_NAME         "Texas Instruments" /**<
// manufacturer name */ #define SUPPLY_VOLTAGE_MIN        2.7f /**< chip min
// supply voltage */ #define SUPPLY_VOLTAGE_MAX        5.5f /**< chip max supply
// voltage */ #define MAX_CURRENT               5.0f /**< chip max current */
// #define TEMPERATURE_MIN           -40.0f                             /**<
// chip min operating temperature */ #define TEMPERATURE_MAX           125.0f
// /**< chip max operating temperature */ #define DRIVER_VERSION            1000
// /**< driver version */

/**
 * @brief     write byte
 * @param[in] *handle pointer to a tpl0501 handle structure
 * @param[in] data set data
 * @return    status code
 *            - 0 success
 *            - 1 spi write failed
 * @note      none
 */
static uint8_t a_tpl0501_spi_write(tpl0501_handle_t *handle, uint8_t data)
{
    if (handle->spi_write_cmd(&data, 1) != 0) /* spi write */
    {
        return 1; /* return error */
    }
    return 0; /* success return 0 */
}

/**
 * @brief     initialize the chip
 * @param[in] *handle pointer to a tpl0501 handle structure
 * @return    status code
 *            - 0 success
 *            - 1 spi initialization failed
 *            - 2 handle is NULL
 *            - 3 linked functions is NULL
 * @note      none
 */
uint8_t tpl0501_init(tpl0501_handle_t *handle)
{
    if (handle == NULL) /* check handle */
    {
        return 2; /* return error */
    }
    if (handle->spi_init == NULL) /* check spi_init */
    {
        return 3; /* return error */
    }
    if (handle->spi_write_cmd == NULL) /* check spi_write_cmd */
    {
        return 5; /* return error */
    }
    if (handle->delay_ms == NULL) /* check delay_ms */
    {
        return 6; /* return error */
    }
    if (handle->spi_init() != 0) /* spi init */
    {
        return 1; /* return error */
    }
    handle->inited = 1; /* flag finish initialization */
    return 0;           /* success return 0 */
}

/**
 * @brief     write data
 * @param[in] *handle pointer to a tpl0501 handle structure
 * @param[in] raw set data
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 *            - 2 handle is NULL
 *            - 3 handle is not initialized
 * @note      none
 */
uint8_t tpl0501_write(tpl0501_handle_t *handle, uint8_t raw)
{
    uint8_t res;
    if (handle == NULL) /* check handle */
    {
        return 2; /* return error */
    }
    if (handle->inited != 1) /* check handle initialization */
    {
        return 3; /* return error */
    }
    res = a_tpl0501_spi_write(handle, raw); /* set data */
    if (res != 0)                           /* check result */
    {
        return 1; /* return error */
    }
    return 0; /* success return 0 */
}

/**
 * @brief      convert the percentage to the register raw data
 * @param[in]  *handle pointer to a tpl0501 handle structure
 * @param[in]  percentage set percentage
 * @param[out] *reg pointer to a register raw buffer
 * @param[out] *wl_ohm pointer to a wl ohm buffer
 * @param[out] *hw_ohm pointer to a hw ohm buffer
 * @return     status code
 *             - 0 success
 *             - 2 handle is NULL
 *             - 3 handle is not initialized
 * @note       none
 */
uint8_t tpl0501_percentage_convert_to_register(tpl0501_handle_t *handle,
                                               float percentage, uint8_t *reg,
                                               float *wl_ohm, float *hw_ohm)
{
    if (handle == NULL) /* check handle */
    {
        return 2; /* return error */
    }
    if (handle->inited != 1) /* check handle initialization */
    {
        return 3; /* return error */
    }
    *reg = (uint8_t)(percentage / 100.0f *
                     256); /* convert real data to register data */
    *wl_ohm = 100 * 1000 * (percentage / 100.0f); /* set wl ohm */
    *hw_ohm = 100 * 1000 - (*wl_ohm);             /* set hw ohm */
    return 0;                                     /* success return 0 */
}

/**
 * @brief      convert the register raw data to percentage
 * @param[in]  *handle pointer to a tpl0501 handle structure
 * @param[in]  reg register raw data
 * @param[out] *percentage pointer to a percentage buffer
 * @param[out] *wl_ohm pointer to a wl ohm buffer
 * @param[out] *hw_ohm pointer to a hw ohm buffer
 * @return     status code
 *             - 0 success
 *             - 2 handle is NULL
 *             - 3 handle is not initialized
 * @note       none
 */
uint8_t tpl0501_percentage_convert_to_data(tpl0501_handle_t *handle,
                                           uint8_t reg, float *percentage,
                                           float *wl_ohm, float *hw_ohm)
{
    if (handle == NULL) /* check handle */
    {
        return 2; /* return error */
    }
    if (handle->inited != 1) /* check handle initialization */
    {
        return 3; /* return error */
    }
    *percentage = (float)((float)(reg) / 256.0f) *
                  100.0f; /* convert register data to real data */
    *wl_ohm = 100 * 1000 * ((float)reg / 256.0f); /* set wl ohm */
    *hw_ohm = 100 * 1000 - (*wl_ohm);             /* set hw ohm */
    return 0; /* success return 0 */
}

/**
 * @brief     set the chip register
 * @param[in] *handle pointer to a tpl0501 handle structure
 * @param[in] raw set data
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 *            - 2 handle is NULL
 *            - 3 handle is not initialized
 * @note      none
 */
uint8_t tpl0501_set_reg(tpl0501_handle_t *handle, uint8_t raw)
{
    if (handle == NULL) /* check handle */
    {
        return 2; /* return error */
    }
    if (handle->inited != 1) /* check handle initialization */
    {
        return 3; /* return error */
    }
    return a_tpl0501_spi_write(handle, raw); /* write data */
}
