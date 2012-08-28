if SUGAR

bin_SCRIPTS = \
	gcompris-activity

application-x-gcompris-wordprocessor.svg: ../src/wordprocessor-activity/wordprocessor.svg
	cp $^ $@

application-x-gcompris-anim.svg: ../src/anim-activity/anim.svg
	cp $^ $@

application-x-gcompris-draw.svg: ../src/draw-activity/draw.svg
	cp $^ $@

sugarbindir = $(datadir)/sugar/activities/GCompris.activity/bin
sugarbin_SCRIPTS = \
	gcompris-activity

sugardir = $(datadir)/sugar/activities/GCompris.activity/activity
sugar_DATA = \
	GCompris.activity/activity.info \
	GCompris.activity/gcompris.svg \
	application-x-gcompris-wordprocessor.svg \
	application-x-gcompris-anim.svg \
	application-x-gcompris-draw.svg

sugaradmdir = $(datadir)/sugar/activities/GComprisAdministration.activity/activity
sugaradm_DATA = \
	GComprisAdministration.activity/activity.info \
	GComprisAdministration.activity/administration.svg

sugaradmbindir = $(datadir)/sugar/activities/GComprisAdministration.activity/bin
sugaradmbin_SCRIPTS = \
	gcompris-activity

iconsdir = $(datadir)/icons/sugar/scalable/mimetypes
icons_DATA = \
	application-x-gcompris-wordprocessor.svg \
	application-x-gcompris-anim.svg \
	application-x-gcompris-draw.svg

endif

EXTRA_DIST = \
	gcompris-activity \
	GCompris.activity/activity.info \
	GCompris.activity/gcompris.svg \
	GComprisAdministration.activity/activity.info \
	GComprisAdministration.activity/administration.svg

