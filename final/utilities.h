
#ifndef utilities_H_
#define utilities_H_
#include "GRAPH.h"




int parsing_commandline(int argc, char *argv[] ,char **input_FN ,char **LOG_FN ,int *PORT , int *S , int *X);
struct mygraph load_graph(char* file_name);
char* convert_to_string(struct mylinkedlist l);
#endif /* utilities_H_ */
