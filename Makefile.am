## Process this file with automake to produce Makefile.in

SUBDIRS = po src docs boards

icondir=$(datadir)/pixmaps
icon_DATA = gcompris.png gcompris-edit.png

Applicationsdir = $(datadir)/applications/
Applications_in_files = gcompris.desktop.in
Applications_DATA = gcompris.desktop

Menudir=$(prefix)/lib/menu
Menu_DATA=gcompris

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = \
	$(Applications_DATA) \
	$(Applications_in_files) \
	$(icon_DATA) \
	$(Menu_DATA) \
	gcompris.spec \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	po-checktrans.py po-checktrans

distcore = $(PACKAGE)-core-$(VERSION)

install-data-local:
	@$(NORMAL_INSTALL)
	if test -d @PACKAGE_DATA_DIR@; then \
	  $(mkinstalldirs) $(DESTDIR)/$(pkgdatadir)/@PACKAGE_DATA_DIR@; \
	  cd @PACKAGE_DATA_DIR@ ; tar cf -  --exclude "*.in" --exclude "*~" --exclude "Makefile" --exclude "Makefile.am" --exclude CVS --exclude .xvpics --exclude "*.assetml" * | ( cd $(DESTDIR)/$(pkgdatadir)/@PACKAGE_DATA_DIR@ ; tar xf -) ; cd .. ; \
	fi

dist-hook:
	if test -d @PACKAGE_DATA_DIR@; then \
	  $(mkinstalldirs) $(distdir)/@PACKAGE_DATA_DIR@; \
	  cd @PACKAGE_DATA_DIR@ ; tar cf -  --exclude "*.in" --exclude "Makefile" --exclude "*~" --exclude ".*" --exclude CVS --exclude .xvpics * | ( cd ../$(distdir)/@PACKAGE_DATA_DIR@ ; tar xf -) ; cd .. ; \
	fi


CLEANFILES = core-translation-report gcompris.desktop gcompris.spec

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

# Display stats on translation
core-translation-report: po-checktrans.py po-checktrans
	-([ -f po/gcompris.pot ] && cd $(srcdir) && ./po-checktrans po/gcompris.pot po/*.po)

all-local: core-translation-report


ACLOCAL_AMFLAGS = 
