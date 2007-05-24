NULL=

lib_LTLIBRARIES=libccc-0.1.la

main_HEADERS=\
	ccc.h \
	$(NULL)
maindir=$(includedir)/ccc-$(MILESTONE)

static_headers=\
	cc-brush.h \
	cc-brush-color.h \
	cc-camera.h \
	cc-color.h \
	cc-color-rgb.c \
	cc-circle.h \
	cc-d-rect.h \
	cc-hash-map.h \
	cc-item.h \
	cc-item-view.h \
	cc-line.h \
	cc-pixbuf.h \
	cc-printer.h \
	cc-rectangle.h \
	cc-rounded-rectangle.h \
	cc-shape.h \
	cc-text.h \
	cc-utils.h \
	cc-view.h \
	cc-view-cell-renderer.h \
	cc-view-png.h \
	cc-view-widget.h \
	$(NULL)
built_headers=\
	cc-enumerations.h \
	$(NULL)
ccc_HEADERS=\
	$(static_headers) \
	$(built_headers) \
	$(NULL)
cccdir=$(maindir)/ccc

BUILT_SOURCES=\
	cc-enumerations.c \
	$(built_headers) \
	cc-marshallers.c \
	cc-marshallers.h \
	$(NULL)
libccc_0_1_la_SOURCES=\
	$(ccc_HEADERS) \
	$(BUILT_SOURCES) \
	cc-brush.c \
	cc-brush-color.c \
	cc-camera.c \
	cc-color.c \
	cc-color-hsv.c \
	cc-circle.c \
	cc-d-rect.c \
	cc-hash-map.c \
	cc-item.c \
	cc-item-view.c \
	cc-line.c \
	cc-pixbuf.c \
	cc-printer.c \
	cc-rectangle.c \
	cc-rounded-rectangle.c \
	cc-shape.c \
	cc-text.c \
	cc-utils.c \
	cc-view.c \
	cc-view-cell-renderer.c \
	cc-view-png.c \
	cc-view-widget.c \
	glib-helpers.h \
	glib-helpers.c \
	gobject-helpers.h \
	pango-helpers.h \
	pango-helpers.c \
	$(NULL)
libccc_0_1_la_LIBADD=\
	$(CCC_LIBS) \
	$(top_builddir)/cdebug/libcdebug.la \
	$(NULL)

cc-marshallers.h: $(srcdir)/marshallers.list $(srcdir)/Makefile
	@( $(GLIB_GENMARSHAL) --header --prefix=cc_marshal $< ) > xgen-cmh \
	&& (cmp -s xgen-cmh $@ || (echo "Creating $@..."; cp xgen-cmh $@) ) \
	&& rm -f xgen-cmh

cc-marshallers.c: $(srcdir)/marshallers.list $(srcdir)/Makefile
	@( echo "#include <ccc/cc-marshallers.h>"; \
	$(GLIB_GENMARSHAL) --body --prefix=cc_marshal $< ) > xgen-cmc \
	&& (cmp -s xgen-cmc $@ || (echo "Creating $@..."; cp xgen-cmc $@) ) \
	&& rm -f xgen-cmc

cc-enumerations.h: $(static_headers) Makefile cc-enumerations.h.in
	@( cd $(srcdir) && $(GLIB_MKENUMS) --template cc-enumerations.h.in \
	   $(ccc_HEADERS) ) > xgen-gtbh \
	&& (cmp -s xgen-gtbh $@ || (echo "Creating $@..."; cp xgen-gtbh $@) ) \
	&& rm -f xgen-gtbh

cc-enumerations.c: $(static_headers) Makefile cc-enumerations.c.in
	@( cd $(srcdir) && $(GLIB_MKENUMS) --template cc-enumerations.c.in \
	   $(ccc_HEADERS) ) >> xgen-gtbc \
	&& (cmp -s xgen-gtbc $@ || (echo "Creating $@..."; cp xgen-gtbc $@) ) \
	&& rm -f xgen-gtbc

EXTRA_DIST=\
	cc-enumerations.c.in \
	cc-enumerations.h.in \
	marshallers.list \
	$(NULL)

CLEANFILES=\
	$(BUILT_SOURCES) \
	$(NULL)

AM_CPPFLAGS=\
	$(CCC_CFLAGS) \
	-std=c99 \
	-I$(top_builddir) \
	-I$(top_srcdir) \
	$(NULL)

