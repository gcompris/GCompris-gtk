# Passthrough, do not break uplevel make rule
install-activity:

SUBDIRS = resources

pythondir = $(PYTHON_PLUGIN_DIR)

dist_python_DATA= \
	lang.py \
	langEnterText.py \
	langLib.py \
	langFindit.py



xmldir = $(pkgdatadir)/@PACKAGE_DATA_DIR@

xml_in_files = \
	lang.xml.in \
	lang-nature.xml.in \
	 lang-object.xml.in \
	 lang-other.xml.in \
	 lang-people.xml.in

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
icon_DATA = \
	lang.svg \
	lang-nature.svg \
	lang-object.svg \
	lang-other.svg \
	lang-people.svg


EXTRA_DIST = $(icon_DATA) ${xml_in_files} init_path.sh
CLEANFILES = $(xml_DATA)
