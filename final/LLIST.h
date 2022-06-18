#ifndef LLIST_H_
#define LLIST_H_
struct NODE{
	int data ;
	struct NODE *next ;
};

struct mylinkedlist{
	int size ;
	struct NODE *head;
};

struct mylinkedlist create_LLIST();
struct NODE* add_NODE(struct mylinkedlist *l , int data);
void print_list(const struct mylinkedlist l);
void free_list(struct mylinkedlist *l);
int linkedlist_index_of(const struct mylinkedlist l , int data);
void add_NODE_front(struct mylinkedlist *l , int data);
#endif