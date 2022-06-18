#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "utilities.h"
#include <stdlib.h>
#include <string.h>

/*
 * function to parsing the input
 * return 0 if success , -1 in case of invalid input .  	
*/
int parsing_commandline(int argc, char *argv[] ,char **input_FN ,char **LOG_FN ,int *PORT , int *S , int *X){
	int opt;
	while((opt = getopt(argc, argv, ":i:p:o:s:x:")) != -1){	
    	
    	if(opt == 'i' && optind == 3){
    		*input_FN = optarg ;
    	}else if (opt == 'p' && optind == 5){
    		sscanf(optarg , "%d" , PORT); 
    	}else if (opt == 'o' && optind == 7){
    		*LOG_FN = optarg ; 
    	}else if (opt == 's' && optind == 9){
    		sscanf(optarg , "%d" , S); 
    	}else if (opt == 'x' && optind == 11){
    		sscanf(optarg , "%d" , X); 
    	}else{
     		return -1 ;
    	}   
    }      
    if(optind < 11){
    	return -1;
    }
    return 0 ;
}

struct mygraph load_graph(char* file_name){
    struct mygraph g = create_graph();
    int fd,r,src,dest ;
    char buff[256];
    char ch ;
    int counter = 0 ;
    fd = open(file_name, O_RDONLY);
    while(1){
        r = read(fd,&ch,1);
        if(r == -1){
           exit(0); 
        }
        if(r == 0){
            if(counter >0){
                sscanf(buff , "%d\t%d" ,&src,&dest);
                //printf("%d\t%d\n",src,dest );
                add_edge(&g,src,dest);
            }
            break ;
        }
        if(ch == '#'){
            do{
               r = read(fd,&ch,1); 
            }while(ch != '\n' && r > 0);
        }else if((ch >= '0' && ch <= '9' )|| ch == '\t'){
            buff[counter++] = ch ;
            buff[counter] = '\0' ;
        }else if(ch == '\n'){
            if(counter > 0){
                //printf(">>>%s\n",buff );
                sscanf(buff , "%d\t%d" ,&src,&dest);
                //printf("%d\t%d\n",src,dest );
                add_edge(&g,src,dest);
            }
            counter = 0;
            buff[0] = '\0';
        }
        
    }
    close(fd);
    return g;
}


char* convert_to_string(struct mylinkedlist l){
    char temp[10];
    char* ret = (char*)malloc(sizeof(char)*512);
    ret[0]= '\0';

    struct NODE *n = l.head;
    while(n !=NULL){
        temp[0] = '\0';
        sprintf(temp,"%d",n->data);
        strcat(ret,temp);
        if(n->next != NULL){
            strcat(ret," -> ");
        }
        n = n->next;
    }
    return ret ;
}
