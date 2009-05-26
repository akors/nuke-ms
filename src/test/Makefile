# root of the project tree, relative from the current directory
PROJECT_ROOT := ../..

# include global makefile
include $(PROJECT_ROOT)/build/global.mak

EXECS := teststringwrap testownership
#LDFLAGS += -l$(B_LIB_SYSTEM)

all: $(EXECS)

teststringwrap: byteprinter.o

%: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) -lnukems

clean:
	-$(RMF) $(EXECS)

