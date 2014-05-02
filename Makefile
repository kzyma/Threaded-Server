############################################
#
# Author: Vaishnavi Talegaonkar & Ken Zyma
# Term: Spring 2014
# Course: CSC552
# Assignment: Sockets Programming Project: A Web Server
#
###########################################

all: webserver-threads

webserver-threads: webserver-threads.c sockutils.o HTTPheader.o RequestQueue.o
	gcc -o webserver-threads webserver-threads.c sockutils.o HTTPheader.o \
	RequestQueue.o -lnsl -lsocket

HTTPheader.o: sockutils.o
	gcc -c HTTPheader.c sockutils.o

sockutils.o:
	gcc -c sockutils.c

RequestQueue.o:
	gcc -c RequestQueue.c

clean:
	rm -f .o
