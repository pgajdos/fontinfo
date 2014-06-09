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
#include <unistd.h>

#include "configuration.h"
#include "filesystem.h"
#include "plain-style.h"
#include "bento-style.h"
#include "constants.h"

#define DEFAULT_LOCATION   "localhost" /* only for heading for Index of Families */

#define DEFAULT_DIR     "html"

#define SVG_OUTPUT      "svg"
#define PNG_OUTPUT      "png"
#define DEFAULT_OUTPUT  PNG_OUTPUT

#define PLAIN_STYLE     "plain"
#define BENTO_STYLE     "bento"
#define DEFAULT_STYLE   PLAIN_STYLE

#define ON              "on"
#define OFF             "off"

#define DEFAULT_SPECIMEN_FROM_PX     7
#define DEFAULT_SPECIMEN_TO_PX       25
#define DEFAULT_USE_HARFBUZZ         1
#define DEFAULT_SPECIMEN_SCRIPT      "Latin"
#define DEFAULT_SPECIMEN_LANG        "la"
#define DEFAULT_OUTLINE_SPECIMEN     1
/* no command line option for minispecimen_pxsize yet */
#define DEFAULT_MINISPECIMEN_PXSIZE  16
#define DEFAULT_MINISPECIMEN_SCRIPT  "Latin"
#define DEFAULT_GENERATE_CHARSET     1
#define DEFAULT_GENERATE_CTOOLTIPS   1
#define DEFAULT_GENERATE_STOOLTIPS   1
#define DEFAULT_CHARSET_PXSIZE       20
#define DEFAULT_PATTERN_STRING       ":"
#define DEFAULT_GENERATE_INDEXES     1
#define DEFAULT_GENERATE_SWPKGINFO   1

#define NONE_INSTALL              "none"
#define REPO_INSTALL              "repo"
#define YMP_INSTALL               "ymp" 
#define DEFAULT_INSTALL           NONE_INSTALL
#define DEFAULT_INSTALL_WHEN_REPO REPO_INSTALL

#define L2R_SPECIMEN_TEXTDIR      "l2r"
#define R2L_SPECIMEN_TEXTDIR      "r2l"
#define T2B_SPECIMEN_TEXTDIR      "t2b"
#define B2T_SPECIMEN_TEXTDIR      "b2t"
#define DEFAULT_SPECIMEN_TEXTDIR  L2R_SPECIMEN_TEXTDIR

fontoutput_t strtofontoutput(const char *str)
{
  if (!strcmp(str, PNG_OUTPUT))
    return PNG;
  else if (!strcmp(str, SVG_OUTPUT)) 
    return SVG;
  else
    return UNKNOWN;
}

installtype_t strtoinstalltype(const char *str)
{
  if (!strcmp(str, NONE_INSTALL))
    return NONE;
  if (!strcmp(str, REPO_INSTALL))
    return REPO;
  if (!strcmp(str, YMP_INSTALL))
    return YMP;
  else
    return WRONG;
}

int strtospecimentextdir(const char *str)
{
  if (!strcmp(str, L2R_SPECIMEN_TEXTDIR))
    return 0; /* enum is defined in constants.h yet */
  if (!strcmp(str, R2L_SPECIMEN_TEXTDIR))
    return 1;
  if (!strcmp(str, T2B_SPECIMEN_TEXTDIR))
    return 2;
  if (!strcmp(str, B2T_SPECIMEN_TEXTDIR))
    return 3;
  else
    return -1;
}

void usage(const char *err)
{
  if (err)
    fprintf(stderr, "ERROR: %s\n\n", err);
  fprintf(stderr, "Usage: fontinfo [-option1 value1] [-option2 value2] ... [-optionN valueN]\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "       Generates html font overview of installed \n");
  fprintf(stderr, "       fonts.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "       -g            output debug information\n");
  fprintf(stderr, "       -d string:    place all generated pages in dir ["DEFAULT_DIR"]\n");
  fprintf(stderr, "       -o string:    type of the font text output (specimens, charset) [%s]\n", DEFAULT_OUTPUT);
  fprintf(stderr, "                     possible values: "SVG_OUTPUT","PNG_OUTPUT"\n");
  fprintf(stderr, "       -b bool:      use harfbuzz for specimen rendering [%s]\n", DEFAULT_USE_HARFBUZZ == 0 ? OFF : ON);
  fprintf(stderr, "                     possible values: "ON", "OFF"\n");
  fprintf(stderr, "       -s string:    enforce this specimen sentence (for details see -D, -S, -L) [no sentence]\n");
  fprintf(stderr, "       -D string:    enforced specimen sentence should be rendered in this direction [%s]\n", DEFAULT_SPECIMEN_TEXTDIR);
  fprintf(stderr, "                     possible values: "L2R_SPECIMEN_TEXTDIR", "R2L_SPECIMEN_TEXTDIR","
                                        T2B_SPECIMEN_TEXTDIR","B2T_SPECIMEN_TEXTDIR"\n");
  fprintf(stderr, "       -S string:    enforced specimen sentence script [%s]\n", DEFAULT_SPECIMEN_SCRIPT);
  fprintf(stderr, "       -L string:    enforced specimen sentence language [%s]\n", DEFAULT_SPECIMEN_LANG);
  fprintf(stderr, "                     (vertical not implemented yet!)\n");
  fprintf(stderr, "       -f int:       specimen: from pixel size [%d]\n", DEFAULT_SPECIMEN_FROM_PX);
  fprintf(stderr, "       -t int:       specimen: to pixel size [%d]\n", DEFAULT_SPECIMEN_TO_PX);
  fprintf(stderr, "       -m string:    mini specimen: prefered script for specimen sentence [%s]\n", DEFAULT_MINISPECIMEN_SCRIPT);
  fprintf(stderr, "       -a bool:      along bitmap specimen, display also outline specimen (to see how the font\n");
  fprintf(stderr, "                     renderes on the client system) [%s]\n", DEFAULT_OUTLINE_SPECIMEN == 0 ? OFF : ON);
  fprintf(stderr, "                     possible values: "ON", "OFF"\n");
  fprintf(stderr, "       -c bool:      display charset [%s]\n", DEFAULT_GENERATE_CHARSET == 0 ? OFF : ON);
  fprintf(stderr, "                     possible values: "ON", "OFF"\n");
  fprintf(stderr, "       -u bool:      display tooltips in charset including\n");
  fprintf(stderr, "                     unicode character number [%s]\n", DEFAULT_GENERATE_CTOOLTIPS == 0 ? OFF : ON);
  fprintf(stderr, "       -z bool:      display tooltips in specimen (fontsize) [%s]\n", DEFAULT_GENERATE_STOOLTIPS == 0 ? OFF : ON);
  fprintf(stderr, "                     possible values: "ON", "OFF"\n");
  fprintf(stderr, "       -x int:       charset pixel size [%d]\n", DEFAULT_CHARSET_PXSIZE);
  fprintf(stderr, "       -p string:    limit output only to pattern [%s]\n", DEFAULT_PATTERN_STRING);
  fprintf(stderr, "       -l string:    describe where you take fonts from, \n");
  fprintf(stderr, "                     e. g. localhost, M17N:fonts, etc. [%s]\n", DEFAULT_LOCATION);
  fprintf(stderr, "       -i bool:      generate indexes [%s]\n", DEFAULT_GENERATE_INDEXES == 0 ? OFF : ON);
  fprintf(stderr, "                     possible values: "ON", "OFF"\n");
  fprintf(stderr, "       -y string:    html style used [%s]\n", DEFAULT_STYLE);
  fprintf(stderr, "                     possible values: "PLAIN_STYLE", "BENTO_STYLE"\n");
  fprintf(stderr, "       -w bool:      generate software package information [%s]\n", DEFAULT_GENERATE_SWPKGINFO == 0 ? OFF : ON);
  fprintf(stderr, "                     possible values: "ON", "OFF"\n");
  fprintf(stderr, "       -n string:    generate links to install software packages [%s]\n", DEFAULT_INSTALL);
  fprintf(stderr, "                     possible values: "NONE_INSTALL", "REPO_INSTALL", "YMP_INSTALL"\n");
  fprintf(stderr, "       -r repo,link: repository definition: repo: repository name, link: remote directory\n");
  fprintf(stderr, "                     where packages reside; maximum %d repositories can be defined;\n", REPOSITORIES_MAX);
  fprintf(stderr, "                     -r implies -n "DEFAULT_INSTALL_WHEN_REPO", if not specified otherwise; for "YMP_INSTALL", "
                                        "repo has to be exact product version\n");
  fprintf(stderr, "                     example: \n");
  fprintf(stderr, "                      -r 'openSUSE 12.3,http://download.opensuse.org/repositories/M17N:/fonts/openSUSE_12.3/'\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "       Running fontinfo without any parameter will result\n");
  fprintf(stderr, "       in full information fontinfo is able to produce\n");
  fprintf(stderr, "       on the system it is called.\n");
  return;
}

void create_config(int argc, char *argv[], config_t *config)
{
  int opt;

  /* defaults */
  config->specimen_type = 
    config->charset_type = strtofontoutput(DEFAULT_OUTPUT);
  config->debug = 0;
  config->dir = DEFAULT_DIR;
  config->specimen_sentence = NULL;
  config->specimen_textdir = strtospecimentextdir(DEFAULT_SPECIMEN_TEXTDIR);
  config->specimen_script = DEFAULT_SPECIMEN_SCRIPT;
  config->specimen_lang = DEFAULT_SPECIMEN_LANG;
  config->use_harfbuzz = DEFAULT_USE_HARFBUZZ;
  config->specimen_from_px = DEFAULT_SPECIMEN_FROM_PX;
  config->specimen_to_px = DEFAULT_SPECIMEN_TO_PX;
  config->minispecimen_pxsize = DEFAULT_MINISPECIMEN_PXSIZE;
  config->minispecimen_script = DEFAULT_MINISPECIMEN_SCRIPT;
  config->outline_specimen = DEFAULT_OUTLINE_SPECIMEN;
  config->generate_charset = DEFAULT_GENERATE_CHARSET;
  config->generate_ctooltips = DEFAULT_GENERATE_CTOOLTIPS;
  config->generate_stooltips = DEFAULT_GENERATE_STOOLTIPS;
  config->charset_pxsize = DEFAULT_CHARSET_PXSIZE;
  config->pattern_string = DEFAULT_PATTERN_STRING;
  config->location = DEFAULT_LOCATION;
  config->generate_indexes = DEFAULT_GENERATE_INDEXES;
  config->style = DEFAULT_STYLE;
  config->generate_swpkginfo = DEFAULT_GENERATE_SWPKGINFO;
  config->install_type = strtoinstalltype(DEFAULT_INSTALL);
  config->nrepositories = 0;

  while ((opt = getopt(argc, argv, "o:d:s:f:t:x:p:hl:c:i:u:y:w:z:a:n:r:D:S:L:m:gb:")) != -1) 
  {
    switch (opt) 
    {
      case 'o':
          if ((config->specimen_type = 
               config->charset_type = strtofontoutput(optarg)) == UNKNOWN)
          { usage("type of the font text output is not correct"); 
            exit(1); }
          break;
      case 'd':
         config->dir = optarg;
         break;
      case 'b':
         config->use_harfbuzz = strcmp(optarg, OFF) ? 1 : 0;
         break;
      case 's':
         config->specimen_sentence = optarg;
         break;
      case 'D':
         if ((config->specimen_textdir = strtospecimentextdir(optarg)) == -1)
           {usage("wrong specimen text direction"); 
            exit(1); }
         break;
      case 'S':
         config->specimen_sentence = optarg;
         break;
      case 'L':
         config->specimen_sentence = optarg;
         break;
      case 'f':
         config->specimen_from_px = atoi(optarg); 
         if (config->specimen_from_px <= 0)
           {usage("specimen from pixel size has to be positive integer"); 
            exit(1); }
         break;
      case 't':
         config->specimen_to_px = atoi(optarg);
         if (config->specimen_to_px <= 0)
           {usage("specimen to pixel size has to be positive integer"); 
            exit(1); }
         break;
      case 'm':
         if (!unicode_script_exists(config->minispecimen_script = optarg))
         {usage("unicode script for minispecimen doesn't exist");
          exit(1);}
         break;
      case 'a':
         config->outline_specimen = strcmp(optarg, OFF) ? 1 : 0;
         break;
      case 'c':
         config->generate_charset = strcmp(optarg, OFF) ? 1 : 0;
         break;
      case 'u':
         config->generate_ctooltips = strcmp(optarg, OFF) ? 1 : 0;
         break;
      case 'z':
         config->generate_stooltips = strcmp(optarg, OFF) ? 1 : 0;
         break;
      case 'x':
         config->charset_pxsize = atoi(optarg);
         if (config->charset_pxsize <= 0)
           {usage("charset pixelsize has to be positive integer"); 
            exit(1); }
         break;
      case 'p':
         config->pattern_string = optarg;
         break;
      case 'l':
         config->location = optarg;
         break;
      case 'i':
         config->generate_indexes = strcmp(optarg, OFF) ? 1 : 0;
         break;
      case 'y':
         config->style = optarg;
         break;
      case 'w':
         config->generate_swpkginfo = strcmp(optarg, OFF) ? 1 : 0;
         break;
      case 'n':
         if ((config->install_type = strtoinstalltype(optarg)) == WRONG)
         { usage("install type incorrect"); exit(1); }
         break;
      case 'r':
         if (config->nrepositories == REPOSITORIES_MAX)
         { usage("maximum number of repositories exceeded"); exit(1); }
         if ((config->repositories[config->nrepositories][1] = strchr(optarg, ',')) == NULL)
         { usage("wrong syntax in repository definition"); exit(1); }
         *config->repositories[config->nrepositories][1] = '\0';
         config->repositories[config->nrepositories][1]++;
         config->repositories[config->nrepositories][0] = optarg;
         config->nrepositories++;
         if (config->install_type == NONE)
           config->install_type = strtoinstalltype(DEFAULT_INSTALL_WHEN_REPO);
         break;
      case 'g':
         config->debug = 1;
         break;
      case 'h':
      default: 
        usage(NULL); 
        exit(1);
    }

    if (config->specimen_from_px > config->specimen_to_px)
      { usage("specimen from pixel size is greater than specimen to pixel size"); 
        exit(1); }
  }

  return;
}

int main(int argc, char *argv[])
{
  config_t config;

  create_config(argc, argv, &config);
  create_dir(config.dir);

  if (! strcmp(config.style, PLAIN_STYLE))
  {
    plain_font_cards(config);
    if (config.generate_indexes)
    {
      plain_families_index(config);
      plain_families_detailed_indexes(config);
      plain_family_styles_indexes(config);
      plain_languages_index(config);
      plain_language_fonts_indexes(config);
      plain_fontformats_index(config);
      plain_scripts_index(config);
      plain_script_fonts_indexes(config);
      plain_thanks(config);
      plain_blocks_index(config);
      plain_block_fonts_indexes(config);
    }
  } 
  else if (! strcmp(config.style, BENTO_STYLE))
  {
    bento_font_cards(config);
    if (config.generate_indexes)
    {
      bento_families_index(config);
      bento_families_detailed_indexes(config);
      bento_family_styles_indexes(config);
      bento_languages_index(config);
      bento_language_fonts_indexes(config);
      bento_fontformats_index(config);
      bento_scripts_index(config);
      bento_script_fonts_indexes(config);
      bento_thanks(config);
      bento_blocks_index(config);
      bento_block_fonts_indexes(config);
    }
  }
  else
  {
    usage("choosen wrong style name");
    exit(1);
  }

  return 0;
}

