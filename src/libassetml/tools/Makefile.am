## Process this file with automake to produce Makefile.in

INCLUDES = \
	-I$(top_srcdir)/intl				\
	-I$(top_srcdir) 				\
	-I$(top_srcdir)/src/libassetml			\
	-I$(top_builddir)				\
	$(WARN_CFLAGS) 					\
	$(GLIB_CFLAGS)					\
	$(GOBJECT_CFLAGS)				\
	$(XML_CFLAGS)					\
	-DG_LOG_DOMAIN=\"Assetml\"			\
	-DVERSION=\"$(VERSION)\"

bin_PROGRAMS = assetml-query

assetml_query_SOURCES = \
	assetml-query.c

assetml_query_LDADD = \
	$(GLIB_LIBS) $(XML_LIBS)  \
	$(GOBJECT_LIBS) \
	-lpopt \
	$(top_builddir)/src/libassetml/libassetml.la
