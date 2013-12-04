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

#ifndef PACKAGE_MANAGER_H
# define PACKAGE_MANAGER_H

#define PACKAGE_NAME_MAX           64
#define PACKAGE_VERSION_MAX        64
#define PACKAGE_URL_MAX          1024
#define PACKAGE_LICENSE_MAX       512
#define PACKAGE_DESCRIPTION_MAX  8192

typedef struct
{
  char name[PACKAGE_NAME_MAX];
  char version[PACKAGE_VERSION_MAX];
  char url[PACKAGE_URL_MAX];
  char license[PACKAGE_LICENSE_MAX];
  char description[PACKAGE_DESCRIPTION_MAX];
} package_info_t;

/* for given file name returns package name and version which */
/* the file contains; if search was not successfull, returns */
/* pkg[0] = '\0' and version[0] = '\0' */
void file_from_package(const char *file, package_info_t *pi);

#endif

