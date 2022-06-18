#ifndef cache_H_
#define cache_H_
#include "LLIST.h"

struct CACHE{
	int S;
	int D;
	struct mylinkedlist l;
	struct CACHE *next ;
};

struct mycache{
	int size ;
	struct CACHE *head ;
};

struct mycache create_cache();
void add_path(struct mycache *c , struct mylinkedlist l );
void free_cache(struct mycache *c);
struct mylinkedlist is_path_in_cache(const struct mycache c,int S,int D);

void print_cache(const struct mycache c);

#endif