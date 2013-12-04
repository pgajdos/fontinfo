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
#include <assert.h>

#include "ymp.h"
#include "filesystem.h"

void generate_ymp(config_t config, const char *pkgname, char *ympname)
{
  char dirname[FILEPATH_MAX];
  char filename[FILEPATH_MAX];

  FILE *ymp;
  int r;

  snprintf(dirname, FILEPATH_MAX, "%s/%s", config.dir, YMP_DIR);
  create_dir(dirname);

  snprintf(ympname, FILEPATH_MAX, "%s/%s.ymp", YMP_DIR, pkgname);
  snprintf(filename, FILEPATH_MAX, "%s/%s", config.dir, ympname);
  if (exists(filename))
    return;
  ymp = open_write(filename, "generate_ymp");

  fprintf(ymp, 
          "<metapackage"
          " xmlns:os=\"http://opensuse.org/Standards/One_Click_Install\""
          " xmlns=\"http://opensuse.org/Standards/One_Click_Install\">\n");
  for (r = 0; r < config.nrepositories; r++)
  {
    fprintf(ymp, "  <group distversion=\"%s\">\n", config.repositories[r][0]);
    fprintf(ymp, "    <repositories>\n");
    fprintf(ymp, "      <repository recomended=\"true\" format=\"yast\">\n");
    fprintf(ymp, "        <name>M17N:fonts (%s)</name>\n", config.repositories[r][0]);
    fprintf(ymp, "        <summary>\n");
    fprintf(ymp, "          %s repository for %s\n", 
            config.location, config.repositories[r][0]);
    fprintf(ymp, "        </summary>\n");
    fprintf(ymp, "        <description>\n");
    fprintf(ymp, "          This repository contains fonts located in\n");
    fprintf(ymp, "          %s installable on %s system.\n",
            config.location, config.repositories[r][0]);
    fprintf(ymp, "        </description>\n");
    fprintf(ymp, "        <url>%s</url>\n", config.repositories[r][1]);
    fprintf(ymp, "      </repository>\n");
    fprintf(ymp, "    </repositories>\n");
    fprintf(ymp, "    <software>\n");
    fprintf(ymp, "      <item>\n");
    fprintf(ymp, "        <name>%s</name>\n", pkgname);
    fprintf(ymp, "        <summary>\n");
    fprintf(ymp, "          Font package from %s\n", config.location);
    fprintf(ymp, "        </summary>\n");
    fprintf(ymp, "        <description>\n");
    fprintf(ymp, "          Latest version of package %s in %s.",
                 pkgname, config.location);
    fprintf(ymp, "        </description>\n");
    fprintf(ymp, "      </item>\n");
    fprintf(ymp, "    </software>\n");
    fprintf(ymp, "  </group>\n");
  }
  fprintf(ymp, "</metapackage>\n");

  fclose(ymp);
  return;
}

