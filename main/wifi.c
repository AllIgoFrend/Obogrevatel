#include "../main/wifi.h"
//------------------------------------------------
/* FreeRTOS event group to signal when we are connected*/
//static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */

//EventBits_t bits;
  typedef struct {
    unsigned wifi_sta_connect:1;
    unsigned wifi_ap_connect:1;
    unsigned ap_sta_mode:1;
    unsigned scan_ready_v:1;
    unsigned start_wifi:1;
    unsigned sta_got_ip:1;
    unsigned rezerv:2;
  } event_custom_bit;
wifi_mode_t mode = 0;
static esp_err_t err = 0;
int32_t id = 0;
uint8_t klient = 0;
//bool apsta = false;
//bool scan_ready_v = false;
static event_custom_bit bits = {0U};
static char my_ip[24] = {0U};
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
//------------------------------------------------
char *ret_my_ip()
{
    if(GET_BIT(bits.sta_got_ip)){
        return my_ip;
    }
    return "net ip adresa";
}
//------------------------------------------------
//static bool start_wifi_ch = false;
//------------------------------------------------
static void ICACHE_FLASH_ATTR wifi_event_handler_cb(void* arg, esp_event_base_t event_base,
        int32_t event_id, void* event_data)
{
  if(event_id != id){
  os_printf("[WiFi] event %u\n", event_id);
  id = event_id;
  }
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE) {
    SET_BIT(bits.scan_ready_v);
    os_printf("skandon wtopaniy \n");
  }


  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
    wifi_sta_list_t sta;
	if (esp_wifi_ap_get_sta_list(&sta) == ESP_OK){
                klient = sta.num;
                os_printf("podklu4ino klientov %d\n", sta.num);
    }
      wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
      os_printf("station "MACSTR" join, AID=%d\n",
               MAC2STR(event->mac), event->aid);
    SET_BIT(bits.wifi_ap_connect);
  }
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
    wifi_sta_list_t sta;
	if (esp_wifi_ap_get_sta_list(&sta) == ESP_OK){
                klient = sta.num;
                os_printf("podklu4ino klientov %d\n", sta.num);
    }
      wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
      os_printf("station "MACSTR" leave, AID=%d\n",
               MAC2STR(event->mac), event->aid);
    CLIRE_BIT(bits.wifi_ap_connect);
  }
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {

    if(!GET_BIT(bits.ap_sta_mode)){
        
        tcpip_adapter_dhcp_status_t status = TCPIP_ADAPTER_DHCP_INIT;
        err = tcpip_adapter_dhcpc_get_status(TCPIP_ADAPTER_IF_STA, &status);
        if((err != ESP_OK) || status != TCPIP_ADAPTER_DHCP_STARTED) {
            os_printf("DHCP is not started. Starting it...\n");
            if(tcpip_adapter_dhcpc_start(TCPIP_ADAPTER_IF_STA) == ESP_OK) {
                uint8_t macadr[6] = {0};
                char hostname[32] = {0};
                if(esp_wifi_get_mac(ESP_IF_WIFI_STA, macadr) != ESP_OK){
                    os_printf("mak addr ne dayot!\n");
                }
                snprintf(hostname, sizeof(hostname), "Electrolux_%02x%02x%02x", (macadr)[3],(macadr)[4],(macadr)[5] );
                if(tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, (const char *)hostname) != ESP_OK){
                    esp_restart();
                }
                
            }else{
                os_printf("DHCP start failed!\n");
            }
        }
        err = esp_wifi_connect();}
    if(err != ESP_OK){
                os_printf("wifi konekt %s\n", esp_err_to_name(err));
            }
  }
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
    SET_BIT(bits.wifi_sta_connect);
  }
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {

        system_event_sta_disconnected_t *event = (system_event_sta_disconnected_t *)event_data;

        if (event->reason == WIFI_REASON_BASIC_RATE_NOT_SUPPORT) {
            /*Switch to 802.11 bgn mode */
                err = esp_wifi_set_protocol(ESP_IF_WIFI_STA,WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N);
                    if(err != ESP_OK){
                        os_printf("wifi 11bgn obosratuwki %s\n", esp_err_to_name(err));
                    }
        }

        os_printf("wifi prichina disconnecta %d\n", event->reason);

    CLIRE_BIT(bits.wifi_sta_connect);
    os_printf("connect to the AP fail\n");
    err = tcpip_adapter_up(TCPIP_ADAPTER_IF_STA);
    if(err != ESP_OK){os_printf("adapter up %s\n", esp_err_to_name(err));}
  }
  if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    snprintf(my_ip, sizeof(my_ip), WIFI_STA_HTTP, ip4addr_ntoa(&event->ip_info.ip));
    SET_BIT(bits.sta_got_ip);
  }
  if (event_base == IP_EVENT && event_id == IP_EVENT_STA_LOST_IP) {
    CLIRE_BIT(bits.sta_got_ip);
  }
}
//------------------------------------------------
bool ICACHE_FLASH_ATTR wifi_station_connected(int8_t *rssi)
{
    if (GET_BIT(bits.wifi_sta_connect)) {
        if(rssi != NULL){
            wifi_ap_record_t ap_inf;
            esp_wifi_sta_get_ap_info(&ap_inf);
            *rssi = ap_inf.rssi;
            //os_printf( "connect bit\n"); //( "connected rssi %i\n", *rssi);
        }
        if(GET_BIT(bits.sta_got_ip)){
            return true;
        }
        return false;
    } else {
        //os_printf( "disconnect bit\n");
        wifi_config_t conf;

        if(esp_wifi_get_config(ESP_IF_WIFI_STA, &conf) != ESP_OK){
           // os_printf("Failed to GET Station config!\n");
            return false;
        }
        //os_printf("Failed to connect %s     %s\n",conf.sta.ssid ,conf.sta.password);

        vTaskDelay(5000 / portTICK_RATE_MS);

        if(!GET_BIT(bits.ap_sta_mode)){
            err = esp_wifi_connect();
            if(err != ESP_OK){
               // os_printf("wifi konekt err %s\n", esp_err_to_name(err));
            }
        }
        return false;
    }
}
//------------------------------------------------
bool ICACHE_FLASH_ATTR scan_ready()
{
    if(!GET_BIT(bits.scan_ready_v)){
        return false;
    }
    CLIRE_BIT(bits.scan_ready_v);
    return true;
}
//------------------------------------------------
u8_t ICACHE_FLASH_ATTR wifi_ap_client()
{
    return klient;
}
//------------------------------------------------
bool ICACHE_FLASH_ATTR stop_wifi()
{
    if(esp_wifi_set_mode(WIFI_MODE_NULL)!=ESP_OK)
    {
        os_printf("Failed to disable STA mode!\n");
        os_printf("Failed to disable AP mode!\n");
        return false;
    }
    if(esp_wifi_stop()==ESP_OK){
        CLIRE_BIT(bits.start_wifi);
    }else{
        os_printf("Failed stop wifi!\n");
        return false;
    }
    return true;
}
//------------------------------------------------
bool ICACHE_FLASH_ATTR wifi_startind()
{
    if(!GET_BIT(bits.start_wifi)){
        err = esp_wifi_start();
        if(err!=ESP_OK){
            os_printf("Failed start wifi! %s\n", esp_err_to_name(err));
            return false;
        }else{
            SET_BIT(bits.start_wifi);
        }
    }
    return true;
}
//------------------------------------------------
bool ICACHE_FLASH_ATTR start_wifi_station(const char * ssid, const char * pass)
{

    if(esp_wifi_set_mode(WIFI_MODE_STA)!=ESP_OK)
    {
        os_printf("Failed to enable Station mode!\n");
        return false;
    }
    if(esp_wifi_get_mode(&mode) == ESP_OK)
    {
        if((mode & WIFI_MODE_STA) == 0)
        {
        os_printf("Failed to set Station mode!\n");
        return false;
        }
    }
    if(!ssid)
    {
        os_printf("No SSID Given\n");
        return false;
    }

    if(pass)
    {
    
    wifi_config_t conf = {
            .sta = {
                .scan_method = WIFI_FAST_SCAN,
                .channel = 0,
                .bssid_set = 0
            },
    };
    memcpy(conf.sta.ssid, ssid, sizeof(conf.sta.ssid));
    memcpy(conf.sta.password, pass, sizeof(conf.sta.password));

        if(esp_wifi_set_config(ESP_IF_WIFI_STA, &conf) != ESP_OK){
            os_printf("Failed to set Station config!\n");
            return false;
        }
    }
    wifi_ps_type_t type;
    if(esp_wifi_get_ps(&type) == ESP_OK){
        if(type != WIFI_PS_NONE){
            if( esp_wifi_set_ps(WIFI_PS_NONE) != ESP_OK){
                os_printf("Set ps falos\n");
            }
        }
    }else{
        os_printf("get ps falos\n");
    }


    
    CLIRE_BIT(bits.ap_sta_mode);

    return wifi_startind();
}
//------------------------------------------------
bool ICACHE_FLASH_ATTR start_wifi_apsta()
{
    tcpip_adapter_ip_info_t ipinfo;
    memset(&ipinfo, 0, sizeof(ipinfo));
    uint8_t macadr[6] = {0};
    esp_wifi_get_mac(ESP_IF_WIFI_AP, macadr);
    uint8_t ssid[32] = {0};
    snprintf((char *)ssid, sizeof(ssid), WIFI_APSSID,macadr[3],macadr[4],macadr[5]);
    wifi_config_t config = {
	        .ap = {
	            //.ssid = {(unsigned char *)ssid},
	            .ssid_len = strlen(WIFI_APSSID),
	            .password = WIFI_APPASSWORD,
				.channel = 7,
	            .max_connection = 10,
				.ssid_hidden = 0,
	            .authmode = WIFI_AUTH_WPA_WPA2_PSK
	        },
	    };
    memcpy(config.ap.ssid, ssid, sizeof(config.ap.ssid));
    if(esp_wifi_set_mode(WIFI_MODE_APSTA) != ESP_OK)
    {
        os_printf("Failed to enable Access Point mode!\n");
        return false;
    }
    err = esp_wifi_set_config(ESP_IF_WIFI_AP, &config);
    if(err != ESP_OK){
        os_printf("set ap config obosratuwki %s\n", esp_err_to_name(err));
    }
    err = esp_wifi_get_mode(&mode);
    if ((err  == ESP_OK)&&(mode == WIFI_MODE_APSTA))
    {
        os_printf("APSTA_MODE\n");
        if(esp_wifi_get_config(ESP_IF_WIFI_AP, &config) == ESP_OK)
            {
                os_printf("OPMODE: %u, SSID: %s, PASSWORD: %s, CHANNEL: %d, AUTHMODE: %d, MACADDRESS: "MACSTR"\n",
                    mode,
                    config.ap.ssid,
                    config.ap.password,
                    config.ap.channel,
                    config.ap.authmode,
                    MAC2STR(macadr));
            }
    }

  tcpip_adapter_dhcp_status_t status = TCPIP_ADAPTER_DHCP_INIT;
  //free(&config);
  //esp_wifi_fpm_set_sleep_type(WIFI_NONE_SLEEP_T);
  err = tcpip_adapter_dhcps_get_status(TCPIP_ADAPTER_IF_AP, &status);
    if((err == ESP_OK)&&(status != TCPIP_ADAPTER_DHCP_STOPPED)){
        tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);
        os_printf("DHCP stopped\n");
    }else{
        os_printf("DHCP status %d\n", status);
    }
    if(!ip4addr_aton(WIFI_AP_IP, &ipinfo.ip)){os_printf("ip hui");}
    if(!ip4addr_aton(WIFI_AP_GW, &ipinfo.gw)){os_printf("gw hui");}
    if(!ip4addr_aton(WIFI_AP_NETMASK, &ipinfo.netmask)){os_printf("nm hui");}
  err = tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &ipinfo);
  if(err != ESP_OK){
    os_printf("tcp adapter ustanovka ip info obosratuwki %s\n", esp_err_to_name(err));
  }

  dhcps_lease_t dhcp_lease;
  dhcp_lease.enable =1;
  dhcp_lease.start_ip.addr = ipaddr_addr(WIFI_AP_IP_CLIENT_START);
  dhcp_lease.end_ip.addr = ipaddr_addr(WIFI_AP_IP_CLIENT_END);

  err = tcpip_adapter_dhcps_option(TCPIP_ADAPTER_OP_SET,
		  TCPIP_ADAPTER_REQUESTED_IP_ADDRESS,
		  &dhcp_lease,
		  sizeof(dhcps_lease_t));
    if(err != ESP_OK){
                os_printf("tcp adapter dhcp optionc obosratuwki %s\n", esp_err_to_name(err));
          }
  err = tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP);
    if(err != ESP_OK){
                os_printf("tcp adapter dhcp start obosratuwki %s\n", esp_err_to_name(err));
            }
  //if(wifi_get_phy_mode() != PHY_MODE_11N) //переделать
  //  wifi_set_phy_mode(PHY_MODE_11N); //переделать
      //if(tcpip_adapter_is_netif_up(TCPIP_ADAPTER_IF_AP)){
    	  if(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ipinfo) != ESP_OK)
    	  {
    	    os_printf("Failed get ip info\n");
    	    }else{
            os_printf("ip info.ip %s\n", ip4addr_ntoa(&ipinfo.ip));
            os_printf("ip info.nm %s\n", ip4addr_ntoa(&ipinfo.netmask));
            os_printf("ip info.gw %s\n ", ip4addr_ntoa(&ipinfo.gw));
            }
    //}
    SET_BIT(bits.ap_sta_mode);
    return wifi_startind();
}
//------------------------------------------------
bool ICACHE_FLASH_ATTR init_esp_wifi(wifi_mode_t *mode_m, const char * ssid, const char * pass)
{
    //s_wifi_event_group = xEventGroupCreate();
    if(esp_netif_init() != ESP_OK){
        os_printf("Esp inif falos\n");
    }
  tcpip_adapter_init();
  esp_event_loop_create_default();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler_cb, NULL);
  esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler_cb, NULL);
  esp_wifi_set_mode(*mode_m);
  if(GET_BIT(bits.start_wifi)){
    if(!stop_wifi()){
    os_printf("Failed wifi init!\n");
    }
  }
  if (*mode_m == WIFI_MODE_APSTA){
    return start_wifi_apsta();
  }
  if (*mode_m == WIFI_MODE_STA){
    return start_wifi_station(ssid, pass);
  }
  return false;
  //esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler_cb);
  //esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler_cb);
  //vEventGroupDelete(s_wifi_event_group);
}
//------------------------------------------------
