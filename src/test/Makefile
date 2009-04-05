# root of the project tree, relative from the current directory
PROJECT_ROOT := ../..

# include global makefile
include $(PROJECT_ROOT)/build/global.mak


# create a variable that contains all source files
# first expand the wildcard in the working directory
SOURCES := $(wildcard *.cpp)
# finally create the names for the executable files
EXECS := $(patsubst %.cpp,%,$(SOURCES))


#LDFLAGS += -l$(B_LIB_SYSTEM)

all: $(EXECS)

%: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) -lnukems

clean:
	-$(RMF) $(EXECS)

