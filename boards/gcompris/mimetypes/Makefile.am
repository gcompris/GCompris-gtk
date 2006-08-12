xmldir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/gcompris

xml_in_files = \
	mime_type.xml.in

xml_DATA = $(xml_in_files:.xml.in=.xml)

#
# Do not use the INTLTOOL_XML_RULE, we don't need to embed the translations
# in the file themselves. GCompris pick the translations from the po file at
# runtime.
#
$(xml_DATA): %.xml: %.xml.in
	sed -e "s/\(\<\)\/*_//g" $< > $@




