## Process this file with automake to produce Makefile.in

if TEXINFO
DOCS_SUBDIR=docs
else
DOCS_SUBDIR=
endif

SUBDIRS = po src boards $(DOCS_SUBDIR)

icondir=$(datadir)/pixmaps
icon_DATA = gcompris.png gcompris-edit.png

Applicationsdir = $(datadir)/applications/
Applications_in_files = gcompris.desktop.in gcompris-edit.desktop.in
Applications_DATA = gcompris.desktop gcompris-edit.desktop

Menudir=$(prefix)/lib/menu
Menu_DATA=gcompris

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = \
	$(Applications_in_files) \
	$(icon_DATA) \
	$(Menu_DATA) \
	gcompris.spec \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	README.mingw config.h.mingw Makefile.mingw global_win32.mak gcompris.ico gcompris-installer.nsi

distcore = $(PACKAGE)-core-$(VERSION)

install-data-local:
	@$(NORMAL_INSTALL)
	if test -d $(srcdir)/$(PACKAGE_DATA_DIR); then \
	  $(mkinstalldirs) $(DESTDIR)/$(pkgdatadir)/$(PACKAGE_DATA_DIR); \
	  cd $(srcdir)/$(PACKAGE_DATA_DIR) ; tar cf -  --exclude "Makefile.in" --exclude "*.in" --exclude "*~" --exclude "Makefile" --exclude "Makefile.am" --exclude CVS --exclude .xvpics --exclude "[12]?.ogg" * | ( cd $(DESTDIR)/$(pkgdatadir)/$(PACKAGE_DATA_DIR) ; tar xf -) ; cd .. ; \
	fi

dist-hook:
	if test -d $(PACKAGE_DATA_DIR); then \
	  $(mkinstalldirs) $(distdir)/$(PACKAGE_DATA_DIR); \
	  cd $(srcdir)/$(PACKAGE_DATA_DIR) ; tar cf -  --exclude "Makefile.in" --exclude "Makefile" --exclude "*~" --exclude ".*" --exclude CVS --exclude .xvpics * | ( cd ../$(distdir)/$(PACKAGE_DATA_DIR) ; tar xf -) ; cd .. ; \
	fi


CLEANFILES = core-translation-report gcompris.desktop gcompris-edit.desktop gcompris.spec

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

all-local: 


ACLOCAL_AMFLAGS = 
