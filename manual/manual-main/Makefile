DESTDIR=../docs


TEX=latex


MAINTEXS= slsDetectorInstall.tex slsDetectors-FAQ.tex 

TEXS=slsDetector-softFAQ.tex singlePhotonCounting-FAQ.tex angConv-FAQ.tex generalDet-FAQ.tex 

DVIS = $(MAINTEXS:.tex=.dvi)
PSS = $(MAINTEXS:.tex=.ps)
PDFS = $(MAINTEXS:.tex=.pdf)
HTMLS = $(MAINTEXS:%.tex=%)



all: pdf html
	echo $(PWD)
	echo $(PDFS)
	echo $(HTMLS)

pdf: $(PDFS)
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	$(shell test -d $(DESTDIR)/pdf || mkdir -p $(DESTDIR)/pdf)
	mv $(PDFS) $(DESTDIR)/pdf	

html: $(HTMLS)


$(HTMLS): $(TEXS) $(MAINTEXS)
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	$(shell test -d $(DESTDIR)/html || mkdir -p $(DESTDIR)/html)
	$(shell test -d $(DESTDIR)/html/$@ && rm -fr $(DESTDIR)/html/$@)
	echo "*****************************  $@"
	latex $@.tex
	latex2html -split 4 $@.tex	
	mv $@ $(DESTDIR)/html


%.dvi : %.tex $(TEXS)
	latex $<
	latex $<

%.ps : %.dvi
	dvips -o $@  $< 


%.pdf : %.ps
	ps2pdf  $<  $@


clean:
	rm -rf *.aux *.log *.toc *.out $(DVIS) $(PSS) $(PDFS) $(HTMLS)
	rm -rf $(DESTDIR)/html/slsDetectors-FAQ
	rm -rf $(DESTDIR)/html/slsDetectorInstall
	rm -rf $(DESTDIR)/pdf/slsDetectors-FAQ.pdf
	rm -rf $(DESTDIR)/pdf/slsDetectorInstall.pdf
