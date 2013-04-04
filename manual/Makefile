DESTDIR?=../docs


#manual-api manual-calwiz manual-client manual-gui manual-main



MAINDIRS= manual-main 
#manual-calwiz manual-calwiz manual-gui manual-client manual-api
CLEANDIRS=$(MAINDIRS:manual-%=clean-%)
PDFDIRS=$(MAINDIRS:manual-%=pdf-%)
HTMLDIRS=$(MAINDIRS:manual-%=html-%)
ALLDIRS=$(MAINDIRS:manual-%=all-%)


all: $(ALLDIRS)
	$(shell test -d $(DESTDIR)/pdf && rm -fr $(DESTDIR)/pdf)
	mv pdf $(DESTDIR)
	$(shell test -d $(DESTDIR)/html && rm -fr $(DESTDIR)/html)
	mv html $(DESTDIR)

pdf: $(PDFDIRS)
	$(shell test -d $(DESTDIR)/pdf && rm -fr $(DESTDIR)/pdf)
	mv pdf $(DESTDIR)

html: $(HTMLDIRS)
	$(shell test -d $(DESTDIR)/html && rm -fr $(DESTDIR)/html)
	mv html $(DESTDIR)

clean: $(CLEANDIRS)
	rm -fr $(DESTDIR)/pdf
	rm -fr $(DESTDIR)/html	

all-%:
	echo 
	cd  $(@:all-%=manual-%) && make all

pdf-%:
	cd  $(@:pdf-%=manual-%) && make pdf

html-%:
	cd  $(@:html-%=manual-%) && make html

clean-%: 
	cd  $(@:clean-%=manual-%) && make clean	


#กฦ$(@D)กว    The directory part of the file name of the target, with the trailing slash removed. If the value of กฦ$@กว is dir/foo.o then กฦ$(@D)กว is dir. This value is . if กฦ$@กว does not contain a slash.
#กฦ$(@F)กว    The file-within-directory part of the file name of the target. If the value of กฦ$@กว is dir/foo.o then กฦ$(@F)กว is foo.o. กฦ$(@F)กว is equivalent to กฦ$(notdir $@)กว.


