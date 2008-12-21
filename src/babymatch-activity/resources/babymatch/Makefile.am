
xmldir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/babymatch

xml_in_files = \
	board1_0.xml.in \
	board2_0.xml.in \
	board3_0.xml.in \
	board4_0.xml.in \
	board5_0.xml.in \
	board6_0.xml.in \
	board7_0.xml.in

xml_DATA = $(xml_in_files:.xml.in=.xml)

#
# Do not use the INTLTOOL_XML_RULE, we don't need to embed the translations
# in the file themselves. GCompris pick the translations from the po file at
# runtime.
#
$(xml_DATA): %.xml: %.xml.in
	sed -e "s/\(<\{1\}\/*\)_/\1/g" $< > $@




imgdir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/babymatch
img_DATA = \
	apple.svg \
	bell.svg \
	bicycle.svg \
	bottle.svg \
	carrot.svg \
	car.svg \
	castle.svg \
	crown.svg \
	egg.svg \
	eggpot.svg \
	fishingboat.svg \
	flower.svg \
	flowerpot.svg \
	football.svg \
	fusee.svg \
	glass.svg \
	sound.svg \
	house.svg \
	lamp.svg \
	lifebuoy.svg \
	lighthouse.svg \
	light.svg \
	minivan.svg \
	pencil.svg \
	postcard.svg \
	postpoint.svg \
	rape.svg \
	raquette.svg \
	sailingboat.svg \
	sofa.svg \
	star.svg \
	sun.svg \
	tree.svg \
	truck.svg \
	tuxhelico.svg \
	tuxplane.svg \
	umbrella.svg \
	windflag5.svg

EXTRA_DIST = $(img_DATA) ${xml_in_files}
CLEANFILES = $(xml_DATA)
