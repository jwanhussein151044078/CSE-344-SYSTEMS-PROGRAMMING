
#include <unistd.h>
#include <stdlib.h>  
#include <string.h>
#include <math.h>
#include "utilities.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>





/*
 * function to parsing the input
 * return 0 if success , -1 in case of invalid input .  	
*/
int parsing_commandline(int argc, char *argv[] , int* N ,int* T , int* S , int* L ,int* U, int* G , char** fn){
	int opt;

	while((opt = getopt(argc, argv, ":N:U:T:S:L:F:G:")) != -1){	
        	
    	if(opt == 'N' && optind == 3){
    		sscanf(optarg,"%d",N);
    	}else if (opt == 'T' && optind == 5){
            sscanf(optarg,"%d",T);
        }else if (opt == 'S' && optind == 7){
            sscanf(optarg,"%d",S); 
        }else if (opt == 'L' && optind == 9){
            sscanf(optarg,"%d",L); 
        }else if (opt == 'U' && optind == 11){
            sscanf(optarg,"%d",U); 
        }else if (opt == 'G' && optind == 13){
            sscanf(optarg,"%d",G); 
        }else if (opt == 'F' && optind == 15){
            *fn = optarg ; 
        }else{
     		return -1 ;
    	}   
    }        
    if(optind < 15){
    	return -1;
    }
    
    return 0 ;
}

int check_constraints(int N,int M,int T,int S,int L){
    if(M>N && N>2 && S>3 && M>T && T>=1 && L>=3){
        return 1 ;
    }
    return -1 ;
}

int minimum(int a, int b, int c){
    if(a <= b  && a <= c){
        return a ;
    }
    if(b <= c){
        return b ;
    }
    return c ;
}

int isfilevalide(int fd,int lm3){
    int counterP = 0 ;
    int counterC = 0 ;
    int counterD = 0 ;
    char ch ;
    while(read(fd,&ch,1)== 1){
        if(ch == 'P'){
            counterP++;
        }else if (ch == 'C'){
            counterC++;
        }else if (ch == 'D'){
            counterD++;
        }else{
            printf("unknown kind of meals was found ( %c ) byte number ( %d ) in input file !!!\n",ch,counterD+counterP+counterC+1 );
            return -1 ;
        }
    }
    if(counterD+counterC+counterP < lm3){
        printf("the input file contains less than (3*L*M) bytes, can not continue !!!\n");
        return -1 ;
    }else if(counterD+counterC+counterP > lm3){
        printf("the input file contains more than (3*L*M) bytes, can not continue !!!\n");
        return -1 ;
    }
    if(counterP != counterD || counterP != counterC || counterD != counterC){
        printf("the input file input file should contain equal amont of all plates !!! P:%3d  C:%3d  D:%3d\n",counterP,counterC,counterD);
        return -1 ;
    }
    lseek(fd,0L,SEEK_SET);
    return 1 ;

}