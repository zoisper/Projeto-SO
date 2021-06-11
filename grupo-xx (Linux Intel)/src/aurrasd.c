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
    int ocupation;
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
    r->ocupation = 0;
    r->prox = NULL;
    return r;
}


filter addFilter(filter f, char s1[]){

    filter r = makeFilter(s1);
    r->prox = f;
    return r;
}

filter getFilter(filter f, char type[]){
    while(f && strcmp(f->type, type)!=0)
        f = f->prox;
    return f;
}





void showFilters(filter f){
    while(f){
        printf("filter %s: %d/%d (running/max)\n",f->type, f->ocupation, f->max );
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

int[] sendStatus(filter f){
    int r[]
    while(f){

    }
}

int numFilters(filter f){
    int r = 0;
    while(f){
        r++;
        f = f->prox;
    }
    return r;
}







int main(int argc, char const *argv[]) 
{

   

    if(argc < 2)
        return 0;
    //printf("Ola\n");


    filter configs = configServer(argv[1]);
    int fn = numFilters(configs);
    char buffer[BUFFSIZE];
    int pid;
    int bytesRead;
    int fildes[2];

    pipe(fildes);


    mkfifo("principal", 0644);


    
    int principal = open("principal", O_RDWR);
    
    
    printf("Ola\n");
    
    while(read(principal, &pid, sizeof(pid)) > 0){
        
        if(fork() == 0 ){
            //close(fildes[1]);
            dup2(fildes[0],0);
            int ocupation;
            read(0, buffer, BUFFSIZE);
            printf("%s",buffer );
            char pidR[10];
            char pidW[10];
            sprintf(pidR,"%dR",pid);
            sprintf(pidW,"%dW",pid);
            int fifo_R = open(pidR, O_RDONLY);
            int fifo_w = open(pidW, O_WRONLY);
            //dup2(fifo_w,1);
            //int fifo_w = open(pidW, O_WRONLY);
            
            while(bytesRead = (read(fifo_w, &buffer, BUFFSIZE))>0 && strcmp(buffer, "end")!=0)
                if(strcmp(buffer, "status")==0)
                    showFilters(configs);
            _exit(0);
        }
        
        else{
            dup2(fildes[1],1);
            //showFilters(configs);
        }
        
    }
    
    


    return 0;
}