SUBDIRS=dog1 dog2 dog3 food

xmldir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/babyshapes

xml_in_files = \
	board1_0.xml.in \
	board2_0.xml.in \
	board3_0.xml.in \
	board4_0.xml.in \
	board5_0.xml.in \
	board6_0.xml.in \
	board7_0.xml.in \
	board8_0.xml.in \
	board8_1.xml.in \
	board8_2.xml.in \
	board8_3.xml.in \
	board8_4.xml.in

xml_DATA = $(xml_in_files:.xml.in=.xml)

#
# Do not use the INTLTOOL_XML_RULE, we don't need to embed the translations
# in the file themselves. GCompris pick the translations from the po file at
# runtime.
#
$(xml_DATA): %.xml: %.xml.in
	sed -e "s/\(<\{1\}\/*\)_/\1/g" $< > $@




imgdir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/babyshapes
img_DATA = \
	Bosschaert_04_1.png \
	Bosschaert_04_2.png \
	Bosschaert_04_3.png \
	Bosschaert_04_4.png \
	Bosschaert_04_5.png \
	Bosschaert_04_6.png \
	Bosschaert_04_background.png \
	lady_unicorn_1.png \
	lady_unicorn_2.png \
	lady_unicorn_3.png \
	lady_unicorn_4.png \
	lady_unicorn_5.png \
	lady_unicorn_background.png \
	Paul_Gauguin_006_background.png \
	Paul_Gauguin_006_chien.png \
	Paul_Gauguin_006_jeunes_filles.png \
	Paul_Gauguin_006_statue.png \
	Pieter_Bruegel_background.png \
	Pieter_Bruegel_fillette.png \
	Pieter_Bruegel_mur.png \
	Pieter_Bruegel_musicien.png \
	Pieter_Bruegel_service.png \
	T_baby_bottle.png \
	T_banana.png \
	T_bread_slice.png \
	T_butter.png \
	T_chocolate_cake.png \
	T_chocolate.png \
	T_cookie.png \
	T_french_croissant.png \
	T_grapefruit.png \
	T_marmelade.png \
	T_milk_cup.png \
	T_milk_shake.png \
	T_orange.png \
	T_pear.png \
	T_round_cookie.png \
	T_suggar_box.png \
	T_yahourt.png \
	Van_Gogh_0011_1.png \
	Van_Gogh_0011_2.png \
	Van_Gogh_0011_3.png \
	Van_Gogh_0011_4.png \
	Van_Gogh_0011_5.png \
	Van_Gogh_0011_background.png

EXTRA_DIST = $(img_DATA) ${xml_in_files}
CLEANFILES = $(xml_DATA)
