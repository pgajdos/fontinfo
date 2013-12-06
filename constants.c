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

int unicode_script_contains(const char *script, FcChar32 ch)
{
  int sm;
  for (sm = 0; sm < NUM_CONSTS(script_map_consts); sm++)
  {
    if (strcmp(script_map_consts[sm].interval_name, script) == 0)
    {
      if (script_map_consts[sm].l <= ch && ch <= script_map_consts[sm].u)
        return 1;
    }
    else
    {
      if (script_map_consts[sm].l <= ch && ch <= script_map_consts[sm].u)
        return 0;
    }

  }
  return 0;
}

void unicode_script_sentences(const char *script,
                              int *nsentences,
                              sentence_t **sentences,
                              int *dir)
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
                                  uinterval_type_t uintype)
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
  return (double)charset_success/(double)uinterval_size*100.0;
}

int charset_uinterval_statistics(FcCharSet *charset,
                                 const char *uinnames[],
                                 double values[],
                                 uinterval_type_t uintype)
{
  int nscripts = 0;
  int v, s, v2;
  double cov;

  const int map_len = uinterval_num(uintype);

  bzero(values, sizeof(double)*map_len);
  bzero(uinnames, sizeof(char*)*map_len);

  /* sort scripts according to coverage values in the font */
  for (s = 0; s < map_len; s++)
  {
    cov = charset_uinterval_coverage(charset, 
                                     uinterval_name(s, uintype), 
                                     uintype);
    if (cov == 0)
      continue;

    v = 0;
    while (v < nscripts)
    {
      if (cov > values[v])
        break;
      v++;
    }
    for (v2 = nscripts; v2 > v; v2--)
    {
      if (v2 == map_len)
        continue;

      uinnames[v2] = uinnames[v2 - 1];
      values[v2] = values[v2 - 1];
    }

    uinnames[v] = uinterval_name(s, uintype);

    values[v] = cov;
    nscripts++;
  }

  return nscripts;
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


