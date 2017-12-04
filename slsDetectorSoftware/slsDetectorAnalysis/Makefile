#---------------------------------------------------


include Makefile.arch

#------------------------------------------------------------------------------

SLSDETO        =  TSlsDetectorDict.$(ObjSuf)  energyCalibration.$(ObjSuf) angularCalibration.$(ObjSuf)

SLSDETSO       = libTSlsDetector.$(DllSuf)

CXXFLAGS      += -Wno-deprecated 
#LIBS          += -L../slsDetectorSoftware -lSlsDetector -pthread

CXXFLAGS += -I ../usersFunctions

HEADERS= energyCalibration.h angularCalibration.h
#-------------------------------------------------------------------------------

#----------- do the shared library
$(SLSDETSO): $(SLSDETO)
	@echo "Generating shared library $@..."
	$(LD) $(SOFLAGS) $(LDFLAGS) -o $@ $(LIBS) $^


# ---------- Default rule

%.$(ObjSuf): %.cpp
	@echo "Compiling source $@..."
	$(CXX) $(CXXFLAGS) -c $<


#----------- do the dictionary
TSlsDetectorDict.$(SrcSuf): $(HEADERS)
	@echo "Generating dictionary $@..."
	rootcint -f $@ -c -p   $^

#-------------------------------------------------------------------------------
all:            $(SLSDETSO)

clean:
		@rm -f $(SLSDETO) core

distclean:      clean
		@rm -f *Dict.* *.so *.o










































