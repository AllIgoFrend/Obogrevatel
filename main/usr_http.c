#include "../main/usr_http.h"
//------------------------------------------------
static const char http_header[] = {"HTTP/1.1 200 OK\r\nServer: nginx\r\nContent-Type: text/html\r\nConnection: keep-alive\r\n\r\n"};

const char *lostpage = {
  "<html><head><style>"
    "svg {width: 400px;}"
  "</style>"
    "<center>%s</center>"
  "</head><body><center><p>"
      "<h1><font style=\"text-align: center;\">=^_^=</font></h1>"
  "</p></center></body></html>"
};
//------------------------------------------------
static uint8_t index_htm[1024] = {0};
static uint8_t mqtt_htm[1150] = {0};
static uint8_t logo[2500] = {0};
static uint8_t suicided_htm[1000] = {0};
static char get_ssid[1024] = {0};
static char suicided[64] = {0};
static int res = 0;
static struct sockaddr_in servaddr, cliaddr, remotehost;
static uint8_t buf[8192] = {0};
static int buflen = sizeof(buf);
static int sockfd, accept_sock, ret;
static socklen_t sockaddrsize;
static esp_err_t err = 0;
static bool restart_now = false;
FILE* fd;
//------------------------------------------------
uint8_t *convert_to_hex( unsigned int num, unsigned int bit ) //convert int to binary, bit is number of digit
{
	uint8_t str_bin[bit];
  uint8_t *hex_num;
	hex_num = ( uint8_t* )calloc( bit + 1, sizeof( char ) ); //bin str 0-7 -  bit

	if ( hex_num == NULL )
	{
		exit(1);
	}

	memset( str_bin, ( char )( ( int )'0' ), bit ); //initializing array
	str_bin[bit] = '\0';

	bit--;
	while ( num > 0 )
	{
		str_bin[bit] += num & 1U;
		bit--;
		num >>= 1U;
	}
  for (int i = 0; str_bin[i] != '\0'; i++)
  {
    if (str_bin[i] == '1' )
    {
      *hex_num |= (1 << (7-i));
    }
  }
	return  hex_num;
}
/* Converts a hex character to its integer value */
char from_hex(char ch) {
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}
/* Returns a url-decoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_decode(char *str) {
  char *pstr = str, *buf = malloc(strlen(str) + 1), *pbuf = buf;
  while (*pstr) {
    if (*pstr == '%') {
      if (pstr[1] && pstr[2]) {
        *pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
        pstr += 2;
      }
    } else if (*pstr == '+') { 
      *pbuf++ = ' ';
    } else {
      *pbuf++ = *pstr;
    }
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}

uint8_t *char_to_utf8( char *str ) //convert to utf8
{
	int i;
	uint8_t *utf8_hex;
	int length;

	utf8_hex = ( uint8_t* )calloc( strlen( str ) * 2 + 1, sizeof( char ) ); //for bin str

	if ( utf8_hex == NULL )
	{
		printf( "Can't allocate memory\n" );
    return 0;
	}else{
    int hex_len = 0;
    length = strlen( str );
    for ( i = 0; i < length; i++ )
    {
      uint8_t *hex = convert_to_hex( str[i], 8 );
      int temp_len = snprintf((char *)utf8_hex + hex_len, 3 * sizeof( char ), (const char *)hex,",");
      hex_len += temp_len;
      free(hex);
    }
    utf8_hex[strlen( str ) * 3] = '\0'; //end of string
  }
  return utf8_hex;
}
void suicided_utf8(char *message)
{
  uint8_t *hex = char_to_utf8(message);
  strcpy(suicided, (const char*)hex);
  free(hex);
}
//------------------------------------------------
static bool ICACHE_FLASH_ATTR txt_create(char input[128], char name[15])
{
  static char tmp[128] = {0};
  char *hex = url_decode(input);
  strcpy(tmp, (const char*)hex);
  free(hex);
  nvs_handle handle;
  if(nvs_open( USER_SETTINGS, NVS_READWRITE, &handle) == ESP_OK){
    if(!strcmp(KEY_WIFI, (const char *)name) ){
      wifi_config_t compars, wifi_config_to_store = {
            .sta = {
            .scan_method = WIFI_FAST_SCAN,
            .channel = 0U,
            .bssid_set = 0U
        },
      };
      size_t cmprs_len = sizeof(compars);
      if(tmp[0] != 0U){
        char *ssid_mem = strstr(tmp,"ssid=");
        char *pass_mem = strstr(tmp,"&pass=");
        char *konec = strstr(tmp,"&save=");
        int ssid_len = strlen(ssid_mem)-strlen(pass_mem)-strlen("ssid=");
        int pass_len = strlen(pass_mem)-strlen(konec)-strlen("&pass=");
        memcpy(wifi_config_to_store.sta.ssid, ssid_mem + strlen("ssid="),ssid_len);
        memcpy(wifi_config_to_store.sta.password, pass_mem + strlen("&pass="),pass_len);
      }else{
        suicided_utf8("Настройки пустые");
        nvs_close(handle);
        return false;
      }
      if( nvs_get_blob( handle, KEY_WIFI, &compars, &cmprs_len) != ESP_OK && 
                      ((compars.sta.ssid != wifi_config_to_store.sta.ssid) || 
                      (compars.sta.password != wifi_config_to_store.sta.password)) ){
        if( nvs_set_blob( handle, KEY_WIFI, &wifi_config_to_store, sizeof(wifi_config_to_store)) == ESP_OK){
          if( nvs_commit(handle) == ESP_OK){
            suicided_utf8("Настройки сохранены");
            nvs_close(handle);
            return true;
          }
        }
        suicided_utf8("Настройки не записались");
        nvs_close(handle);
        return false;
      }else{
        suicided_utf8("Такие настройки уже есть");
        nvs_close(handle);
        return true;
      }
    }
    if(!strcmp(KEY_MQTT, (const char *)name) ){
      uint8_t macaddress[6] = {0U};
      char username[16] = {0U};
      char *mqtt_mem = strstr(tmp,"Broker=");
      char *pass_mem = strstr(tmp,"&pass=");
      char *konec = strstr(tmp,"&save=");
      int mqtt_len = strlen(mqtt_mem)-strlen(pass_mem)-strlen("Broker=");
      int pass_len = strlen(pass_mem)-strlen(konec)-strlen("&pass=");
      esp_wifi_get_mac(ESP_IF_WIFI_STA, macaddress);
      snprintf(username, sizeof(username), "%02x%02x%02x%02x%02x%02x", MAC2STR(macaddress));
      for (int i = 0; username[i] > 0U; i++){
        username[i] = (char)toupper(username[i]);
      }
      //esp_mqtt_client_config_t compars, mqtt_cfg;
      if(tmp[0] != 0U){
        //memcpy(mqtt_mem, mqtt_mem + strlen("Broker="),mqtt_len);
        //memcpy(pass_mem, pass_mem + strlen("&pass="),pass_len);
        //memcpy((char *)mqtt_username, username, strlen(username));
        for (int i = 0; i < mqtt_len; i++){
          mqtt_mem[i] = mqtt_mem[strlen("Broker=") + i];
        }
        mqtt_mem[mqtt_len] = '\0';
        for (int i = 0; i < pass_len; i++){
          pass_mem[i] = pass_mem[strlen("&pass=") + i];
        }
        pass_mem[pass_len] = '\0';
      }else{
        suicided_utf8("Настройки пустые");
        nvs_close(handle);
        return false;
      }
      mqtt_user_conf  mqtt_cfg;
      memcpy(mqtt_cfg.uri, mqtt_mem, sizeof(mqtt_cfg.uri));
      memcpy(mqtt_cfg.username, username, sizeof(mqtt_cfg.username));
      memcpy(mqtt_cfg.password, pass_mem, sizeof(mqtt_cfg.password));
      if( nvs_set_blob( handle, KEY_MQTT, &mqtt_cfg, sizeof(mqtt_cfg)) == ESP_OK){
        if( nvs_commit(handle) == ESP_OK){
          suicided_utf8("Настройки сохранены");
          nvs_close(handle);
          return true;
        }
      }
      suicided_utf8("Настройки не записались");
      nvs_close(handle);
      return false;
    }
  }
  suicided_utf8("Неизвестные настройки"); 
  nvs_close(handle);   
  return false;
}
//------------------------------------------------
bool create_socket()
{
  printf("Create socket...\n");
  
  //xTaskHandle recv_handle = NULL;
  if ( (sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) < 0 ) {
    printf("socket not created\n");
    return false;
  }
  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));
  //Заполнение информации о сервере
  servaddr.sin_family    = AF_INET; // IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(SERVER_PORT);
  //Свяжем сокет с адресом сервера
  if (bind(sockfd, (const struct sockaddr *)&servaddr,  sizeof(struct sockaddr_in)) < 0 )
  {
    printf("socket not binded\n");
    return false;
  }
  printf("socket binded\n");
  listen(sockfd, 1);
  return true;
}
bool scan_ap()
{
  //vTaskSuspend(xHandle_ap_reg_task);
    uint16_t numer = 0;
    wifi_ap_record_t ap_records[MAXIMUM_AP];
    wifi_scan_config_t config = {
    	.ssid = NULL,               /**< SSID of AP */
    	.bssid = NULL,              /**< MAC address of AP */
    	.channel = 0,
    	.show_hidden = false,
		.scan_type = WIFI_SCAN_TYPE_PASSIVE,
		.scan_time.passive = 150
    };
    err = esp_wifi_scan_start(&config, true);
    if(err != ESP_OK)
      {
        printf("scan start: %s\n", esp_err_to_name(err));
        //vTaskResume(xHandle_ap_reg_task);
        return false;
      }
  
  if(scan_ready())
  {
    err = esp_wifi_scan_stop();
    if(err != ESP_OK){
      printf("scan stop: %s\n", esp_err_to_name(err));
      //vTaskResume(xHandle_ap_reg_task);
      return false;
    }
    err = esp_wifi_scan_get_ap_num(&numer);
    if(err != ESP_OK){
      printf("scan get ap num: %s\n", esp_err_to_name(err));
      //vTaskResume(xHandle_ap_reg_task);
      return false;
    }else{
      printf(" AP_kolichestvo: %d\n", numer);
      if(numer >MAXIMUM_AP){
        numer = MAXIMUM_AP;
      }
      err = esp_wifi_scan_get_ap_records(&numer, ap_records);
      if(err != ESP_OK){
        printf("scan get ap records: %s\n", esp_err_to_name(err));
        //vTaskResume(xHandle_ap_reg_task);
        return false;
      }else{
        memset(get_ssid, 0,sizeof(get_ssid));
        uint16_t i = 0;
        int ssid_len = 0;
        while (numer != i)
        {
          char	ssid[33] ;
          strcpy(ssid, (const char *)ap_records[i].ssid);
          //printf("ssid: %s\n", ssid);
          const char select[] = {"<option value=\"%s\">%s</option>"};
          int temp_len = snprintf(get_ssid + ssid_len, sizeof(get_ssid), select, ssid, ssid);
          ssid_len += temp_len;
          i += 1;
        }
      }
    }
  }else{
    //vTaskResume(xHandle_ap_reg_task);
    return false;
  }
  //vTaskResume(xHandle_ap_reg_task);
  return true;
}

void ICACHE_FLASH_ATTR mqtt_reg_task(void *pvParameters)
{
  if(!create_socket()){
    vTaskDelete(NULL);
  }
  if(!esp_spiffs_mounted(conf.partition_label)){
    esp_vfs_spiffs_register(&conf);
  }
  fd = fopen("/spiffs/mqtt.html", "r");
  res = fread((void*)mqtt_htm,1, sizeof(mqtt_htm),fd);
  fclose(fd);
  mqtt_htm[res] = 0;
  fd = fopen("/spiffs/logo.svg",  "r");
  res = fread((void*)logo,1, sizeof(logo),fd);
  fclose(fd);
  fd = fopen("/spiffs/suicided.html","r");
  res = fread((void*)suicided_htm,1, sizeof(suicided_htm),fd);
  fclose(fd);
  suicided_htm[res] = 0;
  uint8_t macaddress[6];
  esp_wifi_get_mac(ESP_IF_WIFI_AP, macaddress); 
    while (wifi_station_connected(NULL))
    {   
      accept_sock = accept(sockfd, (struct sockaddr *)&cliaddr, (socklen_t *)&sockaddrsize);
      printf(" socket: %d\n", accept_sock);

      if(accept_sock >= 0)
        {
          memset(buf,	0,	sizeof (buf));
          ret = recvfrom(accept_sock, buf, buflen, 0, (struct sockaddr *)&remotehost, &sockaddrsize);
          if(ret > 0)
          {
            if ((ret >=5 ) && (strncmp((const char*)buf, "GET /", 5) == 0))
            {
              if ((strncmp((char const *)buf,"GET / ",6)==0)||(strncmp((char const *)buf,"GET /index.html",15)==0))
              {
                //printf("%s\n", (char*)index_htm);
                strcpy((char*)buf,http_header);
                int len = snprintf((char*)buf + strlen(http_header), sizeof (buf),
                                                        (const char *)mqtt_htm,
                                                        (const char *)logo, 
                                                        MAC2STR(macaddress));
                if (len < sizeof (buf)){
                    write(accept_sock, (const unsigned char*)buf, strlen(http_header) + sizeof(mqtt_htm) + sizeof(logo));}
              }
            }
            if ((ret >=5 ) && (strncmp((const char*)buf, "POST /", 6) == 0))
            {
              if ((strncmp((char const *)buf,"POST /get ",10)==0)||(strncmp((char const *)buf,"POST /index.html",16)==0))
              {
                char *start = strstr((const char *)buf,"Broker");
                //strcpy(tmp,start);
                if (!txt_create(start, KEY_MQTT)){
                  printf("ne zapisali%s\n",(char *)start);
                }
                memset(buf,	0,	sizeof (buf));
                strcpy((char*)buf,http_header);
                int len = snprintf((char *)buf + strlen(http_header), sizeof (buf),
                                                        (const char *)suicided_htm,
                                                        (const char *)logo, 
                                                        MAC2STR(macaddress),
                                                        suicided);
                if (len < sizeof (buf)){
                write(accept_sock, (const unsigned char*)buf, strlen(http_header) + sizeof(suicided_htm) + sizeof(logo));}
              }
              if ((strncmp((char const *)buf,"POST /reboot ",13)==0)||(strncmp((char const *)buf,"POST /index.html",16)==0))
              {
                memset(buf,	0,	sizeof (buf));
                strcpy((char*)buf,http_header);
                int len = snprintf((char *)buf + strlen(http_header), sizeof (buf),lostpage, (const char *)logo);
                if (len < sizeof (buf))
                    write(accept_sock, (const unsigned char*)buf, strlen(http_header) + strlen(lostpage) + sizeof(logo));
                vTaskDelay(1000/portTICK_PERIOD_MS);
                restart_now = true;
              }
            }
          }
        close(accept_sock);
        }
      if(restart_now){
        esp_restart();
      }
    }
  
  close(sockfd);
  
  esp_vfs_spiffs_unregister(conf.partition_label);
  vTaskDelete(NULL);


}

void ICACHE_FLASH_ATTR ota_task(void *pvParameters)
{
  if(!create_socket()){
    vTaskDelete(NULL);
  }
  //static int len = 0U;
  static uint8_t macaddress[6];
  static uint8_t ota_htm[2550] = {0};
  esp_wifi_get_mac(ESP_IF_WIFI_STA, macaddress); 
  if(!esp_spiffs_mounted(conf.partition_label)){
      esp_vfs_spiffs_register(&conf);
  }
  fd = fopen("/spiffs/OTA.html", "r");
  int res = fread((void*)ota_htm,1, sizeof(ota_htm),fd);
  fclose(fd);
  ota_htm[res] = 0;
  fd = fopen("/spiffs/logo.svg",  "r");
  res = fread((void*)logo,1, sizeof(logo),fd);
  fclose(fd);
  esp_vfs_spiffs_unregister(conf.partition_label);
  while (wifi_station_connected(NULL))
    {   
      accept_sock = accept(sockfd, (struct sockaddr *)&cliaddr, (socklen_t *)&sockaddrsize);
      printf(" socket: %d\n", accept_sock);

      if(accept_sock >= 0)
        {
          memset(buf,	0,	sizeof (buf));
          ret = recvfrom(accept_sock, buf, buflen, 0, (struct sockaddr *)&remotehost, &sockaddrsize);
          if(ret > 0)
          {
            printf("BUF %s\n", buf);
            if ((ret >=5 ) && (strncmp((const char*)buf, "GET /", 5) == 0))
            {
              if ((strncmp((char const *)buf,"GET / ",6)==0)||(strncmp((char const *)buf,"GET /index.html",15)==0))
              {
                //printf("%s\n", (char*)index_htm);
                strcpy((char*)buf,http_header);
                int len = snprintf((char*)buf + strlen(http_header), sizeof (buf),
                                                        (const char *)ota_htm,
                                                        (const char *)logo, 
                                                        MAC2STR(macaddress));
                if (len < sizeof (buf)){
                    write(accept_sock, (const unsigned char*)buf, strlen(http_header) + sizeof(ota_htm) + sizeof(logo));}
              }
            }
            if ((ret >=5 ) && (strncmp((const char*)buf, "POST /", 6) == 0))
            {
              if ((strncmp((char const *)buf,"POST /",6)==0)||(strncmp((char const *)buf,"POST /index.html",16)==0))
              {
                printf("BUF %s\n", buf);
                //char *start = strstr((const char *)buf,"upload=");
                extern esp_ota_firm_t ota_firm;
                    size_t ret = 0, remaining = 800000;
                    while (remaining > 0) {
                        // Read data from request
                        ret = read(accept_sock, buf, remaining < sizeof(buf) ? remaining : sizeof(buf));

                        // Write read bytes into the OTA partition
                        write_ota(&ota_firm, (char *)buf, ret);

                        remaining -= ret;
                    }
                //printf("start %s\n", start);
                //printf("BUF %s\n", buf);
                //strcpy(tmp,start);
              }
            }
          }
        close(accept_sock);
        }
      if(restart_now){
        esp_restart();
      }
    }
  
  close(sockfd);
  vTaskDelete(NULL);
}

void ICACHE_FLASH_ATTR ap_reg_task(void *pvParameters)
{
  if(!create_socket()){
    vTaskDelete(NULL);
  }
  if(!esp_spiffs_mounted(conf.partition_label)){
    esp_vfs_spiffs_register(&conf);
  }
  fd = fopen("/spiffs/index.html", "r");
  res = fread((void*)index_htm,1, sizeof(index_htm),fd);
  fclose(fd);
  index_htm[res] = 0;
  fd = fopen("/spiffs/logo.svg",  "r");
  res = fread((void*)logo,1, sizeof(logo),fd);
  fclose(fd);
  fd = fopen("/spiffs/suicided.html","r");
  res = fread((void*)suicided_htm,1, sizeof(suicided_htm),fd);
  fclose(fd);
  suicided_htm[res] = 0;
  uint8_t macaddress[6];
  esp_wifi_get_mac(ESP_IF_WIFI_AP, macaddress); 
    while (wifi_ap_client())
    {   
      accept_sock = accept(sockfd, (struct sockaddr *)&cliaddr, (socklen_t *)&sockaddrsize);
      printf(" socket: %d\n", accept_sock);

      if(accept_sock >= 0)
        {
          memset(buf,	0,	sizeof (buf));
          ret = recvfrom(accept_sock, buf, buflen, 0, (struct sockaddr *)&remotehost, &sockaddrsize);
          if(ret > 0)
          {
          //printf("%s\n", (char*)buf);
            if ((ret >=5 ) && (strncmp((const char*)buf, "GET /", 5) == 0))
            {
              if ((strncmp((char const *)buf,"GET / ",6)==0)||(strncmp((char const *)buf,"GET /index.html",15)==0))
              {
                if(!scan_ap()){
                  vTaskDelay(2000/portTICK_PERIOD_MS);
                  if(!scan_ap()){
                    vTaskDelete(NULL);
                  }
                }
                //printf("%s\n", (char*)index_htm);
                strcpy((char*)buf,http_header);
                int len = snprintf((char*)buf + strlen(http_header), sizeof (buf),
                                                        (const char *)index_htm,
                                                        (const char *)logo, 
                                                        MAC2STR(macaddress),
                                                        (char *) get_ssid);
                //memcpy((void*)(buf + strlen(http_header)),(void*)index_htm,sizeof(index_htm));
                //write(accept_sock, (const unsigned char*)buf, strlen(http_header) + sizeof(index_htm));
                
                /* Generate response in JSON format */
                //char response[5000];
                if (len < sizeof (buf)){
                    write(accept_sock, (const unsigned char*)buf, strlen(http_header) + sizeof(index_htm) + sizeof(logo) + sizeof(get_ssid));}
                memset(get_ssid, 0,sizeof(get_ssid));
              }
            }
            if ((ret >=5 ) && (strncmp((const char*)buf, "POST /", 6) == 0))
            {
              if ((strncmp((char const *)buf,"POST /get ",10)==0)||(strncmp((char const *)buf,"POST /index.html",16)==0))
              {
                char *start = strstr((const char *)buf,"ssid");
                //strcpy(tmp,start);
                if (!txt_create(start, KEY_WIFI)){
                  printf("ne zapisali%s\n",(char *)start);
                }
                memset(buf,	0,	sizeof (buf));
                strcpy((char*)buf,http_header);
                int len = snprintf((char *)buf + strlen(http_header), sizeof (buf),
                                                        (const char *)suicided_htm,
                                                        (const char *)logo, 
                                                        MAC2STR(macaddress),
                                                        suicided);
                if (len < sizeof (buf)){
                write(accept_sock, (const unsigned char*)buf, strlen(http_header) + sizeof(suicided_htm) + sizeof(logo));}
              }
              if ((strncmp((char const *)buf,"POST /reboot ",13)==0)||(strncmp((char const *)buf,"POST /index.html",16)==0))
              {
                memset(buf,	0,	sizeof (buf));
                strcpy((char*)buf,http_header);
                int len = snprintf((char *)buf + strlen(http_header), sizeof (buf),lostpage, (const char *)logo);
                if (len < sizeof (buf))
                    write(accept_sock, (const unsigned char*)buf, strlen(http_header) + strlen(lostpage) + sizeof(logo));
                vTaskDelay(1000/portTICK_PERIOD_MS);
                restart_now = true;
              }
            }
          }
        close(accept_sock);
        }
      if(restart_now){
        esp_restart();
      }
    }
  
  close(sockfd);
  
  esp_vfs_spiffs_unregister(conf.partition_label);
  vTaskDelete(NULL);
}
