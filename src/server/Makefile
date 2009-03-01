# root of the project tree, relative from the current directory
PROJECT_ROOT := ../..

# include global makefile
include $(PROJECT_ROOT)/build/global.mak


# create a variable that contains all source files
# first expand the wildcard in the working directory
SOURCES := $(wildcard *.cpp)
# finally create the names for the object file
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))


LDFLAGS += -l$(B_LIB_SYSTEM)

all: nuke-ms-serv

nuke-ms-serv: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) -lnuke-mscommon

clean:
	-$(RMF) $(OBJECTS) nuke-ms-serv

