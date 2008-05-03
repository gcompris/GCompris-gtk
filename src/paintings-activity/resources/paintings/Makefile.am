
xmldir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/paintings

xml_in_files = \
	board1_0.xml.in \
	board2_0.xml.in \
	board2_1.xml.in \
	board2_2.xml.in \
	board2_3.xml.in \
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
	renoir-moulin_de_la_galette-6.png \
	Botticelli_Primaver_a1.png \
	Botticelli_Primaver_a2.png \
	Botticelli_Primaver_a3.png \
	Botticelli_Primaver_a4.png \
	Botticelli_Primaver_b1.png \
	Botticelli_Primaver_b2.png \
	Botticelli_Primaver_b3.png \
	Botticelli_Primaver_b4.png \
	Botticelli_Primaver_c1.png \
	Botticelli_Primaver_c2.png \
	Botticelli_Primaver_c3.png \
	Botticelli_Primaver_c4.png \
	Botticelli_Primaver_d1.png \
	Botticelli_Primaver_d2.png \
	Botticelli_Primaver_d3.png \
	Botticelli_Primaver_d4.png \
	Botticelli_Venus_a1.png \
	Botticelli_Venus_a2.png \
	Botticelli_Venus_a3.png \
	Botticelli_Venus_a4.png \
	Botticelli_Venus_b1.png \
	Botticelli_Venus_b2.png \
	Botticelli_Venus_b3.png \
	Botticelli_Venus_b4.png \
	Botticelli_Venus_c1.png \
	Botticelli_Venus_c2.png \
	Botticelli_Venus_c3.png \
	Botticelli_Venus_c4.png \
	Botticelli_Venus_d1.png \
	Botticelli_Venus_d2.png \
	Botticelli_Venus_d3.png \
	Botticelli_Venus_d4.png \
	Le_Caravage_Egypte_a1.png \
	Le_Caravage_Egypte_a2.png \
	Le_Caravage_Egypte_a3.png \
	Le_Caravage_Egypte_a4.png \
	Le_Caravage_Egypte_b1.png \
	Le_Caravage_Egypte_b2.png \
	Le_Caravage_Egypte_b3.png \
	Le_Caravage_Egypte_b4.png \
	Le_Caravage_Egypte_c1.png \
	Le_Caravage_Egypte_c2.png \
	Le_Caravage_Egypte_c3.png \
	Le_Caravage_Egypte_c4.png \
	Michelangelo_Pieta_a1.png \
	Michelangelo_Pieta_a2.png \
	Michelangelo_Pieta_a3.png \
	Michelangelo_Pieta_a4.png \
	Michelangelo_Pieta_b1.png \
	Michelangelo_Pieta_b2.png \
	Michelangelo_Pieta_b3.png \
	Michelangelo_Pieta_b4.png \
	Michelangelo_Pieta_c1.png \
	Michelangelo_Pieta_c2.png \
	Michelangelo_Pieta_c3.png \
	Michelangelo_Pieta_c4.png

EXTRA_DIST = $(img_DATA) ${xml_in_files}
CLEANFILES = $(xml_DATA)
