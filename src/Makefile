
# directory stuff
DOXYFILE=../doc/Doxyfile


# boost library names follow a certain pattern

B_TOOLSET=-gcc42
B_THREADING=-mt
B_ABI=
B_VERSION=-1_35
B_EXT=.a
B_PREF=lib


B_SUFFIX=-gcc42-mt-1_35


#now composing all boost libraries used
B_LIB_SYSTEM=boost_system$(B_SUFFIX)

B_LIB_THREADS=boost_thread$(B_SUFFIX)


# wxWidgets stuff
WX_CXXFLAGS=`wx-config --cxxflags` 
WX_LIBS= `wx-config --libs` 


CXXFLAGS=-g


all: nms

doc: 
	doxygen $(DOXYFILE)
	

nms: main.o control.o protocol.o
	$(CXX) $(CXXFLAGS) -l$(B_LIB_SYSTEM) $( WX_CXXFLAGS) $(WX_LIBS) -o $@ $^ 

main.o: main.cpp
	$(CXX) $(CXXFLAGS) $(WX_CXXFLAGS) -c -o $@ $^
	
	
	
helloworld_asio: helloworld_asio.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ -l$(B_LIB_SYSTEM) -l$(B_LIB_THREADS)


clean:
	rm -f nms *.o *~

doc_clean:
	rm -r ../doc/html
