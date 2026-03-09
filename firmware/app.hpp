
#ifndef APP_HPP
#define APP_HPP

#include "em_common.h"
#include "app_assert.h"
#include "sl_bluetooth.h"
#include "app_log.h"
#include "sl_bt_api.h"
#include "sl_sleeptimer.h"

#ifdef __cplusplus
extern "C" {
#endif

void app_init(void);
void app_process_action(void);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include "./src/BLEScanner.hpp"
#include "./src/commonconfig.h"
#include "./src/sensormanager.hpp"
#include "./src/storage.hpp"
#include "./src/types.hpp"




SensorManager sensorManager;
BLEScanner bleScanner;
Storage storage;

static uint32_t btn0_press_time_ms = 0;

#endif

#endif