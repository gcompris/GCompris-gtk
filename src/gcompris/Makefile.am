## Process this file with automake to produce Makefile.in
if PYTHON_PLUGIN
python_ldadd = $(PYTHON_LIBS)
python_ldflags = -u Py_Initialize
else
python_ldadd =
python_ldflags =
endif

if PLATFORM_WIN32
no_undefined = -no-undefined
endif

if OS_WIN32
install-libtool-import-lib:
	$(INSTALL) .libs/libgcompris.dll.a $(DESTDIR)$(libdir)

uninstall-libtool-import-lib:
	-rm $(DESTDIR)$(libdir)/libgcompris.dll.a
else
install-libtool-import-lib:
uninstall-libtool-import-lib:
endif

gcompris_edit_INCLUDE = \
	$(GCOMPRIS_EDIT_CFLAGS)


if GCOMPRIS_EDIT
bin_PROGRAMS = gcompris gcompris_edit
gcompris_edit_cflags = $(GCOMPRIS_EDIT_CFLAGS)
else
bin_PROGRAMS = gcompris
gcompris_edit_cflags = 
endif

INCLUDES = \
	-I$(top_srcdir)/intl \
	-DDATADIR=\""$(datadir)"\" \
	$(GCOMPRIS_CFLAGS) \
	$(GNOME_INCLUDEDIR) \
	$(XML_CFLAGS) \
	$(SDL_CFLAGS) \
	-DGNOME_DISABLE_DEPRECATED \
	$(gcompris_edit_cflags)

gcompris_SOURCES = \
	main.c

gcompris_LDFLAGS = \
	-export-dynamic $(python_ldflags) $(SDL_LIBS)

gcompris_LDADD = \
	$(LIBICONV)		\
        $(INTLLIBS) \
	$(python_ldadd) \
	$(top_builddir)/src/gcompris/libgcompris-1.la \
	$(LIBPOPT)


#
# Gcompris edit
#
gcompris_edit_LDFLAGS = \
	-export-dynamic $(shell sdl-config --libs) -lSDL_mixer

gcompris_edit_SOURCES = \
	gcompris-edit.c gcompris-edit.h \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h

gcompris_edit_LDADD = \
	$(GCOMPRIS_LIBS) \
	$(GCOMPRIS_EDIT_LIBS) \
	$(XML_LIBS) $(INTLLIBS) \
	$(top_builddir)/src/gcompris/libgcompris-1.la

#
# Gcompris library
# 
lib_LTLIBRARIES = libgcompris-1.la

libgcompris_1_la_LIBADD = \
	$(GCOMPRIS_LIBS) $(XML_LIBS)

libgcompris_1_la_LDFLAGS = $(no_undefined) \
	-version-info $(LIBGCOMPRIS_CURRENT):$(LIBGCOMPRIS_REVISION):$(LIBGCOMPRIS_AGE) 

libgcomprisincludedir = $(includedir)/libgcompris-1.0/libgcompris

libgcomprisinclude_HEADERS =			\
	gcompris.h gcompris-board.h \
	gameutil.h \
	svgutil.h \
	properties.h \
	pixbuf_util.h \
	soundutil.h \
	cursor.h \
	plugin.h \
	bonus.h \
	timer.h \
	score.h \
	skin.h \
	gcompris_config.h \
	about.h \
	assetml.h

libgcompris_1_la_SOURCES =			\
	$(libgcomprisinclude_HEADERS)	\
	gcompris.c \
	gameutil.c \
	svgutil.c \
	properties.c \
	bar.c \
	file_selector.c \
	images_selector.c \
	help.c \
	pixbuf_util.c \
	board.h board.c \
	soundutil.c \
	cursor.c \
	bonus.c \
	timer.c \
	score.c \
	skin.c \
	config.c \
	about.c \
	log.c \
	sdlplayer.c \
	assetml.c

$(libgcompris_1_la_OBJECTS): $(marshal_sources)

marshal_sources =				\
	gcompris-marshal.c			\
	gcompris-marshal.h

BUILT_SOURCES =					\
	$(marshal_sources)

gcompris-marshal.h: gcompris-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=gnome_canvas_marshal > $@
gcompris-marshal.c: gcompris-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --body --prefix=gnome_canvas_marshal > $@

EXTRA_DIST = \
	gcompris-marshal.list	\
	libgcompris-1.0.pc.in	\
	libgcompris-1.0-uninstalled.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgcompris-1.0.pc

DONT_DIST_SOURCE = $(marshal_sources)

dist-hook:
	for file in $(DONT_DIST_SOURCE) ; do \
	    rm -f $(distdir)/$$file ; \
	done

install-data-local: install-libtool-import-lib

uninstall-local: uninstall-libtool-import-lib

clean-local:
	rm -f gcompris-marshal.*
