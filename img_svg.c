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
#include "constants.h"
#include "fcinfo.h"
#include "ft.h"

#define SPECIMEN_SVG_DIST  10

#define CHARSET_SVG_VDIST   10
#define CHARSET_SVG_HDIST   10
#define CHARSET_LINE_LEN    16

#define SIZES_MAX           64

#if 0
static int best_size(const FcBool *sizes, int size)
{
  int dist = 0;

  while (0 < size - dist || size + dist <= SIZES_MAX)
  {
    if (size + dist <= SIZES_MAX && sizes[size + dist] == FcTrue)
      return size + dist;
    if (size - dist > 0 && sizes[size - dist] == FcTrue)
      return size - dist;

    dist++;
  }

  return 0;
}
#endif

static int best_size(FcPattern *font, int size)
{
  FcBool scalable;
  double s;

  assert(FcPatternGetBool(font, FC_SCALABLE, 0, &scalable)
         == FcResultMatch);

  if (scalable)
    return size;

  /* for !scalable fonts, first entry in pixelsize element */
  /* has the widest charset support */
  /* (thanks to how fcinfo sorts its output) */
  assert(FcPatternGetDouble(font, FC_PIXEL_SIZE, 0, &s)
         == FcResultMatch);

  return (int)s;
}

static void get_pattern_info(FcPattern *pattern, 
                      FcChar8 **family,
                      FcCharSet **charset,
                      int *slant,
                      int *weight,
                      int *width,
                      FcBool sizes[],
                      FcChar8 *files[],
                      config_t config)
{
  FcBool scalable;
  int i, px;
  double size;

  assert(fcinfo_get_translated_string(pattern, FC_FAMILY, LANG_EN, family)
         == FcResultMatch);
  if (charset)
    assert(FcPatternGetCharSet(pattern, FC_CHARSET, 0, charset)
           == FcResultMatch);
  assert(FcPatternGetInteger(pattern, FC_SLANT, 0, slant) 
         == FcResultMatch);
  assert(FcPatternGetInteger(pattern, FC_WEIGHT, 0, weight)
         == FcResultMatch);
  assert(FcPatternGetInteger(pattern, FC_WIDTH, 0, width)
         == FcResultMatch);

  assert(FcPatternGetBool(pattern, FC_SCALABLE, 0, &scalable)
         == FcResultMatch);

  bzero(sizes, sizeof(FcBool)*(SIZES_MAX + 1));

  if (! scalable)
  {
    for (i = 0;
         FcPatternGetDouble(pattern, FC_PIXEL_SIZE, i, &size) == FcResultMatch;
         i++)
      if (i <= SIZES_MAX)
      {
        sizes[(int)size] = FcTrue;
        if (files)
          assert(FcPatternGetString(pattern, FC_FILE, i, &files[(int)size])
                 == FcResultMatch);
      }
  }
  else
  {
    for (px = config.specimen_from_px; px <= config.specimen_to_px; px++)
    {
      sizes[px] = FcTrue;
      if (files)
        assert(FcPatternGetString(pattern, FC_FILE, 0, &files[px])
               == FcResultMatch);
    }
  }

  return;
}


void write_svg_specimen(FILE *html, 
                        FcPattern *font, 
                        FcBool mini, 
                        config_t config, 
                        FcChar32 *sentence, 
                        const char *script,
                        const char *lang,
                        int dir, 
                        const char *html_indent,
                        int maxwidth)
{
  int x, y;
  int px, c;
  int svg_height = SPECIMEN_SVG_DIST;
  int svg_width;

  const char *text_anchor;
  const char *writing_mode;
  
  FcChar8 *family; 
  int weight, slant, width;
  FcBool sizes[SIZES_MAX + 1], bs;
  FcChar8 *files[SIZES_MAX + 1];

  get_pattern_info(font, &family, NULL, &slant, &weight, 
                   &width, sizes, files, config);

  if (mini)
  {
    px = bs = best_size(font, config.minispecimen_pxsize);
  }
  else
  {
    for (px = SIZES_MAX; px >= 0; px--)
      if (sizes[px])
        break;
    assert (px >= 0);
  }
  
  if (dir < 2)
  {
    svg_width = ft_text_length(sentence, font, px,
                               files[px], 1, /* browser rendering will use hb */
                               dir, script, lang);
    if (svg_width <= 0)
      svg_width = maxwidth;
 
  }
  else
  {
    svg_width = SPECIMEN_SVG_DIST;
    for (px = 1; px <= SIZES_MAX; px++)
      if (sizes[px])
      {
        svg_width += (px + SPECIMEN_SVG_DIST);

        /* one entry above config.specimen_to_px is enough */
        /* for minispecimen and for specimen too */
        if (px > config.specimen_to_px)
          break;
      }

  }

  if (maxwidth && svg_width > maxwidth)
    svg_width = maxwidth;

  /* e. g. 'Arabic Newspaper' reports only size 32 */
  /* that's why we consider specimen entries over */
  /* config.specimen_to_px */
  if (mini)
  {
    svg_height = bs + SPECIMEN_SVG_DIST;
  }
  else
  {
    /* svg_height for vertical layouts could be computed as 
       svg_width  for horizontal layouts (ft_text_length
       supports vertical layouts); but it doesn't look good 
       when switching between specimen of horizontal and
       vertical layouts -> we want specimens to have same 
       height or ? */
    svg_height = SPECIMEN_SVG_DIST;
    for (px = 1; px <= SIZES_MAX; px++)
      if (sizes[px])
      {
        svg_height += (px + SPECIMEN_SVG_DIST);

        /* one entry above config.specimen_to_px is enough */
        /* for minispecimen and for specimen too */
        if (px > config.specimen_to_px)
         break;
      }
  }

  fprintf(html, 
          "%s<svg width=\"%dpx\" height=\"%dpx\" version = \"1.1\">\n", 
          html_indent, svg_width, svg_height);

  /* e. g. 'Arabic Newspaper' reports only size 32 */
  /* that's why we consider specimen entries over */
  /* config.specimen_to_px */
  if (mini)
  {
    y = config.specimen_from_px + SPECIMEN_SVG_DIST;
    fprintf(html,
            "%s  <text x = \"0px\" y = \"%dpx\" %s font-family = \"",
            html_indent, y, dir == 1 ? "text-anchor=\"end\"" : "");
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
                 bs, 
                 fc_to_css_weight(weight), style_name(slant),
                 stretch_name(width));
    if (config.generate_stooltips)
      fprintf(html, " title = \"%d px\"", bs);
    fprintf(html, ">");
    c = 0;
    while (sentence[c])
      fprintf(html, "&#x%04x;", sentence[c++]);
    fprintf(html, "</text>\n");
  }
  else
  {
    switch (dir)
    {
      case 0: /* left to right */
        x = 0;
        y = config.specimen_from_px + SPECIMEN_SVG_DIST;
        writing_mode = "lr";
        text_anchor = "start";
        break;
      case 1: /* right to left */
        x = svg_width;
        y = config.specimen_from_px + SPECIMEN_SVG_DIST;
        writing_mode = "rl";
        text_anchor = "end";
        break;
      case 2: /* top to bottom */
        x = SPECIMEN_SVG_DIST;
        y = 0;
        writing_mode = "tb";
        text_anchor = "start";
        break;
      case 3: /* bottom to top */
        x = config.specimen_from_px + SPECIMEN_SVG_DIST;
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
          y += px + SPECIMEN_SVG_DIST;
        else
          x += px + SPECIMEN_SVG_DIST;

        /* one entry above config.specimen_to_px is enough */
        /* for minispecimen and for specimen too */
        if (px > config.specimen_to_px)
          break;
      }
    }
  }

  fprintf(html, "%s</svg>\n", html_indent);
  return;
}

void write_svg_charset(FILE *html,  
                       FcPattern *font, 
                       config_t config, 
                       const char *html_indent, 
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

  get_pattern_info(font, &family, &charset, &slant, &weight,
                   &width, sizes, NULL, config);
  bs = best_size(font, config.charset_pxsize);

  available = fcinfo_chars(charset, &chars, NULL, FcTrue, ALL_CHARS);
  nlines = available/CHARSET_LINE_LEN;

  /* trim for now */
  svg_width = CHARSET_LINE_LEN*(bs + CHARSET_SVG_HDIST);
  if (maxwidth && svg_width > maxwidth)
    svg_width = maxwidth;
  svg_height = (nlines + 1)*(bs + CHARSET_SVG_VDIST);
  if (maxheight && svg_height > maxheight)
    svg_height = maxheight;

  fprintf(html, 
          "%s<svg width=\"%dpx\" height=\"%dpx\" version = \"1.1\">\n", 
          html_indent, svg_width, svg_height);

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
      fprintf(html, ">&#x%04x;</text>\n", chars[i]);
    }

    column++;
    if (column == CHARSET_LINE_LEN)
    {
      column = 1;
      line++;
    }
  }

  fprintf(html, "%s</svg>\n", html_indent);

  free(chars);
  return;
}


