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

#define _GNU_SOURCE /* qsort_r */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>

#include "fcinfo.h"

#define YES                 "yes"
#define NO                  "no"

#define TITLE_MAX           256

#define LANGUAGES_INDEX     "Languages"
#define FAMILIES_INDEX      "Font Families"
#define FAMILY_OVERVIEW     "Family Overview"

#define LANGUAGES_SUBDIR    "languages"
#define FONTS_SUBDIR        "fonts"
#define FAMILIES_SUBDIR     "families"

#define SIZE_MEAN           16.0

static int chars_compare(char c1, char c2)
{
  /* alpha first */

  if (isalpha(c1) && !isalpha(c2))
    return -1;
 
  if (!isalpha(c1) && isalpha(c2))
    return 1;

  /* otherwise usual ordering */

  return toupper(c1) - toupper(c2);
}

/* compare two strings; a-z and A-Z letters are preffered 
   over anything else, i. e.  'Adobe Courier' should go
   sooner than '!Y2KBUG', '1980 Portable', etc. */
static int strings_compare(const char *s1, const char *s2)
{
  int c = 0, r;
  while (1)
  {
    if (s1[c] == '\0' && s2[c] == '\0')
      return 0;

    if (s1[c] == '\0' && s2[c] != '\0')
      return -1;

    if (s1[c] != '\0' && s2[c] == '\0')
      return 1;

    if ((r = chars_compare(s1[c], s2[c])))
      return r;

    c++;
  }
}

static int pattern_compare (const void *arg1, const void *arg2, void *arg3)
{
  const FcPattern *p1 = *(FcPattern * const *)arg1;
  const FcPattern *p2 = *(FcPattern * const *)arg2;
  const char **el = (const char **)arg3;
  FcChar8 *string1, *string2;
  int int1, int2;
  FcBool bool1, bool2;
  FcLangSet *langset1, *langset2;
  double double1, double2;
  int ret = 0;

  while (*el)
  {
    /* string */
    if (!strcmp(*el, FC_FAMILY) || !strcmp(*el, FC_STYLE) || !strcmp(*el, FC_FONTFORMAT))
    {
      if (FcPatternGetString (p1, *el, 0, &string1) == FcResultMatch &&
	  FcPatternGetString (p2, *el, 0, &string2) == FcResultMatch) 
        ret = strings_compare((char *)string1, (char *)string2);
    }
    /* integer */
    else if (!strcmp(*el, FC_WEIGHT) || !strcmp(*el, FC_WIDTH) ||
	     !strcmp(*el, FC_SLANT) || !strcmp(*el, FC_SPACING))
    {
      if (FcPatternGetInteger(p1, *el, 0, &int1) == FcResultMatch && 
	  FcPatternGetInteger(p2, *el, 0, &int2) == FcResultMatch)
	ret = int2 - int1;
    }
    /* bool */
    else if (!strcmp(*el, FC_SCALABLE) || !strcmp(*el, FC_DECORATIVE))
    {
      if (FcPatternGetBool(p1, *el, 0, &bool1) == FcResultMatch &&
	  FcPatternGetBool(p1, *el, 0, &bool2) == FcResultMatch)
       ret = bool2 - bool1;
    }
    /* double */
    else if (!strcmp(*el, FC_PIXEL_SIZE))
    {
      if (FcPatternGetDouble(p1, *el, 0, &double1) == FcResultMatch &&
	  FcPatternGetDouble(p2, *el, 0, &double2) == FcResultMatch)
	ret = fabs(double1 - SIZE_MEAN) - fabs(double2 - SIZE_MEAN);
    }
    /* lang set */
    else if (!strcmp(*el, FC_LANG))
    {
      /* to choose size of bitmap font which supports widest range of langs */
      /* maybe charset would be better? */
      if (FcPatternGetLangSet(p1, *el, 0, &langset1) == FcResultMatch &&
	  FcPatternGetLangSet(p2, *el, 0, &langset2) == FcResultMatch)
      {
        ret = 0;
	if (FcLangSetContains(langset1, langset2))
	  ret = -1;
	if (FcLangSetContains(langset2, langset1))
        {
          if (ret == -1)
            ret = 0; /* langset1 <= langset2 & langset1 >= langset2 */
          else
    	    ret = 1;
        }
      }
    }

    if (ret)
      break;

    el++;
  }

  return ret;
}

static FcBool empty_charset(FcPattern *pattern)
{
  FcCharSet *charset;

  assert(FcPatternGetCharSet(pattern, FC_CHARSET, 0, &charset) 
         == FcResultMatch);
  
  return FcCharSetCount(charset) == 0;
}

static void font_set_sort(const FcFontSet *fontset, const char **elements)
{
  qsort_r(fontset->fonts, fontset->nfont, sizeof(FcPattern *), 
          pattern_compare, elements);
}

FcFontSet * fcinfo(const FcConfig *config, const FcPattern *pattern, 
                   FcBool remove_duplicities, int argnum, ...)
{
  va_list va;
  const char *elements[argnum + 1];
  int a;

  FcFontSet *fontset;
  FcObjectSet *objectset;

  FcInit();
  objectset = FcObjectSetCreate();

  va_start(va, argnum);
  for (a = 0; a < argnum; a++)
  {
    elements[a] = va_arg(va, const char *);
    FcObjectSetAdd(objectset, elements[a]);
  }
  va_end(va);

  fontset = FcFontList((FcConfig *)config, (FcPattern *)pattern, objectset);
  elements[argnum] = NULL;
  font_set_sort(fontset, elements);

  FcObjectSetDestroy(objectset);

  if (remove_duplicities)
  {
    FcFontSet *result = FcFontSetCreate();
    int f;
    FcPattern *added = NULL;

    /* fontlist is linearly ordered */
    f = 0;
    while (f < fontset->nfont)
    {
      FcFontSetAdd(result, FcPatternDuplicate(fontset->fonts[f]));
      added = fontset->fonts[f++];
      while (f < fontset->nfont &&
             !pattern_compare(&fontset->fonts[f], &added, elements))
        f++;
    }

    FcFontSetDestroy(fontset);
    return result;
  }

  return fontset;
}

FcBool pattern_contains_size(FcPattern *pat, int size)
{
  double s;
  int i;

  i = 0;
  while (FcPatternGetDouble(pat, FC_PIXEL_SIZE, i++, &s) == FcResultMatch)
    if ((int)s == size)
      return FcTrue;

  return FcFalse;
}

/* for given fontlist fill in empty fields of pattern according to system 
   configuration. When duplicite (family, style) pair exists in the given
   list, it merges them together - size and file elements. */

/* file is needed for identifying the best font in FC_LANG terms */

/* assuming fontset is sorted by family, style, size, charset */

FcFontSet * fcinfo_match(FcFontSet *fontset, const FcPattern *filter)
{
  int f;

  FcFontSet *result;

  FcChar8 *family, *style, *file;
  FcChar8 *next_family, *next_style;

  double size;

  FcBool scalable;
  FcPattern *pat, *match = NULL;
  FcResult r;

  FcInit();

  if (fontset->nfont == 0)
  {
    result = FcFontSetCreate();
    return result;
  }

  /* fc-match "Misc Fixed:style=Bold" gives many files with 
     various pixelsizes plus two "encodings" ('general' and 
     'ISO8859-1', ISO8859-1 is subset of general) 

     we want only one card for family,style pair

     for MiscFixedBold.html we will take informations from
     file 9x18B.pcf.gz (biggest size and general encoding)

     from fcinfo(), fontset is sorted a way that pattern
     created from 9x18B.pcf.gz comes first in 
     "Misc Fixed:style=Bold" interval (the biggest pixel size
     and subset of supported languages, see pattern_compare()). */

  result = FcFontSetCreate();
  for (f = 0; f < fontset->nfont; )
  {
    /* bitmap fonts: */
    /* we are here always on the best file in FC_LANG terms */
    /* we need it for displaying the widest range of supported languages */
    /* identify it by FC_FILE for FcFontMatch() */
    if (filter)
      pat = FcPatternDuplicate(filter);
    else
      pat = FcPatternCreate();
    assert(fcinfo_get_translated_string(fontset->fonts[f], FC_FAMILY, 
                                        LANG_EN, &family) == FcResultMatch);
    FcPatternAddString(pat, FC_FAMILY, family);
    if (fcinfo_get_translated_string(fontset->fonts[f], FC_STYLE, 
                                     LANG_EN, &style) == FcResultMatch)
      FcPatternAddString(pat, FC_STYLE, style);
    if (FcPatternGetString(fontset->fonts[f], FC_FILE, 0, &file)
        == FcResultMatch)
      FcPatternAddString(pat, FC_FILE, file);

    FcConfigSubstitute(NULL, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);
    match = FcFontMatch(NULL, pat, &r);
    assert(r == FcResultMatch);
    /* should not be needed: FcConfigSubstitute(NULL, match, FcMatchFont); */
    FcPatternDestroy(pat);

    assert(FcPatternGetBool(match, FC_SCALABLE, 0, &scalable)  
           == FcResultMatch);

/*    fprintf(stdout, "Family: %s Style: %s\n", next_family, next_style);
    fprintf(stdout, "        %s\n", file);*/

    /* figure out sizes font provide (needed for specimen) and skip same */
    /* family,style pairs (see e. g. Misc Fixed) */
    if (! scalable)
    {
      next_family = family;
      next_style  = style;
      /* unfortunately e. g. 'ETL Fixed' and 'ETL fixed' 
         are present on my system */

      while (FcStrCmpIgnoreCase(next_family, family) == 0 &&
             FcStrCmpIgnoreCase(next_style, style) == 0)
      {
        if (FcPatternGetDouble(fontset->fonts[f], FC_PIXEL_SIZE, 0, &size)
                 == FcResultMatch)
        {
          if (! pattern_contains_size(match, size))
          {
            FcPatternAddDouble(match, FC_PIXEL_SIZE, size);
            if (FcPatternGetString(fontset->fonts[f], FC_FILE, 0, &file)
                     == FcResultMatch)
              FcPatternAddString(match, FC_FILE, file);
            /*fprintf(stdout, "       %s\n", file);*/
          }
        }

        if (f + 1 == fontset->nfont)  /* last family,style pair at all */
        {
          f++;
          break;
        }

        assert(fcinfo_get_translated_string(fontset->fonts[f + 1], FC_FAMILY, 
                                            LANG_EN, &next_family)
               == FcResultMatch);
        assert(fcinfo_get_translated_string(fontset->fonts[f + 1], FC_STYLE,
                                            LANG_EN, &next_style)
               == FcResultMatch);
        f++;
      }
    }
    else /* family Efont Serif Regular is spread (unintentionally) over */
    {    /* 5 files (Efont Serif <X> should be spread over 2 files,  */
         /* normal and 'Alternate'), so this is needed anyway: */
      FcPatternDel(match, FC_FILE);
      next_family = family;
      next_style  = style;

      while (FcStrCmpIgnoreCase(next_family, family) == 0 &&
             FcStrCmpIgnoreCase(next_style, style) == 0)
      {
        if (FcPatternGetString(fontset->fonts[f], FC_FILE, 0, &file)
                == FcResultMatch)
          FcPatternAddString(match, FC_FILE, file);

        if (f + 1 == fontset->nfont)  /* last family,style pair at all */
        {
          f++;
          break;
        }

        assert(fcinfo_get_translated_string(fontset->fonts[f + 1], FC_FAMILY, 
                                            LANG_EN, &next_family)
               == FcResultMatch);
        assert(fcinfo_get_translated_string(fontset->fonts[f + 1], FC_STYLE,
                                            LANG_EN, &next_style)
               == FcResultMatch);

        f++;
      }
    }

    /* don't add ethio16f-uni.pcf, johabg16.pcf and similar with reported 
       empty charset */
    if (! empty_charset(match))
      FcFontSetAdd(result, FcPatternDuplicate(match));
  }

  if (match)
    FcPatternDestroy(match);
  return result;
}

int fcinfo_utf8toucs4(const FcChar8 *utf8str, FcChar32 *ucs4str, int max_len)
{
  const FcChar8 *s = utf8str;
  int pos;
  int n;
  const int len = strlen((const char *)utf8str); 

  pos = n = 0;
  while (pos < len && n < max_len)
  {
    pos += FcUtf8ToUcs4(&s[pos], &ucs4str[n], 
                        len - pos);
    n++;
  }

  return n;
}


int fcinfo_charset_coveres_sentence(FcCharSet *charset, 
                                    FcChar32 *sentence,
                                    int len,
                                    const char *script,
                                    int *charsfromscript)
{
  int c;

  if (script)
    *charsfromscript = 0;

  for (c = 0; c < len; c++)
  {
    if (! FcCharSetHasChar(charset, sentence[c]))
      return c;

    if (script && unicode_interval_contains(script, SCRIPT, sentence[c]))
      (*charsfromscript)++;
  }
 
  return c;
}

/* character set: grid FcTrue, specimen: grid FcFalse */
/* grid FcTrue: keep blanks + add ' ' where character */
/* doesn't exist; of course leave out lines (maps) where */
/* no character exists */
FcChar32 fcinfo_chars(FcCharSet *charset, FcChar32 **chars, 
                      const char *uinterval, uinterval_type_t uintype,
                      FcBool grid, FcChar32 maxchars)
{
  FcChar32 available, nchars, n;
  FcBlanks *blanks = FcConfigGetBlanks(NULL);
  FcChar32 ucs4;
  FcChar32 map[FC_CHARSET_MAP_SIZE];
  FcChar32 next;

  int i, j;
  int nlines;

  nlines = 0;
  for (ucs4 = FcCharSetFirstPage (charset, map, &next);
       ucs4 != FC_CHARSET_DONE;
       ucs4 = FcCharSetNextPage (charset, map, &next))
  {
    int i;
    /* for ucs4 == 0, skip i == 0 and i == 4, see below */
    for (i = (ucs4 == 0 ? 1 : 0); 
         i < FC_CHARSET_MAP_SIZE; 
         ucs4 == 0 && i == 3 ? i = 5 : i++)
      if (map[i] && 
          (!uinterval || 
           unicode_interval_contains(uinterval, uintype, ucs4 + 32*i) ||
           unicode_interval_contains(uinterval, uintype, ucs4 + 32*i + 0x10)))
        nlines++;
  }

  if (grid)
    available = 32*nlines;
  else
    available = FcCharSetCount(charset);

  nchars = available < maxchars ? available : maxchars;
  *chars = (FcChar32*)malloc(nchars*sizeof(FcChar32));

  if (! *chars)
  {
    fprintf(stderr, "fcinfo_chars(): can't allocate memory\n");
    exit(1);
  }

  /* for() code below borrowed from fontconfig-2.10.2/src/fclang.c */
  n = 0;
  for (ucs4 = FcCharSetFirstPage (charset, map, &next);
       ucs4 != FC_CHARSET_DONE;
       ucs4 = FcCharSetNextPage (charset, map, &next))
  {
    /* for some fonts (e. g. Dina) fontconfig reports they contain 
       control characters;  skip them for sure (we get 'wrong unicode 
       character' while converting svg if not) */
    for (i = (ucs4 == 0 ? 1 : 0); 
         i < FC_CHARSET_MAP_SIZE; 
         ucs4 == 0 && i == 3 ? i = 5 : i++)
      if (map[i] && 
          (!uinterval || 
           unicode_interval_contains(uinterval, uintype, ucs4 + 32*i) ||
           unicode_interval_contains(uinterval, uintype, ucs4 + 32*i + 0x10)))
      {
         for (j = 0; j < 32; j++)
         {
           if (map[i] & (1 << j))
           {
             FcChar32 ch = ucs4 + 32*i + j;

             if (grid == FcTrue || !FcBlanksIsMember(blanks, ch))
               (*chars)[n++] = ch;
             
           }
           else if (grid == FcTrue)
             (*chars)[n++] = NO_CHAR;

           if (n == nchars)
             return n;
         }
      }
  }

  return n;
}

/* take first nchars characters from charset */
/* if script not NULL, then restrict to script interval only */
int fcinfo_charset_generate_sentence(FcCharSet *charset, 
                                     const char *script,
                                     FcChar32 *sentence, 
                                     int maxchars)
{
  FcChar32 *chars;
  FcChar32 i, available;

  available = fcinfo_chars(charset, &chars, script, SCRIPT, FcFalse, maxchars);

  for (i = 0; i < available && i < maxchars; i++)
    sentence[i] = chars[i];

  free(chars);
  return i;
}

FcResult fcinfo_get_translated_string(const FcPattern *pattern, 
                                      const char *object, 
                                      const FcChar8 *lang, FcChar8 **s)
{
  FcChar8 *l;
  int n;
  FcResult r;
  const char *lang_object;

  if (strcmp(object, FC_FAMILY) == 0)
    lang_object = FC_FAMILYLANG;
  else if (strcmp(object, FC_STYLE) == 0)
    lang_object = FC_STYLELANG;
  else if (strcmp(object, FC_FULLNAME))
    lang_object = FC_FULLNAMELANG;
  else
  {
    fprintf(stderr, "fcinfo_get_translated_string():"
                    " no translatable string\n");
    exit (1);
  }

  if (FcPatternGetString(pattern, lang_object, 0, &l) != FcResultMatch)
  {
    /* no *_LANG element */
    return FcPatternGetString(pattern, object, 0, s);
  }

  n = 0;
  while ((r = FcPatternGetString(pattern, lang_object, n, &l)) == FcResultMatch)
    if (FcStrCmp(l, lang) == 0)
      break;
    else
      n++;
    
  if (r != FcResultMatch)
    n = 0; /* no lang found */
  
  return FcPatternGetString(pattern, object, n, s);
}

void fc_version(int *maj, int *min, int *rev)
{
  *maj = FC_MAJOR;
  *min = FC_MINOR;
  *rev = FC_REVISION;
  return;
}

FcFontSet *fcinfo_families_index(const FcPattern *filter)
{
  int unused_int, f, nfamilies;

  FcPattern *pattern, *match;
  FcFontSet *fontset, *tmp, 
            *families;
  FcResult r;

  FcChar8 *family, *next_family, *normal_style, *style;

  FcInit();
  pattern = FcPatternDuplicate(filter);
  fontset = fcinfo(NULL, filter, FcFalse, 5,
                   FC_FAMILY,
                   FC_STYLE,
                   FC_LANG,/* fonts like Misc Fixed need 
                              to be sorted by LANG (subset-like) */
                   FC_PIXEL_SIZE,
                   FC_FILE); /* for identifying the best font in 
                                FC_LANG terms */
  if (fontset->nfont == 0)
    return fontset;

  tmp = fcinfo_match(fontset, pattern);
  FcFontSetDestroy(fontset);
  fontset = tmp;

  if (FcPatternGetInteger(pattern, FC_WEIGHT, 0, &unused_int) != FcResultMatch)
    FcPatternAddInteger(pattern, FC_WEIGHT, FC_WEIGHT_REGULAR);
  if (FcPatternGetInteger(pattern, FC_SLANT, 0, &unused_int) != FcResultMatch)
    FcPatternAddInteger(pattern, FC_SLANT, FC_SLANT_ROMAN);
  if (FcPatternGetInteger(pattern, FC_WIDTH, 0, &unused_int) != FcResultMatch)
    FcPatternAddInteger(pattern, FC_WIDTH, FC_WIDTH_NORMAL);

  /* choose 'normal' styles according to pattern */
  families = FcFontSetCreate();
  nfamilies = 0;
  for (f = 0; f < fontset->nfont; )
  {
    assert(fcinfo_get_translated_string(fontset->fonts[f], FC_FAMILY, 
                                        LANG_EN, &family)
           == FcResultMatch);

    FcPatternDel(pattern, FC_FAMILY);
    FcPatternAddString(pattern, FC_FAMILY, family);
    match = FcFontSetMatch(NULL, &fontset, 1, pattern, &r);
    assert(fcinfo_get_translated_string(match, FC_STYLE, LANG_EN, &normal_style)
            == FcResultMatch);
    do
    {
      assert(fcinfo_get_translated_string(fontset->fonts[f], FC_STYLE, 
                                          LANG_EN, &style)
             == FcResultMatch);
      
      if (FcStrCmpIgnoreCase(style, normal_style) == 0)
        FcFontSetAdd(families, FcPatternDuplicate(fontset->fonts[f]));

      if (++f == fontset->nfont)
        break;

      assert(fcinfo_get_translated_string(fontset->fonts[f], FC_FAMILY, 
                                          LANG_EN, &next_family)
             == FcResultMatch);
    }
    while (FcStrCmpIgnoreCase(family, next_family) == 0);
    nfamilies++;
  }

  FcPatternDestroy(pattern);
  FcFontSetDestroy(fontset);

  return families;
}

static void family_info(const FcPattern *pattern, family_info_t *fi)
{
  FcChar8 *str;

  fi->fontformat[0] = '\0';
  fi->capability[0] = '\0';
  fi->foundry[0] = '\0';
  
  if (FcPatternGetString(pattern, FC_FONTFORMAT, 0, &str) == FcResultMatch)
    snprintf(fi->fontformat, FONTFORMAT_MAX, "%s", str);


  if (FcPatternGetString(pattern, FC_CAPABILITY, 0, &str) == FcResultMatch)
    snprintf(fi->capability, CAPABILITY_MAX, "%s", str);

  if (FcPatternGetString(pattern, FC_FOUNDRY, 0, &str) == FcResultMatch)
    if (strcmp((char *)str, "unknown") != 0)
      snprintf(fi->foundry, FOUNDRY_MAX, "%s", str);

  return;
}

FcFontSet *fcinfo_styles_index(const FcChar8 *family, const FcPattern *filter,
                               family_info_t *fi)
{
  FcFontSet *fontset, *result;
  FcPattern *pattern;

  pattern = FcPatternDuplicate(filter);
  FcPatternDel(pattern, FC_FAMILY); /* perhaps shouldn't be needed */
  FcPatternAddString(pattern, FC_FAMILY, family);
  fontset = fcinfo(NULL, pattern, FcFalse, 5,
                   FC_FAMILY,
                   FC_STYLE,
                   FC_LANG,/* fonts like Misc Fixed need 
                              to be sorted by LANG (subset-like) */
                   FC_PIXEL_SIZE,
                   FC_FILE); /* for identifying the best font in 
                                FC_LANG terms */
  FcPatternDestroy(pattern);

  if (fontset->nfont == 0)
    return fontset;

  result = fcinfo_match(fontset, filter);
  /* assuming family_info entries all same
     for each style of family; might not be true always */
  if (fi)
    family_info(result->fonts[0], fi);

  FcFontSetDestroy(fontset);
  return result;
}

FcFontSet *fcinfo_font_index(const FcPattern *filter)
{
  FcFontSet *result, *fontset;

  FcInit();
  fontset = fcinfo(NULL, filter, FcFalse, 5,
                   FC_FAMILY,
                   FC_STYLE,
                   FC_LANG,/* fonts like Misc Fixed need 
                              to be sorted by LANG (subset-like) */
                   FC_PIXEL_SIZE,
                   FC_FILE); /* for identifying the best font in 
                                FC_LANG terms */

  result = fcinfo_match(fontset, NULL);

  FcFontSetDestroy(fontset);
  return result;
}

FcStrSet *fcinfo_languages(const FcPattern *filter)
{
  FcObjectSet *objectset;
  FcFontSet *fontset;
  FcLangSet *union_langset;
  FcStrSet *result;
  int f;

  FcInit();
  objectset = FcObjectSetBuild(FC_LANG, NULL);
  fontset = FcFontList(NULL, (FcPattern *)filter, objectset);
  FcObjectSetDestroy(objectset);

  union_langset = FcLangSetCreate();
  for (f = 0; f < fontset->nfont; f++)
  {
    FcLangSet *langset;
    FcStrList *strlist;
    FcChar8 *lang;

    if (FcPatternGetLangSet(fontset->fonts[f], FC_LANG, 0, &langset)
        != FcResultMatch)
      continue;
    if (! langset)
      continue;

    strlist = FcStrListCreate(FcLangSetGetLangs(langset));
    while ((lang = FcStrListNext(strlist)))
      if (FcLangSetHasLang(union_langset, lang))
       FcLangSetAdd(union_langset, FcStrCopy(lang));
  }

  result = FcLangSetGetLangs(union_langset);
  
  FcFontSetDestroy(fontset);
  FcLangSetDestroy(union_langset);
  return result;
}

FcFontSet *fcinfo_language_font_index(const FcChar8 *lang, 
                                      const FcPattern *filter)
{
  FcFontSet *fontset;
  FcPattern *pattern;
  FcLangSet *langset;

  FcInit();
  pattern = FcPatternDuplicate(filter);
  langset = FcLangSetCreate();
  FcLangSetAdd(langset, lang);
  FcPatternAddLangSet(pattern, FC_LANG, langset);
  fontset = fcinfo(NULL, pattern, FcFalse, 2, FC_FAMILY, FC_STYLE);

  /*result = fcinfo_match(fontset, filter, FcFalse);  this is redundant 
    right now */
  /* but language index could want to display spcimen or so in the future 
     be sure to include also size, charset and file FcObjects when        
     turning this on: see above call of fcinfo()                          */
  /* we don't need to remove  ethio16f-uni.pcf, johabg16.pcf and similar
     as they have reported empty charset, so they do not cover any language */

  FcPatternDestroy(pattern);
  FcLangSetDestroy(langset);
  return fontset;
}

FcStrSet *fcinfo_fontformats(const FcPattern *filter)
{
  FcFontSet *fontset;
  FcStrSet *result;
  FcChar8 *format;
  int f;

  FcInit();
  fontset = fcinfo(NULL, filter, FcFalse, 1, FC_FONTFORMAT);

  result = FcStrSetCreate();
  for (f = 0; f < fontset->nfont; f++)
  {
    assert(FcPatternGetString(fontset->fonts[f], FC_FONTFORMAT, 0, &format)
             == FcResultMatch);
    FcStrSetAdd(result, format);
  }

  return result;
}

FcFontSet *fcinfo_fontformat_family_index(const FcChar8 *format,
                                          const FcPattern *filter)
{
  FcFontSet *fontset, *result;
  FcPattern *pattern;
  FcPattern *font;
  FcResult r;
  int f;

  pattern = FcPatternDuplicate(filter);
  FcPatternAddString(pattern, FC_FONTFORMAT, format);

  fontset = fcinfo(NULL, pattern, FcTrue, 1, FC_FAMILY);

  result = FcFontSetCreate();
  for (f = 0; f < fontset->nfont; f++)
  {
    font = FcFontMatch(NULL, fontset->fonts[f], &r);
    assert(r == FcResultMatch);
    /* don't add ethio16f-uni.pcf, johabg16.pcf and similar with reported 
       empty charset */
    /* could be done with fcinfo_match(), but that is superfluous
       there like for fcinfo_language_font_index() - it will be needed
       when we will need to display some kind of specimen in fontformat 
       index */
    if (!empty_charset(font))
      FcFontSetAdd(result, FcPatternDuplicate(fontset->fonts[f]));
    FcPatternDestroy(font);
  }

  FcPatternDestroy(pattern);
  FcFontSetDestroy(fontset);
  return result;
}

FcFontSet *fcinfo_uinterval_statistics(const FcPattern *filter, 
                                       double *uinterval_stat[],
                                       uinterval_type_t uintype)
{
  FcFontSet *fontset, *tmp;
  FcCharSet *charset;
  int s, f;
  FcChar8 *file;

  FcInit(); /* bitmap font files sort according to FC_LANG, identified by FC_FILE */
  fontset = fcinfo(NULL, filter, FcFalse, 5, FC_FAMILY, FC_STYLE, FC_CHARSET, FC_LANG, FC_FILE);

  tmp = fcinfo_match(fontset, filter); /* wipe duplicite (family, style)s */
  FcFontSetDestroy(fontset);
  fontset = tmp;

  for (s = 0; s < uinterval_num(uintype); s++)
  {
    uinterval_stat[s] = malloc(sizeof(double)*fontset->nfont);
    if (! uinterval_stat[s])
    {
      fprintf(stderr, "fcinfo_uinterval_statistics(): out of memory\n");
      exit(1);
    }
  }

  for (f = 0; f < fontset->nfont; f++)
  {
    FcPatternGetString(fontset->fonts[f], FC_FILE, 0, &file);
    assert(FcPatternGetCharSet(fontset->fonts[f], FC_CHARSET, 0, &charset)
           == FcResultMatch);
    for (s = 0; s < uinterval_num(uintype); s++)
      uinterval_stat[s][f] = 
        charset_uinterval_coverage(charset, 
                                   uinterval_name(s, uintype), 
                                   uintype,
                                   NULL, NULL);
  }

  return fontset; /* holds one of table headers */
}

FcPattern *fcinfo_name_parse(const FcChar8 *pattern)
{
  FcChar8 sanitized_pattern[2*strlen((char *)pattern)];
  int c1, c2;

  c1 = c2 = 0;
  while (pattern[c1])
  {
    if (pattern[c1] == '-') /* escape dash */
      sanitized_pattern[c2++] = '\\';
    sanitized_pattern[c2++] = pattern[c1++];
  }
  sanitized_pattern[c2] = '\0';
  //fprintf(stdout, "name: %s\n", (char *)sanitized_pattern);

  return FcNameParse(sanitized_pattern);
}

FcPattern *fcinfo_get_font(const FcChar8 *request)
{
  FcPattern *pattern, *match;
  FcResult r;
  FcChar8 *string;
  int integer;
  double double_num;

  pattern = fcinfo_name_parse((FcChar8 *) request);
  if (FcPatternGetString(pattern, FC_FAMILY, 0, &string)
      != FcResultMatch)
    FcPatternAddString(pattern, FC_FAMILY, (FcChar8 *)"sans-serif");
  if (FcPatternGetDouble(pattern, FC_PIXEL_SIZE, 0, &double_num)
      != FcResultMatch)
    FcPatternAddDouble(pattern, FC_PIXEL_SIZE, 12.0);
  if (FcPatternGetInteger(pattern, FC_WEIGHT, 0, &integer)
      != FcResultMatch)
    FcPatternAddInteger(pattern, FC_WEIGHT, FC_WEIGHT_REGULAR);
  if (FcPatternGetInteger(pattern, FC_SLANT, 0, &integer)
      != FcResultMatch)
    FcPatternAddInteger(pattern, FC_SLANT, FC_SLANT_ROMAN);
  if (FcPatternGetInteger(pattern, FC_WIDTH, 0, &integer)
      != FcResultMatch)
    FcPatternAddInteger(pattern, FC_WIDTH, FC_WIDTH_NORMAL);

  FcConfigSubstitute(NULL, pattern, FcMatchPattern);
  FcDefaultSubstitute(pattern);
  match = FcFontMatch(0, pattern, &r);
  assert(r == FcResultMatch);

  FcPatternDestroy(pattern);
  return match;
}

