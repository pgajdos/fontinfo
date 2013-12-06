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

#ifndef FT_H
# define FT_H

#include <stdint.h>

#include <fontconfig/fontconfig.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "filesystem.h"

/* http://scripts.sil.org/cms/scripts/page.php?item_id=IWS-AppendixC */
static const char *tt_tables[] =
{
  "acnt",
  "avar",
  "BASE",
  "bdat",
  "BDF ",
  "bhed",
  "bloc",
  "bsln",
  "CBDT",
  "CBLC",
  "CFF ",
  "CID ",
  "cmap",
  "cvar",
  "cvt ",
  "DSIG",
  "EBDT",
  "EBLC",
  "EBSC",
  "fdsc",
  "feat", 
  "Feat",
  "fmtx", 
  "FOND",
  "fpgm",
  "fvar",
  "gasp",
  "GDEF",
  "Glat",
  "Glot",
  "glyf",
  "GPOS",
  "GSUB",
  "gvar",
  "hdmx",
  "head",
  "hhea",
  "hsty",
  "hmtx",
  "JSTF",
  "just",
  "kern",
  "lcar",
  "loca",
  "LTSH",
  "LWFN",
  "MATH",
  "maxp",
  "META",
  "MMFX",
  "MMSD",
  "mort",
  "morx",
  "name",
  "opbd",
  "OS/2",
  "OTTO",
  "PCLT",
  "POST",
  "post",
  "prep",
  "prop",
  "sfnt",
  "Silf",
  "SING",
  "trak",
  "true",
  "ttc ",
  "ttcf",
  "TYP1",
  "typ1",
  "VDMX",
  "vhea",
  "vmtx",
  "VORG",
  "Zapf"
};

#define NUMTTTABLES \
  (int) (sizeof (tt_tables) / sizeof (tt_tables[0]))

typedef struct
{
  unsigned char **data;
  int width;
  int height;

  FT_Library library;
  FT_Face face;
  int text_direction;
  FT_Bool use_harfbuzz;
  const char *script;
  uint32_t script_tag;
  const char *lang;
  FT_Int32 load_flags;
  FT_Render_Mode render_mode;
} bitmap_t;

typedef struct
{
  /* shortcuts into face object */
  FT_Int num_fixed_sizes;
  FT_Bitmap_Size *available_sizes;

  FT_Bool tt_tables_present;
  FT_Bool tt_tables[NUMTTTABLES];

  FT_Library library;
  FT_Face face;
} face_info_t;

/* pxsize == 0 -> don't initialize face */
char *freetype_version(char *string, int maxlen);
const char *harfbuzz_version(void);
void ft_initialize_bitmap(bitmap_t *bitmap, int height, int width);
void ft_bitmap_set_font(bitmap_t *bitmap, 
                        FcPattern *pattern, 
                        int pxsize, 
                        FcChar8 *file, 
                        FT_Bool use_harfbuzz,
                        int text_direction, 
                        const char *script, 
                        const char *lang);
void ft_reduce_height(bitmap_t *bitmap, int newheight);
void ft_free_bitmap(bitmap_t *bitmap);

int ft_text_length(FcChar32 text[], FcPattern *pattern, int size, 
                   FcChar8 *font_file, FT_Bool use_harfbuzz,
                   int dir, const char *script, const char *lang);
int ft_draw_text(FcChar32 text[], int x, int y, bitmap_t *bitmap);

void ft_initialize_info(face_info_t *info, FcChar8 *file);
void ft_free_info(face_info_t *info);

void ft_fill_region(bitmap_t *bitmap, int left, int top, 
                    int right, int bottom, unsigned char gray);

#endif
