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
	libtangram.la

libdir = $(plugindir)
INCLUDES = 	-I$(top_srcdir)/src \
	-I$(top_srcdir)/intl \
	$(GCOMPRIS_CFLAGS) \
	$(XML_CFLAGS) \
	$(dll_export)


libtangram_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libtangram_la_LIBADD =
libtangram_la_SOURCES = gtans_callbacks.c \
	gtans_callbacks.h \
	gtans.c \
	gtans.h \
	gtans_interface.c \
	gtans_interface.h \
	gtans_support.c \
	gtans_support.h

