## Process this file with automake to produce Makefile.in

SUBDIRS = intl po macros src sounds docs boards

icondir=$(datadir)/pixmaps
icon_DATA = gcompris.png

Gamesdir=$(datadir)/gnome/apps/Games
Games_in_files = gcompris.desktop.in
Games_DATA = $(Games_in_files:.desktop.in=.desktop)

Menudir=$(prefix)/lib/menu
Menu_DATA=gcompris

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = \
	$(Games_DATA) \
	$(Games_in_files) \
	$(icon_DATA) \
	$(Menu_DATA) \
	gcompris.spec.in \
	gcompris.spec \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in

distcore = $(PACKAGE)-core-$(VERSION)

install-data-local:
	@$(NORMAL_INSTALL)
	if test -d @PACKAGE_DATA_DIR@; then \
	  $(mkinstalldirs) $(DESTDIR)/$(pkgdatadir)/@PACKAGE_DATA_DIR@; \
	  find @PACKAGE_DATA_DIR@ -name \*~ -printf %f\\n >/tmp/GCOMPRIS_LIST.txt ; \
	  find @PACKAGE_DATA_DIR@ -name \*.in  -printf %f\\n >>/tmp/GCOMPRIS_LIST.txt ; \
	  cd @PACKAGE_DATA_DIR@ ; tar cf -  --exclude CVS --exclude .xvpics --exclude-from  /tmp/GCOMPRIS_LIST.txt * | ( cd $(DESTDIR)/$(pkgdatadir)/@PACKAGE_DATA_DIR@ ; tar xf -) ; cd .. ; \
	  rm -f /tmp/GCOMPRIS_LIST.txt ; \
	fi

dist-hook:
	if test -d @PACKAGE_DATA_DIR@; then \
	  $(mkinstalldirs) $(distdir)/@PACKAGE_DATA_DIR@; \
	  find @PACKAGE_DATA_DIR@ -name \*~ -printf %f\\n >/tmp/GCOMPRIS_LIST.txt ; \
	  find @PACKAGE_DATA_DIR@ -name \.\* -printf %f\\n >>/tmp/GCOMPRIS_LIST.txt ; \
	  cd @PACKAGE_DATA_DIR@ ; tar cf -  --exclude CVS --exclude .xvpics --exclude-from  /tmp/GCOMPRIS_LIST.txt * | ( cd ../$(distdir)/@PACKAGE_DATA_DIR@ ; tar xf -) ; cd .. ; \
	  rm -f /tmp/GCOMPRIS_LIST.txt ; \
	fi


clean-local:
	rm -f intltool-extract intltool-merge intltool-update

# Creates separates distribution files for the sounds by locale
dist-sounds:
	./create_dist_sounds en @ALL_LINGUAS@

# Create a core dist file without localized sounds file
dist-core:
	cp $(distdir).tar.gz $(distcore).tar.gz
	gzip -df $(distcore).tar.gz
	tar f $(distcore).tar --delete $(distdir)/boards/sounds
	gzip --best $(distcore).tar
	@echo Created $(distcore).tar.gz
