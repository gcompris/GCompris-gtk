
xmldir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/advanced_colors

xml_in_files = \
	board1.xml.in \
	board2.xml.in \
	board3.xml.in \
	board4.xml.in \
	board5.xml.in \
	board6.xml.in \
	board7.xml.in \
	board8.xml.in \
	board9.xml.in

xml_DATA = $(xml_in_files:.xml.in=.xml)

#
# Do not use the INTLTOOL_XML_RULE, we don't need to embed the translations
# in the file themselves. GCompris pick the translations from the po file at
# runtime.
#
$(xml_DATA): %.xml: %.xml.in
	sed -e "s/\(<\{1\}\/*\)_/\1/g" $< > $@


CLEANFILES = $(xml_DATA)
