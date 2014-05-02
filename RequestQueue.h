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
 * File RequestQueue.h contains the data structure for holding
 * server requests.
 */

#ifndef REQUEST_QUEUE_H
#define REQUEST_QUEUE_H

#include <stdio.h>
#include <stdlib.h>


typedef struct request Request;

typedef struct node Node;

typedef struct requestQueue RequestQueue;

struct request{
    int fd;
};

struct node{
    Request data;
    struct node* next;
};

struct requestQueue{
    Node* front;
    Node* rear;
    int size;
};

/*
 * Return a new, empty RequestQueue.
 */
RequestQueue init_RequestQueue();

/*
 *Empty requestQueue.
 *Returns 0 on success, 1 otherwise.
 */
int empty_RequestQueue(RequestQueue* queue);

/*
 *Insert element at rear of Queue.
 *Returns 0 on success, else returns 1.
 */
int enqueue_RequestQueue(RequestQueue* queue,Request data);

/*
 *Return and return next element in Queue.
 */
Request* dequeue_RequestQueue(RequestQueue* queue);
/*
 *Print Queue
 */
void print_RequestQueue(RequestQueue* queue);

/*
 * Free used resources from the request Queue
 */
void destroy_RequestQueue(RequestQueue* queue);

#endif  //end REQUEST_QUEUE_H
 
 
 
 
 
 
 
