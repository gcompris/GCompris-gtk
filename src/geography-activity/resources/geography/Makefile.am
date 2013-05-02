<<<<<<< HEAD
SUBDIRS = africa afrique continents europe northamerica southamerica centralamerica oceania
=======
SUBDIRS = africa afrique centralamerica \
	continents europe northamerica oceania southamerica
>>>>>>> upstream/master

xmldir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/geography

xml_in_files = \
	board1_0.xml.in \
	board2_0.xml.in \
	board3_0.xml.in \
	board3_1.xml.in \
	board4_0.xml.in \
	board5_0.xml.in \
	board6_0.xml.in \
	board7_0.xml.in \
	board8_0.xml.in \
	board9_0.xml.in \
	board10_0.xml.in \
	board11_0.xml.in

xml_DATA = $(xml_in_files:.xml.in=.xml)

asiadir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/geography/asia
asia_DATA = \
	asia/afghanistan.png \
	asia/africa2.png \
	asia/africa.png \
	asia/armenia.png \
	asia/azerbaijan.png \
	asia/background.png \
	asia/bahrain.png \
	asia/bangladesh.png \
	asia/bhutan.png \
	asia/black_sea.png \
	asia/brunei.png \
	asia/cambodia.png \
	asia/caspian_sea.png \
	asia/china.png \
	asia/cyprus.png \
	asia/georgia.png \
	asia/india.png \
	asia/indonesia.png \
	asia/iraq.png \
	asia/iran.png \
	asia/israel.png \
	asia/japan.png \
	asia/jordan.png \
	asia/kazakhstan.png \
	asia/kyrgyzstan.png \
	asia/kuwait.png \
	asia/laos.png \
	asia/lebanon.png \
	asia/malaysia.png \
	asia/mediteranean_sea.png \
	asia/mongolia.png \
	asia/nepal.png \
	asia/north_korea.png \
	asia/myanmar.png \
	asia/oman.png \
	asia/uzbekistan.png \
	asia/pakistan.png \
	asia/palestine.png \
	asia/persian_gulf.png \
	asia/philippines.png \
	asia/qatar.png \
	asia/red_sea.png \
	asia/russia.png \
	asia/saudi_arabia.png \
	asia/south_korea.png \
	asia/sri_lanka.png \
	asia/syria.png \
	asia/tajikistan.png \
	asia/taiwan.png \
	asia/thailand.png \
	asia/turkey.png \
	asia/turkmenistan.png \
	asia/united_arab_emirates.png \
	asia/vietnam.png \
	asia/yemen.png


#
# Do not use the INTLTOOL_XML_RULE, we don't need to embed the translations
# in the file themselves. GCompris pick the translations from the po file at
# runtime.
#
$(xml_DATA): %.xml: %.xml.in
	sed -e "s/\(<\{1\}\/*\)_/\1/g" $< > $@

EXTRA_DIST = ${xml_in_files} ${asia_DATA}
CLEANFILES = $(xml_DATA)
