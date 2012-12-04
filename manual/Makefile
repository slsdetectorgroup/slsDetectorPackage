DESTDIR?=../docs


all: $(DESTDIR)/pdf/slsDetectors-FAQ.pdf $(DESTDIR)/pdf/slsDetectorInstall.pdf

$(DESTDIR)/pdf/slsDetectors-FAQ.pdf: slsDetectors-FAQ.tex
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	$(shell test -d $(DESTDIR)/pdf || mkdir -p $(DESTDIR)/pdf)
	latex slsDetectors-FAQ.tex
	latex slsDetectors-FAQ.tex
	dvips -o slsDetectors-FAQ.ps slsDetectors-FAQ.dvi
	ps2pdf slsDetectors-FAQ.ps slsDetectors-FAQ.pdf
	mv slsDetectors-FAQ.pdf $(DESTDIR)/pdf/

$(DESTDIR)/pdf/slsDetectorInstall.pdf: slsDetectorInstall.tex
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	$(shell test -d $(DESTDIR)/pdf || mkdir -p  $(DESTDIR)/pdf)
	latex slsDetectorInstall.tex
	latex slsDetectorInstall.tex
	dvips -o slsDetectorInstall.ps slsDetectorInstall.dvi
	ps2pdf slsDetectorInstall.ps slsDetectorInstall.pdf
	mv  slsDetectorInstall.pdf $(DESTDIR)/pdf


html: $(DESTDIR)/html/slsDetectorInstall $(DESTDIR)/html/slsDetectors-FAQ  index.html
	cp index.html $(DESTDIR)/html/



$(DESTDIR)/html/slsDetectorInstall: slsDetectorInstall.tex
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	$(shell test -d $(DESTDIR)/html || mkdir -p  $(DESTDIR)/html)
	$(shell test -d $(DESTDIR)/html/slsDetectorInstall && rm -r  $(DESTDIR)/html/slsDetectorInstall)
	latex slsDetectorInstall.tex
	latex2html slsDetectorInstall.tex
	mv slsDetectorInstall $(DESTDIR)/html

$(DESTDIR)/html/slsDetectors-FAQ: slsDetectors-FAQ.tex
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	$(shell test -d $(DESTDIR)/html || mkdir -p $(DESTDIR)/html)
	$(shell test -d $(DESTDIR)/html/slsDetectors-FAQ && rm -r  $(DESTDIR)/html/slsDetectors-FAQ)
	latex slsDetectors-FAQ.tex
	latex2html slsDetectors-FAQ.tex
	mv slsDetectors-FAQ $(DESTDIR)/html

clean:
	rm -rf *.pdf *.ps *.dvi *.aux *.log *.toc *.out slsDetectorInstall slsDetectors-FAQ
