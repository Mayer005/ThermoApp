#include <cmath>

#include "sensormanager.hpp"
#include "types.hpp"

#ifdef __cplusplus
extern "C" {
#endif
    #include "sl_bt_api.h"
    #include "app_log.h"
    #include "sl_sensor_rht.h"
    #include "sl_health_thermometer.h"
    #include "sl_mic.h"
    #include "sl_status.h"
    #include "sl_imu.h"
    #include "sl_icm20689.h"
#ifdef __cplusplus
}
#endif

SensorManager::SensorManager() : moving(false), temperature(0), humidity(0), noiseLevel(0) {}

SensorManager::~SensorManager() {}

void SensorManager::init() {
    sl_status_t sc;

    sc = sl_sensor_rht_init();
    if (sc != SL_STATUS_OK) {
        app_log_warning("[SensorManager] Relative Humidity and Temperature sensor initialization failed [0x%04lx]" APP_LOG_NL, sc);
    }
    app_log_info("[SensorManager] Health thermometer initialized" APP_LOG_NL);

    //sc = sl_mic_init(16000, MIC_N_CHANNELS);
    if (sc != SL_STATUS_OK) {
        app_log_warning("[SensorManager] Microphone initialization failed [0x%04lx]" APP_LOG_NL, sc);
    }
    app_log_info("[SensorManager] Microphone initialized" APP_LOG_NL);

    sc = sl_imu_init();
    if (sc != SL_STATUS_OK) {
        app_log_warning("[SensorManager] IMU initialization failed [0x%04lx]" APP_LOG_NL, sc);
    }
    app_log_info("[SensorManager] IMU initialized" APP_LOG_NL);

    sc = sl_icm20689_init();
    if (sc != SL_STATUS_OK) {
        app_log_warning("[SensorManager] ICM20689 initialization failed [0x%04lx]" APP_LOG_NL, sc);
    }
    app_log_info("[SensorManager]  ICM20689 initialized" APP_LOG_NL);

}

void SensorManager::reset() {
    temperature = 0;
    humidity = 0;
    noiseLevel = 0;
    moving = false;
}

void SensorManager::update() {
    // Measure temperature; units are % and milli-Celsius.
    sl_status_t sc;
    int32_t temperature = 0;
    uint32_t humidity = 0;
    sc = sl_sensor_rht_get(&humidity, &temperature);

    #ifdef DEBUG
    app_log_info("[SensorManager] RHT raw data: Humidity: %lu, Temperature: %ld" APP_LOG_NL, humidity, temperature);
    #endif

    if (SL_STATUS_NOT_INITIALIZED == sc) {
      app_log_info("[SensorManager] Relative Humidity and Temperature sensor is not initialized" APP_LOG_NL);
    } else if (sc != SL_STATUS_OK) {
      app_log_warning("[SensorManager] Invalid RHT reading: %lu %ld" APP_LOG_NL, humidity, temperature);
    }

    this->temperature = static_cast<int16_t>(temperature/10); //it is in milli-Celsius by raw. It means 35000->35.000C. Firstly I will divide it only by 10 so it will be 3500->35.00C and the android app will be responsible to divide it by 100 to get the real value. MAX = 327.67C (it is more than enough, if there is a fire in the room, it will burn anyway..)
    this->humidity = static_cast<uint8_t>(humidity/1000); //uint32_t to uint8_t, so whole numbers as precentages, should be fine :)) max 255% (more than 100% so no problem here)

    uint64_t sum = 0;
    sc = sl_mic_get_n_samples(buffer, n_samples); // magic will happen (RMS)
    #ifdef DEBUG
    app_log_info("[SensorManager] MIC sample buffer filled with %lu frames" APP_LOG_NL, n_samples);
    #endif

    if (SL_STATUS_NOT_INITIALIZED == sc) {
      app_log_info("[SensorManager] Microphone sensor is not initialized" APP_LOG_NL);
    } else if (sc != SL_STATUS_OK) {
      app_log_warning("[SensorManager] Invalid MIC reading: %lu %ld" APP_LOG_NL, humidity, temperature);
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
    } else {
        this->noiseLevel = 0;
    }

    float accel_data[3];
    sc = sl_icm20689_accel_read_data(accel_data);
    #ifdef DEBUG
    app_log_info("[SensorManager] ICM20689 raw accel data: X: %f, Y: %f, Z: %f" APP_LOG_NL, accel_data[0], accel_data[1], accel_data[2]);
    #endif

    if (SL_STATUS_NOT_INITIALIZED == sc) {
        app_log_info("[SensorManager] ICM20689 sensor is not initialized" APP_LOG_NL);
        this->moving = false; 
    } else if (sc != SL_STATUS_OK) {
        app_log_warning("[SensorManager] Invalid ICM20689 reading" APP_LOG_NL);
        this->moving = false; 
    }
    
    float accel_resultant = std::sqrt(accel_data[0]*accel_data[0] + accel_data[1]*accel_data[1] + accel_data[2]*accel_data[2]);
    float deviation = std::abs(accel_resultant - 1.0f);

    if (deviation > 0.15f) {
        this->moving = true;
    } else {
        this->moving = false;
    }
    
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

bool SensorManager::isMoving() const {
    return moving;
}