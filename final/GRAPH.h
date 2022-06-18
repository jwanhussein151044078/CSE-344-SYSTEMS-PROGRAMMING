#ifndef GRAPH_H_
#define GRAPH_H_
# include "LLIST.h"
# include "QUEUE.h"

struct mygraph{
	int V;
	int E;
	int cap;
	struct mylinkedlist* adjlist ;
	int *visited ;	
};

struct mygraph create_graph();
void add_edge(struct mygraph *g , int src , int dest);
int index_of(const struct mygraph g  , int vertex);
void print_graph(const struct mygraph g );
void free_graph(struct mygraph *g);
struct mylinkedlist bfs(struct mygraph *g , int src , int dest);
int is_connected(struct mygraph *g , int src , int dest , int* dist , int* pred);
#endif /* GRAPH_H_ */
