
%.assetml:       %.assetml.in       $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -x -u -c $(top_builddir)/po/.intltool-merge-cache

assetmldir = $(prefix)/$(DATADIRNAME)/$(ASSETML_DIR)

assetml_in_files = \
	gcompris_misc_es.assetml.in

assetml_DATA = $(assetml_in_files:.assetml.in=.assetml)

@INTLTOOL_XML_RULE@



