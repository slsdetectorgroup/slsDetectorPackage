DESTDIR?=../docs

#manual-api manual-calwiz manual-client manual-gui manual-main


TEX=latex

MAINTEXS= slsDetectorClientHowTo.tex

DVIS = $(MAINTEXS:.tex=.dvi)
PSS = $(MAINTEXS:.tex=.ps)
PDFS = $(MAINTEXS:.tex=.pdf)
HTMLS = $(MAINTEXS:%.tex=%)


all: docs pdf html
	echo $(PWD)
	echo $(PDFS)
	echo $(HTMLS)
	
docs: 	createdocs docspdf docshtml removedocs
	
createdocs: slsDetectorClient.doxy slsDetectorCommand.cpp
	doxygen slsDetectorClient.doxy	

docspdf: 
	cd slsDetectorClientDocs/latex && make 
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	$(shell test -d $(DESTDIR)/pdf || mkdir -p $(DESTDIR)/pdf)
	mv slsDetectorClientDocs/latex/refman.pdf $(DESTDIR)/pdf/slsDetectorClientDocs.pdf

docshtml: 
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	$(shell test -d $(DESTDIR)/html || mkdir -p $(DESTDIR)/html)
	$(shell test -d $(DESTDIR)/html/slsDetectorClientDocs && rm -r $(DESTDIR)/html/slsDetectorClientDocs)
	mv slsDetectorClientDocs/html $(DESTDIR)/html/slsDetectorClientDocs
	
removedocs:
	rm -rf 	slsDetectorClientDocs;

pdf: $(PDFS)
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	$(shell test -d $(DESTDIR)/pdf || mkdir -p $(DESTDIR)/pdf)
	mv $(PDFS) $(DESTDIR)/pdf	
	

html: $(HTMLS)

$(HTMLS): $(MAINTEXS)
	$(shell test -d $(DESTDIR) || mkdir -p $(DESTDIR))
	$(shell test -d $(DESTDIR)/html || mkdir -p $(DESTDIR)/html)
	$(shell test -d $(DESTDIR)/html/$@ && rm -r $(DESTDIR)/html/$@)
	latex $@.tex
	latex2html -split 4 $@.tex
	mv $@ $(DESTDIR)/html
	


%.dvi : %.tex
	latex $<
	latex $<

%.ps : %.dvi
	dvips -o $@  $< 


%.pdf : %.ps
	ps2pdf  $<  $@


clean:
	echo "cleaning for manual-client"
	rm -rf *.aux *.log *.toc *.out $(DVIS) $(PSS) $(PDFS) $(HTMLS)
	rm -rf slsDetectorClientDocs
	rm -rf $(DESTDIR)/html/slsDetectorClientDocs
	rm -rf $(DESTDIR)/html/slsDetectorClientHowTo
	rm -rf $(DESTDIR)/pdf/slsDetectorClientDocs.pdf
	rm -rf $(DESTDIR)/pdf/slsDetectorClientHowTo.pdf
