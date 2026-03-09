#include "storage.hpp"

#include <cstdio>

#if __cplusplus
extern "C" {
#endif
    #include "em_msc.h"
    #include "em_device.h"
    #include "app_log.h"
#if __cplusplus
}
#endif

Storage::Storage() : writeHeadAddr(FLASH_START_ADDRESS) {}

Storage::~Storage() {}

void Storage::init() {
    MSC_Init();
    
    uint32_t current = FLASH_START_ADDRESS;
    const uint32_t end = FLASH_START_ADDRESS + FLASH_SIZE_BYTES;
    while (current < end) {
        // yeah, yeah reinterpret_cast is bad practice, but its faster and i like to live dangerously :D
        const LogEntry* entry = reinterpret_cast<const LogEntry*>(current);

        if(entry->timestamp == 0xFFFFFFFF) {
            writeHeadAddr = current;
            app_log_info("Storage initialized. Write head at address: 0x%08lX (Entries: %lu)\n", writeHeadAddr, getEntryCount());
            return;
        }
        current += sizeof(LogEntry);
    }
    writeHeadAddr = end;
    app_log_warning("Storage initialized. No free space found, write head at end of flash: 0x%08lX\n", writeHeadAddr);
}

bool Storage::append(const LogEntry& entry) {
    if (writeHeadAddr + sizeof(LogEntry) > FLASH_START_ADDRESS + FLASH_SIZE_BYTES) {
        app_log_warning("Storage append failed: No more space to write new entry.\n");
        return false; // No more space to write
    }

    MSC_Status_TypeDef result = MSC_WriteWord(reinterpret_cast<uint32_t*>(writeHeadAddr), reinterpret_cast<const void*>(&entry), sizeof(LogEntry));

    if (result != mscReturnOk) {
        app_log_warning("Storage append failed: MSC_WriteWord returned error code %d\n", result);
        return false; // Write operation failed
    }

    writeHeadAddr += sizeof(LogEntry);
    app_log_info("Storage append successful. New write head at address: 0x%08lX\n", writeHeadAddr);
    return true;
}

void Storage::wipeAll() {
    uint32_t start = FLASH_START_ADDRESS;
    const uint32_t end = FLASH_START_ADDRESS + FLASH_SIZE_BYTES;


    app_log_info("Wiping storage from address: 0x%08lX to 0x%08lX\n", start, end);

    while (start < end) {
        erasePage(reinterpret_cast<uint32_t*>(start));
        start += FLASH_PAGE_SIZE;
    }
    writeHeadAddr = FLASH_START_ADDRESS;
    app_log_info("Storage wiped. Write head reset to address: 0x%08lX\n", writeHeadAddr);
}

uint32_t Storage::getEntryCount() const {
    return (writeHeadAddr - FLASH_START_ADDRESS) / sizeof(LogEntry); // should work 
}

void Storage::readTimeRange(uint32_t start_time, uint32_t end_time, std::function<void(const LogEntry&)> callback) const {
    uint32_t current = FLASH_START_ADDRESS;
    
    while (current < writeHeadAddr) {
        const LogEntry* entry = reinterpret_cast<const LogEntry*>(current);
        
        if (entry->timestamp >= start_time && entry->timestamp <= end_time) {
            callback(*entry);
        }
        
        current += sizeof(LogEntry);
    }
}