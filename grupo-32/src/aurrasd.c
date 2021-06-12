#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include<string.h>
#define BUFFSIZE 1024

int numFilters = 0;
char tasksPath[100] = "tasks.txt";
char statusPath[100] = "status.txt";

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


filter addFilter(filter f, char s[]){

    filter r = makeFilter(s);
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

int isAvailable(filter f, char type[]){
    while(f && strcmp(f->type, type)!=0)
        f = f->prox;
    return f->ocupation < f->max;
}



filter configServer(char const *path){
    char buffer[BUFFSIZE];
    filter r = NULL;
    int config_file = open(path, O_RDONLY);
    numFilters = 0;
    while(readln(config_file, buffer)){
       r = addFilter(r, buffer);
       numFilters++;
    }
   return r;
}



void createFiltersStatusFile(){
    int fd = open(statusPath,O_RDWR | O_TRUNC |O_CREAT, 0600);
    close(fd);
}

void createTasksFile(){
    int fd = open(tasksPath, O_RDWR | O_TRUNC | O_CREAT, 0777);
    close(fd);

}

void saveStatus(filter f){
    int fd = open(statusPath, O_RDWR);
    int ocupation[numFilters];
    for(int i = 0; i< numFilters && f; i++){
        ocupation[i] = f->ocupation;
        f = f->prox;
    }
    lseek(fd, 0, SEEK_SET);
    write(fd, ocupation, sizeof(int)*numFilters);
    close(fd);
}

void loadStatus(filter f){
    int fd = open(statusPath, O_RDWR);
    int ocupation[numFilters];
    lseek(fd, 0, SEEK_SET);
    read(fd, ocupation, sizeof(int)*numFilters);
    for(int i = 0; i<numFilters && f; i++){
        f->ocupation = ocupation[i];
        f = f->prox;
    }
    close(fd);
}

int addTask(char process[]){
    int tasks = open(tasksPath,O_RDWR);
    task new, aux;
    strcpy(new.process, process);
    new.status = 0;
    lseek(tasks, - sizeof(struct Task), SEEK_END);
    if (read(tasks, &aux, sizeof(struct Task))>0)
        new.num = aux.num + 1;
    else
        new.num = 1;
    lseek(tasks, 0, SEEK_END);
    write(tasks, &new, sizeof(struct Task));
    close(tasks);
    return new.num;
}

void doneTask(int taskNumber){
    int tasks = open(tasksPath,O_RDWR);
    task aux;
    lseek(tasks, 0, SEEK_SET);
    while(read(tasks, &aux, sizeof(struct Task))>0 && aux.num!=taskNumber);
    lseek(tasks, -sizeof(struct Task), SEEK_CUR);
    aux.status = 1;
    write(tasks, &aux, sizeof(struct Task) );
    close(tasks);

}


void writeStatus(int file, filter f){
    char buffer[BUFFSIZE];
    char result[BUFFSIZE] = "";
    int bytesRead = 0;
    task aux;
    int tasks = open(tasksPath, O_RDONLY);
    lseek(tasks, 0, SEEK_SET);
    while(read(tasks, &aux, sizeof(struct Task))>0){
        if(aux.status == 0){
            bytesRead += sprintf(buffer,"Task %d %s\n", aux.num, aux.process);
            strcat(result, buffer);
        }
    }
    close(tasks);

    while(f){
        bytesRead += sprintf(buffer,"filter %s: %d/%d (running/max)\n", f->type, f->ocupation, f->max);
        strcat(result, buffer);
        f = f->prox;
    }
        bytesRead += sprintf(buffer,"pid: %d\n", getppid());
        strcat(result, buffer);

    

    write(file, result, bytesRead);
}




int main(int argc, char const *argv[]) 
{
    char buffer[BUFFSIZE];
    int pid;
    int bytesRead;
    char pidR[10];
    char pidW[10];
    int fildes[2];
    

    createTasksFile();
    createFiltersStatusFile();
    filter configs = configServer(argv[1]);
    addTask("Ola");
    addTask("Mundo");
    doneTask(1);
    

    /*if(fork() == 0){
        increaseFilter(configs, "alto");
        saveStatus(configs);
        _exit(0);
    }
    else{
        wait(NULL);
        loadStatus(configs);
        writeStatus(1, configs);
    }*/


    if(argc < 2)
        return 0;



    mkfifo("principal", 0644);
    int principal = open("principal", O_RDWR);
    
    
    
    
    while(read(principal, &pid, sizeof(pid)) > 0){
        
        if(fork() == 0 ){
            sprintf(pidR,"%dR",pid);
            sprintf(pidW,"%dW",pid);
            int fifo_R = open(pidW, O_RDONLY);
            int fifo_W = open(pidR, O_WRONLY);

            
            while((bytesRead = read(fifo_R, &buffer, BUFFSIZE))>0){
                if(strcmp(buffer, "status")==0)
                    writeStatus(fifo_W, configs);
                fflush(stdin);

            }
        }
        
    }

    

    return 0;
}