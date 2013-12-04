# collections-map.sh -- generate special character collections 
#                       mapping
#
# Copyright (C) 2013 Petr Gajdos (pgajdos at suse)
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

# DO NOT forgot to add new collection to collections.sh
grep 'Mathematical\|Musical\|MATHEMATICAL\|MUSICAL' | sed -e 's:;.*MUSICAL.*:;"Musical_Symbols":' -e 's:;.*MATHEMATICAL.*:;"Math":' -e 's:;.*Mathematical.*: ;"Math":' -e 's:;.*Musical.*: ;"Musical_Symbols":' | sed 's:^\([0-9A-F]\+\)\s\+:\1..\1 :' | sed -e 's:^:{0x:' -e 's:\.\.:, 0x:' -e 's:\s\+;:, :' -e 's:$:},:'
