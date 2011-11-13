SUBDIRS =

icondir = $(pkgdatadir)/@PACKAGE_DATA_DIR@

icon_DATA = algebra_group.svg \
	algebramenu.svg \
	ballcatch.svg \
	book.svg \
	braille.svg \
	chess.svg \
	computer.svg \
	discovery.svg \
	ear.svg \
	experience.svg \
	eye_colors.svg \
	fun.svg \
	geometry.svg \
	gnumch.svg \
	keyboard.svg \
	math.svg \
	memory_elephant.svg \
	mem_solo_calc.svg \
	mem_tux_calc.svg \
	miscelaneous.svg \
	money_group.svg \
	mouse.svg \
	numeration.svg \
	puzzles.svg \
	strategy.svg \
	warning.svg

xmldir = $(pkgdatadir)/@PACKAGE_DATA_DIR@

xml_in_files = \
	algebra_group.xml.in \
	algebramenu.xml.in \
	braille.xml.in \
	chess.xml.in \
	colors_group.xml.in \
	computer.xml.in \
	discovery.xml.in \
	experience.xml.in \
	experimental.xml.in \
	fun.xml.in \
	geometry.xml.in \
	gnumchmenu.xml.in \
	keyboard.xml.in \
	math.xml.in \
	mazeMenu.xml.in \
	memory_group.xml.in \
	memory_op_group_tux.xml.in \
	memory_op_group.xml.in \
	menu.xml.in \
	miscelaneous.xml.in \
	money_group.xml.in \
	mouse.xml.in \
	numeration.xml.in \
	puzzle.xml.in \
	reading.xml.in \
	sound_group.xml.in \
	strategy.xml.in



xml_DATA = $(xml_in_files:.xml.in=.xml)

# Do not use the INTLTOOL_XML_RULE, we don't need to embed the translations
# in the file themselves. GCompris pick the translations from the po file at
# runtime.
#
$(xml_DATA): %.xml: %.xml.in
	sed -e "s/\(<\{1\}\/*\)_/\1/g" $< > $@

# Use this directive and instead of the one above to embed the translations
# in the xml files directly, this is needed for the tool gcompris2spip
#@INTLTOOL_XML_RULE@

CLEANFILES = $(xml_DATA)

EXTRA_DIST = $(icon_DATA)

