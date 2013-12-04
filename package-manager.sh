# package-manager.sh -- detect package manager
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

rpm --version > /dev/null 2>&1
if [ $? -eq 0 ]; then
  CFLAGS='-DRPM_PKG_MANAGER' `pkg-config --cflags rpm`
  LIBS=`pkg-config --libs rpm`
else
  dpkg --version > /dev/null 2>&1
  if [ $? -eq 0 ]; then
    CFLAGS='-DDEB_PKG_MANAGER' `pkg-config --cflags dpkg`
    LIBS=`pkg-config --libs dpkg`
  else
    CFLAGS='-DOTHER_PKG_MANAGER'
    LIBS=''
  fi
fi

case $1 in
  '--cflags')
    echo $CFLAGS ;;
  '--libs')
    echo $LIBS ;;
esac

