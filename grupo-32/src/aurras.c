
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include<string.h>
#define BUFFSIZE 1024

void showStatus(int r, int w){
    char buffer[BUFFSIZE];
    write(w, "status", strlen("status"));
    int bytesRead = read(r, buffer, BUFFSIZE);
    write(1, buffer, bytesRead);
}

void showError(){
    char error[] = "Comando Invalido\n";
    write(1, error, strlen(error));

}

void joinMesasge(char const *comands[], int numComands, char result []){
    result[0] = '\0';
    for(int i=2; i<numComands; i++){
        strcat(result, comands[i]);
        if(i != numComands-1)
            strcat(result, " ");

    }
}

int checkFilters(int argc, char const *argv[]){
    int controlo = 1;
    for(int i=4; i<argc && controlo; i++)
        if(strcmp(argv[i], "alto")!=0 && strcmp(argv[i], "baixo")!=0 && strcmp(argv[i], "eco")!=0 && strcmp(argv[i], "rapido")!=0 && strcmp(argv[i], "lento")!=0)
            controlo = 0;
        return controlo;

}


int main(int argc, char const *argv[]) 
{

    
    if (argc <2)
        return 0;

    int bytesRead;
    char pidR[20];
    char pidW[20];
    char buffer[BUFFSIZE];
    char done[] = "done\n";
    
    
    int pid = getpid();
    sprintf(pidR,"../tmp/%dR",pid);
    sprintf(pidW,"../tmp/%dW",pid);

    

    
    mkfifo(pidR,0644);
    mkfifo(pidW,0644);

    

    int principal = open("principal", O_WRONLY);
    
    
    
    write(principal, &pid, sizeof(pid));
    close(principal);
     

    int fifo_W = open(pidW, O_WRONLY);
    int fifo_R = open(pidR, O_RDONLY);
    
    if(argc==2){
        if(strcmp(argv[1], "status") == 0 )
            showStatus(fifo_R, fifo_W);
        else
            showError();
        return 0;
    }

    
    
    if(strcmp(argv[1], "transform")!=0 || argc<5){
        showError();
        return 0;
    }

    if(checkFilters(argc, argv) == 0){
        showError();
        return 0;
    }

    joinMesasge(argv, argc, buffer);
    int r = strlen(buffer);
    

    write(fifo_W, buffer, strlen(buffer));
    
    
    while((bytesRead = read(fifo_R, buffer, BUFFSIZE)) >0){
        write(1, buffer, bytesRead);
        buffer[0] = '\0';
    }

    close(fifo_R);
    close(fifo_W);
    close(principal);
    unlink(pidR);
    unlink(pidW);
    write(1, done, strlen(done));

    return 0;

}