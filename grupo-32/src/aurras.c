
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


int main(int argc, char const *argv[]) 
{

    int bytesRead;
    char pidR[10];
    char pidW[10];
    char buffer[BUFFSIZE];
    char buffer2[BUFFSIZE];
    
    
    
    
    
    int pid = getpid();
    sprintf(pidR,"%dR",pid);
    sprintf(pidW,"%dW",pid);

    

    
    mkfifo(pidR,0644);
    mkfifo(pidW,0644);

    

    int principal = open("principal", O_WRONLY);
    
    
    
    write(principal, &pid, sizeof(pid));
    close(principal);
     

    int fifo_W = open(pidW, O_WRONLY);
    int fifo_R = open(pidR, O_RDONLY);
    
    if(argc>1){
        if(strcmp(argv[1], "status") == 0 )
            showStatus(fifo_R, fifo_W);
        else 
            if(strcmp (argv[1], "transform") == 0)
                printf("%s\n", argv[1]);

        else
            showError();
    }

    char message[20] = "end\n";


    while((bytesRead = read(0, buffer, BUFFSIZE))>0){
        buffer[bytesRead-1] = '\0';
        if(strcmp(buffer, "status") == 0 )
            showStatus(fifo_R, fifo_W);
        else{
                int controlo = 1;
                write(fifo_W, buffer, bytesRead);
                while(controlo){
                    bytesRead = read(fifo_R, buffer, BUFFSIZE);
                    buffer[bytesRead] = '\0';
                    if(strcmp(buffer, "end") ==0)
                        controlo = 0;
                    else
                        write(1, buffer, bytesRead);
                
                }

            }
            
        }


            
    
    

    
    
   


    
    


    return 0;

}