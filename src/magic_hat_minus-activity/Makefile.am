SUBDIRS = activity resources

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
	libmagic_hat.la

libdir = $(plugindir)
INCLUDES = 	-I$(top_srcdir)/src \
	-I$(top_srcdir)/intl \
	$(GCOMPRIS_CFLAGS) \
	$(XML_CFLAGS) \
	$(dll_export)


libmagic_hat_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libmagic_hat_la_LIBADD =
libmagic_hat_la_SOURCES = magic_hat.c
