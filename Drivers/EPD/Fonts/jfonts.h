/**
 *   Copyright (C) 2026 jfgd
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
**/


#ifndef __JFONTS_H
#define __JFONTS_H

#include <stdint.h>

typedef struct _font
{
	uint16_t max_width;
	uint16_t height;
	char default_char;
	char min_char;
	char max_char;
	int16_t nb_glyphs;
	struct glyph {
		char c;
		uint16_t width;
		const uint8_t *table;
	} glyphs[];
} jFont;


static inline int jfont_get_width(jFont *font, char c)
{
	for (int i = 0 ; i < font->nb_glyphs ; i++) {
		if (font->glyphs[i].c == c) {
			return font->glyphs[i].width;
		}
	}
	for (int i = 0 ; i < font->nb_glyphs ; i++) {
		if (font->glyphs[i].c == font->default_char) {
			return font->glyphs[i].width;
		}
	}
	return font->max_width;
}

extern jFont font12;
extern jFont Digits25NotoSansSemiCondensedBold;
extern jFont Digits50NotoSansSemiCondensedBold;
extern jFont Digits90NotoSansSemiCondensedBold;
extern jFont Airnet40NotoSansSemiCondensedBoldItalic;
extern jFont CO253NotoSansExtraBold;
extern jFont CO2ppm25NotoSansMedium;
extern jFont Thermometer50;
extern jFont Droplet20;

#endif /* __JFONTS_H */
