
%.assetml:       %.assetml.in       $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -x -u -c $(top_builddir)/po/.intltool-merge-cache

assetmldir = $(prefix)/$(DATADIRNAME)/$(ASSETML_DIR)

assetml_in_files = \
	gcompris_flags.assetml.in

assetml_DATA = $(assetml_in_files:.assetml.in=.assetml)

#
# Do not use the INTLTOOL_XML_RULE, we don't need to embed the translations
# in the file themselves. GCompris pick the translations from the po file at
# runtime.
#
$(xml_DATA): %.xml: %.xml.in
	sed -e "s/\(\<\)\/*_//g" $< > $@




