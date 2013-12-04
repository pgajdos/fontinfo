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

#include "package-manager.h"

#if defined RPM_PKG_MANAGER

#include <rpm/rpmts.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmlib.h>

static int config_files_read = 0;

void file_from_package(const char *file, package_info_t *pi)
{
  rpmts ts;
  rpmdbMatchIterator iter;
  Header h;
  rpmtd td_name, td_version, td_url, td_license, td_description;

  pi->name[0] = '\0';
  pi->version[0] = '\0';
  pi->url[0] = '\0';
  pi->license[0] = '\0';
  pi->description[0] = '\0';

  if (! config_files_read)
  {
    if (rpmReadConfigFiles(NULL, NULL) == -1)
    {
      fprintf(stderr, "file_from_package(): can't read rpm config files\n");
      exit(1);
    }
    config_files_read = 1;
  }

  ts = rpmtsCreate();
  td_name = rpmtdNew();
  td_version = rpmtdNew();
  td_url = rpmtdNew();
  td_license = rpmtdNew();
  td_description = rpmtdNew();

  iter = rpmtsInitIterator(ts, RPMDBI_BASENAMES, file, 0);

  if ((h = rpmdbNextIterator(iter)) != NULL)
  {
    if (headerGet(h, RPMTAG_NAME, td_name, HEADERGET_EXT))
      snprintf(pi->name, PACKAGE_NAME_MAX, "%s", 
               rpmtdGetString(td_name));
    if (headerGet(h, RPMTAG_VERSION, td_version, HEADERGET_EXT))
      snprintf(pi->version, PACKAGE_VERSION_MAX, "%s", 
              rpmtdGetString(td_version));
    if (headerGet(h, RPMTAG_URL, td_url, HEADERGET_EXT))
      snprintf(pi->url, PACKAGE_URL_MAX, "%s", 
               rpmtdGetString(td_url));
    if (headerGet(h, RPMTAG_LICENSE, td_license, HEADERGET_EXT))
      snprintf(pi->license, PACKAGE_LICENSE_MAX, "%s", 
               rpmtdGetString(td_license));
    if (headerGet(h, RPMTAG_DESCRIPTION, td_description, HEADERGET_EXT))
      snprintf(pi->description, PACKAGE_DESCRIPTION_MAX, "%s", 
               rpmtdGetString(td_description));
  }

  rpmdbFreeIterator(iter);
  rpmtsFree(ts);
  rpmtdFree(td_name);
  rpmtdFree(td_version);
  rpmtdFree(td_url);
  rpmtdFree(td_license);
  rpmtdFree(td_description);
  return;
}

#elif defined DEB_PKG_MANAGER
#warning Debian package manager detected, but handler was not implemented yet.
#warning There will be no package information in the fontinfo output.

void file_from_package(const char *file, package_info_t *pi)
{
  pi->name[0] = '\0';
  pi->version[0] = '\0';
  pi->url[0] = '\0';
  pi->license[0] = '\0';
  return;
}

#else
#warning No package manager detected!
#warning There will be no package information in the fontinfo output.

void file_from_package(const char *file, package_info_t *pi)
{
  pi->name[0] = '\0';
  pi->version[0] = '\0';
  pi->url[0] = '\0';
  pi->license[0] = '\0';
  return;
}

#endif
