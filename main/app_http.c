
#include "../main/app_http.h"
#include "../main/wifi.h"
//static const char http_header[] = {"HTTP/1.1 200 OK\r\nServer: nginx\r\nContent-Type: text/html\r\nConnection: keep-alive\r\n\r\n"};

static const char * INDEX_PAGE = 
"<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "<meta http-equiv='X-UA-Compatible' content='ie=edge'>"
    "<title>OTA Update</title>"
    "<style>"
        "body {"
            "background-color: #c4c4c4;"
            "font-family: Arial, Helvetica, sans-serif;"
            "margin: 0;"
            "padding: 0;"
            "height: 100vh;"
            "display: flex;"
            "flex-direction: column;"
            "justify-content: center;"
            "align-items: center;"
        "}"
        "h1 {"
            "padding: 1em 1em 0 1em;"
            "margin: 0;"
        "}"
        "h3 {"
            "margin: 0 0 2em 0;"
        "}"
        "svg {"
            "width: 10em;"
            "height: auto;"
        "}"
        "input, button {"
            "cursor: pointer;"
            "font-size: large;"
        "}"
        "#btn-restart {"
            "display: none;"
        "}"
    "</style>"
    "<script>"
		"function upload(file) {"
		  "let xhr = new XMLHttpRequest();"
		  "xhr.upload.onprogress = function(event) {"
			"console.log(`Отправлено ${event.loaded} из ${event.total}`);"
		  "};"
		  "xhr.onloadend = function() {"
		  "document.getElementById('upload-response').innerHTML = xhr.responseText;"
			"if (xhr.status == 200) {"
			  "console.log('Успех');"
			  "document.getElementById('btn-restart').style.display = 'block';"
			"} else {"
			  "console.log('Ошибка ' + this.status);"
			"}"
		  "};"
		  "xhr.open('POST', '/');"
		  "xhr.send(file);"
		"}"
        "function restart() {"
            "var xhr = new XMLHttpRequest();"
            "xhr.onreadystatechange = function () {"
                "if (this.readyState == 4 && this.status == 200) {"
                    "window.location.reload();"
                "}"
            "};"
            "xhr.open('PUT', '/');"
            "xhr.send();"
        "}"
    "</script>"
"</head>"
"<body>"
    "<div>"
        "<svg xmlns='http://www.w3.org/2000/svg' fill='none' viewBox='0 0 24 24'>"
            "<path xmlns='http://www.w3.org/2000/svg'"
                "d='M11 4C8.79082 4 7 5.79085 7 8C7 8.03242 7.00047 8.06627 7.00131 8.10224C7.01219 8.56727 6.70099 8.97839 6.25047 9.09416C4.95577 9.42685 4 10.6031 4 12C4 13.6569 5.34317 15 7 15H8C8.55228 15 9 15.4477 9 16C9 16.5523 8.55228 17 8 17H7C4.23861 17 2 14.7614 2 12C2 9.93746 3.2482 8.16845 5.02926 7.40373C5.32856 4.36995 7.88746 2 11 2C13.2236 2 15.1629 3.20934 16.199 5.00324C19.4207 5.10823 22 7.75289 22 11C22 14.3137 19.3138 17 16 17C15.4477 17 15 16.5523 15 16C15 15.4477 15.4477 15 16 15C18.2092 15 20 13.2091 20 11C20 8.79085 18.2092 7 16 7C15.8893 7 15.78 7.00447 15.6718 7.01322C15.2449 7.04776 14.8434 6.8066 14.6734 6.4135C14.0584 4.99174 12.6439 4 11 4ZM11.2929 9.29289C11.6834 8.90237 12.3166 8.90237 12.7071 9.29289L14.7071 11.2929C15.0976 11.6834 15.0976 12.3166 14.7071 12.7071C14.3166 13.0976 13.6834 13.0976 13.2929 12.7071L13 12.4142V20C13 20.5523 12.5523 21 12 21C11.4477 21 11 20.5523 11 20V12.4142L10.7071 12.7071C10.3166 13.0976 9.68342 13.0976 9.29289 12.7071C8.90237 12.3166 8.90237 11.6834 9.29289 11.2929L11.2929 9.29289Z'"
                "fill='black'>"
            "</path>"
        "</svg>"
    "</div>"
    "<h1>"
        "Прошивка по Wi-Fi"
    "</h1>"
    "<h3>"
        "Обогреватель"
    "</h3>"
    "<input type='file' onchange='upload(this.files[0])'>"
    "<p id='upload-response'></p>"
    "<button id='btn-restart' onclick='restart()'>Перезапуск</button>"
"</body>"
"</html>";

static const char * UPLOAD_SUCCESS = "Прошивка загрузилась в устройство";
static const char * UPLOAD_FAIL = "Что-то пошло не так";
extern esp_ota_firm_t ota_firm;
/*
static FILE* fd;
static int len = 0U;
static uint8_t logo[2350] = {0};
static uint8_t buf[5000] = {0};
static uint8_t ota_htm[2550] = {0};
static size_t index_page(){
    uint8_t macaddress[6];
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
    //strcpy((char*)buf,http_header);
    len = snprintf((char*)buf + strlen(http_header), sizeof (buf),
                                            (const char *)ota_htm,
                                            (const char *)logo, 
                                            MAC2STR(macaddress));
    if (len < sizeof (buf)){
        buf[len] = '\0';
        return len;
    }
    return 0U;
}
*/
esp_err_t get_handler(httpd_req_t * req)
{
    //if(index_page()){ 
        httpd_resp_send(req, INDEX_PAGE, strlen(INDEX_PAGE));
    //}
    

    return ESP_OK;
}

esp_err_t restart_handler(httpd_req_t * req)
{
    //if(index_page()){ 
        httpd_resp_send(req, INDEX_PAGE, strlen(INDEX_PAGE));
    //}

    //ESP_LOGI(TAG, "Перезагрузка через 3!");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    //ESP_LOGI(TAG, "Перезагрузка через 2!");
    //vTaskDelay(1000 / portTICK_PERIOD_MS);
    //ESP_LOGI(TAG, "Перезагрузка через 1!");
    //vTaskDelay(1000 / portTICK_PERIOD_MS);
    esp_restart();

    return ESP_OK;
}

esp_err_t post_handler(httpd_req_t * req)
{

    //ESP_LOGI(TAG, "Received file upload %d bytes", req->content_len);

    // Basic check
    if (req->method != HTTP_POST) {
        httpd_resp_set_status(req, "400 Bad Request");
        httpd_resp_send(req, UPLOAD_FAIL, strlen(UPLOAD_FAIL));
        return ESP_OK;
    }

    char buf[100];
    size_t ret = 0, remaining = req->content_len;
    while (remaining > 0) {
        // Read data from request
        ret = httpd_req_recv(req, buf, remaining < sizeof(buf) ? remaining : sizeof(buf));

        // Retry receiving if timeout occurred
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            continue;
        } else if (ret <= 0) {
            return ESP_FAIL;
        }

        // Write read bytes into the OTA partition
        if (write_ota(&ota_firm, buf, ret) != ESP_OK) {
            return ESP_FAIL;
        }

        remaining -= ret;
    }

    httpd_resp_set_status(req, "200 OK");
    httpd_resp_send(req, UPLOAD_SUCCESS, strlen(UPLOAD_SUCCESS));

    // End ota_firm and restart
    if (end_ota(&ota_firm) != ESP_OK) {
        //ESP_LOGE(TAG, "Error: end_ota");
        return ESP_FAIL;
    }

    return ESP_OK;
}

void init_http()
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server on default port
    //ESP_LOGI(TAG, "Starting HTTP server on port: '%d'", config.server_port);
    httpd_start(&server, &config);

    // URI handler for root
    httpd_uri_t home = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = get_handler,
        .user_ctx  = NULL
    };
    httpd_register_uri_handler(server, &home);

    // URI handle for upload
    httpd_uri_t upload = {
        .uri       = "/",
        .method    = HTTP_POST,
        .handler   = post_handler,
        .user_ctx  = NULL
    };
    httpd_register_uri_handler(server, &upload);

    // URI handle for restart
    httpd_uri_t restart = {
        .uri       = "/",
        .method    = HTTP_PUT,
        .handler   = restart_handler,
        .user_ctx  = NULL
    };
    httpd_register_uri_handler(server, &restart);

    // Create event loop to handle WiFi and HTTP events
    //ESP_ERROR_CHECK(esp_event_loop_create_default());
}
