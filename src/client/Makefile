# root of the project tree, relative from the current directory
PROJECT_ROOT := ../..

# include global makefile
include $(PROJECT_ROOT)/make/global.mak


# the list of the components of the application
COMPONENTS := gui protocol controls

# create a variable that contains all source files
# first expand the wildcard in the working directory
SOURCES := $(wildcard *.cpp)
# then expand the wildcards in the components directories
SOURCES += $(foreach comp,$(COMPONENTS),$(wildcard $(comp)/*.cpp))
# finally create the names for the object file
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))

LDFLAGS += -l$(B_LIB_THREAD) -l$(B_LIB_SYSTEM) $(WX_LIBS)

# add the current directory to the Include flags
CXXFLAGS += -I.

all: nuke-ms-client

nuke-ms-client: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) -lnukems

clean:
	-$(RMF) $(OBJECTS) nuke-ms-client
