#ifndef MQTT_H_
#define MQTT_H_

#include "../main/usr_http.h"
//#include <stddef.h>
//#include "esp_system.h"
//#include "esp_event.h"
//#include "freertos/semphr.h"
//#include "lwip/dns.h"
//#include "lwip/netdb.h"
#include "mqtt_client.h"

typedef struct {
    char uri[64];
    char username[16];
    char password[32];
} mqtt_user_conf;


bool mqtt_app_start(char *config_broker_url, char *config_username, char *config_pass);

void mqtt_pub(char topic[24], char message[9]);
bool mqtt_sub(char topic[24]);
bool mqtt_data(char topic[24], char data_out[5]);

#endif /*MQTT_H_*/