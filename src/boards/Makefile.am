if PYTHON_PLUGIN
python_plugin = libpython.la
python_subdir = python
else
python_plugin =
python_subdir =
endif

SUBDIRS= $(python_subdir)

lib_LTLIBRARIES = \
	libalgebra.la \
	libalgebra_guesscount.la \
	libcanal_lock.la \
	libchess.la \
	libclick_on_letter.la \
	libclickgame.la \
	libclockgame.la \
	libcolors.la \
	libdraw.la \
	liberase.la \
	libenumerate.la \
	libfifteen.la \
	libgletters.la \
	libhanoi.la \
	libimageid.la \
	libleftright.la \
	libsuperbrain.la \
	libmachpuzzle.la \
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
	$(python_plugin)

libdir = $(plugindir)
INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/intl \
	$(GCOMPRIS_CFLAGS) \
	$(XML_CFLAGS) \
	$(PYTHON_CFLAGS) \
	$(PYGTK_CFLAGS)

libmenu_la_LDFLAGS = -module -avoid-version
libmenu_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libmenu_la_SOURCES = menu.c


libclickgame_la_LDFLAGS = -module -avoid-version
libclickgame_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libclickgame_la_SOURCES = clickgame.c

libalgebra_la_LDFLAGS = -module -avoid-version
libalgebra_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libalgebra_la_SOURCES = algebra.c


libclockgame_la_LDFLAGS = -module -avoid-version
libclockgame_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libclockgame_la_SOURCES = clockgame.c


libshapegame_la_LDFLAGS = -module -avoid-version
libshapegame_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libshapegame_la_SOURCES = shapegame.c


libwordsgame_la_LDFLAGS = -module -avoid-version
libwordsgame_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libwordsgame_la_SOURCES = wordsgame.c


libgletters_la_LDFLAGS = -module -avoid-version
libgletters_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libgletters_la_SOURCES = gletters.c


libplanegame_la_LDFLAGS = -module -avoid-version
libplanegame_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libplanegame_la_SOURCES = planegame.c


libsmallnumbers_la_LDFLAGS = -module -avoid-version
libsmallnumbers_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libsmallnumbers_la_SOURCES = smallnumbers.c


libparatrooper_la_LDFLAGS = -module -avoid-version
libparatrooper_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libparatrooper_la_SOURCES = paratrooper.c


libreading_la_LDFLAGS = -module -avoid-version
libreading_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libreading_la_SOURCES = reading.c


libdraw_la_LDFLAGS = -module -avoid-version
libdraw_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libdraw_la_SOURCES = draw.c


libmemory_la_LDFLAGS = -module -avoid-version
libmemory_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libmemory_la_SOURCES = memory.c


libimageid_la_LDFLAGS = -module -avoid-version
libimageid_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libimageid_la_SOURCES = imageid.c


libmissingletter_la_LDFLAGS = -module -avoid-version
libmissingletter_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libmissingletter_la_SOURCES = missingletter.c


libclick_on_letter_la_LDFLAGS = -module -avoid-version
libclick_on_letter_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libclick_on_letter_la_SOURCES = click_on_letter.c

librailroad_la_LDFLAGS = -module -avoid-version
librailroad_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
librailroad_la_SOURCES = railroad.c

liberase_la_LDFLAGS = -module -avoid-version
liberase_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
liberase_la_SOURCES = erase.c

libalgebra_guesscount_la_LDFLAGS = -module -avoid-version
libalgebra_guesscount_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libalgebra_guesscount_la_SOURCES = algebra_guesscount.c

libleftright_la_LDFLAGS = -module -avoid-version
libleftright_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libleftright_la_SOURCES = leftright.c

libcolors_la_LDFLAGS = -module -avoid-version
libcolors_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libcolors_la_SOURCES = colors.c

libtarget_la_LDFLAGS = -module -avoid-version
libtarget_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libtarget_la_SOURCES = target.c

libmaze_la_LDFLAGS = -module -avoid-version
libmaze_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libmaze_la_SOURCES = maze.c

libreversecount_la_LDFLAGS = -module -avoid-version
libreversecount_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libreversecount_la_SOURCES = reversecount.c

libcanal_lock_la_LDFLAGS = -module -avoid-version
libcanal_lock_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libcanal_lock_la_SOURCES = canal_lock.c

libadvanced_colors_la_LDFLAGS = -module -avoid-version
libadvanced_colors_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libadvanced_colors_la_SOURCES = advanced_colors.c

libsuperbrain_la_LDFLAGS = -module -avoid-version
libsuperbrain_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libsuperbrain_la_SOURCES = superbrain.c

libread_colors_la_LDFLAGS = -module -avoid-version
libread_colors_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libread_colors_la_SOURCES = read_colors.c

libchess_la_LDFLAGS = -module -avoid-version
libchess_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libchess_la_SOURCES = chess.c chess_position.c chess_notation.c \
	chess_notation.h chess_position.h chess_makros.h

libenumerate_la_LDFLAGS = -module -avoid-version
libenumerate_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libenumerate_la_SOURCES = enumerate.c

libmoney_la_LDFLAGS = -module -avoid-version
libmoney_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libmoney_la_SOURCES = money.c money_widget.c money_widget.h

libtraffic_la_LDFLAGS = -module -avoid-version
libtraffic_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libtraffic_la_SOURCES = traffic.c traffic.h

libmachpuzzle_la_LDFLAGS = -module -avoid-version
libmachpuzzle_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libmachpuzzle_la_SOURCES = machpuzzle.c

libsubmarine_la_LDFLAGS = -module -avoid-version
libsubmarine_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libsubmarine_la_SOURCES = submarine.c

libhanoi_la_LDFLAGS = -module -avoid-version
libhanoi_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libhanoi_la_SOURCES = hanoi.c

libfifteen_la_LDFLAGS = -module -avoid-version
libfifteen_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libfifteen_la_SOURCES = fifteen.c


libpython_la_LDFLAGS = -export-dynamic -module -avoid-version
libpython_la_LIBADD = $(top_builddir)/src/gcompris/libgcompris-1.la
libpython_la_SOURCES = python.c \
	py-gcompris-board.c	py-gcompris-board.h	\
	py-mod-gcompris.c	py-mod-gcompris.h	\
	py-mod-utils.c		py-mod-utils.h		\
	py-mod-bonus.c		py-mod-bonus.h		\
	py-mod-score.c		py-mod-score.h		\
	py-mod-timer.c		py-mod-timer.h		\
	py-mod-sound.c		py-mod-sound.h		\
	py-mod-skin.c		py-mod-skin.h


EXTRA_DIST = README

