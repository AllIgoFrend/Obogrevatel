#ifndef WIFI_H_
#define WIFI_H_

//------------------------------------------------
#include "../main/esp_common.h"
//#include <rom/ets_sys.h>
#include "freertos/event_groups.h"
#include <esp_wifi.h>
#include "esp_netif.h"
//#include "esp_event.h"
//#include "esp_event_legacy.h"
//#include "esp_sleep.h"
#include "../main/user_config.h"
#include "nvs_flash.h"
//#include "esp_mac.h"
//------------------------------------------------

u8_t ICACHE_FLASH_ATTR wifi_ap_client();
bool ICACHE_FLASH_ATTR wifi_station_connected(int8_t *rssi);
bool ICACHE_FLASH_ATTR init_esp_wifi(wifi_mode_t *mode, const char * ssid, const char * pass);
bool ICACHE_FLASH_ATTR scan_ready();
char *ret_my_ip();
//------------------------------------------------
#endif /* WIFI_H_ */
