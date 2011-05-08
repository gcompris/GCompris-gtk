imgdir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/advanced_colors
img_DATA = \
	activity.desktop \
	background.svgz \
	absinthe_butterfly.svg \
	alabaster_butterfly.svg \
	almond_butterfly.svg \
	amber_butterfly.svg \
	amethyst_butterfly.svg \
	anise_butterfly.svg \
	aquamarine_butterfly.svg \
	aubergine_butterfly.svg \
	auburn_butterfly.svg \
	azure_butterfly.svg \
	bistre_butterfly.svg \
	celadon_butterfly.svg \
	cerulean_butterfly.svg \
	ceruse_butterfly.svg \
	chartreuse_butterfly.svg \
	chestnut_butterfly.svg \
	claret_butterfly.svg \
	cobalt_butterfly.svg \
	coral_butterfly.svg \
	corn_butterfly.svg \
	crimson_butterfly.svg \
	cyan_butterfly.svg \
	dark_purple_butterfly.svg \
	dove_butterfly.svg \
	emerald_butterfly.svg \
	fawn_butterfly.svg \
	fuchsia_butterfly.svg \
	garnet_butterfly.svg \
	glaucous_butterfly.svg \
	greyish_blue_butterfly.svg \
	greyish-brown_butterfly.svg \
	indigo_butterfly.svg \
	ivory_butterfly.svg \
	jade_butterfly.svg \
	larch_butterfly.svg \
	lavender_butterfly.svg \
	lichen_butterfly.svg \
	lilac_butterfly.svg \
	lime_butterfly.svg \
	magenta_butterfly.svg \
	mahogany_butterfly.svg \
	malachite_butterfly.svg \
	mauve_butterfly.svg \
	mimosa_butterfly.svg \
	navy_butterfly.svg \
	ochre_butterfly.svg \
	olive_butterfly.svg \
	opaline_butterfly.svg \
	pistachio_butterfly.svg \
	platinum_butterfly.svg \
	plum_butterfly.svg \
	prussian_blue_butterfly.svg \
	purple_butterfly.svg \
	ruby_butterfly.svg \
	rust_butterfly.svg \
	saffron_butterfly.svg \
	sage_butterfly.svg \
	salmon_butterfly.svg \
	sapphire_butterfly.svg \
	sepia_butterfly.svg \
	sienna_butterfly.svg \
	sulphur_butterfly.svg \
	tea_butterfly.svg \
	turquoise_butterfly.svg \
	ultramarine_butterfly.svg \
	vanilla_butterfly.svg \
	verdigris_butterfly.svg \
	vermilion_butterfly.svg \
	veronese_butterfly.svg \
	wine_butterfly.svg

# Do not use the INTLTOOL_DESKTOP_RULE, we don't need to embed the translations
# in the file themselves. GCompris pick the translations from the po file at
# runtime.
activity.desktop: activity.desktop.in
	sed -e "s/^_//g" $< > $@

EXTRA_DIST = $(img_DATA) activity.desktop.in

CLEANFILES = activity.desktop
