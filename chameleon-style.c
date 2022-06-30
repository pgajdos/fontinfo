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
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <assert.h>

#include <fontconfig/fontconfig.h>

#include "fcinfo.h"
#include "constants.h"
#include "filesystem.h"
#include "img.h"
#include "img_svg.h"
#include "styles-commons.h"
#include "package-manager.h"
#include "ft.h"
#include "js-lib.h"
#include "ymp.h"

#define SOR                "https://static.opensuse.org/"
#define CHAMELEON_LOCATION SOR"chameleon-3.0/"

#define YES                 "yes"
#define NO                  "no"

#define TITLE_MAX                256
#define FONT_INDEX_PARTS_MAX      64
#define VERSION_MAX               16

/* limit for specimen sentence */
#define SENTENCE_NCHARS      70

/* output maximum SCRIPTS_MAX scripts in font card */
# define SCRIPTS_MAX           32
/* output scripts that has at least SCRIPTS_COVERAGE_MIN % coverage by
   given font in font card; this is a soft limit: if no script have
   more than SCRIPTS_COVERAGE_MIN, at least one will be displayed; ... */
# define SCRIPTS_COVERAGE_MIN  5.0
/* ... but at the same time scripts with coverage below 
   SCRIPTS_COVERAGE_HARD_MIN will be concealed, e. g. Latin in STIXVariants */
# define SCRIPTS_COVERAGE_HARD_MIN 1.0

# define BLOCKS_COVERAGE_MIN   5.0

#define LANGUAGES_INDEX         "Supported Languages"
#define LANGUAGES_INDEX_DESC    "Covered languages"
#define FAMILIES_INDEX          "Font Families"
#define FAMILIES_INDEX_DESC     "Index of all font families"
#define FONTFORMATS_INDEX       "Families by Font Formats"
#define FONTFORMATS_INDEX_DESC  "Families Sorted by Font Format"
#define SCRIPTS_INDEX           "Families by Scripts"
#define SCRIPTS_INDEX_DESC      "Fonts Sorted by Script Coverage"
#define BLOCKS_INDEX            "Fonts by Unicode Blocks"
#define BLOCKS_INDEX_DESC       "Fonts Sorted by Unicode Block Coverage"
#define FAMILY_OVERVIEW         "Family Overview"
#define FAMILY_OVERVIEW_DESC    "Meta informations and styles of family"
#define FAMILY_STYLES           "Available Styles"
#define FAMILY_FONT_FORMAT      "Font Format"
#define FAMILY_FONT_CAPABILITY  "Capability"
#define FAMILY_FONT_FOUNDRY     "Foundry"
#define FONT_CARD               "Font Card"
#define FONT_CARD_DESC          "Overview of a style of given family"
#define FONT_CARD_SHAPE         "Shape and Spacing"
#define FONT_CARD_RENDERING     "Rendering Algorithms"
#define FONT_CARD_LANGUAGES     "Supported Languages"
#define FONT_CARD_CHARSET       "Character Set"
#define NONE_BITMAP_STRIKES     "none"
#define SPECIMEN                "Specimen"
#define SOFTWARE_PACKAGE        "Software Package Information"
#define LINK_ALL                "all"
#define TITLE_DETAILED_LETTER   "Show mini-specimens for families begining on this letter"
#define TITLE_DETAILED_ALL      "Show mini-specimens for all families (huge page)"
#define THANKS                  "Thanks"
#define THANKS_DESC             "People which land their hand"

#define COLLECTION_INDICATOR    "collection"
#define COLLECTION              "Collection"
#define COLLECTIONS             "Collections"

#define FONT_HAVE_BITMAPS   "Font contains bitmap strikes."

#define MAIN_MENU           "Main Menu"
#define DETAILED_VIEW       "Detailed View"

#define LANGUAGES_SUBDIR    "languages"
#define FONTS_SUBDIR        "fonts"
#define FAMILIES_SUBDIR     "families"
#define SCRIPTS_SUBDIR      "scripts"
#define BLOCKS_SUBDIR       "blocks"
#define DETAILIDX_SUBDIR    "detailidx"

#define FAMILIES_INDEX_NAME       "families"
#define FONTFORMATS_INDEX_NAME    "fontformats"
#define LANGUAGES_INDEX_NAME      "languages"
#define SCRIPTS_INDEX_NAME        "scripts"
#define BLOCKS_INDEX_NAME         "blocks"
#define THANKS_NAME               "thanks"

#define SPECIMEN_WIDTH_MAX  510
#define CHARSET_HEIGHT_MAX  32767 /* charset img will be split if not meet this */

#define HEADER_LINKS \
 {{FAMILIES_INDEX,  FAMILIES_INDEX_NAME".html", FAMILIES_INDEX_DESC, 1},\
  {LANGUAGES_INDEX, LANGUAGES_INDEX_NAME".html", LANGUAGES_INDEX_DESC, 1},\
  {FONTFORMATS_INDEX, FONTFORMATS_INDEX_NAME".html",\
                                               FONTFORMATS_INDEX_DESC, 1},\
  {SCRIPTS_INDEX, SCRIPTS_INDEX_NAME".html", SCRIPTS_INDEX_DESC, 1},\
  {BLOCKS_INDEX, BLOCKS_INDEX_NAME".html", BLOCKS_INDEX_DESC, 1},\
  {THANKS, THANKS_NAME".html", THANKS_DESC, 1}}
#define HL_FAMILIES_INDEX      0
#define HL_LANGUAGES_INDEX     1
#define HL_FONTFORMATS_INDEX   2
#define HL_SCRIPTS_INDEX       3
#define HL_BLOCKS_INDEX        4
#define HL_THANKS              5

typedef void (*content_function)(FILE *html, config_t config, void *output_args[]);

static void chameleon_page(FILE *html, const char *title, html_links page_path, 
                           content_function content_output, config_t config, 
                           void *output_arg[])
{
  static int js_written = 0;
  int i, j;
  char fontinfo_ver[VERSION_MAX];
  char fontconfig_ver[VERSION_MAX];
  char libpng_ver[VERSION_MAX];
  char freetype_ver[VERSION_MAX];
  char date[11];
  char year[5];

  timestamp(date, TS_DATE);
  timestamp(year, TS_YEAR);

  fprintf(html, 
          "<!DOCTYPE html>\n");
  fprintf(html, 
          "<html lang=\"en\">\n");
  fprintf(html, 
          "<head>\n");
  fprintf(html,
          "  <meta charset=\"utf-8\">\n");
  fprintf(html,
	  "  <link href=\""SOR"/favicon-32.png\" rel=\"icon\" sizes=\"32x32\" type=\"image/png\">\n");
  fprintf(html,
	  "  <link href=\""SOR"/favicon-48.png\" rel=\"icon\" sizes=\"48x48\" type=\"image/png\">\n");
  fprintf(html,
	  "  <link href=\""SOR"/favicon-64.png\" rel=\"icon\" sizes=\"64x64\" type=\"image/png\">\n");
  fprintf(html,
	  "  <link href=\""SOR"/favicon-96.png\" rel=\"icon\" sizes=\"96x96\" type=\"image/png\">\n");
  fprintf(html,
	  "  <link href=\""SOR"/favicon-144.png\" rel=\"icon\" sizes=\"144x144\" type=\"image/png\">\n");
  fprintf(html,
	  "  <link href=\""SOR"/favicon-192.png\" rel=\"icon\" sizes=\"192x192\" type=\"image/png\">\n");
  fprintf(html,
          "  <link href=\""CHAMELEON_LOCATION"dist/css/chameleon.css\" rel=\"stylesheet\">\n");
  fprintf(html,
	  "  <link href=\""CHAMELEON_LOCATION"docs/css/doc.css\" rel=\"stylesheet\">\n");
  
  if (!js_written)
  {
    js_write_script_specimen_view(config, "js-specimen-view.js");
    js_write_script_charset_view(config, "js-charset-view.js");
    js_write_script_family_search(config, "js-family-search.js");
    js_written = 1;
  }
  fprintf(html,
          "  <script src=../"JS_SUBDIR"/js-specimen-view.js "
          "type=\"text/javascript\" charset=\"utf-8\"></script>\n");
  fprintf(html,
          "  <script src=../"JS_SUBDIR"/js-charset-view.js "
          "type=\"text/javascript\" charset=\"utf-8\"></script>\n");
  fprintf(html,
          "  <script src=../"JS_SUBDIR"/js-family-search.js "
          "type=\"text/javascript\" charset=\"utf-8\"></script>\n");
  
  fprintf(html,
          "    <title>%s</title>\n", title);
  fprintf(html, 
          "</head>\n");
  fprintf(html,
          "  \n");
  fprintf(html,
          "<body class=\"d-flex flex-column\" style=\"min-height:100vh;\">\n");

  fprintf(html,
          "    <!-- Start: Header -->\n");
  fprintf(html,
          "    <nav class=\"navbar navbar-expand-lg sticky-top\">\n");
  fprintf(html,
          "      <a class=\"navbar-brand\" href=\"/families.html\"><img src=\""SOR"favicon.svg\" width=\"30\" height=\"30\""
	  "         class=\"d-inline-block align-top\" alt=\"🦎\" title=\"openSUSE\">Fontinfo</a>\n");
  fprintf(html,
          "      <div class=\"collapse navbar-collapse\" id=\"navbar-collapse\">\n");
  fprintf(html,
          "        <ul class=\"navbar-nav mr-auto\">\n");
  fprintf(html,
          "          <li class=\"nav-item\"><a class=\"nav-link\" href=\"/families.html\">Font Families</a></li>\n");
  fprintf(html,
          "          <li class=\"nav-item\"><a class=\"nav-link\" href=\"/languages.html\">Supported Languages</a></li>\n");
  fprintf(html,
          "          <li class=\"nav-item\"><a class=\"nav-link\" href=\"/fontformats.html\">Families by Font Formats</a></li>\n");
  fprintf(html,
          "          <li class=\"nav-item\"><a class=\"nav-link\" href=\"/scripts.html\">Families by Scripts</a></li>\n");
  fprintf(html,
          "          <li class=\"nav-item\"><a class=\"nav-link\" href=\"/blocks.html\">Families by Unicode Blocks</a></li>\n");
  fprintf(html,
          "          <li class=\"nav-item\"><a class=\"nav-link\" href=\"/thanks.html\">Thanks</a></li>\n");
  fprintf(html,
          "        </ul>\n");
  fprintf(html,
          "      </div>\n");
  fprintf(html,
          "    </nav>\n");
  fprintf(html,
          "    <div class=\"flex-fill py-5\"><div class=\"container\"><div class=\"row\"><div class=\"col-lg-9 toc-scope\">\n");
  fprintf(html,
          "      <nav aria-label=\"breadcrumb\">\n");
  fprintf(html,
          "        <ol class=\"breadcrumb\">\n");
  fprintf(html,
          "          <li class=\"breadcrumb-item\">Fontinfo</li>\n");
  for (i = 0; i < page_path.nlinks; i++)
  if (!is_null_link(page_path.links[i]))
  {
    if (page_path.links[i].active)
      fprintf(html,
              "          <li class=\"breadcrumb-item\"><a href=\"%s%s\" title=\"%s\">%s</a></li>\n",
              page_path.href_pref, page_path.links[i].href,
              page_path.links[i].title, page_path.links[i].text);
    else
      fprintf(html,
              "          <li class=\"breadcrumb-item\">%s</li>\n",
              page_path.links[i].text);
  }

  fprintf(html,
          "        </ol>\n");
  fprintf(html,
          "      </nav>\n");
  fprintf(html,
          "\n");
  content_output(html, config, output_arg);
  fprintf(html,
          "    </div><div class=\"toc-sidebar col-lg-3 d-none d-lg-block\"><div class=\"toc\"></div></div></div></div></div>\n");
  fprintf(html,
          "    <footer class=\"footer\"><div class=\"container\">&copy; 2017&ndash;2022 openSUSE contributors</div></footer>\n");
  fprintf(html,
          "    <script src=\""CHAMELEON_LOCATION"dist/js/jquery.slim.js\"></script>\n");
  fprintf(html,
          "    <script src=\""CHAMELEON_LOCATION"dist/js/bootstrap.bundle.js\"></script>\n");
  fprintf(html,
          "    <script src=\""CHAMELEON_LOCATION"dist/js/chameleon.js\">\n");
  fprintf(html,
          "    <script src=\""CHAMELEON_LOCATION"docs/js/doc.js\"></script>\n");
  fprintf(html,
          "  </body>\n");
  fprintf(html,
          "</html>\n");
}

static void content_families_index(FILE *html, config_t config, 
                                   void *opt_arg[])
{
  int f;

  FcFontSet *fontset = (FcFontSet *)opt_arg[0];
  FcChar8 *family, *file;
  char family_ww[FAMILY_NAME_LEN_MAX];

  FcChar32 ucs4_sentence[SENTENCE_NCHARS];
  int dir, random;
  const char *script;
  const char *lang;

  uinterval_stat_t script_stats[NUMSCRIPTS];
  int nscripts, v;

  package_info_t pi;
  char filename[FILEPATH_MAX];

  if (fontset->nfont == 0)
  {
    fprintf(stderr, "content_families_index(): no fonts found\n");
    return;
  }

  fprintf(html, 
          "          <h1>"FAMILIES_INDEX" in %s</h1>\n", config.location);  
  fprintf(html, 
          "    \n");
  fprintf(html, 
          "    <div class=\"mb-3\">"
	  "<input id=family_filter onkeyup=\"filter_function()\" class=\"form-control mb-3\" "
	  "type=\"text\" placeholder=\"Search in family names, e. g. Source, mono, etc. or all\"></div>\n");
  fprintf(html, 
          "    <br/>\n");

  fprintf(html, 
          "    <div class=\"table-responsive\"><table class=\"table\" id=\"list_families\">\n");

  for (f = 0; f < fontset->nfont; f++)
  {
    FcCharSet *charset;

    assert(fcinfo_get_translated_string(fontset->fonts[f], FC_FAMILY, 
                                        LANG_EN, &family)
           == FcResultMatch);
    snprintf(family_ww, FAMILY_NAME_LEN_MAX, "%s", (const char *)family);
    remove_spaces_and_slashes(family_ww);    

    assert(FcPatternGetString(fontset->fonts[f], FC_FILE, 0, &file)
           == FcResultMatch);
    file_from_package((char *)file, &pi);

    fprintf(html, 
            "            <tr style=\"display:none\">\n");
    fprintf(html, 
            "              <td style=\"vertical-align:middle\">"
            "<a href=\"../%s/%s.html\">%s</a></td>", 
            FAMILIES_SUBDIR, family_ww, family);

    assert(FcPatternGetCharSet(fontset->fonts[f], FC_CHARSET, 0, &charset)
           == FcResultMatch);

    if (config.debug)
      fprintf(stdout, "\nFONT(minispecimen, detailed index): %s\n", family);
    script = config.minispecimen_script;
    specimen_sentence(config, charset, config.minispecimen_script,
                      &dir, NULL, &lang,
                      &random, ucs4_sentence, SENTENCE_NCHARS);

    if (random)
    {
      /* font doesn't cover any sentence from config.minispecimen_script */
      /* try another script inside font */
      nscripts = charset_uinterval_statistics(charset, script_stats, 
                                              SCRIPT, UI_SORT_PERCENT);
      for (v = 0; v < nscripts; v++)
      {
        if (config.debug)
          fprintf(stdout, "trying %s script: ", script_stats[v].ui_name);
        specimen_sentence(config, charset, script_stats[v].ui_name,
                          &dir, NULL, &lang,
                          &random, ucs4_sentence, SENTENCE_NCHARS);
        if (! random)
          break;
      }

      /* no sentence which would fit found, use random characters instead */
      if (v == nscripts)
        specimen_sentence(config, charset, config.minispecimen_script,
                          &dir, NULL, &lang,
                          &random, ucs4_sentence, SENTENCE_NCHARS);
      else
        script = script_stats[v].ui_name;
    }

    fprintf(html, 
            "              <td>\n");
    write_minispecimen(html, fontset->fonts[f], 
                       "", config, ucs4_sentence, 
                       script, lang, dir % 2, 
                       "                ", 
                       1, "specimenimgmap",
                       SPECIMEN_WIDTH_MAX, NULL, NULL);
    fprintf(html, 
            "              </td>\n");

    if (config.install_type == YMP && pi.name[0])
    {
      generate_ymp(config, pi.name, filename);
      fprintf(html,
              "              <td style=\"vertical-align:middle\">\n");
      fprintf(html,
              "              <a style=\"font-weight:bold;color:#690\""
              " href=../%s>1&nbsp;Click Install</a>", filename);
      fprintf(html,
              "              </td>\n");
    }

    fprintf(html, 
            "            </tr>\n");
  }
  fprintf(html, 
          "    </table>\n");

  return;
}

void chameleon_families_index(config_t config)
{
  char fname[FILEPATH_MAX];
  char parts_letters[FONT_INDEX_PARTS_MAX][2];
  char parts_links[FONT_INDEX_PARTS_MAX][FILEPATH_MAX];
  char all_link[FILEPATH_MAX];

  FILE *html;
  int f, nparts;
  FcChar8 *family, *prev_family;

  FcFontSet *fontset;
 
  const html_link path_links[] = 
     {{FAMILIES_INDEX, FAMILIES_INDEX_NAME".html", FAMILIES_INDEX_DESC, 0}};
  const html_links path = {"", path_links, ASIZE(path_links), ""};

  void *arg[1];

  snprintf(fname, FILEPATH_MAX, "%s/"FAMILIES_INDEX_NAME".html", config.dir);
  html = open_write(fname, "chameleon_families_index");

  fontset = families_index(config);

  prev_family = NULL;
  nparts = 0;
  for (f = 0; f < fontset->nfont; f++)
  {
    assert(fcinfo_get_translated_string(fontset->fonts[f], FC_FAMILY, 
                                        LANG_EN, &family)
           == FcResultMatch);
    /* ignore non standard family names (probably utf-8 or so) */
    if (!isprint(family[0]))
      continue;

    if (!prev_family || toupper(prev_family[0]) != toupper(family[0]))
    {
      assert(nparts < FONT_INDEX_PARTS_MAX);

      parts_letters[nparts][0] = toupper(family[0]);
      parts_letters[nparts][1] = '\0';
      snprintf(parts_links[nparts], FILEPATH_MAX, FAMILIES_INDEX_NAME".%c.html",
               tolower(family[0]));
      nparts++;
    }

    prev_family = family;
  }

  snprintf(all_link, FILEPATH_MAX, FAMILIES_INDEX_NAME".ALL.html");

  arg[0] = fontset;
  chameleon_page(html, FAMILIES_INDEX, path, 
                 content_families_index, config, arg);

  fclose(html);
  return;
}

static void content_family_styles_indexes(FILE *html, config_t config, 
                                          void *output_arg[])
{
  FcFontSet *styleset = (FcFontSet *)output_arg[0];
  family_info_t *fi = (family_info_t *)output_arg[1];

  FcChar8 *family, *style, *file;
  FcCharSet *charset;
  char style_ww[STYLE_NAME_LEN_MAX];
  char family_ww[FAMILY_NAME_LEN_MAX];

  int s;

  FcChar32 ucs4_sentence[SENTENCE_NCHARS];
  int dir, random;
  const char *script;
  const char *lang;

  uinterval_stat_t script_stats[NUMSCRIPTS];
  int nscripts, v;

  char filename[FILEPATH_MAX];
  package_info_t pi;

  assert(fcinfo_get_translated_string(styleset->fonts[0], FC_FAMILY, 
                                      LANG_EN, &family)
         == FcResultMatch);
  assert(FcPatternGetString(styleset->fonts[0], FC_FILE, 0, &file)
             == FcResultMatch);
  file_from_package((char *)file, &pi);


  fprintf(html, 
          "          <h1>"FAMILY_OVERVIEW"</h1>\n");
  fprintf(html,
          "          \n");

  fprintf(html, "    <div class=\"table-responsive\">\n");
  fprintf(html, 
          "          <table class=\"table\">\n"); /* at least fontformat should be there */
  if (fi->fontformat[0])
    fprintf(html, 
            "            <tr><td><b>"FAMILY_FONT_FORMAT
            "</b></td><td>%s</td></tr>\n",
            fi->fontformat);
  if (fi->capability[0])
    fprintf(html, 
            "            <tr><td><b>"FAMILY_FONT_CAPABILITY
            "</b></td><td>%s</td></tr>\n",
            fi->capability);
  if (fi->foundry[0])
    fprintf(html,
            "            <tr><td><b>"FAMILY_FONT_FOUNDRY
            "</b></td><td>%s</td></tr>\n", fi->foundry);
  fprintf(html, 
          "           </table>\n");
  fprintf(html, 
          "          \n");
  fprintf(html,
	  "           </div>\n");
  fprintf(html,
          "           <br/>\n");


  fprintf(html,
          "          <table style=\"width:100%\"><tr><td><h2>"FAMILY_STYLES"</h2></td>\n");

  if (config.install_type == YMP && pi.name[0])
  {
    generate_ymp(config, pi.name, filename);
    fprintf(html,
            "              <td style=\"text-align:right\">"
	    "<a style=\"font-size:medium;font-weight:bold;color:#690\""
            " href=../%s>1&nbsp;Click Install</a></td>", filename);
  }
  fprintf(html, "</tr></table>\n");
	  
  fprintf(html, 
	  "    <div class=\"table-responsive\">\n");
  fprintf(html, 
          "          <table class=\"table\">");
  for (s = 0; s < styleset->nfont; s++)
  {
    assert(fcinfo_get_translated_string(styleset->fonts[s], FC_STYLE, 
                                        LANG_EN, &style)
           == FcResultMatch);
    snprintf(family_ww, FAMILY_NAME_LEN_MAX, "%s", (char *)family);
    remove_spaces_and_slashes(family_ww);    
    snprintf(style_ww, STYLE_NAME_LEN_MAX, "%s", (char *)style);
    remove_spaces_and_slashes(style_ww);

    fprintf(html, 
            "            <tr>\n");
    fprintf(html, 
            "              <td style=\"vertical-align:middle\">"
                             "<a href=\"../%s/%s%s.html\">%s %s</a></td>\n", 
            FONTS_SUBDIR, family_ww, style_ww, family, style);
     
    assert(FcPatternGetCharSet(styleset->fonts[s], FC_CHARSET, 0, &charset)
           == FcResultMatch);

    if (config.debug)
      fprintf(stdout, "\nFONT(minispecimen, styles index): %s, %s\n", 
              family, style);
    script = config.minispecimen_script;
    specimen_sentence(config, charset, config.minispecimen_script,
                      &dir, NULL, &lang,
                      &random, ucs4_sentence, SENTENCE_NCHARS);

    if (random)
    {
      /* font doesn't cover any sentence from config.minispecimen_script */
      /* try another script inside font */
      nscripts = charset_uinterval_statistics(charset, script_stats, 
                                              SCRIPT, UI_SORT_PERCENT);
      for (v = 0; v < nscripts; v++)
      {
        if (config.debug)
          fprintf(stdout, "trying %s script: ", script_stats[v].ui_name);
        specimen_sentence(config, charset, script_stats[v].ui_name,
                          &dir, NULL, &lang,
                          &random, ucs4_sentence, SENTENCE_NCHARS);
        if (! random)
          break;
      }

      /* no sentence which would fit found, use random characters instead */
      if (v == nscripts)
        specimen_sentence(config, charset, config.minispecimen_script,
                          &dir, NULL, &lang,
                          &random, ucs4_sentence, SENTENCE_NCHARS);
      else
        script = script_stats[v].ui_name;
    }

    fprintf(html, 
            "              <td>\n");
    write_minispecimen(html, styleset->fonts[s],
                       FAMILIES_SUBDIR, config, ucs4_sentence, 
                       script, lang, dir % 2, 
                       "                ", 1, "specimenimgmap",
                       SPECIMEN_WIDTH_MAX, NULL, NULL);
    fprintf(html, 
            "              </td>\n");
    fprintf(html, 
            "            </tr>\n");    
  }
  fprintf(html, 
          "          </table>\n");
  fprintf(html,
	  "          </div>\n");

  return;
}

void chameleon_family_styles_indexes(config_t config)
{
  FcFontSet *fontset;
  int f;
  char dirname[FILEPATH_MAX];

  FcChar8 *family;
  FILE *html;
  char fname[FILEPATH_MAX];

  html_link path_links[] = 
     {{FAMILIES_INDEX, FAMILIES_INDEX_NAME".html", FAMILIES_INDEX_DESC, 1}, 
      {NULL, NULL, NULL, 0}}; 
  const html_links path = {"../", path_links, ASIZE(path_links), ""};

  FcFontSet *styleset;
  family_info_t fi;
  void *arg[2];

  snprintf(dirname, FILEPATH_MAX, "%s/%s", config.dir, FAMILIES_SUBDIR);
  create_dir((const char *)dirname);

  fontset = families_index(config);

  for (f = 0; f < fontset->nfont; f++)
  {
    assert(fcinfo_get_translated_string(fontset->fonts[f], FC_FAMILY, 
                                        LANG_EN, &family)
           == FcResultMatch);

    snprintf(fname, FILEPATH_MAX, "%s/%s.html", dirname, family);
    remove_spaces_and_slashes(&fname[strlen(dirname)+1]);
    html = open_write(fname, "chameleon_family_styles_indexes");

    styleset = styles_index(family, config, &fi);

    snprintf(fname, FILEPATH_MAX, "%s.html", family);
    remove_spaces_and_slashes(fname);
    path_links[1].text = (char *)family;
    path_links[1].href = fname;
    path_links[1].title = "";
    path_links[1].active = 0;
 
    arg[0] = styleset;
    arg[1] = &fi;
    chameleon_page(html, (const char *)family, path,
                   content_family_styles_indexes, config, arg);

    fclose(html);
  }

  FcFontSetDestroy(fontset);
  return; 
}

static void content_font_card(FILE *html, config_t config, 
                              void *output_arg[])
{
  FcPattern *pattern = (FcPattern *)output_arg[0];

  FcCharSet *charset;
  FcStrList *strlist = NULL;
  FcLangSet *langset;

  FcChar8 *family, *style, *file, *lang;
  int slant, weight, width, spacing, hintstyle, rgba, lcdfilter;
  FcBool scalable, embeddedbitmaps, antialias, hinting, autohint;

  face_info_t fi;
  int s, t;

  int f;

  int sp_height, sp_height_max;

  char fname[FILEPATH_MAX];

  FcChar32 ucs4_sentence[SENTENCE_NCHARS];
  int dir, random;
  const char *language;
  img_transform_t transform;

  package_info_t pi;
  int r;
  char filename[FILEPATH_MAX];

  uinterval_stat_t script_stats[NUMSCRIPTS];
  uinterval_stat_t block_stats[NUMBLOCKS];
  int nscripts, nsignificantscripts, v, v1;
  int nblocks;
  char script_wu[SCRIPT_NAME_LEN_MAX];
  char block_ws[BLOCK_NAME_LEN_MAX];

  int nth_table;

  assert(fcinfo_get_translated_string(pattern, FC_FAMILY, LANG_EN, &family)
         == FcResultMatch);
  assert(fcinfo_get_translated_string(pattern, FC_STYLE, LANG_EN, &style)
         == FcResultMatch);
  assert(FcPatternGetString(pattern, FC_FILE, 0, &file) 
         == FcResultMatch);
  assert(FcPatternGetCharSet(pattern, FC_CHARSET, 0, &charset)
         == FcResultMatch);
  assert(FcPatternGetInteger(pattern, FC_SLANT, 0, &slant)
         == FcResultMatch);
  assert(FcPatternGetInteger(pattern, FC_WEIGHT, 0, &weight)
         == FcResultMatch);
  assert(FcPatternGetInteger(pattern, FC_WIDTH, 0, &width)
         == FcResultMatch);
  if (FcPatternGetInteger(pattern, FC_SPACING, 0, &spacing)
      != FcResultMatch)
    spacing = CONST_NUM_UNKNOWN;
  assert(FcPatternGetInteger(pattern, FC_HINT_STYLE, 0, &hintstyle)
         == FcResultMatch);
  assert(FcPatternGetBool(pattern, FC_SCALABLE, 0, &scalable)
         == FcResultMatch);
  assert(FcPatternGetBool(pattern, FC_EMBEDDED_BITMAP, 0, &embeddedbitmaps)
         == FcResultMatch);
  assert(FcPatternGetBool(pattern, FC_ANTIALIAS, 0, &antialias)
         == FcResultMatch);
  assert(FcPatternGetBool(pattern, FC_HINTING, 0, &hinting)
         == FcResultMatch);
  assert(FcPatternGetBool(pattern, FC_AUTOHINT, 0, &autohint)
         == FcResultMatch);
  assert(FcPatternGetLangSet(pattern, FC_LANG, 0, &langset)
         == FcResultMatch);
  if (FcPatternGetInteger(pattern, FC_RGBA, 0, &rgba)
      != FcResultMatch)
    rgba = CONST_NUM_UNKNOWN;
  if (FcPatternGetInteger(pattern, FC_LCD_FILTER, 0, &lcdfilter)
      != FcResultMatch)
    lcdfilter = CONST_NUM_UNKNOWN;

  strlist = FcStrListCreate(FcLangSetGetLangs(langset));

  ft_initialize_info(&fi, file);

  file_from_package((char *)file, &pi);

  nscripts = charset_uinterval_statistics(charset, script_stats, 
                                          SCRIPT, UI_SORT_PERCENT);
  nblocks = charset_uinterval_statistics(charset, block_stats, 
                                         BLOCK, UI_SORT_NONE);

  if (config.debug)
    fprintf(stdout, "\nFONT(specimen): %s, %s\n", family, style);

  fprintf(html,  
          "          <h1>"FONT_CARD"</h1>\n");
  fprintf(html, 
          "          \n");

  fprintf(html,
          "          <div class=\"table-responsive\">\n");
  fprintf(html, 
          "          <table class=\"table\">\n");
  if (config.generate_indexes)
  {
    snprintf(fname, FILEPATH_MAX, "%s.html", family);
    remove_spaces_and_slashes(fname);
    fprintf(html, 
            "            <tr><td><b>Family:</b></td><td>"
            "<a href=\"../%s/%s\">%s</a></td></tr>\n",
            FAMILIES_SUBDIR, fname, family);
  }
  else
    fprintf(html,
            "            <tr><td><b>Family:</b></td><td>%s</td></tr>", family);
  fprintf(html, 
          "            <tr><td><b>Style:</b></td><td>%s</td></tr>\n", style);
  fprintf(html, 
          "            <tr>\n");
  fprintf(html, 
          "              <td><b>File(s):</b></td>\n");
  fprintf(html, 
          "              <td>\n");
  fprintf(html, 
          "                <table>\n");
  for (f = 0; 
       FcPatternGetString(pattern, FC_FILE, f, &file) == FcResultMatch; 
       f++)
    fprintf(html, 
            "                  <tr><td>%s</td></tr>\n", file);
  fprintf(html, 
          "                </table>\n");
  fprintf(html, 
          "               </td>\n");
  fprintf(html, 
          "            </tr>\n");
  fprintf(html, 
          "            <tr>\n");
  fprintf(html, 
          "              <td><b>Scripts (most covered):</b></td>\n");
  fprintf(html, 
          "              <td>\n");
  fprintf(html, 
          "                <table>\n");
  v = 0;
  while (v < SCRIPTS_MAX && v < nscripts)
  {
    /* wipe scripts with really low coverage, even for v == 0 */
    if (script_stats[v].coverage < SCRIPTS_COVERAGE_HARD_MIN)
      break;

    /* wipe scripts with low coverage if we have at least one yet */
    if (v > 0 && script_stats[v].coverage < SCRIPTS_COVERAGE_MIN)
      break;

    snprintf(script_wu, SCRIPT_NAME_LEN_MAX, "%s", script_stats[v].ui_name);
    underscores_to_spaces(script_wu);

    if (config.generate_indexes)
      fprintf(html, 
              "                    <tr><td><a href=\"../%s/script-%s.html\">"
              "%s%s</a> (%.1f%%)</td></tr>\n",
              SCRIPTS_SUBDIR, script_stats[v].ui_name, script_wu, 
              script_is_collection(script_stats[v].ui_name) 
                  ? " ("COLLECTION_INDICATOR")" : "",
              script_stats[v].coverage);
    else
      fprintf(html, 
              "                    <tr><td>%s%s (%.1f%%)</td></tr>\n",
              script_is_collection(script_stats[v].ui_name) 
                  ? " ("COLLECTION_INDICATOR")" : "",
              script_wu, script_stats[v].coverage);
    v++;
  }
  nsignificantscripts = v;

  if (config.debug)
  { 
    fprintf(stdout, "scripts leaving aside: [");
    for (v1 = v; v1 < nscripts; v1++)
      fprintf(stdout, "%s (%.1f%%) ", 
              script_stats[v1].ui_name, script_stats[v1].coverage);
    fprintf(stdout, "]\n");
  }   

  if (nsignificantscripts == 0)
    fprintf(html,
            "                  <tr><td>none</td></tr>\n");
  fprintf(html, 
          "                </table>\n");
  fprintf(html, 
          "              </td>\n");
  fprintf(html, 
          "            </tr>\n");
  fprintf(html, 
          "          </table>\n");
  fprintf(html,
	  "          </div>\n");
  fprintf(html,
          "          <br/>\n");
  fprintf(html,
          "          \n");

  fprintf(html, 
          "          <h2 id=\"specimen\">"SPECIMEN"</h2>\n");
  fprintf(html,
          "          <table>\n");
  fprintf(html,
          "            <tr><td>\n");
  fprintf(html,
          "              <script>\n");
  fprintf(html,
          "                scripts = [");
  if (!config.specimen_sentence)
  {
    for (v = 0; v < nsignificantscripts; v++)
    {
      fprintf(html, "%s'%s'", v > 0 ? ", " : "", script_stats[v].ui_name);
    }
  }
  
  fprintf(html,
          "];\n");
  fprintf(html,
          "                specimen_types = ['bitmap'%s];\n", 
          config.outline_specimen ? ", 'svg'" : "");
  fprintf(html,
          "                initialize_specimen_tabs(scripts, specimen_types, 'Specimen');\n");
  fprintf(html,
          "              </script>\n");
  fprintf(html,
          "            </td></tr>\n");

  fprintf(html,
          "            <tr><td>\n");
  /* try to reuse following for(;;) for 'sentence' of random chars */
  /* and for enforced sentence */
  if (nsignificantscripts == 0)
  {
    script_stats[0].ui_name = NULL;
    nsignificantscripts = 1;
  } 
  else if (config.specimen_sentence)
  {
    script_stats[0].ui_name = config.specimen_script;
    nsignificantscripts = 1;
  }
  

  sp_height_max = 0;
  for (v = 0; v < nsignificantscripts; v++)
  {
    if (config.debug)
    {
      if (script_stats[v].ui_name)
        fprintf(stdout, "%s script (%.1f%%): ", 
                script_stats[v].ui_name, script_stats[v].coverage);
      else
        fprintf(stdout, "no script found: ");
    }
    /* random flag not used here so far, but for minispecimens */
    specimen_sentence(config, charset, script_stats[v].ui_name, 
                      &dir, &transform, &language,
                      &random, ucs4_sentence, SENTENCE_NCHARS);

    fprintf(html,
        "              <div class=\"specimen\" id=\"bitmap%sSpecimen\""
                          " style=\"display:%s\">\n", 
        script_stats[v].ui_name ? script_stats[v].ui_name : NO_SCRIPT, 
        v == 0 ? "block" : "none");
    write_specimen(html, pattern, FONTS_SUBDIR, 
                   config, ucs4_sentence, 
                   script_stats[v].ui_name, language, dir, transform, 
                   "                  ", "specimenimgmap",
                   SPECIMEN_WIDTH_MAX, NULL, &sp_height);

    /* nowdays should all sp_height be same per font card
       because of naive specimen text size computation,
       but 'set the stage' for variable length of specimens */
    if (sp_height > sp_height_max)
      sp_height_max = sp_height;

    fprintf(html,
          "              </div>\n");

    if (config.outline_specimen)
    {
      /* write also SVG specimen to see how font renderes */
      /* on client system */
      fprintf(html,
          "              <div class=\"specimen\" id=\"svg%sSpecimen\""
                            " style=\"display:none\">\n",
          script_stats[v].ui_name ? script_stats[v].ui_name : "");
      write_svg_specimen(html, pattern, FcFalse,
                         config, ucs4_sentence, 
                         script_stats[v].ui_name, language, dir, transform,
                         "                    ", 
                         SPECIMEN_WIDTH_MAX, NULL, NULL);
      fprintf(html,
              "              </div>\n");
    }
  }
  fprintf(html,
          "            </td></tr>\n");
  fprintf(html,
          "          </table>\n");
  fprintf(html,
          "          </br>\n");
  /* ensure specimen images are bounded to the same height */
  /* even if nowdays should all specimen have same height,
     svg happens to render slightly bigger (tested with
     firefox and opera) */
  fprintf(html,
          "          <style>div.specimen { height:%dpx }</style>\n", 
          sp_height_max);
  fprintf(html, 
          "          \n");

  if (config.generate_swpkginfo)
  {
    fprintf(html, 
            "          <h2 id=\"sw_package\">"SOFTWARE_PACKAGE"</h2>\n");
    fprintf(html, "    <div class=\"table-responsive\">\n");
    fprintf(html, 
            "          <table class=\"table\">\n");
    if (pi.name[0])
    {
      fprintf(html, 
              "            <tr><td><b>Name:</b></td><td>%s</td><td>", pi.name);
      if (config.install_type == YMP)
      {
        generate_ymp(config, pi.name, filename);
        fprintf(html,
                "<a style=\"font-weight:bold;color:#690\" href=../%s>"
                "1&nbsp;Click Install</a>", filename);
      }
      fprintf(html, 
              "</td></tr>\n");
    }
    if (pi.version[0])
      fprintf(html, 
              "            <tr><td><b>Version:</b></td>"
              "<td colspan=\"%d\">%s</td></tr>\n",
              config.install_type == YMP ? 2 : 1, pi.version);
    if (pi.url[0])
      fprintf(html, 
              "            <tr><td><b>Url:</b></td>"
              "<td colspan=\"%d\"><a href=\"%s\">%s</a></td></tr>\n",
              config.install_type == YMP ? 2 : 1, pi.url, pi.url);
    if (pi.license[0])
      fprintf(html, 
              "            <tr><td><b>License:</b></td>"
              "<td colspan=\"%d\">%s</td></tr>\n",
              config.install_type == YMP ? 2 : 1, pi.license);
    if (pi.description[0])
      fprintf(html, 
              "            <tr><td><b>Description:</b></td>"
              "<td colspan=\"%d\">%s</td></tr>\n",
              config.install_type == YMP ? 2 : 1, pi.description);
    if (config.install_type == REPO)
    {
      fprintf(html, 
              "              <tr>\n");
      fprintf(html,
              "                <td><b>Repositories:</b></td>\n");
      fprintf(html,
              "                <td>\n");
      for (r = 0; r < config.nrepositories; r++)
        fprintf(html,
                "                  <a href=\"%s\">%s</a>\n", 
                config.repositories[r][1], config.repositories[r][0]);
      fprintf(html,
              "                </td>\n");
      fprintf(html,
              "              </tr>\n");
    }
    fprintf(html, 
            "          </table>\n");
    fprintf(html, 
            "          </div>\n");
    fprintf(html, 
            "          <br/>\n");
    fprintf(html, 
            "          \n");
  }

  fprintf(html, 
          "          <h2 id=\"shape\">"FONT_CARD_SHAPE"</h2>\n");
  fprintf(html, "    <div class=\"table-responsive\">\n");
  fprintf(html, 
          "          <table class=\"table\">\n");
  fprintf(html, 
          "            <tr><td><b>Slant:</b></td><td>%s</td></tr>\n",
          slant_name(slant));
  fprintf(html, 
          "            <tr><td><b>Weight:</b></td><td>%s (%d)</td></tr>\n",
          weight_name(weight), weight);
  fprintf(html, 
          "            <tr><td><b>Width:</b></td><td>%s (%d)</td></tr>\n",
          width_name(width), width);
  if (spacing != CONST_NUM_UNKNOWN)
    fprintf(html, 
            "            <tr><td><b>Spacing:</b></td><td>%s</td></tr>\n",
            spacing_name(spacing));
  fprintf(html, 
          "            <tr><td><b>Scalable:</b></td><td>%s</td></tr>\n",
          scalable ? YES : NO);
  if (scalable)
  {
    fprintf(html, 
            "            <tr>\n");
    fprintf(html, 
            "              <td><b>TrueType Tables:</b></td>\n");
    fprintf(html, 
            "              <td><table>\n");
    if (fi.tt_tables_present)
    {
      nth_table = 0;
      for (t = 0; t < NUMTTTABLES; t++)
        if (fi.tt_tables[t])
        {
          if (nth_table % 5 == 0)
            fprintf(html, 
                    "                <tr>");
          fprintf(html, 
                  "<td>%s</td>", tt_tables[t]);
          nth_table++;
          if (nth_table % 5 == 0)
            fprintf(html, "</tr>\n");
        }

      if (nth_table % 5 != 0)
      {
        for (; nth_table % 5 != 0; nth_table++)
          fprintf(html, "<td></td>");
        fprintf(html, "</tr>\n");
      }
    }
    else
      fprintf(html, 
              "                <tr><td>none</td></tr>");
    fprintf(html, 
            "              </table></td>\n");
    fprintf(html, 
            "            </tr>\n");
  }
  fprintf(html, 
          "          </table>\n");
  fprintf(html, 
          "          </div>\n");
  fprintf(html, 
          "          <br/>\n");
  fprintf(html,
          "          \n");

  if (scalable)
  {
    fprintf(html, 
            "          <h2 id=\"rendering\">"FONT_CARD_RENDERING"</h2>\n");
    fprintf(html, "    <div class=\"table-responsive\">\n");
    fprintf(html, 
            "          <table class=\"table\">\n");
    fprintf(html, 
            "            <tr><td><b>Antialias:</b></td><td>%s</td></tr>\n",
            antialias ? YES : NO);
    fprintf(html, 
            "            <tr><td><b>Hinting:</b></td><td>%s</td></tr>\n",
            hinting ? YES : NO);
    fprintf(html, 
            "            <tr><td><b>Force autohint:</b></td><td>%s</td></tr>\n",
            autohint ? YES : NO);
    fprintf(html, 
            "            <tr><td><b>Hintstyle:</b></td>"
            "<td>%s (%d)</td></tr>\n",
             hintstyle_name(hintstyle), hintstyle);
    if (rgba != CONST_NUM_UNKNOWN)
      fprintf(html, 
              "      <tr><td><b>Subpixel layout:</b></td><td>%s</td></tr>\n",
              rgba_name(rgba));
    if (lcdfilter != CONST_NUM_UNKNOWN)
      fprintf(html, 
              "      <tr><td><b>LCD filter:</b></td><td>%s</td></tr>\n",
              lcdfilter_name(lcdfilter));

    fprintf(html,
            "            <tr><td><b>Available Bitmap Strikes:</b></td><td>");
    if (fi.num_fixed_sizes > 0)
    {
      for (s = 0; s < fi.num_fixed_sizes; s++)
        fprintf(html, "%dx%d ", fi.available_sizes[s].width,
                                fi.available_sizes[s].height);
    }
    else
    {
      fprintf(html, NONE_BITMAP_STRIKES);
    }
    fprintf(html,
            "</td></tr>\n");

    if (fi.num_fixed_sizes > 0)
      fprintf(html, 
              "            <tr><td><b>Use Embedded Bitmaps:</b></td><td>%s</td></tr>\n",
              embeddedbitmaps ? YES : NO);
    fprintf(html, 
            "          </table>\n");
    fprintf(html, 
            "          </div>\n");
    fprintf(html,
            "          <br/>\n");
    fprintf(html,
            "          \n");
  }

  if (langset)
  {
    fprintf(html, 
            "          <h2 id=\"languages\">"FONT_CARD_LANGUAGES"</h2>\n");
    fprintf(html, 
            "          <table>\n");
    fprintf(html, 
            "            <tr><td>");
    FcChar8 *prev_lang = NULL;
    while ((lang = FcStrListNext(strlist)))
    {
      if (prev_lang && prev_lang[0] != lang[0])
      {
        fprintf(html, " |</td></tr>\n");
        fprintf(html, "            <tr><td>");
      }
      fprintf(html, " | <a href=\"../%s/lang-%s.html\">%s</a>",
              LANGUAGES_SUBDIR, lang, lang);
      prev_lang = lang;
    }
    if (!prev_lang)
      fprintf(html, "| none");

    fprintf(html, 
            " |</td></tr>\n");
    fprintf(html, 
            "          </table>\n");
    fprintf(html, 
            "          <br/>\n");
    fprintf(html, 
            "          \n");
  }

  if (config.generate_charset && charset)
  {
    fprintf(html, 
            "          <h2 id=\"character_set\">"FONT_CARD_CHARSET"</h2>\n");
    fprintf(html, "    <div class=\"my-3\">\n");
    fprintf(html,
            "          <table><tr>\n");
    fprintf(html,
            "            <td><button class=\"btn btn-secondary\" onclick=\"charset_blocks_show()\">Show all</button></td>");
    fprintf(html,
            "            <td><button class=\"btn btn-secondary\" onclick=\"charset_blocks_hide()\">Hide all</button></td>");
    fprintf(html,
            "          </tr></table>\n");
    fprintf(html,
            "          </div>\n");
    fprintf(html, 
            "          <table>\n");
    fprintf(html,
            "            <script>charset_shown = {}; charset_html = {}; </script>\n");
    
    for (v = 0; v < nblocks; v++)
    {
      snprintf(block_ws, BLOCK_NAME_LEN_MAX, "%s",  block_stats[v].ui_name);
      remove_spaces_and_slashes(block_ws);

      fprintf(html,
              "            <script>charset_html['cellCharset%s'] = \n", 
              block_ws);
      fprintf(html,
              "              '\\\n");
      write_charset(html,
                    pattern,
                    block_stats[v].ui_name,
                    BLOCK,
                    FONTS_SUBDIR,
                    config,
                    "                ",
                    "\\",
                    "charsetimgmap",
                    0,
                    CHARSET_HEIGHT_MAX);
      fprintf(html,
              "              '\n");
      fprintf(html,
              "            </script>\n");

      fprintf(html, 
              "            <script>charset_shown['cellCharset%s'] = 0"
                           "</script>\n", 
              block_ws);
      fprintf(html,
              "            <tr><td><a "
              "onclick=\"charset_block_toggle('cellCharset%s', "
              "charset_html['cellCharset%s'])""\">"
              "[<b id=\"cellCharset%sToggle\" style=\"font-family:"
              " monospace;\">+</b>] %s (U+%05X - U+%05X)</a></td></tr>\n",
              block_ws, block_ws, block_ws, block_stats[v].ui_name,
              block_lbound(block_stats[v].ui_name), 
              block_ubound(block_stats[v].ui_name));
      fprintf(html,
              "            <tr><td style=\"display:none\""
              " id=\"cellCharset%s\">\n", block_ws);

      fprintf(html,
              "            </td></tr>\n");
    }

    fprintf(html, 
            "          </table>\n");
  }

  ft_free_info(&fi);
  return;
}

void chameleon_font_cards(config_t config)
{
  FcFontSet *fontset;
  int f;
  char dirname[FILEPATH_MAX];

  FcChar8 *family, *style;
  FcBool scalable;
  FILE *html;
  char fname[FILEPATH_MAX];
  char fname2[FILEPATH_MAX];
  char family_and_style[TITLE_MAX];

  html_link path_links[] = 
     {{FAMILIES_INDEX, FAMILIES_INDEX_NAME".html", FAMILIES_INDEX_DESC, 1}, 
      {NULL, NULL, NULL, 0},
      {NULL, NULL, NULL, 0}}; 
  const html_links path = {"../", path_links, ASIZE(path_links), ""};
  const html_link lsw_package = {SOFTWARE_PACKAGE, "#sw_package", "", 1};
  const html_link lshape = {FONT_CARD_SHAPE, "#shape", "", 1};
  const html_link lrendering = {FONT_CARD_RENDERING, "#rendering", "", 1};
  const html_link llanguages = {FONT_CARD_LANGUAGES, "#languages", "", 1};
  const html_link lcharset = {FONT_CARD_CHARSET, "#character_set", "", 1};
  void *arg[1];
  int rendering_idx;

  snprintf(dirname, FILEPATH_MAX, "%s/%s", config.dir, FONTS_SUBDIR);
  create_dir((const char *)dirname);

  fontset = font_index(config);

  for (f = 0; f < fontset->nfont; f++)
  {
    assert(fcinfo_get_translated_string(fontset->fonts[f], FC_FAMILY, 
                                        LANG_EN, &family)
           == FcResultMatch);
    assert(fcinfo_get_translated_string(fontset->fonts[f], FC_STYLE, 
                                        LANG_EN, &style)
           == FcResultMatch);
    assert(FcPatternGetBool(fontset->fonts[f], FC_SCALABLE, 0, &scalable)
           == FcResultMatch);

    snprintf(fname, FILEPATH_MAX, "%s/%s%s.html", dirname, family, style);
    remove_spaces_and_slashes(&fname[strlen(dirname)+1]);
    html = open_write(fname, "chameleon_font_cards");

    snprintf(fname, FILEPATH_MAX, FAMILIES_SUBDIR"/%s.html", family);
    remove_spaces_and_slashes(&fname[strlen(FAMILIES_SUBDIR)+1]);
    path_links[1].text  = (char *)family;
    path_links[1].href  = fname;
    path_links[1].title = FAMILY_OVERVIEW_DESC;
    path_links[1].active = 1;

    snprintf(fname2, FILEPATH_MAX, "%s%s.html", family, style);
    remove_spaces_and_slashes(fname2);
    snprintf(family_and_style, TITLE_MAX, "%s %s", family, style);
    path_links[2].text  = family_and_style; 
    path_links[2].href  = fname2;
    path_links[2].title = FONT_CARD_DESC;
    path_links[2].active = 0;

    arg[0] = fontset->fonts[f]; 
    chameleon_page(html, (const char *)family, path, 
               content_font_card, config, arg);

    fclose(html);
  }

  FcFontSetDestroy(fontset);
  return;
}

static void content_languages_index(FILE *html, config_t config, 
                                    void *output_arg[])
{
  FcStrList *langs = (FcStrList *)output_arg[0];
  FcChar8 *lang, *prev_lang;

  char title[TITLE_MAX];
  const char *lname;
  
  snprintf(title, TITLE_MAX, "Languages Supported by %s", config.location);
  fprintf(html, "          <h1>%s</h1>\n", title);
  fprintf(html, "          \n");
  prev_lang = NULL;
  while ((lang = FcStrListNext(langs)))
  {
    if (prev_lang)
    {
      if (prev_lang[0] != lang[0])
      {
        fprintf(html, " </p>\n");
        fprintf(html, "          <h2>%c</h2>\n", lang[0]);
        fprintf(html, "          <p> |");
      }
    }
    else
    {
      fprintf(html, "          <h2>%c</h2>\n", lang[0]);
      fprintf(html, "          <p> |");
    }

    lname = (const char *)lang_name(lang);
    fprintf(html, " <a href=\"%s/lang-%s.html\">%s",
            LANGUAGES_SUBDIR, lang, lang);
    if (strcmp(lname, (const char *)CONST_STRING_UNKNOWN) != 0)
      fprintf(html, " (%s)", lname);
    fprintf(html, "</a> |");

    prev_lang = lang;
  }
  fprintf(html, " </p>\n");
  return;
}

void chameleon_languages_index(config_t config)
{
  FILE * html;
  char fname[FILEPATH_MAX];
  FcStrSet *strset;
  FcStrList *langs;

  const html_link path_links[] = 
     {{LANGUAGES_INDEX, LANGUAGES_INDEX_NAME".html", LANGUAGES_INDEX_DESC, 0}};
  const html_links path = {"", path_links, ASIZE(path_links), ""};

  void *arg[1];

  snprintf(fname, FILEPATH_MAX, "%s/"LANGUAGES_INDEX_NAME".html", config.dir);
  html = open_write(fname, "chameleon_languages_index");

  strset = language_index(config);
  langs = FcStrListCreate(strset);

  arg[0] = langs;
  chameleon_page(html, LANGUAGES_INDEX, path,
                 content_languages_index, config, arg);

  fclose(html);
  FcStrListDone(langs);
  FcStrSetDestroy(strset);
  return;
}

static void content_language_font_index(FILE *html, config_t config, 
                                        void *output_arg[])
{
  FcChar8 *lang = (FcChar8 *)output_arg[0];
  FcFontSet *fontset;
  FcChar8 *family, *style, *prev_family;

  char title[TITLE_MAX];
  char fname[FILEPATH_MAX];
  int f;

  const char *title_lang;

  /* full name if known */
  title_lang = (const char *)lang_name(lang);
  if (strcmp(title_lang, (const char *)CONST_STRING_UNKNOWN) == 0)
    title_lang = (const char *)lang;

  fontset = language_font_index(lang, config);

  snprintf(title, TITLE_MAX, "Fonts in %s Supporting %s Language", 
           config.location, title_lang);
  fprintf(html, 
          "          <h1>%s</h1>\n", title);
  fprintf(html, 
          "          \n");

  prev_family = NULL;
  for (f = 0; f < fontset->nfont; f++)
  {
    assert(fcinfo_get_translated_string(fontset->fonts[f], FC_FAMILY, 
                                        LANG_EN, &family)
           == FcResultMatch);
    if (fcinfo_get_translated_string(fontset->fonts[f], FC_STYLE, 
                                     LANG_EN, &style)
           != FcResultMatch)
    {
      /* there exist fonts withoug style field defined,
       * ignore them https://github.com/pgajdos/fontinfo/issues/2 */
      f++;
      continue;
    }
    snprintf(fname, FILEPATH_MAX, "%s%s.html", family, style);
    remove_spaces_and_slashes(fname);

    if (prev_family)
    {
      if (toupper(prev_family[0]) != toupper(family[0]))
      {
        fprintf(html, 
                " </p>\n");
        fprintf(html, 
                "          <h2>%c</h2>\n", toupper(family[0]));
        fprintf(html, 
                "          <p> |");
      }
    }
    else
    {
      fprintf(html, 
              "          <h2>%c</h2>\n", toupper(family[0]));
      fprintf(html, 
              "          <p> |");
    }

    fprintf(html, " <a href=\"../%s/%s\">%s (%s)</a> |", 
            FONTS_SUBDIR, fname, family, style);
    prev_family = family;
  }

  fprintf(html,
          " </p>\n");

  FcFontSetDestroy(fontset);
  return;
}

void chameleon_language_fonts_indexes(config_t config)
{
  FcStrSet *strset;
  FcStrList *langs;
  FcChar8 *lang;

  char dirname[FILEPATH_MAX];
  char fname[FILEPATH_MAX];
  char title[TITLE_MAX];
  const char *title_lang;
  FILE *html;

  html_link path_links[] = 
     {{LANGUAGES_INDEX, LANGUAGES_INDEX_NAME".html", LANGUAGES_INDEX_DESC, 1}, 
      {NULL, NULL, NULL, 0}}; 
  const html_links path = {"../", path_links, ASIZE(path_links), ""} ;

  void *arg[1];

  snprintf(dirname, FILEPATH_MAX, "%s/%s", config.dir, LANGUAGES_SUBDIR);
  create_dir(dirname);

  strset = language_index(config);
  langs = FcStrListCreate(strset);

  while ((lang = FcStrListNext(langs)))
  { 
    /* full name if known */
    title_lang = (const char *)lang_name(lang);
    if (strcmp(title_lang, (const char *)CONST_STRING_UNKNOWN) == 0)
      title_lang = (const char *)lang;

    snprintf(title, TITLE_MAX, "Fonts in %s Supporting %s Language", 
             config.location, title_lang);

    snprintf(fname, FILEPATH_MAX, "%s/lang-%s.html", dirname, lang);
    html = open_write(fname, "chameleon_language_fonts_index");

    snprintf(fname, FILEPATH_MAX, "lang-%s.html", lang);
    remove_spaces_and_slashes(fname);
 
    path_links[1].text  = (char *)title_lang;
    path_links[1].href  = fname;
    path_links[1].title = "";
    path_links[1].active = 0;

    arg[0] = lang;
    chameleon_page(html, title, path, 
                   content_language_font_index, config, arg);

    fclose(html);
  }

  FcStrListDone(langs);
  FcStrSetDestroy(strset);
  return;
}

static void content_fontformats_index(FILE *html, config_t config,
                                       void *output_arg[])
{
  FcStrList *formats = (FcStrList *)output_arg[0];
  FcChar8 *format;
  FcChar8 *family;
  char family_ww[FAMILY_NAME_LEN_MAX];
  FcFontSet *familyset = NULL;
  int f;

  char title[TITLE_MAX];

  FcFontSet *styleset;
  FcChar8 *file;
  int s, have_bitmaps;
  face_info_t fi;
  FcBool scalable;

  snprintf(title, TITLE_MAX, "Font Formats in %s", config.location);
  fprintf(html, "    <h1>%s</h1>\n", title);
  fprintf(html, "    \n");
  while ((format = FcStrListNext(formats)))
  {
    fprintf(html, "    <h2>%s</h2>\n", format);
    fprintf(html, "    <p> | ");

    familyset = fontformat_family_index(format, config);

    for (f = 0; f < familyset->nfont; f++)
    {
      FcPatternGetString(familyset->fonts[f], FC_FAMILY, 0, &family);

      /* figure out if (some of style of) family contains bitmap strikes */
      styleset = styles_index(family, config, NULL);
      assert(FcPatternGetBool(styleset->fonts[0], FC_SCALABLE, 0, &scalable)
             == FcResultMatch);
      have_bitmaps = 0;
      if (scalable)
      {
        for (s = 0; s < styleset->nfont; s++)
        {
          FcPatternGetString(styleset->fonts[s], FC_FILE, 0, &file);
          ft_initialize_info(&fi, file);
          if (fi.num_fixed_sizes)
            have_bitmaps = 1;
          ft_free_info(&fi);
          if (have_bitmaps)
            break;
        }
      }
      FcFontSetDestroy(styleset);

      snprintf(family_ww, FAMILY_NAME_LEN_MAX, "%s", (const char *)family);
      remove_spaces_and_slashes(family_ww);
      fprintf(html, " <a href=%s/%s.html>%s</a>",
              FAMILIES_SUBDIR, family_ww, family);
      if (have_bitmaps)
        fprintf(html,  " <sup>[<a href=#bitmaps_note title=\""
                       FONT_HAVE_BITMAPS"\"><b>B</b></a>]</sup>");
      fprintf(html, " |");
    }

    fprintf(html, "    </p>\n");
  }

  fprintf(html, 
          "    <p id=\"bitmaps_note\">[<b>B</b>] "
                FONT_HAVE_BITMAPS"</p>\n");

  if (familyset)
    FcFontSetDestroy(familyset);

  return;
}

void chameleon_fontformats_index(config_t config)
{
  FILE * html;
  char fname[FILEPATH_MAX];
  FcStrSet *strset;
  FcStrList *formats;

  html_link path_links[] =
     {{FONTFORMATS_INDEX, FONTFORMATS_INDEX_NAME".html", FONTFORMATS_INDEX_DESC, 0}};
  html_links path = {"", path_links, ASIZE(path_links), ""};

  void *arg[1];

  snprintf(fname, FILEPATH_MAX, "%s/"FONTFORMATS_INDEX_NAME".html", config.dir);

  html = open_write(fname, "chameleon_fontformats_index");

  strset = fontformats(config);
  formats = FcStrListCreate(strset);

  arg[0] = formats;
  chameleon_page(html, FONTFORMATS_INDEX, path,
             content_fontformats_index, config, arg);

  fclose(html);
  FcStrListDone(formats);
  FcStrSetDestroy(strset);
  return;
}

static void content_scripts_index(FILE *html, config_t config,
                                  void *output_arg[])
{
  const char **supported_scripts = (const char **)output_arg[0];
  int sup;
  int coll_section;

  char title[TITLE_MAX];
  char script_wu[SCRIPT_NAME_LEN_MAX];

  snprintf(title, TITLE_MAX, "Scripts Supported by %s", config.location);
  fprintf(html, "    <h1>%s</h1>\n", title);
  fprintf(html, "    \n");
  sup = 0;
  coll_section = 0;
  while (supported_scripts[sup])
  {
    if (sup > 0)
    {
      if (!script_is_collection(supported_scripts[sup-1]) &&
           script_is_collection(supported_scripts[sup]))
      {
        fprintf(html, " </p>\n");
        fprintf(html, "    <h1 id=\"collections\">"COLLECTIONS"</h1>\n");
        fprintf(html, "    <p> |");
        coll_section = 1;
      }

      if (!coll_section && 
          supported_scripts[sup-1][0] != supported_scripts[sup][0])
      {
        fprintf(html, " </p>\n");
        fprintf(html, "    <h2>%c</h2>\n", supported_scripts[sup][0]);
        fprintf(html, "    <p> |");
      }
    }
    else if (script_is_collection(supported_scripts[sup]))
    {
      /* there are only collections (unlikely even for only one font) */
      fprintf(html, "    <h1 id=\"collections\">"COLLECTIONS"</h1>\n");
      fprintf(html, "    <p> |");
      coll_section = 1;
    }
    else if (!coll_section)
    {
      fprintf(html, "    <h2>%c</h2>\n", supported_scripts[sup][0]);
      fprintf(html, "    <p> |");
    }

    snprintf(script_wu, SCRIPT_NAME_LEN_MAX, "%s", supported_scripts[sup]);
    underscores_to_spaces(script_wu);
    fprintf(html, " <a href=\"%s/script-%s.html\">%s</a> |",
            SCRIPTS_SUBDIR, supported_scripts[sup], script_wu);
    sup++;
  }
  fprintf(html, " </p>\n");
  return;
}

void chameleon_scripts_index(config_t config)
{
  FILE * html;
  char fname[FILEPATH_MAX];
  double *script_stat[NUMSCRIPTS];
  FcFontSet *fontset;
  const char *supported_scripts[NUMSCRIPTS + 1];
  char *script;
  int s, sup, f;

  const html_link path_links[] =
     {{SCRIPTS_INDEX, SCRIPTS_INDEX_NAME".html", LANGUAGES_INDEX_DESC, 0}};
  const html_links path = {"", path_links, ASIZE(path_links), ""};

  void *arg[1];

  snprintf(fname, FILEPATH_MAX, "%s/"SCRIPTS_INDEX_NAME".html", config.dir);

  html = open_write(fname, "chameleon_scripts_index");

  fontset = uinterval_statistics(config, script_stat, SCRIPT);

  bzero(supported_scripts, sizeof(const char *)*(NUMSCRIPTS + 1));
  sup = 0;
  for (s = 0; s < NUMSCRIPTS; s++)
  {
    script = (char *)unicode_script_name(s);
    for (f = 0; f < fontset->nfont; f++)
    {
      if (script_stat[s][f] >= SCRIPTS_COVERAGE_MIN)
      {
        supported_scripts[sup++] = script;
        break;
      }
    }
  }

  arg[0] = supported_scripts;
  chameleon_page(html, SCRIPTS_INDEX, path,
             content_scripts_index, config, arg);

  fclose(html);
  for (s = 0; s < NUMSCRIPTS; s++)
    free(script_stat[s]);
  FcFontSetDestroy(fontset);
  return;
}

static void content_script_font_index(FILE *html, config_t config,
                                      void *output_arg[])
{
  FcPattern **ranking = (FcPattern **)output_arg[0];
  double *svalues = (double *)output_arg[1];
  int *nranks = (int *)output_arg[2];
  int *script_idx = (int *)output_arg[3];
  FcChar8 *family, *style;
  int ten = -1;

  char title[TITLE_MAX];
  char fname[FILEPATH_MAX];
  int f;

  const char *blocks[SCRIPT_BLOCKS_MAX];
  int lbounds[SCRIPT_BLOCKS_MAX];
  int ubounds[SCRIPT_BLOCKS_MAX];
  int nblocks, b;
  char block_ww[BLOCK_NAME_LEN_MAX];

  const char *script = unicode_script_name(*script_idx);
  char script_wu[SCRIPT_NAME_LEN_MAX];

  snprintf(script_wu, SCRIPT_NAME_LEN_MAX, "%s", script);
  underscores_to_spaces(script_wu); 
  snprintf(title, TITLE_MAX, "Fonts in %s Supporting %s %s",
           config.location, script_wu,
           script_is_collection(script) ? COLLECTION : "Script");
  fprintf(html, "    <h1>%s</h1>\n", title);
  fprintf(html, "    \n");

  if (!script_is_collection(script))
  {
    nblocks = unicode_script_blocks(script, blocks, lbounds, ubounds);
    fprintf(html, "    <div class=\"table-responsive\">\n");
    fprintf(html, "    <table class=\"table\">\n");
    for (b = 0; b < nblocks; b++)
    {
      snprintf(block_ww, BLOCK_NAME_LEN_MAX, "%s", blocks[b]);
      remove_spaces_and_slashes(block_ww);
      fprintf(html, 
              "      <tr>"
              "<td style=\"font-family: monospace;\">U+%05X - U+%05X</td>"
              "<td><a href=\"../"BLOCKS_SUBDIR"/block-%s.html\">%s</a></td>"
              "</tr>\n",
              lbounds[b], ubounds[b], block_ww, blocks[b]);
    }
    fprintf(html, "    </table>\n");
    fprintf(html, "    </div>\n");
    fprintf(html, "    <br\>\n");
    fprintf(html, "    \n");
  }

  for (f = 0; f < *nranks; f++)
  {
    assert(fcinfo_get_translated_string(ranking[f], FC_FAMILY,
                                        LANG_EN, &family)
           == FcResultMatch);
    if (fcinfo_get_translated_string(ranking[f], FC_STYLE,
                                     LANG_EN, &style)
           != FcResultMatch)
    {
      /* there exist fonts withoug style field defined,
       * ignore them https://github.com/pgajdos/fontinfo/issues/2 */
      f++;
      continue;
    }
    snprintf(fname, FILEPATH_MAX, "%s%s.html", family, style);
    remove_spaces_and_slashes(fname);

    if (ten != -1)
    {
      if ((int)svalues[f]/10 != ten)
      {
        ten = (int)svalues[f]/10;
        fprintf(html, " </p>\n");
        if (ten > 0)
          fprintf(html, "    <h2>%d</h2>\n", ten*10);
        else
          fprintf(html, "    <h2>Below 10</h2>\n");
        fprintf(html, "    <p> |");
      }
    }
    else
    {
      ten = (int)svalues[f]/10;
      if (ten > 0)
        fprintf(html, "    <h2>%d</h2>\n", ten*10);
      else
        fprintf(html, "    <h2>Below 10</h2>\n");
      fprintf(html, "    <p> |");
    }

    /* do not round with printf, just cut off not significant decimals */
    fprintf(html, " <a href=\"../%s/%s\">%s (%s)</a> [%.1f%%] |",
            FONTS_SUBDIR, fname, family, style, ((int)(svalues[f]*10.0))/10.0);
  }

  fprintf(html, "    </p>\n");
  return;
}

void chameleon_script_fonts_indexes(config_t config)
{
  double *script_stat[NUMSCRIPTS];
  FcFontSet *fontset;
  FcPattern **ranking;
  double *svalues;
  int s, nranks, v, v2, f;
  char *script;

  char dirname[FILEPATH_MAX];
  char fname[FILEPATH_MAX];
  char title[TITLE_MAX];
  FILE *html;

   html_link path_links[] = 
     {{SCRIPTS_INDEX, SCRIPTS_INDEX_NAME".html", SCRIPTS_INDEX_DESC, 1}, 
      {NULL, NULL, NULL, 0}}; 
  const html_links path = {"../", path_links, ASIZE(path_links), ""};

  void *arg[4];

  snprintf(dirname, FILEPATH_MAX, "%s/%s", config.dir, SCRIPTS_SUBDIR);
  create_dir(dirname);

  fontset = uinterval_statistics(config, script_stat, SCRIPT);
  ranking = malloc(sizeof(FcPattern*)*fontset->nfont);
  svalues = malloc(sizeof(double)*fontset->nfont);
  if (!ranking || !svalues)
  {
    fprintf(stderr, "chameleon_script_fonts_indexes(): out of memory\n");
    exit(1);
  }

  bzero(ranking, sizeof(FcPattern*)*fontset->nfont);

  for (s = 0; s < NUMSCRIPTS; s++)
  {
    script = (char *)unicode_script_name(s);

    nranks = 0;
    /* sort fonts according coverage svalues of the script */
    for (f = 0; f < fontset->nfont; f++)
    {
      if (script_stat[s][f] < SCRIPTS_COVERAGE_MIN)
        continue;

      v = 0;
      while (v < nranks)
      {
        if (script_stat[s][f] > svalues[v])
          break;
        v++;
      }
      for (v2 = nranks; v2 > v; v2--)
      {
        ranking[v2] = ranking[v2 - 1];
        svalues[v2] = svalues[v2 - 1];
      }

      /*fprintf(stdout, "nranks = %d v = %d f = %d value = %.1f%%\n", nranks, v, f, script_stat[s][f]);*/
      ranking[v] = fontset->fonts[f];
      svalues[v] = script_stat[s][f];

      nranks++;
    }

    if (nranks == 0)
      continue; /* there's no font that would have this script */
                /* coverage >= SCRIPTS_COVERAGE_MIN */

    snprintf(title, TITLE_MAX, "Font Ranks for %s (%s)",
             script, config.location);

    snprintf(fname, FILEPATH_MAX, "%s/script-%s.html",
             dirname, script);
    html = open_write(fname, "chameleon_script_fonts_indexes");

    path_links[1].text  = (char *)script;
    path_links[1].href  = fname;
    path_links[1].title = "";
    path_links[1].active = 0;

    arg[0] = ranking; arg[1] = svalues; arg[2] = &nranks; arg[3] = &s;
    chameleon_page(html, title, path, 
                   content_script_font_index, config, arg);
    fclose(html);
  }

  for (s = 0; s < NUMSCRIPTS; s++)
    free(script_stat[s]);
  free(ranking);
  free(svalues);
  FcFontSetDestroy(fontset);
  return;
}

static void content_raw_html_text(FILE *html, config_t config,
                              void *output_arg[])
{
  const char **lines = (const char **)output_arg[0];
  const char *indent = (const char *)output_arg[1];
  int l;

  l = 0;
  while (lines[l])
    fprintf(html, "%s%s\n", indent, lines[l++]);

  return;
}

void chameleon_thanks(config_t config)
{
  FILE * html;
  char fname[FILEPATH_MAX];

  const html_link path_links[] =
     {{THANKS, "thanks.html", THANKS_DESC, 0}};
  const html_links path = {"", path_links, ASIZE(path_links), ""};

  void *arg[2];

  snprintf(fname, FILEPATH_MAX, "%s/thanks.html", config.dir);

  html = open_write(fname, "chameleon_thanks");

  arg[0] = thanks_html_text();
  arg[1] = "    ";
  chameleon_page(html, THANKS, path,
                 content_raw_html_text, config, arg);

  fclose(html);
  return;
}

static void content_blocks_index(FILE *html, config_t config,
                                 void *output_arg[])
{
  uinterval_map_t **supported_blocks = (uinterval_map_t **)output_arg[0];
  int sup;

  char title[TITLE_MAX];
  char block_ww[BLOCK_NAME_LEN_MAX];

  snprintf(title, TITLE_MAX,
           "Unicode Blocks Supported by %s", config.location);
  fprintf(html, "    <h1>%s</h1>\n", title);
  fprintf(html, "    \n");
  fprintf(html, "    <div class=\"table-responsive\">\n");
  fprintf(html, "    <table class=\"table\">\n");
  sup = 0;
  while (supported_blocks[sup])
  {
    snprintf(block_ww, BLOCK_NAME_LEN_MAX, "%s",
             supported_blocks[sup]->interval_name);
    remove_spaces_and_slashes(block_ww);
    fprintf(html, "      <tr><td style=\"font-family: monospace;\">"
                  "U+%05X - U+%05X</td><td><a href=\"%s/block-%s.html\">"
                  "%s</a></td></tr>\n",
                  supported_blocks[sup]->l,
                  supported_blocks[sup]->u,
                  BLOCKS_SUBDIR,
                  block_ww,
                  supported_blocks[sup]->interval_name);
    sup++;
  }
  fprintf(html, "    </table>\n");
  fprintf(html, "    </div>\n");
  return;
}

void chameleon_blocks_index(config_t config)
{
  FILE * html;
  char fname[FILEPATH_MAX];
  double *block_stat[NUMBLOCKS];
  FcFontSet *fontset;
  const uinterval_map_t *supported_blocks[NUMBLOCKS + 1];
  int b, sup, f;

  const html_link path_links[] =
     {{BLOCKS_INDEX, BLOCKS_INDEX_NAME".html", BLOCKS_INDEX_DESC, 0}};
  const html_links path = {"", path_links, ASIZE(path_links), ""};

  void *arg[1];

  snprintf(fname, FILEPATH_MAX, "%s/"BLOCKS_INDEX_NAME".html", config.dir);

  html = open_write(fname, "plain_blocks_index");

  fontset = uinterval_statistics(config, block_stat, BLOCK);

  bzero(supported_blocks, sizeof(const uinterval_map_t *)*(NUMBLOCKS + 1));
  sup = 0;
  for (b = 0; b < NUMBLOCKS; b++)
  {
    for (f = 0; f < fontset->nfont; f++)
      if (block_stat[b][f] >= BLOCKS_COVERAGE_MIN)
      {
        supported_blocks[sup++] = &block_map_consts[b];
        break;
      }
  }
  arg[0] = supported_blocks; 
  chameleon_page(html, BLOCKS_INDEX, path,
                 content_blocks_index, config, arg);

  fclose(html);
  for (b = 0; b < NUMBLOCKS; b++)
    free(block_stat[b]);
  FcFontSetDestroy(fontset);
  return;
}

static void content_block_font_index(FILE *html, config_t config,
                                     void *output_arg[])
{
  FcPattern **ranking = (FcPattern **)output_arg[0];
  double *svalues = (double *)output_arg[1];
  int *nranks = (int *)output_arg[2];
  int *block_idx = (int *)output_arg[3];
  FcChar8 *family, *style;
  int ten = -1;

  char title[TITLE_MAX];
  char fname[FILEPATH_MAX];
  int f;

  const char *block = unicode_block_name(*block_idx);

  snprintf(title, TITLE_MAX, "Fonts in %s Supporting %s Block",
           config.location, block);
  fprintf(html, "    <h1>%s</h1>\n", title);
  fprintf(html, "    \n");

  for (f = 0; f < *nranks; f++)
  {
    assert(fcinfo_get_translated_string(ranking[f], FC_FAMILY,
                                        LANG_EN, &family)
           == FcResultMatch);
    if (fcinfo_get_translated_string(ranking[f], FC_STYLE,
                                     LANG_EN, &style)
           != FcResultMatch)
    {
      /* there exist fonts withoug style field defined,
       * ignore them https://github.com/pgajdos/fontinfo/issues/2 */
      f++;
      continue;
    }
    snprintf(fname, FILEPATH_MAX, "%s%s.html", family, style);
    remove_spaces_and_slashes(fname);

    if (ten != -1)
    {
      if ((int)svalues[f]/10 != ten)
      {
        ten = (int)svalues[f]/10;
        fprintf(html, " </p>\n");
        if (ten > 0)
          fprintf(html, "    <h2>%d</h2>\n", ten*10);
        else
          fprintf(html, "    <h2>Below 10</h2>\n");
        fprintf(html, "    <p> |");
      }
    }
    else
    {
      ten = (int)svalues[f]/10;
      if (ten > 0)
        fprintf(html, "    <h2>%d</h2>\n", ten*10);
      else
        fprintf(html, "    <h2>Below 10</h2>\n");
      fprintf(html, "    <p> |");
    }

    /* do not round with printf, just cut off not significant decimals */
    fprintf(html, " <a href=\"../%s/%s\">%s (%s)</a> [%.1f%%] |",
            FONTS_SUBDIR, fname, family, style, ((int)(svalues[f]*10.0))/10.0);
  }

  fprintf(html, "    </p>\n");
  return;
}

void chameleon_block_fonts_indexes(config_t config)
{
  double *block_stat[NUMBLOCKS];
  FcFontSet *fontset;
  FcPattern **ranking;
  double *svalues;
  int b, nranks, v, v2, f;
  char *block;
  char block_ww[BLOCK_NAME_LEN_MAX];

  char dirname[FILEPATH_MAX];
  char fname[FILEPATH_MAX];
  char title[TITLE_MAX];
  FILE *html;

  html_link path_links[] =
     {{BLOCKS_INDEX, BLOCKS_INDEX_NAME".html", BLOCKS_INDEX_DESC, 1},
      {NULL, NULL, NULL, 0}};
  const html_links path = {"../", path_links, ASIZE(path_links), ""};

  void *arg[4];

  snprintf(dirname, FILEPATH_MAX, "%s/%s", config.dir, BLOCKS_SUBDIR);
  create_dir(dirname);

  fontset = uinterval_statistics(config, block_stat, BLOCK);
  ranking = malloc(sizeof(FcPattern*)*fontset->nfont);
  svalues = malloc(sizeof(double)*fontset->nfont);
  if (!ranking || !svalues)
  {
    fprintf(stderr, "plain_block_fonts_indexes(): out of memory\n");
    exit(1);
  }

  for (b = 0; b < NUMBLOCKS; b++)
  {
    block = (char *)unicode_block_name(b);
    snprintf(block_ww, BLOCK_NAME_LEN_MAX, "%s", block);
    remove_spaces_and_slashes(block_ww);

    nranks = 0;
    /* sort fonts according coverage svalues of the block */
    for (f = 0; f < fontset->nfont; f++)
    {
      if (block_stat[b][f] < BLOCKS_COVERAGE_MIN)
        continue;

      v = 0;
      while (v < nranks)
      {
        if (block_stat[b][f] > svalues[v])
          break;
        v++;
      }
      for (v2 = nranks; v2 > v; v2--)
      {
        ranking[v2] = ranking[v2 - 1];
        svalues[v2] = svalues[v2 - 1];
      }

      ranking[v] = fontset->fonts[f];
      svalues[v] = block_stat[b][f];
      nranks++;
    }

    if (nranks == 0)
      continue; /* there's no font that would have this block */
                /* coverage >= BLOCKS_COVERAGE_MIN */

    snprintf(title, TITLE_MAX, "Unicode Blocks in %s",
             config.location);

    snprintf(fname, FILEPATH_MAX, "%s/block-%s.html",
             dirname, block_ww);
    html = open_write(fname, "plain_block_fonts_indexes");

    path_links[1].text  = (char *)block;
    path_links[1].href  = fname;
    path_links[1].title = "";
    path_links[1].active = 0;

    arg[0] = ranking; arg[1] = svalues; arg[2] = &nranks; arg[3] = &b;
    chameleon_page(html, title, path,
               content_block_font_index, config, arg);
    fclose(html);
  }

  for (b = 0; b < NUMBLOCKS; b++)
    free(block_stat[b]);
  free(ranking);
  free(svalues);
  FcFontSetDestroy(fontset);
  return;
}

