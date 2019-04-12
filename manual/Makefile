DESTDIR?=docs


#manual-api manual-calwiz manual-client manual-gui manual-main



MAINDIRS= manual-main manual-api  manual-calwiz manual-client manual-gui
#manual-calwiz manual-calwiz manual-gui manual-client manual-api
CLEANDIRS=$(MAINDIRS:manual-%=clean-%)
PDFDIRS=$(MAINDIRS:manual-%=pdf-%)
HTMLDIRS=$(MAINDIRS:manual-%=html-%)
ALLDIRS=$(MAINDIRS:manual-%=all-%)


all: $(ALLDIRS)
#	$(shell test -d $(DESTDIR)/pdf && rm -fr $(DESTDIR)/pdf)
#	mv pdf $(DESTDIR)
#	$(shell test -d $(DESTDIR)/html && rm -fr $(DESTDIR)/html)
#	mv html $(DESTDIR)
	
pdf: $(PDFDIRS)
#	$(shell test -d $(DESTDIR)/pdf && rm -fr $(DESTDIR)/pdf)
#	mv pdf $(DESTDIR)

html: $(HTMLDIRS)
#	$(shell test -d $(DESTDIR)/html && rm -fr $(DESTDIR)/html)
#	mv html $(DESTDIR)

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


#��$(@D)��    The directory part of the file name of the target, with the trailing slash removed. If the value of ��$@�� is dir/foo.o then ��$(@D)�� is dir. This value is . if ��$@�� does not contain a slash.
#��$(@F)��    The file-within-directory part of the file name of the target. If the value of ��$@�� is dir/foo.o then ��$(@F)�� is foo.o. ��$(@F)�� is equivalent to ��$(notdir $@)��.


