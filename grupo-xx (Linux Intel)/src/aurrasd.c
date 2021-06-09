#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include<string.h>
#define BUFFSIZE 1024

ssize_t readln(int fd, char *line)
{

    ssize_t res = 0;
    ssize_t j = 0;
    char local[BUFFSIZE];

    while((res = read(fd, local, 1))>0)
    {
        for(int i=0; i<res; i++)
        {
            if(local[i] != '\n')
            {
                line[j] = local[i];
                j++;
            }
            else{
                line[j] = '\0';
                return j;
            }
        }
    }
    return j;
    
}


typedef struct Filter{
    char type[10];
    char name[100];
    int max;
    struct Filter * prox;

} *filter;



filter makeFilter(char s1[]){
    filter r = malloc(sizeof(struct Filter));
    char * token;
    token = strtok(s1, " ");
    strcpy(r->type, token);
    token = strtok(NULL, " ");
    strcpy(r->name, token);
    token = strtok(NULL, " ");
    r->max = atoi(token);
    r->prox = NULL;
    return r;
}


filter addFilter(filter f, char s1[]){

    filter r = makeFilter(s1);
    r->prox = f;
    return r;
}




void showFilters(filter f){
    while(f){
        printf("%s %s %d \n",f->type, f->name, f->max );
        f = f->prox;
    }
}

filter configServer(char const *path){
    char buffer[BUFFSIZE];
    filter r = NULL;
    int config_file = open(path, O_RDONLY);
    while(readln(config_file, buffer))
       r = addFilter(r, buffer);
   return r;
}






int main(int argc, char const *argv[]) 
{

   
    if(argc < 2)
        return 0;
    
    filter r = configServer(argv[1]);

    
    showFilters(r);

    
        



    mkfifo("client_server_fifo", 0644);
    mkfifo("server_client_fifo", 0644);










    //int client_server_fifo = open("client_server_fifo", O_RDONLY);
    //int server_client_fifo = open("server_client_fifo", O_WRONLY);
    //read(client_server_fifo, buffer, 1024);
    //close(server_client_fifo);
    




    return 0;
}