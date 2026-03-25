#include "../main/drow.h"

#define PICTURE_WIDS 16U
#define PICTURE_HIGH 13U

#define CELEVAYA_TEMPER_WIDS 32U

void drowing_wifi(const ssd1306_t *dev, uint8_t *buffer, uint8_t x, uint8_t y, uint8_t link, int8_t *rssi, bool invert){

    char wifi_level = WiFi_100;
    if(rssi != NULL){
        if(LEVEL100(*rssi)){
            wifi_level = link;
        }else if (LEVEL75(*rssi))
        {
            wifi_level = link + 1U; 
        }else if (LEVEL50(*rssi))
        {
            wifi_level = link + 2U;
        }else if (LEVEL25(*rssi))
        {
            wifi_level = link + 3U;
        }
    }
    
    if(invert){
        ssd1306_draw_char(dev, buffer, font_builtin_fonts[FONT_FACE_PICTURES], x, y, PICTURE_WIDS, wifi_level, OLED_COLOR_BLACK, OLED_COLOR_WHITE);
    }else{
        ssd1306_draw_char(dev, buffer, font_builtin_fonts[FONT_FACE_PICTURES], x, y, PICTURE_WIDS, wifi_level, OLED_COLOR_WHITE, OLED_COLOR_BLACK); //пишем надпись в выставленной позиции белым цветом. 
    }
    
    //ssd1306_load_frame_buffer(dev, buffer);
}

void drowing_wifi_ap(const ssd1306_t *dev, uint8_t *buffer, uint8_t x, uint8_t y){

    ssd1306_fill_circle(dev, buffer, x, y, 3U, OLED_COLOR_WHITE); 
    //ssd1306_load_frame_buffer(dev, buffer);
}
void clire_wifi(const ssd1306_t *dev, uint8_t *buffer, uint8_t x, uint8_t y){

    ssd1306_fill_rectangle(dev, buffer, x, y, PICTURE_WIDS, PICTURE_HIGH, OLED_COLOR_BLACK);
    //ssd1306_load_frame_buffer(dev, buffer);
}
void drowing_mqtt(const ssd1306_t *dev, uint8_t *buffer, uint8_t x, uint8_t y, bool cline){
   
    if(cline){
        ssd1306_draw_char(dev, buffer, font_builtin_fonts[FONT_FACE_PICTURES], x, y, PICTURE_WIDS, MQTT_PIC, OLED_COLOR_BLACK, OLED_COLOR_BLACK);
    }else{
        ssd1306_draw_char(dev, buffer, font_builtin_fonts[FONT_FACE_PICTURES], x, y, PICTURE_WIDS, MQTT_PIC, OLED_COLOR_WHITE, OLED_COLOR_BLACK); //пишем надпись в выставленной позиции белым цветом. 
    }
    
    //ssd1306_load_frame_buffer(dev, buffer);
}

void drowing_write(const ssd1306_t *dev, uint8_t *buffer, uint8_t x, uint8_t y, bool cline){
   
    if(cline){
        ssd1306_draw_char(dev, buffer, font_builtin_fonts[FONT_FACE_PICTURES], x, y, PICTURE_WIDS, write_pic, OLED_COLOR_BLACK, OLED_COLOR_BLACK);
    }else{
        ssd1306_draw_char(dev, buffer, font_builtin_fonts[FONT_FACE_PICTURES], x, y, PICTURE_WIDS, write_pic, OLED_COLOR_WHITE, OLED_COLOR_BLACK); //пишем надпись в выставленной позиции белым цветом. 
    }
    
    //ssd1306_load_frame_buffer(dev, buffer);
}

void drowing_heater(const ssd1306_t *dev, uint8_t *buffer, uint8_t x, uint8_t y, uint8_t *rele_num){

    char heater[2] = {HEATER_ON,HEATER_ON};
    if(rele_num != NULL){
        if(*rele_num  == 0U){
            heater[0] = HEATER_Off;
            heater[1] = HEATER_Off;
        }else if (*rele_num  == 1U)
        {
            heater[0] = HEATER_ON;
            heater[1] = HEATER_Off;
        }else if (*rele_num  == 2U)
        {
            heater[0] = HEATER_Off;
            heater[1] = HEATER_ON;
        }else if (*rele_num  == 3U)
        {
            heater[0] = HEATER_ON;
            heater[1] = HEATER_ON;
        }
    }
    
    ssd1306_draw_string(dev, buffer, font_builtin_fonts[FONT_FACE_PICTURES], x, y, (PICTURE_WIDS<<1U), (const char *)heater, OLED_COLOR_WHITE, OLED_COLOR_BLACK); //пишем надпись в выставленной позиции белым цветом. 

    //ssd1306_load_frame_buffer(dev, buffer);
}
void drowing_celevaya_temper(const ssd1306_t *dev, uint8_t *buffer, uint8_t x, uint8_t y, uint8_t set_temper, ssd1306_color_t foreground, ssd1306_color_t background){
    set_temper -= 4U;
    ssd1306_draw_char(dev, buffer, font_builtin_fonts[FONT_FACE_CELEVAYA_TEMPER], x, y, CELEVAYA_TEMPER_WIDS, (char )set_temper, foreground, background); //пишем надпись в выставленной позиции белым цветом. 
}

void drowing_QRcode(const ssd1306_t *dev, uint8_t *buffer, const char* char_to_qr, u8_t ver, u8_t ecc, u8_t x0, u8_t y0){
    if(char_to_qr != NULL){
        //printf("%s",char_to_qr);
        QRCode qrcode;
        uint8_t qrcodeData[qrcode_getBufferSize(ver)];
        ssd1306_fill_rectangle(dev, buffer, x0, y0, (ver<<3U) + 36U, (ver<<3U) + 36U, OLED_COLOR_WHITE);
        if(qrcode_initText(&qrcode, qrcodeData, ver, ecc, char_to_qr) == 0U){
            for (uint8_t y = 0; y < qrcode.size<<1U; y++) {
            // Each horizontal module
                for (uint8_t x = 0; x < qrcode.size<<1U; x++) {
                    if (qrcode_getModule(&qrcode, x>>1U, y>>1U)) {
                        ssd1306_draw_pixel(dev, buffer, x0+1U + x, y0+1U + y, OLED_COLOR_BLACK );     
                    } else {
                        ssd1306_draw_pixel(dev, buffer, x0+1U + x, y0+1U + y, OLED_COLOR_WHITE );     
                    }
                }
            }
        }
    }else{
        ssd1306_fill_rectangle(dev, buffer, x0, y0, (ver<<3U)+36U, (ver<<3U)+36U, OLED_COLOR_BLACK);
    }
}
