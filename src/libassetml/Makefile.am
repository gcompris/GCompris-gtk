## Process this file with automake to produce Makefile.in

SUBDIRS = . tools test

INCLUDES = \
	-I$(top_srcdir)/intl				\
	-I$(top_srcdir) 				\
	-I$(top_builddir)				\
	$(WARN_CFLAGS) 					\
	$(GLIB_CFLAGS)					\
	$(GOBJECT_CFLAGS)				\
	$(XML_CFLAGS)					\
	-DG_LOG_DOMAIN=\"Assetml\"			\
	-DVERSION=\"$(VERSION)\"

lib_LTLIBRARIES = libassetml.la

libassetml_la_LIBADD = \
	$(LIBASSETML_LIBS)			\
	$(INTLLIBS)

libassetmlincludedir = $(includedir)/libassetml-1.0/libassetml

libassetmlinclude_HEADERS =			\
	assetml.h

BUILT_SOURCES =					\
	assetml-marshal.c			\
	assetml-marshal.h

libassetml_la_SOURCES =			\
	$(libassetmlinclude_HEADERS)		\
	$(BUILT_SOURCES)			\
	assetml.c


assetml-marshal.h: assetml-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=assetml_marshal > $@
assetml-marshal.c: assetml-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --body --prefix=assetml_marshal > $@.tmp
	echo '/* This file has been automatically generated.  Do not edit.  */' >$@
	echo '#include "'$*.h'"' >>$@
	cat $@.tmp >>$@
	rm -f $@.tmp

EXTRA_DIST = \
	assetml-marshal.list	\
	libassetml.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libassetml.pc

DONT_DIST_SOURCE = $(BUILT_SOURCES)

dist-hook:
	for file in $(DONT_DIST_SOURCE) ; do \
	    rm -f $(distdir)/$$file ; \
	done
