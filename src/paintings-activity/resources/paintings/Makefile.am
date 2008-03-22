
xmldir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/paintings

xml_in_files = \
	board1_0.xml.in \
	board2_0.xml.in \
	board3_0.xml.in \
	board3_1.xml.in \
	board4_0.xml.in \
	board5_0.xml.in \
	board6_0.xml.in

xml_DATA = $(xml_in_files:.xml.in=.xml)

#
# Do not use the INTLTOOL_XML_RULE, we don't need to embed the translations
# in the file themselves. GCompris pick the translations from the po file at
# runtime.
#
$(xml_DATA): %.xml: %.xml.in
	sed -e "s/\(<\{1\}\/*\)_/\1/g" $< > $@

imgdir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/paintings
img_DATA = \
	bazille_rampart_1.jpg \
	bazille_rampart_2.jpg \
	bazille_rampart_3.jpg \
	bazille_rampart_4.jpg \
	bazille_rampart_5.jpg \
	bazille_rampart_6.jpg \
	cassat_summertime_1.jpg \
	cassat_summertime_2.jpg \
	cassat_summertime_3.jpg \
	cassat_summertime_4.jpg \
	cassat_summertime_5.jpg \
	cassat_summertime_6.jpg \
	gogh_auvert_1.jpg \
	gogh_auvert_2.jpg \
	gogh_auvert_3.jpg \
	gogh_auvert_4.jpg \
	gogh_auvert_5.jpg \
	gogh_auvert_6.jpg \
	degas_class-dance_1.png \
	degas_class-dance_2.png \
	degas_class-dance_3.png \
	degas_class-dance_4.png \
	degas_class-dance_5.png \
	degas_class-dance_6.png \
	puzzle1.png \
	puzzle2.png \
	puzzle3.png \
	puzzle4.png \
	puzzle5.png \
	puzzle6.png \
	puzzle7.png \
	puzzle8.png \
	renoir-filles_piano-1.png \
	renoir-filles_piano-2.png \
	renoir-filles_piano-3.png \
	renoir-filles_piano-4.png \
	renoir-filles_piano-5.png \
	renoir-filles_piano-6.png \
	renoir-filles_piano-7.png \
	renoir-filles_piano-8.png \
	renoir-moulin_de_la_galette-1.png \
	renoir-moulin_de_la_galette-2.png \
	renoir-moulin_de_la_galette-3.png \
	renoir-moulin_de_la_galette-4.png \
	renoir-moulin_de_la_galette-5.png \
	renoir-moulin_de_la_galette-6.png

EXTRA_DIST = $(img_DATA) ${xml_in_files}
CLEANFILES = $(xml_DATA)
