## Process this file with automake to produce Makefile.in

INCLUDES = \
	-I$(top_srcdir)/intl \
	-I$(top_srcdir)/src/unicode \
	$(GNOME_INCLUDEDIR) \
	$(GDK_PIXBUF_CFLAGS) \
	$(XML_CFLAGS)

bin_PROGRAMS = gcompris gcompris-edit

#noinst_LIBRARIES = \
#	libgcompris.a

libgcompris_a_SOURCES = \
	pixbuf_util.c pixbuf_util.h \
	gcompris.c gcompris.h gcompris-board.h \
	properties.c properties.h \
	gameutil.c gameutil.h \
	soundutil.c soundutil.h \
	bar.c \
	help.c \
	plugin.h \
	pluginenum.c pluginenum.h board.h board.c \
	bonus.c bonus.h \
	timer.c timer.h \
	score.c score.h \
	config.c gcompris_config.h \
	about.c about.h \
	cursor.c cursor.h \
	images_selector.c


gcompris_SOURCES = \
	main.c \
	$(libgcompris_a_SOURCES)

gcompris_LDADD = \
	-L$(top_builddir)/src/unicode	\
	-lgcompris_unicode		\
	$(LIBICONV)			\
	$(GNOME_LIBDIR) $(GNOMEUI_LIBS) \
	$(XML_LIBS) $(INTLLIBS) \
	$(GDK_PIXBUF_LIBS) 


gcompris_edit_SOURCES = \
	gcompris-edit.c \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h \
	$(libgcompris_a_SOURCES)

gcompris_edit_LDADD = \
	-L$(top_builddir)/src/unicode	\
	-lgcompris_unicode		\
	$(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(INTLLIBS) \
	$(XML_LIBS) $(GDK_PIXBUF_LIBS)
