    float tempC;
    ATMO_SHTI2C_GetTemperature(&tempC);
    ATMO_CreateValueFloat(out, tempC);
    return ATMO_Status_Success;
