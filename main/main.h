#ifndef MAIN_H_
#define MAIN_H_

//------------------------------------------------
//#include "esp_common.h"
#include "../main/termostat.h"
#include "../main/usr_http.h"
#include "../main/wifi.h"
#include "../main/user_config.h"
#include "../main/mqtt.h"
#include "../main/ssd1306.h"
#include "../main/drow.h"

#include "../main/app_http.h"
#include "../main/app_ota.h"


//------------------------------------------------
//WIFI_MODE ICACHE_FLASH_ATTR init_esp_wifi();
void ICACHE_FLASH_ATTR mainTask(void *pvParameters);
//------------------------------------------------
#endif /* MAIN_H_ */



