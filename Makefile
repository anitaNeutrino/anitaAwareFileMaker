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



PROGRAM =    makePrettyHkJsonFiles makeHeaderJsonFiles makeEventJsonFiles makeAcqdStartRunJsonFiles makeMonitorHkJsonFiles

all : $(PROGRAM)


% :  %.$(SRCSUF) 
	@echo "<**Linking**> "  
	$(LD)  $(CXXFLAGS) $(LDFLAGS)  $<  $(LIBS) -o $@


clean:
	@rm -f *Dict*
	@rm -f *.${OBJSUF}
	@rm -f $(PROGRAM)

