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

noinst_PROGRAMS = test
#bin_PROGRAMS = test

test_SOURCES = \
	test.c

test_LDADD = \
	$(GCOMPRIS_LIBS)	\
	$(top_builddir)/src/libassetml/libassetml.la
