## Process this file with automake to produce Makefile.in.

doc = gcompris.html

EXTRA_DIST = overview.jpg \
	     overview.txt \
	     topic.dat \
	     overview.dia

gcompris_helpdir = $(datadir)/gnome/help/$(PACKAGE)/fr

all: $(doc)

info_TEXINFOS = gcompris.texi

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(gcompris_helpdir)
	$(INSTALL_DATA)  $(srcdir)/gcompris.html $(DESTDIR)$(gcompris_helpdir)
	$(INSTALL_DATA)  $(srcdir)/overview.jpg $(DESTDIR)$(gcompris_helpdir)
	$(INSTALL_DATA)  $(srcdir)/topic.dat $(DESTDIR)$(gcompris_helpdir)

%.html: %.texi
	texi2html -monolithic $<

%.eps: %.dia
	dia $< -e $@

%.ps: %.dvi
	dvips -o $@ $<

%.dvi: %.texi
	texi2dvi $<

%.pdf: %.ps
	ps2pdf $<

gcompris.pdf gcompris.dvi: overview.eps

clean-local:
	rm -f overview.eps gcompris.pdf gcompris.html gcompris.info
