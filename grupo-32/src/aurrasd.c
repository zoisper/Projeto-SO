#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include<string.h>
#define BUFFSIZE 1024

int numFilters = 0;
char tasksPath[100] = "tmp/tasks.txt";
char filtersOcupationPath[100] = "tmp/filtersOcupation.txt";
int principal;
char fifo[] = "tmp/fifo";



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


int readline(int src, char *dest)
{

    int bytesRead = 0;
    char local[1] = "";

    while(read(src, local, 1) > 0 && local[0] != '\n')
        dest[bytesRead++] = local[0];
    
    dest[bytesRead] = '\0';
    return bytesRead;
    
}


int splitLine(char src[], char *dest[])
{

    char *token;
    int destSize = 0;


    token = strtok(src, " ");
    while(token != NULL)
    {
        
        dest[destSize] = token;
        token = strtok(NULL, " ");
        destSize++;
    }

    return destSize;

}



filter makeFilter(char s[], char const *path)
{
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


filter addFilter(filter f, char s[], char const *path)
{

    filter new = makeFilter(s, path);
    filter *ptr = &f;
    while(*ptr && strcmp((*ptr)->type, new->type)<0)
        ptr = & ((*ptr)->prox);
    new->prox = (*ptr);
    (*ptr) = new;
    return f;
}


int configServer(char const *path[], filter *f)
{
    char buffer[BUFFSIZE];
    filter r = NULL;
    int config_file = open(path[1], O_RDONLY);
    
    if(config_file <0)
        return 0;
    numFilters = 0;
    
    while(readline(config_file, buffer))
    {
       r = addFilter(r, buffer, path[2]);
       numFilters++;
    }
    close(config_file);
   *f = r;
   return 1;
}





void increaseFiltersOcupation(filter f, char *requests[], int numRequests)
{
    while(f)
    {
        for(int i=2; i<numRequests; i++)
            if(strcmp(f->type, requests[i]) == 0)
                f->ocupation++;
        f=f->prox;
    }
}

void decreaseFiltersOcupation(filter f, char *requests[], int numRequests)
{
    while(f)
    {
        for(int i=2; i<numRequests; i++)
            if(strcmp(f->type, requests[i]) == 0)
                f->ocupation--;
        f=f->prox;
    }
}




void createFiltersOcupationFile()
{
    int fd = open(filtersOcupationPath,O_RDWR | O_TRUNC |O_CREAT, 0600);
    close(fd);
}


void createTasksFile()
{
    int fd = open(tasksPath, O_RDWR | O_TRUNC, 0777);
    if(fd <1)
        fd = open(tasksPath, O_RDWR | O_CREAT, 0777); 
    close(fd);

}



void saveFiltersOcupation(filter f)
{
    int fd = open(filtersOcupationPath, O_RDWR);
    int ocupation[numFilters];
    for(int i = 0; i< numFilters && f; i++)
    {
        ocupation[i] = f->ocupation;
        f = f->prox;
    }
    lseek(fd, 0, SEEK_SET);
    write(fd, ocupation, sizeof(int)*numFilters);
    close(fd);
}

void loadFiltersOcupation(filter f)
{
    int fd = open(filtersOcupationPath, O_RDWR);
    int ocupation[numFilters];
    lseek(fd, 0, SEEK_SET);
    read(fd, ocupation, sizeof(int)*numFilters);
    for(int i = 0; i<numFilters && f; i++)
    {
        f->ocupation = ocupation[i];
        f = f->prox;
    }
    close(fd);
}


int addTask(char process[])
{
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

void deleteTask(int taskNumber)
{
    int tasks = open(tasksPath,O_RDWR);
    task aux;
    lseek(tasks, 0, SEEK_SET);
    while(read(tasks, &aux, sizeof(struct Task))>0 && aux.num!=taskNumber);
    lseek(tasks, -sizeof(struct Task), SEEK_CUR);
    aux.status = 1;
    write(tasks, &aux, sizeof(struct Task) );
    close(tasks);
}




void writeStatus(int file, filter f)
{
    char buffer[BUFFSIZE];
    char result[BUFFSIZE] = "";
    int bytesRead = 0;
    task aux;
    int tasks = open(tasksPath, O_RDONLY);
    lseek(tasks, 0, SEEK_SET);
    while(read(tasks, &aux, sizeof(struct Task))>0)
    {
        if(aux.status == 0)
        {
            bytesRead += sprintf(buffer,"task #%d transform %s\n", aux.num, aux.process);
            strcat(result, buffer);
        }
    }
    close(tasks);
    loadFiltersOcupation(f);
    while(f)
    {
        bytesRead += sprintf(buffer,"filter %s: %d/%d (running/max)\n", f->type, f->ocupation, f->max);
        strcat(result, buffer);
        f = f->prox;
    }
        bytesRead += sprintf(buffer,"pid: %d\n", getpid());
        strcat(result, buffer);

    

    write(file, result, bytesRead);
}



int isAllFiltersAvailable(filter f, char *requests[], int numRequests)
{
    int i, acc, result = 1;
    while(f && result)
    {
        for(i=2, acc=0; i<numRequests && result; i++)
            if(strcmp(requests[i],f->type)==0)
                acc++;
        if(f->max < f->ocupation + acc)
            result = 0;
        f = f->prox;
    }
    return result;
}



filter getFilter(filter f, char type[])
{
    while(f && strcmp(f->type, type)!=0)
        f = f->prox;
    return f;
}



/*int apllyFilter(filter configs, char type[], char inFile[], char outFile[])
{
    int in = open(inFile, O_RDONLY, 0777);
    int out = open(outFile, O_WRONLY | O_CREAT  , 0777);
    if(in  < 0 || out < 0)
        return 0;
    
    filter f = getFilter(configs, type);
    
    if (fork() == 0){
        dup2(in,0);
        dup2(out,1);
        execl(f->name, f->name, NULL);
    }

    else{
        close(in);
        close(out);
        wait(NULL);
    }
    return 1;
}



int apllyFilters(filter configs, char *requests[], int numRequests)
{
    int controlo = 1;
    for(int i=2; i<numRequests && controlo; i++)
    {
        if(i==2)
            controlo = apllyFilter(configs, requests[i], requests[0], requests[1]);
        else
            controlo = apllyFilter(configs, requests[i], requests[1], requests[1]);
        
    }

    return controlo;
}*/

void closePipes(int fildes[][2], int n)
{
    int i;
    for (i=0; i <n; i++)
    {
        close(fildes[i][0]);
        close(fildes[i][1]);
    }

}

void makePipes(int fildes[][2], int n){
    for(int i=0; i<n; i++)
        pipe(fildes[i]);
}


int apllyFilters(filter configs, char *requests[], int numRequests)
{
    int in = open( requests[0],O_RDONLY, 0777);
    int out = open(requests[1], O_RDWR | O_CREAT ,0777);
    if(in  < 0 || out < 0)
        return 0;
    
    int fildes[numRequests-2][2];


    if (fork() == 0)
    {
        if(numRequests == 3)
        {
            filter f = getFilter(configs,requests[2]);
            dup2(in,0);
            dup2(out,1);
            execl(f->name, f->name, NULL);
        }
        else
        {


            makePipes(fildes, numRequests-2);

            for(int i=2; i<numRequests; i++)
            {
                filter f = getFilter(configs, requests[i]);
                if(fork() == 0)
                {
                    if(i == numRequests-1)
                    {
                        dup2(in,0);
                        dup2(fildes[i-3][1],1);
                        closePipes(fildes,numRequests-2);
                        execl(f->name, f->name, NULL);
                    
                    }
                }
                else
                    if(i == 2)
                    {
                        dup2(out,1);
                        dup2(fildes[i-2][0],0);
                        closePipes(fildes,numRequests-2);
                        execl(f->name, f->name, NULL);
                    
                    }
                    else
                        if(i != numRequests-1)
                        {
                            dup2(fildes[i-2][0],0);
                            dup2(fildes[i-3][1],1);
                            closePipes(fildes,numRequests-2);
                            execl(f->name, f->name, NULL);
                        }
                    
                    else
                    {
                        closePipes(fildes, numRequests-1);
                        _exit(0);
                    }
            }    
        }
    
    }
    

    else
    {
        wait(NULL);
        close(in);
        close(out);
    }

            
    return 1;
    
}



int checkInput(filter configs, char *requests[], int numRequests)
{
    int acc, i, controlo = 1;;
    
    while(configs && controlo)
    {
        for(i=2, acc=0 ; i<numRequests; i++)
            if(strcmp(configs->type, requests[i])==0)
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
    char * requests[BUFFSIZE];
    int pid;
    int bytesRead;
    char pidR[20];
    char pidW[20];
    char pending[] = "pending\n";
    char processing[] = "processing\n";
    char numFiltersExceeded[] = "exceeded number of filters allowed\n";
    char sourceError[] = "source file not found\n";
    char argumentsError[] = "invalid number of arguments\n";
    char configServerError[] = "filters not found\n";

    
    
    if(argc < 3)
    {
        write(1, argumentsError, strlen(argumentsError));
        return 0;
    }
    

    filter configs = NULL; 
    
    if (!configServer(argv, &configs))
    {
        write(1, configServerError, strlen(configServerError));
        return 0;
    }

    
    createTasksFile();
    createFiltersOcupationFile();
    
    saveFiltersOcupation(configs);

    mkfifo(fifo, 0777);
    principal = open(fifo, O_RDWR);

       
    signal(SIGTERM, handler);
    
    while(read(principal, &pid, sizeof(pid)) > 0)
    {
        
        if(fork() == 0 )
        {
        
            sprintf(pidR,"tmp/%dR",pid);
            sprintf(pidW,"tmp/%dW",pid);
            int fifo_R = open(pidW, O_RDONLY);
            int fifo_W = open(pidR, O_WRONLY);

            bytesRead = read(fifo_R, &buffer, BUFFSIZE);
            buffer[bytesRead] = '\0';
            if(strcmp(buffer, "status") == 0)
                writeStatus(fifo_W, configs);
            else{
                    char newTask[BUFFSIZE] = "transform ";
                    strcpy(newTask, buffer);
                    int numRequests = splitLine(buffer, requests);
                        
                    if (checkInput(configs, requests, numRequests) == 0)
                        write(fifo_W,numFiltersExceeded, strlen(numFiltersExceeded));
                    else
                    {
                        sleep(1);
                        loadFiltersOcupation(configs);
                        if(!isAllFiltersAvailable(configs, requests, numRequests))
                        {
                            write(fifo_W, pending, strlen(pending));
                            while(!isAllFiltersAvailable(configs, requests, numRequests)){
                                sleep(1);
                                loadFiltersOcupation(configs);
                            }
                        }
                        
                        
                        increaseFiltersOcupation(configs, requests, numRequests);
                        saveFiltersOcupation(configs);
                        write(fifo_W, processing, strlen(processing));
                        int numTask = addTask(newTask);
                
                        if (apllyFilters(configs, requests, numRequests) == 0 )
                            write(fifo_W, sourceError, strlen(sourceError));
                        sleep(1);
                        loadFiltersOcupation(configs);
                        decreaseFiltersOcupation(configs, requests, numRequests);
                        saveFiltersOcupation(configs); 
                        deleteTask(numTask);
                    }
               

                }
                
            close(fifo_W);
            close(fifo_R);
            _exit(0);
            
        }
        
    }

    unlink(fifo);


    

return 0;
}