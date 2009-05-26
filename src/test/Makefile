# root of the project tree, relative from the current directory
PROJECT_ROOT := ../..

# include global makefile
include $(PROJECT_ROOT)/build/global.mak

EXECS := teststringwrap testownership testsegmlayer
#LDFLAGS += -l$(B_LIB_SYSTEM)

all: $(EXECS)


teststringwrap: teststringwrap.cpp byteprinter.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) -lnukems

testsegmlayer: testsegmlayer.cpp byteprinter.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) -lnukems


%: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) -lnukems


clean:
	-$(RMF) $(EXECS)

