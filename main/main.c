#include "../main/main.h"
//------------------------------------------------------
size_t total = 0;
size_t used = 0;
TaskHandle_t xHandle_mqtt_reg_task = NULL;
TaskHandle_t xHandle_ota_task = NULL;
FILE* fd = NULL;
static int8_t rssi = 0U;
struct stat st;
static uint8_t defolt = 89U;
static u8_t value = 0U;
static uint8_t mqtt_cT = 0U;
static uint8_t mqtt_st = 0U;
static char celt_sost[5] = {0};
static char mqtt_out[5] = {0};
static uint8_t sost_in = 0U;
static uint8_t rele_num = 0U;
static mqtt_user_conf mqtt_cfg;
static char celevaya_str[4] = {0};
static char mac_str[24] = {0};
static char sost_str[11] = {0U};
static char sostoyanie_str[6] = {0};
static char rele_str[2] = {0};
static uint8_t display_type = 1U;
static uint8_t buffer[(SSD1306_WIDTH * SSD1306_HEIGHT) >> 3U];
  typedef struct {
    u8_t change_celevaya:1;
    u8_t change_sostoyanie:1;
    u8_t change_mes_temper:1;
    u8_t wifi_sta_connect:1;
    u8_t wifi_ap_connect:1;
    u8_t wifi_up:1;
    u8_t wifi_down:1;
    u8_t rele_change:1;
  } sost_bit;
  typedef struct {
    u8_t send_1:1;
    u8_t send_2:1;
    u8_t send_3:1;
    u8_t send_4:1;
    u8_t send_5:1;
    u8_t send_6:1;
    u8_t send_7:1;
    u8_t send_8:1;
  } send_pub_bit;
typedef struct {
    u8_t ssid_check:1;
    u8_t mqtt_check:1;
    u8_t restart_ready:1;
    u8_t bro_connect:1;
    u8_t qr_clire:1;
    u8_t mem_chng:3;
} flagi;
static const int i2c_master_port = I2C_NUM_0;
static  ssd1306_t dev = {
                .i2c_port = i2c_master_port,
                .i2c_addr = SSD1306_I2C_ADDR_1,
                .screen = SH1106_SCREEN, // or SH1106_SCREEN or SSD1306_SCREEN
                .width = SSD1306_WIDTH,
                .height = SSD1306_HEIGHT
};
wifi_config_t wifi_cfg;
size_t wicfg_len = sizeof(wifi_cfg);
static char wifi_qr[121U] = {0};
sost_bit s_b = {0U};
send_pub_bit s_p_b = {0U};
flagi f = {0U};
esp_ota_firm_t ota_firm;

#define DROW_CELEVAYA_T(b,w) &dev, buffer, 3U, 16U, mqtt_cT, b, w

//------------------------------------------------------
void mqtt_task_start()
{
  if(xHandle_mqtt_reg_task == NULL){
    xTaskCreate(mqtt_reg_task, "mqtt_reg_task", 10240, NULL, 5, &xHandle_mqtt_reg_task);
    buZZ(400U);
  }
}
//------------------------------------------------------
static bool temper_chng(char *argument)
{
  if(strcmp((const char *) argument, (const char *) state_control(NULL))){ 
    SET_BIT(s_b.change_mes_temper);
    strcpy(argument, (const char *) state_control(NULL));
    return true;
  }else{
    CLIRE_BIT(s_b.change_mes_temper);
    return false;
  }
}
//------------------------------------------------------
static char *stat_2_str(uint8_t *st_ct)
{
  if((*st_ct >> 2U) != mqtt_cT){
    mqtt_cT = (*st_ct >> 2U);
    memset(celevaya_str,0,1);
    snprintf(celevaya_str,sizeof (celevaya_str), "%hd", mqtt_cT);
    SET_BIT(s_b.change_celevaya);
    return celevaya_str;
  }else{
    CLIRE_BIT(s_b.change_celevaya);
  }
  if((*st_ct & 3U) != mqtt_st){
    mqtt_st = (*st_ct & 3U);
    memset(sostoyanie_str,0,1);
    if(mqtt_st == 0U){
      strcpy(sostoyanie_str, OFF);
      strcpy(sost_str, " ");
    }else if(mqtt_st == 1U){
      strcpy(sostoyanie_str, AUTO);
      strcpy(sost_str, " АВТОМАТ");//"Мощность: АВТОМАТ"
    }else if(mqtt_st == 2U){
      strcpy(sostoyanie_str, HALF);
      strcpy(sost_str, "  500 Вт");
    }else if(mqtt_st == 3U){
      strcpy(sostoyanie_str, FULL);
      strcpy(sost_str, "  1000 Вт");
    }
    SET_BIT(s_b.change_sostoyanie);
    return sostoyanie_str;
  }else{
    CLIRE_BIT(s_b.change_sostoyanie);
  }
  return NULL;
}
//------------------------------------------------------
static bool all_together_now() //рисуем экран
{
  if(GET_BIT(s_b.wifi_ap_connect)){
    drowing_wifi(&dev, buffer, 3U, 0U, WiFi_100,NULL,true);
    if(mqtt_st == 0U){
      if(!GET_BIT(f.qr_clire)){
        drowing_QRcode(&dev, buffer, WIFI_AP_HTTP, 3U, 3U, 33U, 0U);
        SET_BIT(f.qr_clire);
      }
    }else if(GET_BIT(f.qr_clire)){
      drowing_QRcode(&dev, buffer, NULL, 3U, 3U, 33U, 0U);
      CLIRE_BIT(f.qr_clire);
    }
  }else if(!GET_BIT(f.ssid_check)){
    clire_wifi(&dev, buffer, 3U, 0U);
    drowing_wifi_ap(&dev, buffer, 9U, 6U);
    if(mqtt_st == 0U){
      if(!GET_BIT(f.qr_clire)){
        drowing_QRcode(&dev, buffer, (const char *)wifi_qr, 3U, 0U, 33U, 0U);
        SET_BIT(f.qr_clire);
      }
    }else if(GET_BIT(f.qr_clire)){
      drowing_QRcode(&dev, buffer, NULL, 3U, 0U, 33U, 0U);
      CLIRE_BIT(f.qr_clire);
    }
  }
  if(GET_BIT(s_b.wifi_sta_connect)){
    drowing_wifi(&dev, buffer, 3U, 0U, WiFi_100,&rssi,false);
    if(mqtt_st == 0U && !GET_BIT(f.mqtt_check)){
      if(!GET_BIT(f.qr_clire)){
        drowing_QRcode(&dev, buffer, (const char *)ret_my_ip(), 3U, 3U, 33U, 0U);
        SET_BIT(f.qr_clire);
      }
    }else if(GET_BIT(f.qr_clire)){
      drowing_QRcode(&dev, buffer, NULL, 3U, 3U, 33U, 0U);
      CLIRE_BIT(f.qr_clire);
    }
  }else{
    //printf("clire wifi\n");
    clire_wifi(&dev, buffer, 3U, 0U);
  }
  if(!GET_BIT(f.qr_clire)){
    if(stat_2_str(mem_state_rw(NULL)) != NULL){
      if(GET_BIT(s_b.change_celevaya)){
        drowing_celevaya_temper(DROW_CELEVAYA_T(OLED_COLOR_BLACK,OLED_COLOR_WHITE));
      }
      if(GET_BIT(s_b.change_sostoyanie)){
        ssd1306_draw_string(&dev, buffer, font_builtin_fonts[FONT_FACE_TAHOMA_6], 3U, 44U, 120U, (const char *)sost_str, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
      }
    }else if(!GET_3BIT(f.mem_chng) && mqtt_st != 0U){
      drowing_celevaya_temper(DROW_CELEVAYA_T(OLED_COLOR_WHITE,OLED_COLOR_BLACK));
    }
    if(temper_chng(mqtt_out)){
      ssd1306_draw_string(&dev, buffer, font_builtin_fonts[FONT_FACE_TAHOMA_9], 53U, 17U, 48U, (const char *)mqtt_out, OLED_COLOR_WHITE, OLED_COLOR_BLACK); //пишем надпись в выставленной позиции белым цветом. 
      ssd1306_draw_string(&dev, buffer, font_builtin_fonts[FONT_FACE_TAHOMA_9], 101U, 17U, 23U, "°C", OLED_COLOR_WHITE, OLED_COLOR_BLACK); //пишем надпись в выставленной позиции белым цветом. 
    }
    if(GET_BIT(s_b.rele_change)){
      drowing_heater(&dev, buffer, 47U, 0U, &rele_num);
    }
  }
  if(GET_BIT(s_b.wifi_up)){
    drowing_wifi(&dev, buffer, 3U, 0U, WiFi_100_up,&rssi,false);
      CLIRE_BIT(s_b.wifi_up);
  }
  if(GET_BIT(s_b.wifi_down)){
    drowing_wifi(&dev, buffer, 3U, 0U, WiFi_100_down,&rssi,false);
      CLIRE_BIT(s_b.wifi_down);
  }
  if(GET_BIT(f.bro_connect)){
    drowing_mqtt(&dev, buffer, 110U, 0U, false);
  }else{
    drowing_mqtt(&dev, buffer, 110U, 0U, true);
  }
  if(GET_3BIT(f.mem_chng) > 0U && GET_3BIT(f.mem_chng) < 7U){
    drowing_write(&dev, buffer, 3U, 51U, false);
  }else{
    drowing_write(&dev, buffer, 3U, 51U, true);
  }

  return true;

}
//------------------------------------------------------
bool mqtt_setting_rw()
{
  if(xHandle_mqtt_reg_task != NULL) {
    CLIRE_BIT(f.mqtt_check);
    return false;
  }
  if(!GET_BIT(f.mqtt_check)){
    nvs_handle handle;
    size_t mqtt_len = sizeof(mqtt_cfg);
    if( nvs_open( USER_SETTINGS, NVS_READWRITE, &handle)  == ESP_OK){
      if(nvs_get_blob( handle, KEY_MQTT, &mqtt_cfg, &mqtt_len) == ESP_OK && (mqtt_cfg.username[0] != '\0') && (mqtt_cfg.uri[0] != '\0') && (mqtt_cfg.password[0] != '\0')){
        mqtt_app_start(mqtt_cfg.uri, mqtt_cfg.username, mqtt_cfg.password);
        SET_BIT(f.mqtt_check);
      }else{
        CLIRE_BIT(f.mqtt_check);
        mqtt_task_start();
      }
      nvs_close(handle);
    }
  }
  return GET_BIT(f.mqtt_check);
}
//------------------------------------------------------
static bool rele_chng(uint8_t *r_n)
{
  //printf("rele1  %d rele2  %d  arg %d \n", gpio_get_level(RELE_1), gpio_get_level(RELE_2), *r_n);
  if((gpio_get_level(RELE_1) != (*r_n & 1U) || gpio_get_level(RELE_2) != ((*r_n >> 1U) & 1U))){ 
    *r_n = (gpio_get_level(RELE_2)<<1U | gpio_get_level(RELE_1)) & 3U;
    //printf("rele %d \n", *r_n);
    SET_BIT(s_b.rele_change);
    return true;
  }else{
    CLIRE_BIT(s_b.rele_change);
    return false;
  }
}
//------------------------------------------------------
static void send_act()
{
  if(rele_num == 0 && strcmp(sostoyanie_str,OFF)){
    mqtt_pub(AKTIVNOSTI(mqtt_cfg.username,mac_str,"/act/rej"), IDLE);
  }else if (!strcmp(sostoyanie_str,OFF))
  {
    mqtt_pub(AKTIVNOSTI(mqtt_cfg.username,mac_str,"/act/rej"), OFF);
  }else if (rele_num != 0){
    mqtt_pub(AKTIVNOSTI(mqtt_cfg.username,mac_str,"/act/rej"), HEATING);
  }
    rele_str[0] = rele_num + 48U;
    rele_str[1] = '\0';
  //snprintf(rele_str,sizeof (rele_str), "%hd", rele_num);
  mqtt_pub(AKTIVNOSTI(mqtt_cfg.username,mac_str,"/act/htr"), rele_str);
}
//------------------------------------------------------
bool read_u8_from_nvs (const char *user_set, const char *key_read, nvs_handle *handle, u8_t *val)
{
  if( nvs_open( user_set, NVS_READWRITE, handle) == ESP_OK){
    if(nvs_get_u8(*handle, key_read, val) == ESP_OK){
      nvs_close(*handle);
      return true;
    }
  }
  return false;
}
bool write_u8_to_nvs (const char *user_set, const char *key_set, nvs_handle *handle, u8_t *val)
{
  if( nvs_open( user_set, NVS_READWRITE, handle) == ESP_OK){
    if(nvs_set_u8(*handle, key_set, *val) == ESP_OK){
      if( nvs_commit(*handle) == ESP_OK){
        nvs_close(*handle);
        return true;
      }
    }
  }
  return false;
}
bool erase_key_from_nvs (const char *user_set, const char *key_erase, nvs_handle *handle)
{
  if( nvs_open( user_set, NVS_READWRITE, handle) == ESP_OK ){
    if(nvs_erase_key(*handle, key_erase) == ESP_OK){
      if( nvs_commit(*handle) == ESP_OK ){
        nvs_close(*handle);
        return true;
      }
    }
  }
  return false;
}
//------------------------------------------------------
void ICACHE_FLASH_ATTR mainTask(void *pvParameters)
{
  nvs_handle handle;
  buZZ(100U);
  wifi_mode_t mode = 0;
  ssd1306_display_on(&dev, true);
  ssd1306_set_whole_display_lighting(&dev, false);
  if(esp_wifi_get_mode(&mode) != ESP_OK)
    {
      os_printf("Failed get mode!\n");
    }
  TaskHandle_t xHandle_ap_reg_task = NULL;
  TaskHandle_t xHandle_termostat_task = NULL;
  TaskHandle_t xHandle_termosens_task = NULL;
  xTaskCreate(termosens_task, "termosens_task", 512U, NULL, 2U, &xHandle_termosens_task); //(Stack canary watchpoint triggered) если 128 
  xTaskCreate(termostat_task, "termostat_task", 1024U, NULL, 11U, &xHandle_termostat_task);
  SET_BIT(s_b.rele_change);
  SET_BIT(s_p_b.send_1);
  while(1)
  {
    if(all_together_now()){
      if(ssd1306_load_frame_buffer(&dev, buffer)){
        printf("buffer xui \n");
      }
    } //рисуем экран
    if(mode == WIFI_MODE_STA)
      {
        if(wifi_station_connected(&rssi)){
          SET_BIT(s_b.wifi_sta_connect);
          CLIRE_BIT(f.qr_clire);
          //отправить uint8_t *rssi в рисование вифи, поставить сюда функцию рисования через drow вместо xbm
          //ssd1306_load_xbm(&dev, wifi_bits, buffer, 0U, 0U, 11U, 11U);
          //os_printf( "connected rssi %i\n", rssi);
          if(mqtt_setting_rw() && GET_BIT(f.bro_connect) ){
            if(GET_BIT(s_b.change_mes_temper)){
              mqtt_pub(TERMOSENSOR(mqtt_cfg.username,mac_str), mqtt_out);
              //printf("%s \n",mqtt_out);
              SET_BIT(s_b.wifi_up);
            }
            if(GET_BIT(s_b.rele_change)){
              send_act();
              SET_BIT(s_b.wifi_up);
              SET_BIT(s_p_b.send_4);
            }
            if(GET_BIT(s_b.change_celevaya)){
              mqtt_pub(CELEVAYA_GET(mqtt_cfg.username,mac_str), celevaya_str);
              SET_BIT(s_b.wifi_up);
            }else if(mqtt_data(CELEVAYA(mqtt_cfg.username,mac_str), celt_sost) && (f.mem_chng == 0U)){
              //printf("mqtt_data_celevaya %s\n", celt_sost);
              sost_in = atoi((const char *)celt_sost);
              if((sost_in < 31) && (sost_in > 4)){
                sost_in = (sost_in << 2)+(*mem_state_rw(NULL) & 3U);
                state_control(&sost_in);
              }
              SET_BIT(s_b.wifi_down);
            }
            if(GET_BIT(s_b.change_sostoyanie)){
              mqtt_pub(SOSTOYANIE_GET(mqtt_cfg.username,mac_str), sostoyanie_str);
              SET_BIT(s_b.wifi_up);
            }else if(mqtt_data(SOSTOYANIE(mqtt_cfg.username,mac_str), celt_sost) && (f.mem_chng == 0U)){
              //printf("mqtt_data_sostoyanie %s\n", celt_sost);
              if(!strcmp((const char*) celt_sost, OFF)){
                sost_in = (*mem_state_rw(NULL) & 252U);
                state_control(&sost_in);
              }
              if(!strcmp((const char*) celt_sost, AUTO)){
                sost_in = (*mem_state_rw(NULL) & 252U) + 1U;
                state_control(&sost_in);
              }
              if(!strcmp((const char*) celt_sost, HALF)){
                sost_in = (*mem_state_rw(NULL) & 252U) + 2U;
                state_control(&sost_in);
              }
              if(!strcmp((const char*) celt_sost, FULL)){
                sost_in = (*mem_state_rw(NULL) & 252U) + 3U;
                state_control(&sost_in);
              }
              SET_BIT(s_b.wifi_down);
            }
            if(mqtt_data(VERSION(mqtt_cfg.username,mac_str), celt_sost)){
              if(!strcmp((const char*) celt_sost, "?")){
                mqtt_pub(VERSION(mqtt_cfg.username,mac_str), VER);
                SET_BIT(s_b.wifi_up);
              }else if(!strcmp((const char*) celt_sost, "!")){
                //start http server firmware;

                // Initialize OTA-update
                if(init_ota(&ota_firm) != ESP_OK){
                  init_ota(&ota_firm);
                }
                // Initialize HTTP server for the webpage to upload updates
                init_http();
                //if(xHandle_ota_task == NULL){
                //  xTaskCreate(ota_task, "ota_task", 10240, NULL, 5, &xHandle_ota_task);
                  buZZ(200U);
                //}
              }else if (!strcmp((const char*) celt_sost, "ssd")){
                display_type = SSD1306_SCREEN;
                if(write_u8_to_nvs(USER_SETTINGS, KEY_DISPLAY, &handle, &display_type)){
                  dev.screen = SSD1306_SCREEN; // or SH1106_SCREEN or SSD1306_SCREEN
                  ssd1306_init(&dev);
                  ssd1306_set_whole_display_lighting(&dev, false);
                }
              }else if (!strcmp((const char*) celt_sost, "sh")){
                display_type = SH1106_SCREEN;
                if(write_u8_to_nvs(USER_SETTINGS, KEY_DISPLAY, &handle, &display_type)){
                  dev.screen = SH1106_SCREEN; // or SH1106_SCREEN or SSD1306_SCREEN
                  ssd1306_init(&dev);
                  ssd1306_set_whole_display_lighting(&dev, false);
                }
              }
            }
          }
        }else{
          CLIRE_BIT(s_b.wifi_sta_connect);
        }
          ///////////////////////////////////////////////////////
      }
    if(xHandle_termostat_task == NULL){
      os_printf("Termostat sdox run again\n");
      xTaskCreate(termostat_task, "termostat_task", 1024U, NULL, 11U, &xHandle_termostat_task); //(Stack canary watchpoint triggered) если 128 
      }
    if(xHandle_termosens_task == NULL){
      os_printf("Termosens run again\n");
      xTaskCreate(termosens_task, "termosens_task", 512U, NULL, 2U, &xHandle_termosens_task); //(Stack canary watchpoint triggered) если 128 
      }
    if(mode == WIFI_MODE_APSTA)
      {
        if(wifi_ap_client() != 0U && xHandle_ap_reg_task == NULL)
        {
          xTaskCreate(ap_reg_task, "ap_reg_task", 10240U, NULL, 5U, &xHandle_ap_reg_task);
          buZZ(300U);
          SET_BIT(s_b.wifi_ap_connect);
          CLIRE_BIT(f.qr_clire);
        }
        if(wifi_ap_client() == 0U){
          CLIRE_BIT(s_b.wifi_ap_connect);
        }
      }
    if(delete_broker()){
        if(GET_BIT(f.mqtt_check))
        {
          if(erase_key_from_nvs (USER_SETTINGS, KEY_MQTT, &handle)){
            CLIRE_BIT(f.mqtt_check);
          }
        }else{
          printf("нету файла mqtt.txt");
        }
    }
    if(delete_ap()){
        if(GET_BIT(f.ssid_check))
        {
          if(erase_key_from_nvs (USER_SETTINGS, KEY_WIFI, &handle)){
            SET_BIT(f.restart_ready);
          }
        }else{
          printf("нету файла ssid.txt");
        }
    }
    if(value != *mem_state_rw(NULL)){
      value = *mem_state_rw(NULL);
      CLIRE_BIT(f.mem_chng);
      SET_BIT(f.mem_chng);
    }
    if(GET_3BIT(f.mem_chng) > 0U && GET_3BIT(f.mem_chng) < 7U){
      INK_3BIT(f.mem_chng);
    }else if(7U == GET_3BIT(f.mem_chng)){
      if(write_u8_to_nvs(USER_SETTINGS, KEY_USER, &handle, &value)){
        CLIRE_BIT(f.mem_chng);
      }
      //printf("f.rr %d f.mqtt %d f.ssid %d f.mem %d readmem %d \n", f.restart_ready, f.mqtt_check, f.ssid_check, f.mem_chng, value);
    }
    
    DELAY_X(200); 
    if(mqtt_sub(NULL)){
      if(!GET_BIT(f.bro_connect)){
        mqtt_sub(CELEVAYA(mqtt_cfg.username,mac_str));
        mqtt_sub(SOSTOYANIE(mqtt_cfg.username,mac_str));
        mqtt_sub(VERSION(mqtt_cfg.username,mac_str));
        SET_BIT(f.bro_connect);
      }
    }else{
      CLIRE_BIT(f.bro_connect);
    }

    if(GET_BIT(f.bro_connect)){
      if((time(NULL)&15U) == 12U && GET_BIT(s_p_b.send_1)){
        if(LEVEL100(rssi)){
          mqtt_pub(AKTIVNOSTI(mqtt_cfg.username,mac_str,"/act/rssi"), "100");
        }else if (LEVEL75(rssi))
        {
          mqtt_pub(AKTIVNOSTI(mqtt_cfg.username,mac_str,"/act/rssi"), "75");
        }else if (LEVEL50(rssi))
        {
          mqtt_pub(AKTIVNOSTI(mqtt_cfg.username,mac_str,"/act/rssi"), "50");
        }else if (LEVEL25(rssi))
        {
          mqtt_pub(AKTIVNOSTI(mqtt_cfg.username,mac_str,"/act/rssi"), "25");
        }else{
          mqtt_pub(AKTIVNOSTI(mqtt_cfg.username,mac_str,"/act/rssi"), "0");
        }
        CLIRE_BIT(s_p_b.send_1);
        SET_BIT(s_p_b.send_2);
        SET_BIT(s_b.wifi_up);
      }
      if((time(NULL)&15U) == 13U && GET_BIT(s_p_b.send_2) && !GET_BIT(s_b.change_celevaya)){
        mqtt_pub(CELEVAYA_GET(mqtt_cfg.username,mac_str), celevaya_str);
        CLIRE_BIT(s_p_b.send_2);
        SET_BIT(s_p_b.send_3);
        SET_BIT(s_b.wifi_up);
      }
      if((time(NULL)&15U) == 14U && GET_BIT(s_p_b.send_3) && !GET_BIT(s_b.rele_change)){
        send_act();
        CLIRE_BIT(s_p_b.send_3);
        SET_BIT(s_p_b.send_4);
        SET_BIT(s_b.wifi_up);
      }
      if((time(NULL)&15U) == 15U && GET_BIT(s_p_b.send_4) && !GET_BIT(s_b.change_sostoyanie)){
        mqtt_pub(SOSTOYANIE_GET(mqtt_cfg.username,mac_str), sostoyanie_str);
        CLIRE_BIT(s_p_b.send_4);
        SET_BIT(s_p_b.send_1);
        SET_BIT(s_b.wifi_up);
      }
    }
    rele_chng(&rele_num);
    ////////////////////////////////////////////
    if(GET_BIT(f.restart_ready)){
      esp_restart();
    }
    ////////////////////////////////////////////
  }
  
  vTaskDelete(NULL);
}
/////////////////////////////////////////////////////////////////////////////////////
static void display_init (void)
{
      // init i2s
    i2c_config_t i2c_conf;
    i2c_conf.mode = I2C_MODE_MASTER;
    i2c_conf.sda_io_num = I2C_MASTER_SDA_IO;
    i2c_conf.sda_pullup_en = 0;
    i2c_conf.scl_io_num = I2C_MASTER_SCL_IO;
    i2c_conf.scl_pullup_en = 0;
    i2c_conf.clk_stretch_tick = 300;
    if(i2c_driver_install(i2c_master_port, i2c_conf.mode) != ESP_OK){
      i2c_driver_install(i2c_master_port, i2c_conf.mode);
    }
    if(i2c_param_config(i2c_master_port, &i2c_conf) != ESP_OK){
      i2c_param_config(i2c_master_port, &i2c_conf);
    }
    nvs_handle handle;
    if(read_u8_from_nvs(USER_SETTINGS, KEY_DISPLAY, &handle, &display_type) && display_type == 0U){
      dev.screen = SSD1306_SCREEN;
    }

    while (ssd1306_init(&dev) != 0) {
        printf("%s: failed to init SSD1306 lcd addr %d\n", __func__, dev.i2c_addr);
        if(dev.i2c_addr == SSD1306_I2C_ADDR_1){
          dev.i2c_addr = SSD1306_I2C_ADDR_2;
        }else if(dev.i2c_addr == SSD1306_I2C_ADDR_2){
          dev.i2c_addr = SSD1306_I2C_ADDR_3;
        }else if(dev.i2c_addr == SSD1306_I2C_ADDR_3){
          dev.i2c_addr = SSD1306_I2C_ADDR_4;
        }else if(dev.i2c_addr == SSD1306_I2C_ADDR_4){
          dev.i2c_addr = SSD1306_I2C_ADDR_1;
        }
        DELAY_X(300);
    }
    printf("SSD1306 lcd addr: %d\n", dev.i2c_addr);
    ssd1306_set_segment_remapping_enabled(&dev,true);
    ssd1306_set_scan_direction_fwd(&dev, false);
    //ssd1306_set_whole_display_lighting(&dev, false);
    ssd1306_display_on(&dev, false);
}
/////////////////////////////////////////////////////////////////////////////////////
void ap_sta_mode_run(wifi_mode_t *apsta_mode)
{
  CLIRE_BIT(f.ssid_check);
  if(init_esp_wifi(apsta_mode,NULL,NULL)){
    if(esp_wifi_get_config(ESP_IF_WIFI_AP, &wifi_cfg) == ESP_OK){
      snprintf(wifi_qr, sizeof(wifi_qr), QRCODE_WIFI,(char *)wifi_cfg.ap.ssid);
    }
  }
}

void app_main(void)
{
  printf("\r\n\r\n");
  printf("SDK version: %d.%d.%d\n", ESP_IDF_VERSION_MAJOR, ESP_IDF_VERSION_MINOR, ESP_IDF_VERSION_PATCH);
  printf("\r\n\r\n");
    knopki_init();
    display_init();
  #if WRITEFS
  //------------------------------------------------------
    esp_vfs_spiffs_unregister(conf.partition_label);
    esp_err_t ret = 0;
    ret = esp_spiffs_format(conf.partition_label);
    if (ret != ESP_OK) {
      printf("format failed, %s\n", esp_err_to_name(ret));
    }

    ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
      printf("mount failed, %s\n", esp_err_to_name(ret));
    }

    fd = fopen("/spiffs/index.html", "w");
    fprintf(fd,"%s",(const char*)data__index_html);
    fclose(fd);
    fd = fopen("/spiffs/mqtt.html", "w");
    fprintf(fd,"%s",(const char*)data__mqtt_html);
    fclose(fd);
    fd = fopen("/spiffs/logo.svg", "w");
    fprintf(fd,"%s",(const char*)data__logo_svg);
    fclose(fd);
    fd = fopen("/spiffs/suicided.html", "w");
    fprintf(fd,"%s",(const char*)data__suicided_html);
    fclose(fd);
    fd = fopen("/spiffs/OTA.html", "w");
    fprintf(fd,"%s",(const char*)data__OTA_html);
    fclose(fd);
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        os_printf("Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        os_printf("SPIFFS: total %d, used %d\n", total, used);
        DIR* dir = opendir("/spiffs");
        if (dir == NULL) {
            return;
        }
        while (true) {
            struct dirent* de = readdir(dir);
            if (!de) {
                break;
            }
            printf("Found file: %s\n", de->d_name);
        }
    }
    esp_vfs_spiffs_unregister(conf.partition_label);
    //------------------------------------------------------
  #else
    //os_printf("Heap free size: %d\n", xPortGetFreeHeapSize());
    os_printf("mount filesystem begin\n");
    esp_err_t ret = 0;
    if(!esp_spiffs_mounted(conf.partition_label)){
        esp_vfs_spiffs_register(&conf);
    }
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
      printf("Failed to initialize SPIFFS (%s)\n", esp_err_to_name(ret));
      return;
    } else {
      printf("Partition size: total: %u, used: %u\n", total, used);
        DIR* dir = opendir("/spiffs");
        if (dir == NULL) {
            return;
        }
        while (true) {
            struct dirent* de = readdir(dir);
            if (!de) {
                break;
            }
            printf("Found file: %s\n", de->d_name);
        }
    }
    if(nvs_flash_init() != ESP_OK){
      nvs_flash_init();
    }
    wifi_mode_t sta_mode = WIFI_MODE_STA;
    wifi_mode_t apsta_mode = WIFI_MODE_APSTA;
    nvs_handle handle;
    if( nvs_open( USER_SETTINGS, NVS_READWRITE, &handle) == ESP_OK){
      if( nvs_get_blob( handle, KEY_WIFI, &wifi_cfg, &wicfg_len) == ESP_OK && (wifi_cfg.sta.ssid[0] != '\0') && (wifi_cfg.sta.password[0] != '\0'))
      {
        //os_printf("ssid: %s  lens: %i pass: %s lenp: %i\n",ssid_out, ssid_len,pass_out, pass_len);
        if(!init_esp_wifi(&sta_mode, (const char *)wifi_cfg.sta.ssid, (const char *)wifi_cfg.sta.password)){
          os_printf("Failed sta start");
          ap_sta_mode_run(&apsta_mode);
        }else{
          SET_BIT(f.ssid_check);
          os_printf("WIFI_STA begin\n");
        }
      } else {
        ap_sta_mode_run(&apsta_mode);
        os_printf("WIFI_AP_STA begin\n");
      }
    }
    nvs_close(handle);
    if(read_u8_from_nvs(USER_SETTINGS, KEY_USER, &handle, &value) && value != 0U){
      mem_state_rw( &value);
    }else{
      value = *mem_state_rw(&defolt);
      write_u8_to_nvs(USER_SETTINGS, KEY_USER, &handle, &value);
    }
    fclose(fd);
    esp_vfs_spiffs_unregister(conf.partition_label);
    xTaskCreate(mainTask, "mainTask", 2048U, NULL, 1U, NULL);
  #endif
}
//------------------------------------------------------
