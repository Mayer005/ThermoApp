#ifndef TYPES_HPP
#define TYPES_HPP

#include <cstdint>

namespace StatusFlag {
    static constexpr uint8_t None = 0;
    static constexpr uint8_t Crowded = 1 << 0; // 0000 0001 (Decimal 1) 
    static constexpr uint8_t OverCrowded = 1 << 1; // 0000 0010 (Decimal 2)
    static constexpr uint8_t Jamming = 1 << 2; // 0000 0100 (Decimal 4)
    static constexpr uint8_t Rain = 1 << 3; // 0000 1000 (Decimal 8)
    static constexpr uint8_t Loud = 1 << 4; // 0001 0000 (Decimal 16)
    static constexpr uint8_t Movement = 1 << 5; // 0010 0000 (Decimal 32)
    static constexpr uint8_t LowBattery = 1 << 6; // 0100 0000 (Decimal 64)
    static constexpr uint8_t Error = 1 << 7; // 1000 0000 (Decimal 128)
}

#pragma pack(push, 1)

struct LogEntry {
    uint32_t timestamp; // Unix timestamp to know when the log entry was created --> 4byte

    int16_t temperature; // Temperature of the room in Celsius, multiplied by 100 to preserve two decimal places --> 2byte    -|
    uint8_t humidity; // Humidity percentage, multiplied by 10 to preserve one decimal places --> 1byte                        |--> with theese three fields, I can estimate if someone is in the room or not, and if the conditions are good for them to breathe. (its always good to breathe xd)
    uint8_t noise_level; // Noise level in the room --> 1byte                                                                 -|

    uint16_t ble_device_count_total; // Number of BLE devices detected anywhere nearby (this includes every device which has a ble signal which reaches the device) --> 2byte
    uint16_t ble_device_count_in_close; // Number of BLE devices detected up close --> 2byte

    uint8_t avg_rssi; // Average RSSI of the detected BLE devices, multiplied by 100 to preserve two decimal places (for body shading detection) --> 1byte
    uint8_t padding; // padding --> 1byte

    uint8_t battery_level; // battery level --> 1byte
    uint8_t status; //reserved for future use, to make the struct size a multiple of 4 bytes (for better memory alignment) --> 1byte
};

#pragma pack(pop)

static_assert(sizeof(LogEntry) == 16, "LogEntry struct must be exactly 16 bytes in size"); // I think (4+2+1+1+2+2+2+1+1) = 16 yeah

#endif // TYPES_HPP