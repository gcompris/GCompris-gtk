xmldir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/lang

xml_in_files = \
	lang.xml.in

xml_DATA = $(xml_in_files:.xml.in=.xml)

#
# Do not use the INTLTOOL_XML_RULE, we don't need to embed the translations
# in the file themselves. GCompris pick the translations from the po file at
# runtime.
#
$(xml_DATA): %.xml: %.xml.in
	sed -e "s/\(<\{1\}\/*\)_/\1/g" $< > $@


imgdir = $(pkgdatadir)/@PACKAGE_DATA_DIR@/lang/numbers
img_DATA = \
	intro/a_boy.jpg \
	intro/a_girl.jpg \
	intro/automn.jpg \
	intro/basketball.jpg \
	intro/beach2.jpg \
	intro/beach.jpg \
	intro/birthday_clara.jpg \
	intro/birthday_clement.jpg \
	intro/blackboard.jpg \
	intro/book.jpg \
	intro/cantine.jpg \
	intro/cat_on_chair.jpg \
	intro/christmas_tree.jpg \
	intro/clara_and_grandma.jpg \
	intro/clara_clement_sabrina_play.jpg \
	intro/clara_clown.jpg \
	intro/clara_dance.jpg \
	intro/clara_diving.jpg \
	intro/clara_eats.jpg \
	intro/clara_guitar.jpg \
	intro/clara_homework.jpg \
	intro/clara.jpg \
	intro/clara_sea_side.jpg \
	intro/clara_ski.jpg \
	intro/clara_teeth_brush.jpg \
	intro/clement_and_dad.jpg \
	intro/clement_drinks.jpg \
	intro/clement_is_drawing.jpg \
	intro/clement_kayaking.jpg \
	intro/climbing_tree.jpg \
	intro/dad_iron.jpg \
	intro/dad.jpg \
	intro/dad_programs.jpg \
	intro/djill_piano.jpg \
	intro/Earth_Western_Hemisphere.jpg \
	intro/Eiffel_Tower_Paris.jpg \
	intro/eraser.jpg \
	intro/family_dinner.jpg \
	intro/fire.jpg \
	intro/fishing.jpg \
	intro/flower.jpg \
	intro/football.jpg \
	intro/glue.jpg \
	intro/good_afternoon.jpg \
	intro/good_evening.jpg \
	intro/good_morning.jpg \
	intro/good_night.jpg \
	intro/grandma_gardening.jpg \
	intro/grandma_sewing.jpg \
	intro/grandpa_cooking.jpg \
	intro/grandpa_newspaper.jpg \
	intro/he_is_sick.jpg \
	intro/hello_clara.jpg \
	intro/hello_clement.jpg \
	intro/hello.jpg \
	intro/history.jpg \
	intro/horse_riding.jpg \
	intro/how_do_you_do.jpg \
	intro/how_old_are_you.jpg \
	intro/i_am_fine.jpg \
	intro/is_he_sick.jpg \
	intro/mum_cooking.jpg \
	intro/mum.jpg \
	intro/notebook.jpg \
	intro/pancakes.jpg \
	intro/Paris-Grand-Palais.jpg \
	intro/pencil_case.jpg \
	intro/pencil.jpg \
	intro/pencil_sharpener.jpg \
	intro/pen.jpg \
	intro/playground.jpg \
	intro/pupils.jpg \
	intro/rain.jpg \
	intro/reading.jpg \
	intro/red_pencil.jpg \
	intro/ruler.jpg \
	intro/sabrina_sea_side.jpg \
	intro/sabrina_sings.jpg \
	intro/school_bag.jpg \
	intro/school.jpg \
	intro/science.jpg \
	intro/scissors.jpg \
	intro/simon_bicycle.jpg \
	intro/sit_down.jpg \
	intro/ski_fall.jpg \
	intro/snow.jpg \
	intro/spring.jpg \
	intro/stapler.jpg \
	intro/student_walk.jpg \
	intro/sunny.jpg \
	intro/swimming.jpg \
	intro/teacher.jpg \
	intro/vacation.jpg \
	intro/welcome.jpg \
	intro/what_is_your_name.jpg \
	intro/who_are_you.jpg \
	numbers/10_litchis.jpg \
	numbers/1_mango.jpg \
	numbers/2_pears.jpg \
	numbers/3_apples.jpg \
	numbers/4_bananas.jpg \
	numbers/5_lemons.jpg \
	numbers/6_oranges.jpg \
	numbers/7_strawberries.jpg \
	numbers/8_guavas.jpg \
	numbers/9_apricots.jpg

EXTRA_DIST = $(img_DATA) ${xml_in_files}
CLEANFILES = $(xml_DATA)
