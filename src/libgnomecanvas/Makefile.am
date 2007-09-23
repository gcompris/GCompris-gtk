
INCLUDES = \
	-I$(top_builddir) 				\
	-I$(top_builddir)				\
	-I$(top_builddir)/src				\
	$(WARN_CFLAGS) 					\
	$(GCOMPRIS_CFLAGS) \
	$(FT2_CFLAGS)					\
        -DGNOMECANVASLIBDIR=\""$(libdir)"\" 		\
        -DGNOMECANVASDATADIR=\""$(datadir)"\" 		\
        -DGNOMECANVASPIXMAPDIR=\""$(datadir)/pixmaps"\"	\
        -DGNOMECANVASBINDIR=\""$(bindir)"\" 		\
        -DGNOMECANVASLOCALSTATEDIR=\""$(localstatedir)"\" 	\
	-DG_LOG_DOMAIN=\"GnomeCanvas\"

noinst_LIBRARIES = libgnomecanvas-2.a

libgnomecanvas_2_a_LIBADD = \
	$(LIBGNOMECANVAS_LIBS)

libgnomecanvasinclude_headers =			\
	gnome-canvas-shape.h			\
	gnome-canvas-bpath.h			\
	gnome-canvas-clipgroup.h		\
	gnome-canvas-line.h			\
	gnome-canvas-pixbuf.h			\
	gnome-canvas-polygon.h			\
	gnome-canvas-rect-ellipse.h		\
	gnome-canvas-text.h			\
	gnome-canvas-rich-text.h		\
	gnome-canvas-util.h			\
	gnome-canvas-widget.h			\
	gnome-canvas-path-def.h			\
	gnome-canvas.h				\
	libgnomecanvas.h

libgnomecanvas_2_a_SOURCES =			\
	$(libgnomecanvasinclude_headers)	\
	gnome-canvas-shape.c			\
	gnome-canvas-shape-private.h		\
	gnome-canvas-bpath.c			\
	gnome-canvas-path-def.c			\
	gnome-canvas-clipgroup.c		\
	gnome-canvas-i18n.h			\
	gnome-canvas-line.c			\
	gnome-canvas-pixbuf.c			\
	gnome-canvas-polygon.c			\
	gnome-canvas-rect-ellipse.c		\
	gnome-canvas-text.c			\
	gnome-canvas-rich-text.c		\
	gnome-canvas-util.c			\
	gnome-canvas-widget.c			\
	gnome-canvas.c				\
	libgnomecanvastypes.c

$(libgnomecanvas_2_a_OBJECTS): $(marshal_sources)

marshal_sources =                               \
        gnome-canvas-marshal.c                  \
        gnome-canvas-marshal.h

BUILT_SOURCES =                                 \
        $(marshal_sources)

gnome-canvas-marshal.h: gnome-canvas-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=gnome_canvas_marshal > $@
gnome-canvas-marshal.c: gnome-canvas-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --body --prefix=gnome_canvas_marshal > $@

CLEANFILES = $(marshal_sources)

DONT_DIST_SOURCE = $(marshal_sources)

EXTRA_DIST = \
	gnome-canvas-marshal.list
