#ifndef USER_CONFIG_H_
#define USER_CONFIG_H_

#include "esp_spiffs.h"
//#include "include/sys/dirent.h"
#include "sys/dirent.h"
#pragma once
//----------------------------------------------------------
//Клиент
//#define WIFI_CLIENTSSID "Megawerewolf"
//#define WIFI_CLIENTPASSWORD "Z43C7JC1F6XUGF3S79PWiddqd"
//----------------------------------------------------------
//HTTP_server
//#define SERVER_IP "192.168.1.231"
#define SERVER_PORT 80
#define CLIENT_PORT 4444
#define MAXIMUM_AP 10
//----------------------------------------------------------
//Очко доступа
#define WIFI_APSSID "Electrolux_%02x%02x%02x"
#define WIFI_APPASSWORD "12345678"
#define WIFI_STA_HTTP "http://%s"
#define WIFI_AP_HTTP "http://"WIFI_AP_IP
#define WIFI_AP_IP "192.168.22.1"
#define WIFI_AP_GW "192.168.22.1"
#define WIFI_AP_NETMASK "255.255.255.0"
#define WIFI_AP_IP_CLIENT_START "192.168.22.100"
#define WIFI_AP_IP_CLIENT_END "192.168.22.105"
#define QRCODE_WIFI "WIFI:S:%s;T:WPA;P:"WIFI_APPASSWORD";H:false;;"
//spiffs_config
static const esp_vfs_spiffs_conf_t conf = {
	      .base_path = "/spiffs",
	      .partition_label = "storage",
	      .max_files = 5,
	      .format_if_mount_failed = true
	    };
#define KEY_WIFI "WiFi"
#define KEY_MQTT "MQTT"
#define KEY_USER "Save param"
#define KEY_DISPLAY "DISPLAY"
#define USER_SETTINGS "User settings"
//---------------------------------------------------------- 
//MQTT
//TOPIC GET me
#define CELEVAYA_GET(s,t) strcat(strcpy(t,s),"/Ct/g")
#define TERMOSENSOR(s,t) strcat(strcpy(t,s),"/Ta")
#define SOSTOYANIE_GET(s,t) strcat(strcpy(t,s),"/Sst/g")
#define AKTIVNOSTI(s,t,n) strcat(strcpy(t,s),n)
//TOPIC SET me
#define SOSTOYANIE(s,t) strcat(strcpy(t,s),"/Sst/s")
#define CELEVAYA(s,t) strcat(strcpy(t,s),"/Ct/s")
#define VERSION(s,t) strcat(strcpy(t,s),"/Ver")
//MQTT
//MESSAGE
#define OFF "off"
#define AUTO "auto"
#define HALF "low"
#define FULL "high"
#define HEATING "heating"
#define IDLE "idle"
//----------------------------------------------------------
//RSSI
#define	LEVEL100(a) ((a > -70)? true : false)    //100%
#define	LEVEL75(a)   ((a > -80)? true : false) 
#define	LEVEL50(a)   ((a > -100)? true : false) 
#define	LEVEL25(a)  ((a > -127)? true : false) 
//DELAY
#define DELAY_X(a) vTaskDelay(a/portTICK_PERIOD_MS)
//BIT_MASK
#define SET_BIT(b) (b |= 1U)
#define CLIRE_BIT(b) (b &= 0U)
#define GET_BIT(b) (b & 1U ? true : false)
#define GET_3BIT(b) (b & 7U)
#define INK_3BIT(b) ((b & 7U) == 7U ? b : b++)
//revisiya
#define VER "109"
#endif /*USER_CONFIG_H_*/
