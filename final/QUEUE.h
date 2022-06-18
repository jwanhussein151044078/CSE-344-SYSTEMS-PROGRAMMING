#ifndef QUEUE_H_
#define QUEUE_H_
#include "LLIST.h" 

struct myqueue{
	int size ;
	struct mylinkedlist queue;
	struct NODE *front ;
	struct NODE *rear ;
};

struct myqueue create_QUEUE();
void enqueue(struct myqueue* q , int data);
int dequeue(struct myqueue* q );
int is_empty_queue(const struct myqueue q);
void print_queue(const struct myqueue q);
void free_queue(struct myqueue *q);
#endif