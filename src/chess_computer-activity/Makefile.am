SUBDIRS = activity

if PLATFORM_WIN32
shared = -shared
no_undefined = -no-undefined
gc_libs = $(GCOMPRIS_LIBS)
dll_export = -DBOARD_DLL_EXPORT
else
shared =
no_undefined =
gc_libs =
dll_export =
endif

lib_LTLIBRARIES = \
	libchess.la

libdir = $(plugindir)
INCLUDES = 	-I$(top_srcdir)/src \
	-I$(top_srcdir)/intl \
	$(GCOMPRIS_CFLAGS) \
	$(XML_CFLAGS) \
	$(dll_export)


libchess_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libchess_la_LIBADD =
libchess_la_SOURCES = chess.c \
	chess_makros.h \
	chess_notation.c \
	chess_notation.h \
	chess_position.c \
	chess_position.h

