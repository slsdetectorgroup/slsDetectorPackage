DESTDIR?=../docs


#manual-api manual-calwiz manual-client manual-gui manual-main

TEX=latex

MAINTEXS= angularCalibrationHowTo.tex energyCalibrationHowTo.tex 

TEXS=ancCal.tex enCal.tex installation.tex 

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
	$(shell test -d $(DESTDIR)/html/$@ && rm -r $(DESTDIR)/html/$@)
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
	rm -rf $(DESTDIR)/html/angularCalibrationHowTo
	rm -rf $(DESTDIR)/html/energyCalibrationHowTo
	rm -rf $(DESTDIR)/pdf/angularCalibrationHowTo.pdf
	rm -rf $(DESTDIR)/pdf/energyCalibrationHowTo.pdf
