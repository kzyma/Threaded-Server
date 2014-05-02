************************************************************************

Author: Vaishnavi Talegaonkar
        Ken Zyma
Term:   Spring 2014
Course: CSC552
Assignment: A Web Server: Threads

************************************************************************
INTRODUCTION

This program implements a web server using threads in UNIX to service 
client connections and respond to their HTTP requests. This webserver can parse
requests for .jpg images, texts and html forms.
 
Usage: webserver-threads <port>

URL to test: http://unixweb.kutztown.edu:<port>/index.html

************************************************************************
HOW TO COMPILE

1.Run 'make' utility to compile and link to executable named 'webserver-threads’

2.To compile and run manually -

gcc -o webserver-threads webserver-threads.c sockutils.o HTTPheader.o \
	RequestQueue.o -lnsl -lsocket

************************************************************************
HOW TO RUN

Run the executable webserver with port number as below:

./webserver-threads <port>

************************************************************************
DESIGN OVERVIEW

SDLC & PROGRAMMING METHODOLOGY:

The webserver was developed interatively using Pair-Programming methodology. 
The initial version of webserver using IPC was enhanced by replacing processes with threads.
The project code is written in 'modular' fashion and tested for ability to serve multiple 
client connections using threads, generated and managed using a 'thread-pool'. 

PROJECT DESIGN:

The webserver program consists of three programs -

1. sockutils
This program provides auxilary functions to read from and write to the sockets.

2. webserver-threads.c
This program provides the main functionality of the webserver and implements
functions such as socket, bind, listen, accept, gethostbyaddr and signal handlers.

The major functions which provide the above functionality are as follows -
	1. signal_handler:Implements SIGINT, SIGHUP, SIGSTOP and SIGTERM signal handlers.
	   SIGINT and SIGHUP are configured to be ignored where as SIGSTOP and SIGTERM
	   are used to close the sockets and terminated the program.
	2. session_handler: This function forms one of the two major functions in this
	   program and provides functionality to read a request from the client and provide
	   corresponding response.
	   Major actions performed by this function are -
		1. Read from the client socket descriptor.
		2. Obtain GET request from the client.
		3. Parse the file requested in the GET request.
		4. Determine content type - image or html
		5. Display error message on the client browser, if the requested file (index.html) 
		   is not found on the webserver using HTML tags.
	3. writeToClient: This function sends the appropriate HTTP response using sendHTTPheader and
		   written functions.
	4. main: This function forms the core of webserver program which forks the process, 
	   establishes the client-server connections and detects signals.
	   Major actions performed by this function are -
		1. Accept port number from the user.
		2. Detect SIGINT, SIGHUP, SIGTERM and SIGQUIT signals.
		3. Create socket for the webserver and store its descriptor.
		4. Obtain IP addresses and port numbers.
		5. Use bind() to bind address to the socket.
		6. Use listen() function to listen to the socket. Note that the backlog for this
		   function is currently set at 10 and can be configured as needed.
		7. Create a socket descriptor for each client.
		8. Use accept() to accept a connection from the client.
		9. Use gethostbyaddr() to obtain client details such as IP address and port number.
		10. Use pthread functions to generate threads for handling multiple client connections.

3. RequestQueue
This program provides a queue for storing request’s, implemented as a linked list
in c. Note that enqueue and dequeue are not protected by mutual exclusion and 
so this must be handled outside of the function, by the caller.

************************************************************************
PERFORMANCE TESTING

The following section compares the thread-pool based implementation of a simple
server to process based, measured using latency and throughput. These test’s
were run on serverTest.html, which is loaded with 20 images (a mixture of 
common ones for comparing request times as well as randomly generated and
duplicates). 

See attached files: ProcessTest_Overview and ThreadedTest_Overview for
	reference.

LATENCY

Threaded server out-performed Process server in each category as well as overall.
Overall time from initial request until finish of the threaded server was 520ms, 
while the process implementation ran 1.1 seconds.

Looking at a few specific request further proves the above statement:
Test Case		Process Time		Threaded Time
people-q-c-640-480-4	   673ms		    284ms
people-q-c-640-480-3       737ms		    169ms
city-q-c-640-480-4         208ms 		    148ms
city-q-c-640-480-5         157ms		    146ms

THROUGHPUT:

Throughput = number of completed requests / time to complete the requests

For proper testing this should be done using a much larger dataset, so
these number may not be representative of the actual server performance.
However, for academic purpose of comparing both implementation’s this
should be sufficient.

Threaded: 13 requests/520ms = .025

Process: 13 requests/1100ms = .01

************************************************************************
OTHER

HOW THE PROJECT WAS DISTRIBUTED AMONGST GROUP:

Both Ken and Vaishnavi worked on their own ‘version’ of this project 
separately until almost completion. At the last stage, utilizing the 
Pair-Programming methodology Vaishnavi's code and logic from webserver-threads.c 
was incorporated with Ken's Queue logic and pseudo-object oriented modularization
technique.Testing and documentation was shared amongst both persons involved.

BUGS
    none known.

AMBIGUITY IN SPEC
    none.

************************************************************************



 
