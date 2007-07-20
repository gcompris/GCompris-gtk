
xmldir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/doubleentry

xml_in_files = \
	board1_0.xml.in \
	board2_0.xml.in \
	board3_0.xml.in

xml_DATA = $(xml_in_files:.xml.in=.xml)

#
# Do not use the INTLTOOL_XML_RULE, we don't need to embed the translations
# in the file themselves. GCompris pick the translations from the po file at
# runtime.
#
$(xml_DATA): %.xml: %.xml.in
	sed -e "s/\(<\{1\}\/*\)_/\1/g" $< > $@

imgdir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/doubleentry
img_DATA = \
	circle-r0-g2.png \
	circle-r0-g3.png \
	circle-r0-g4.png \
	circle-r2-g0.png \
	circle-r2-g2.png \
	circle-r2-g3.png \
	circle-r2-g4.png \
	circle-r3-g0.png \
	circle-r3-g2.png \
	circle-r3-g3.png \
	circle-r3-g4.png \
	circle-r4-g0.png \
	circle-r4-g2.png \
	circle-r4-g3.png \
	circle-r4-g4.png \
	d-entry_banane_1.png \
	d-entry_banane_2.png \
	d-entry_banane_3.png \
	d-entry_galette_1.png \
	d-entry_galette_2.png \
	d-entry_galette_3.png \
	d-entry_pomme_1.png \
	d-entry_pomme_2.png \
	d-entry_pomme_3.png \
	doubleentry-bg.png

EXTRA_DIST = $(img_DATA)
