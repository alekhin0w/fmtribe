#ifndef __FONT_H__
#define __FONT_H__

#include "pbm.h"
#include "vga.h"

typedef struct font_t {
    unsigned int width;
    unsigned int height;
    uint8_t* buffer;
} font_t;

bool create_font_from_pbm(const pbm_file_t* pbm, const int chars_per_row, font_t* font);
void print_char(const font_t* f, const int x, const int y, const char c);
void free_font(font_t* f);

#endif // __FONT_H__