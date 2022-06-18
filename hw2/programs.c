#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "programs.h"
#include "utilities.h"

#define BUFSIZE  20


int process_p(int fd , int tfd , pid_t child_id){
    int bytesread ,byteswriten ,totalbytes = 0  , linesize;
    char read_buf[BUFSIZE] ;
    char *line;
    char *bp ;
    sigset_t bset , eset ;
    struct flock lock ;

    memset(&lock , 0 , sizeof(lock));
    if(sigfillset(&bset) == -1){
        perror("fill set ERROR :");
        kill(child_id,SIGKILL);
        exit(1);
    }
    
    line = (char*)malloc(sizeof(char)*BUFSIZE*7);
    while(1){
        while((( bytesread = read(fd,read_buf,BUFSIZE)) == -1 )   && (errno == EINTR));
        if(bytesread < BUFSIZE){          
            break;
        }
        totalbytes += bytesread ;
        if(sigprocmask(SIG_BLOCK , &bset , NULL) == -1){
            perror("parent > sigprocmask blocking ERROR :");
            kill(child_id,SIGKILL);
            exit(1);  
        }   // all signal should be blocked now !!!!!!!!!!!!!!!!!!!!!!!!!!        
        lock.l_type = F_WRLCK ;
        fcntl(tfd , F_SETLKW , &lock);  
            // temp file is locked now !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        linesize = convertline(&line,read_buf,BUFSIZE);
        bp = line ;
        lseek(tfd , 0 , SEEK_END );   
        while(linesize > 0){
            while(((byteswriten = write(tfd,bp,linesize))==-1) && (errno == EINTR));
            linesize -= byteswriten ;
            bp += byteswriten ; 
        }
        line[0] = '\0';
        lock.l_type = F_UNLCK ;
        fcntl(tfd , F_SETLKW , &lock);
            // temp file is unlocked now !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  
            // all signal should be unblocked now !!!!!!!!!!!!!!!!!!!!!!!!
        kill(child_id,SIGUSR1);           
        sigsuspend(&eset);
    }
    kill(child_id,SIGUSR1);
    free(line);
    
    return totalbytes ;
}

int process_ch(int fd , int tfd , double** a , double** b , double** c){
    char buf[BUFSIZE*10] , temp[25] ;
    struct flock lock ;
    sigset_t bset , eset ;
    int lastline_length , lastline_start;
    int linenumber = 0 , size = 100;
    double *mea,*mse,*rmse;
    double smea=0,smse=0,srmse=0;
    if(sigfillset(&bset) == -1){
        perror("fill mask ERROR :");
        exit(1);
    }
    if((mea = (double*)malloc(sizeof(double)*size))==NULL){
        perror("malloc ERROR :");
        exit(0);
    }
    if((mse = (double*)malloc(sizeof(double)*size))==NULL){
        perror("malloc ERROR :");
        exit(0);
    }
    if((rmse = (double*)malloc(sizeof(double)*size))==NULL){
        perror("malloc ERROR :");
        exit(0);
    }


    memset(&lock , 0 , sizeof(lock));
    while(1){
        sigsuspend(&eset);
        if(size == linenumber){
            size *= 2 ;
            if((mea = (double*)realloc(mea,sizeof(double)*size))==NULL){
                perror("realloc ERROR :");
                exit(0);
            }
            if((mse = (double*)realloc(mse,sizeof(double)*size))==NULL){
                perror("realloc ERROR :");
                exit(0);
            }
            if((rmse = (double*)realloc(rmse,sizeof(double)*size))==NULL){
                perror("realloc ERROR :");
                exit(0);
            }

        }
        if(sigprocmask(SIG_BLOCK , &bset , NULL) == -1){
            perror("child > sigprocmask blocking ERROR :");
            exit(0);
        }
        lock.l_type = F_WRLCK ;
        fcntl(tfd , F_SETLKW , &lock); // all signals are locked and the temp file is locked 
        if((lastline_length = last_line(tfd,&lastline_start)) == -1){
            break;
        }

        lseek(tfd,lastline_start,SEEK_SET);

        if(read(tfd,buf,lastline_length-1)==-1){
            perror("read temp file ERROR :");
            exit(0);
        }
        buf[lastline_length-1] = '\0';

        ftruncate(tfd,lastline_start);
        
        find_line_errors(buf,&mea[linenumber],&mse[linenumber],&rmse[linenumber]);
        strcat(buf,", ");
        lastline_length += sprintf(temp , "%.3f",mea[linenumber]);
        strcat(buf,temp);
        strcat(buf,", ");
        lastline_length += sprintf(temp , "%.3f",mse[linenumber]);
        strcat(buf,temp);
        strcat(buf,", ");
        lastline_length += sprintf(temp , "%.3f",rmse[linenumber]);
        strcat(buf,temp);
        strcat(buf,"\n");
        lastline_length += 6 ; 

        lock.l_type = F_UNLCK ;
        fcntl(tfd , F_SETLKW , &lock);
        if(write(fd , buf , lastline_length) == -1){
            perror("writing to output ERROR :");
            exit(1);
        }

        buf[0] = '\0';
        smea += mea[linenumber] ;
        smse += mse[linenumber] ;
        srmse += rmse[linenumber] ;
        linenumber++ ;
        kill(getppid(),SIGUSR1);
    }  
    *a = mea  ;
    *b = mse  ;
    *c = rmse ; 
    return linenumber ; 
}

