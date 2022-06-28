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
#include "constants.h"

void js_write_script(config_t config, const char *js_fname, const char *script)
{
  char dirname[FILEPATH_MAX];
  char scriptname[FILEPATH_MAX];
  FILE *js;
  
  snprintf(dirname, FILEPATH_MAX, "%s/%s", config.dir, JS_SUBDIR);
  create_dir(dirname);
  
  snprintf(scriptname, FILEPATH_MAX, "%s/%s", dirname, js_fname);

  js = open_write(scriptname, "js_write_script");
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
                  "  cscript = scripts.length > 0 ? scripts[0] : \""NO_SCRIPT"\";\n"
                  "  cspecimen_type = specimen_types[0];\n"
                  "  specimen_id_suffix = id_suffix\n"
                  "  var el;\n"
                  "  \n"
                  "  if (scripts.length > 0 && specimen_types.length > 0)\n"
                  "  {\n"
                  "    document.write('<table><tr><td>');\n"
                  "  }\n"
                  "  \n"
                  "  if (scripts.length > 0)\n"
                  "  {\n"
                  "    document.write('<table><tr>');\n"
                  "    for (var i = 0; i < scripts.length; i++)\n"
                  "    {\n"
                  "      var script_wu = scripts[i].replace(/_/g, \" \");\n"
                  "      document.write(\"<td><div class=\\\"my-3\\\"><button class=\\\"btn btn-secondary\\\"\"\n"
		  "                     + \"id='\" + scripts[i] + \"SpecimenAnchor' onclick=\\\"display_specimen('\"\n"
                  "                     + scripts[i] + \"', cspecimen_type)\\\">\" + script_wu + \"</button></div></td>\"); \n"
                  "      if (i % 10 == 9)\n"
                  "      {\n"
                  "        document.write('</tr><tr>');\n"
                  "      }\n"
                  "    }\n"
                  "    document.write('</tr></table>');\n"
                  "  }\n"
                  "  \n"
                  "  if (scripts.length > 0 && specimen_types.length > 0)"
                  "  {\n"
                  "    document.write('</td><td>');\n"
                  "  }\n"
                  "  if (specimen_types.length > 1)\n"
                  "  {\n"
                  "    document.write('<table><tr>');\n"
                  "    for (var i = 0; i < specimen_types.length; i++)\n"
                  "    {\n"
                  "      document.write(\"<td><div class=\\\"my-3\\\"><button class=\\\"btn btn-primary\\\"\" + \n"
		  "                     \"id='\" + specimen_types[i] + \"SpecimenAnchor' \" + \n"
                  "                     \"onclick=\\\"display_specimen(cscript, '\" + specimen_types[i] + \"')\\\"\" +\n"
                  "                     \">\" + specimen_types[i] + \"</button></div></td>\");\n"
                  "    }\n"
                  "    document.write('</tr></table>');\n"
                  "  }\n"
                  "  \n"
                  "  if (scripts.length > 0 && specimen_types.length > 0)\n"
                  "  {\n"
                  "    document.write('</td></tr></table>');\n"
                  "  }\n"
                  "  \n"
                  "  if (scripts.length > 0)\n"
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

void js_write_script_charset_view(config_t config, const char *js_fname)
{
  js_write_script(config, js_fname,
                  "function charset_block_show(cell_id, html)\n"
                  "{\n"
                  "  el = document.getElementById(cell_id);\n"
                  "  el.innerHTML = html;\n"
                  "  el.style.display = 'block';\n"
                  "  charset_shown[cell_id] = 1;\n"
                  "  el = document.getElementById(cell_id + 'Toggle');\n"
                  "  el.innerHTML = '-'\n"
                  "}\n"
                  "\n"
                  "function charset_block_hide(cell_id)\n"
                  "{\n"
                  "  el = document.getElementById(cell_id);\n"
                  "  el.innerHTML = '';\n"
                  "  el.style.display = 'none';\n"
                  "  charset_shown[cell_id] = 0;\n"
                  "  el = document.getElementById(cell_id + 'Toggle');\n"
                  "  el.innerHTML = '+'\n"
                  "}\n"
                  "\n"
                  "function charset_block_toggle(cell_id, html)\n"
                  "{\n"
                  //"  el.innerHTML = \"AHOJ<script src=\" + txt_file + \"></srcipt>\"\n"
                  "  if (charset_shown[cell_id])\n"
                  "  {\n"
                  "    charset_block_hide(cell_id);\n"
                  "  }\n"
                  "  else\n"
                  "  {\n"
                  "    charset_block_show(cell_id, html);\n"
                  "  }\n"
                  "}\n"
                  "\n"
                  "function charset_blocks_show()\n"
                  "{\n"
                  "  for (var charset_block in charset_shown)\n"
                  "  {\n"
                  "    charset_block_show(charset_block, charset_html[charset_block]);\n"
                  "  }\n"
                  "}\n"
                  "\n"
                  "function charset_blocks_hide()\n"
                  "{\n"
                  "  for (var charset_block in charset_shown)\n"
                  "  {\n"
                  "    charset_block_hide(charset_block);\n"
                  "  }\n"
                  "}\n");
  return;
}

