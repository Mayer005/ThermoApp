#include "BLEScanner.hpp"

extern "C" {
    #include "app_log.h"
    #include "sl_bt_api.h"
}
#include <memory>

BLEScanner::BLEScanner() : seen_device_count(0), ble_device_count_total(0), ble_device_count_in_close(0), rssi_sum(0), rssi_count(0), avg_rssi(0) {}

BLEScanner::~BLEScanner() {}

void BLEScanner::init() {
    sl_status_t sc = sl_bt_scanner_set_parameters(sl_bt_scanner_scan_mode_passive, 160, 80); // 1 second timer for periodic tasks
    if(sc != SL_STATUS_OK) {
        app_log_warning("Failed to set BLE scanner parameters [0x%04lx]" APP_LOG_NL, sc);
    } else {
        app_log_info("BLE scanner initialized" APP_LOG_NL);
    }
}

void BLEScanner::start() {
    sl_status_t sc = sl_bt_scanner_start(sl_bt_gap_1m_phy, sl_bt_scanner_discover_generic);

    if(sc != SL_STATUS_OK) {
        app_log_warning("Failed to start BLE scanner [0x%04lx]" APP_LOG_NL, sc);
    } else {
        app_log_info("BLE scanner started" APP_LOG_NL);
    }
}

void BLEScanner::stop() {
    sl_status_t sc = sl_bt_scanner_stop();

    if(sc != SL_STATUS_OK) {
        app_log_warning("Failed to stop BLE scanner [0x%04lx]" APP_LOG_NL, sc);
    } else {
        app_log_info("BLE scanner stopped" APP_LOG_NL);
    }
}

void BLEScanner::reset() {
    this->seen_device_count = 0;
    this->ble_device_count_total = 0;
    this->ble_device_count_in_close = 0;
    this->rssi_sum = 0;
    this->rssi_count = 0;
}

bool BLEScanner::isDeviceSeen(const bd_addr& addr, uint8_t type) {
    for (uint16_t i = 0; i < seen_device_count; ++i) {
        if (seen_devices[i].address_type == type && std::memcmp(seen_devices[i].address.addr, addr.addr, 6) == 0) {
            return true;
        }
    }
    return false;
}

void BLEScanner::on_bt_event(sl_bt_msg_t *evt) {
    if (SL_BT_MSG_ID(evt->header) != sl_bt_evt_scanner_legacy_advertisement_report_id) {
        return;
    }

    int8_t rssi = evt->data.evt_scanner_legacy_advertisement_report.rssi;
    uint8_t addr_type = evt->data.evt_scanner_legacy_advertisement_report.address_type;
    bd_addr addr = evt->data.evt_scanner_legacy_advertisement_report.address;
    
    if (isDeviceSeen(addr, addr_type)) return;

    if (seen_device_count < MAX_SEEN_DEVICES) {
        seen_devices[seen_device_count].address = addr;
        seen_devices[seen_device_count].address_type = addr_type;
        seen_device_count++;
    } 

    ble_device_count_total++;
    rssi_sum += rssi;
    rssi_count++;

    if (rssi > RSSI_CLOSE_THRESHOLD) {
        ble_device_count_in_close++;
    }
}

uint16_t BLEScanner::getBleDeviceCountTotal() const { return ble_device_count_total; }
uint16_t BLEScanner::getBleDeviceCountInClose() const { return ble_device_count_in_close; }
int8_t BLEScanner::getAvgRssi() const {
    if (rssi_count == 0) return 0;
    return (int8_t)(rssi_sum / rssi_count);
}