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

#ifndef IMG_SVG_H
# define IMG_SVG_H

# include <fontconfig/fontconfig.h>
# include <stdio.h>

# include "configuration.h"

void write_svg_specimen(FILE *html, 
                        FcPattern *font, 
                        FcBool mini,
                        config_t config, 
                        FcChar32 *sentence, 
                        const char *script,
                        const char *lang,
                        int dir, 
                        const char *html_indent,
                        int maxwidth,
                        int *width,
                        int *height);
void write_svg_charset(FILE *html, 
                       FcPattern *pattern, 
                       config_t config,
                       const char *html_indent, 
                       int maxwidth, 
                       int maxheight);
#endif
