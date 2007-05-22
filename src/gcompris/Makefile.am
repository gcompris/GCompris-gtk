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
shared = -shared
else
no_undefined =
shared =
endif

if WITH_NSBUNDLE
gcompris_nsbundle_m = gcompris-nsbundle.m
gcompris_nsbundle_h = gcompris-nsbundle.h
else
gcompris_nsbundle_m =
gcompris_nsbundle_h =
endif

bin_PROGRAMS = gcompris

INCLUDES = \
	-I$(top_srcdir)/intl \
	-DDATADIR=\""$(datadir)"\" \
	$(GCOMPRIS_CFLAGS) \
	$(GNOME_INCLUDEDIR) \
	$(XML_CFLAGS) \
	-DGNOME_DISABLE_DEPRECATED \
        $(sqlite_cflags) \
	$(GNET_CFLAGS) \
	$(cairo_cflags) \
	$(NSBUNDLE_CPPFLAGS)

gcompris_SOURCES = \
	$(cairo_c) \
	$(cairo_h) \
	$(gcompris_nsbundle_h) \
	$(gcompris_nsbundle_m) \
	about.c \
	about.h \
	anim.c \
	anim.h \
	bar.c \
	binreloc.c \
	binreloc.h \
	board.h board.c \
	board_config.h \
	board_config.h board_config.c \
	bonus.c \
	bonus.h \
	cache.c \
	config.c \
	dialog.c \
	drag.c \
	drag.h \
	file_selector.c \
	gameutil.c \
	gameutil.h \
	gc_core.h \
	gc_net.c \
	gc_net.h \
	gcompris-board.h \
	gcompris.c \
	gcompris.h \
	gcompris_alphabeta.c \
	gcompris_alphabeta.h \
	gcompris_config.h \
	gcompris_confirm.c \
	gcompris_db.c \
	gcompris_db.h \
	gcompris_im.c \
	gcompris_im.h \
	help.c \
	images_selector.c \
	log.c \
	menu.c \
	pixbuf_util.c \
	pixbuf_util.h \
	plugin.h \
	profile.c \
	profile.h \
	properties.c \
	properties.h \
	score.c \
	score.h \
	skin.c \
	skin.h \
	soundutil.c \
	soundutil.h \
	timer.c \
	timer.h \
	wordlist.c \
	wordlist.h

gcompris_LDFLAGS = \
	-export-dynamic $(cairo_ldflags)


gcompris_LDADD = \
	$(GCOMPRIS_LIBS) $(sqlite_ldadd) $(XML_LIBS) $(GNET_LIBS) \
        $(INTLLIBS) $(NSBUNDLE_LDFLAGS)

#	$(LIBICONV)


EXTRA_DIST = \
	Makefile.mingw gcomprisrc.rc


gcompris_nsbundle_CPPFLAGS = -fconstant-string-class=NSConstantString