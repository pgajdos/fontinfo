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

#ifndef FCINFO_CONSTANTS_H
# define FCINFO_CONSTANTS_H

#include <fontconfig/fontconfig.h>
#include <stdint.h>

typedef struct _FcStringConst 
{
    const FcChar8   *name;
    int             value;
} FcStringConst;

static const FcStringConst  weightConsts[] = 
{
    { (FcChar8 *) "thin",           FC_WEIGHT_THIN },
    { (FcChar8 *) "extralight",     FC_WEIGHT_EXTRALIGHT },
    { (FcChar8 *) "ultralight",     FC_WEIGHT_EXTRALIGHT },
    { (FcChar8 *) "light",          FC_WEIGHT_LIGHT },
    { (FcChar8 *) "book",           FC_WEIGHT_BOOK },
    { (FcChar8 *) "regular",        FC_WEIGHT_REGULAR },
    { (FcChar8 *) "medium",         FC_WEIGHT_MEDIUM },
    { (FcChar8 *) "demibold",       FC_WEIGHT_DEMIBOLD },
    { (FcChar8 *) "semibold",       FC_WEIGHT_DEMIBOLD },
    { (FcChar8 *) "bold",           FC_WEIGHT_BOLD },
    { (FcChar8 *) "extrabold",      FC_WEIGHT_EXTRABOLD },
    { (FcChar8 *) "ultrabold",      FC_WEIGHT_EXTRABOLD },
    { (FcChar8 *) "black",          FC_WEIGHT_BLACK },
    { (FcChar8 *) "heavy",          FC_WEIGHT_HEAVY }
};

static const FcStringConst  widthConsts[] = 
{
    { (FcChar8 *) "ultracondensed", FC_WIDTH_ULTRACONDENSED },
    { (FcChar8 *) "extracondensed", FC_WIDTH_EXTRACONDENSED },
    { (FcChar8 *) "condensed",      FC_WIDTH_CONDENSED },
    { (FcChar8 *) "semicondensed",  FC_WIDTH_SEMICONDENSED },
    { (FcChar8 *) "normal",         FC_WIDTH_NORMAL },
    { (FcChar8 *) "semiexpanded",   FC_WIDTH_SEMIEXPANDED },
    { (FcChar8 *) "expanded",       FC_WIDTH_EXPANDED },
    { (FcChar8 *) "extraexpanded",  FC_WIDTH_EXTRAEXPANDED },
    { (FcChar8 *) "ultraexpanded",  FC_WIDTH_ULTRAEXPANDED }
};

static const FcStringConst  stretchConsts[] = 
{ /* for html <span> */
    { (FcChar8 *) "ultra-condensed", FC_WIDTH_ULTRACONDENSED },
    { (FcChar8 *) "extra-condensed", FC_WIDTH_EXTRACONDENSED },
    { (FcChar8 *) "condensed",       FC_WIDTH_CONDENSED },
    { (FcChar8 *) "semi-condensed",  FC_WIDTH_SEMICONDENSED },
    { (FcChar8 *) "normal" ,         FC_WIDTH_NORMAL },
    { (FcChar8 *) "semi-expanded",   FC_WIDTH_SEMIEXPANDED },
    { (FcChar8 *) "expanded",        FC_WIDTH_EXPANDED },
    { (FcChar8 *) "extra-expanded",  FC_WIDTH_EXTRAEXPANDED },
    { (FcChar8 *) "ultra-expanded",  FC_WIDTH_ULTRAEXPANDED }
};

static const FcStringConst  slantConsts[] = 
{
    { (FcChar8 *) "roman",            FC_SLANT_ROMAN },
    { (FcChar8 *) "italic",           FC_SLANT_ITALIC },
    { (FcChar8 *) "oblique",          FC_SLANT_OBLIQUE }
};

static const FcStringConst  styleConsts[] = 
{
    { (FcChar8 *) "normal",           FC_SLANT_ROMAN },
    { (FcChar8 *) "italic",           FC_SLANT_ITALIC },
    { (FcChar8 *) "oblique",          FC_SLANT_OBLIQUE }
};

static const FcStringConst spacingConsts[] = 
{
    { (FcChar8 *) "proportional",   FC_PROPORTIONAL },
    { (FcChar8 *) "dual",           FC_DUAL },
    { (FcChar8 *) "mono",           FC_MONO },
    { (FcChar8 *) "charcell",       FC_CHARCELL }
};

static const FcStringConst hintstyleConsts[] = 
{
    { (FcChar8 *) "hintnone",       FC_HINT_NONE },
    { (FcChar8 *) "hintslight",     FC_HINT_SLIGHT },
    { (FcChar8 *) "hintmedium",     FC_HINT_MEDIUM },
    { (FcChar8 *) "hintfull",       FC_HINT_FULL },
};

static const FcStringConst rgbaConsts[] = 
{
    { (FcChar8 *) "unknown",        FC_RGBA_UNKNOWN },
    { (FcChar8 *) "rgb",            FC_RGBA_RGB, },
    { (FcChar8 *) "bgr",            FC_RGBA_BGR, },
    { (FcChar8 *) "vrgb",           FC_RGBA_VRGB },
    { (FcChar8 *) "vbgr",           FC_RGBA_VBGR },
    { (FcChar8 *) "none",           FC_RGBA_NONE },
};

static const FcStringConst lcdfilterConsts[] = 
{
    { (FcChar8 *) "lcdnone",        FC_LCD_NONE },
    { (FcChar8 *) "lcddefault",     FC_LCD_DEFAULT },
    { (FcChar8 *) "lcdlight",       FC_LCD_LIGHT },
    { (FcChar8 *) "lcdlegacy",      FC_LCD_LEGACY },
};

typedef struct
{
  const char *l;
  const char *lang;
} lang_t;

static const lang_t languages_consts[] = 
{
  #include "languages/languages.txt"
};

typedef struct
{
  FcChar32 num;
  const char *name;
} unicode_lang_t;

static const unicode_lang_t unicode_char_consts[] = 
{
  #include "unicode/unicode.txt"
};

typedef struct
{
  const char *name;
  int is_collection;
} script_t;

static const script_t script_consts[] = 
{
  #include "unicode/scripts.txt"
};

/* script map interval */
typedef struct
{
  FcChar32 l; /* lower bound */
  FcChar32 u; /* uppser bound */
  const char *interval_name;
} uinterval_map_t;

typedef struct
{
  const char *ui_name;
  double coverage;
  FcChar32 success;
  FcChar32 uinterval_size;
} uinterval_stat_t;

typedef enum
{
  UI_NONE,
  SCRIPT,
  BLOCK,
} uinterval_type_t;

typedef enum
{
  UI_SORT_NONE,
  UI_SORT_ABSOLUTE,
  UI_SORT_PERCENT,
} uinterval_sort_t;

static const uinterval_map_t script_map_consts[] = 
{
  #include "unicode/scripts-map.txt"
};

#define SCRIPT_BLOCKS_MAX  16

static const uinterval_map_t block_map_consts[] =
{
  #include "unicode/blocks-map.txt"
};

#define uinterval_maps(uintype) \
  (uintype == SCRIPT ? script_map_consts : block_map_consts)
#define uinterval_num_maps(uintype) \
  (uintype == SCRIPT ? NUM_CONSTS(script_map_consts) : NUMBLOCKS)
#define uinterval_num(uintype) \
  (uintype == SCRIPT ? NUMSCRIPTS : NUMBLOCKS)
#define uinterval_name(index, uintype) \
  (uintype == SCRIPT ? script_consts[index].name \
                     : block_map_consts[index].interval_name)
#define NO_LANG     ""
#define NO_SCRIPT   ""

typedef struct 
{
  const char *sent;
  const char *lang;
} sentence_t;

#define L2R 0
#define R2L 1
#define T2B 2
#define B2T 3

#define TAG(c1,c2,c3,c4) ((uint32_t)((((uint8_t)(c1))<<24)|(((uint8_t)(c2))<<16)|(((uint8_t)(c3))<<8)|((uint8_t)(c4))))

typedef enum
{
  TRNS_NONE,
  TRNS_ROT270
} img_transform_t;

typedef struct
{
  const char *script;
  uint32_t tag;
  int nsentences;
  int dir;
  img_transform_t transform;
  sentence_t *sentences;
} script_data_t;

static const script_data_t script_data_consts[] =
{
  #include "unicode/sentences.txt"
};

#define CONST_STRING_UNKNOWN   (FcChar8 *)"unknown"
#define CONST_NUM_UNKNOWN      -1
#define CONST_BOOL_UNKNOWN     -1
#define CONST_LANGSET_UNKNOWN  NULL
#define CONST_CHARSET_UNKNOWN  NULL
#define CONST_FTFACE_UNKNOWN   NULL

#define NUM_CONSTS(constsArray)  \
        (int) (sizeof (constsArray) / sizeof (constsArray[0]))
#define NUMSCRIPTS NUM_CONSTS(script_consts)
#define NUMBLOCKS  NUM_CONSTS(block_map_consts)

const FcChar8 * const_name(int c, const FcStringConst * consts, int nc);

#define weight_name(w)     \
	const_name(w, weightConsts, NUM_CONSTS(weightConsts))
#define width_name(w)      \
	const_name(w, widthConsts, NUM_CONSTS(widthConsts))
#define stretch_name(w)    \
	const_name(w, stretchConsts, NUM_CONSTS(widthConsts))
#define slant_name(s)      \
	const_name(s, slantConsts, NUM_CONSTS(slantConsts))
#define style_name(s)      \
	const_name(s, styleConsts, NUM_CONSTS(styleConsts))
#define spacing_name(s)    \
	const_name(s, spacingConsts, NUM_CONSTS(spacingConsts))
#define hintstyle_name(h)  \
	const_name(h, hintstyleConsts, NUM_CONSTS(hintstyleConsts))
#define rgba_name(a) \
        const_name(a, rgbaConsts, NUM_CONSTS(rgbaConsts))
#define lcdfilter_name(f) \
        const_name(f, lcdfilterConsts, NUM_CONSTS(lcdfilterConsts))

const FcChar8 *lang_name(const FcChar8 *l);
const FcChar8 *unicode_char_name(FcChar32 ch);
int fc_to_css_weight(int fc_weight);
uint32_t unicode_script_tag(const char *script);
int unicode_script_exists(const char *script);
int unicode_interval_contains(const char *script, 
                              uinterval_type_t type, 
                              FcChar32 ch);
void unicode_script_sentences(const char *script,
                              int *nsentences,
                              sentence_t **sentences,
                              int *dir,
                              img_transform_t *transform);
const char *unicode_script_name(int id);
const char *unicode_block_name(int id);
double charset_uinterval_coverage(FcCharSet *charset, 
                                  const char *uinterval_name,
                                  uinterval_type_t uintype,
                                  FcChar32 *ch_success,
                                  FcChar32 *ui_size);
int charset_uinterval_statistics(FcCharSet *charset,
                                 uinterval_stat_t scripts[],
                                 uinterval_type_t uintype,
                                 uinterval_sort_t sort_type);
int unicode_script_blocks(const char *script,
                          const char *blocks[SCRIPT_BLOCKS_MAX],
                          int lbounds[SCRIPT_BLOCKS_MAX],
                          int ubounds[SCRIPT_BLOCKS_MAX]);
/* following two have not sense for scripts as they 
   are not neccesarily compact */
int block_ubound(const char *block_name);
int block_lbound(const char *block_name);
int script_is_collection(const char *script_name);
#endif
