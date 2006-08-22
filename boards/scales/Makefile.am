
xmldir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/scales

xml_in_files = \
	board1_0.xml.in \
	board1_1.xml.in \
	board1_2.xml.in \
	board1_3.xml.in \
	board1_4.xml.in \
	board1_5.xml.in \
	board2_0.xml.in \
	board2_1.xml.in \
	board2_2.xml.in \
	board2_3.xml.in \
	board2_4.xml.in \
	board2_5.xml.in \
	board3_0.xml.in \
	board3_1.xml.in \
	board3_2.xml.in \
	board3_3.xml.in \
	board3_4.xml.in \
	board3_5.xml.in

xml_DATA = $(xml_in_files:.xml.in=.xml)

#
# Do not use the INTLTOOL_XML_RULE, we don't need to embed the translations
# in the file themselves. GCompris pick the translations from the po file at
# runtime.
#
$(xml_DATA): %.xml: %.xml.in
	sed -e "s/\(\<\)\/*_//g" $< > $@


CLEANFILES = $(xml_DATA)
