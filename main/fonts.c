/**
 * LCD/OLED fonts library
 *
 * FIXME: License?
 *
 * @date: 8 dec. 2016
 *      Author: zaltora
 */
#include "fonts.h"

#ifndef FONT_TAHOMA_6
#define FONT_TAHOMA_6 1
#endif
#ifndef FONT_TAHOMA_9
#define FONT_TAHOMA_9 1
#endif
#ifndef FONT_TAHOMA_NUM
#define FONT_TAHOMA_NUM 1
#endif
#ifndef FONT_PICTURES
#define FONT_PICTURES 1
#endif
#ifndef FONT_CELEVAYA_TEMPER
#define FONT_CELEVAYA_TEMPER 1
#endif

#if FONT_TAHOMA_9
    #include "data/tahoma_9_font.h"
#endif
#if FONT_TAHOMA_NUM
    #include "data/number_font.h"
#endif
#if FONT_TAHOMA_6
    #include "data/tahoma_6_font.h"
#endif
#if FONT_PICTURES
    #include "data/pictures_font.h"
#endif
#if FONT_CELEVAYA_TEMPER
    #include "data/celevaya_temper_font.h"
#endif
/////////////////////////////////////////////

// FIXME: this declaration is noisy

const font_info_t *font_builtin_fonts[] =
{
#if FONT_TAHOMA_9
    [FONT_FACE_TAHOMA_9] = &_tahoma_9_font_info,
#else
    [FONT_FACE_TAHOMA_9] = NULL,
#endif
#if FONT_TAHOMA_NUM
    [FONT_FACE_TAHOMA_NUM] = &_tahoma_NUM_font_info,
#else
    [FONT_FACE_TAHOMA_NUM] = NULL,
#endif
#if FONT_TAHOMA_6
    [FONT_FACE_TAHOMA_6] = &_tahoma_6_font_info,
#else
    [FONT_FACE_TAHOMA_6] = NULL,
#endif
#if FONT_PICTURES
    [FONT_FACE_PICTURES] = &_pictures_font_info,
#else
    [FONT_FACE_PICTURES] = NULL,
#endif
#if FONT_CELEVAYA_TEMPER
    [FONT_FACE_CELEVAYA_TEMPER] = &_celevaya_temper_font_info,
#else
    [FONT_FACE_CELEVAYA_TEMPER] = NULL,
#endif
};

const size_t font_builtin_fonts_count = (sizeof(font_builtin_fonts) / sizeof(font_info_t *));

/////////////////////////////////////////////
//font_char_desc_t descriptors;

uint16_t font_measure_string(const font_info_t *fnt, const char *s)
{
    if (!s || !fnt) return 0;

    uint16_t res = 0;
    while (*s)
    {
        //const font_char_desc_t *d = font_get_char_desc(fnt, *s);
        uint16_t width = font_get_char(fnt, *s);
        if (width)
            res += width + fnt->c;
        s++;
    }

    return res > 0 ? res - fnt->c : 0;
}