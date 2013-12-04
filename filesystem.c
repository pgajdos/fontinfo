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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "filesystem.h"

void create_dir(const char *dir)
{
  struct stat s;
  if (stat(dir, &s) == 0)
  {
    if (!S_ISDIR(s.st_mode))
    {
      fprintf(stderr, "create_dir(): %s is not directory\n", dir);
      exit(1);
    }
  }
  else
  {
    if (mkdir(dir, 0755))
    {
      fprintf(stderr, "create_dir(): cannot create %s: \n", dir);
      perror("");
      exit(1);
    }
  }
  return;
}

FILE *open_write(const char *fname, const char *func)
{
  FILE *f;

  if (! (f = fopen(fname, "w")))
  {
    fprintf(stderr, "%s(): "
                    "unable to open %s for writing: ", func, fname);
    perror("");
    exit (1);
  }

  return f;
}

int exists(const char *fname)
{
  struct stat s;
  return (stat(fname, &s) == 0);
}

FILE *open_binary_write(const char *fname, const char *func)
{
  FILE *f;

  if (! (f = fopen(fname, "wb")))
  {
    fprintf(stderr, "%s(): "
                    "unable to open %s for binary writing: ", func, fname);
    perror("");
    exit (1);
  }

  return f;
}

/* removes spaces and slashes from string */
void remove_spaces_and_slashes(char *string)
{
  char *p1 = string, *p2 = string;

  while (*p2 != '\0')
  {
    if (!isspace(*p2) && *p2 != '/')
    {
      *p1 = *p2;
      p1++;
    }
    else 
    {    
      /* a capital letter before space */
      /* as side efect of this, both 'ETL fixed' */ 
      /*  and 'ETL Fixed' maps to the 'ETLFixed' */ 
      *(p2 + 1) = toupper(*(p2 + 1));
    }
    p2++;
  }

  *p1 = '\0';
  return;
}


