#ifndef __TPL0501_H__
#define __TPL0501_H__

#include "ti_msp_dl_config.h"


typedef struct 
{
    uint8_t (*spi_init)(void); /**< point to a spi_init function address */
    uint8_t (*spi_write_cmd)(
        uint8_t *buf,
        uint16_t len); /**< point to a spi_write_cmd function address */
    void (*delay_ms)(uint32_t ms); /**< point to a delay_ms function address */
    uint8_t inited;                /**< inited flag */
} tpl0501_handle_t;

// /**
//  * @brief tpl0501 information structure definition
//  */
// typedef struct tpl0501_info_s
// {
//     char chip_name[32];         /**< chip name */
//     char manufacturer_name[32]; /**< manufacturer name */
//     char interface[8];          /**< chip interface name */
//     float supply_voltage_min_v; /**< chip min supply voltage */
//     float supply_voltage_max_v; /**< chip max supply voltage */
//     float max_current_ma;       /**< chip max current */
//     float temperature_min;      /**< chip min operating temperature */
//     float temperature_max;      /**< chip max operating temperature */
//     uint32_t driver_version;    /**< driver version */
// } tpl0501_info_t;

/**
 * @}
 */

/**
 * @defgroup tpl0501_link_driver tpl0501 link driver function
 * @brief    tpl0501 link driver modules
 * @ingroup  tpl0501_driver
 * @{
 */

/**
 * @brief     initialize tpl0501_handle_t structure
 * @param[in] HANDLE pointer to a tpl0501 handle structure
 * @param[in] STRUCTURE tpl0501_handle_t
 * @note      none
 */
#define DRIVER_TPL0501_LINK_INIT(HANDLE, STRUCTURE)                            \
    memset(HANDLE, 0, sizeof(STRUCTURE))

/**
 * @brief     link spi_init function
 * @param[in] HANDLE pointer to a tpl0501 handle structure
 * @param[in] FUC pointer to a spi_init function address
 * @note      none
 */
#define DRIVER_TPL0501_LINK_SPI_INIT(HANDLE, FUC) (HANDLE)->spi_init = FUC

/**
 * @brief     link spi_deinit function
 * @param[in] HANDLE pointer to a tpl0501 handle structure
 * @param[in] FUC pointer to a spi_deinit function address
 * @note      none
 */
#define DRIVER_TPL0501_LINK_SPI_DEINIT(HANDLE, FUC) (HANDLE)->spi_deinit = FUC

/**
 * @brief     link spi_write_cmd function
 * @param[in] HANDLE pointer to a tpl0501 handle structure
 * @param[in] FUC pointer to a spi_write_cmd function address
 * @note      none
 */
#define DRIVER_TPL0501_LINK_SPI_WRITE_COMMAND(HANDLE, FUC)                     \
    (HANDLE)->spi_write_cmd = FUC

/**
 * @brief     link delay_ms function
 * @param[in] HANDLE pointer to a tpl0501 handle structure
 * @param[in] FUC pointer to a delay_ms function address
 * @note      none
 */
#define DRIVER_TPL0501_LINK_DELAY_MS(HANDLE, FUC) (HANDLE)->delay_ms = FUC

/**
 * @brief     link debug_print function
 * @param[in] HANDLE pointer to a tpl0501 handle structure
 * @param[in] FUC pointer to a debug_print function address
 * @note      none
 */
#define DRIVER_TPL0501_LINK_DEBUG_PRINT(HANDLE, FUC) (HANDLE)->debug_print = FUC

/**
 * @}
 */

/**
 * @defgroup tpl0501_base_driver tpl0501 base driver function
 * @brief    tpl0501 base driver modules
 * @ingroup  tpl0501_driver
 * @{
 */

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
uint8_t tpl0501_init(tpl0501_handle_t *handle);

/**
 * @brief     close the chip
 * @param[in] *handle pointer to a tpl0501 handle structure
 * @return    status code
 *            - 0 success
 *            - 1 deinit failed
 *            - 2 handle is NULL
 *            - 3 handle is not initialized
 * @note      none
 */
uint8_t tpl0501_deinit(tpl0501_handle_t *handle);

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
uint8_t tpl0501_write(tpl0501_handle_t *handle, uint8_t raw);

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
                                               float *wl_ohm, float *hw_ohm);

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
                                           float *wl_ohm, float *hw_ohm);

/**
 * @}
 */

#endif
