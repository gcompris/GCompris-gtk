SUBDIRS = resources

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
	libchess.la

libdir = $(plugindir)
INCLUDES = 	-I$(top_builddir)/src \
	-I$(top_builddir)/intl \
	-I$(top_builddir)/src/goocanvas/src \
	$(GCOMPRIS_CFLAGS) \
	$(XML_CFLAGS) \
	$(dll_export)


libchess_la_LDFLAGS = $(shared) $(no_undefined) -module -avoid-version $(gc_libs)
libchess_la_LIBADD =
libchess_la_SOURCES = chess.c \
	chess_makros.h \
	chess_notation.c \
	chess_notation.h \
	chess_position.c \
	chess_position.h


xmldir = $(pkgdatadir)/@PACKAGE_DATA_DIR@

xml_in_files = \
	chess_computer.xml.in


xml_DATA = $(xml_in_files:.xml.in=.xml)

# Do not use the INTLTOOL_XML_RULE, we don't need to embed the translations
# in the file themselves. GCompris pick the translations from the po file at
# runtime.
#
$(xml_DATA): %.xml: %.xml.in
	sed -e "s/\(<\{1\}\/*\)_/\1/g" $< > $@

# Use this directive and instead of the one above to embed the translations
# in the xml files directly, this is needed for the tool gcompris2spip
#@INTLTOOL_XML_RULE@


icondir = $(pkgdatadir)/@PACKAGE_DATA_DIR@
icon_DATA = chess_computer.svg

EXTRA_DIST = $(icon_DATA) ${xml_in_files} init_path.sh
CLEANFILES = $(xml_DATA)
