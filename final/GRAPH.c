






#include <stdio.h> 
#include <stdlib.h> 
#include "GRAPH.h"  

struct mygraph create_graph(){
    struct mygraph g ;
    g.V = 0 ;
    g.E = 0 ;
    g.cap = 256 ;
    g.adjlist = (struct mylinkedlist*)malloc(g.cap*sizeof(struct mylinkedlist));
    //g.visited = (int*)calloc(g.cap,sizeof(int));
    return g;
}

void add_edge(struct mygraph *g , int src , int dest){
    int ind ;
    
    if(g->V >= g->cap - 2 ){
        g->cap += 256 ;
        g->adjlist = (struct mylinkedlist*)realloc(g->adjlist,g->cap*sizeof(struct mylinkedlist));
        //g->visited = (int*)realloc(g->visited,g->cap*sizeof(int));  
    }
    if((ind = index_of(*g,src)) != -1 ){
        if(linkedlist_index_of(g->adjlist[ind],dest) == -1){
            add_NODE(&g->adjlist[ind],dest);
        }
        if(index_of(*g,dest) == -1){
            g->adjlist[g->V] = create_LLIST();
            add_NODE(&g->adjlist[g->V],dest);
            g->V++;            
        }
        g->E++;
        return;
    }
    g->adjlist[g->V] = create_LLIST();
    add_NODE(&g->adjlist[g->V],src);
    if(src != dest){
        add_NODE(&g->adjlist[g->V],dest);
    }
    g->V++;
    if(index_of(*g,dest) == -1){
        g->adjlist[g->V] = create_LLIST();
        add_NODE(&g->adjlist[g->V],dest);
        g->V++;            
    }
    g->E++;    
}

int index_of(const struct mygraph g  , int vertex){
    int i ;
    for(i = 0; i < g.V ; i++){
        if(g.adjlist[i].head->data == vertex ){
            return i ;
        }
    }
    return -1 ;
}
 
void print_graph(const struct mygraph g ){
    int i ;
    for(i = 0 ; i < g.V ; i++){
        print_list(g.adjlist[i]);
    }
}

void free_graph(struct mygraph *g){
    int i ;
    for (i = 0; i < g->V; ++i){
        free_list(&g->adjlist[i]);
    }
    //free(g->visited);
    free(g->adjlist);
}

struct mylinkedlist bfs(struct mygraph *g , int src , int dest){
    struct myqueue path = create_QUEUE();
    struct myqueue temp = create_QUEUE();
    struct mylinkedlist ret  = create_LLIST();    
    int *pred = (int*)malloc(sizeof(int)*g->V);
    int *dist = (int*)malloc(sizeof(int)*g->V);
    for(int i = 0 ; i < g->V ; i++){
        pred[i] = -1;
        dist[i] = -1;
    }
    if(!is_connected(g,src,dest,dist,pred)){
        free(pred);
        free(dist);
        return ret ;
    }
    int trace = index_of(*g,dest);
    enqueue(&temp,trace);
    while(pred[trace] != -1){
        enqueue(&temp,pred[trace]);
        trace = pred[trace];
    }
    while(!is_empty_queue(temp)){
        int data = g->adjlist[dequeue(&temp)].head->data;
        enqueue(&path,data);
    }
    free(pred);
    free(dist);
    //free_queue(&temp);
    /////////////////////////////////////////////////////

    while(!is_empty_queue(path)){
        add_NODE_front(&ret,dequeue(&path));
        //dequeue(&path);
    }
 
    return ret ;

    
}

int is_connected(struct mygraph *g , int src , int dest , int* dist , int* pred){
    struct myqueue q = create_QUEUE();
    int index_of_src = index_of(*g,src);
    int *visited = (int*)malloc(sizeof(int)*g->V);
    if(index_of_src == -1){
        return 0;
    }
    for(int i = 0 ; i < g->V ; i++){
        visited[i] = 0 ;
    }
    enqueue(&q,index_of_src);
    dist[index_of_src] = 0 ;
    visited[index_of_src] = 1 ;
    while(!is_empty_queue(q)){
        int u = dequeue(&q);
        struct NODE *temp = g->adjlist[u].head;
        while(temp){
            if(visited[index_of(*g,temp->data)] == 0){
                visited[index_of(*g,temp->data)] = 1;
                dist[index_of(*g,temp->data)] = dist[u]+1;
                pred[index_of(*g,temp->data)] = u ;
                enqueue(&q,index_of(*g,temp->data));
                if(dest == temp->data){
                    free_queue(&q);
                    free(visited);
                    return 1 ;
                }
            }
            temp = temp->next ;
        }
    }
    free(visited);
    free_queue(&q);    
    return 0 ;
}