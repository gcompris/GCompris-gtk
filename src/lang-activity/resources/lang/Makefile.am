xmldir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/lang

xml_in_files = \
	lang.xml.in

xml_DATA = $(xml_in_files:.xml.in=.xml)

#
# Do not use the INTLTOOL_XML_RULE, we don't need to embed the translations
# in the file themselves. GCompris pick the translations from the po file at
# runtime.
#
$(xml_DATA): %.xml: %.xml.in
	sed -e "s/\(<\{1\}\/*\)_/\1/g" $< > $@


imgdir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/lang/numbers
img_DATA = \
	numbers/banana.svg \
	numbers/empty.svg \
	numbers/lemon.svg \
	numbers/mango.svg \
	numbers/one.svg \
	numbers/orange.svg \
	numbers/peach.svg \
	numbers/pear.svg \
	numbers/pineapples.svg \
	numbers/raspberries.svg \
	numbers/strawberries.svg



EXTRA_DIST = $(img_DATA) ${xml_in_files}
CLEANFILES = $(xml_DATA)
