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

#ifndef IMG_COMMON_H
# define IMG_COMMON_H

#define SIZES_MAX           64
#define SPECIMEN_DIST       10

#include <fontconfig/fontconfig.h>

#include "configuration.h"

int get_pattern_info(FcPattern *pattern,
                     FcChar8 **family,
                     FcChar8 **style,
                     FcCharSet **charset,
                     int *slant,
                     int *weight,
                     int *width,
                     int sizes[],
                     FcChar8 *files[],
                     config_t config,
                     int wanted_size);

void specimen_img_size(FcChar32 *sentence,
                       FcPattern *font,
                       int sizes[],
                       FcChar8 *files[],
                       int best_size,
                       int use_harfbuzz,
                       int text_direction,
                       const char *script,
                       const char *lang,
                       int maxwidth,
                       config_t config,
                       int *width,
                       int *height);

#endif
