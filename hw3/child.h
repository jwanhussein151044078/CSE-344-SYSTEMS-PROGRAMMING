#ifndef child_H_
#define child_H_

void child(int quarter , int p1[2] , int p2[2] , int n);
int** init_arr(int** in , int size);
int** process_line(char* read_line ,int length ,int r);
int** product(int** arr ,int r);
char* preper_to_send(int **arr,int size,int *length ,char quarter[]);

#endif 