    float humidity;
    ATMO_SHTI2C_GetHumidity(&humidity);
    ATMO_CreateValueFloat(out, humidity);
    return ATMO_Status_Success;
