#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "utilities.h"
#include "programs.h"

void myhandler1(int signo);
void print_pending_sig(sigset_t pendingset, char* str);

int main(int argc, char *argv[]){

	char* input_file_name;
	char* output_file_name ;
    char template[] = "tempfile_XXXXXX";
    int tfd ;
    pid_t _pid ;

    sigset_t newset ; 
    struct sigaction sa ;
    memset(&sa , 0 , sizeof(sa));
    sa.sa_handler = &myhandler1; 

	if( parsing_commandline(argc,argv,&input_file_name,&output_file_name) != 0){
    	write(1,usage_information(),80);	
    	return 2 ; // in case of wrong input 
    }
    if(sigaction(SIGUSR1,&sa , NULL) == -1 || sigaction(SIGUSR2,&sa , NULL) == -1){
        perror("sigaction ERROR :");
        return 2 ;
    }
    if( (sigemptyset(&newset) == -1) || (sigaddset(&newset,SIGUSR1) == -1)|| (sigaddset(&newset,SIGUSR2) == -1) ){
        perror("adding to the new set ERROR :");
        exit(2);
    }
    if(sigprocmask(SIG_BLOCK,&newset,NULL) == -1 ){
        perror("blocking sigusr1 ERROR :");
        exit(2);   
    }
    if((tfd = mkstemp(template)) == -1){
        perror("creat temp file ERROR :");
        return 2 ;
    }
    unlink(template);
    
    _pid = fork();
    if(_pid == 0){
        /********************    child process     ******************/
        int fd , linenum ;
        double *mae , *mse , *rmse ;
        double smae , smse , srmse ;
        double sdmae , sdmse , sdrmse ;

        sigset_t childset ,pendingset;
        if(sigemptyset(&pendingset)==-1){
            perror("empty set ERROR :");
            exit(1);   
        }
        if((fd = open(output_file_name , O_WRONLY | O_CREAT | O_APPEND , S_IRUSR | S_IWUSR)) == -1){
            perror("input file error :");
            exit(1);
        }
        linenum = process_ch(fd , tfd , &mae , &mse , &rmse);
        find_SD( mae , mse , rmse , linenum , &smae , &smse , &srmse , &sdmae , &sdmse , &sdrmse  );
        free(mae);
        free(mse);
        free(rmse);
        sigsuspend(&childset);
        printf("error metric :\n\tmaen of  mae  = %.3f\tsd of mea  = %.3f\n\tmaen of  mse  = %.3f\tsd of mse  = %.3f\n\tmaen of  rmse = %.3f\tsd of rmse = %.3f\n",
                                                    smae,sdmae,smse,sdmse,srmse,sdrmse );
        if(close(fd)== -1){
            perror("close output file ERROR :");
        }
        if(close(tfd) == -1){
            perror("close temp file ERROR :");
        }
        //unlink(input_file_name);
        if(sigpending(&pendingset)==-1){
            perror("child > pendingset ERROR :");
            exit(1);
        }
        print_pending_sig(pendingset,"child process");
        return 0 ;
    }
    else{
        /********************   parent process     ******************/
        int fd , totalbytesread;
        sigset_t pendingset  ;
        if(sigemptyset(&pendingset)==-1){
            perror("empty set ERROR :");
            kill(_pid,SIGKILL);
            exit(1);   
        }   
        if((fd = open(input_file_name , O_RDONLY)) == -1){
            perror("input file ERORR :");
            kill(_pid,SIGKILL);
            exit(1);
        } 

        totalbytesread = process_p(fd , tfd , _pid);
        printf("total bytes read from input file = %d\nestimated line equation = %d\n",totalbytesread,totalbytesread/20 );
        if(sigpending(&pendingset)==-1){
            perror("parent > pendingset ERROR :");
            kill(_pid,SIGKILL);
            exit(1);
        }
        print_pending_sig(pendingset,"parent process");
        if(close(fd) == -1){
            perror("close input file ERROR :");
            kill(_pid,SIGKILL);
            exit(1);
        }
        kill(_pid,SIGUSR2);
        wait(NULL);
        
        return 0 ;
    }
}


void myhandler1(int signo){
}

void print_pending_sig(sigset_t pendingset , char* str){
    for (int i = 1; i < 50; ++i)
    {
        if(sigismember(&pendingset,i)==1){
            printf("SIGno (%d) was pending to %s\n",i,str);
        }
    }
    
} 