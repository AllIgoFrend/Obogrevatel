/**
 * LCD/OLED fonts library
 *
 * Copyright (c) 2016 Ruslan V. Uss (https://github.com/UncleRus),
 *                    Zaltora (https://github.com/Zaltora)
 * MIT Licensed as described in the file LICENSE
 */
#ifndef _EXTRAS_FONTS_H_
#define _EXTRAS_FONTS_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    FONT_FACE_TAHOMA_9 = 0,
    FONT_FACE_TAHOMA_NUM,
    FONT_FACE_TAHOMA_6,
    FONT_FACE_PICTURES,
    FONT_FACE_CELEVAYA_TEMPER
} font_face_t;

/**
 * Character descriptor
 */
typedef struct _font_char_desc
{
    uint8_t width;   ///< Character width in pixel
    uint16_t offset; ///< Offset of this character in bitmap
} font_char_desc_t;

/**
 * Font information
 */
typedef struct _font_info
{
    uint8_t height;                           ///< Character height in pixel, all characters have same height
    uint8_t c;                                ///< Simulation of "C" width in TrueType term, the space between adjacent characters
    char char_start;                          ///< First character
    char char_end;                            ///< Last character
    //const font_char_desc_t *char_descriptors; ///< descriptor for each character
    const uint8_t *bitmap;                    ///< Character bitmap
    uint8_t little_end;                       ///< литл енд биг енд
    uint8_t wids;                             
} font_info_t;

/**
 * Built-in fonts
 */
extern const font_info_t *font_builtin_fonts[];
extern const size_t font_builtin_fonts_count;

/**
 * Find character decriptor in font
 * @param fnt Poniter to font information struct
 * @param c Character
 * @return Character descriptor or NULL if no character found
 
inline const font_char_desc_t *font_get_char_desc(const font_info_t *fnt, char c)
{
    return c < fnt->char_start || c > fnt->char_end
        ? NULL
        : fnt->char_descriptors + (c - fnt->char_start);
}
*/
/**
 * Find character in font
 * @param fnt Poniter to font information struct
 * @param c Character
 * @return Character descriptor or NULL if no character found
 */
inline uint16_t font_get_char(const font_info_t *fnt, char c)
{
    return c < fnt->char_start || c > fnt->char_end
        ? 0U
        : ((1U + fnt->height*fnt->wids)*(c - fnt->char_start));
}

/**
 * Calculate width of string in pixels
 * @param fnt Poniter to font information struct
 * @param s String
 * @return String width
 */
uint16_t font_measure_string(const font_info_t *fnt, const char *s);

#ifdef __cplusplus
}
#endif

#endif /* _EXTRAS_FONTS_H_ */