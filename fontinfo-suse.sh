#!/bin/bash
# fontinfo-suse.sh -- run fontinfo with opensuse repos and bento style
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

# fonts-config package is needed to correctly fill in FcPattern entries
rpm -q fonts-config >/dev/null 2>&1 || { echo 'fonts-config package not installed!'; exit 1; }
fontinfo -l M17N:fonts \
         -y bento \
         -n ymp \
         -g \
         -r 'openSUSE Tumbleweed,http://download.opensuse.org/repositories/M17N:/fonts/openSUSE_Tumbleweed' \
         -r 'openSUSE Leap 42.2,http://download.opensuse.org/repositories/M17N:/fonts/openSUSE_Leap_42.2' \
         -r 'SUSE Linux Enterprise Server 12,http://download.opensuse.org/repositories/M17N:/fonts/SLE_12_SP2' \
         -r 'SUSE Linux Enterprise Desktop 12,http://download.opensuse.org/repositories/M17N:/fonts/SLE_12_SP2' \
         -r 'SUSE Linux Enterprise Server 11,http://download.opensuse.org/repositories/M17N:/fonts/SLE_11_SP4/' \
         -r 'SUSE Linux Enterprise Desktop 11,http://download.opensuse.org/repositories/M17N:/fonts/SLE_11_SP4/' \
