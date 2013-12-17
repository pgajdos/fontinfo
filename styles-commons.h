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

#ifndef STYLES_COMMON_H
# define STYLES_COMMON_H

# include "configuration.h"
# include "fcinfo.h"

# define SIZES_MAX                 64

# define SCRIPT_SENTENCE_LEN_MIN   4

# define FAMILY_NAME_LEN_MAX 128
# define STYLE_NAME_LEN_MAX   64
# define SCRIPT_NAME_LEN_MAX  32
# define BLOCK_NAME_LEN_MAX   64

# define ASIZE(constsArray)  \
        (int) (sizeof (constsArray) / sizeof (constsArray[0]))

typedef struct
{
  char *text;
  char *href; 
  char *title;
  int active;
} html_link;

extern const html_link null_link;
#define is_null_link(a) (a.text == NULL && a.href == NULL && \
                         a.title == NULL && a.active == 0)

typedef struct
{
  char *href_pref;  /* e. g. ../ */
  const html_link *links;
  int nlinks;
  char *title;
} html_links;

typedef enum
{
  TS_YEAR,
  TS_DATE
} ts_output_t;

FcFontSet *families_index(config_t config);
FcFontSet *styles_index(const FcChar8 *family, config_t config, 
                        family_info_t *fi);
FcFontSet *font_index(config_t config);

FcStrSet *language_index(config_t config);
FcFontSet *language_font_index(const FcChar8 *lang, config_t config);

FcStrSet *fontformats(config_t config);
FcFontSet *fontformat_family_index(const FcChar8 *format, config_t config);

FcFontSet *uinterval_statistics(config_t config,
                                double *uinterval_stat[],
                                uinterval_type_t uintype);

char *fontinfo_version(char *string, int maxlen);
char *fontconfig_version(char *string, int maxlen);

void timestamp(char *dest, ts_output_t o);

/* script can be NULL */
/* random: 0 sentence, 1 random chars from script, 
           2 random chars from whole font */ 
int specimen_sentence(config_t config, FcCharSet *charset, 
                      const char *wanted_script, int *dir, 
                      const char **lang, 
                      int *random, FcChar32 *ucs4str, int maxlen);

void underscores_to_spaces(char *str);

const char **thanks_html_text(void);

#endif
