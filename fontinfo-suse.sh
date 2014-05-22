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

fontinfo -l M17N:fonts \
         -y bento \
         -n ymp \
         -g \
         -r 'openSUSE 13.2,http://download.opensuse.org/repositories/M17N:/fonts/openSUSE_Factory' \
         -r 'SUSE Linux Enterprise Server 12,http://download.opensuse.org/repositories/M17N:/fonts/openSUSE_Factory' \
         -r 'SUSE Linux Enterprise Desktop 12,http://download.opensuse.org/repositories/M17N:/fonts/openSUSE_Factory' \
         -r 'openSUSE 13.1,http://download.opensuse.org/repositories/M17N:/fonts/openSUSE_13.1' \
         -r 'openSUSE 12.3,http://download.opensuse.org/repositories/M17N:/fonts/openSUSE_12.3' \
         -r 'openSUSE 12.2,http://download.opensuse.org/repositories/M17N:/fonts/openSUSE_12.2' \
         -r 'SUSE Linux Enterprise Server 11,http://download.opensuse.org/repositories/M17N:/fonts/SLE_11_SP3/' \
         -r 'SUSE Linux Enterprise Desktop 11,http://download.opensuse.org/repositories/M17N:/fonts/SLE_11_SP3/' \
