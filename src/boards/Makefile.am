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

if USE_CAIRO
cairo_ldflags = $(CAIRO_LIBS)
cairo_cflags = $(CAIRO_CFLAGS)
else
cairo_ldflags =
cairo_cflags =
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
	libalgebra.la \
	libalgebra_guesscount.la \
	libawele.la \
	libcanal_lock.la \
	libchess.la \
	libclick_on_letter.la \
	libclickgame.la \
	libclockgame.la \
	libcolors.la \
	libcrane.la \
	liberase.la \
	libenumerate.la \
	libfifteen.la \
	libgletters.la \
	libgtans.la \
	libhanoi.la \
	libhanoi_real.la \
	libimageid.la \
	libleftright.la \
	libsuperbrain.la \
	libmachpuzzle.la \
	libmagic_hat.la \
	libmaze.la \
	libmemory.la \
	libmenu.la \
	libmissingletter.la \
	libmoney.la \
	libparatrooper.la \
	libplanegame.la \
	librailroad.la \
	libreading.la \
	libreversecount.la \
	libshapegame.la \
	libsmallnumbers.la \
	libsubmarine.la \
	libtarget.la \
	libtraffic.la \
	libwordsgame.la \
	libadvanced_colors.la \
	libread_colors.la \
	libdraw.la \
	libwordprocessor.la \
	$(python_plugin)

libdir = $(plugindir)
INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/intl \
	$(GCOMPRIS_CFLAGS) \
	$(XML_CFLAGS) \
	$(PYTHON_CFLAGS) \
	$(PYGTK_CFLAGS) \
	$(sqlite_cflags) \
	$(cairo_cflags) \
	$(dll_export)

libdraw_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libdraw_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libdraw_la_SOURCES = draw.c

libawele_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libawele_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libawele_la_SOURCES = awele.c awele_alphaBeta.c awele_utils.c awele_utils.h awele.h

libcrane_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libcrane_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libcrane_la_SOURCES = crane.c

libmenu_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libmenu_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libmenu_la_SOURCES = menu2.c

libclickgame_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libclickgame_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libclickgame_la_SOURCES = clickgame.c

libalgebra_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libalgebra_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libalgebra_la_SOURCES = algebra.c

libclockgame_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libclockgame_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libclockgame_la_SOURCES = clockgame.c


libshapegame_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs) $(XML_LIBS)
libshapegame_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libshapegame_la_SOURCES = shapegame.c


libwordsgame_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libwordsgame_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libwordsgame_la_SOURCES = wordsgame.c


libgletters_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libgletters_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libgletters_la_SOURCES = gletters.c

libgtans_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libgtans_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libgtans_la_SOURCES = gtans.c gtans_interface.c gtans_support.c gtans_callbacks.c \
	gtans_callbacks.h gtans.h gtans_interface.h gtans_support.h

libplanegame_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libplanegame_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libplanegame_la_SOURCES = planegame.c


libsmallnumbers_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libsmallnumbers_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libsmallnumbers_la_SOURCES = smallnumbers.c


libparatrooper_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libparatrooper_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libparatrooper_la_SOURCES = paratrooper.c


libreading_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libreading_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libreading_la_SOURCES = reading.c


libmemory_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libmemory_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libmemory_la_SOURCES = memory.c


libimageid_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs) $(XML_LIBS)
libimageid_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libimageid_la_SOURCES = imageid.c


libmissingletter_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs) $(XML_LIBS)
libmissingletter_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libmissingletter_la_SOURCES = missingletter.c


libclick_on_letter_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libclick_on_letter_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libclick_on_letter_la_SOURCES = click_on_letter.c

librailroad_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
librailroad_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
librailroad_la_SOURCES = railroad.c

liberase_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
liberase_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
liberase_la_SOURCES = erase.c

libalgebra_guesscount_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libalgebra_guesscount_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libalgebra_guesscount_la_SOURCES = algebra_guesscount.c

libleftright_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libleftright_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libleftright_la_SOURCES = leftright.c

libcolors_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libcolors_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libcolors_la_SOURCES = colors.c

libtarget_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libtarget_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libtarget_la_SOURCES = target.c

libmagic_hat_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libmagic_hat_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libmagic_hat_la_SOURCES = magic_hat.c

libmaze_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libmaze_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libmaze_la_SOURCES = maze.c

libreversecount_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libreversecount_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libreversecount_la_SOURCES = reversecount.c

libcanal_lock_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libcanal_lock_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libcanal_lock_la_SOURCES = canal_lock.c

libadvanced_colors_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs) $(XML_LIBS)
libadvanced_colors_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libadvanced_colors_la_SOURCES = advanced_colors.c

libsuperbrain_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libsuperbrain_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libsuperbrain_la_SOURCES = superbrain.c

libread_colors_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs) $(XML_LIBS)
libread_colors_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libread_colors_la_SOURCES = read_colors.c

libchess_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libchess_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libchess_la_SOURCES = chess.c chess_position.c chess_notation.c \
	chess_notation.h chess_position.h chess_makros.h

libenumerate_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libenumerate_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libenumerate_la_SOURCES = enumerate.c

libmoney_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libmoney_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libmoney_la_SOURCES = money.c money_widget.c money_widget.h

libtraffic_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libtraffic_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libtraffic_la_SOURCES = traffic.c traffic.h

libmachpuzzle_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libmachpuzzle_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libmachpuzzle_la_SOURCES = machpuzzle.c

libsubmarine_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libsubmarine_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libsubmarine_la_SOURCES = submarine.c

libhanoi_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libhanoi_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libhanoi_la_SOURCES = hanoi.c

libhanoi_real_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libhanoi_real_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libhanoi_real_la_SOURCES = hanoi_real.c

libfifteen_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libfifteen_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libfifteen_la_SOURCES = fifteen.c

libwordprocessor_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libwordprocessor_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libwordprocessor_la_SOURCES = wordprocessor.c


libpython_la_LDFLAGS = -export-dynamic $(shared) $(no_undefined) -module -avoid-version $(gc_libs) $(PYTHON_LIBS)
libpython_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libpython_la_CFLAGS = -fno-strict-aliasing $(PYGTK_CFLAGS)
libpython_la_SOURCES = python.c \
	canvas.c canvasmodule.c \
	py-gcompris-board.c	py-gcompris-board.h	\
	py-gcompris-properties.c	py-gcompris-properties.h	\
	py-gcompris-profile.c	py-gcompris-profile.h	\
	py-gcompris-wordlist.c	py-gcompris-wordlist.h	\
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


EXTRA_DIST = README boards2static.pl \
	draw.c Makefile.mingw


