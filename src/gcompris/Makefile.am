## Process this file with automake to produce Makefile.in

INCLUDES = \
	-I$(top_srcdir)/intl \
	-DDATADIR=\""$(datadir)"\" \
	$(GCOMPRIS_CFLAGS) \
	$(GNOME_INCLUDEDIR) \
	$(XML_CFLAGS) \
	$(PTHREAD_CFLAGS) \
	-DGNOME_DISABLE_DEPRECATED

bin_PROGRAMS = gcompris gcompris-edit
#bin_PROGRAMS = gcompris

#noinst_LIBRARIES = \
#	libgcompris.a

libgcompris_a_SOURCES = \
	gcompris.c \
	gcompris.h gcompris-board.h \
	gameutil.c gameutil.h \
	properties.c properties.h \
	bar.c \
	images_selector.c \
	oggplayer.c \
	help.c \
	pixbuf_util.c pixbuf_util.h \
	board.h board.c \
	soundutil.c soundutil.h \
	cursor.c cursor.h \
	plugin.h \
	pluginenum.c pluginenum.h \
	bonus.c bonus.h \
	timer.c timer.h \
	score.c score.h \
	config.c gcompris_config.h \
	about.c about.h 


gcompris_SOURCES = \
	main.c \
	$(libgcompris_a_SOURCES)


gcompris_LDFLAGS = \
	-export-dynamic \
	-lvorbisfile

gcompris_LDADD = \
	$(LIBICONV)		\
	$(GCOMPRIS_LIBS)	\
	$(XML_LIBS) $(INTLLIBS) \
	$(PTHREAD_LIB) \
	$(VORBISFILE_LIBS) $(VORBIS_LIBS) $(OGG_LIBS) $(AO_LIBS)


gcompris_edit_LDFLAGS = \
	-export-dynamic \
	-lvorbisfile

gcompris_edit_SOURCES = \
	gcompris-edit.c gcompris-edit.h \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h \
	$(libgcompris_a_SOURCES)

gcompris_edit_LDADD = \
	$(GCOMPRIS_LIBS) \
	$(XML_LIBS) $(INTLLIBS) \
	$(PTHREAD_LIB) \
	$(VORBISFILE_LIBS) $(VORBIS_LIBS) $(OGG_LIBS) $(AO_LIBS)
