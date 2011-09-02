imgdir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/louis_braille
img_DATA = \
	activity.desktop \
	back.png \
	braile.jpg \
	braille_ball.jpg \
	chart.png \
	fireplace.jpg \
	louis.jpg \
	night_printing.jpg \
	piano.jpg \
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

CLEANFILES = activity.desktop


