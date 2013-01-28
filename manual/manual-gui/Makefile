DESTDIR?=../docs


#manual-api manual-calwiz manual-client manual-gui manual-main


TEX=latex

MAINTEXS= slsDetectorGuiHowTo.tex

#TEXS=

DVIS = $(MAINTEXS:.tex=.dvi)
PSS = $(MAINTEXS:.tex=.ps)
PDFS = $(MAINTEXS:.tex=.pdf)

#TARGETS=$(join $(DESTDIR)/, $(notdir $(MAINTEXS:.tex=.pdf)))

#destdir?!?!?



DVIS = $(MAINTEXS:.tex=.dvi)
PSS = $(MAINTEXS:.tex=.ps)
PDFS = $(MAINTEXS:.tex=.pdf)

HTMLS = $(MAINTEXS:%.tex=%)


#destdir?!?!?

all: $(PDFS) $(HTMLS)
	echo $(PWD)
	echo $(PDFS)
	echo $(HTMLS)

pdf: $(PDFS)

html: $(HTMLS)

$(HTMLS): $(TEXS) $(MAINTEXS)
	latex $@.tex
	latex2html -split 4 $@.tex


%.dvi : %.tex $(TEXS)
	latex $<
	latex $<

%.ps : %.dvi
	dvips -o $@  $< 


%.pdf : %.ps
	ps2pdf  $<  $@


clean:
	rm -rf *.aux *.log *.toc *.out $(DVIS) $(PSS) $(PDFS) $(HTMLS)
