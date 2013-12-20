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

#ifndef FCINFO_H
# define FCINFO_H

# include <fontconfig/fontconfig.h>

#include "constants.h"

#define LANG_EN             (FcChar8 *)"en"

#define FONTFORMAT_MAX      16
#define CAPABILITY_MAX    1024
#define FOUNDRY_MAX        128

#define ALL_CHARS   0xFFFFFFFF
#define NO_CHAR            '\0'

typedef struct
{
  char fontformat[FONTFORMAT_MAX];
  char capability[CAPABILITY_MAX];
  char foundry[FOUNDRY_MAX];
} family_info_t;

FcFontSet * fcinfo(const FcConfig *config, const FcPattern *pattern, 
                   FcBool remove_duplicities, int argnum, ...);

FcResult fcinfo_get_translated_string(const FcPattern *pattern, 
                                      const char *object, 
                                      const FcChar8 *lang, FcChar8 **s);


int fcinfo_utf8toucs4(const FcChar8 *utf8str, FcChar32 *ucs4str, int max_len);
/* returns idx for which the algorithm terminated */
/* => if return value == len, then charset coveres sentence, */
/* otherwise return value is first character in the sentence */
/* which is not element of charset */
int fcinfo_charset_coveres_sentence(FcCharSet *charset, 
                                    FcChar32 *sentence,
                                    int len,
                                    const char *script,
                                    int *charsfromscript);
int fcinfo_charset_generate_sentence(FcCharSet *charset, 
                                     const char *script,
                                     FcChar32 *sentence, 
                                     int maxchars);

void fc_version(int *maj, int *min, int *rev);

FcFontSet *fcinfo_families_index(const FcPattern *filter);
FcFontSet *fcinfo_styles_index(const FcChar8 *family, const FcPattern *filter, 
                               family_info_t *fi);
FcFontSet *fcinfo_font_index(const FcPattern *filter);

FcStrSet *fcinfo_languages(const FcPattern *pattern);
FcFontSet *fcinfo_language_font_index(const FcChar8 *lang, 
                                      const FcPattern *filter);

FcStrSet *fcinfo_fontformats(const FcPattern *filter);
FcFontSet *fcinfo_fontformat_family_index(const FcChar8 *format,
                                          const FcPattern *filter);

FcFontSet *fcinfo_uinterval_statistics(const FcPattern *filter,
                                       double *uinterval_stat[],
                                       uinterval_type_t uintype);

FcChar32 fcinfo_chars(FcCharSet *charset, FcChar32 **chars,
                      const char *uinterval, uinterval_type_t uintype,
                      FcBool bl, FcChar32 maxchars);

FcPattern *fcinfo_name_parse(const FcChar8 *pattern);
FcPattern *fcinfo_get_font(const FcChar8 *request);
#endif
