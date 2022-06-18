#include <stdio.h>
#include <stdlib.h>
#include "LLIST.h"

struct mylinkedlist create_LLIST(){
	struct mylinkedlist l ;
	l.size = 0 ;
	l.head = NULL ;
	return l ;
}

struct NODE* add_NODE(struct mylinkedlist *l , int data){
	struct NODE n ;	
	n.data = data ;
	n.next = NULL ;
	if(l->head == NULL){
		l->head = (struct NODE*)malloc(sizeof(struct NODE));
		l->head->data = n.data;
		l->head->next = NULL;
		l->size = 1 ;
		return l->head ;
	}
	struct NODE *temp = l->head ;
	while(temp->next != NULL){
		temp = temp->next;
	}
	temp->next = (struct NODE*)malloc(sizeof(struct NODE));
	temp = temp->next ;
	temp->data = n.data;
	temp->next = NULL;
	l->size++;
	return temp ;
}

void add_NODE_front(struct mylinkedlist *l , int data){
	if(l->head == NULL){
		l->head = (struct NODE*)malloc(sizeof(struct NODE));
		l->head->data = data;
		l->head->next = NULL;
		l->size = 1 ;
		return ;	
	}else{
		struct NODE *n = (struct NODE*)malloc(sizeof(struct NODE));
		n->data = data ;	
		n->next = l->head ;
		l->head = n ;
		l->size++ ;
	}
	return ;
}

int linkedlist_index_of(const struct mylinkedlist l , int data){
	struct NODE *temp = l.head ;
	int ret = 0;
	while(temp){
		if(temp->data == data){
			return ret ; 
		}
		ret++;
		temp = temp->next;
	}
	return -1 ;
}

void free_list(struct mylinkedlist *l){
   struct NODE* temp;
   l->size = 0 ;
   while (l->head != NULL)
    {
       temp = l->head;
       l->head = l->head->next;
       free(temp);
    }

}

void print_list(const struct mylinkedlist l){
	printf("[ ");
	struct NODE *temp = l.head ;
	while(temp){
		printf("%d ",temp->data );
		if(temp->next){
			printf("-> ");
		}
		temp = temp->next;
	}
	printf("]\n");	
}