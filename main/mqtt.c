#include "../main/mqtt.h"
//bool start = false;
//bool bro_connect = false;
//bool sub = false;
//bool resive_data = false;
int msg_id;
char topic_ev[24] = {0};
char data_ev[5] = {0};

typedef struct {
    u8_t start:1;
    u8_t bro_connect:1;
    u8_t sub:1;
    u8_t resive_data:1;
    u8_t sand_timeout:1;
    u8_t rezerv:3;
  } mqtt_bit;

mqtt_bit sost_bit = {0U};

//static os_timer_t timer_send;
//bool sand_timeout = false;
esp_mqtt_client_handle_t client;
//static const char *TAG = "MQTT";

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    client = event->client;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            SET_BIT(sost_bit.bro_connect);
            //bro_connect = true;
            //ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            //msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
            //ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            CLIRE_BIT(sost_bit.bro_connect);
            //bro_connect = false;
            //ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            //ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            SET_BIT(sost_bit.sub);
            //sub = true;
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            CLIRE_BIT(sost_bit.sub);
            //sub = false;
            //ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            //ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            //ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            if((event->topic_len + 1U) <= sizeof(topic_ev)){
                snprintf(topic_ev, event->topic_len + 1U, event->topic);
            }
            if((event->data_len + 1U) <= sizeof(data_ev)){
                snprintf(data_ev, event->data_len + 1U, event->data);
            }
            SET_BIT(sost_bit.resive_data);
            //resive_data = true;
            break;
        case MQTT_EVENT_ERROR:
            //ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            //ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    //ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

bool mqtt_app_start(char *config_broker_url, char *config_username, char *config_pass)
{
    if(!GET_BIT(sost_bit.start)){
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = config_broker_url,
        .username = config_username,
        .password = config_pass,
    };
    //printf("url %s \n uname %s \n pass %s \n",config_broker_url, config_username, config_pass);
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
    SET_BIT(sost_bit.start);
    //start = true;
    }
    return GET_BIT(sost_bit.start);
}

void mqtt_pub(char topic[24], char message[9])
{
    if(GET_BIT(sost_bit.bro_connect)){
        msg_id = esp_mqtt_client_publish(client, topic, message, 0, 0, 0);
    }
    //ESP_LOGI(TAG, "[APP] Startup..");
    //ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    //ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
    /*
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);
    */
    
    //ESP_ERROR_CHECK(esp_netif_init());
    //ESP_ERROR_CHECK(esp_event_loop_create_default());    
}
bool mqtt_sub(char topic[24])
{
    if(topic != NULL){
        if(GET_BIT(sost_bit.bro_connect)){
            msg_id = esp_mqtt_client_subscribe(client, topic, 0);
            //ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        }
    }
    return GET_BIT(sost_bit.bro_connect);
}
bool mqtt_data(char topic[24], char data_out[5])
{
    if(GET_BIT(sost_bit.resive_data)){
        if(strstr(topic, topic_ev) != 0){
            strcpy(data_out, (const char *) data_ev);
            CLIRE_BIT(sost_bit.resive_data);
            return true;
        }
        return false;
    }
    return false;
}