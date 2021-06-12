#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include<string.h>
#define BUFFSIZE 1024

int FN;

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

typedef struct Task{
    int num;
    int status;
    char process[100];
} task;



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

void updateFilter(filter f, char type[], int value){
    while(f && strcmp(f->type, type)!=0)
        f = f->prox;
    f->ocupation = value;
}

void increaseFilter(filter f, char type[]){
    while(f && strcmp(f->type, type)!=0)
        f = f->prox;
    f->ocupation++;
}

void decreaseFilter(filter f, char type[]){
    while(f && strcmp(f->type, type)!=0)
        f = f->prox;
    f->ocupation--;
}

int isAvailable( filter f, char type[]){
    while(f && strcmp(f->type, type)!=0)
        f = f->prox;
    return f->ocupation < f->max;
}




void writeFilters(int file, filter f ){
    char buffer[BUFFSIZE];
    char result[1024] = "";
    int bytes = 0;
    while(f){
        bytes += sprintf(buffer,"filter %s: %d/%d (running/max)\n", f->type, f->ocupation, f->max);
        strcat(result, buffer);
        f = f->prox;
    }
        bytes += sprintf(buffer,"pid: %d\n", getppid());
        strcat(result, buffer);
        write(file, result, bytes);
}




filter configServer(char const *path){
    char buffer[BUFFSIZE];
    filter r = NULL;
    int config_file = open(path, O_RDONLY);
    while(readln(config_file, buffer))
       r = addFilter(r, buffer);
   return r;
}


int numFilters(filter f){
    int r = 0;
    while(f){
        r++;
        f = f->prox;
    }
    return r;
}

int createStatusFile(){
    return open("status.txt",O_RDWR | O_CREAT, 0600);
}

int createTasksFile(){
    return open("tasks.txt", O_RDWR | O_TRUNC | O_CREAT, 0777);;
}

void saveStatus(int file, filter f){
    int ocupation[FN];
    for(int i = 0; i< FN && f; i++){
        ocupation[i] = f->ocupation;
        f = f->prox;
    }
    lseek(file, 0, SEEK_SET);
    write(file, ocupation, sizeof(int)*FN);
}

void loadStatus(int file, filter f){
    int ocupation[FN];
    lseek(file, 0, SEEK_SET);
    read(file, ocupation, sizeof(int)*FN);
    for(int i = 0; i<FN && f; i++){
        f->ocupation = ocupation[i];
        f = f->prox;
    }
}

int addTask(int file, char process[]){
    task new, aux;
    strcpy(new.process, process);
    new.status = 0;
    lseek(file, - sizeof(struct Task), SEEK_END);
    if (read(file, &aux, sizeof(struct Task))>0)
        new.num = aux.num + 1;
    else
        new.num = 1;
    lseek(file, 0, SEEK_END);
    write(file, &new, sizeof(struct Task));
    return new.num;
}

void doneTask(int file, int taskNumber){
    task aux;
    lseek(file, 0, SEEK_SET);
    while(read(file, &aux, sizeof(struct Task))>0 && aux.num!=taskNumber);
    lseek(file, -sizeof(struct Task), SEEK_CUR);
    aux.status = 1;
    write(file, &aux, sizeof(struct Task) );

}

void writeTasks(int file){
    task aux;
    int tasks = open("tasks.txt", O_RDONLY);
    lseek(tasks, 0, SEEK_SET);
    char buffer[BUFFSIZE];
    char result[BUFFSIZE] = "";
    int bytesRead = 0;
    while(read(tasks, &aux, sizeof(struct Task))>0){
        if(aux.status == 0){
            bytesRead += sprintf(buffer,"Task %d %s\n", aux.num, aux.process);
            strcat(result, buffer);
        }
    }

    write(file, result, bytesRead);

}






int main(int argc, char const *argv[]) 
{

    int tasks = createTasksFile();
    
    if(argc < 2)
        return 0;


    filter configs = configServer(argv[1]);
    FN = numFilters(configs);
    char buffer[BUFFSIZE];
    int pid;
    int bytesRead;
    char pidR[10];
    char pidW[10];
    int fildes[2];

    int status = createStatusFile();
    int tasks = createTasksFile();
    saveStatus(status, configs);

    



    mkfifo("principal", 0644);
    int principal = open("principal", O_RDWR);
    
    
    
    
    while(read(principal, &pid, sizeof(pid)) > 0){
        
        if(fork() == 0 ){
            sprintf(pidR,"%dR",pid);
            sprintf(pidW,"%dW",pid);
            int fifo_R = open(pidW, O_RDONLY);
            int fifo_W = open(pidR, O_WRONLY);

            
            while((bytesRead = read(fifo_R, &buffer, BUFFSIZE))>0){
                if(strcmp(buffer, "status\n") == 0 || strcmp(buffer, "status") ==0)
                    writeFilters(fifo_W, configs);
                fflush(stdin);

            }
        }
        
    }


    return 0;
}