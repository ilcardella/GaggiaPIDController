#include "CommonTest.h"

class TestSafetyTimeouts : public CommonTest
{
};

TEST_F(TestSafetyTimeouts, testWaterSafetyTimeout)
{
    // Mock healthy temp sensor, pid controller and a "low" temperature
    mode_detector.mode = Gaggia::Mode::WATER_MODE;
    water_sensor.temp_c = 10.0f;
    water_sensor.healthy = true;
    // Steam sensor should not be used in this mode
    steam_sensor.temp_c = 0.0f;
    steam_sensor.healthy = false;
    // Force the PID controller ashealthy
    pid.relay = true;
    pid.healthy = true;

    auto machine = make_machine();
    auto status = machine.spin();

    ASSERT_EQ(status.machine_mode, Gaggia::Mode::WATER_MODE);
    ASSERT_EQ(status.current_temperature, 10.0);
    ASSERT_EQ(status.target_temperature, Configuration::TARGET_WATER_TEMP);
    ASSERT_TRUE(status.water_heater_on);
    ASSERT_EQ(status.status_message, "Heating...");
    ASSERT_EQ(normalize_time(status.time_since_start), 0.0);
    ASSERT_EQ(normalize_time(status.time_since_steam_mode), 0.0);

    // Simulate getting at the limit of the safety timeout
    unsigned long exceeded_time = Configuration::SAFETY_TIMEOUT;
    Adapter::millis_ret = exceeded_time;
    status = machine.spin();

    ASSERT_EQ(status.machine_mode, Gaggia::Mode::WATER_MODE);
    ASSERT_EQ(status.current_temperature, 10.0);
    ASSERT_EQ(status.target_temperature, Configuration::TARGET_WATER_TEMP);
    ASSERT_TRUE(status.water_heater_on);
    ASSERT_EQ(status.status_message, "Heating...");
    ASSERT_EQ(normalize_time(status.time_since_start), 0.0);
    ASSERT_EQ(normalize_time(status.time_since_steam_mode), exceeded_time);

    // Simulate exceeding the safety timeout of 1 second
    exceeded_time += 1000;
    Adapter::millis_ret = exceeded_time;
    status = machine.spin();

    ASSERT_EQ(status.machine_mode, Gaggia::Mode::WATER_MODE);
    ASSERT_EQ(status.current_temperature, 10.0);
    ASSERT_EQ(status.target_temperature, Configuration::TARGET_WATER_TEMP);
    ASSERT_FALSE(status.water_heater_on);
    ASSERT_EQ(status.status_message, "Safety timeout expired");
    ASSERT_EQ(normalize_time(status.time_since_start), 0.0);
    ASSERT_EQ(normalize_time(status.time_since_steam_mode), exceeded_time);

    // Change to STEAM mode and verify the safety measures are still in effect
    mode_detector.mode = Gaggia::Mode::STEAM_MODE;
    water_sensor.temp_c = 0.0f;
    water_sensor.healthy = false;
    steam_sensor.temp_c = 10.0f;
    steam_sensor.healthy = true;

    exceeded_time += 1000;
    Adapter::millis_ret = exceeded_time;
    status = machine.spin();

    ASSERT_EQ(status.machine_mode, Gaggia::Mode::STEAM_MODE);
    ASSERT_EQ(status.current_temperature, 10.0);
    ASSERT_EQ(status.target_temperature, Configuration::TARGET_STEAM_TEMP);
    ASSERT_FALSE(status.water_heater_on);
    ASSERT_EQ(status.status_message, "Safety timeout expired");
    ASSERT_EQ(normalize_time(status.time_since_start), 0.0);
    // -1000 because the mode changed to steam 1 second ago
    ASSERT_EQ(normalize_time(status.time_since_steam_mode), exceeded_time - 1000);

    // Change back to WATER mode and verify again
    mode_detector.mode = Gaggia::Mode::WATER_MODE;
    water_sensor.temp_c = 10.0f;
    water_sensor.healthy = true;
    steam_sensor.temp_c = 0.0f;
    steam_sensor.healthy = false;

    exceeded_time += 1000;
    Adapter::millis_ret = exceeded_time;
    status = machine.spin();

    ASSERT_EQ(status.machine_mode, Gaggia::Mode::WATER_MODE);
    ASSERT_EQ(status.current_temperature, 10.0);
    ASSERT_EQ(status.target_temperature, Configuration::TARGET_WATER_TEMP);
    ASSERT_FALSE(status.water_heater_on);
    ASSERT_EQ(status.status_message, "Safety timeout expired");
    ASSERT_EQ(normalize_time(status.time_since_start), 0.0);
    ASSERT_EQ(normalize_time(status.time_since_steam_mode), exceeded_time);
}

TEST_F(TestSafetyTimeouts, testSteamSafetyTimeout)
{
    // Mock healthy temp sensor, pid controller and a "low" temperature
    mode_detector.mode = Gaggia::Mode::WATER_MODE;
    water_sensor.temp_c = 10.0f;
    water_sensor.healthy = true;
    // Steam sensor should not be used in this mode
    steam_sensor.temp_c = 0.0f;
    steam_sensor.healthy = false;
    // Force the PID controller ashealthy
    pid.relay = true;
    pid.healthy = true;

    auto machine = make_machine();
    auto status = machine.spin();

    ASSERT_EQ(status.machine_mode, Gaggia::Mode::WATER_MODE);
    ASSERT_EQ(status.current_temperature, 10.0);
    ASSERT_EQ(status.target_temperature, Configuration::TARGET_WATER_TEMP);
    ASSERT_TRUE(status.water_heater_on);
    ASSERT_EQ(status.status_message, "Heating...");
    ASSERT_EQ(normalize_time(status.time_since_start), 0.0);
    ASSERT_EQ(normalize_time(status.time_since_steam_mode), 0.0);

    // Change mode to STEAM
    mode_detector.mode = Gaggia::Mode::STEAM_MODE;
    water_sensor.temp_c = 0.0f;
    water_sensor.healthy = false;
    steam_sensor.temp_c = 10.0f;
    steam_sensor.healthy = true;

    // Simulate getting at the limit of the STEAM safety timeout
    unsigned long exceeded_time = Configuration::STEAM_TIMEOUT;
    Adapter::millis_ret = exceeded_time;
    status = machine.spin();

    ASSERT_EQ(status.machine_mode, Gaggia::Mode::STEAM_MODE);
    ASSERT_EQ(status.current_temperature, 10.0);
    ASSERT_EQ(status.target_temperature, Configuration::TARGET_STEAM_TEMP);
    ASSERT_TRUE(status.water_heater_on);
    ASSERT_EQ(status.status_message, "Heating...");
    ASSERT_EQ(normalize_time(status.time_since_start), 0.0);
    // 0.0 because last time in water mode was at creation
    ASSERT_EQ(normalize_time(status.time_since_steam_mode), 0.0);

    // Exceed the steam timeout by one second
    exceeded_time += 1000;
    Adapter::millis_ret = exceeded_time;
    status = machine.spin();

    // Verify the heater is turned off
    ASSERT_EQ(status.machine_mode, Gaggia::Mode::STEAM_MODE);
    ASSERT_EQ(status.current_temperature, 10.0);
    ASSERT_EQ(status.target_temperature, Configuration::TARGET_STEAM_TEMP);
    ASSERT_FALSE(status.water_heater_on);
    ASSERT_EQ(status.status_message, "Steam mode timeout expired");
    ASSERT_EQ(normalize_time(status.time_since_start), 0.0);
    // 0.0 because last time in water mode was at creation
    ASSERT_EQ(normalize_time(status.time_since_steam_mode), 0.0);

    // Change back to WATER mode and verify the timeout is reset
    mode_detector.mode = Gaggia::Mode::WATER_MODE;
    water_sensor.temp_c = 10.0f;
    water_sensor.healthy = true;
    steam_sensor.temp_c = 0.0f;
    steam_sensor.healthy = false;

    exceeded_time += 1000;
    Adapter::millis_ret = exceeded_time;
    status = machine.spin();

    ASSERT_EQ(status.machine_mode, Gaggia::Mode::WATER_MODE);
    ASSERT_EQ(status.current_temperature, 10.0);
    ASSERT_EQ(status.target_temperature, Configuration::TARGET_WATER_TEMP);
    ASSERT_TRUE(status.water_heater_on);
    ASSERT_EQ(status.status_message, "Heating...");
    ASSERT_EQ(normalize_time(status.time_since_start), 0.0);
    ASSERT_EQ(normalize_time(status.time_since_steam_mode), exceeded_time);

    // Change mode to STEAM again and verify it's working ok
    mode_detector.mode = Gaggia::Mode::STEAM_MODE;
    water_sensor.temp_c = 0.0f;
    water_sensor.healthy = false;
    steam_sensor.temp_c = 10.0f;
    steam_sensor.healthy = true;

    exceeded_time += 1000;
    Adapter::millis_ret = exceeded_time;
    status = machine.spin();

    ASSERT_EQ(status.machine_mode, Gaggia::Mode::STEAM_MODE);
    ASSERT_EQ(status.current_temperature, 10.0);
    ASSERT_EQ(status.target_temperature, Configuration::TARGET_STEAM_TEMP);
    ASSERT_TRUE(status.water_heater_on);
    ASSERT_EQ(status.status_message, "Heating...");
    ASSERT_EQ(normalize_time(status.time_since_start), 0.0);
    ASSERT_EQ(normalize_time(status.time_since_steam_mode), exceeded_time - 1000);
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}