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
	numbers/10_litchis.jpg \
	numbers/1_mango.jpg \
	numbers/2_pears.jpg \
	numbers/3_apples.jpg \
	numbers/4_bananas.jpg \
	numbers/5_lemons.jpg \
	numbers/6_oranges.jpg \
	numbers/7_strawberries.jpg \
	numbers/8_guavas.jpg \
	numbers/9_apricots.jpg

EXTRA_DIST = $(img_DATA) ${xml_in_files}
CLEANFILES = $(xml_DATA)
