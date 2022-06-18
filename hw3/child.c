#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <math.h>
#include <string.h>
#include "child.h"
#include "utilities.h"


void child(int quarter , int p1[2] , int p2[2] , int n ){
	char ch[1];
	char qu[5];
	char temp[10];
	char *data ;
	int r = pow(2,n);
	int **in , **out ;
	int readsize = 500 ;
	char *read_line; 
	int counter = 0 ;
	int data_size ;

	if(close(p1[1]) == -1 || close(p2[0])== -1 ){
		perror("pipe close ERROR :");
		kill(getppid(),SIGINT);
		exit(0);
	}
	read_line = (char*)malloc(sizeof(char)*readsize);
	if(read_line==NULL){
		perror("malloc ERROR :");
		kill(getppid(),SIGINT);
		exit(0);	
	}
	read_line[0] = '\0';
	read(p1[0],qu,3);
//	printf("reading , %d\n",quarter);
	while(read(p1[0],ch,1) != 0 ){
		if(ch[0] == '.'){
			break;
		}
		else{
			strncat(read_line ,ch,1);
			counter++;
			if ((counter - readsize) < 10){
				readsize += 100 ;
				read_line = (char*)realloc(read_line,sizeof(char)*readsize);
				if(read_line == NULL){
					kill(getppid(),SIGINT);
					exit(0);				
				}

			}
		}

	}
	if(close(p1[0]) == -1){
		perror("pipe close ERROR :");
		kill(getppid(),SIGINT);
		exit(0);
	}

	in = process_line(read_line , counter , r);
	if(in == NULL){
		kill(getppid(),SIGINT);
		exit(0);
	}
	out = product(in , r);
	if(out == NULL){
		_free(in,n);
		kill(getppid(),SIGINT);
		exit(0);
	}

	data = preper_to_send(out,r/2,&data_size,qu);
	if(data == NULL){
		_free(in,n);
		_free(out,n-1);
		kill(getppid(),SIGINT);
		exit(0);	
	}
	if(write(p2[1],data,data_size) != data_size){
		perror("write ERROR :");
		_free(in,n);
		_free(out,n-1);
		free(data);
		kill(getppid(),SIGINT);
		exit(0);	
	}
	
	if(close(p2[1])== -1 ){
		perror("pipe close ERROR :");
		_free(in,n);
		_free(out,n-1);
		free(data);
		kill(getppid(),SIGINT);
		exit(0);
	}
	_free(in,n);
	_free(out,n-1);
	free(data);
		
	
}


int** init_arr(int** in , int size){
	in = (int**)malloc(sizeof(int*)*size);
    if (in == NULL){
        printf("malloc ERROR !!\n");
        return NULL ;
    }
    for(int i = 0 ; i < size ; i++){
        in[i] = (int*)malloc(sizeof(int)*size);
        if(in[i] == NULL){
            int q = 0;
            printf("malloc ERROR !!\n");
            while(q<i){
                free(in[q]);
            }
            free(in);
            return NULL ;
        }
    }
    return in ;
}

int** process_line(char* read_line ,int length ,int r){
	int **res;
	char temp[5];
	int w ;
	if((res = init_arr(res , r))==NULL){
		free(read_line);
		kill(getppid(),SIGINT);
		exit(0);	
	}
	int i = 0,j = 0,k = 0,z = 0 ;
	
	while(read_line[i]){
		if(read_line[i] >= '0' && read_line[i] <= '9'){
			temp[j++] = read_line[i] ;
			temp[j] = '\0';
		}
		else{
			j = 0 ;
			sscanf(temp , "%d" , &w);
			res[k][z] = w ;
			z++;
			if(z == r){
				k++;
				z = 0 ;
			}
		}
		i++;
	}
	return res;
}

int** product(int** arr ,int r){
	int **out ;
	if((out = init_arr(out, r/2))==NULL){
		perror("malloc ERROR :");
		return NULL ;
	}
	for(int i = 0 ; i < r/2 ; i++){
		for(int j = 0 ; j < r/2 ; j++){
			out[i][j] = 0 ;
		}
	}
	int temp = r/2;
	for (int i = 0; i < temp ; ++i){
		for (int j = temp ; j < r ; ++j){
			for (int k = 0; k < r ; ++k){
				out[i][j-temp] += arr[i][k]*arr[j][k];
			}
		}
	}
	return out;
}

char* preper_to_send(int **arr,int size,int *length ,char quarter[]){
	char *out ;
	int l = 256 ;
	int counter = 3 ;
	int bytwritin ;
	char temp[10];
	out = (char*)malloc(sizeof(char)*l);
	if(out == NULL){
		perror("malloc ERROR :");
		return NULL ;
	}
	strcat(out , quarter);
	for (int i = 0; i < size; ++i){
		for (int j = 0; j < size; ++j){
			bytwritin = sprintf(temp,"%d",arr[i][j]);
			strcat(out , temp);
			strcat(out , " ");
			counter += bytwritin +1;
			if((l - counter) < 5){
				l += 256 ;
				out = (char*)realloc(out,sizeof(char)*l);
				if(out == NULL){
					perror("realloc ERROR :");
					return NULL ;
				}
			} 
		}
	}
	strcat(out , ".");
	counter++;
	*length = counter ;	
	return out ;
}
