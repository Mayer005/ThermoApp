#ifndef STORAGE_HPP
#define STORAGE_HPP

#include "types.hpp"
#include <functional>
class Storage {
public:
    Storage();
    ~Storage();
    void init();
    bool append(const LogEntry& entry);
    void wipeAll();
    uint32_t getEntryCount() const; // uint16_t should be enough too
    void readTimeRange(uint32_t start_time, uint32_t end_time, std::function<void(const LogEntry&)> callback) const; ///TODO

private:
    static constexpr uint32_t FLASH_START_ADDRESS = 0x08080000; // Adjust this based on your microcontroller's flash memory map
    static constexpr uint32_t FLASH_SIZE_BYTES = 0x000800000; // 512KB of flash memory

    uint32_t writeHeadAddr;
};


#endif