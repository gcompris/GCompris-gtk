## Process this file with automake to produce Makefile.in

SUBDIRS = intl po macros src sounds docs boards

icondir=$(datadir)/pixmaps
icon_DATA = gcompris.png

Gamesdir=$(datadir)/gnome/apps/Games
Games_in_files = gcompris.desktop.in
Games_DATA = $(Games_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

Menudir=$(prefix)/lib/menu
Menu_DATA=gcompris

EXTRA_DIST = \
	$(Games_DATA) \
	$(icon_DATA) \
	$(Menu_DATA) \
	gcompris.spec.in \
	gcompris.spec \
	xml-i18n-extract.in \
	xml-i18n-merge.in \
	xml-i18n-update.in

install-data-local:
	@$(NORMAL_INSTALL)
	if test -d @PACKAGE_DATA_DIR@; then \
	  $(mkinstalldirs) $(DESTDIR)/$(pkgdatadir)/@PACKAGE_DATA_DIR@; \
	  cd @PACKAGE_DATA_DIR@ ; tar cf -  --exclude CVS --exclude .xvpics --exclude .in --exclude ~ * | ( cd $(DESTDIR)/$(pkgdatadir)/@PACKAGE_DATA_DIR@ ; tar xf -) ; cd .. ; \
	fi

dist-hook:
	if test -d @PACKAGE_DATA_DIR@; then \
	  $(mkinstalldirs) $(distdir)/@PACKAGE_DATA_DIR@; \
	  cd @PACKAGE_DATA_DIR@ ; tar cf -  --exclude CVS --exclude .xvpics --exclude ~ * | ( cd ../$(distdir)/@PACKAGE_DATA_DIR@ ; tar xf -) ; cd .. ; \
	fi


