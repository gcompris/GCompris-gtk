include $(top_srcdir)/Makefile.activity

SUBDIRS =

pythondir = $(PYTHON_PLUGIN_DIR)

dist_python_DATA= 	color_mix_light.py

xmldir = $(pkgdatadir)/@PACKAGE_DATA_DIR@

xml_in_files = \
	color_mix_light.xml.in


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

icondir = $(pkgdatadir)/@PACKAGE_DATA_DIR@
icon_DATA = color_mix_light.svg

EXTRA_DIST = $(icon_DATA) ${xml_in_files} init_path.sh
CLEANFILES = $(xml_DATA)

