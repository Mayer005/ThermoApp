#include <cmath>

#include "sensormanager.hpp"

extern "C" {
    #include "app_log.h"
    #include "sl_sensor_rht.h"
    #include "sl_health_thermometer.h"
    #include "sl_mic.h"
    #include "sl_status.h"
    #include "sl_imu.h"
    #include "sl_icm20689.h"
}

SensorManager::SensorManager() : temperature(0), humidity(0), noiseLevel(0), steps(0) {}

SensorManager::~SensorManager() {}

void SensorManager::init() {
    // Initialize sensors
    sl_status_t sc;
    // Init temperature sensor.
    sc = sl_sensor_rht_init();
    if (sc != SL_STATUS_OK) {
        app_log_warning("Relative Humidity and Temperature sensor initialization failed [0x%04lx]" APP_LOG_NL, sc);
    }
    app_log_info("Health thermometer initialized" APP_LOG_NL);

    sc = sl_mic_init();
    if (sc != SL_STATUS_OK) {
        app_log_warning("Microphone initialization failed [0x%04lx]" APP_LOG_NL, sc);
    }
    app_log_info("Microphone initialized" APP_LOG_NL);

    sc = sl_imu_init();
    if (sc != SL_STATUS_OK) {
        app_log_warning("IMU initialization failed [0x%04lx]" APP_LOG_NL, sc);
    }
    app_log_info("IMU initialized" APP_LOG_NL);

    sc = sl_icm20689_init();
    if (sc != SL_STATUS_OK) {
        app_log_warning("ICM20689 initialization failed [0x%04lx]" APP_LOG_NL, sc);
    }
    app_log_info("ICM20689 initialized" APP_LOG_NL);

}

void SensorManager::update() {
    // Measure temperature; units are % and milli-Celsius.
    sl_status_t sc;
    int32_t temperature = 0;
    uint32_t humidity = 0;
    sc = sl_sensor_rht_get(&humidity, &temperature);
    if (SL_STATUS_NOT_INITIALIZED == sc) {
      app_log_info("Relative Humidity and Temperature sensor is not initialized" APP_LOG_NL);
    } else if (sc != SL_STATUS_OK) {
      app_log_warning("Invalid RHT reading: %lu %ld" APP_LOG_NL, humidity, temperature);
    }
    this->temperature = static_cast<int16_t>(temperature/10); //it is in milli-Celsius by raw. It means 35000->35.000C. Firstly I will divide it only by 10 so it will be 3500->35.00C and the android app will be responsible to divide it by 100 to get the real value. MAX = 327.67C (it is more than enough, if there is a fire in the room, it will burn anyway..)
    this->humidity = static_cast<uint8_t>(humidity/1000); //uint32_t to uint8_t, so whole numbers as precentages, should be fine :)) max 255% (more than 100% so no problem here)

    uint64_t sum;
    sc = sl_mic_get_n_samples(buffer, n_samples); // magic will happen (RMS)
    if (SL_STATUS_NOT_INITIALIZED == sc) {
      app_log_info("Microphone sensor is not initialized" APP_LOG_NL);
    } else if (sc != SL_STATUS_OK) {
      app_log_warning("Invalid MIC reading: %lu %ld" APP_LOG_NL, humidity, temperature);
      this->noiseLevel = 0;
    }
    for(auto& sample: buffer) {
        int32_t val = static_cast<int32_t>(sample);
        sum += val * val; // sum the power of all samples
    }
    float rms = std::sqrt(sum / MIC_SAMPLE_BUFFER_SIZE);
    if (rms > 0) {
        float raw_db = 20.0f * std::log10(rms); // This will be between 0-90

        float real_db_spl = raw_db + 29.7f; // calibration offset
        if (real_db_spl < 0) real_db_spl = 0;

        this->noiseLevel = static_cast<uint8_t>(real_db_spl);
    }

    /// TODO: Accel 
}

int16_t SensorManager::getTemperature() const {
    return temperature;
}

uint8_t SensorManager::getHumidity() const {
    return humidity;
}

uint8_t SensorManager::getNoiseLevel() const {
    return noiseLevel;
}