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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OPENTYPE_VALIDATE_H
#include FT_TRUETYPE_TABLES_H
#include FT_GLYPH_H

#include <fontconfig/fontconfig.h>

#include "ft.h"
#include "hbz.h"

char *freetype_version(char *string, int maxlen)
{
  int major, minor, patch;
  FT_Library library;
  if (FT_Init_FreeType(&library))
  {
    fprintf(stderr,
            "ft_version(): can't initialize freetype library\n");
    exit(1);
  }
  FT_Library_Version(library, &major, &minor, &patch);
  FT_Done_FreeType(library);

  snprintf(string, maxlen, "%d.%d.%d", major, minor, patch);
  return string;
}

const char *harfbuzz_version(void)
{
  return hbz_version();
}

void ft_initialize_bitmap(bitmap_t *bitmap, int height, int width)
{
  int row;

  FT_Error error;

  bitmap->data = (unsigned char **)malloc(height*sizeof(unsigned char *));
  if (! bitmap->data)
  {
    fprintf(stderr, "ft_initialize_bitmap(): no free memory\n");
    exit(1);
  }

  for (row = 0; row < height; row++)
  {
    bitmap->data[row] = (unsigned char *)malloc(width*sizeof(unsigned char));
    memset(bitmap->data[row], 255, width*sizeof(unsigned char)); /* white */
    if (! bitmap->data[row])
    {
      fprintf(stderr, "ft_initialize_bitmap(): no free memory\n");
      exit(1);
    }
  }

  bitmap->height = height;
  bitmap->width = width;

  error = FT_Init_FreeType(&bitmap->library);
  if (error)
  {
    fprintf(stderr, 
            "ft_initialize_bitmap(): can't initialize freetype library\n");
    exit(1);
  }

  bitmap->face = NULL;

  return;
}

void ft_bitmap_set_font(bitmap_t *bitmap, 
                        FcPattern *pattern, 
                        int pxsize, 
                        FcChar8 *file, 
                        FT_Bool use_harfbuzz, 
                        int text_direction,
                        const char *script, 
                        const char *lang)
{
  FcBool scalable;
  FcBool hinting;
  FcBool autohint;
  FcBool antialias;
  int hintstyle;
  FcBool embeddedbitmaps;

  FT_Error error;

  //printf("ft_bitmap_set_font(): use_harfbuzz = %d text_direction = %d script = %s lang = %s\n", use_harfbuzz, text_direction, script, lang);

  if (bitmap->face)
    FT_Done_Face(bitmap->face);

  if (! file)
    assert(FcPatternGetString(pattern, FC_FILE, 0, &file)
             == FcResultMatch);

  if (! pxsize)
    assert(FcPatternGetInteger(pattern, FC_PIXEL_SIZE, 0, &pxsize)
             == FcResultMatch);

  error = FT_New_Face(bitmap->library, (char *)file, 0, &bitmap->face);
  if (error)
  {
    fprintf(stderr, "ft_bitmap_set_font(): can't create face object\n");
    exit(1);
  }

  error = FT_Set_Pixel_Sizes(bitmap->face, 0, pxsize);
  if (error)
  {
    fprintf(stderr, "ft_bitmap_set_font(): can't set face size\n");
    exit(1);
  }

  assert(FcPatternGetBool(pattern, FC_SCALABLE, 0, &scalable)
         == FcResultMatch);

  bitmap->load_flags = FT_LOAD_DEFAULT;
  bitmap->render_mode = FT_RENDER_MODE_NORMAL;

  if (scalable)
  {
    assert(FcPatternGetBool(pattern, FC_ANTIALIAS, 0, &antialias)
           == FcResultMatch);
    assert(FcPatternGetBool(pattern, FC_HINTING, 0, &hinting)
           == FcResultMatch);
    assert(FcPatternGetBool(pattern, FC_AUTOHINT, 0, &autohint)
           == FcResultMatch);
    assert(FcPatternGetInteger(pattern, FC_HINT_STYLE, 0, &hintstyle)
           == FcResultMatch);
    assert(FcPatternGetBool(pattern, FC_EMBEDDED_BITMAP, 0, &embeddedbitmaps)
           == FcResultMatch);

    if (!antialias)
    {
      bitmap->render_mode = FT_RENDER_MODE_MONO;
      bitmap->load_flags |= FT_LOAD_TARGET_MONO;
    }

    if (!hinting)
      bitmap->load_flags |= FT_LOAD_NO_HINTING;

    if (autohint)
      bitmap->load_flags |= FT_LOAD_FORCE_AUTOHINT;

    switch(hintstyle)
    {
      case FC_HINT_NONE:   bitmap->load_flags |= FT_LOAD_NO_HINTING; break;
      case FC_HINT_SLIGHT: bitmap->load_flags |= FT_LOAD_TARGET_LIGHT; break;
      case FC_HINT_MEDIUM: bitmap->load_flags |= FT_LOAD_TARGET_NORMAL; break;
      case FC_HINT_FULL:   bitmap->load_flags |= FT_LOAD_TARGET_NORMAL; break;
    }

    if (!embeddedbitmaps)
      bitmap->load_flags |= FT_LOAD_NO_BITMAP;
  }

  bitmap->text_direction = text_direction;
  bitmap->use_harfbuzz = use_harfbuzz;
  bitmap->script = script;
  bitmap->lang = lang;

  if (text_direction >= 2)
    bitmap->load_flags |= FT_LOAD_VERTICAL_LAYOUT;

  return;
}

void ft_reduce_height(bitmap_t *bitmap, int new_height)
{
  int row;

  assert(new_height >= 0);
  assert(new_height <= bitmap->height);

  for (row = bitmap->height - 1; row > new_height - 1; row--)
    free(bitmap->data[row]);
  bitmap->height = new_height; 
  return;
}

void ft_free_bitmap(bitmap_t *bitmap)
{
  int row;
  for (row = 0; row < bitmap->height; row++)
    free(bitmap->data[row]);
  free(bitmap->data);

  bitmap->data = NULL;
  bitmap->height = 0;
  bitmap->width = 0;

  FT_Done_Face(bitmap->face);
  FT_Done_FreeType(bitmap->library);
  bitmap->load_flags = 0;
}

void draw_bitmap(FT_Bitmap *glyph,
                 FT_Int x,
                 FT_Int y, 
                 bitmap_t bitmap,
                 int monochrome)
{
  FT_Int i, j, p, q;
  FT_Int x_max = x + glyph->width;
  FT_Int y_max = y + glyph->rows;

  for (i = x, p = 0; i < x_max; i++, p++)
  {
    for (j = y, q = 0; j < y_max; j++, q++)
    {
      if (i < 0 || j < 0 ||
          i >= bitmap.width || j >= bitmap.height)
        continue;

      if (monochrome)
      {
        if (glyph->buffer[glyph->pitch * q + (p >> 3)] & (128 >> (p & 7)))
          bitmap.data[j][i] = 0;
      }
      else
      {
        bitmap.data[j][i] &= ~(glyph->buffer[q * glyph->width + p]);
      }
    }
  }
}

static int ft_draw_text_(FcChar32 text[], int x,  int y,
                         bitmap_t *bitmap, FT_Bool dry);

int ft_draw_text(FcChar32 text[], int x,  int y, bitmap_t *bitmap)
{
  return ft_draw_text_(text, x, y, bitmap, 0);
}

int ft_text_length(FcChar32 text[], FcPattern *pattern, int size, 
                   FcChar8 *font_file, FT_Bool use_harfbuzz,
                   int dir, const char *script, const char *lang)
{
  bitmap_t bitmap;
  int len;
  ft_initialize_bitmap(&bitmap, 0, 0);
  ft_bitmap_set_font(&bitmap, pattern, size, font_file, 
                     use_harfbuzz, dir, script, lang);
  len = ft_draw_text_(text, 0, 0, &bitmap, 1);
  ft_free_bitmap(&bitmap);
  return len;
}

int text_length(FcChar32 text[])
{
  int len = 0;
  while (text[len])
    len++;
  return len;
}

/* dry true: do not draw the bitmap, only return string length */
static int ft_draw_text_(FcChar32 text[], int x,  int y,
                  bitmap_t *bitmap, FT_Bool dry)
{
  FT_Error error;
  FT_Vector pen, delta;
  FT_Bool use_kerning;

  FT_UInt *glyph_codepoints;
  FT_Vector *glyph_offsets;
  FT_Vector *glyph_advances;
  FT_Vector *glyph_positions;
  FT_Glyph *glyphs;
  FT_GlyphSlot slot;

  int monochrome;
  int nglyphs, g;
  int sum_advances_x, sum_advances_y;
  int text_width;

  text_width = 0;
  if (bitmap->use_harfbuzz && FT_IS_SCALABLE(bitmap->face))
  {
    nglyphs = hbz_glyphs(text, 
                         text_length(text), 
                         bitmap->script, 
                         bitmap->lang, 
                         bitmap->text_direction,
                         bitmap->face, 
                         &glyph_codepoints, 
                         &glyph_offsets,
                         &glyph_advances,
                         &sum_advances_x,
                         &sum_advances_y);

    if (nglyphs == -1)
      return -1;

    glyph_positions = malloc(nglyphs*sizeof(FT_Vector));
    glyphs = malloc(nglyphs*sizeof(FT_Glyph));
    if (glyph_positions == NULL || glyphs == NULL)
    {
      fprintf(stderr, "ft_draw_text_(): out of memory\n");
      exit(1);
    }

 
    pen.x = x << 6;
    if (bitmap->text_direction == 1)
      pen.x -= sum_advances_x;
    pen.y = y << 6;
    if (bitmap->text_direction == 3)
      pen.y -= (-sum_advances_y);
    if (bitmap->text_direction == 2)
      pen.y -= glyph_advances[0].y - glyph_offsets[0].y;
    for (g = 0; g < nglyphs; g++)
    {
      glyph_positions[g].x = (pen.x + glyph_offsets[g].x) >> 6;
      glyph_positions[g].y = (pen.y - glyph_offsets[g].y) >> 6;
      pen.x += glyph_advances[g].x;
      pen.y -= glyph_advances[g].y;

      error = FT_Load_Glyph(bitmap->face, 
                            glyph_codepoints[g], bitmap->load_flags);
      if (error)
      {
        fprintf(stderr, "ft_draw_text(): can't load glyph\n");
        exit(1);
      }
 
      error = FT_Get_Glyph(bitmap->face->glyph, &glyphs[g]);
      if (error)
      {
        fprintf(stderr, "ft_draw_text(): can't get glyph\n");
        exit(1);
      }
    }

    /* one of operands is zero */
    text_width = (sum_advances_x + (-sum_advances_y)) >> 6;

    free(glyph_codepoints);
    free(glyph_offsets);
    free(glyph_advances);
  }
  else
  {
    use_kerning = FT_HAS_KERNING(bitmap->face) && bitmap->text_direction < 2;
    nglyphs = text_length(text);

    glyph_codepoints = malloc(nglyphs*sizeof(FT_UInt));
    glyph_offsets = malloc(nglyphs*sizeof(FT_Vector));
    glyph_advances = malloc(nglyphs*sizeof(FT_Vector));
    glyph_positions = malloc(nglyphs*sizeof(FT_Vector));
    glyphs = malloc(nglyphs*sizeof(FT_Glyph));
    if (glyph_positions == NULL || glyphs == NULL || 
        glyph_codepoints == NULL || glyph_offsets == NULL ||
        glyph_advances == NULL)
    {
      fprintf(stderr, "ft_draw_text_(): out of memory\n");
      exit(1);
    }

    pen.x = x << 6;
    pen.y = y << 6;
    for (g = 0; g < nglyphs; g++)
    {
      glyph_codepoints[g] =  FT_Get_Char_Index(bitmap->face, text[g]);

      if (!dry && !glyph_codepoints[g])
        return -1; /* text[c] is an undefined symbol in a face */

      if (use_kerning && g > 0 && glyph_codepoints[g])
      {
        switch (bitmap->text_direction)
        {
          case 0: /* left to right */
            FT_Get_Kerning(bitmap->face, 
                           glyph_codepoints[g-1], glyph_codepoints[g], 
                           FT_KERNING_DEFAULT, &delta);
            pen.x += delta.x;
            break;
          case 1: /* right to left */
            /* freetype 2 tutorial, step 2: Note that the ‘left’ and ‘right’ 
               positions correspond to the visual order of the glyphs in 
               the string of text. This is important for bidirectional text, 
               or simply when writing right-to-left text. */
             FT_Get_Kerning(bitmap->face, 
                           glyph_codepoints[g], glyph_codepoints[g-1], 
                           FT_KERNING_DEFAULT, &delta);
            pen.x -= delta.x;
            break;
          default:
            assert(1 == 0);
            break;
        }
      }

      error = FT_Load_Glyph(bitmap->face, 
                            glyph_codepoints[g], bitmap->load_flags);
      if (error)
      {
        fprintf(stderr, "ft_draw_text(): can't load glyph\n");
        exit(1);
      }
 
      slot = bitmap->face->glyph;
      error = FT_Get_Glyph(slot, &glyphs[g]);
      if (error)
      {
        fprintf(stderr, "ft_draw_text(): can't get glyph\n");
        exit(1);
      }

      if (bitmap->text_direction == 1)
        pen.x -= slot->advance.x;

      if (bitmap->text_direction == 2) 
        pen.y += slot->advance.y;

      glyph_positions[g].x = pen.x >> 6;
      glyph_positions[g].y = pen.y >> 6;

      if (bitmap->text_direction == 0) 
        pen.x += slot->advance.x;

      if (bitmap->text_direction == 3)
        pen.y -= slot->advance.y;
    }

    switch (bitmap->text_direction)
    {
      case 0: text_width = (pen.x >> 6) - x; break;
      case 1: text_width = x - (pen.x >> 6); break;
      case 2: text_width = (pen.y >> 6) - y; break;
      case 3: text_width = y - (pen.y >> 6); break;
      default: assert(1 == 0);
    }

    free(glyph_codepoints);
    free(glyph_offsets);
    free(glyph_advances);
  }

  if (! dry)
  {
    for (g = 0; g < nglyphs; g++)
    {
      monochrome = 0;
      if (bitmap->load_flags & FT_LOAD_TARGET_MONO || 
          glyphs[g]->format == FT_GLYPH_FORMAT_BITMAP)
        monochrome = 1;

      error = FT_Glyph_To_Bitmap(&glyphs[g], bitmap->render_mode, 
                                 NULL, 1);
      if (error)
      {
        fprintf(stderr, "ft_draw_text(): can't render glyph\n");
        exit(1);
      }

      FT_BitmapGlyph bit;
      bit = (FT_BitmapGlyph)glyphs[g];
      draw_bitmap(&bit->bitmap, 
                  glyph_positions[g].x + bit->left, 
                  glyph_positions[g].y - bit->top, 
                  *bitmap, monochrome);
    }
  }

  free(glyph_positions);
  free(glyphs);

  if (text_width < 0)
    text_width = 0;
  return text_width;
}

void ft_initialize_info(face_info_t *info, FcChar8 *file)
{
  FT_Error error;
  FT_ULong table_length;
  int t;

  error = FT_Init_FreeType(&info->library);
  if (error)
  {
    fprintf(stderr, 
            "ft_initialize_info():"" can't initialize freetype library\n");
    exit(1);
  }
  
  error = FT_New_Face(info->library, (char *)file, 0, &info->face);
  if (error)
  {
    fprintf(stderr, "ft_draw_text(): can't create face object\n");
    exit(1);
  }

  info->tt_tables_present = 0;
  for (t = 0; t < NUMTTTABLES; t++)
  {
    table_length = 0;
    FT_Load_Sfnt_Table(info->face, FT_MAKE_TAG(tt_tables[t][0], 
                                               tt_tables[t][1],
                                               tt_tables[t][2],
                                               tt_tables[t][3]), 
                       0, NULL, &table_length);
    info->tt_tables_present |= info->tt_tables[t] = table_length > 0;
  }

  /* shortcuts */
  info->num_fixed_sizes = info->face->num_fixed_sizes;
  info->available_sizes = info->face->available_sizes;

  return;
}

void ft_free_info(face_info_t *info)
{
  info->num_fixed_sizes = 0;
  info->available_sizes = NULL;

  FT_Done_Face(info->face);
  FT_Done_FreeType(info->library);
  return;
}

void ft_fill_region(bitmap_t *bitmap, int left, int top, 
                    int right, int bottom, unsigned char gray)
{
  int i, j;

  if (left >= bitmap->width ||
      top  >= bitmap->height ||
      right < 0 ||
      bottom < 0)
    return;

  if (left < 0)
    left = 0;
  if (top < 0)
    top = 0;
  if (right >= bitmap->width)
    right = bitmap->width - 1;
  if (bottom >= bitmap->height)
    bottom = bitmap->height - 1;

  for (i = left; i <= right; i++)
    for (j = top; j <= bottom; j++)
      bitmap->data[j][i] = gray;
}

void ft_rot270(bitmap_t *bitmap)
{
  int tmp;
  int row, col;
  unsigned char **data;

  data = bitmap->data;

  tmp = bitmap->height;
  bitmap->height = bitmap->width;
  bitmap->width = tmp;

  bitmap->data 
    = (unsigned char **)malloc(bitmap->height*sizeof(unsigned char *));
  if (! bitmap->data)
  {
    fprintf(stderr, "ft_bitmap_transform(): no free memory\n");
    exit(1);
  }

  for (row = 0; row < bitmap->height; row++)
  {
    bitmap->data[row]
      = (unsigned char *)malloc(bitmap->width*sizeof(unsigned char));
    if (! bitmap->data[row])
    {
      fprintf(stderr, "ft_bitmap_transform(): no free memory\n");
      exit(1);
    }
  }

  for (row = 0; row < bitmap->height; row++)
    for (col = 0; col < bitmap->width; col++)
      bitmap->data[row][bitmap->width-col-1] = data[col][row];

  /* bitmap->width is now former bitmap->height */
  for (row = 0; row < bitmap->width; row++)
    free(data[row]);
  free(data);

  return;
}

