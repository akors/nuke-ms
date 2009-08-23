This is the readme file for the Nuclear Messaging System project. In this file you will find information on how to compile, install and use the application.

The Nuclear Messaging System (nuke-ms) is intended as a reliable and secure distributed instant messaging system available on many platforms.
Right now it provides only a very rudimentary functionality and is probably only useful for testing and evaluatio purposes.

nuke-ms is a free software project hosted on BerliOS Developer, a german organization supporting the development of open source projects. Various services for nuke-ms such as a git repository, a bugtracker or a wiki page. You can browse the nuke-ms project page here: http://developer.berlios.de/projects/nuke-ms/
    
1. Where to get nuke-ms
2. Installation
3. Usage
4. Compatibility notes
5. Contributing




1) Where to get nuke-ms

1.1) Git Repository

You can download nuke-ms from the Git content tracker repository on BerliOS. 
For this purpose install Git (instructions can be found here: http://git.or.cz/gitwiki/Installation), and change into the directory you want to place the project folder. You can then check out the repository by entering the following into the command line:

    git clone git://git.berlios.de/nuke-ms 
    
This will create a folder called "nuke-ms" containing the most recent version all project files. To keep up with the development of nuke-ms you can download the latest changes by changing into the project directory and pulling from the repository:

    cd nuke-ms
    git pull
    
1.2) Download the last release from the BerliOS project page

 TODO
 
 
2) Installation

nuke-ms is intended to be as portable as possible and it might compile on several platforms, however few were tested. 
It is mainly distributed as a source code tarball, but a compiled and zipped version for win32 platforms is available for convenience.

To use the win32 version of nuke-ms, simply extract the zip file to a folder. You can find the executables in the bin/ directory.

 
2.1) Prerequisites

To compile nuke-ms from source you need to install some libraries and tools. These dependencies might seem annoying, however they allow effective and portable development.
These are the tools, their minimum required versions and their homepages that are needet to build nuke-ms:

    C++ Compiler
    A fairly recent C++ compiler is required to build nuke-ms. GCC is allways a safe bet, however other compilers might also work. It's best to simply try out if you are able to compile nuke-ms.

    Boost C++ Libraries,    1.35,   http://www.boost.org/
    The Boost Libraries are a set of portable high quality libraries of which most are header-only. The libraries that need to be included at link-time are Boost.System and Boost.Thread. 
    
    wxWidgets,              2.8,    http://www.wxwidgets.org/
    wxWidgets is a library for portable GUI programming and allows a native look-and-feel on various platforms. The components used are base and core.
    
    CMake,                  2.6,    http://www.cmake.org/
    CMake is portable build tool that can create native build files (such as Makefiles or project files) on a variety of platforms.


On many GNU/Linux or Unix distributions the tools are packaged and ready to use. To install them you only need to specify the package names to your favourite package manager. Following you see a list of some distributions with instructions on how to install the prerequisites:


Debian GNU/Linux:
    5.0 ("Lenny"):
    Testing ("Squeeze"):
    Unstable ("Sid"):
        Install the following packages and their dependencies: build-essentials cmake libboost-system1.35-dev libboost-thread1.35-dev libwxgtk2.8-dev 
        For Debian 5.0 "Squeeze", the Boost libraries are not contained in the official archives, however Debian Backpors provides these packages. Follow the instruction on this site to install the necessary packages: http://www.backports.org/dokuwiki/doku.php?id=instructions .
        
    
Ubuntu:
    8.10 ("Intrepid Ibex"):
    9.04 ("Jaunty Jackalope"):
        Install the following packages and their dependencies: cmake libboost-system1.35-dev libboost-thread1.35-dev libwxgtk2.8-dev
    8.10 ("Karmic Koala"):
        TODO
    
Fedora:
    TODO
    
Arch Linux:
    TODO
    

For Platforms not listed here (such as Windows), please visit the respective websites and follow the instructions to install/compile the tools/libraries that are listed above.

2.2) Compiling

2.2.1) Compiling under Unix

After installing the required prerequisites, you can start compiling nuke-ms. 
To do so, change into a directory where you want to build nuke-ms, this directory can't be the same directory as the one the sourcecode lies in. You might want to use the "build" directory provided in the project root directory for this.
Then run the "cmake" command with the directory of the nuke-ms source - this is the "src" directory of the project directory - as argument. You might want to specify additional options, such as the location of the libraries or the generator to be used:
    add "-DBOOST_INCLUDEDIR" to specify the Boost include directory or "-DBOOST_LIBRARYDIR" to specify the Boost library directory to the command line in case cmake has troubles finding Boost
    add "-G" and your favourite toolchain supported by cmake to produce something other than makefiles for GNU Make. See the cmake manpage for the available generators.
Finally call your favourite toolchain to compile the source. This will simply be "make" if you didn't give the "-G" option to cmake.

A simple compilation of nuke-ms might look like this:
    $ cd ~/nuke-ms/build
    $ cmake ~/nuke-ms/src
    $ make

Of course you have to change the path for the project directory according to where you put it.

If the compilation succeeded you will find the executables in the "bin" directory of the directory you built nuke-ms in.



2.2.2) Compiling under Windows


3) Usage

nuke-ms consists of two programs: a client that sends and receives messages to the server, called nuke-ms-client and a simple dispatching server that receives the messages from the clients and passes them on to other clients, called nuke-ms-serv.


Start the server by simply executing the nuke-ms-serv file. It does not take any parameters, shows no graphical or command line interface but simply listens on the port 34443 for incoming connections. To stop the server application you have to interrupt it, for example by hitting Ctrl-C in your console, with the kill program or with the Task Manager.


Then start the nuke-ms-client application, it should show a window with two text fields. Connect to a running server by entering the following command into the text input field of the window:
    /connect <host>:<port>
Replace <host> with the hostname (IP address or DNS name) of the host the server is running on. If this is your local computer, enter "localhost". 
The <port> is allways 34443. The first letter of the command must be a '/', a slash, otherwise the command will not be recognized.
If you see a message that says something like "Connection succeeded.", you can now start typing messages in the text input field. As soon as you hit Enter the message will be sent. You can send multiline messages by inserting a line break by pressing Shift+Enter. The message you sent will be echoed back to you by the server.
If the connection attempt fails, a message with the reason will be displayed.
To tear down a connection, enter the command "/disconnect" into the text input field.
To close the application either Press on the X on the top right corner of the window, Select File->Quit from the menu or enter the command "/exit".

A note on security:
BEWARE! nuke-ms does not provide any means of encryption or authentication! The identity of a participant is not at all vouched, and the messages are sent unencryptedly over the network and therefore can be evesdropped. 
Dont run the server on security critical devices, as it is very easy to perform various attacks on it. In fact, please use nuke-ms only in a trusted environment and only for testing.
These problems are based on the fact that no measures have been taken yet to secure the program in this early phase of development. However, security is a major goal to this project and hopefully it will improve much in the future.


4. Compatibility notes
 
The communication between clients from windows and linux TODO

nuke-ms is not yet a mature piece of software. As such, the communication protocol between the clients is also not mature and it is subject to ongoing change. Compatibilities between different versions is not granted at all in this early phase of development, in fact an incompatible change to the protocol is planned for the recent future. To use the program for communication you should allways use the same version for every participant.
