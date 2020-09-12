#pragma once

#include "Configuration.h"

#include "max6675.h"
#include <TSIC.h>

namespace sensors
{
class KTypeThermocouple
{
  public:
    KTypeThermocouple(const uint8_t &pin)
        : sensor(Configuration::SPI_CLK_PIN, pin, Configuration::SPI_DO_PIN)
    {
    }

    bool read_sensor(float *value)
    {
        float reading = sensor.readCelsius();
        if (reading == NAN)
        {
            return false;
        }
        *value = reading;
        return true;
    }

  private:
    MAX6675 sensor;
};

class TSICTempSensor
{
  public:
    TSICTempSensor(const uint32_t &pin) : sensor(pin, NO_VCC_PIN, TSIC_30x)
    {
        pinMode(pin, INPUT);
    }

    bool read_sensor(float *value)
    {
        uint16_t raw;
        if (not sensor.getTemperature(&raw))
        {
            return false;
        }
        // FIXME Occasionally the raw value spikes to 60k+ or other times
        // it drops to ~ 0. For this project it should be safe to constraint
        // the raw reads between safety margins to ignore these errors.
        // At 23C ambient temp the raw value is around 730
        if (raw < 600 || raw > 900)
        {
            return false;
        }

        auto temp = sensor.calc_Celsius(&raw);
        // FIXME: Same type of check as explained above
        if (temp < 10 || temp > 200)
        {
            return false;
        }

        *value = temp;
        return true;
    }

  private:
    TSIC sensor;
};
} // namespace sensors
