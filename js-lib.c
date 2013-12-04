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

#include "js-lib.h"
#include "filesystem.h"

void js_write_script(config_t config, const char *js_fname, const char *script)
{
  char dirname[FILEPATH_MAX];
  char scriptname[FILEPATH_MAX];
  FILE *js;
  
  snprintf(dirname, FILEPATH_MAX, "%s/%s", config.dir, JS_SUBDIR);
  create_dir(dirname);
  
  snprintf(scriptname, FILEPATH_MAX, "%s/%s", dirname, js_fname);

  js = open_write(scriptname, "js_write_function_toggle_view");
  fprintf(js, script);
  fclose(js);

  return;
}

void js_write_script_specimen_view(config_t config, const char *js_fname)
{
  js_write_script(config, js_fname,
                  "var cscript;\n"
                  "var cspecimen_type;\n"
                  "var specimen_id_suffix;\n"
                  "\n"                    
                  "function display_specimen(script, specimen_type)\n"
                  "{\n"
                  "  var el;\n"
                  "  var link_color;\n"
                  "\n"
                  "  el = document.getElementById(cspecimen_type + script + specimen_id_suffix);\n"
                  "  link_color = el.style.color;\n"
                  
                  "  /* turn off old */\n"
                  "  el = document.getElementById(cspecimen_type + cscript + specimen_id_suffix);\n"
                  "  el.style.display = 'none';\n"
                  "  if (scripts.length > 1)\n"
                  "  {\n"
                  "    el = document.getElementById(cscript + 'SpecimenAnchor');\n"
                  "    el.style.color = link_color;\n"
                  "  }\n"
                  "  if (specimen_types.length > 1)\n"
                  "  {\n"
                  "    el = document.getElementById(cspecimen_type + 'SpecimenAnchor');\n"
                  "    el.style.color = link_color;\n"
                  "  }\n"
                  "\n"
                  "  cscript = script;\n"
                  "  cspecimen_type = specimen_type;\n"
                  "\n"
                  "  /* turn on new */\n"
                  "  el = document.getElementById(cspecimen_type + cscript + specimen_id_suffix);\n"
                  "  el.style.display = 'block';\n"
                  "  if (scripts.length > 1)\n"
                  "  {\n"
                  "    el = document.getElementById(cscript + 'SpecimenAnchor');\n"
                  "    el.style.color = 'black'\n"
                  "  }\n"
                  "  if (specimen_types.length > 1)\n"
                  "  {\n"
                  "    el = document.getElementById(cspecimen_type + 'SpecimenAnchor');\n"
                  "    el.style.color = 'black'\n"
                  "  }\n"
                  "}\n"
                  "\n"
                  "function initialize_specimen_tabs(scripts, specimen_types, id_suffix)\n"
                  "{\n"
                  "  cscript = scripts[0];\n"
                  "  cspecimen_type = specimen_types[0];\n"
                  "  specimen_id_suffix = id_suffix\n"
                  "  var el;\n"
                  "  \n"
                  "  document.write('<table><tr><td>');\n"
                  "  \n"
                  "  if (scripts.length > 1)\n"
                  "  {\n"
                  "    document.write('<table><tr>');\n"
                  "    for (var i = 0; i < scripts.length; i++)\n"
                  "    {\n"
                  "      var script_wu = scripts[i].replace(/_/g, \" \");\n"
                  "      document.write(\"<td><a id='\" + scripts[i] + \"SpecimenAnchor' onclick=\\\"display_specimen('\"\n"
                  "                     + scripts[i] + \"', cspecimen_type)\\\">\" + script_wu + \"</a></td>\"); \n"
                  "      if (i % 10 == 9)\n"
                  "      {\n"
                  "        document.write('</tr><tr>');\n"
                  "      }\n"
                  "    }\n"
                  "    document.write('</tr></table>');\n"
                  "  }\n"
                  "  \n"
                  "  if (specimen_types.length > 1)\n"
                  "  {\n"
                  "    document.write('</td><td>')\n"
                  "    document.write('<table><tr>');\n"
                  "    for (var i = 0; i < specimen_types.length; i++)\n"
                  "    {\n"
                  "      document.write(\"<td><a id='\" + specimen_types[i] + \"SpecimenAnchor' \" + \n"
                  "                     \"onclick=\\\"display_specimen(cscript, '\" + specimen_types[i] + \"')\\\"\" +\n"
                  "                     \">\" + specimen_types[i] + \"</a></td>\");\n"
                  "    }\n"
                  "    document.write('</tr></table>');\n"
                  "  }\n"
                  "  \n"
                  "  document.write('</td></tr></table>');\n"
                  "  \n"
                  "  if (scripts.length > 1)\n"
                  "  {\n"
                  "    el = document.getElementById(scripts[0] + \"SpecimenAnchor\");\n"
                  "    el.style.color = 'black';\n"
                  "  }\n"
                  "  if (specimen_types.length > 1)\n"
                  "  {\n"
                  "    el = document.getElementById(specimen_types[0] + \"SpecimenAnchor\");\n"
                  "    el.style.color = 'black';\n"
                  "  }\n"
                  "}");
  return;
}

