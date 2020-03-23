#include <unistd.h>  
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utilities.h"


int write_to_output(const char* FN , char* src , int src_size , int _Time );
int read_from_input(const char* FN ,char** line,int* line_length , int _Time);


int main(int argc, char *argv[]){
    char* line_read ;
    char* input_file_name ;
    char* output_file_name ;
    int sleep_time ;
    int other;
    int start_line  , end_line , length ;
    int Xre[16] ;
    int Xim[16] ;
    int outXre[16] ;
    int outXim[16] ;
    char final[250];
    int final_length;

    if( parsing_commandline(argc,argv,&input_file_name,&output_file_name,&sleep_time) != 0){
        write(1,usage_information(),94);    
        return 2 ; // in case of wrong input 
    }
    usleep(sleep_time*1000);
    while((line_read = malloc(sizeof(char)*160)) == NULL) {usleep(sleep_time*1000);}
	usleep(sleep_time*1000);
    while((other = read_from_input(input_file_name,&line_read,&length,sleep_time)) == 1 ){
        convert_StoI(Xre,Xim,line_read);
        DFT(Xre,Xim,outXre,outXim);
        final_length = DFT_TO_STR(final,outXre,outXim);
        write_to_output(output_file_name,final, final_length-1 ,sleep_time);
        usleep(sleep_time*1000);
    }
    free(line_read);
    return 0 ;
}

int read_from_input(const char* FN ,char** line,int* line_length , int _Time){
    int file_dis , i;
    struct flock lock ;
    char ch[1] ;
    char* temp_line ;
    int _return = 0;
    int line_start = 0 ;
    int waiting_for_file = 0 ; 
    memset(&lock , 0 , sizeof(lock));
    
    while(( file_dis = open(FN, O_RDWR ,  S_IRUSR | S_IWUSR  )) == -1 ){
        usleep(_Time*1000);
        if(waiting_for_file > 1000  ){
            
            perror("ERROR OPEN FILE : ");exit(1);
        }
        waiting_for_file++;
    }

    lock.l_type = F_WRLCK;
    fcntl(file_dis,F_SETLKW,&lock);
    /*        read a line           */
    while( read(file_dis,ch,1) == 1 ){
        if(ch[0] != '\n'){
            i = 1 ;
            while(( temp_line = malloc(sizeof(char)*160)) == NULL) {usleep(_Time*1000);}
            temp_line[0] = ch[0] ;
            while( read(file_dis,ch,1) == 1){
                temp_line[i++] = ch[0];
                if(ch[0] == '\n'){break;}
            }
            strcpy(*line , temp_line) ;
            free(temp_line);
            _return = 1 ;   
            break ;
        }
        line_start++;
    }
    /*******************************/
    /*       remove a line         */
    if(_return == 1){
        int file_size =  lseek(file_dis,0,SEEK_END) - lseek(file_dis,0,SEEK_SET) ;
        int rest_size =  lseek(file_dis,0,SEEK_END) - lseek(file_dis,i,SEEK_SET) ;
        while(( temp_line = malloc(sizeof(char)*(rest_size+1))) == NULL) {usleep(_Time*1000);}
        lseek(file_dis,i+line_start,SEEK_SET);
        read(file_dis,temp_line,rest_size);
        lseek(file_dis,line_start,SEEK_SET);
        write(file_dis,"\n",1);
        write(file_dis,temp_line,rest_size);
        ftruncate(file_dis,file_size - i+1);
    } 
    *line_length = i ;
    /*******************************/

    lock.l_type = F_UNLCK;
    fcntl(file_dis,F_SETLKW,&lock); 
    close(file_dis);
    return _return ;
}

int write_to_output(const char* FN , char* src , int src_size , int _Time ){
    struct flock lock ;
    int file_dis;
    memset(&lock , 0 , sizeof(lock));
    if(( file_dis = open(FN, O_CREAT | O_WRONLY | O_APPEND ,  S_IRUSR | S_IWUSR  )) == -1){
        perror("ERROR OPEN FILE : ");exit(1);
    }
    lock.l_type = F_WRLCK;
    fcntl(file_dis,F_SETLKW,&lock);

    if (write(file_dis,src,src_size) != src_size){
        perror("ERROR WRITE : ");exit(1);
    }
    write(file_dis,"\n",1);
    lock.l_type = F_UNLCK;
    fcntl(file_dis,F_SETLKW,&lock); 
    close(file_dis); 
}
