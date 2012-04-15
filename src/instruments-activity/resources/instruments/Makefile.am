imgdir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/instruments
img_DATA = \
	activity.desktop \
	background.svgz \
	accordion.ogg \
	accordion.png \
	banjo.ogg \
	banjo.svg \
	bongo.ogg \
	bongo.png \
	castanets.ogg \
	castanets.svg \
	cello.ogg \
	cello.svg \
	clarinet.ogg \
	clarinet.svg \
	cymbal.ogg \
	cymbal.svg \
	drum_kit.ogg \
	drum_kit.svg \
	electric_guitar.ogg \
	electric_guitar.svg \
	flute_traversiere.ogg \
	flute_traversiere.svg \
	guitar.ogg \
	guitar.svg \
	harmonica.ogg \
	harmonica.svg \
	harp.ogg \
	harp.svg \
	horn.ogg \
	horn.svg \
	maracas.ogg \
	maracas.svg \
	organ.ogg \
	organ.png \
	piano.ogg \
	piano.svg \
	saxophone.ogg \
	saxophone.svg \
	snare_drum.ogg \
	snare_drum.svg \
	tambourine.ogg \
	tambourine.svg \
	timpani.ogg \
	timpani.svg \
	triangle.ogg \
	triangle.png \
	trombone.ogg \
	trombone.svg \
	trumpet.ogg \
	trumpet.svg \
	tuba.ogg \
	tuba.svg \
	violin.ogg \
	violin.svg

# Do not use the INTLTOOL_DESKTOP_RULE, we don't need to embed the translations
# in the file themselves. GCompris pick the translations from the po file at
# runtime.
activity.desktop: activity.desktop.in
	sed -e "s/^_//g" $< > $@

EXTRA_DIST = $(img_DATA) activity.desktop.in

CLEANFILES = activity.desktop
