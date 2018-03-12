DESTDIR=../docs


TEX=latex


MAINTEXS2= slsDetectorInstall.tex 
MAINTEXS=slsDetectors-FAQ.tex 

TEXS=slsDetector-softFAQ.tex singlePhotonCounting-FAQ.tex angConv-FAQ.tex generalDet-FAQ.tex 

DVIS = $(MAINTEXS:.tex=.dvi)
PSS = $(MAINTEXS:.tex=.ps)
PDFS = $(MAINTEXS:.tex=.pdf)
PDFS2 = $(MAINTEXS2:.tex=.pdf)
HTMLS = $(MAINTEXS:%.tex=%)
HTMLS2 = $(MAINTEXS2:%.tex=%)



all: pdf html
	echo $(PWD)
	echo $(PDFS)
	echo $(PDFS2)
	echo $(HTMLS)
	echo $(HTMLS2)

pdf: $(PDFS) $(PDFS2)
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	$(shell test -d $(DESTDIR)/pdf || mkdir -p $(DESTDIR)/pdf)
	mv $(PDFS) $(DESTDIR)/pdf
	mv $(PDFS2) $(DESTDIR)/pdf	

html: $(HTMLS) $(HTMLS2)


$(HTMLS): $(TEXS) $(MAINTEXS)
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	$(shell test -d $(DESTDIR)/html || mkdir -p $(DESTDIR)/html)
	$(shell test -d $(DESTDIR)/html/$@ && rm -fr $(DESTDIR)/html/$@)
	echo "*****************************  $@"
	latex $@.tex
	latex2html $@.tex
	mv $@ $(DESTDIR)/html
	
$(HTMLS2): $(MAINTEXS2)
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	$(shell test -d $(DESTDIR)/html || mkdir -p $(DESTDIR)/html)
	$(shell test -d $(DESTDIR)/html/$@ && rm -fr $(DESTDIR)/html/$@)
	echo "*****************************  $@"
	latex $@.tex
	latex2html -split 16 -no_navigation -info "" -address  "" -long_titles 5 -link 0  $@.tex
	#-show_section_numbers $@.tex	
	# -local_icons
	mv $@ $(DESTDIR)/html


%.dvi : %.tex $(TEXS)
	latex $<
	latex $<

%.ps : %.dvi
	dvips -o $@  $< 


%.pdf : %.ps
	ps2pdf  $<  $@


clean:
	rm -rf *.aux *.log *.toc *.out $(DVIS) $(PSS) $(PDFS) $(PDFS2) $(HTMLS) $(HTMLS2)
	rm -rf $(DESTDIR)/html/slsDetectors-FAQ
	rm -rf $(DESTDIR)/html/slsDetectorInstall
	rm -rf $(DESTDIR)/pdf/slsDetectors-FAQ.pdf
	rm -rf $(DESTDIR)/pdf/slsDetectorInstall.pdf
