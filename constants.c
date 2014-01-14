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

#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "constants.h"

const FcChar8 *const_name(int c, const FcStringConst * consts, int nc)
{
  int i;

  for (i = 0; i < nc; i++)
    if (c == consts[i].value)
      return consts[i].name;

  return (CONST_STRING_UNKNOWN);
}

const FcChar8 *lang_name(const FcChar8 *l)
{
  int i;

  for (i = 0; i < NUM_CONSTS(languages_consts); i++)
    if (! strcmp((const char *)l, languages_consts[i].l))
      return (const FcChar8 *)languages_consts[i].lang;

  return (CONST_STRING_UNKNOWN);
}

/* TODO: this is sorted, so we could use bisection 
   or like */
const FcChar8 * unicode_char_name(FcChar32 ch)
{
  FcChar32 i;
  
  for (i = 0; i < NUM_CONSTS(unicode_char_consts); i++)
    if (unicode_char_consts[i].num == ch)
      return (const FcChar8 *)unicode_char_consts[i].name;

  return (CONST_STRING_UNKNOWN);
}

/* mapping is not one to one, hope this is correct;
   assuming that font provides only one from {BOOK, 
   REGULAR} and {BLACK, ULTRABLACK} */
int fc_to_css_weight(int fc_weight)
{
  switch (fc_weight)
  {
    case FC_WEIGHT_THIN:       /*   0 */
      return 100;
    case FC_WEIGHT_ULTRALIGHT: /*  40 */
      return 200;
    case FC_WEIGHT_LIGHT:      /*  50 */
      return 300;
    case FC_WEIGHT_BOOK:       /*  75 */
    case FC_WEIGHT_REGULAR:    /*  80 */
      return 400;
    case FC_WEIGHT_MEDIUM:     /* 100 */
      return 500;
    case FC_WEIGHT_SEMIBOLD:   /* 180 */
      return 600;
    case FC_WEIGHT_BOLD:       /* 200 */
      return 700;
    case FC_WEIGHT_EXTRABOLD:  /* 205 */
      return 800;
    case FC_WEIGHT_BLACK:      /* 210 */
    case FC_WEIGHT_ULTRABLACK: /* 215 */
      return 900;
    default:
      fprintf(stderr, 
              "fc_to_css_weight(): fontconfig font "
              "weight %d not known, choosing 400\n", 
              fc_weight);
      return 400;
  }
}

uint32_t unicode_script_tag(const char *script)
{
  int s;
  for (s = 0; s < NUM_CONSTS(script_data_consts); s++)
    if (strcmp(script_data_consts[s].script, script) == 0)
      return script_data_consts[s].tag;

  return 0;
}

int unicode_script_exists(const char *script)
{
  int s;
  for (s = 0; s < NUMSCRIPTS; s++)
    if (strcmp(script_consts[s].name, script) == 0)
      return 1;

  return 0;
}

int unicode_interval_contains(const char *uinterval_name, 
                              uinterval_type_t type,
                              FcChar32 ch)
{
  int in;
  const uinterval_map_t *map = uinterval_maps(type);
  for (in = 0; in < uinterval_num_maps(type); in++)
  {
    if (strcmp(map[in].interval_name, uinterval_name) == 0)
    {
      if (map[in].l <= ch && ch <= map[in].u)
        return 1;
    }
    else
    {
      if (map[in].l <= ch && ch <= map[in].u)
        return 0;
    }

  }
  return 0;
}

void unicode_script_sentences(const char *script,
                              int *nsentences,
                              sentence_t **sentences,
                              int *dir,
                              img_transform_t *transform)
{
  int sd;
  *sentences = NULL;
  *nsentences = 0;
  for (sd = 0; sd < NUM_CONSTS(script_data_consts); sd++)
    if (strcmp(script_data_consts[sd].script, script) == 0)
    {
      *nsentences = script_data_consts[sd].nsentences;
      *sentences = script_data_consts[sd].sentences;
      *dir = script_data_consts[sd].dir;
      if (transform)
        *transform = script_data_consts[sd].transform;
      break;
    }

  return;
}

const char *unicode_script_name(int id)
{
  assert(0 <= id);
  assert(id < NUMSCRIPTS);
  return script_consts[id].name;
}

int script_is_collection(const char *script_name)
{
  int s;
  for (s = 0; s < NUMSCRIPTS; s++)
    if (strcmp(script_name, script_consts[s].name) == 0)
      return script_consts[s].is_collection;

  assert(1 == 0);
}

const char *unicode_block_name(int id)
{
  assert(0 <= id);
  assert(id < NUMBLOCKS);
  return block_map_consts[id].interval_name;
}

double charset_uinterval_coverage(FcCharSet *charset, 
                                  const char *uinname,
                                  uinterval_type_t uintype,
                                  FcChar32 *ch_success,
                                  FcChar32 *ui_size)
{
  int m;
  FcChar32 c;
  FcChar32 uinterval_size, charset_success;

  /* could be done with FcCharSetIsSubset(), but this is without */
  /* allocating memory */
  uinterval_size = charset_success = 0;
  for (m = 0; m < uinterval_num_maps(uintype); m++)
  {
    if (! strcmp(uinname, uinterval_maps(uintype)[m].interval_name))
      for (c = uinterval_maps(uintype)[m].l; 
           c < uinterval_maps(uintype)[m].u; c++)
      {
        if (FcCharSetHasChar(charset, c) == FcTrue)
          charset_success++;
        uinterval_size++;
      }
  }
  assert(uinterval_size > 0);
  if (ch_success)
    *ch_success = charset_success;
  if (ui_size) 
    *ui_size = uinterval_size;
  return (double)charset_success/(double)uinterval_size*100.0;
}

int charset_uinterval_statistics(FcCharSet *charset,
                                 uinterval_stat_t stats[],
                                 uinterval_type_t uintype,
                                 uinterval_sort_t sort_type)
{
  int nintervals = 0;
  int v, s, v2, loop_end;

  const int map_len = uinterval_num(uintype);

  uinterval_stat_t stat;

  bzero(stats, sizeof(uinterval_stat_t)*map_len);

  /* sort intervals according to coverage values in the font */
  for (s = 0; s < map_len; s++)
  {
    stat.ui_name = uinterval_name(s, uintype);
    stat.coverage
       = charset_uinterval_coverage(charset, 
                                    stat.ui_name, 
                                    uintype,
                                    &stat.success,
                                    &stat.uinterval_size);
    if (stat.success == 0)
      continue; /* trow stat away */

    v = 0;
    loop_end = 0;
    while (v < nintervals)
    {
      switch (sort_type)
      {
        case UI_SORT_NONE:
          break;
        case UI_SORT_ABSOLUTE:
          if (stat.success > stats[v].success)
            loop_end = 1;
          break;
        case UI_SORT_PERCENT:
          if (stat.coverage > stats[v].coverage)
            loop_end = 1;
          break;
        default:
          assert(0 == 1);
      }

      if (loop_end)
        break;

      v++;
    }
    for (v2 = nintervals; v2 > v; v2--)
    {
      if (v2 == map_len)
        continue;

      stats[v2] = stats[v2 - 1];
    }

    stats[v] = stat;
    nintervals++;
  }

  return nintervals;
}

int unicode_script_blocks(const char *script,
                          const char *blocks[SCRIPT_BLOCKS_MAX],
                          int lbounds[SCRIPT_BLOCKS_MAX],
                          int ubounds[SCRIPT_BLOCKS_MAX])
{
  int nblocks = 0, sm, b;

  for (b = 0; b < NUMBLOCKS; b++)
  {
    for (sm = 0; sm < NUM_CONSTS(script_map_consts); sm++)
      if (strcmp(script_map_consts[sm].interval_name, script) == 0 && 
                 block_map_consts[b].l <= script_map_consts[sm].l &&
                 script_map_consts[sm].u <= block_map_consts[b].u)
      { /* script map of script is subset of block => block is subset of script */

        assert(nblocks < SCRIPT_BLOCKS_MAX);
        blocks[nblocks] = block_map_consts[b].interval_name;
        lbounds[nblocks] = block_map_consts[b].l;
        ubounds[nblocks] = block_map_consts[b].u;
        nblocks++;
        break;
      }
  }

  return nblocks;
}

int block_bound(const char *block_name, int upper_bound)
{
  int b;
  for (b = 0; b < NUMBLOCKS; b++)
  {
    if (strcmp(block_map_consts[b].interval_name, block_name) == 0)
      return upper_bound ? block_map_consts[b].u : block_map_consts[b].l;
  }

  assert(1 == 0);
  return 0;
}

int block_ubound(const char *block_name)
{
  return block_bound(block_name, 1);
}

int block_lbound(const char *block_name)
{
  return block_bound(block_name, 0);
}
