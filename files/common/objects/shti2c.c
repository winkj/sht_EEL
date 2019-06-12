// This is an adaption of:
//   https://github.com/Sensirion/arduino-ess/blob/master/sensirion_ess.cpp
//
// Based on IR Thermoclick example
//
// Author: Johannes Winkelmann, jw@smts.ch
//

#include "shti2c.h"

#define SHT_ADDR (0x70)
#define SHT_CMD_LENGTH (2)
#define SHT_DATA_LENGTH (6)

#define SHT_UPDATE_THRESHOLD_MS 800

#define SHT_MEASUREMENT_DELAY 15

static uint64_t sLastSHTUpdateMs = 0;

static float sHumidityCache = 0;
static float sTemperatureCache = 0;

typedef struct {
    ATMO_SHTI2C_Config_t config;
    bool configured;
} ATMO_SHTI2C_Priv_Config;

static ATMO_SHTI2C_Priv_Config _ATMO_SHTI2C_config;

static ATMO_I2C_Peripheral_t _ATMO_SHTI2C_i2cConfig = {
    .operatingMode = ATMO_I2C_OperatingMode_Master,
    .baudRate = ATMO_I2C_BaudRate_Standard_Mode
};

ATMO_SHTI2C_Status_t ATMO_SHTI2C_updateSHT_Internal();

ATMO_SHTI2C_Status_t ATMO_SHTI2C_Init(ATMO_SHTI2C_Config_t *config)
{
    if (config) { // Did the user supply a configuration?
        ATMO_SHTI2C_SetConfiguration(config);
    } else {
        _ATMO_SHTI2C_config.configured = false;
    }

    // initial calls to update function to set lastUpdate variables
    ATMO_SHTI2C_updateSHT_Internal();

    return ATMO_SHTI2C_Status_Success;
}

ATMO_SHTI2C_Status_t ATMO_SHTI2C_SetConfiguration(const ATMO_SHTI2C_Config_t *config)
{
    if (config == NULL) {
        return ATMO_SHTI2C_Status_Fail;
    }

    if (ATMO_I2C_SetConfiguration(config->i2cDriverInstance, &_ATMO_SHTI2C_i2cConfig) != ATMO_I2C_Status_Success) {
        return ATMO_SHTI2C_Status_Fail;
    }
    memcpy( &_ATMO_SHTI2C_config.config, config, sizeof(ATMO_SHTI2C_Config_t) );
    _ATMO_SHTI2C_config.configured = true;

    return ATMO_SHTI2C_Status_Success;
}

ATMO_SHTI2C_Status_t ATMO_SHTI2C_GetConfiguration(ATMO_SHTI2C_Config_t *config)
{
    if (config == NULL || !_ATMO_SHTI2C_config.configured) {
        return ATMO_SHTI2C_Status_Fail;
    }

    memcpy(config, &_ATMO_SHTI2C_config.config, sizeof(ATMO_SHTI2C_Config_t));
    return ATMO_SHTI2C_Status_Success;
}

ATMO_SHTI2C_Status_t ATMO_SHTI2C_ReadData_Internal(uint8_t addr, uint8_t* cmd, uint16_t cmdLength,
          uint8_t* data, uint16_t dataLength, uint16_t measurementDelay)
{
  ATMO_I2C_Status_t readStatus;
  readStatus = ATMO_I2C_MasterWrite(_ATMO_SHTI2C_config.config.i2cDriverInstance,
                                    addr, cmd, cmdLength,
                                    NULL, 0,
                                    0);
  if (readStatus != ATMO_I2C_Status_Success) {
      return ATMO_SHTI2C_Status_Fail;
  }

  ATMO_DelayMillisecondsNonBlock(measurementDelay);

  readStatus = ATMO_I2C_MasterRead(_ATMO_SHTI2C_config.config.i2cDriverInstance,
                                  addr, NULL, 0, data, dataLength, 0);
  if (readStatus != ATMO_I2C_Status_Success) {
      return ATMO_SHTI2C_Status_Fail;
  }

  return ATMO_SHTI2C_Status_Success;
}

uint8_t ATMO_SHTI2C_CheckCrc_Internal(const uint8_t* data, uint8_t len)
{
    // adapted from SHT21 sample code from http://www.sensirion.com/en/products/humidity-temperature/download-center/

    uint8_t crc = 0xff;
    uint8_t byteCtr;
    for (byteCtr = 0; byteCtr < len; ++byteCtr) {
        crc ^= (data[byteCtr]);
        for (uint8_t bit = 8; bit > 0; --bit) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}

// - SHT
ATMO_SHTI2C_Status_t ATMO_SHTI2C_updateSHT_Internal()
{
  uint64_t now = ATMO_PLATFORM_UptimeMs();
  if ((now - sLastSHTUpdateMs) < SHT_UPDATE_THRESHOLD_MS) {
    // reuse cached values
    return ATMO_SHTI2C_Status_Success;
  }
  sLastSHTUpdateMs = now;

  if (!_ATMO_SHTI2C_config.configured) {
      return ATMO_SHTI2C_Status_Fail;
  }

  uint8_t data[SHT_DATA_LENGTH] = { 0 };
  uint8_t cmd[SHT_CMD_LENGTH] = { 0x78, 0x66 };
  if (ATMO_SHTI2C_ReadData_Internal(SHT_ADDR, cmd,
                                    SHT_CMD_LENGTH, data,
                                    SHT_DATA_LENGTH,
                                    SHT_MEASUREMENT_DELAY) != ATMO_SHTI2C_Status_Success) {
    return ATMO_SHTI2C_Status_Fail;
  }

  if (ATMO_SHTI2C_CheckCrc_Internal(data+0, 2) != data[2] ||
      ATMO_SHTI2C_CheckCrc_Internal(data+3, 2) != data[5]) {
    return ATMO_SHTI2C_Status_Fail;
  }

  //  mTemperature = mA + mB * (val / mC);
  uint16_t rawTemp = (data[0] << 8) | data[1];
  sTemperatureCache = (float)(-45 + (175.0 * rawTemp / 65535.0));

  //  mHumidity = mX * (val / mY);
  uint16_t rawHumidity = (data[3] << 8) | data[4];
  sHumidityCache = (float)(100.0 * rawHumidity / 65535.0);

  return ATMO_SHTI2C_Status_Success;
}

ATMO_SHTI2C_Status_t ATMO_SHTI2C_GetTemperature(float *temperatureCelsius)
{
  if (ATMO_SHTI2C_updateSHT_Internal() != ATMO_SHTI2C_Status_Success) {
    return ATMO_SHTI2C_Status_Fail;
  }
  *temperatureCelsius = sTemperatureCache;

  return ATMO_SHTI2C_Status_Success;
}


ATMO_SHTI2C_Status_t ATMO_SHTI2C_GetHumidity(float *humidity)
{
  if (ATMO_SHTI2C_updateSHT_Internal() != ATMO_SHTI2C_Status_Success) {
    return ATMO_SHTI2C_Status_Fail;
  }
  *humidity = sHumidityCache;

  return ATMO_SHTI2C_Status_Success;
}
