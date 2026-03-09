#ifndef BLESCANNER_HPP
#define BLESCANNER_HPP

#if __cplusplus 
extern "C" {
#endif
    #include "sl_bt_api.h"
#if __cplusplus
}
#endif

#define MAX_SEEN_DEVICES 2500

struct DeviceID {
    bd_addr address;      
    uint8_t address_type; 
};

class BLEScanner {
public:
    BLEScanner();
    ~BLEScanner();

    void init();
    void start();
    void stop();
    void reset();
    void on_bt_event(sl_bt_msg_t *evt);

    uint16_t getBleDeviceCountTotal() const;
    uint16_t getBleDeviceCountInClose() const;
    int8_t getAvgRssi() const;

private:
    DeviceID seen_devices[MAX_SEEN_DEVICES];
    uint16_t seen_device_count;
    uint16_t ble_device_count_total;
    uint16_t ble_device_count_in_close;
    int32_t rssi_sum;
    uint16_t rssi_count;

    static constexpr int8_t RSSI_CLOSE_THRESHOLD = -70;

    bool isDeviceSeen(const bd_addr& addr, uint8_t type);
};

#endif