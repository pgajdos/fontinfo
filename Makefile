# Makefile
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

DIRS		= languages
VERSION		 = 20170627
PKGMAN_CFLAGS	 = $(shell sh ./package-manager.sh --cflags)
PKGMAN_LIBS	 = $(shell sh ./package-manager.sh --libs)
LIBPNG_CFLAGS	 = $(shell pkg-config --cflags libpng)
LIBPNG_LIBS	 = $(shell pkg-config --libs libpng)
FT2_CFLAGS	 = $(shell pkg-config --cflags freetype2)
FT2_LIBS	 = $(shell pkg-config --libs freetype2)
HB_CFLAGS	 = $(shell pkg-config --cflags harfbuzz)
HB_LIBS		 = $(shell pkg-config --libs harfbuzz)
FONTCONFIG_LIBS	 = $(shell pkg-config --libs fontconfig)
MYCFLAGS	 = -DFONTINFO_VERSION=$(VERSION) $(PKGMAN_CFLAGS) $(LIBPNG_CFLAGS) $(FT2_CFLAGS) $(HB_CFLAGS) -Wall -g
MYLIBS		 = $(PKGMAN_LIBS) $(FONTCONFIG_LIBS) $(LIBPNG_LIBS) $(FT2_LIBS) $(HB_LIBS)
MARKDOWN_COMMAND = $(shell if [ -x /usr/bin/markdown ]; then echo "/usr/bin/markdown -f 'html'"; else echo "/usr/bin/cat"; fi)

OBJS		= constants.o img.o ft.o hbz.o img_svg.o img_png.o img_common.o fcinfo.o \
                  filesystem.o package-manager.o plain-style.o bento-style.o styles-commons.o js-lib.o ymp.o

fontinfo:			$(OBJS) fontinfo.c configuration.h filesystem.h plain-style.h bento-style.h constants.h
				gcc $(MYLDFLAGS) $(LDLAGS) -o fontinfo $(OBJS) fontinfo.c $(MYLIBS)
plain-style.o:			plain-style.c plain-style.h fcinfo.h constants.h filesystem.h img.h package-manager.h configuration.h styles-commons.h ft.h ymp.h
				gcc -c $(MYCFLAGS) $(CFLAGS) plain-style.c
bento-style.o:			bento-style.c bento-style.h fcinfo.h constants.h filesystem.h img.h package-manager.h configuration.h styles-commons.h ft.h js-lib.h ymp.h
				gcc -c $(MYCFLAGS) $(CFLAGS) bento-style.c
styles-commons.o:		styles-commons.c styles-commons.h fcinfo.h img_svg.h constants.h
				gcc -c $(MYCFLAGS) $(CFLAGS) styles-commons.c
fcinfo.o:			fcinfo.c fcinfo.h constants.h
				gcc -c $(MYCFLAGS) $(CFLAGS) fcinfo.c
img.o:				img.c img.h img_png.h img_svg.h configuration.h constants.h
				gcc -c $(MYCFLAGS) $(CFLAGS) img.c
img_svg.o:			img_svg.c img_svg.h configuration.h constants.h fcinfo.h img_common.h
				gcc -c $(MYCFLAGS) $(CFLAGS) img_svg.c
img_png.o:			img_png.c img_png.h configuration.h constants.h ft.h filesystem.h fcinfo.h img_common.h
				gcc -c $(MYCFLAGS) $(CFLAGS) img_png.c
img_common.o:			img_common.c img_common.h ft.h configuration.h ft.h fcinfo.h constants.h
				gcc -c $(MYCFLAGS) $(CFLAGS) img_common.c
ft.o:				ft.c ft.h hbz.h constants.h
				gcc -c $(MYCFLAGS) $(CFLAGS) ft.c
hbz.o:				hbz.c hbz.h constants.h
				gcc -c $(MYCFLAGS) $(CFLAGS) hbz.c
constants.o:			constants.c constants.h  
				gcc -c $(MYCFLAGS) $(CFLAGS) constants.c
constants.h:			languages/languages.txt unicode/unicode.txt unicode/scripts-map.txt unicode/scripts.txt unicode/sentences.txt unicode/blocks-map.txt
				touch constants.h
filesystem.o:			filesystem.c filesystem.h
				gcc -c $(MYCFLAGS) $(CFLAGS) filesystem.c
package-manager.o:		package-manager.c package-manager.h
				gcc -c $(MYCFLAGS) $(CFLAGS) package-manager.c
js-lib.o:			js-lib.c js-lib.h filesystem.h configuration.h constants.h
				gcc -c $(MYCFLAGS) $(CFLAGS) js-lib.c
ymp.o:				ymp.c ymp.h filesystem.h configuration.h
				gcc -c $(MYCFLAGS) $(CFLAGS) ymp.c
languages/languages.txt: 	languages/language-subtag-registry languages/languages.sh
				cd languages; cat language-subtag-registry | sh languages.sh > languages.txt;
unicode/unicode.txt:		unicode/UnicodeData.txt unicode/unicode.sh
				cd unicode; cat UnicodeData.txt | sh unicode.sh > unicode.txt; 
unicode/scripts.txt:		unicode/Scripts.txt unicode/scripts.sh unicode/collections.sh
				cd unicode; cat Scripts.txt | sh scripts.sh > scripts.txt; sh collections.sh >> scripts.txt
unicode/scripts-map.txt:	unicode/Scripts.txt unicode/scripts-map.sh unicode/collections-map.sh
				cd unicode; cat Scripts.txt | sh scripts-map.sh > scripts-map.txt; cat Scripts.txt Blocks.txt | sh collections-map.sh >> scripts-map.txt;
unicode/blocks-map.txt:		unicode/Blocks.txt unicode/blocks.sh
				cd unicode; cat Blocks.txt | sh blocks.sh > blocks-map.txt
doc/thanks.txt:			doc/THANKS.md
				cd doc; cat THANKS.md | $(MARKDOWN_COMMAND) | sed 's:":\\":g' | sed 's:\(.*\):  \"\1\", :' > thanks.txt
styles-commons.h:		doc/thanks.txt
				touch styles-commons.h

install:	fontinfo	
		install -Dd -m 0755 $(PREFIX) $(DESTDIR)/usr/bin
		install     -m 0755 fontinfo $(DESTDIR)/usr/bin
		install     -m 0755 fontinfo-suse.sh $(DESTDIR)/usr/bin/fontinfo-suse

clean:
		rm -f *.o fontinfo languages/languages.txt unicode/unicode.txt unicode/scripts-map.txt unicode/scripts.txt unicode/blocks-map.txt doc/thanks.txt

release:	
		mkdir fontinfo-$(VERSION)
		cp *.c *.h *.sh COPYING CHANGES Makefile README.md fontinfo-$(VERSION)
		mkdir fontinfo-$(VERSION)/doc
		cp doc/NEW* fontinfo-$(VERSION)/doc
		cp doc/THANKS.md fontinfo-$(VERSION)/doc
		mkdir fontinfo-$(VERSION)/languages
		cp languages/language-subtag-registry fontinfo-$(VERSION)/languages
		cp languages/languages.sh fontinfo-$(VERSION)/languages
		cp languages/SOURCE fontinfo-$(VERSION)/languages
		mkdir fontinfo-$(VERSION)/unicode
		cp unicode/UnicodeData.txt fontinfo-$(VERSION)/unicode
		cp unicode/unicode.sh fontinfo-$(VERSION)/unicode
		cp unicode/SOURCES fontinfo-$(VERSION)/unicode
		cp unicode/Scripts.txt fontinfo-$(VERSION)/unicode
		cp unicode/scripts-map.sh fontinfo-$(VERSION)/unicode
		cp unicode/scripts.sh fontinfo-$(VERSION)/unicode
		cp unicode/sentences.txt fontinfo-$(VERSION)/unicode
		cp unicode/blocks.sh fontinfo-$(VERSION)/unicode
		cp unicode/Blocks.txt fontinfo-$(VERSION)/unicode
		cp unicode/collections.sh fontinfo-$(VERSION)/unicode
		cp unicode/collections-map.sh fontinfo-$(VERSION)/unicode
		tar cvjf fontinfo-$(VERSION).tar.bz2 fontinfo-$(VERSION)
		rm -r fontinfo-$(VERSION)

