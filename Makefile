# do not change below this line



WD=$(shell pwd)
LIBRARYDIR=$(WD)/slsDetectorSoftware
TLIBRARYDIR=$(WD)/TMythenDetector
CLIENTDIR=$(WD)/mythenClient
GUIDIR=$(WD)/mythenGUI
LIBDOCDIR=$(WD)/slsDetectorSoftware


all: lib mythenClient mythenGUI 


lib:
	cd $(LIBRARYDIR) && $(MAKE) lib

Tlib:
	cd $(TLIBRARYDIR) && $(MAKE) lib

mythenClient: lib
	cd $(CLIENTDIR) && $(MAKE)
	mv $(CLIENTDIR)/bin/* bin/

mythenGUI: lib Tlib 
	cd $(GUIDIR) && qmake mythenGUI.pro
	cd $(GUIDIR) && $(MAKE)
	mv $(GUIDIR)/bin/* bin/

clean:
	rm bin/*
	cd $(LIBRARYDIR) && $(MAKE) clean
	cd $(TLIBRARYDIR) && $(MAKE) clean
	cd $(CLIENTDIR) && $(MAKE) clean
	cd $(GUIDIR) && $(MAKE) clean
#	cd $(LIBDOCDIR) && $(MAKE) clean

install_lib:
	cd $(LIBRARYDIR) && $(MAKE) install_lib DESTDIR=$(INSTALLROOT)/$(LIBDIR)
	cd $(LIBRARYDIR) && $(MAKE) install_inc DESTDIR=$(INSTALLROOT)/$(INCDIR)

install_tlib:
	cd $(TLIBRARYDIR) && $(MAKE) install_lib DESTDIR=$(INSTALLROOT)/$(LIBDIR)
	cd $(TLIBRARYDIR) && $(MAKE) install_inc DESTDIR=$(INSTALLROOT)/$(INCDIR)

install_client:
	cd $(CLIENTDIR) && $(MAKE) install DESTDIR=$(INSTALLROOT)/$(BINDIR)

install_gui:  
	cd $(GUIDIR) && $(MAKE) install_target DESTDIR=$(INSTALLROOT)/$(BINDIR)

install_libdoc: lib_doc
	cd $(LIBDOCDIR)  && $(MAKE) install_doc DESTDIR=$(INSTALLROOT)/$(DOCDIR)/slsdetector

install_clientdoc:
	cd $(CLIENTDIR) && $(MAKE) install_doc DESTDIR=$(INSTALLROOT)/$(DOCDIR)/mythenClient

install_guidoc: 
	cd $(GUIDIR) && doxygen doxy.config
	cd $(GUIDIR) && qmake
	cd $(GUIDIR) && $(MAKE) install_documentation INSTALL_ROOT=$(INSTALLROOT) DOCPATH=$(DOCDIR)/mythenGui

lib_doc:
	cd $(LIBDOCDIR)  && $(MAKE) doc

install_doc: install_libdoc install_clientdoc install_guidoc


install: configure install_lib install_client install_gui install_doc


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
	@echo "make tlib           	compile Root/Qt library"
	@echo "make mythenClient  	compile mythenClient"
	@echo "make mythenGUI	   	compile mythenGUI"
	@echo "make install_client     install mythenClient"
	@echo "make install_gui        install mythenGUI"
	@echo "make install_lib        install detector library and include files"
	@echo "make install_tlib        install detector Root/Qt library and include files"
	@echo "make install            install library, include files, mythenClient and mythenGUI"
	@echo "make install_libdoc     install library documentaion"
	@echo "make install_clientdoc  install mythenClient documentation"
	@echo "make install_guidoc     install mythenGUI documentation"
	@echo "make install_doc        install all documentation"
	@echo "make clean              remove object files and executables"
	@echo "make help               lists possible targets"
	@echo ""
	@echo "Variables:"
	@echo "INSTALLROOT=</yourdir>:    installation root dir, default /usr/local"
	@echo "QTDIR=</yourqtdir>:	   your qt3 installation, default /usr/lib/qt-3.3"
	@echo "ROOTSYS=</yourroot>:	   your root installation, default /usr/local/root"
	@echo "BINDIR=<yourbin>:          binary installation dir below INSTALLROOT, default bin"
	@echo "LIBDIR=<yourlib>:          library installation dir below INSTALLROOT, default lib"
	@echo "INCDIR=<yourincludes>:     header installation dir below INSTALLROOT, default include/slsdetector"
	@echo "DOCDIR=<yourdoc>:          documentation installation dir below INSTALLROOT, default share/doc"
