xmldir = $(datadir)/xml

xml_in_files = \
	algebramenu.xml.in \
	algebra_plus.xml.in \
	algebra_minus.xml.in \
	algebra_by.xml.in \
	algebra_div.xml.in \
	babymatch.xml.in \
	babyshapes.xml.in \
	clickgame.xml.in \
	clockgame.xml.in \
	computer.xml.in \
	draw.xml.in \
	fun.xml.in \
	gletters.xml.in \
	imagename.xml.in \
	memory.xml.in \
	menu.xml.in \
	paintings.xml.in \
	paratrooper.xml.in \
	planegame.xml.in \
	readingh.xml.in \
	readingv.xml.in \
	reading.xml.in \
	scalesboard.xml.in \
	smallnumbers.xml.in \
	wordsgame.xml.in \
	imageid.xml.in \
	missing_letter.xml.in \
	click_on_letter.xml.in \
	erase.xml.in

xml_DATA = $(xml_in_files:.xml.in=.xml)

@XML_I18N_MERGE_XML_RULE@

EXTRA_DIST=$(xml_in_files) $(xml_DATA)
