#############################################################################
## Makefile -- New Version of my Makefile that works on both linux
##              and mac os x
## Ryan Nichol <rjn@hep.ucl.ac.uk>
##############################################################################
include Makefile.arch

SRCSUF=cxx
DLLSUF=so
OBJSUF=o

SYSLIBS=#-L /home/rjn/thirdParty/lib/ -l profiler

#Toggles the FFT functions on and off
USE_FFT_TOOLS=1

ifdef USE_FFT_TOOLS
FFTLIBS = -lRootFftwWrapper -lfftw3
FFTFLAG = -DUSE_FFT_TOOLS
else
FFTLIBS =
FFTFLAG =
endif


ifdef ANITA_UTIL_INSTALL_DIR
ANITA_UTIL_LIB_DIR=${ANITA_UTIL_INSTALL_DIR}/lib
ANITA_UTIL_INC_DIR=${ANITA_UTIL_INSTALL_DIR}/include
LD_ANITA_UTIL=-L$(ANITA_UTIL_LIB_DIR)
INC_ANITA_UTIL=-I$(ANITA_UTIL_INC_DIR)
ANITA_UTIL_MAP_DIR=$(ANITA_UTIL_INSTALL_DIR)/share/anitaMap
else
ANITA_UTIL_LIB_DIR=/usr/local/lib
ANITA_UTIL_INC_DIR=/usr/local/include
ANITA_UTIL_MAP_DIR=/usr/local/share/anitaMap
ifdef EVENT_READER_DIR
LD_ANITA_UTIL=-L$(EVENT_READER_DIR)
INC_ANITA_UTIL=-I$(EVENT_READER_DIR)
endif
endif


AWARE_INCLUDES = -I../
AWARE_LIBS = -L../ -l AwareWeb

#Generic and Site Specific Flags
CXXFLAGS     += $(ROOTCFLAGS) $(FFTFLAG) $(SYSINCLUDES) $(AWARE_INCLUDES) $(INC_ANITA_UTIL)
LDFLAGS      += -g $(ROOTLDFLAGS) 

LIBS          = $(ROOTLIBS)  -lMinuit $(SYSLIBS)  $(LD_ANITA_UTIL) -lAnitaEvent $(FFTLIBS) $(AWARE_LIBS)
GLIBS         = $(ROOTGLIBS) $(SYSLIBS)



#Now the bits we're actually compiling
ROOT_LIBRARY = libAnitaAware.${DLLSUF}
LIB_OBJS =  AnitaAwareHandler.o
CLASS_HEADERS =  AnitaAwareHandler.h 



PROGRAM = makeHkJsonFiles makeSSHkJsonFiles makeHeaderJsonFiles makeEventJsonFiles makeAcqdStartRunJsonFiles makeMonitorHkJsonFiles makeOtherMonitorHkJsonFiles makeSurfHkJsonFiles makeAdu5PatJsonFiles makeAdu5SatJsonFiles makeAdu5VtgJsonFiles makeG12PosJsonFiles makeG12SatJsonFiles makeGpsGgaJsonFiles makeAvgSurfHkJsonFiles makeTurfRateJsonFiles makeSumTurfRateJsonFiles



all : $(PROGRAM) $(ROOT_LIBRARY)

#The library
$(ROOT_LIBRARY) : $(LIB_OBJS) 
	@echo "Linking $@ ..."
ifeq ($(PLATFORM),macosx)
# We need to make both the .dylib and the .so
	$(LD) $(SOFLAGS) $^ $(OutPutOpt) $@
ifeq ($(MACOSX_MINOR),4)
	ln -sf $@ $(subst .$(DLLSUF),.so,$@)
else
	$(LD) -bundle -undefined $(UNDEFOPT) $(LDFLAGS) $^ \
	 $(OutPutOpt) $(subst .$(DLLSUF),.so,$@)
endif
else
	$(LD) $(SOFLAGS) $(LDFLAGS) $(LIB_OBJS) $(LIBS)  -o $@
endif



% :  %.$(SRCSUF) $(ROOT_LIBRARY)
	@echo "<**Linking**> "  
	$(LD)  $(CXXFLAGS) $(LDFLAGS)  $<  $(LIBS) $(ROOT_LIBRARY) -o $@


%.$(OBJSUF) : %.$(SRCSUF)
	@echo "<**Compiling**> "$<
	$(CXX) $(CXXFLAGS) -c $< -o  $@


install: $(ROOT_LIBRARY)
ifeq ($(PLATFORM),macosx)
	cp $(ROOT_LIBRARY) $(subst .$(DLLSUF),.so,$(ROOT_LIBRARY)) $(ANITA_UTIL_LIB_DIR)
else
	cp $(ROOT_LIBRARY) $(ANITA_UTIL_LIB_DIR)
endif
	cp  $(CLASS_HEADERS) $(ANITA_UTIL_INC_DIR)


clean:
	@rm -f *Dict*
	@rm -f *.${OBJSUF}
	@rm -f $(PROGRAM)


