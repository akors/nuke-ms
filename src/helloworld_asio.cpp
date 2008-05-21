// helloworld_asio.cpp

#include <iostream>
#include <string>
#include <boost/system/system_error.hpp>
#include <boost/asio.hpp>

namespace ba = boost::asio;


int main()
{
	
	try {
		// create an I/O service object
		ba::io_service io;
		
		// create an endpoint,
		ba::ip::tcp::endpoint ep(ba::ip::address_v4::loopback(), 34443);
		
		// create a socket
		ba::ip::tcp::socket s(io);
		
		// and connect
		s.connect(ep);
		
		
		// now create a string
		std::string str("Hello Remote world!\n");
		
		// and dump it on the wire
		s.send(ba::buffer(str));
		
		
		s.close();
	}
	catch (boost::system::system_error& e)
	{
		std::cout<<"Operation failed: "<<e.what()<<"\n";
	}
	
	
	return 0;
}
