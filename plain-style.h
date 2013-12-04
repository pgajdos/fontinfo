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

#ifndef PLAIN_STYLE_H
# define PLAIN_STYLE_H

# include "configuration.h"

void plain_families_index(config_t config);
void plain_families_partial_indexes(config_t config);
void plain_family_styles_indexes(config_t config);
void plain_font_cards(config_t config);
void plain_languages_index(config_t config);
void plain_language_fonts_indexes(config_t config);
void plain_fontformats_index(config_t config);
void plain_scripts_index(config_t config);
void plain_script_fonts_indexes(config_t config);
void plain_blocks_index(config_t config);
void plain_block_fonts_indexes(config_t config);

#endif
