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
	libhanoi_real.la

libdir = $(plugindir)
INCLUDES = 	-I$(top_srcdir)/src \
	-I$(top_srcdir)/intl \
	$(GCOMPRIS_CFLAGS) \
	$(XML_CFLAGS) \
	$(dll_export)



libhanoi_real_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libhanoi_real_la_LIBADD =
libhanoi_real_la_SOURCES = hanoi_real.c
