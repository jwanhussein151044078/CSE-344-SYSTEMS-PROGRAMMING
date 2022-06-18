#include <stdio.h>
#include <stdlib.h>
#include "cache.h"


struct mycache create_cache(){
	struct mycache cache ;
	cache.size = 0 ;
	cache.head = NULL;
	return cache ;
}

void add_path(struct mycache *c , struct mylinkedlist l ){
	if(c->size == 0){
		
		c->head = (struct CACHE*)malloc(sizeof(struct CACHE));
		c->head->l = create_LLIST() ;
		c->head->S = l.head->data ;
		struct NODE *node = l.head ;
		while(node != NULL){
			add_NODE(&c->head->l,node->data);
			c->head->D = node->data ;
			node = node->next;
		}
		c->head->next = NULL ;
	}else{
		struct NODE *node = l.head ;
		struct CACHE *temp = c->head;
		while(temp->next != NULL)temp = temp->next ;
		temp->next = (struct CACHE*)malloc(sizeof(struct CACHE));
		temp = temp->next;
		temp->S = l.head->data ;
		while(node != NULL){
			add_NODE(&temp->l,node->data);
			temp->D = node->data ;	
			node = node->next;
		}
		temp->next = NULL;
	}
	c->size++;
}

void free_cache(struct mycache *c){
	struct CACHE* temp ;
	while(c->head != NULL){
		temp = c->head; 
		c->head = c->head->next ;
		free_list(&temp->l);
		free(temp);
	}
}

struct mylinkedlist is_path_in_cache(const struct mycache c,int S,int D){
	struct mylinkedlist l = create_LLIST();
	struct CACHE *cache = c.head;
	while(cache != NULL){
		if(cache->S == S && cache->D == D){
			struct NODE *temp = cache->l.head;
			while(temp != NULL){
				add_NODE(&l,temp->data);
				temp = temp->next ;
			}
		}
		cache = cache->next;
	}
	return l ;
}

void print_cache(const struct mycache c){
	struct CACHE *cache = c.head;
	while(cache != NULL){
		printf("SRC = %d | DEST = %d path :  ",cache->S,cache->D );
		print_list(cache->l);
		printf(" .\n" );
		cache = cache->next;
	}
}