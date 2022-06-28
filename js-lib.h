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

#ifndef JS_LIB_H
# define JS_LIB_H

# define JS_SUBDIR  "js-lib"

#include "configuration.h"

void js_write_script_specimen_view(config_t config, const char *js_fname);
void js_write_script_charset_view(config_t config, const char *js_fname);
void js_write_script_family_search(config_t config, const char *js_fname);

#endif
