#pragma once

#include "interfaces.h"

template <class Adapter, class SensorType>
class TemperatureSensor : public BaseTemperatureSensor
{
  public:
    TemperatureSensor(const char *name, const unsigned char &pin,
                      const unsigned long read_period,
                      const unsigned int &moving_avg_size = 1,
                      const float &temp_offset = 0)
        : name(name), m_avg(moving_avg_size), time_last_read(Adapter::millis()),
          healthy(true), read_period(read_period), temp_offset(temp_offset), sensor(pin)
    {
    }

    char *get_name() override
    {
        return name;
    }

    bool get_temperature_celsius(float *value) override
    {
        unsigned long now = Adapter::millis();
        if (now - time_last_read > read_period)
        {
            time_last_read = now;
            float reading;
            if (not sensor.read_sensor(&reading))
            {
                healthy = false;
                return false;
            }
            reading += temp_offset;

            healthy = true;
            m_avg.add(reading);
        }
        *value = m_avg.get();
        return healthy;
    }

  private:
    SensorType sensor;
    char *name;
    unsigned long time_last_read;
    unsigned long read_period;
    MovingAverage<float> m_avg;
    bool healthy;
    float temp_offset;
};