#ifndef SENSORMANAGER_HPP
#define SENSORMANAGER_HPP

#include <cstdint>
#include "commonconfig.h"

class SensorManager {
public:
    SensorManager();
    ~SensorManager();

    void init();
    void update();

    int16_t getTemperature() const;
    uint8_t getHumidity() const;
    uint8_t getNoiseLevel() const;
    uint8_t getSteps() const;
    void resetSteps();
private:

    int16_t buffer[MIC_SAMPLE_BUFFER_SIZE * MIC_N_CHANNELS];
    int32_t n_samples = MIC_SAMPLE_BUFFER_SIZE;
    int16_t temperature; 
    uint8_t humidity; 
    uint8_t noiseLevel;
    uint8_t steps;
};

#endif // SENSORMANAGER_HPP