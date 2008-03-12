## Process this file with automake to produce Makefile.in.

doc = gcompris.html

EXTRA_DIST = overview.jpg \
	     overview.txt \
	     topic.dat \
	     overview.dia \
	     gcompris.texi

gcompris_helpdir = $(datadir)/gnome/help/$(PACKAGE)/fr

all: $(doc)

# FIXME: Is it possible to have I18N info files installed ?
#info_TEXINFOS = gcompris.texi

# No more install the documentation, it's not accurate enough
#install-data-local:
#	$(mkinstalldirs) $(DESTDIR)$(gcompris_helpdir)
#	$(INSTALL_DATA)  gcompris.html $(DESTDIR)$(gcompris_helpdir)
#	$(INSTALL_DATA)  $(srcdir)/overview.jpg $(DESTDIR)$(gcompris_helpdir)
#	$(INSTALL_DATA)  $(srcdir)/topic.dat $(DESTDIR)$(gcompris_helpdir)

gcompris.html: gcompris.texi
	texi2html -monolithic $<

overview.eps: overview.dia
	dia $< -e $@

gcompris.ps: gcompris.dvi
	dvips -o $@ $<

gcompris.dvi: gcompris.texi
	texi2dvi $<

gcompris.pdf: gcompris.ps
	ps2pdf $<


gcompris.pdf gcompris.dvi: overview.eps

clean-local:
	rm -f overview.eps gcompris.pdf gcompris.html gcompris.info
