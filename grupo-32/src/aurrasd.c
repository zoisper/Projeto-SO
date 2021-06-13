#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include<string.h>
#define BUFFSIZE 1024

int numFilters = 0;
char tasksPath[100] = "../tmp/tasks.txt";
char statusPath[100] = "../tmp/status.txt";
int principal;
char fifo[] = "../tmp/fifo";

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


int getComandos(char args[], char *result[BUFFSIZE])
{

    char *token;
    int numComandos = 0;


    token = strtok(args, " ");
    while(token != NULL)
    {
        
        result[numComandos] = token;
        token = strtok(NULL, " ");
        numComandos++;
    }

    return numComandos;

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



filter makeFilter(char s[], char const *path){
    filter r = malloc(sizeof(struct Filter));
    char * token;
    token = strtok(s, " ");
    strcpy(r->type, token);
    strcpy(r->name, path);
    strcat(r->name, "/");
    token = strtok(NULL, " ");
    strcat(r->name, token);
    token = strtok(NULL, " ");
    r->max = atoi(token);
    r->ocupation = 0;
    r->prox = NULL;
    return r;
}


filter addFilter(filter f, char s[], char const *path){

    filter r = makeFilter(s, path);
    r->prox = f;
    return r;
}


filter configServer(char const *path[]){
    char buffer[BUFFSIZE];
    filter r = NULL;
    int config_file = open(path[1], O_RDONLY);
    numFilters = 0;
    while(readln(config_file, buffer)){
       r = addFilter(r, buffer, path[2]);
       numFilters++;
    }
    close(config_file);
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




void createFiltersStatusFile(){
    int fd = open(statusPath,O_RDWR | O_TRUNC |O_CREAT, 0600);
    close(fd);
}

void createTasksFile(){
    int fd = open(tasksPath, O_RDWR | O_TRUNC, 0777);
    if(fd <1)
        fd = open(tasksPath, O_RDWR | O_CREAT, 0777); 
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
            bytesRead += sprintf(buffer,"task #%d transform %s\n", aux.num, aux.process);
            strcat(result, buffer);
        }
    }
    close(tasks);
    loadStatus(f);
    while(f){
        bytesRead += sprintf(buffer,"filter %s: %d/%d (running/max)\n", f->type, f->ocupation, f->max);
        strcat(result, buffer);
        f = f->prox;
    }
        bytesRead += sprintf(buffer,"pid: %d\n", getppid());
        strcat(result, buffer);

    

    write(file, result, bytesRead);
}


int isAvailable(filter f, char type[]){
    while(f && strcmp(f->type, type)!=0)
        f = f->prox;
    return f->ocupation < f->max;
}


int isAllAvailable(filter f, char *comandos[BUFFSIZE], int numComandos ){
    int controlo = 1;
    for(int i=2; i<numComandos && controlo; i++)
        if (!isAvailable(f, comandos[i] ))
            controlo = 0;

    return controlo;
}


void increaseAllFilters(filter f, char *comandos[BUFFSIZE], int numComandos){
    for(int i = 2; i<numComandos; i++)
        increaseFilter(f, comandos[i]);
}

void decreaseAllFilters(filter f, char *comandos[BUFFSIZE], int numComandos){
    for(int i = 2; i<numComandos; i++)
        decreaseFilter(f, comandos[i]);
}

void apllyFilter(filter configs, char type[], char inFile[], char outFile[]){
    int in = open(inFile, O_RDONLY, 0777);
    if(in <0)
        return;
    int out = open(outFile, O_CREAT | O_TRUNC | O_WRONLY, 0777);
    dup2(in,0);
    dup2(out,1);
    filter f = getFilter(configs, type);
    
    if (fork() ==0)
        execl(f->name, f->name, NULL);
}

void apllyFilters(filter configs, char *comandos[BUFFSIZE], int numComandos){
    for(int i=2; i<numComandos; i++)
        if(i==2)
            apllyFilter(configs, comandos[i], comandos[0], comandos[1]);
        else
            apllyFilter(configs, comandos[i], comandos[1], comandos[1]);


}

int checkInput(filter configs, char *comandos[BUFFSIZE], int numComandos){
    int acc, i, controlo = 1;;
    
    while(configs && controlo)
    {
        for(i=2, acc=0 ; i<numComandos; i++)
            if(strcmp(configs->type, comandos[i])==0)
                acc++;

        if(configs->max < acc)
            controlo = 0;
        configs = configs->prox;
    }
    return controlo;
}

void handler(int signum){
    
    close(principal);

}




int main(int argc, char const *argv[]) 
{
    char buffer[BUFFSIZE];
    char * comandos[BUFFSIZE];
    int pid;
    int bytesRead;
    char pidR[20];
    char pidW[20];
    int fildes[2];
    char pending[] = "pending\n";
    char processing[] = "processing\n";
    char numFiltersExceeded[] = "exceeded number of filters allowed\n";
    
    
    if(argc < 2)
        return 0;
    

    createTasksFile();
    createFiltersStatusFile();
    filter configs = configServer(argv);
    saveStatus(configs);
    


    



    mkfifo(fifo, 0644);
    principal = open(fifo, O_RDWR);

       
    signal(SIGTERM, handler);
    signal(SIGINT, handler);
    
    while(read(principal, &pid, sizeof(pid)) > 0){
        
        if(fork() == 0 ){
            sprintf(pidR,"../tmp/%dR",pid);
            sprintf(pidW,"../tmp/%dW",pid);
            int fifo_R = open(pidW, O_RDONLY);
            int fifo_W = open(pidR, O_WRONLY);

            bytesRead = read(fifo_R, &buffer, BUFFSIZE);
                buffer[bytesRead] = '\0';
                if(strcmp(buffer, "status") == 0)
                    writeStatus(fifo_W, configs);
                else{
                        char newTask[BUFFSIZE] = "transform ";
                        strcpy(newTask, buffer);
                        int numComandos = getComandos(buffer, comandos);
                        
                        if (checkInput(configs, comandos, numComandos) == 0)
                            write(fifo_W,numFiltersExceeded, strlen(numFiltersExceeded));
                        
                        else
                        {
                            
                            loadStatus(configs);
                            if(! isAllAvailable(configs, comandos, numComandos))
                            {
                                write(fifo_W, pending, strlen(pending));
                                while(! isAllAvailable(configs, comandos, numComandos))
                                    loadStatus(configs);
                            }
                        

                        
                            write(fifo_W, processing, strlen(processing));
                            increaseAllFilters(configs,comandos, numComandos);
                            saveStatus(configs);
                            int numTask = addTask(newTask);
                            apllyFilters(configs, comandos, numComandos);
                            sleep((numComandos-2)*5);
                            loadStatus(configs);
                            decreaseAllFilters(configs, comandos, numComandos);
                            doneTask(numTask);
                            saveStatus(configs); 
                        }    

                    }
                
                close(fifo_W);
                close(fifo_R);
            
        }
        
    }

    unlink(fifo);

    

    return 0;
}