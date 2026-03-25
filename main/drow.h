#ifndef DROW_H_
#define DROW_H_

#include "../main/ssd1306.h"
#include "../main/qrcode.h"
#include "../main/user_config.h"
//**************************************************************|
//   wifi       rele1  rele2                          mqtt      |
//    0,0,16       20,0,40                            100,0,130 |
//**************************************************************|
//                          |                                   |
//                          |                                   |
//                          |                                   |
//      целевая             |  термодатчик                      |
//      температура         |                                   |
//                          |                                   |
//                          |                                   |
//**************************************************************|
// sawe               мощность                                  |
//                                                              |
//**************************************************************|

    #define WiFi_100 0x01
    #define WiFi_75 0x02
    #define WiFi_50 0x03
    #define WiFi_25 0x04
    #define WiFi_100_up_down 0x05
    #define WiFi_75_up_down 0x06
    #define WiFi_50_up_down 0x07
    #define WiFi_25_up_down 0x08
    #define WiFi_100_down 0x09
    #define WiFi_75_down 0x0a
    #define WiFi_50_down 0x0b
    #define WiFi_25_down 0x0c
    #define WiFi_100_up 0x0d
    #define WiFi_75_up 0x0e
    #define WiFi_50_up 0x0f
    #define WiFi_25_up 0x10
    #define MQTT_PIC 0x11
    #define HEATER_ON 0x12
    #define HEATER_Off 0x13
    #define strelki_vverh 0x14
    #define strelki_vniz 0x15
    #define TEMPERATURA_pic 0x16
    #define SAVE_pic 0x17
    #define write_pic 0x18
    #define check_pic 0x19



/**
 * drow pictures wifi into the SSD1306 RAM.
 * @param dev Pointer to device descriptor
 * @param buffer Pointer to framebuffer or NULL for clear RAM. Framebuffer size = width * height / 8
 * @param x pozition column
 * @param y pozition row
 * @param link LINK UP DOWN NO_LINK
 * @param rssi 0-255 level wifi in (-)dBm
 * @param invert bakgraund <-> forwardgraund change
 */
void drowing_wifi(const ssd1306_t *dev, uint8_t *buffer, uint8_t x, uint8_t y, uint8_t link, int8_t *rssi, bool invert);

/**
 * Draw a filled circle
 * @param dev Pointer to device descriptor
 * @param buffer Pointer to framebuffer. Framebuffer size = width * height / 8
 * @param x0 X coordinate or center
 * @param y0 Y coordinate or center
 */
void drowing_wifi_ap(const ssd1306_t *dev, uint8_t *buffer, uint8_t x, uint8_t y);

/**
 * Draw a filled rectangle black
 * @param dev Pointer to device descriptor
 * @param buffer Pointer to framebuffer. Framebuffer size = width * height / 8
 * @param x0 X coordinate or center
 * @param y0 Y coordinate or center
 */
void clire_wifi(const ssd1306_t *dev, uint8_t *buffer, uint8_t x, uint8_t y);

/**
 * drow pictures mqtt into the SSD1306 RAM.
 * @param dev Pointer to device descriptor
 * @param buffer Pointer to framebuffer or NULL for clear RAM. Framebuffer size = width * height / 8
 * @param x pozition column
 * @param y pozition row
 * @param cline bakgraund = forwardgraund change
 */
void drowing_mqtt(const ssd1306_t *dev, uint8_t *buffer, uint8_t x, uint8_t y, bool cline);

/**
 * drow pictures write into the SSD1306 RAM.
 * @param dev Pointer to device descriptor
 * @param buffer Pointer to framebuffer or NULL for clear RAM. Framebuffer size = width * height / 8
 * @param x pozition column
 * @param y pozition row
 * @param cline bakgraund = forwardgraund change
 */
void drowing_write(const ssd1306_t *dev, uint8_t *buffer, uint8_t x, uint8_t y, bool cline);

/**
 * drow pictures heater into the SSD1306 RAM.
 * @param dev Pointer to device descriptor
 * @param buffer Pointer to framebuffer or NULL for clear RAM. Framebuffer size = width * height / 8
 * @param x pozition column
 * @param y pozition row
 * @param rele_num nomer rele
 */
void drowing_heater(const ssd1306_t *dev, uint8_t *buffer, uint8_t x, uint8_t y, uint8_t *rele_num);

/**
 * drow pictures celevaya_temper into the SSD1306 RAM.
 * @param dev Pointer to device descriptor
 * @param buffer Pointer to framebuffer or NULL for clear RAM. Framebuffer size = width * height / 8
 * @param x pozition column
 * @param y pozition row
 * @param set_temper celevaya temper ot 5 do 30 degrees (5 = 0, 30 = 26)
 * @param foreground osnovnoy cvet
 * @param background cvet zadnego fona
 */
void drowing_celevaya_temper(const ssd1306_t *dev, uint8_t *buffer, uint8_t x, uint8_t y, uint8_t set_temper, ssd1306_color_t foreground, ssd1306_color_t background);

/**
 * Draw a qrcode
 * @param dev Pointer to device descriptor
 * @param buffer Pointer to framebuffer. Framebuffer size = width * height / 8
 * @param qr_to_char qrcode from string esli parametor NULL to risuem rectangul 
 * @param ver version qrcode square = (ver*4+17)*2+2(wite frame) (3 = 60х60)
 * @param ecc code correction from 0 to 3 
 *                  num     abc    bit
 *   0 LOW	        (41	    25	    17)*ver
 *   1 MEDIUM	    (34	    20	    14)*ver
 *   2 QUARTILE	    (27	    16	    11)*ver
 *   3 HIGH	        (17 	10	    7)*ver
 * @param x0 X coordinate 
 * @param y0 Y coordinate 
 */
void drowing_QRcode(const ssd1306_t *dev, uint8_t *buffer, const char* char_to_qr, u8_t ver, u8_t ecc, u8_t x0, u8_t y0);
#endif //DROW_H_