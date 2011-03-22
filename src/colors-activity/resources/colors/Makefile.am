imgdir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/colors
img_DATA = \
	activity.desktop \
	background.svgz \
	black_duck.svg \
	blue_duck.svg \
	brown_duck.svg \
	green_duck.svg \
	grey_duck.svg \
	orange_duck.svg \
	purple_duck.svg \
	red_duck.svg \
	white_duck.svg \
	yellow_duck.svg

# Do not use the INTLTOOL_DESKTOP_RULE, we don't need to embed the translations
# in the file themselves. GCompris pick the translations from the po file at
# runtime.
activity.desktop: activity.desktop.in
	sed -e "s/^_//g" $< > $@

EXTRA_DIST = $(img_DATA) activity.desktop.in

CLEANFILES = activity.desktop
