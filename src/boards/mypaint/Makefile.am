PYTHON_INCLUDES=$(PYTHON_CFLAGS)
INCLUDES = $(PYTHON_INCLUDES) $(PYGTK_CFLAGS)

if PLATFORM_WIN32
shared = -shared
no_undefined = -no-undefined
else
shared =
no_undefined =
endif


#defsdir = $(datadir)/pygtk/2.0/defs

# gcompris will put this lib here.
pkgpyexecdir=$(plugindir)/pythonlib

# mydrawwidget binding
pkgpyexec_LTLIBRARIES = mydrawwidget.la

mydrawwidget_la_CFLAGS = $(MYPAINT_CFLAGS)
mydrawwidget_la_LDFLAGS = $(shared) $(no_undefined) $(common_ldflags)-module  -export-symbols-regex initmydrawwidget
mydrawwidget_la_LIBADD = $(MYPAINT_LIBS) $(PYTHON_LIBS)

nodist_mydrawwidget_la_CFLAGS = $(MYPAINT_CFLAGS)
nodist_mydrawwidget_la_SOURCES = \
	mydrawwidget.defs.c

mydrawwidget_la_SOURCES = \
	brush_dab.c \
	brush_dab.h \
	gtkmybrush.c \
	gtkmybrush.h \
	gtkmydrawwidget.c \
	gtkmydrawwidget.h \
	helpers.c \
	helpers.h \
	mydrawwidgetmodule.c \
	surface.c \
	surface.h

CLEANFILES = mydrawwidget.c mydrawwidget.defs mydrawwidget.defs.c brushsettings.h $(wildcard *.pyc)

EXTRA_DIST = mydrawwidget.override

# PYTHON PART
pyboarddir = $(PYTHON_PLUGIN_DIR)
pyboard_SCRIPTS = mypaint.py

mypaintlibdir = $(PYTHON_PLUGIN_DIR)/mypaint

EXTRA_DIST += caller_owns_return.py generate.py mypaint mypaint.py

mypaintlib_PYTHON = \
	application.py \
	brush.py \
	brushselectionwindow.py \
	brushsettings.py \
	brushsettingswindow.py \
	colorselectionwindow.py \
	drawwindow.py \
	functionwindow.py \
	helpers.py \
	infinitemydrawwidget.py \
	rename_brushes.py

brushsettings.h:	generate.py brushsettings.py
	$(PYTHON) ./generate.py

gtkmydrawwidget.c: brushsettings.h gtkmydrawwidget.h

gtkmybrush.c:	brushsettings.h gtkmybrush.h

mydrawwidget.defs.c: mydrawwidget.defs mydrawwidget.override
	$(PYGTK_CODEGEN) --prefix mydrawwidget \
	--register $(PYGTK_DEFSDIR)/gdk-types.defs \
	--register $(PYGTK_DEFSDIR)/gtk-types.defs \
	--override mydrawwidget.override \
	mydrawwidget.defs > mydrawwidget.defs.c

mydrawwidget.defs: gtkmydrawwidget.h gtkmybrush.h surface.h Makefile
	$(PYGTK_H2DEF) gtkmydrawwidget.h gtkmybrush.h > mydrawwidget.defs
	$(PYTHON) ./caller_owns_return.py mydrawwidget.defs get_nonwhite_as_pixbuf get_as_pixbuf


