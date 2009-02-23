# root of the project tree, relative from the current directory
PROJECT_ROOT := ../..

# include global makefile
include $(PROJECT_ROOT)/build/global.mak


# create a variable that contains all source files
# first expand the wildcard in the working directory
SOURCES := $(wildcard *.cpp)
# finally create the names for the object file
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))

all: libnmscommon.a

libnmscommon.a: $(OBJECTS)
	$(AR) rcs $@ $^

clean:
	-$(RMF) $(OBJECTS) libnmscommon.a

