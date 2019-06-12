#ifndef _ATMO_SHTI2C_H_
#define _ATMO_SHTI2C_H_

#include "../app_src/atmosphere_platform.h"
#include "../i2c/i2c.h"

typedef enum {
    ATMO_SHTI2C_Status_Success              = 0x00u,  // Common - Operation was successful
    ATMO_SHTI2C_Status_Fail                 = 0x01u,  // Common - Operation failed
    ATMO_SHTI2C_Status_Initialized          = 0x02u,  // Common - Peripheral already initialized
    ATMO_SHTI2C_Status_Invalid              = 0x03u,  // Common - Invalid operation or result
    ATMO_SHTI2C_Status_NotSupported         = 0x04u,  // Common - Feature not supported by platform
} ATMO_SHTI2C_Status_t;

typedef struct {
    ATMO_DriverInstanceHandle_t i2cDriverInstance;
} ATMO_SHTI2C_Config_t;

/**
 * Initialize SHTI2C Driver
 *
 * @param[in] config - Device configuration (optional)
 */
ATMO_SHTI2C_Status_t ATMO_SHTI2C_Init(ATMO_SHTI2C_Config_t *config);

/**
 * Set basic device configuration
 *
 * @param[in] config
 */
ATMO_SHTI2C_Status_t ATMO_SHTI2C_SetConfiguration(const ATMO_SHTI2C_Config_t *config);

/**
 * Get device configuration
 *
 * @param[out] config
 */
ATMO_SHTI2C_Status_t ATMO_SHTI2C_GetConfiguration(ATMO_SHTI2C_Config_t *config);

/**
 * Get Temperature in degrees celsius
 *
 * @param[out] temperature
 */
ATMO_SHTI2C_Status_t ATMO_SHTI2C_GetTemperature(float *temperatureCelsius);

/**
 * Get Relative Humidity in percent
 *
 * @param[out] relative humidity
 */
ATMO_SHTI2C_Status_t ATMO_SHTI2C_GetHumidity(float *humidity);


#endif
