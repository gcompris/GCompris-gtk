## Process this file with automake to produce Makefile.in.

doc = gcompris.html

man_MANS = gcompris.6

EXTRA_DIST = $(man_MANS)

gcompris_helpdir = $(datadir)/gnome/help/$(PACKAGE)/C

# Info documentation is outdated and useless
#all: $(doc)

#info_TEXINFOS = gcompris.texi

# No more install the documentation, it's not accurate enough
#install-data-local:
#	$(mkinstalldirs) $(DESTDIR)$(gcompris_helpdir)
#	$(INSTALL_DATA)  gcompris.html $(DESTDIR)$(gcompris_helpdir)
#	$(INSTALL_DATA)  $(srcdir)/topic.dat $(DESTDIR)$(gcompris_helpdir)

gcompris.html: gcompris.texi
	texi2html -monolithic $<

gcompris.ps: gcompris.dvi
	dvips -o $@ $<

gcompris.dvi: gcompris.texi
	texi2dvi $<

gcompris.pdf: gcompris.ps
	ps2pdf $<

gcompris.pdf gcompris.dvi:

clean-local:
	rm -f gcompris.pdf gcompris.html gcompris.info
