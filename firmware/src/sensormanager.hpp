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
    void reset();

    bool isMoving() const;
private:

    int16_t buffer[MIC_SAMPLE_BUFFER_SIZE * MIC_N_CHANNELS];
    uint32_t n_samples = MIC_SAMPLE_BUFFER_SIZE;

    bool moving;

    int16_t temperature; 
    uint8_t humidity; 
    uint8_t noiseLevel;
};

#endif // SENSORMANAGER_HPP