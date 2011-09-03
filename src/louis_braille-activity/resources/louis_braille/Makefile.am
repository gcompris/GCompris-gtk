imgdir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/louis_braille
img_DATA = \
	activity.desktop \
<<<<<<< HEAD
	back.png \
=======
	background.svgz \
>>>>>>> gcomprisbraille
	braile.jpg \
	braille_ball.jpg \
	chart.png \
	fireplace.jpg \
<<<<<<< HEAD
	louis.jpg \
	night_printing.jpg \
	piano.jpg \
=======
	louis.png \
	night_printing.jpg \
	piano.svg \
>>>>>>> gcomprisbraille
	rnib.jpg \
	statue.jpg \
	teach.jpg \
	workshop.jpg

# Do not use the INTLTOOL_DESKTOP_RULE, we don't need to embed the translations
# in the file themselves. GCompris pick the translations from the po file at
# runtime.
activity.desktop: activity.desktop.in
	sed -e "s/^_//g" $< > $@

EXTRA_DIST = $(img_DATA) activity.desktop.in
<<<<<<< HEAD

CLEANFILES = activity.desktop


=======
CLEANFILES = activity.desktop
>>>>>>> gcomprisbraille
