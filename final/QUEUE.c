# include <stdio.h>
# include <stdlib.h>
# include "QUEUE.h"

struct myqueue create_QUEUE(){
	struct myqueue q;
	q.size = 0;
	q.queue = create_LLIST();
	q.front = q.queue.head ;
	q.rear  = q.queue.head ;
	return q ;
}

void enqueue(struct myqueue* q , int data){
	
	q->rear = add_NODE(&q->queue , data);
	q->front = q->queue.head;
	q->size = q->queue.size;
}
int dequeue(struct myqueue* q ){
	int ret = q->front->data ;
	struct NODE *node = q->front ;
	q->queue.head = q->queue.head->next ;
	free(node);
	q->front = q->queue.head;

	q->size--;
	q->queue.size--;
	return ret ;
}
int is_empty_queue(const struct myqueue q){
	return(q.size == 0);
}
void print_queue(const struct myqueue q){
	print_list(q.queue);
}
void free_queue(struct myqueue *q){
	free_list(&q->queue);
}