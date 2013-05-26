# Passthrough, do not break uplevel make rule
install-activity:

if PYTHON_PLUGIN
python_plugin = libpython.la
python_subdir = python
else
python_plugin =
python_subdir =
endif


if USE_SQLITE
sqlite_ldflags = $(SQLITE3_LIBS)
sqlite_cflags = $(SQLITE3_CFLAGS)
else
sqlite_ldflags =
sqlite_cflags =
endif

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

SUBDIRS= $(python_subdir)

lib_LTLIBRARIES = \
	libmenu.la \
	$(python_plugin)

libdir = $(plugindir)
AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/intl \
	-I$(top_srcdir)/src/goocanvas/src \
	$(GCOMPRIS_CFLAGS) \
	$(XML_CFLAGS) \
	$(PYTHON_CFLAGS) \
	$(PYGTK_CFLAGS) \
	$(sqlite_cflags) \
	$(dll_export)

libmenu_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libmenu_la_LIBADD =
libmenu_la_SOURCES = menu2.c

libpython_la_LDFLAGS = -export-dynamic $(shared) $(no_undefined) -module -avoid-version $(gc_libs) $(PYTHON_LIBS)

libpython_la_LIBADD =
libpython_la_CFLAGS = -fno-strict-aliasing $(PYGTK_CFLAGS) \
	 -DPYGOOCANVAS_MAJOR_VERSION=0 \
	 -DPYGOOCANVAS_MINOR_VERSION=0 \
	 -DPYGOOCANVAS_MICRO_VERSION=0

libpython_la_SOURCES = python.c \
	goocanvas.c goocanvasmodule.c \
	py-gcompris-board.c	py-gcompris-board.h	\
	py-gcompris-properties.c	py-gcompris-properties.h	\
	py-gcompris-profile.c	py-gcompris-profile.h	\
	py-gcompris-wordlist.c	py-gcompris-wordlist.h	\
	py-gcompris-boardconfig.c py-gcompris-boardconfig.h \
	py-gcompris-user.c	\
	py-gcompris-class.c	\
	py-gcompris-group.c	\
	py-mod-gcompris.c	py-mod-gcompris.h	\
	py-mod-utils.c		py-mod-utils.h		\
	py-mod-admin.c		py-mod-admin.h		\
	py-mod-bonus.c		py-mod-bonus.h		\
	py-mod-score.c		py-mod-score.h		\
	py-mod-timer.c		py-mod-timer.h		\
	py-mod-sound.c		py-mod-sound.h		\
	py-mod-skin.c		py-mod-skin.h		\
	py-mod-anim.c		py-mod-anim.h


EXTRA_DIST = README \
	Makefile.mingw


