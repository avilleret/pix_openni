# change to your local directories!
OPENNI_DIR=/home/nusmuk/kinect/OpenNI-Linux-x64-2.2

#linux doesnt work yet
UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
 CPPFLAGS = `pkg-config --cflags pd` `pkg-config --cflags Gem` -I$(OPENNI_DIR)/Include
 CXXFLAGS = -g -O2 -fPIC -freg-struct-return -Os -falign-loops=32 -falign-functions=32 -falign-jumps=32 -funroll-loops -ffast-math -mmmx
 LDFLAGS = -shared -rdynamic -Wl,-rpath,"\$$ORIGIN",--enable-new-dtags
 LIBS = -lOpenNI2 -L.
 EXTENSION = pd_linux
 USER_EXTERNALS=$(HOME)/pd-externals
endif
ifeq ($(UNAME),Darwin)
 CPPFLAGS = -I$(GEM_DIR)/src -I$(PD_APP_DIR)/include
 CPPFLAGS += -I/sw/include/ni
 CXXFLAGS = -g -fast -msse3 -arch i386
 LDFLAGS = -arch i386 -bundle -bundle_loader $(PD_APP_DIR)/bin/pd -undefined dynamic_lookup -mmacosx-version-min=10.6 -L/sw/lib
 LIBS = -lm -lOpenNI
 EXTENSION = pd_darwin
 USER_EXTERNALS=$(HOME)/Library/Pd
endif

.SUFFIXES = $(EXTENSION)

SOURCES = pix_openni2

all:
	g++ $(CPPFLAGS) $(CXXFLAGS) -o $(SOURCES).o -c $(SOURCES).cc
	g++ -o $(SOURCES).$(EXTENSION) $(LDFLAGS) $(SOURCES).o $(LIBS)
	rm -fr ./*.o

deploy:
	install -d $(USER_EXTERNALS)/pix_openni
	install -p *.pd $(USER_EXTERNALS)/pix_openni
	install -p $(SOURCES).$(EXTENSION) $(USER_EXTERNALS)/pix_openni

clean:
	rm -f $(SOURCES)*.o
	rm -f $(SOURCES)*.$(EXTENSION)

distro: clean all
	rm *.o
