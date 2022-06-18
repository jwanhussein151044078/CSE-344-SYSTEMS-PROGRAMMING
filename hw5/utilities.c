# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <signal.h>
# include "utilities.h"

int parsing_commandline(int argc, char *argv[] , char** fn){
	int opt;

	while((opt = getopt(argc, argv, ":i:")) != -1){	
        	
    	if(opt == 'i' && optind == 3){
    		*fn = optarg ;
    	}else{
     		return -1 ;
    	}   
    }        
    if(optind < 3){
    	return -1;
    }
    
    return 1 ;
}

struct FLORIST* get_florists( int fd , int *num ){
	char buf[BUF_SIZE] ;
	struct FLORIST *florists ;
	int counter = 0 ;
	char ch,lch = '\0' ;
	int i = 0;
	int size = 3;
	int flowernum;
	char *token ;
	
	florists = (struct FLORIST*)malloc(sizeof(struct FLORIST)*size);

	while(1){
		if(read(fd , &ch , 1)==-1){
			printf("read ERROR !!\n");
			return NULL ;
		}
		if(ch == '\n'){
			int j = 0 ;
			if(lch == '\n'){
				break ;
			}
			buf[i] = '\0';
			if(counter == size){
				size *= 2 ;
				florists = (struct FLORIST*)realloc(florists,sizeof(struct FLORIST)*size);				
			}
			sscanf(buf,"%s (%lf,%lf;%lf) :",florists[counter].name,&florists[counter].pos.x , &florists[counter].pos.y , &florists[counter].speed);
			while(buf[j] != ':')j++;
			flowernum = 0;
			florists[counter].flowers = (struct FLOWER*)malloc(sizeof(struct FLOWER)*(flowernum));
			token = strtok(&buf[j+1],", ");
			while(token != NULL){
				flowernum++;
				florists[counter].flowers = (struct FLOWER*)realloc(florists[counter].flowers,sizeof(struct FLOWER)*(flowernum +1) );
				strcpy(florists[counter].flowers[flowernum-1].name,token);
				token = strtok(NULL,", ");
			}
			florists[counter].kinds = flowernum ;
			i = 0 ;
			counter++;
			lch = ch ;
		}
		else{
			buf[i] = ch ;
			lch = ch ;
			i++;
		}
	}
	*num = counter;
	return florists ;
}

struct REQUEST_QUEUE get_QUEUE(){
	struct REQUEST_QUEUE queue ;
	memset(&queue,0,sizeof(queue));
	return queue ;
}

struct REQUEST* get_REQUEST(int fd){
	struct REQUEST* req;
	int r = 0;
	int counter = 0 ;
	char ch ;
	char buf[BUF_SIZE];

	
	while(1){
		r = read(fd,&ch,1);
		if(r == -1){
			printf("read ERROR!!!\n");
			return NULL;
		}
		else if(r == 0 && counter == 0){
			return NULL;
		}
		else if(ch == '\n' || r == 0){
			if(counter == 0 ){
				return NULL;
			}
			buf[counter] = '\0';
			req = (struct REQUEST*)malloc(sizeof(struct REQUEST));
			sscanf(buf,"%s (%lf,%lf): %s",req->name,&req->pos.x,&req->pos.y,req->flower);
			return req ;
		}
		else{
			buf[counter++] = ch ;
		}

	}
}

int get_closest_florist(const struct FLORIST* florists,const struct REQUEST* req,int number_of_florists){
	int i ;
	int temp ;
	int ret = -1 ;
	int flag =0;
	int min ;

	for( i = 0 ; i < number_of_florists ;i++){
		int j = 0 ;
		while(florists[i].kinds > j){
			if(strcmp(florists[i].flowers[j].name,req->flower) == 0){
				ret = i ;
				min = get_Chebyshev_distance(florists[0].pos,req->pos);
				flag = 1 ;
				break ;
			}
			j++;
		}
		if(flag){
			break ;
		}
	}
	for ( ; i < number_of_florists; ++i){
		temp = get_Chebyshev_distance(florists[i].pos,req->pos);
		if(temp < min){
			int j = 0 ;
			while(florists[i].kinds > j){
				if(strcmp(florists[i].flowers[j].name,req->flower) == 0){
					ret = i ;
					min = temp ;
					break;
				}
				j++;
			}		
		}
	}
	return ret ;
}

double get_Chebyshev_distance(struct POSITION pos1 ,struct POSITION pos2 ){
	double temp1 , temp2 ;

	temp1 = pos1.x - pos2.x;
	temp2 = pos1.y - pos2.y;
	if(temp1 < 0)temp1 *= -1 ;
	if(temp2 < 0)temp2 *= -1 ;

	if(temp1 > temp2){
		return temp1 ;
	}
	return temp2;
}

struct REQUEST push_REQUEST(struct REQUEST req , struct POSITION pos ){
	struct REQUEST r ;

	strcpy(r.name,req.name);
	strcpy(r.flower,req.flower);
	r.pos.x = req.pos.x ;
	r.pos.y = req.pos.y ;
	r.dis = get_Chebyshev_distance(req.pos,pos);

	return r ;

}

int get_req_processed(struct sales *s,int size){
	int sum = 0 ;
	int i ;
	for(i = 0 ; i < size ; i++){
	 	sum += s[i].totalsale ;
	}
	return sum ;
}