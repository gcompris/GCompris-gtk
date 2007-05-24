INCLUDES = $(PYTHON_INCLUDES) $(PYGTK_CFLAGS)

defsdir = $(datadir)/pygtk/2.0/defs
defs_DATA = cccanvas.defs

EXTRA_DIST = $(defs_DATA)

pkgpythondir = $(pyexecdir)/gtk-2.0
pkgpyexecdir = $(pyexecdir)/gtk-2.0

common_ldflags = -module -avoid-version

# libcccanvas binding
pkgpyexec_LTLIBRARIES = cccanvas.la
cccanvas_la_CFLAGS = $(LIBCCC_CFLAGS) $(CCC_CFLAGS) $(PYCAIRO_CFLAGS) -I../ccc
cccanvas_la_LDFLAGS = $(common_ldflags) -export-symbols-regex initcccanvas
cccanvas_la_LIBADD = $(CCCANVAS_LIBS) $(CCC_LIBS) $(PYCAIRO_LIBS) -L../ccc/.libs -lccc-0.1
nodist_cccanvas_la_CFLAGS = $(LIBCCC_CFLAGS) $(CCC_CFLAGS) $(PYCAIRO_CFLAGS) -I../ccc
cccanvas_la_SOURCES = cccanvasmodule.c
nodist_cccanvas_la_SOURCES = cccanvas.c
CLEANFILES = cccanvas.c
EXTRA_DIST += cccanvas.override
cccanvas.c: cccanvas.defs cccanvas.override

.defs.c:
	(cd $(srcdir) \
	 && $(PYGTK_CODEGEN) \
	    --register $(PYGTK_DEFSDIR)/gdk-types.defs \
	    --register $(PYGTK_DEFSDIR)/gtk-types.defs \
	    --register $(PYGTK_DEFSDIR)/pango-types.defs \
	    --override $*.override \
	    --prefix py$* $*.defs) > gen-$*.c \
	&& cp gen-$*.c $*.c \
	&& rm -f gen-$*.c

