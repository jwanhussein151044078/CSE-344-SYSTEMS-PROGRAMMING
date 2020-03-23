#include <unistd.h>  
#include <errno.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "utilities.h"


#include <stdio.h>

void convert_to_complex(char _32BYTE[33] ,char _16complex[1][160] ,int* _16complex_size , int _time);
char* int_to_string(int input , int* output_size , int _time);
int write_to_file(const char* outfilename , const char src[160] ,int size_src, int _time);

int main(int argc, char *argv[]){
	int file_dis ;
	int sleep_time ;
    char* input_file_name ;
    char* output_file_name ;

    char _32byte[33];
    int complex_size ;
    char _16COMPLEX[1][160];

    if( parsing_commandline(argc,argv,&input_file_name,&output_file_name,&sleep_time) != 0){
    	write(1,usage_information(),94);	
    	return 2 ; // in case of wrong input 
    }
    if( (file_dis = open(input_file_name,O_RDONLY,S_IRUSR)) == -1){
    	perror("error :");exit(1);
    }

    while(read(file_dis,_32byte,32) == 32){
    	_32byte[32] = '\0' ;
    	convert_to_complex(_32byte , _16COMPLEX , &complex_size , sleep_time);
    	write_to_file(output_file_name,_16COMPLEX[0],complex_size,sleep_time);
    	usleep(sleep_time*1000);
    }


    if (close(file_dis) == -1 ){
    	perror("error :");exit(1);	
    }


	return 0 ;   

}

void convert_to_complex(char _32BYTE[33] ,char _16complex[1][160] ,int* _16complex_size , int _time){
	int i = 0 , j = 0 ;
	int firstbyte , secondbyte;
	char* temp ;
	int temp_size ;
	int pos_complex = 0 ;
	while(i<32){
		j = 0 ;
		firstbyte = (int)_32BYTE[i] ;
		secondbyte = (int)_32BYTE[i+1] ;
		temp = int_to_string(firstbyte,&temp_size,_time);
		while(j<temp_size){
			_16complex[0][pos_complex++] = temp[j++];
		}
		free(temp);
		
		_16complex[0][pos_complex++] = ' ' ;
		_16complex[0][pos_complex++] = '+' ;
		_16complex[0][pos_complex++] = 'i' ;
		j = 0 ;
		temp = int_to_string(secondbyte,&temp_size,_time);
		while(j<temp_size){
			_16complex[0][pos_complex++] = temp[j++];
		}
		free(temp);
		if (i < 30)
			_16complex[0][pos_complex++] = ',' ;

		i += 2 ;
	}
	_16complex[0][pos_complex++] = '\0' ;
	*_16complex_size = pos_complex ;
}

/**
 * function to convert ineger to string
 * return a string  
 * return null if input is less than zero 
 * input must be at max (3 digit)
*/

char* int_to_string(int input , int* output_size , int _time){
	
	char* temp ;
	int count = 0 , i = 0  ;
	if (input < 0){return NULL;}
	while (( temp = malloc(sizeof(char)*5)) == NULL ){usleep(_time*1000);}		
	if(input/100 != 0){
		temp[count++] = (char)(input/100)+48;

	}
	if((input/10)%10 != 0 || count > 0){
		temp[count++] = (char)((input/10)%10)+48;
	}
	temp[count++] = (char)(input%10)+48 ;
	temp[count] = '\0';
	*output_size = count ; 
	return temp;
	
}

/*
 * writing to the file 
*/
int write_to_file(const char* outfilename , const char* src , int size_src , int _time){
	int file_dis ;
	char* ch = malloc(sizeof(char));
	struct flock lock ;
	memset(&lock,0,sizeof(lock));	
	if((file_dis = open(outfilename, O_RDWR | O_CREAT , S_IRUSR | S_IWUSR )) == -1){
		perror("ERROR :");exit(1);
	}
	/* locking the file */
	lock.l_type =  F_WRLCK ;
	fcntl(file_dis,F_SETLKW,&lock);
	/*  *************  */
	lseek(file_dis,0L,SEEK_SET);
	if( read(file_dis,ch,1) != 1){
		if( write(file_dis,src,size_src-1) != size_src-1 ){
			perror("error :");exit(1);
		}
		if(write(file_dis,"\n",1) != 1){
			perror("error :");exit(1);
		}
		
	}else{
		if(read(file_dis,ch,1) != 1){
			lseek(file_dis,0L,SEEK_SET);
			if( write(file_dis,src,size_src-1) != size_src-1 ){
				perror("error :");exit(1);
			}
			if(write(file_dis,"\n",1) != 1){
				perror("error :");exit(1);
			}	
		}
		while(read(file_dis,ch,1) == 1){
			if(ch[0] == '\n'){
				if (read(file_dis,ch,1) == 1){
					if (ch[0] == '\n'){
						int start = lseek(file_dis,-1,SEEK_CUR);
						int end = lseek(file_dis,0,SEEK_END);
						char* temp ;
						while (( temp = malloc(sizeof(char)*(end-start))) == NULL ){usleep(_time*1000);}						
						start = lseek(file_dis,start,SEEK_SET);	
						read(file_dis,temp,(end-start));
						lseek(file_dis,start,SEEK_SET);
						if(write(file_dis,src,size_src-1) != size_src-1){
							perror("ERROR WRITE : ");exit(1);
						}
						if(write(file_dis,temp,(end-start)) != (end-start)){
							perror("ERROR WRITE : ");exit(1);
						}
						free(temp);						
					}
				}else{
					if (errno == 0){
						if( write(file_dis,src,size_src-1) != size_src-1 ){
							perror("error :");exit(1);
						}
						if(write(file_dis,"\n",1) != 1){
							perror("error :");exit(1);
						}
					}
					else{
						perror("ERROR READ : ");exit(1);
					}
				}
			}
		}
	}
	/* unlocking the file */
	free(ch);
	lock.l_type =  F_UNLCK;
	fcntl(file_dis,F_SETLKW,&lock);

	/* ****************** */
	if(close(file_dis) == -1){
		perror("CLOSE ERROR :");exit(1);
	}

}
