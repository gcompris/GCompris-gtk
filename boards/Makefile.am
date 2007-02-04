SUBDIRS = gcompris read_colors advanced_colors babymatch babyshapes imageid imagename missing_letter paintings doubleentry geography chronos

xmldir = $(pkgdatadir)/@PACKAGE_DATA_DIR@

xml_in_files = \
	administration.xml.in \
	advanced_colors.xml.in \
	algebra_by.xml.in \
	algebra_group.xml.in \
	algebra_guesscount.xml.in \
	algebramenu.xml.in \
	algebra_minus.xml.in \
	algebra_plus.xml.in \
	algorithm.xml.in \
	anim.xml.in \
	awele.xml.in \
	babymatch.xml.in \
	babyshapes.xml.in \
	ballcatch.xml.in \
	bargame.xml.in \
	billard.xml.in \
	canal_lock.xml.in \
	chat.xml.in \
	chess_computer.xml.in \
	chess_movelearn.xml.in \
	chess_partyend.xml.in \
	chess.xml.in \
	chronos.xml.in \
	clickgame.xml.in \
	click_on_letter.xml.in \
	clockgame.xml.in \
	colors_group.xml.in \
	colors.xml.in \
	computer.xml.in \
	connect4.xml.in \
	connect4-2players.xml.in \
	crane.xml.in \
	discovery.xml.in \
	doubleentry.xml.in \
	draw.xml.in \
	electric.xml.in \
	enumerate.xml.in \
	erase_clic.xml.in \
	erase_double_clic.xml.in \
	erase.xml.in \
	experience.xml.in \
	experimental.xml.in \
	fifteen.xml.in \
	followline.xml.in \
	fun.xml.in \
	geography.xml.in \
	geometry.xml.in \
	gletters.xml.in \
	gnumch-equality.xml.in \
	gnumch-factors.xml.in \
	gnumch-inequality.xml.in \
	gnumchmenu.xml.in \
	gnumch-multiples.xml.in \
	gnumch-primes.xml.in \
	guessnumber.xml.in \
	hanoi_real.xml.in \
	hanoi.xml.in \
	hexagon.xml.in \
	imageid.xml.in \
	imagename.xml.in \
	keyboard.xml.in \
	leftright.xml.in \
	login.xml.in \
	magic_hat_plus.xml.in \
	magic_hat_minus.xml.in \
	math.xml.in \
	maze2DRelative.xml.in \
	maze3D.xml.in \
	mazeInvisible.xml.in \
	mazeMenu.xml.in \
	maze.xml.in \
	melody.xml.in \
	memory_group.xml.in \
	memory_sound_tux.xml.in \
	memory_sound.xml.in \
	memory_op_group.xml.in \
	memory_op_group_tux.xml.in \
	memory_add.xml.in \
	memory_add_tux.xml.in \
	memory_minus.xml.in \
	memory_minus_tux.xml.in \
	memory_mult.xml.in \
	memory_mult_tux.xml.in \
	memory_div.xml.in \
	memory_div_tux.xml.in \
	memory_add_minus.xml.in \
	memory_add_minus_tux.xml.in \
	memory_mult_div.xml.in \
	memory_mult_div_tux.xml.in \
	memory_add_minus_mult_div.xml.in \
	memory_add_minus_mult_div_tux.xml.in \
	memory_tux.xml.in \
	memory.xml.in \
	menu.xml.in \
	miscelaneous.xml.in \
	missing_letter.xml.in \
	money_cents.xml.in \
	money.xml.in \
	mosaic.xml.in \
	mouse.xml.in \
	mypaint.xml.in \
	numeration.xml.in \
	oscar_and_friends.xml.in \
	paintings.xml.in \
	paratrooper.xml.in \
	planegame.xml.in \
	puzzle.xml.in \
	pythontest.xml.in \
	railroad.xml.in \
	read_colors.xml.in \
	readingh.xml.in \
	readingv.xml.in \
	reading.xml.in \
	redraw_symmetrical.xml.in \
	redraw.xml.in \
	reversecount.xml.in \
	scalesboard.xml.in \
	searace1player.xml.in \
	searace.xml.in \
	smallnumbers2.xml.in \
	smallnumbers.xml.in \
	sound_group.xml.in \
	strategy.xml.in \
	submarine.xml.in \
	sudoku.xml.in \
	superbrain.xml.in \
	tangram.xml.in \
	target.xml.in \
	traffic.xml.in \
	tuxpaint.xml.in \
	watercycle.xml.in \
	wordprocessor.xml.in \
	wordsgame.xml.in

xml_DATA = $(xml_in_files:.xml.in=.xml)

#
# Do not use the INTLTOOL_XML_RULE, we don't need to embed the translations
# in the file themselves. GCompris pick the translations from the po file at
# runtime.
#
$(xml_DATA): %.xml: %.xml.in
	sed -e "s/\(<\{1\}\/*\)_/\1/g" $< > $@

CLEANFILES = $(xml_DATA)
