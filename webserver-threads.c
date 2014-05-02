
/***********************************************************
 *
 * Author: Vaishnavi Talegaonkar
 *         Ken Zyma
 * Term: Spring 2014
 * Course: CSC552
 * Assignment: A Web Server: Threads
 *
 ***********************************************************/

/*
 * File webserver-threads.c contains the source for a thread-pool web
 * server implemented in c.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>	
#include <sys/stat.h>
#include <semaphore.h>
#include <pthread.h>

/* Including the header file for sockutils program */
#include "sockutils.h"
#include "RequestQueue.h"
//#include "HTTPheader.c"

/* Requirement Assumption - A pool of 10 threads is implemented for this program */
#define MAX_THREADS 10

typedef enum { false, true } bool;

//structure for a request sent from server to thread
static struct thread_req{
	int th_soc;
	struct thread_req * next;
} req;

/* Server Socket's descriptor */
static int srvr_soc = 0;
/* PID for server (parent) */
static pid_t srvr_pid = 0;
static int srvr_port;

/* Initializing thread pool */
static pthread_t thread_pool[MAX_THREADS];
/* Initializing MUTEX - Mutual Exclusion  */
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond;
static RequestQueue requestQ;

void signal_handler(int signo);
void session_handler(Request* req);
static void * thread_function(void * param);
void writeToClient(int* client_soc,char * filename,FILE * fl);


int main(const int argc, char * const argv[]){
    
    //check usage
    if(argc == 2) {
        //get port number from argv[1].
        sscanf(argv[1],"%d",&srvr_port);
    }else{
        printf("usage: %s [port number]\n",argv[0]);
        return 1;
    }
    
    requestQ = init_RequestQueue();

	printf("Starting the Web Server...\n");
	/* Store Server PID */
	srvr_pid = getpid();
    
	/* Call Signal Handler function when a Signal is received */
	struct sigaction act, act_old;
	act.sa_handler = signal_handler;
	act.sa_flags = SA_RESTART | SA_NOCLDSTOP;
	sigemptyset(&act.sa_mask);
	if(	(sigaction(SIGINT,  &act, &act_old) == -1)	||
       (sigaction(SIGQUIT, &act, &act_old) == -1) 	||
       (sigaction(SIGTERM, &act, &act_old) == -1)  ||
       (sigaction(SIGHUP, &act, &act_old) == -1) ){
		perror("signal");
		exit(EXIT_FAILURE);
	}
    
    //socket()
    if((srvr_soc = socket(AF_INET,SOCK_STREAM,0))==-1){
        //error from 'socket'
        perror("Failure to create socket.");
        return EXIT_FAILURE;
    }
    
	printf("Web Server socket created at fd: %i\n", srvr_soc);
    
    //bind()
	struct sockaddr_in server;
	memset(&server, 0, sizeof(struct sockaddr_in));
	server.sin_family		= AF_INET;
	server.sin_addr.s_addr	= htonl(INADDR_ANY);
	server.sin_port			= htons(srvr_port);
    
    if (bind(srvr_soc, (struct sockaddr*)&server,sizeof(struct sockaddr_in))==-1){
        //error from 'bind'
        perror("Failure to bind socket.");
        close(srvr_soc);
        return EXIT_FAILURE;
    }
	printf("Bind to the Server socket successful\n");
    
    //listen()- w/ max connect queue set to 5.
    if(listen(srvr_soc, 10)==-1){
        //error from 'listen'
        fprintf (stderr,"Failure to mark file descriptor %d as passive socket. %s\n",
                 srvr_soc, strerror(errno));
        close(srvr_soc);
		return EXIT_FAILURE;
    }
	printf("Listening on Port: %i\n", srvr_port);
    
	struct sockaddr_in	client;
	struct hostent*		client_details;
	int 				client_len = sizeof(struct sockaddr_in);
    
	//Thread pool init
	pthread_cond_init(&cond, NULL);
	int i;
	int ids[MAX_THREADS];
	for(i=0; i < MAX_THREADS; i++){
		int ret;
		ids[i] = i;
		if( 0 != (ret = pthread_create(&thread_pool[i], NULL,
                                       thread_function, &ids[i])) ) {
			fprintf(stderr, "Error: Failed to create thread! Ret = %d\n", ret);
			return 1;
		}
	}
	printf("Thread Pool initialized. Ready to process Client requests.\n");
	printf("Waiting for Client connections...\n");
	int rc = 0;
    
	while(1) {
		/* socket descriptor for each connected client*/
		int client_sckt;
		/* Using accept() to accept a connection from a client */
		if( (client_sckt = accept(srvr_soc, (struct sockaddr *) &client,
                                  (socklen_t *) &client_len)) == -1 ){
			if(errno == EINTR)
				continue;
            
			perror("accept");
			close(srvr_soc);
			rc = EXIT_FAILURE;
			break;
		}
        
        //create a thread request
		struct thread_req * r=(struct thread_req*)malloc(sizeof(struct thread_req));
		if(r == NULL){
			perror("malloc");
			break;
		}
        
		//Print client details for current request
		if( (client_details =
             gethostbyaddr((void*)&client.sin_addr.s_addr,4,AF_INET)) == NULL){
			perror("gethostbyaddr err:");
		}else{
            printf("%s connected on Port %d\n",
                   client_details->h_name, ntohs(client.sin_port));
        }
        
        
        //enqueue request to RequestQueue
        Request struct_request;
        struct_request.fd = client_sckt;
        
        //lock mutex to ensure exclusive access
        pthread_mutex_lock(&mutex);
        //add conn file descriptor to queue
        enqueue_RequestQueue(&requestQ,struct_request);
        //signal there is a new request to handle
        pthread_cond_signal(&cond);
        //unlock mutex
        pthread_mutex_unlock(&mutex);
        
	}
	close(srvr_soc);
    
    //cleanup after server ends
	for(i=0; i < MAX_THREADS; i++){
		pthread_mutex_lock(&mutex);
		/* Save Socket */
		req.th_soc = -1;
		pthread_mutex_unlock(&mutex);
	}
    
    destroy_RequestQueue(&requestQ);
    
	for(i=0; i < MAX_THREADS; i++){
		pthread_join(thread_pool[i], NULL);
	}
	return rc;
}

/*
 * Handles SIGINT,SIGUP,SIGTERM,and SIGQUIT.
 * SIGINT/SIGUP: ignore
 * SIGTERM/SIGQUIT: gracefully exit program, closing any open
 *                  file desciptors and free all memory allocated
 *                  on the heap.
 */
void signal_handler(int signo){
	switch(signo){
		
		/* SIGINT - Interrupt signal received from Keyboard */
		case SIGINT:
		/* SIGHUP - Hangup detected on terminal or the process is dead */
		case SIGHUP:
            printf("\nSIG Detected.\n");
            fprintf(stderr, "Ignoring...\n");
            break;
		
		/* SIGTERM - Termination signal */
		case SIGTERM:
		/* SIGQUIT - Quit signal received from Keyboard */
		case SIGQUIT:
		/* Close the Server Socket if SIGQUIT is detected */
			printf("\nSIG Detected.\n");
			if((srvr_pid == getpid()) && (srvr_soc != 0)){	
				printf("Closing Server socket...\n");
				close(srvr_soc);	
			}
			exit(EXIT_FAILURE);
			break;
	}
}

/*
 * Function handles requests as they are added to a 
 * RequestQueue.
 */
static void * thread_function(void * param){
    
	int thread_id = *((unsigned int*)param);
    Request* req;

	while(true){
        
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&cond, &mutex);
        req = dequeue_RequestQueue(&requestQ);
        pthread_mutex_unlock(&mutex);
        
		/* Invoke session_handler if we have a request in the thread */
		if(req->fd){
			printf("Thread %i processing request on Socket %i\n",
                   thread_id, req->fd);
			session_handler(req);
            free(req);
		}
	}
	pthread_exit((void *) 0);
}


/*
 * Handles opening request, parsing head/body and sending
 * appropriate response.
 */
void session_handler(Request* req){

    int client_soc = req->fd;
	/* Parsing request */
	char buf[4096]={0};
	unsigned int buf_int=0;
    
	/* Calling function readn to read the client socket */
	int reqst = readn(client_soc, buf, 4096);
	
    if(reqst == -1){
        perror("Request Err: Failed to read from socket.");
        close(client_soc);
        return;
    }else{
        buf_int += reqst;
    }
    
	buf[buf_int] = '\0';
    
    //returns pointer to first instance of "GET"
	char * GET = strstr(buf, "GET");
	if(GET){
		char * filename = &GET[5];
		char * filename_end = strchr(filename, ' ');
		filename_end[0] = '\0';
		printf("Requesting file '%s'\n", filename);

		/* Check file */
		FILE * fl = fopen(filename, "r");
		if(fl){
            writeToClient(&client_soc,filename,fl);
			fclose(fl);
		}else{
			fprintf(stderr, "Failed to open file '%s'\n", filename);
			/* Display ERROR message on the browser */
			char* no_file = "<html>\n<body>\n404 File Not Found."
                    "\n</body>\n</html>\n";
			/* Send the Header and Body of the ERROR message to the browser*/
			sendHTTPheader(client_soc, 4);
            printf("Sending the following body to the browser:\n");
            printf("%s\n",no_file);
            printf("^end of message^\n");
			writen(client_soc, no_file, strlen(no_file));
		}
	}else{
		fprintf(stderr, "Invalid GET request\n");
		/* Display ERROR message on the browser */
		char* no_get = "<html>\n<body>\n Invalid Request. Must be GET."
                    "\n</body>\n</html>\n";
		/* Send the Header and Body of the ERROR message to the browser*/
		sendHTTPheader(client_soc, 0);
        printf("Sending the following body to the browser:\n");
        printf("%s\n",no_get);
        printf("^end of message^\n");
		writen(client_soc,no_get, strlen(no_get));
	}
	close(client_soc);
	fflush(stdout);
}

/*
 * Write specified header/body to socket client_soc.
 */
void writeToClient(int* client_soc,char * filename,FILE * fl){
    struct stat fstat;
    stat(filename, &fstat);
	char buf[4096]={0};
    
    /* Determine file extension after '.' */
    char * ext = strchr(filename, '.');
    ext++;
    
    /* Send Response Header using sendHTTPheader function */
    int i_type = 0;
    if(ext == NULL){
        i_type = 0;
    }else if((strncmp(ext, "htm", 3) == 0) || (strncmp(ext, "html", 4) == 0)){
        i_type = 1;
    }else if((strncmp(ext, "jpg", 3) == 0) || (strncmp(ext, "jpeg", 4) == 0)){
        i_type = 2;
    }else{
        i_type = 3;
    }
    sendHTTPheader(*client_soc,i_type);
    
    /* Send Data */
    printf("Sending data of size: %lld\n", fstat.st_size);
    do{
        /* Read data from the file */
        int reqst = fread(buf, 1, 4096, fl);
        if(reqst > 0)
        /* Write data to the Client Socket using writen function */
            writen(*client_soc, buf, reqst);
    }while(feof(fl) == 0);
    
}



