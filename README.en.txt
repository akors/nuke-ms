This is the readme file for the Nuclear Messaging System project. In this file you can find informations about the project and informations on how to obtain and use the program.

The Nuclear Messaging System (nuke-ms) is intended as a reliable and secure distributed instant messaging system available on many platforms.
Right now it provides only a very rudimentary functionality and is probably only useful for testing and evaluation purposes.

nuke-ms is a free software project written by Alexander Korsunsky. You can use, modify and redistribute it under the conditions of the GNU General Public License version 3 (GPLv3), a copy of which can be found in the file LICENSE.txt in the "doc" folder.

The project is hosted on BerliOS Developer, a german organization supporting the development of open source projects. Various services for nuke-ms such as a git repository, a bugtracker or a wiki page are hosted there. You can browse the nuke-ms project page here: http://developer.berlios.de/projects/nuke-ms/


================================================================================
Contents of this file:

1. Where to get nuke-ms
    1.1 Git Repository
    1.2 Download the latest release from the BerliOS project page

2. Installation

3. Usage

4. Compatibility notes

5. Help, comments, bugreports and patches

---------------------------------------------------------------------------------

1) Where to get nuke-ms

1.1) Git Repository

You can download nuke-ms from the Git content tracker repository on BerliOS.
For this purpose install Git (instructions can be found here: http://git.or.cz/gitwiki/Installation), and change into the directory you want to place the project folder. You can then check out the repository by entering the following into the command line:

    git clone git://git.berlios.de/nuke-ms

This will create a folder called "nuke-ms" containing the most recent version all project files. To keep up with the development of nuke-ms you can download the latest changes by changing into the project directory and pulling from the repository:

    $ cd nuke-ms
    $ git pull

1.2) Download the latest release from the BerliOS project page

You can download the source code and a compiled win32 version of nuke-ms on the BerliOS project page from these addresses:

    ftp://ftp.berlios.de/pub/nuke-ms/alpha/nuke-ms-0.2-src.tar.gz
        This is the source code version of nuke-ms, with unix line endings (LF).

    ftp://ftp.berlios.de/pub/nuke-ms/alpha/nuke-ms-0.2-win32.zip
        This is the compiled win32 version, with all text files having windows line endings (CR LF).


---------------------------------------------------------------------------------

2) Installation

For the compiled win32 version, download the archive from the link above and uncompress it. The binaries can be found in the "bin" directory of the project tree and are called "nuke-ms-client.exe", "nuke-ms-serv.exe". Some dll's are required to run nuke-ms, you can find them in the bin/ directory. Make sure these dll-files resides in the same directory as the exe-files.

For the source code version, the short instructions for compiling the application under unixlike system are as follows:
    - Install CMake >= 2.6, wxWidgets >= 2.8 and Boost >= 1.39
    - Change into a directory where you want to build nuke-ms, run cmake on the project tree and finally run make:
        
        $ cd nuke-ms/build
        $ cmake ..
        $ make
      The binaries can then be found in the "bin" directory of the directory you built nuke-ms in.

For detailed instructions on installation of the prerequisites and compilation on Unixlike and Windows platforms, please refer to the file INSTALL.en.txt in the "doc" directory of the project tree.


---------------------------------------------------------------------------------

3) Usage

nuke-ms consists of two programs: a client that sends and receives messages to the server, called nuke-ms-client and a simple dispatching server that receives the messages from the clients and passes them on to other clients, called nuke-ms-serv.


Start the server by simply executing the nuke-ms-serv file. It does not take any parameters, shows no graphical or command line interface but simply listens on the port 34443 for incoming connections. If you have a nagging firewall, allow the server to bind to a port, that means click the "Allow", "Do not block", "Unblock" Button or anything similar of your firewall nag window.
To stop the server application you have to interrupt it, for example by hitting Ctrl-C in your console, with the "kill" program or with the Task Manager.

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
Dont run the server on security critical devices, as it is very easy to perform various attacks on it. Please use nuke-ms only in a trusted environment (such as your home network behind a firewall) and only for testing.
These problems are based on the fact that no measures have been taken yet to secure the program in this early phase of development. However, security is a major goal to this project and hopefully it will improve much in the future.

---------------------------------------------------------------------------------

4. Compatibility notes

nuke-ms is not yet a mature piece of software. As such, the communication protocol between the clients is also not mature and it is subject to ongoing change. Compatibilities between different versions is not granted at all in this early phase of development, in fact an incompatible change to the protocol is planned for the near future. To use the program for communication you should allways use the same version for every participant.

---------------------------------------------------------------------------------

5. Help, comments, bugreports and patches

I am very interested in any feedback you have. Tell me how you find nuke-ms!
If you need help with nuke-ms, have noticed some bugs, have changed the program and wrote a patch or simply want to contact me, send an email to the nuke-ms mailing list.
The address of this list is:
    nuke-ms-users@lists.berlios.de
To send emails to this list, you have to subscribe (don't worry, it doesn't cost anything and you won't get any spam except the one you send to the list yourself ;-) ) on this site:
    https://lists.berlios.de/mailman/listinfo/nuke-ms-users

If you have something to tell me that you think isn't suited for public, mail me to this address: fat.lobyte9@gmail.com

