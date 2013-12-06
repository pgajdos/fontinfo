/*
 * fontinfo
 *
 * Overview of Installed Fonts
 *
 * Copyright (C) 2013 Petr Gajdos (pgajdos at suse)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef HBZ_H
# define HBZ_H

#include "constants.h"

#include <stdint.h>

#include <ft2build.h>
#include FT_FREETYPE_H

const char *hbz_version(void);
unsigned hbz_glyphs(uint32_t s[], 
                    int slen,
                    const char *script, 
                    const char *lang,
                    int dir, 
                    FT_Face face,
                    FT_UInt **glyph_codepoints,
                    FT_Vector **glyph_offsets, 
                    FT_Vector **glyph_advances,
                    int *sum_advances_x,
                    int *sum_advances_y);

#endif
