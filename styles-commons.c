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

#include <fontconfig/fontconfig.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include "styles-commons.h"
#include "constants.h"

const html_link null_link = {NULL, NULL, NULL, 0};

static const char *thanks_html[] = 
{
  #include "doc/thanks.txt"
  NULL
};

FcFontSet *families_index(config_t config)
{
  FcPattern *filter;
  FcFontSet *result;

  FcInit();
  filter = fcinfo_name_parse((FcChar8 *)config.pattern_string);
  result = fcinfo_families_index(filter);

  FcPatternDestroy(filter);
  return result;
}

FcFontSet *styles_index(const FcChar8 *family, config_t config, 
                        family_info_t *fi)
{
  FcFontSet *result;
  FcPattern *filter;

  FcInit();
  filter = fcinfo_name_parse((FcChar8 *)config.pattern_string);
  result = fcinfo_styles_index(family, filter, fi);

  FcPatternDestroy(filter);
  return result;
}

FcFontSet *font_index(config_t config)
{
  FcPattern *filter;
  FcFontSet *result;

  FcInit();
  filter = fcinfo_name_parse((FcChar8 *) config.pattern_string);
  result = fcinfo_font_index(filter);

  FcPatternDestroy(filter);
  return result;
}

FcStrSet *language_index(config_t config)
{
  FcStrSet *result;
  FcPattern *filter;

  FcInit();
  filter = fcinfo_name_parse((FcChar8 *) config.pattern_string);
  result = fcinfo_languages(filter);

  FcPatternDestroy(filter);
  return result;
}

FcFontSet *language_font_index(const FcChar8 *lang, config_t config)
{
  FcFontSet *result;
  FcPattern *filter;

  FcInit();
  filter = fcinfo_name_parse((FcChar8 *) config.pattern_string);
  result = fcinfo_language_font_index(lang, filter);

  FcPatternDestroy(filter);
  return result;
}

FcStrSet *fontformats(config_t config)
{
  FcStrSet *result;
  FcPattern *filter;

  FcInit();
  filter = fcinfo_name_parse((FcChar8 *) config.pattern_string);
  result = fcinfo_fontformats(filter);

  FcPatternDestroy(filter);
  return result;
}

FcFontSet *fontformat_family_index(const FcChar8 *format, config_t config)
{
  FcFontSet *result;
  FcPattern *filter;

  FcInit();
  filter = fcinfo_name_parse((FcChar8 *) config.pattern_string);
  result = fcinfo_fontformat_family_index(format, filter);

  FcPatternDestroy(filter);
  return result;
}

FcFontSet *uinterval_statistics(config_t config,
                                double *uinterval_stat[],
                                uinterval_type_t uintype)
{
  FcFontSet *result;
  FcPattern *filter;

  FcInit();
  filter = fcinfo_name_parse((FcChar8 *) config.pattern_string);
  result = fcinfo_uinterval_statistics(filter, uinterval_stat, uintype);

  return result;
}

char *fontinfo_version(char *string, int maxlen)
{
  snprintf(string, maxlen, "%d", FONTINFO_VERSION);
  return string;
}

char *fontconfig_version(char *string, int maxlen)
{
  int maj, min, rev;
  fc_version(&maj, &min, &rev);
  snprintf(string, maxlen, "%d.%d.%d", maj, min, rev);
  return string;
}

void timestamp(char *dest, ts_output_t o)
{
  time_t rawtime;
  struct tm * timeinfo;
  
  time (&rawtime);
  timeinfo = localtime (&rawtime);

  switch (o)
  {
    case TS_YEAR:
     sprintf(dest, "%4d", 1900 + timeinfo->tm_year);
     break;

    case TS_DATE:
     sprintf(dest, "%4d-%02d-%02d", 
             1900 + timeinfo->tm_year, 1 + timeinfo->tm_mon, timeinfo->tm_mday);
     break;

    default:
     break;
  }

  return;
}

int specimen_sentence(config_t config, FcCharSet *charset,
                      const char *script, int *dir, 
                      const char **lang, int *random,
                      FcChar32 *ucs4str, int maxlen)
{
  int n;

  int s, nsentences, charsfromscript;
  sentence_t *sentences;
  int missing; /* first missing character index */

  if (config.specimen_sentence)
  {
    n = fcinfo_utf8toucs4((FcChar8 *)config.specimen_sentence, 
                          ucs4str, maxlen - 1);

    if ((missing = fcinfo_charset_coveres_sentence(charset, ucs4str, n, NULL, NULL)) < n)
    {
      if (config.debug)
        fprintf(stdout, "missing #%d 0x%04x: ", missing, ucs4str[missing]);

      n = fcinfo_charset_generate_sentence(charset, script, ucs4str, maxlen - 1);
      *random = 1;
      if (n < SCRIPT_SENTENCE_LEN_MIN)  
      { /* there's nearly nothing in charset defined by script */
        n = fcinfo_charset_generate_sentence(charset, NULL, ucs4str, maxlen - 1);
        /* look for some chars in font's universe */
        *random = 2;
      }

      ucs4str[n] = 0;
      *dir = 0;
      *lang = NO_LANG;
      return n;
    }

    if (config.debug)
      fprintf(stdout, "using '%s' (%s) sentence\n", 
              config.specimen_sentence, config.specimen_lang);

    ucs4str[n] = 0;
    *dir = config.specimen_textdir;
    *lang = config.specimen_lang;
    *random = 0;
    return n;
  }

  if (script && unicode_script_exists(script))
  {
    unicode_script_sentences(script, &nsentences, &sentences, dir);
  }
  else /* no or unknown script */
  {
    n = fcinfo_charset_generate_sentence(charset, NULL, ucs4str, maxlen - 1);
    ucs4str[n] = 0;

    if (config.debug)
    {
      for (s = 0; s < n; s++)
        fprintf(stdout, "0x%x ", ucs4str[s]);
      fprintf(stdout, "\n");
    }

    *dir = 0;
    *lang = NO_LANG;
    *random = 2;
    return n;
  }

  for (s = 0; s < nsentences; s++)
  {
    n = fcinfo_utf8toucs4((FcChar8 *)sentences[s].sent, ucs4str, maxlen - 1);
    if ((missing = fcinfo_charset_coveres_sentence(charset, ucs4str, n, 
                                                    script, &charsfromscript)) == n)
    {
      /* sentence must have at least one character from requested script */
      /* otherwise it is probably sentence from other script */
      assert(charsfromscript > 0);
      if (config.debug)
        fprintf(stdout, "using '%s' (%s) sentence\n", 
                sentences[s].sent, sentences[s].lang);
      ucs4str[n] = 0;
      *lang = sentences[s].lang;
      /* dir set in unicode_script_sentences() */
      *random = 0;
      return n;
    } 
    else if (config.debug)
      fprintf(stdout, "missing #%d 0x%04x: ", missing, ucs4str[missing]);
  }

  n = fcinfo_charset_generate_sentence(charset, script, ucs4str, maxlen - 1);
  *random = 1;
  if (n < SCRIPT_SENTENCE_LEN_MIN) 
  { /* there's nearly nothing in charset defined by script */
    n = fcinfo_charset_generate_sentence(charset, NULL, ucs4str, maxlen - 1);
    if (config.debug)
      fprintf(stdout, "using random characters from charset: ");
    /* look for some chars in font's universe */
    *random = 2;
  }
  else if (config.debug)
    fprintf(stdout, "using random characters from script: ");

  if (config.debug)
  {
    for (s = 0; s < n; s++)
      fprintf(stdout, "0x%x ", ucs4str[s]);
    fprintf(stdout, "\n");
  }

  ucs4str[n] = 0;

  *dir = 0;
  *lang = NO_LANG;
  return n;
}

void underscores_to_spaces(char *str)
{
  char *p = str;
  while (*p)
  {
    if (*p == '_')
      *p = ' ';
    p++;
  }
  return;
}

const char **thanks_html_text(void)
{
  return thanks_html;
}

