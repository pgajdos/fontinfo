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

#include <string.h> /* bzero */
#include <stdlib.h> /* malloc, free */
#include <assert.h>

#include "img_svg.h"
#include "img_common.h"
#include "fcinfo.h"

#define CHARSET_SVG_VDIST   10
#define CHARSET_SVG_HDIST   10
#define CHARSET_LINE_LEN    16

void write_svg_specimen(FILE *html, 
                        FcPattern *font, 
                        FcBool mini, 
                        config_t config, 
                        FcChar32 *sentence, 
                        const char *script,
                        const char *lang,
                        int dir, 
                        img_transform_t transform,
                        const char *html_indent,
                        int maxwidth,
                        int *res_width,
                        int *res_height)
{
  int x, y;
  int px, c;
  int svg_width;
  int svg_height;
  int tmp;
  int wanted_size;
  int bs;

  const char *text_anchor;
  const char *writing_mode;
  
  FcChar8 *family; 
  int weight, slant, width;
  int sizes[SIZES_MAX + 1];
  FcChar8 *files[SIZES_MAX + 1];

  wanted_size = mini ? config.minispecimen_pxsize : 0;
  bs = get_pattern_info(font, &family, NULL, NULL, &slant, &weight, 
                        &width, sizes, files, config, wanted_size);
  specimen_img_size(sentence, font, sizes, files,
                    bs, config.use_harfbuzz, dir, script, lang,
                    maxwidth, config, &svg_width, &svg_height);

  fprintf(html, "%s<svg version = \"1.1\" ", html_indent);

  switch (transform)
  {
    case TRNS_ROT270:
      tmp = svg_height;
      svg_height = svg_width;
      svg_width = tmp;
      fprintf(html, "transform = \"translate(%d, 0) rotate(90)\" ", 
              svg_width);
      break;

    case TRNS_NONE:
      break;

    default:
      assert(1 == 0);
      break;
  }

  fprintf(html, "width=\"%dpx\" height=\"%dpx\">\n", 
          svg_width, svg_height);

  /* e. g. 'Arabic Newspaper' reports only size 32 */
  /* that's why we consider specimen entries over */
  /* config.specimen_to_px */
  switch (dir)
  {
    case 0: /* left to right */
      x = 0;
      y = (bs ? bs : config.specimen_from_px) + SPECIMEN_DIST;
      writing_mode = ""; /* autodetection seems to do better */
      text_anchor = "start";
      break;
    case 1: /* right to left */
      x = svg_width;
      y = (bs ? bs : config.specimen_from_px) + SPECIMEN_DIST;
      writing_mode = ""; /* autodetection seems to do better */
      text_anchor = "end";
      break;
    case 2: /* top to bottom */
      x = (bs ? bs : config.specimen_from_px);
      y = 0;
      writing_mode = "tb-rl";
      text_anchor = "start";
      break;
    case 3: /* bottom to top */
      x = (bs ? bs : config.specimen_from_px);
      y = svg_height;
      writing_mode = "tb"; /* it seems there isn't "bt" */
      text_anchor = "end";
      break;
    default:
      assert(1 == 0);
      break;
  }

  for (px = 1; px <= SIZES_MAX; px++)
  {
    if (sizes[px])
    {
      fprintf(html, 
              "%s  <text x = \"%dpx\" y = \"%dpx\" text-anchor = \"%s\""
              " writing-mode = \"%s\" font-family = \"", 
              html_indent, x, y, text_anchor, writing_mode);
      for (c = 0; c < strlen((char *)family); c++)
      {
        if (family[c] == '/')  /* e. g. Anka/Coder */
          fputc('\\', html);
        fputc(family[c], html);
        if (family[c] == '&') /* e. g. B&H Lucida */
          fprintf(html, "amp;");
      }
      fprintf(html, "\" font-size = \"%dpx\" font-weight = \"%d\" "
                   "font-style = \"%s\" font-stretch = \"%s\"", 
                   px, fc_to_css_weight(weight), style_name(slant), 
                   stretch_name(width));
      if (config.generate_stooltips)
        fprintf(html, " title=\"%d px\"", px);
      fprintf(html, ">");
      c = 0;
      while (sentence[c])
          fprintf(html, "&#x%04x;", sentence[c++]);
      fprintf(html, "</text>\n");

      if (dir < 2)
        y += px + SPECIMEN_DIST;
      else
        x += px + SPECIMEN_DIST;

      /* one entry above config.specimen_to_px is enough */
      /* for minispecimen and for specimen too */
      if (px > config.specimen_to_px)
        break;
    }
  }
  fprintf(html, "%s</svg>\n", html_indent);

  if (res_width)
    *res_width = svg_width;
  if (res_height)
    *res_height = svg_height;
  return;
}

void write_svg_charset(FILE *html,  
                       FcPattern *font, 
                       const char *uinterval,
                       uinterval_type_t uintype,
                       config_t config, 
                       const char *html_indent, 
                       const char *line_suffix,
                       int maxwidth, 
                       int maxheight)
{
  FcChar32 *chars;
  FcChar32 available;
  FcChar32 i;

  int line, column, nlines;
  int svg_width, svg_height;
  int c;

  const FcChar8 *uchar_name;

  FcCharSet *charset; 
  FcChar8 *family;
  int weight, slant, width;

  int sizes[SIZES_MAX + 1], bs;

  bs = get_pattern_info(font, &family, NULL, &charset, &slant, &weight,
                        &width, sizes, NULL, config, 0);

  available = fcinfo_chars(charset, &chars, uinterval, uintype,
                           FcTrue, ALL_CHARS);
  nlines = available/CHARSET_LINE_LEN;

  /* trim for now */
  svg_width = CHARSET_LINE_LEN*(bs + CHARSET_SVG_HDIST);
  if (maxwidth && svg_width > maxwidth)
    svg_width = maxwidth;
  svg_height = (nlines + 1)*(bs + CHARSET_SVG_VDIST);
  if (maxheight && svg_height > maxheight)
    svg_height = maxheight;

  fprintf(html, 
          "%s<svg version = \"1.1\" width=\"%dpx\" height=\"%dpx\">%s\n", 
          html_indent, svg_width, svg_height, line_suffix);

  line = 1;
  column = 1;
  for (i = 0; i < available; i++)
  {
    if (chars[i] != NO_CHAR)
    {
      fprintf(html, "%s  <text x=\"%dpx\" y=\"%dpx\" font-family=\"", 
              html_indent,
              (column - 1)*(bs + CHARSET_SVG_HDIST), 
              line*(bs + CHARSET_SVG_VDIST)); 
      for (c = 0; c < strlen((char *)family); c++)
      {
        fputc(family[c], html);
        if (family[c] == '&')
          fprintf(html, "amp;");
      }
      fprintf(html, 
              "\" font-size=\"%dpx\" font-weight=\"%d\""
              " font-style=\"%s\" font-stretch=\"%s\"",
              bs, fc_to_css_weight(weight), 
              style_name(slant), stretch_name(width));
      if (config.generate_ctooltips)
      {
        if ((uchar_name = unicode_char_name(chars[i])) != CONST_STRING_UNKNOWN)
          fprintf(html, " title=\"%s (0x%04x)\"", uchar_name, chars[i]);
        else
          fprintf(html, " title=\"0x%04x\"", chars[i]);
      }
      fprintf(html, ">&#x%04x;</text>%s\n", chars[i], line_suffix);
    }

    column++;
    if (column == CHARSET_LINE_LEN)
    {
      column = 1;
      line++;
    }
  }

  fprintf(html, "%s</svg>%s\n", html_indent, line_suffix);

  free(chars);
  return;
}


