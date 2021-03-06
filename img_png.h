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

#ifndef IMG_PNG_H
# define IMG_PNG_H

# include <fontconfig/fontconfig.h>
# include <stdio.h>

# include <png.h>

# include "configuration.h"
# include "constants.h"

void write_png_specimen(const char *subdir, 
                        FILE *html,
                        FcPattern *font, 
                        FcBool mini,
                        config_t config, 
                        FcChar32 *sentence, 
                        const char *script, 
                        const char *lang,
                        int dir, 
                        img_transform_t transform,
                        const char *html_indent, 
                        FcBool create_png, 
                        const char *mapname_prefix,
                        int maxwidth,
                        int *res_width,
                        int *res_height);
void write_png_charset(const char *subdir, 
                       FILE *html, 
                       FcPattern *font, 
                       const char *uinterval,
                       uinterval_type_t uintype,
                       config_t config, 
                       const char *html_indent,
                       const char *line_suffix,
                       const char *mapname,
                       int maxwidth, int maxheight);
char *libpng_version(char *string, int maxlen);
#endif
