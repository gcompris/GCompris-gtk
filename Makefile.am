## Process this file with automake to produce Makefile.in

if !BUILD_MINGW32
if TEXINFO
DOCS_SUBDIR=docs
else
DOCS_SUBDIR=
endif
else
DOCS_SUBDIR=
endif

SUBDIRS = po src boards $(DOCS_SUBDIR) autopackage

#if OS_MACOSX
#SUBDIRS += macosx
#endif


icondir=$(datadir)/pixmaps
icon_DATA = gcompris.png gcompris-edit.png

if BUILD_MINGW32
Applicationsdir = $(datadir)/
Applications_in_files =
Applications_DATA =

Menudir=$(prefix)
Menu_DATA=

else
Applicationsdir = $(datadir)/applications/
Applications_in_files = gcompris.desktop.in gcompris-edit.desktop.in
Applications_DATA = gcompris.desktop gcompris-edit.desktop

Menudir=$(prefix)/lib/menu
Menu_DATA=gcompris
endif

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = \
	$(Applications_in_files) \
	$(icon_DATA) \
	$(Menu_DATA) \
	gcompris.spec \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	gnome2-macros \
	README.mingw README.translators README.windows \
	config.h.mingw Makefile.mingw global_win32.mak gcompris.ico gcompris-uninstal.ico \
	gcompris-installer.nsi


distcore = $(PACKAGE)-core-$(VERSION)

install-data-local:
	@$(NORMAL_INSTALL)
	if test -d $(srcdir)/$(PACKAGE_DATA_DIR); then \
	  $(mkinstalldirs) $(DESTDIR)/$(pkgdatadir)/$(PACKAGE_DATA_DIR); \
	  cd $(srcdir)/$(PACKAGE_DATA_DIR) ; tar cf -  --exclude "Makefile.in" --exclude "*.in" --exclude "*~" --exclude "Makefile" --exclude "Makefile.am" --exclude .svn --exclude .xvpics --exclude "1[1-9].ogg"  --exclude "2?.ogg" * | ( cd $(DESTDIR)/$(pkgdatadir)/$(PACKAGE_DATA_DIR) ; tar xf -) ; cd .. ; \
	fi

dist-hook:
	if test -d $(PACKAGE_DATA_DIR); then \
	  $(mkinstalldirs) $(distdir)/$(PACKAGE_DATA_DIR); \
	  cd $(srcdir)/$(PACKAGE_DATA_DIR) ; tar cf -  --exclude "Makefile.in" --exclude "Makefile" --exclude "*~" --exclude ".*" --exclude .svn --exclude .xvpics * | ( cd ../$(distdir)/$(PACKAGE_DATA_DIR) ; tar xf -) ; cd .. ; \
	fi


CLEANFILES = core-translation-report gcompris.desktop gcompris-edit.desktop gcompris.spec config.h.mingw

clean-local:
	rm -f intltool-extract intltool-merge intltool-update
	-rm -fr $(BUNDLE_APPDIR)

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

if OS_MACOSX
BUNDLE_RESOURCES_DIR=$(BUNDLE_APPDIR)/Contents/Resources
BUNDLE_EXEC_DIR=$(BUNDLE_APPDIR)/Contents/MacOS
else
BUNDLE_RESOURCES_DIR=$(BUNDLE_APPDIR)/Resources
BUNDLE_EXEC_DIR=$(BUNDLE_APPDIR)
endif

if WITH_NSBUNDLE

ETC_PANGO=etc/pango
ETC_GTK=etc/gtk-2.0
GTK_CONF_FILES=gtk.immodules gdk-pixbuf.loaders im-multipress.conf
PANGO_CONF_FILES=pango.modules
FONTCONFIG_PATH=etc/fonts
GTK_MODULES_PATH=lib/gtk-2.0
PANGO_MODULES_PATH=lib/pango
EXTERN_BIN= gnuchess gnucap
PYTHON_MODULES_PATH_ORIG=lib/python$(PYTHON_VERSION)
PYTHON_MODULES_PATH_DEST=lib/python
PLIST=Info.plist
ICON=gcompris.png

FCT_CP_TAR=function cp_tar { \
	$(INSTALL) -d $$2 ; \
	(cd $$1 ; tar cf - $$3 * )| ( cd $$2; tar xf -) ; \
	}

FCT_LDD_CP=function ldd_cp { \
	for f in $$(ldd $$1 | grep $$2 | eval "sed -e 's,^.*$$2/\(.*\) (.*$$,\1,' " ); do \
           if test ! -f $$3/$$f ; then \
                echo $(INSTALL) -D $$2/$$f $$3/$$f ; \
                $(INSTALL) -D $$2/$$f $$3/$$f ;\
	        ldd_cp $$3/$$f  $$2 $$3;\
           fi \
        done ; } 

FCT_OTOOL_CP=function otool_cp { \
	for f in $$(otool -L $$1 | grep $$2 | eval "sed -e 's,^.*$$2/\(.*.dylib\).*$$,\1,' " ); do \
	    install_name_tool -change $$2/$$f @executable_path$$4/$$f ;\
	    if test ! -f $$3/$$f ; then \
		echo $(INSTALL) -D $$2/$$f $$3/$$f ; \
		$(INSTALL) -D $$2/$$f $$3/$$f ;\
	        otool_cp $$3/$$f $$2 $$3 $$4 ;\
            fi \
          done; \
	}

nsbundle: 
	$(INSTALL) -d $(BUNDLE_RESOURCES_DIR)
	$(INSTALL) -d $(BUNDLE_EXEC_DIR)
	$(INSTALL) -d $(BUNDLE_EXEC_DIR)$(NSBUNDLE_GTK_DIR)
	make install prefix=$(BUNDLE_RESOURCES_DIR) \
		     exec_prefix=$(BUNDLE_RESOURCES_DIR) \
	             bindir=$(BUNDLE_EXEC_DIR) \
	             PYTHON_PLUGIN_DIR=$(BUNDLE_RESOURCES_DIR)/share/gcompris/python 
	mv $(BUNDLE_EXEC_DIR)/gcompris $(BUNDLE_EXEC_DIR)/$(BUNDLE_NAME)
if OS_MACOSX
	$(FCT_OTOOL_CP) otool_cp $(BUNDLE_EXEC_DIR)/$(BUNDLE_NAME) $(prefix) $(BUNDLE_EXEC_DIR)$(NSBUNDLE_GTK_DIR) $(NSBUNDLE_GTK_DIR)
else
	$(FCT_LDD_CP); ldd_cp $(BUNDLE_EXEC_DIR)/$(BUNDLE_NAME) $(prefix) $(BUNDLE_EXEC_DIR)$(NSBUNDLE_GTK_DIR)
endif
	$(INSTALL) -d $(BUNDLE_EXEC_DIR)$(NSBUNDLE_GTK_DIR)/$(ETC_GTK)
#sed -e 's,$(prefix),@{prefix},g' $(prefix)/$(ETC_PANGO)/pangorc > $(BUNDLE_EXEC_DIR)$(NSBUNDLE_GTK_DIR)/$(ETC_PANGO)/pangord
	for f in $(GTK_CONF_FILES); do\
	    sed -e 's,$(prefix),@{prefix},g' $(prefix)/$(ETC_GTK)/$$f > $(BUNDLE_EXEC_DIR)$(NSBUNDLE_GTK_DIR)/$(ETC_GTK)/$$f ;\
        done
	$(INSTALL) -d $(BUNDLE_EXEC_DIR)$(NSBUNDLE_GTK_DIR)/$(ETC_PANGO)
	for f in $(PANGO_CONF_FILES); do\
	    sed -e 's,$(prefix),@{prefix},g' $(prefix)/$(ETC_PANGO)/$$f > $(BUNDLE_EXEC_DIR)$(NSBUNDLE_GTK_DIR)/$(ETC_PANGO)/$$f ;\
        done
	$(INSTALL) $(top_srcdir)/macosx/pangorc $(BUNDLE_EXEC_DIR)$(NSBUNDLE_GTK_DIR)/$(ETC_PANGO)/pangorc
	$(INSTALL) -d $(BUNDLE_EXEC_DIR)$(NSBUNDLE_GTK_DIR)/$(FONTCONFIG_PATH)
	$(FCT_CP_TAR); cp_tar $(prefix)/$(FONTCONFIG_PATH) $(BUNDLE_EXEC_DIR)$(NSBUNDLE_GTK_DIR)/$(FONTCONFIG_PATH)
	$(INSTALL) macosx/fonts.conf $(BUNDLE_EXEC_DIR)$(NSBUNDLE_GTK_DIR)/$(FONTCONFIG_PATH)
	$(FCT_CP_TAR); cp_tar $(prefix)/$(GTK_MODULES_PATH)  $(BUNDLE_EXEC_DIR)$(NSBUNDLE_GTK_DIR)/$(GTK_MODULES_PATH) "--exclude include"
	$(FCT_CP_TAR); cp_tar $(prefix)/$(PANGO_MODULES_PATH) $(BUNDLE_EXEC_DIR)$(NSBUNDLE_GTK_DIR)/$(PANGO_MODULES_PATH)
	$(FCT_CP_TAR); cp_tar $(prefix)/$(PYTHON_MODULES_PATH_ORIG) $(BUNDLE_EXEC_DIR)$(NSBUNDLE_GTK_DIR)/$(PYTHON_MODULES_PATH_DEST)
	for f in $(EXTERN_BIN); do \
	   $(INSTALL) -D  $(prefix)/bin/$$f $(BUNDLE_RESOURCES_DIR)/bin/$$f; \
	done
	$(INSTALL) $(top_srcdir)/macosx/Info.plist $(BUNDLE_RESOURCES_DIR)/
	$(INSTALL) $(top_srcdir)/macosx/gcompris.svg $(BUNDLE_RESOURCES_DIR)/
	$(INSTALL) $(top_srcdir)/macosx/gcompris.png $(BUNDLE_RESOURCES_DIR)/
endif

