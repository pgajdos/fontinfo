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
#include <stdlib.h> /* exit */
#include <assert.h>

#include "img_png.h"
#include "ft.h"
#include "constants.h"
#include "filesystem.h"
#include "fcinfo.h"

#define SPECIMEN_PNG_DIST    10

#define CHARSET_PNG_VDIST     10
#define CHARSET_PNG_HDIST     10
#define CHARSET_LINE_LEN      16
#define CHARSET_HEAD_PXSIZE   12
#define CHARSET_LINES_GRAY    0xD0

#define SIZES_MAX           64

#define IMG_SUBDIR          "img"

#define SCRIPT_NOT_SUPPORTED { '\0' }
/*  {'s', 'c', 'r', 'i', 'p', 't', ' ', 'n', 'o', 't',  \
   ' ', 's' ,'u', 'p', 'p', 'o' , 'r', 't', 'e', 'd', ' ', \
   'f', 'o', 'r', ' ', 't', 'h', 'i', 's', ' ', \
   's', 'i', 'z', 'e', '\0'}*/

#define SCRIPT_NOT_SUPPORTED_REV { '\0' }
/*  {'e', 'z', 'i', 's', ' ', 's', 'i', 'h', 't', ' ',  \
   'r', 'o' ,'f', ' ', 'd', 'e' , 't', 'r', 'o', 'p', 'p', \
   'u', 's', ' ', 't', 'o', 'n', ' ', 't', 'p', \
   'i', 'r', 'c', 's', '\0'}*/
char *libpng_version(char *string, int maxlen)
{
  snprintf(string, maxlen, "%d.%d.%d", 
           PNG_LIBPNG_VER_MAJOR,
           PNG_LIBPNG_VER_MINOR,
           PNG_LIBPNG_VER_RELEASE);
  return string;
}

static void write_png(const char *filename, bitmap_t bitmap)
{
  int  j;
  FILE *png;
  png_structp png_ptr;
  png_infop info_ptr;

  png = open_binary_write(filename, "write_png");

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL) 
  {
    fprintf(stderr, "write_png(): can't create write structure\n");
    exit(1);
  }

  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) 
  {
    fprintf(stderr, "write_png(): can't create info structure\n");
    exit (1);
  }

  if (setjmp(png_jmpbuf(png_ptr))) 
  {
    fprintf(stderr, "write_png(): can't create png\n");
    exit (1);
  }

  png_init_io(png_ptr, png);

  png_set_IHDR(png_ptr, info_ptr, bitmap.width, bitmap.height,
               8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  /* --- write png */
  png_write_info(png_ptr, info_ptr);
  for ( j = 0; j < bitmap.height; j++ )
    png_write_row(png_ptr, bitmap.data[j]);
  png_write_end(png_ptr, NULL);
  /* --- */

  fclose(png);
  png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
  png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

  return;
}

static int get_pattern_info(FcPattern *pattern,
                            FcChar8 **family,
                            FcChar8 **style,
                            FcCharSet **charset,
                            int sizes[],
                            FcChar8 *files[],
                            config_t config,
                            int wanted_size)
{
  FcBool scalable;  
  int i, px;
  double size;

  assert(fcinfo_get_translated_string(pattern, FC_FAMILY, LANG_EN, family)
         == FcResultMatch);
  assert(fcinfo_get_translated_string(pattern, FC_STYLE, LANG_EN, style)
         == FcResultMatch);
  if (charset)
    assert(FcPatternGetCharSet(pattern, FC_CHARSET, 0, charset)
           == FcResultMatch);
  assert(FcPatternGetBool(pattern, FC_SCALABLE, 0, &scalable)
         == FcResultMatch);
  
  bzero(sizes, sizeof(FcBool)*(SIZES_MAX + 1));

  if (! scalable)
  {
    for (i = 0;
         FcPatternGetDouble(pattern, FC_PIXEL_SIZE, i, &size) == FcResultMatch;
         i++)
      if ((int)size <= SIZES_MAX)
      {
        assert(FcPatternGetString(pattern, FC_FILE, i, &files[(int)size]) 
               == FcResultMatch);
        sizes[(int)size] = FcTrue;

        if (wanted_size)
        { 
          /* it has the best charset */
          return (int)size;
        }
     }
   
   assert(i > 0); /* no size found */
  }
  else
  {
    if (wanted_size)
    {
      sizes[wanted_size] = FcTrue;
      assert(FcPatternGetString(pattern, FC_FILE, 0, &files[wanted_size]) 
             == FcResultMatch);
      return wanted_size;
    }

    for (px = config.specimen_from_px; px <= config.specimen_to_px; px++)
    {
      sizes[px] = FcTrue;
      assert(FcPatternGetString(pattern, FC_FILE, 0, &files[px]) 
             == FcResultMatch);
    }
  }

  return 0;
}

void write_png_specimen(const char *subdir, 
                        FILE *html, 
                        FcPattern *font, 
                        FcBool mini, 
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
  char dirname_img[FILEPATH_MAX];
  char png_name[FILEPATH_MAX];
  char png_link[FILEPATH_MAX];
  char mapname[FILEPATH_MAX];
 
  int x, y, crd;
  int px;
  int png_height;
  int png_width;
  int wanted_size;

  FcChar32 no_script[] = SCRIPT_NOT_SUPPORTED;
  FcChar32 no_script_rev[] = SCRIPT_NOT_SUPPORTED_REV;

  bitmap_t bitmap;

  FcChar8 *family, *style;
  int sizes[SIZES_MAX + 1];
  FcChar8 *files[SIZES_MAX + 1];

  char sans_serif_request[64];
  FcPattern *sans_serif;

  wanted_size = mini ? config.minispecimen_pxsize : 0;
  get_pattern_info(font, &family, &style, NULL, sizes, files, 
                   config, wanted_size);

  for (px = SIZES_MAX; px >= 0; px--)
    if (sizes[px])
      break;
  assert(px > 0);

  if (dir < 2)
  {
    png_width = ft_text_length(sentence, font, px, 
                               files[px], config.use_harfbuzz,
                               dir, script, lang);
    if (png_width <= 0)
      png_width = maxwidth;
  }
  else
  {
    png_width = SPECIMEN_PNG_DIST;
    for (px = 1; px <= SIZES_MAX; px++)
      if (sizes[px])
      {
        png_width += (px + SPECIMEN_PNG_DIST);

        /* one entry above config.specimen_to_px is enough */
        /* for minispecimen and for specimen too */
        if (px > config.specimen_to_px)
          break;
      }

  }

  if (maxwidth && png_width > maxwidth)
    png_width = maxwidth;
  
  /* png_height for vertical layouts could be computed as 
     png_width  for horizontal layouts (ft_text_length
     supports vertical layouts); but it doesn't look good 
     when switching between specimen of horizontal and
     vertical layouts -> we want specimens to have same 
     height or ? */
  png_height = SPECIMEN_PNG_DIST;
  for (px = 1; px <= SIZES_MAX; px++)
    if (sizes[px])
    {
      png_height += (px + SPECIMEN_PNG_DIST);

      /* one entry above config.specimen_to_px is enough */
      /* for minispecimen and for specimen too */
      if (px > config.specimen_to_px)
        break;
    }

  if (create_png)
  {
    /* e. g. 'Arabic Newspaper' reports only size 32 */
    /* that's why we consider specimen entries over */
    /* config.specimen_to_px */

    ft_initialize_bitmap(&bitmap, png_height, png_width);

    switch (dir)
    {
      case 0: /* left to right */
        x = 0;
        y = config.specimen_from_px + SPECIMEN_PNG_DIST; 
        break;
      case 1: /* right to left */
        x = png_width;
        y = SPECIMEN_PNG_DIST;
        break;
      case 2: /* top to bottom */
        x = config.specimen_from_px + SPECIMEN_PNG_DIST; 
        y = 0;
        break;
      case 3: /* bottom to top */
        x = config.specimen_from_px + SPECIMEN_PNG_DIST;
        y = png_height;
        break;
      default:
        assert(1 == 0);
        break;
    }

    /* e. g. 'Arabic Newspaper' reports only size 32 */
    /* that's why we consider specimen entries over */
    /* config.specimen_to_px */
    for (px = 1; px <= SIZES_MAX; px++)
    {
      if (sizes[px])
      {
        ft_bitmap_set_font(&bitmap, font, px, files[px], 
                          config.use_harfbuzz, dir, script, lang);
        if (ft_draw_text(sentence, x, y, &bitmap)
              == -1)
        {
          snprintf(sans_serif_request, 64, "sans-serif:size=%d", px);   
          sans_serif = fcinfo_get_font((FcChar8 *)sans_serif_request);
          ft_bitmap_set_font(&bitmap, sans_serif, 0, NULL, 
                             0, 0, "Latin", "en");

          ft_fill_region(&bitmap, 0, y - px, 
                         png_width - 1, y + SPECIMEN_PNG_DIST - 1, 0xFF); 
          ft_draw_text(dir % 2 == 0 ? no_script : no_script_rev,  
                       x, y, &bitmap);
          FcPatternDestroy(sans_serif);
        }

        /* one entry above config.specimen_to_px is enough */
        /* for minispecimen and for specimen too */
        if (px > config.specimen_to_px)
          break;
 
        if (dir < 2)
          y += px + SPECIMEN_PNG_DIST;
        else
          x += px + SPECIMEN_PNG_DIST;
      }
    }

    snprintf(dirname_img, FILEPATH_MAX, "%s/%s/%s", 
             config.dir, subdir, IMG_SUBDIR);
    create_dir(dirname_img);

    snprintf(png_name, FILEPATH_MAX, "%s/%s%s.%s.s.png",
             dirname_img, family, style, script);
    remove_spaces_and_slashes(&png_name[strlen(dirname_img)+1]);
    write_png(png_name, bitmap);

    ft_free_bitmap(&bitmap);
  }

  /* link png to html */
  if (config.generate_stooltips)
  {
    snprintf(mapname, FILEPATH_MAX, "%s%s%s", family, style, script);
    remove_spaces_and_slashes(mapname);    
    fprintf(html, "%s<map name=\"%s%s\">\n", 
            html_indent, mapname_prefix, mapname);
    crd = 0;
    for (px = 1; px <= SIZES_MAX; px++)
      if (sizes[px])
      {
        if (dir < 2)
        {
          fprintf(html, 
                  "%s  <area shape=\"rect\" "
                  "coords=\"%d,%d,%d,%d\" title=\"%d px\">\n",
                  html_indent, 0, crd, png_width, crd + px + SPECIMEN_PNG_DIST, px);
        }
        else
        {
          fprintf(html, 
                  "%s  <area shape=\"rect\" "
                  "coords=\"%d,%d,%d,%d\" title=\"%d px\">\n",
                  html_indent, crd, 0, crd + px + SPECIMEN_PNG_DIST, png_height, px);
        }
        crd += px + SPECIMEN_PNG_DIST;
      }
    fprintf(html, "%s</map>\n", html_indent);
  }

  snprintf(png_link, FILEPATH_MAX, "%s/%s%s.%s.s.png", 
           IMG_SUBDIR, family, style, script);
  remove_spaces_and_slashes(&png_link[strlen(IMG_SUBDIR)+1]);
  fprintf(html, 
          "%s<img src=\"%s\" alt=\"Specimen for %s %s (%s script).\"",
          html_indent, png_link, family, style, script);
  if (config.generate_stooltips)
    fprintf(html, " usemap=\"#%s%s\"", mapname_prefix, mapname);
  fprintf(html, "/>\n");
  
  return;
}

void write_png_charset(const char *subdir, 
                       FILE *html,  
                       FcPattern *font, 
                       config_t config, 
                       const char *html_indent, 
                       const char *mapname,
                       int maxwidth, 
                       int maxheight)
{
  FcChar32 *chars;
  FcChar32 available;
  FcChar32 char_index, first_char_in_line;
  FcChar32 text[2] = { '\0', '\0' };
  FcChar32 head_text[8] = { 'U', '+', 'x', 'x', 'x', 'x', 'x', '\0' };
  char ucode[6];

  int line, column, nlines;
  int png_nparts, nlines_part, line_part;
  int empty_lines_part, last_line;
  int png_width, png_height_part;
  int c, charset_head_width;

  const FcChar8 *uchar_name;

  bitmap_t bitmap;
  char dirname_img[FILEPATH_MAX];
  char png_name[FILEPATH_MAX];
  char png_link[FILEPATH_MAX];

  FcChar8 *family, *style;
  FcCharSet *charset;

  FcPattern *monospace;
  char monospace_request[64];

  int sizes[SIZES_MAX + 1], bs;
  FcChar8 *files[SIZES_MAX + 1];

  bs = get_pattern_info(font, &family, &style, &charset, 
                        sizes, files, config, config.charset_pxsize);
  snprintf(monospace_request, 64, "monospace:style=Bold:size=%d", 
           CHARSET_HEAD_PXSIZE);
  monospace = fcinfo_get_font((FcChar8 *)monospace_request);
  charset_head_width = ft_text_length(head_text, monospace, 0, 
                                      NULL, 0, 0, NULL, NULL)
                       + CHARSET_PNG_VDIST;

  assert(bs > 0);
  available = fcinfo_chars(charset, &chars, NULL, FcTrue, ALL_CHARS);
  nlines = available/CHARSET_LINE_LEN;
  
  png_width = charset_head_width + CHARSET_LINE_LEN*(bs + CHARSET_PNG_HDIST);
  if (maxwidth && png_width > maxwidth)
  {
    fprintf(stderr, 
            "warning: truncating width of charset"
            " image for %s %s (%d -> %d)\n", 
            family, style, png_width, maxwidth);
    png_width = maxwidth;
  }

  png_nparts = 0;
  line = 0;
  first_char_in_line = available; /* == 'unset' */
  while (line < nlines)
  {
    if (maxheight && (nlines - line)*(bs + CHARSET_PNG_HDIST) > maxheight)
      nlines_part = maxheight / (bs + CHARSET_PNG_HDIST);
    else
      nlines_part = nlines - line;
    
    png_height_part = nlines_part * (bs + CHARSET_PNG_HDIST);

    ft_initialize_bitmap(&bitmap, png_height_part, png_width);
    ft_fill_region(&bitmap, 
                   charset_head_width - CHARSET_PNG_VDIST/2, 
                   CHARSET_PNG_HDIST,
                   charset_head_width - CHARSET_PNG_VDIST/2, 
                   png_height_part,
                   CHARSET_LINES_GRAY);

    if (config.generate_ctooltips)
      fprintf(html, "%s<map name=\"%s%d\">\n", 
                    html_indent, mapname, png_nparts);

    line_part = 0;
    empty_lines_part = 0;
    last_line = line + nlines_part - 1;
    for (; line <= last_line; line++)
    { 
      if (line != 0)
        ft_fill_region(&bitmap, 
                       0,
                       line_part*(bs + CHARSET_PNG_HDIST) + CHARSET_PNG_HDIST/2,
                       png_width,
                       line_part*(bs + CHARSET_PNG_HDIST) + CHARSET_PNG_HDIST/2, 
                       CHARSET_LINES_GRAY);

      ft_bitmap_set_font(&bitmap, font, bs, files[bs], 0, 0, NULL, NULL);
      for (column = 0; column < CHARSET_LINE_LEN; column++)
      {
        char_index = line*CHARSET_LINE_LEN + column;
        assert (char_index < available);
        if (chars[char_index] != NO_CHAR)
        {
          text[0] = chars[char_index];
          ft_draw_text(text,
                       charset_head_width + column*(bs + CHARSET_PNG_HDIST),
                       (line_part + 1)*(bs + CHARSET_PNG_VDIST),
                       &bitmap);
          if (config.generate_ctooltips)
          {
            fprintf(html,
                    "%s  <area shape=\"rect\" coords=\"%d,%d,%d,%d\""
                    " title=\"",
                    html_indent,
                    charset_head_width + column*(bs + CHARSET_PNG_HDIST),
                    line_part*(bs + CHARSET_PNG_VDIST),
                    charset_head_width + 
                        (column + 1)*(bs + CHARSET_PNG_HDIST) - 1,
                    (line_part + 1)*(bs + CHARSET_PNG_VDIST) - 1);
            uchar_name = unicode_char_name(chars[char_index]);
            if (strcmp((char *)uchar_name, (char *)CONST_STRING_UNKNOWN) != 0)
              fprintf(html, "%s (U+%04X)", uchar_name, chars[char_index]);
            else
               fprintf(html, "U+%04X", chars[char_index]);
            fprintf(html, "\">\n");
          }

          if (first_char_in_line == available) /* == unset */
            first_char_in_line = char_index;          
        }
      }

      if (first_char_in_line < available) /* -> first_char_in_line set */
      {
        ft_bitmap_set_font(&bitmap, monospace, 0, NULL, 0, 0, NULL, NULL);
        sprintf(ucode, "%05X", chars[first_char_in_line]);
        for (c = 0; c < 4; c++)
          head_text[c + 2] = ucode[c];
        ft_draw_text(head_text,
                     0,
                     (line_part + 1)*(bs + CHARSET_PNG_HDIST)
                        - bs/2 + CHARSET_HEAD_PXSIZE/2,
                     &bitmap);

        line_part++, first_char_in_line = available;
      }
      else
        empty_lines_part++;
    }

    if (config.generate_ctooltips)
      fprintf(html, "%s</map>\n", html_indent);

    if (empty_lines_part)
    {
      if (empty_lines_part == nlines_part) /* only empty lines left */
      {
        ft_free_bitmap(&bitmap);
        break; /* probably nothing left to do */
      }
      
    ft_reduce_height(&bitmap, 
          png_height_part - (empty_lines_part * (bs + CHARSET_PNG_VDIST)));
    }

    snprintf(dirname_img, FILEPATH_MAX, "%s/%s/%s", 
             config.dir, subdir, IMG_SUBDIR);
    create_dir(dirname_img);
    snprintf(png_name, FILEPATH_MAX, "%s/%s%s.c.%d.png",
             dirname_img, family, style, png_nparts);
    remove_spaces_and_slashes(&png_name[strlen(dirname_img)+1]);
    write_png(png_name, bitmap);

    snprintf(png_link, FILEPATH_MAX, "%s/%s%s.c.%d.png", 
             IMG_SUBDIR, family, style, png_nparts);
    remove_spaces_and_slashes(&png_link[strlen(IMG_SUBDIR)+1]);
    fprintf(html,
            "%s<img src=\"%s\" alt=\"Character Set for %s %s (part %d).\"",
            html_indent, png_link, family, style, png_nparts);
    if (config.generate_ctooltips)
      fprintf(html, " usemap=\"#%s%d\"", mapname, png_nparts);
    fprintf(html, "/>\n");

    ft_free_bitmap(&bitmap);
    png_nparts++;
  }

  FcPatternDestroy(monospace);
  free(chars);
  return;
}


