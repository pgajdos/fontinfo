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

#ifndef CONFIGURATION_H
# define CONFIGURATION_H

typedef enum
{
  SVG,
  PNG,
  UNKNOWN
} fontoutput_t;

typedef enum
{
  YMP,
  REPO,
  NONE,
  WRONG
} installtype_t;

#define REPOSITORIES_MAX  16

typedef struct
{
  fontoutput_t specimen_type;
  fontoutput_t charset_type;
  int debug; /* 1 yes, 0 no */
  const char *dir;
  const char *specimen_sentence;
  int specimen_textdir; /* 0 L2R, 1 R2L, 2 T2B, 3 B2T */
  const char *specimen_script;
  const char *specimen_lang;
  int specimen_from_px;
  int specimen_to_px;
  int minispecimen_pxsize;
  const char *minispecimen_script;
  int outline_specimen; /* 1 yes, 0 no */
  int generate_charset; /* 1 yes, 0 no */
  int generate_ctooltips; /* 1 yes, 0 no */
  int generate_stooltips; /* 1 yes, 0 no */
  int charset_pxsize;
  const char *pattern_string;
  const char *location;
  int generate_indexes; /* 1 yes, 0 no */
  const char *style;
  int generate_swpkginfo;
  installtype_t install_type;
  int nrepositories;
  char *repositories[REPOSITORIES_MAX][2];
  int use_harfbuzz; /* 1 yes, 0 no */
} config_t;

#endif
