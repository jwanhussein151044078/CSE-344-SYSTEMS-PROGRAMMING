#ifndef utilities_H_
#define utilities_H_

# define QUEUE_SIZE 64
# define BUF_SIZE 1024

struct POSITION{
	double x ;
	double y ;
};

struct FLOWER{
	char name[32];
};

struct FLORIST{
	int kinds ;
	char name[32];
	double speed ;
	struct POSITION pos ;
	struct FLOWER *flowers ;
};

struct sales {
	int totalsale ;
	double totaltime ;
};

struct REQUEST {
	char name[32];
	int dis ;
	char flower[32];
	struct POSITION pos ;
};

struct REQUEST_QUEUE{
	struct REQUEST req[QUEUE_SIZE];
	int pointer_Producer ;
	int pointer_Consumer ;
	int used ;
};

int parsing_commandline(int argc, char *argv[] , char** fn);
int get_closest_florist(const struct FLORIST* florists,const struct REQUEST* req,int number_of_florists);
struct FLORIST* get_florists( int fd , int *num );
struct REQUEST_QUEUE get_QUEUE();
struct REQUEST* get_REQUEST(int fd);
struct REQUEST push_REQUEST(struct REQUEST req ,struct POSITION pos );
double get_Chebyshev_distance(struct POSITION pos1 ,struct POSITION pos2 );
int get_req_processed(struct sales *s,int size);

#endif 