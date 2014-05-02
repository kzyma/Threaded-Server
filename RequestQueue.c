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
 * File RequestQueue.c contains the implementation of the
 * data structure for holding server requests, implemented
 * as a linked list queue. The same data will be used to fill
 * the queue, so an array implementation would have worked fine,
 * however, I did not want the overhead of copying all elements
 * as capacity increased.
 */

#include "RequestQueue.h"

/*
 * Return a new, empty RequestQueue.
 */
RequestQueue init_RequestQueue(){
    RequestQueue newQueue;
    newQueue.front = NULL;
    newQueue.rear = NULL;
    newQueue.size = 0;
    return newQueue;
}

/*
 *Empty requestQueue.
 *Returns 0 on success, 1 otherwise.
 */
int empty_RequestQueue(RequestQueue* queue){
    queue->front = NULL;
    queue->rear = NULL;
    return 0;
}


/*
 *Insert element at rear of Queue.
 *Returns 0 on success, else returns 1.
 */
int enqueue_RequestQueue(RequestQueue* queue,Request data){
    
    /* constuct node object */
    Node* newData;
    newData = (Node *)malloc(sizeof(Node));
    
    //out of memory->malloc failed
    if(newData == NULL){
        return 1;
    }
    
    newData->data = data;
    newData->next = NULL;
    
    //only change front if list is empty
    if(queue->front == NULL){
        queue->front = newData;
    }
    
    if(queue->rear == NULL){
        queue->rear = newData;
    }else{
        queue->rear->next = newData;
        queue->rear = newData;
    }
    
    queue->size++;
    
    return 0;
}

/*
 *Return and return next element in Queue.
 */
Request* dequeue_RequestQueue(RequestQueue* queue){
    
    Request* req = (Request *)malloc(sizeof(Request));
    
    //if queue is empty return -1
    if(queue->front == NULL){
        req->fd = -1;
        return req;
    }
    req->fd = queue->front->data.fd;
    
    queue->front = queue->front->next;
    
    queue->size--;

    return req;
}

/*
 *Print Queue
 */
void print_RequestQueue(RequestQueue* queue){
    Node* temp = (Node *)malloc(sizeof(Node));
    temp = queue->front;
    while(temp != NULL){
        printf("%d \n",temp->data.fd);
        temp = temp->next;
    }
}

/*
 * Free used resources from the request Queue
 */
void destroy_RequestQueue(RequestQueue* queue){
    if (queue->front != NULL){
        free(queue->front);
        queue->front = null;
    }
    if (queue->rear != NULL){
        free(queue->rear);
        queue->rear = null;
    }
}



