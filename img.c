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

#include "img_svg.h"
#include "img_png.h"
#include "img.h"

void write_specimen(FILE *html, 
                    FcPattern *font,
                    const char *png_subdir, 
                    config_t config,
                    FcChar32 *sentence, 
                    const char *script, 
                    const char *lang,
                    int dir, 
                    const char *html_indent, 
                    const char *mapname_prefix,
                    int maxwidth)
{
  switch (config.specimen_type)
  {
    case SVG:
          /* dir needed also in svg, see http://www.w3.org/International/tutorials/svg-tiny-bidi/ */
          write_svg_specimen(html, font, FcFalse, config, 
                             sentence, script, lang, dir, 
                             html_indent, maxwidth);
      break;
    case PNG:
          write_png_specimen(png_subdir, html, font, FcFalse, 
                             config, sentence, script, lang, dir, 
                             html_indent, 1, mapname_prefix, maxwidth);
      break;
    default:
      break;
  }

  return;  
}

void write_minispecimen(FILE *html, 
                        FcPattern *font,
                        const char *png_subdir, 
                        config_t config,
                        FcChar32 *sentence, 
                        const char *script, 
                        const char *lang,
                        int dir, 
                        const char *html_indent,
                        int create_png, 
                        const char *mapname_prefix,
                        int maxwidth)
{
  switch (config.specimen_type)
  {
    case SVG:
          write_svg_specimen(html, font, FcTrue, config, 
                             sentence, script, lang, dir, 
                             html_indent, maxwidth);
      break;
    case PNG:
          write_png_specimen(png_subdir, html, font, FcTrue, 
                             config, sentence, script, lang, dir, 
                             html_indent, create_png, mapname_prefix, 
                             maxwidth);
      break;
    default:
      break;
  }

  return;  
}

void write_charset(FILE *html, 
                   FcPattern *font,
                   const char *png_subdir, 
                   config_t config, 
                   const char *html_indent, 
                   const char *mapname,
                   int maxwidth, 
                   int maxheight)
{
  switch (config.charset_type)
  {
    case SVG:
          write_svg_charset(html, font, config, html_indent, 
                            maxwidth, maxheight);
      break;
    case PNG:
          write_png_charset(png_subdir, html, font,   
                            config, html_indent, 
                            mapname, maxwidth, maxheight);
      break;
    default:
      break;
  }

  return;  
}


