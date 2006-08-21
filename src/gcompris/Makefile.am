## Process this file with automake to produce Makefile.in
if PYTHON_PLUGIN
python_ldadd = $(PYTHON_LIBS)
python_ldflags = -u Py_Initialize
else
python_ldadd =
python_ldflags =
endif

if USE_SQLITE
sqlite_ldadd = $(SQLITE3_LIBS)
sqlite_cflags = $(SQLITE3_CFLAGS)
else
sqlite_ldadd =
sqlite_cflags =
endif

#if USE_CAIRO
#cairo_ldflags = $(CAIRO_LIBS)
#cairo_cflags = $(CAIRO_CFLAGS)
#cairo_h = gcompris-cairo.h
#cairo_c = gcompris-cairo.c
#else
#cairo_ldflags =
#cairo_cflags =
#cairo_h =
#cairo_c =
#endif

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

bin_PROGRAMS = gcompris

INCLUDES = \
	-I$(top_srcdir)/intl \
	-DDATADIR=\""$(datadir)"\" \
	$(GCOMPRIS_CFLAGS) \
	$(GNOME_INCLUDEDIR) \
	$(XML_CFLAGS) \
	$(SDL_CFLAGS) \
	-DGNOME_DISABLE_DEPRECATED \
        $(sqlite_cflags) \
	$(GNET_CFLAGS) \
	$(cairo_cflags)

gcompris_SOURCES = \
	main.c

gcompris_LDFLAGS = \
	-export-dynamic $(python_ldflags) $(SDL_LIBS) $(cairo_ldflags)


gcompris_LDADD = \
	$(LIBICONV)		\
        $(INTLLIBS) \
	$(python_ldadd) \
	$(top_builddir)/src/gcompris/libgcompris-1.la \
	$(LIBPOPT)


#
# Gcompris library
# 
lib_LTLIBRARIES = libgcompris-1.la

libgcompris_1_la_LIBADD = \
	$(GCOMPRIS_LIBS) $(sqlite_ldadd) $(XML_LIBS) $(GNET_LIBS)

libgcompris_1_la_LDFLAGS = $(no_undefined) \
	-version-info $(LIBGCOMPRIS_CURRENT):$(LIBGCOMPRIS_REVISION):$(LIBGCOMPRIS_AGE) 

libgcomprisincludedir = $(includedir)/libgcompris-1.0/libgcompris

libgcomprisinclude_HEADERS =			\
	about.h \
	anim.h \
	binreloc.h \
	bonus.h \
	gc_core.h \
	cursor.h \
	gameutil.h \
	gc_net.h \
	gcompris.h \
	gcompris-board.h \
	board_config.h \
	gcompris_alphabeta.h \
	gcompris_config.h \
	gcompris_db.h \
	gcompris_im.h \
	gcompris_files.h \
	wordlist.h \
	pixbuf_util.h \
	plugin.h \
	profile.h \
	properties.h \
	score.h \
	skin.h \
	soundutil.h \
	timer.h \
	$(cairo_h)

libgcompris_1_la_SOURCES =			\
	$(libgcomprisinclude_HEADERS)	\
	about.c \
	anim.c \
	bar.c \
	binreloc.c \
	board.h board.c \
	board_config.h board_config.c \
	bonus.c \
	config.c \
	cursor.c \
	dialog.c \
	file_selector.c \
	gameutil.c \
	gc_net.c \
	gcompris.c \
	gcompris_alphabeta.c \
	gcompris_confirm.c \
	gcompris_db.c \
	gcompris_files.c \
	gcompris_im.c \
	help.c \
	images_selector.c \
	log.c \
	menu.c \
	pixbuf_util.c \
	profile.c \
	properties.c \
	score.c \
	sdlplayer.c \
	skin.c \
	soundutil.c \
	timer.c \
	wordlist.c \
        $(profile_c) \
	$(cairo_c)

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
	libgcompris-1.0-uninstalled.pc.in \
	Makefile.mingw gcomprisrc.rc


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
	rm -f $(marshal_sources)
