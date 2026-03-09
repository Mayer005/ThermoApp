#include "app.hpp"

#include <cstdio>
#include "em_common.h"
#include "app_assert.h"
#include "sl_bluetooth.h"
#include "app_log.h"
#include "sl_sleeptimer.h"
#include "sl_simple_button_instances.h"

#define LOG_INTERVAL_MS 300000 // 5 min
#define LOG_SCAN_INTERVAL_MS 10000 // 10 sec

enum AppState {
    STATE_INIT,
    STATE_SLEEPING,
    STATE_PREPARE_MEAS,
    STATE_MEASURING,
    STATE_SAVING
};

static AppState app_state = STATE_INIT;
static sl_sleeptimer_timer_handle_t log_timer;
volatile static bool timer_expired = false;

void log_timer_callback(sl_sleeptimer_timer_handle_t *handle, void *data) {
    (void)handle;
    (void)data;
    timer_expired = true;
}

void app_log_periodically(const char* message) {
    static uint32_t last_log_time = 0;
    uint32_t current_time = sl_sleeptimer_get_tick_count();
    uint32_t freq = sl_sleeptimer_get_timer_frequency();
    
    uint32_t elapsed_ms = (uint32_t)(((uint64_t)(current_time - last_log_time) * 1000) / freq);

    if (elapsed_ms >= 3000) {
      app_log_info("%s" APP_LOG_NL, message);
      last_log_time = current_time;
    }
}

extern "C" {
    void app_init() {
        app_log_info("App init" APP_LOG_NL);

        sensorManager.init();
        bleScanner.init();
        storage.init();

    }

    void app_process_action() {
        switch(app_state) {
            case STATE_SLEEPING: {
                if(timer_expired) {
                    timer_expired = false;
                    app_log_info("Timer expired, preparing measurement" APP_LOG_NL);
                    app_state = STATE_PREPARE_MEAS;
                }
                
                app_log_periodically("STATE_SLEEPING");
                break;
            }
                
            case STATE_PREPARE_MEAS: {
                bleScanner.reset();
                bleScanner.start();
                sensorManager.update();
                timer_expired = false;
                sl_sleeptimer_start_timer(&log_timer, LOG_SCAN_INTERVAL_MS, log_timer_callback, NULL, 0, 0);

                #ifdef DEBUG
                app_log_periodically("STATE_PREPARE_MEAS");
                #endif

                app_state = STATE_MEASURING;
                break;
            }
                
            case STATE_MEASURING:
            {
                if(timer_expired) {
                    timer_expired = false;
                    app_state = STATE_SAVING;
                }
                #ifdef DEBUG
                app_log_periodically("STATE_MEASURING");
                #endif
                break; 
            }
                
            case STATE_SAVING:{
                bleScanner.stop();
                LogEntry entry;
                uint64_t ticks = sl_sleeptimer_get_tick_count();
                uint32_t freq = sl_sleeptimer_get_timer_frequency();
                entry.timestamp = (uint32_t)((ticks * 1000) / freq);
                entry.temperature = sensorManager.getTemperature();
                entry.humidity = sensorManager.getHumidity();
                entry.noise_level = 10;//sensorManager.getNoiseLevel();
                entry.ble_device_count_total = bleScanner.getBleDeviceCountTotal();
                entry.ble_device_count_in_close = bleScanner.getBleDeviceCountInClose();
                entry.avg_rssi = bleScanner.getAvgRssi();
                entry.battery_level = 0; //TODO: battery level
                entry.status = 0; //TODO: status flags
                entry.padding = 0xAA; //padding ._.

                if (sensorManager.isMoving()) {
                    entry.status |= StatusFlag::Movement;
                    app_log_info("-> Motion Detected!" APP_LOG_NL);
                }

                if(storage.append(entry)) {
                    app_log_info("Log entry saved" APP_LOG_NL);
                } else {
                    app_log_warning("Failed to save log entry" APP_LOG_NL);
                }

                sl_sleeptimer_start_timer(&log_timer, LOG_INTERVAL_MS, log_timer_callback, NULL, 0, 0);

                app_state = STATE_SLEEPING;
                #ifdef DEBUG
                 app_log_periodically("STATE_SAVING");
                #endif
                break;
            }
                
            case STATE_INIT:{
                #ifdef DEBUG
                app_log_periodically("STATE_INIT");
                #endif
                app_state = STATE_PREPARE_MEAS;
                break;
            }
                

            default: {
                app_state = STATE_PREPARE_MEAS;
                break;
            }
                
        }
    }

    void hello(void *arguments) {
        (void)arguments;
        app_log_info("Hello from Crowd Monitor!" APP_LOG_NL);
    }

    
}

void sl_bt_on_event(sl_bt_msg_t *evt) {
        bleScanner.on_bt_event(evt);
        switch (SL_BT_MSG_ID(evt->header)) {
        
        case sl_bt_evt_system_boot_id:
            app_log_info("Bluetooth Stack Booted." APP_LOG_NL);
            break;

        case sl_bt_evt_connection_opened_id:
            app_log_warning("Connection opened unexpectedly!" APP_LOG_NL);
            sl_bt_connection_close(evt->data.evt_connection_opened.connection);
            break;
            
        default:
            break;
        }
}

void sl_button_on_change(const sl_button_t *handle)
{

    if (handle == &sl_button_btn0) {
        

        if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
            uint32_t current_ticks = sl_sleeptimer_get_tick_count();
            btn0_press_time_ms = sl_sleeptimer_tick_to_ms(current_ticks);
            
            app_log_info("BTN0 lenyomva! Tartsd 5 masodpercig a torleshez..." APP_LOG_NL);
        } 
        
        else if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_RELEASED) {
            uint32_t current_ticks = sl_sleeptimer_get_tick_count();
            uint32_t current_time_ms;
            current_time_ms = sl_sleeptimer_tick_to_ms(current_ticks);
            
            uint32_t duration = current_time_ms - btn0_press_time_ms;
            
            if (duration >= 5000) {
                app_log_info("BTN0 nyomva tartva %lu ms-ig. Flash torlese indul..." APP_LOG_NL, duration);
                storage.wipeAll();
            } else {
                app_log_info("BTN0 felengedve %lu ms utan. (Nem volt eleg hosszu a torleshez)" APP_LOG_NL, duration);
            }
        }
    }
}

