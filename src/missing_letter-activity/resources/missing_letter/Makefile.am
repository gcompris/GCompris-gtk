
xmldir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/missing_letter

xml_in_files = \
	board1.xml.in \
	board2.xml.in \
	board3.xml.in \
	board4.xml.in \
	board5.xml.in

xml_DATA = $(xml_in_files:.xml.in=.xml)

#
# Do not use the INTLTOOL_XML_RULE, we don't need to embed the translations
# in the file themselves. GCompris pick the translations from the po file at
# runtime.
#
$(xml_DATA): %.xml: %.xml.in
	sed -e "s/\(<\{1\}\/*\)_/\1/g" $< > $@


imgdir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/missing_letter
img_DATA = \
	avion.png \
	ballon.png \
	banana.png \
	bed.png \
	bottle.png \
	cake.png \
	car.png \
	cartable.png \
	chien.png \
	fish.png \
	maison.png \
	missingletter-bg.jpg \
	pomme.png

EXTRA_DIST = $(img_DATA) ${xml_in_files}
CLEANFILES = $(xml_DATA)
