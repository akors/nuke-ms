
# directory stuff
DOXYFILE=../doc/Doxyfile

# boost stuff
BOOST_INCLUDE_DIR=/usr/local/include/boost-1_35/
BOOST_LIB_DIR=/usr/local/lib/

# boost library names follow a certain pattern
B_PREF=lib
B_TOOLSET=-gcc42
B_THREADING=-mt
B_ABI=
B_VERSION=-1_35
B_EXT=.a

#now composing all boost libraries used
B_LIB_SYSTEM=boost_system$(B_TOOLSET)$(B_THREADING)$(B_ABI)$(B_VERSION)

B_LIB_THREADS=boost_thread$(B_TOOLSET)$(B_THREADING)$(B_ABI)$(B_VERSION)


# wxWidgets stuff
WX_CXXFLAGS=`wx-config --cxxflags` 
WX_LIBS= `wx-config --libs` 


CXXFLAGS=-g -I$(BOOST_INCLUDE_DIR)


all: nms

doc: 
	doxygen $(DOXYFILE)
	

nms: main.o control.o protocol.o
	$(CXX) $(CXXFLAGS)  $( WX_CXXFLAGS) $(WX_LIBS) -o $@ $^

main.o: main.cpp
	$(CXX) $(CXXFLAGS) $(WX_CXXFLAGS) -c -o $@ $^
	
	
	
helloworld_asio: helloworld_asio.cpp
	$(CXX) $(CXXFLAGS) -L$(BOOST_LIB_DIR) -l$(B_LIB_SYSTEM) -l$(B_LIB_THREADS) -o $@ $^ 


clean:
	rm -f nms *.o

doc_clean:
	rm -r ../doc/html
