#include "storage.hpp"
#include "commonconfig.hpp"
#include <cstdint>
#include <cstdio>

extern "C" {
    #include "em_msc.h"
    #include "em_device.h"
}

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
            #ifdef DEBUG
                printf("Storage initialized. Write head at address: 0x%08lX (Entries: %lu)\n", writeHeadAddr, getEntryCount());
            #endif
            return;
        }
        current += sizeof(LogEntry);
    }
    writeHeadAddr = end;
    #ifdef DEBUG
        printf("Storage initialized. No free space found, write head at end of flash: 0x%08lX\n", writeHeadAddr);
    #endif
}

bool Storage::append(const LogEntry& entry) {
    if (writeHeadAddr + sizeof(LogEntry) > FLASH_START_ADDRESS + FLASH_SIZE_BYTES) {
        #ifdef DEBUG
            printf("Storage append failed: No more space to write new entry.\n");
        #endif
        return false; // No more space to write
    }

    MSC_Status_TypeDef result = MSC_WriteWord(static_cast<uint32_t*>(writeHeadAddr), static_cast<const void*>(&entry), sizeof(LogEntry));

    if (result != mscReturnOk) {
        #ifdef DEBUG
            printf("Storage append failed: MSC_WriteWord returned error code %d\n", result);
        #endif
        return false; // Write operation failed
    }

    writeHeadAddr += sizeof(LogEntry);
    #ifdef DEBUG
        printf("Storage append successful. New write head at address: 0x%08lX\n", writeHeadAddr);
    #endif
    return true;
}

void Storage::wipeAll() {
    uint32_t start = FLASH_START_ADDRESS;
    const uint32_t end = FLASH_START_ADDRESS + FLASH_SIZE_BYTES;

    #ifdef DEBUG
        printf("Wiping storage from address: 0x%08lX to 0x%08lX\n", start, end);
    #endif

    while (start < end) {
        erasePage(static_cast<uint32_t*>(start));
        start += FLASH_PAGE_SIZE;
    }
    writeHeadAddr = FLASH_START_ADDRESS;
    #ifdef DEBUG
        printf("Storage wiped. Write head reset to address: 0x%08lX\n", writeHeadAddr);
    #endif
}

uint32_t Storage::getEntryCount() const {
    return (writeHeadAddr - FLASH_START_ADDRESS) / sizeof(LogEntry); // should work 
}