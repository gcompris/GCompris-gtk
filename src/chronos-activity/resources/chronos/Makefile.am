
xmldir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/chronos

xml_in_files = \
	board1_0.xml.in \
	board1_1.xml.in \
	board1_2.xml.in \
	board2_0.xml.in \
	board3_0.xml.in \
	board3_1.xml.in \
	board3_2.xml.in \
	board3_3.xml.in \
	board3_4.xml.in \
	board4_0.xml.in \
	board4_1.xml.in \
	board4_2.xml.in \
	board4_3.xml.in

xml_DATA = $(xml_in_files:.xml.in=.xml)

#
# Do not use the INTLTOOL_XML_RULE, we don't need to embed the translations
# in the file themselves. GCompris pick the translations from the po file at
# runtime.
#
$(xml_DATA): %.xml: %.xml.in
	sed -e "s/\(<\{1\}\/*\)_/\1/g" $< > $@

imgdir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/chronos
img_DATA = \
	1955ds19.png \
	1.png \
	2.png \
	3.png \
	1.ogg \
	2.ogg \
	3.ogg \
	4.png \
	autumn.png \
	bell_X1.png \
	benz1885.png \
	bg.jpg \
	bleriot.png \
	bolle1878.png \
	celerifere.png \
	chronos-tuxtree1.png \
	chronos-tuxtree2.png \
	chronos-tuxtree3.png \
	chronos-tuxtree4.png \
	Eole.png \
	fardier.png \
	garden1.png \
	garden2.png \
	garden3.png \
	garden4.png \
	helico_cornu.png \
	lancia1923.png \
	lindbergh.png \
	mongolfiere.png \
	rafale.png \
	renault1899.png \
	spring.png \
	st_rocket.png \
	summer.png \
	winter.png \
	wright_flyer.png

EXTRA_DIST = $(img_DATA) ${xml_in_files}
CLEANFILES = $(xml_DATA)
