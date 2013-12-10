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

#include <assert.h>

#include "img_common.h"
#include "ft.h"
#include "fcinfo.h"

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
                     int wanted_size)
{
  FcBool scalable;
  int i, px;
  double size;

  assert(fcinfo_get_translated_string(pattern, FC_FAMILY, LANG_EN, family)
         == FcResultMatch);
  if (style)
    assert(fcinfo_get_translated_string(pattern, FC_STYLE, LANG_EN, style)
           == FcResultMatch);
  if (charset)
    assert(FcPatternGetCharSet(pattern, FC_CHARSET, 0, charset)
           == FcResultMatch);
  if (slant)
    assert(FcPatternGetInteger(pattern, FC_SLANT, 0, slant)
           == FcResultMatch);
  if (weight)
    assert(FcPatternGetInteger(pattern, FC_WEIGHT, 0, weight)
           == FcResultMatch);
  if (width)
    assert(FcPatternGetInteger(pattern, FC_WIDTH, 0, width)
           == FcResultMatch);

  assert(FcPatternGetBool(pattern, FC_SCALABLE, 0, &scalable)
         == FcResultMatch);

  bzero(sizes, sizeof(int)*(SIZES_MAX + 1));

  if (! scalable)
  {
    for (i = 0;
         FcPatternGetDouble(pattern, FC_PIXEL_SIZE, i, &size) == FcResultMatch;
         i++)
      if ((int)size <= SIZES_MAX)
      {
        sizes[(int)size] = FcTrue;
        if (files)
          assert(FcPatternGetString(pattern, FC_FILE, i, &files[(int)size])
                 == FcResultMatch);

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
      if (files)
        assert(FcPatternGetString(pattern, FC_FILE, 0, &files[wanted_size])
               == FcResultMatch);
      return wanted_size;
    }

    for (px = config.specimen_from_px; px <= config.specimen_to_px; px++)
    {
      sizes[px] = FcTrue;
      if (files)
        assert(FcPatternGetString(pattern, FC_FILE, 0, &files[px])
               == FcResultMatch);
    }
  }

  return 0;
}

int naive_specimen_text_size(int sizes[], config_t config)
{
  int size, px;

  size = SPECIMEN_DIST;
  for (px = 1; px <= SIZES_MAX; px++)
    if (sizes[px])
    {
      size += (px + SPECIMEN_DIST);

      /* one entry above config.specimen_to_px is enough */
      /* for minispecimen and for specimen too */
      if (px > config.specimen_to_px)
        break;
    }

  return size;
}

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
                       int *height)
{
  int px;

  if (best_size)
  {
    px = best_size;
  }
  else
  {
    for (px = SIZES_MAX; px >= 0; px--)
      if (sizes[px])
        break;
    assert(px > 0);
  }

  /* e. g. 'Arabic Newspaper' reports only size 32 */
  /* that's why we consider specimen entries over */
  /* config.specimen_to_px */

  /* width */
  if (text_direction < 2)
  {
    *width = ft_text_length(sentence, font, px,
                            files[px], use_harfbuzz,
                            text_direction, script, lang);
    if (*width <= 0)
      *width = maxwidth;

  }
  else
  {
    *width = naive_specimen_text_size(sizes, config);
  }

  if (maxwidth && *width > maxwidth)
    *width = maxwidth;

  /* height */
  /* svg_height for vertical layouts could be computed as 
     svg_width  for horizontal layouts (ft_text_length
     supports vertical layouts); but it doesn't look good 
     when switching between specimen of horizontal and
     vertical layouts -> we want specimens to have same 
     height or ? */
  *height = naive_specimen_text_size(sizes, config);

  return;
}
