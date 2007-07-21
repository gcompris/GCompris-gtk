
xmldir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/imagename

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


imgdir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/imagename
img_DATA = \
	apple.png \
	bicycle.png \
	bottle.png \
	carot.png \
	car.png \
	castle.png \
	cocotier.png \
	crown.png \
	egg.png \
	eggpot.png \
	fishingboat.png \
	flower.png \
	flowerpot.png \
	football.png \
	fusee.png \
	glass.png \
	house.png \
	lamp.png \
	lighthouse.png \
	light.png \
	minivan.png \
	pencil.png \
	postcard.png \
	postpoint.png \
	rape.png \
	raquette.png \
	sailingboat.png \
	sapin.png \
	sofa.png \
	star.png \
	tree.png \
	truck.png \
	windflag5.png

EXTRA_DIST = $(img_DATA)
