This is the readme file for the Nuclear Messaging System project. In this file you will find information on how to compile, install and use the application.

The Nuclear Messaging System (nuke-ms) is intended as a reliable and secure distributed instant messaging system available on many platforms.
Right now it provides only a very rudimentary functionality and is probably only useful for testing and evaluation purposes.

nuke-ms is a free software project written by Alexander Korsunsky. It is hosted on BerliOS Developer, a german organization supporting the development of open source projects. Various services for nuke-ms such as a git repository, a bugtracker or a wiki page are hosted there. You can browse the nuke-ms project page here: http://developer.berlios.de/projects/nuke-ms/

================================================================================
Contents of this file:

1. Where to get nuke-ms
    1.1 Git Repository
    1.2 Download the last release from the BerliOS project page TODO!!
    
2. Installation
    2.1 Prerequisites TODO!! ( Windows)
    2.2 Compiling
        2.2.1 Compiling under Unix
        2.2.2 Compiling under Windows TODO!!
        
3. Usage

4. Compatibility notes

---------------------------------------------------------------------------------

1) Where to get nuke-ms 

1.1) Git Repository

You can download nuke-ms from the Git content tracker repository on BerliOS. 
For this purpose install Git (instructions can be found here: http://git.or.cz/gitwiki/Installation), and change into the directory you want to place the project folder. You can then check out the repository by entering the following into the command line:

    git clone git://git.berlios.de/nuke-ms 
    
This will create a folder called "nuke-ms" containing the most recent version all project files. To keep up with the development of nuke-ms you can download the latest changes by changing into the project directory and pulling from the repository:

    $ cd nuke-ms
    $ git pull
    
1.2) Download the last release from the BerliOS project page

 TODO
 
---------------------------------------------------------------------------------
 
2) Installation

nuke-ms is intended to be as portable as possible and it might compile on several platforms, however few were tested. 
It is mainly distributed as a source code tarball, but a compiled and zipped version for win32 platforms is available for convenience.

To use the win32 version of nuke-ms, you can simply extract the zip file to a folder. You can find the executables in the bin/ directory.
To compile nuke-ms, install the required prerequisites and follow the compilation instructions below.

 
2.1) Prerequisites

To compile nuke-ms from source you need to install some libraries and tools. These dependencies might seem annoying, however they allow effective and portable development.
These are the tools, their minimum required versions and their homepages that are needet to build nuke-ms:

    C++ Compiler
    A fairly recent C++ compiler is required to build nuke-ms. GCC is allways a safe bet, however other compilers might work as well. It's best to simply try out if you are able to compile nuke-ms.

    Boost C++ Libraries,    1.35,       http://www.boost.org/
    The Boost Libraries are a set of portable high quality libraries of which most are header-only. The libraries that need to be included at link-time are Boost.System and Boost.Thread. 
    
    wxWidgets,              2.8,        http://www.wxwidgets.org/
    wxWidgets is a library for portable GUI programming and allows a native look-and-feel on various platforms. The components used are base and core.
    
    CMake,                  2.6,        http://www.cmake.org/
    CMake is portable build tool that can create native build files (such as Makefiles or project files) on a variety of platforms.


On many GNU/Linux or Unix distributions the tools are packaged and ready to use. To install them you only need to specify the package names to your favourite package manager. Following you see a list of some distributions with instructions on how to install the prerequisites:


Debian GNU/Linux:
    4.0 ("Etch"):
        Install the following packages and their dependencies: build-essentials cmake libboost-system1.35-dev libboost-thread1.35-dev
        The Boost libraries are not contained in the official archives, however Debian Backpors provides these packages. Follow the instruction on this site to install the necessary packages: http://www.backports.org/dokuwiki/doku.php?id=instructions .
        Unfortunately, the wxWidgets 2.8 libraries are not included in Debian 4.0, not even in the backports archive. You will have to download them from the wxWidgets website and compile them yourself. Refer to the wxWidgets documentation for details.
    5.0 ("Lenny"):
        Install the following packages and their dependencies: build-essentials cmake libboost-system1.35-dev libboost-thread1.35-dev libwxgtk2.8-dev 
        The Boost libraries are not contained in the official archives, however Debian Backpors provides these packages. Follow the instruction on this site to install the necessary packages: http://www.backports.org/dokuwiki/doku.php?id=instructions .
    Testing ("Squeeze"), Unstable ("Sid"):
        Install the following packages and their dependencies: build-essentials cmake libboost-system1.35-dev libboost-thread1.35-dev libwxgtk2.8-dev 
        
    
Ubuntu:
    8.10 ("Intrepid Ibex"), 9.04 ("Jaunty Jackalope"):
        Install the following packages and their dependencies: cmake libboost-system1.35-dev libboost-thread1.35-dev libwxgtk2.8-dev
    9.10 ("Karmic Koala"):
        Install the following packages and their dependencies: cmake libboost-system-dev libboost-thread-dev libwxgtk2.8-dev
    
Fedora:
    Fedora 10 ("Cambridge"):
        Install the following packages and their dependencies: cmake wxGTK
        Unfortunately, the Boost libraries included in Fedora 10 are outdated. You will have to download them from the Boost website and compile them yourself. Refer to the Boost documentation for details.
    Fedora 11 ("Leonidas")
        Install the following packages and their dependencies: cmake boost wxGTK
    
Arch Linux:
    Install the following packages and their dependencies: cmake boost wxgtk
    
Windows:
    Prerequisite installation is pretty complicated on Windows. You might want to use the prepacked version of nuke-ms available here: TODO.
    First of all you have to install CMake, this is the easiest step. Just download the Win32 installer package, double click on it and follow the usual installation dialogue.
    The second step is to download and install wxWidgets and Boost. For wxWidgets, you come best off installing via the Win32 installer package, as the installer places the Libraries on a good place (somewhere where CMake can find them easily) by default, only the necessary headers and code are installed and because it offers nice buttons for documentation and uninstallation in the start menu. For Boost, you have to download the source package which suits your favourite compression utility best, and uncompress it to a folder.
    Now comes the tricky part: you have to compile the libraries. This is possible with several compilers, I only tested three. Of course, you have to compile the libraries with the same compiler you want to compile nuke-ms. The instructions for each can be found in the documentation of the libraries. Below are some points that should be mentioned and are specific to nuke-ms:
        - nuke-ms relies on strings to be unicode. This means that the wxWidgets libraries also have to be built as unicode. This is an option you have to set when compiling wxWidgets.
        - The linkage should be "consistent" throughout all libraries and nuke-ms. I recommend compiling all libraries as static, and link them statically to the executable. This implies some size overhead but saves you dll-troubles.
        - Use debugging symbols in all libraries and in nuke-ms, or nowhere. When building nuke-ms you will have the choice of including debugging symbols, and cmake will complain if you don't have the libraries with the right debugging configuration installed.        
        - When using MinGW, you have to check the "WinAPI" option in the MinGW installer, it is required for the GUI and the Network components. (Of course, g++ and make must be installed too.)
        - When using Visual Studio 2008 or 2005, you have to install two additional boost libraries, namely date_time and regex. This is because the boost thread library implicitly depends on date_time, which itself depends on regex. Other compilers are smart enough to understand that no symbols from there are used in the project and do not require their presence, but obviously Visual Studio is not.


For Platforms not listed here, please visit the respective websites and follow the instructions to install/compile the tools/libraries that are listed above.

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

---------------------------------------------------------------------------------

3) Usage

nuke-ms consists of two programs: a client that sends and receives messages to the server, called nuke-ms-client and a simple dispatching server that receives the messages from the clients and passes them on to other clients, called nuke-ms-serv.


Start the server by simply executing the nuke-ms-serv file. It does not take any parameters, shows no graphical or command line interface but simply listens on the port 34443 for incoming connections. 
To stop the server application you have to interrupt it, for example by hitting Ctrl-C in your console, with the kill program or with the Task Manager.

Then start the nuke-ms-client application, it should show a window with two text fields. Connect to a running server by entering the following command into the text input field of the window:
    /connect <host>:<port>
Replace <host> with the hostname (IP address or DNS name) of the host the server is running on. If this is your local computer, enter "localhost". 
The <port> is allways 34443. The first letter of the command must be a '/' (a slash), otherwise the command will not be recognized.
If you see a message that says something like "Connection succeeded.", you can now start typing messages in the text input field. As soon as you hit Enter the message will be sent. You can send multiline messages by inserting a line break by pressing Shift+Enter. The message you sent will be echoed back to you by the server.
If the connection attempt fails, a message with the reason will be displayed.
To tear down a connection, enter the command "/disconnect" into the text input field.
To close the application either Press on the X on the top right corner of the window, Select File->Quit from the menu or enter the command "/exit".

A note on security:
BEWARE! nuke-ms does not provide any means of encryption or authentication! The identity of a participant is not at all vouched, and the messages are sent unencryptedly over the network and therefore can be evesdropped. 
Dont run the server on security critical devices, as it is very easy to perform various attacks on it. Please use nuke-ms only in a trusted environment such as (your home network behind a firewall) and only for testing.
These problems are based on the fact that no measures have been taken yet to secure the program in this early phase of development. However, security is a major goal to this project and hopefully it will improve much in the future.

---------------------------------------------------------------------------------

4. Compatibility notes
 
Communication between clients on different systems, especially communication between Linux and Windows hosts is not possible. The reason lies in the difference of the default UTF encoding size on both platforms - Linux uses UTF-32 whereas Windows uses UTF-16. This issue will be addressed in near future.

nuke-ms is not yet a mature piece of software. As such, the communication protocol between the clients is also not mature and it is subject to ongoing change. Compatibilities between different versions is not granted at all in this early phase of development, in fact an incompatible change to the protocol is planned for the near future. To use the program for communication you should allways use the same version for every participant.
