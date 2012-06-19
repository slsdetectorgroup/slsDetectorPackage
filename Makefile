# do not change below this line


WD=$(shell pwd)
LIBRARYDIR=$(WD)/slsDetectorSoftware
TLIBRARYDIR=$(WD)/TMythenDetector
CLIENTDIR=$(WD)/slsDetectorClient
GUIDIR=$(WD)/mythenGUI
LIBDOCDIR=$(WD)/slsDetectorSoftware




#FLAGS=-DVERBOSE



all: lib  slsDetectorClient
	

lib:
	cd $(LIBRARYDIR) && $(MAKE) lib FLAGS=$(FLAGS)

slsDetectorClient: lib
	cd  $(CLIENTDIR) && $(MAKE)  FLAGS=$(FLAGS)
	$(shell test -d bin || mkdir -p bin)
	mv $(CLIENTDIR)/bin/* bin/


clean:
	rm -rf bin/sls_detector_*
	cd $(LIBRARYDIR) && $(MAKE) clean
	cd $(CLIENTDIR) && $(MAKE) clean

install_lib:
	cd $(LIBRARYDIR) && $(MAKE) install_lib DESTDIR=$(INSTALLROOT)/$(LIBDIR)
	cd $(LIBRARYDIR) && $(MAKE) install_inc DESTDIR=$(INSTALLROOT)/$(INCDIR)


install_client:
	cd $(CLIENTDIR) && $(MAKE) install DESTDIR=$(INSTALLROOT)/$(BINDIR)


install_libdoc: lib_doc
	cd $(LIBDOCDIR)  && $(MAKE) install_doc DESTDIR=$(INSTALLROOT)/$(DOCDIR)/slsDetector

install_clientdoc:
	cd $(CLIENTDIR) && $(MAKE) install_doc DESTDIR=$(INSTALLROOT)/$(DOCDIR)/slsDetectorClient

lib_doc:
	cd $(LIBDOCDIR)  && $(MAKE) doc

install_doc: install_libdoc install_clientdoc 
	cp -r manual $(INSTALLROOT)/$(DOCDIR)/


install: conf install_lib install_client install_doc
	


conf:
	@echo "QTDIR is $(QTDIR)"
	@echo "ROOTSYS is $(ROOTSYS)"
	@echo "INSTALLROOT is $(INSTALLROOT)"
	@echo "BINDIR is $(BINDIR)"
	@echo "LIBDIR is $(LIBDIR)"
	@echo "INCDIR is $(INCDIR)"
	@echo "DOCDIR is $(DOCDIR)"

tar:
	cd .. && tar czf newMythenSoftware.tgz newMythenSoftware

help:
	@echo "Targets:"
	@echo "make all           	compile library, mythenClient and mythenGUI"
	@echo "make lib           	compile library"
	@echo "make slsDetectorClient  	compile slsDetectorClient"
	@echo "make install_client     install slsDetectorClient"
	@echo "make install_lib        install detector library and include files"
	@echo "make install            install library, include files, mythenClient and mythenGUI"
	@echo "make install_libdoc     install library documentaion"
	@echo "make install_clientdoc  install mythenClient documentation"
	@echo "make install_doc        install all documentation"
	@echo "make clean              remove object files and executables"
	@echo "make help               lists possible targets"
	@echo ""
	@echo "Variables:"
	@echo "INSTALLROOT=</yourdir>:    installation root dir, default /usr/local"
	@echo "BINDIR=<yourbin>:          binary installation dir below INSTALLROOT, default bin"
	@echo "LIBDIR=<yourlib>:          library installation dir below INSTALLROOT, default lib"
	@echo "INCDIR=<yourincludes>:     header installation dir below INSTALLROOT, default include/slsdetector"
	@echo "DOCDIR=<yourdoc>:          documentation installation dir below INSTALLROOT, default share/doc"
