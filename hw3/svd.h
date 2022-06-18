#ifndef svd_H_
#define svd_H_


#define Abs(x) ((x)>0.0?(x):(-(x)))
#define SIGN(u,v) ((v)>=0.0? Abs(u):-Abs(u))
#define MAX(a,b) (((a)>(b))?(a):(b))


int dsvd(float **a, int m, int n, float *w, float **v);

#endif 