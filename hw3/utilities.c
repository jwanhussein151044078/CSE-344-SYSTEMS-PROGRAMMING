
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
int parsing_commandline(int argc, char *argv[] ,char **input1FN ,char **input2FN,char** n){
	int opt;
	while((opt = getopt(argc, argv, ":i:j:n:")) != -1){	
    	
    	if(opt == 'i' && optind == 3){
    		*input1FN = optarg ;
    	}else if (opt == 'j' && optind == 5){
    		*input2FN = optarg ; 
        }else if (opt == 'n' && optind == 7){
            *n = optarg ; 
        }else{
     		return -1 ;
    	}   
    }        
    if(optind < 7){
    	return -1;
    }
    return 0 ;
}

int stoi(char* str){
    int result = 0;
    int temp = 10 ;
    int i = 0 ;
    while(str[i]){
        if( str[i] <= '9' && str[i] >= '0'){
            result *= temp ;
            result += str[i]-48 ;   
        }
        else{
            return -1;
        }
        i++;
    }
    return result ;
}

char* get_data_from_file(char* fn , int n){
    int fd ;
    int size = pow(2,2*n);
    
    char* data = (char*)malloc(sizeof(char)*size); 
    if((fd= open(fn,O_RDONLY))==-1){
        perror("input file open ERROR :");
        return NULL;
    }
    if( read(fd,data,size)!= size){
        perror("could not read (2^2n)bytes from input file:");
        free(data);
        return NULL;    
    }
    if(close(fd)==-1){
        perror("input file close ERROR :");
        free(data);
        return NULL;    
    }
    return data ;
}

int** convert_data_to_arr(char* data , int n){
    int **mx ;
    int r = pow(2,n);
    int i = 0 , j = 0 , c = 0 ;
    
    mx = (int**)malloc(sizeof(int*)*r);
    if (mx == NULL){
        printf("malloc ERROR !!\n");
        return NULL ;
    }
    for(i = 0 ; i < r ; i++){
        mx[i] = (int*)malloc(sizeof(int)*r);
        if(mx[i] == NULL){
            int q = 0;
            printf("malloc ERROR !!\n");
            while(q<i){
                free(mx[q]);
            }
            free(mx);
            return NULL ;
        }
    }
    i = 0 ;
    while(data[i]){
        mx[j][c] = (int)data[i];
        c++;
        if(c == r){
            c = 0 ;
            j++;
        }

        i++;
    }
    return mx ;
}

void print_arr(int** a , int s){
    int i , j ;
    int l = pow(2,s);
    for (i = 0; i < l; ++i)
    {
        for (j = 0; j < l; ++j)
        {
            printf("%3d  ",a[i][j] );
        }
        printf("\n");
    }
}

void _free(int **a ,int s){
    int r = pow(2,s);
    for (int i = 0; i < r; ++i){
        if (a[i] != NULL){
            free(a[i]);
        }
    }
    free(a);
}

char* data_processor(int** A,int** B,int n,int quarter , int *data_len){
    int size = pow(2,2*n)*8 ;
    int r    = pow(2,n);
    int counter  ;
    int is , js , ie , je;
    char* out = (char*)malloc(sizeof(char)*size);
    if(out == NULL){
        printf("malloc ERROR !!\n");
        return NULL ;
    }  
    if(quarter == 1 || quarter == 2){
        is = 0 ;
        js = 0 ;
        ie = r/2 ;
        je = r ;
    }
    else if (quarter == 3 || quarter == 4){
        is = r/2 ;
        js = 0 ;
        ie = r ;
        je = r ;
    }
      
    char temp[4];
    out[0] = '\0';
    counter = sprintf(temp,"%d",quarter);
    strcat(out,temp);
    strcat(out,"  ");
    counter += 2 ;
    int bytewriten ; 
    int j ;
    for( is ; is < ie ; is++){
        for( j = js ; j < je ; j++){
            bytewriten = sprintf(temp,"%d",A[is][j]);
            strcat(out,temp);
            strcat(out," ");
            counter += bytewriten+1;
            if((size - counter) < 10 ){
                size += 100 ;
                out = (char*)realloc(out,sizeof(char)*size);
                if(out == NULL){
                    return NULL ;
                }
            }
        }
    }
    if(quarter == 1 || quarter == 3){
        is = 0 ;
        js = 0 ;
        ie = r/2 ;
        je = r ;
    }
    else if (quarter == 2 || quarter == 4){
        is = r/2 ;
        js = 0 ;
        ie = r ;
        je = r ;
    }
    for( is ; is < ie ; is++){
        for( j = js ; j < je ; j++){
            bytewriten = sprintf(temp,"%d",B[j][is]);
            strcat(out,temp);
            strcat(out," ");
            counter += bytewriten+1;
            if((size - counter) < 10 ){
                size += 100 ;
                out = (char*)realloc(out,sizeof(char)*size);
                if(out == NULL){
                    return NULL ;
                }
            }
        }
    }
    *data_len = counter ;
    strcat(out,".");
    return out;
}

char* get_data(int fd, int *length){
    char* data ;
    char ch[1] ;
    int l = 256 ;
    int counter = 0 ;

    data = (char*)malloc(sizeof(char)*l);
    if(data == NULL){
        perror("malloc ERROR :");
        return NULL;
    }
    data[0] = '\0';
    while(read(fd,ch ,1) != 0 ){
        if(ch[0] == '.'){
            break;
        }
        strcat(data , ch );
        counter += 1;
        if((counter - l) < 5){
            l += 256 ;
            data = (char*)realloc(data,sizeof(char)*l);
            if(data == NULL ){
                perror("realloc ERROR :");
                return NULL;
            }
        }
    }
    *length = counter ;
    return data ;
}

int** fill_matrix(char* q1,char* q2,char* q3,char* q4,int n){
    int** out;
    int r = pow(2,n);
    int temp ;
    char* token ;
    int i = 0 , j = 0 ;
    out = (int**)malloc(sizeof(int*)*r);
    if(out == NULL){
        perror("malloc ERROR : ");
        return NULL ;
    }
    for(int w = 0 ; w<r ; w++ ){
        out[w] = (int*)malloc(sizeof(int)*r);
        if(out[w] == NULL){
            int q = 0;
            printf("malloc ERROR !!\n");
            while(q<i){
                free(out[q]);
            }
            free(out);
            return NULL ;
        }
    }

    token = strtok(&q1[3]," ");
    while(token != NULL){
        
        sscanf(token , "%d" , &temp );
        out[i][j] = temp ;
        j++;
        if(j == r/2 ){
            j = 0 ;
            i++;
        }

        token = strtok(NULL , " ");
    }
    i = 0 ;
    j = r/2 ;
    token = strtok(&q2[3]," ");
    while(token != NULL){
        
        sscanf(token , "%d" , &temp );
        out[i][j] = temp ;
        j++;
        if(j == r ){
            j = r/2 ;
            i++;
        }

        token = strtok(NULL , " ");
    }
    i = r/2 ;
    j = 0 ;
    token = strtok(&q3[3]," ");
    while(token != NULL){
        
        sscanf(token , "%d" , &temp );
        out[i][j] = temp ;
        j++;
        if(j == r/2 ){
            j = 0 ;
            i++;
        }

        token = strtok(NULL , " ");
    }
    i = r/2 ;
    j = r/2 ;
    token = strtok(&q4[3]," ");
    while(token != NULL){
        
        sscanf(token , "%d" , &temp );
        out[i][j] = temp ;
        j++;
        if(j == r ){
            j = r/2 ;
            i++;
        }

        token = strtok(NULL , " ");
    }

    return out ;
}

float** convert_to_float(int **C,int n){
    int r = pow(2,n);
    float** out ;
    out = (float**)malloc(sizeof(float*)*r);
    if(out == NULL){
        perror("malloc ERROR :");
        return NULL;
    }
    for (int i = 0; i < r; ++i){
        out[i] = (float*)malloc(sizeof(float)*r);
        if(out[i] == NULL){
            int q = 0;
            printf("malloc ERROR !!\n");
            while(q<i){
                free(out[q]);
            }
            free(out);
            return NULL ;
        }
    }

    for(int i = 0 ; i < r ; i++){
        for(int j = 0 ; j < r ; j++){
            out[i][j] = (float)C[i][j];
        }
    }


    return out ;
}

int getpow(int n){
    return pow(2,n);
}

