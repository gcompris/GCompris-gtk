## Process this file with automake to produce Makefile.in

INCLUDES = \
	-I$(top_srcdir)/intl				\
	-I$(top_srcdir) 				\
	-I$(top_srcdir)/src/libassetml			\
	-I$(top_builddir)				\
	$(WARN_CFLAGS) 					\
	$(GCOMPRIS_CFLAGS)				\
	-DG_LOG_DOMAIN=\"Assetml\"			\
	-DVERSION=\"$(VERSION)\"

bin_PROGRAMS = assetml-query

assetml_query_SOURCES = \
	assetml-query.c

assetml_query_LDADD = \
	$(GCOMPRIS_LIBS)	\
	$(top_builddir)/src/libassetml/libassetml.la
