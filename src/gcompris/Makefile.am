## Process this file with automake to produce Makefile.in

INCLUDES = \
	-I$(top_srcdir)/intl \
	$(GNOME_INCLUDEDIR) $(GCONF_CFLAGS)

bin_PROGRAMS = gcompris

gcompris_SOURCES = \
	pixbuf_util.c pixbuf_util.h \
	gcompris.c gcompris.h gcompris-board.h \
	properties.c properties.h \
	gameutil.c gameutil.h \
	bar.c \
	help.c \
	plugin.h \
	pluginenum.c pluginenum.h board.h board.c \
	bonus.c bonus.h \
	score.c score.h \
	config.c gcompris_config.h \
	about.c about.h

gcompris_LDADD = \
	$(GNOME_LIBDIR) $(GNOMEUI_LIBS) \
	$(GNOME_XML_LIB) $(INTLLIBS) \
	$(GDK_PIXBUF_LIBS) \
	$(GCONF_LIBS) $(GTKHTML_LIBS)

