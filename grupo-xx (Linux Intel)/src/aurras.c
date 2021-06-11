
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include<string.h>
#define BUFFSIZE 1024

int main(int argc, char const *argv[]) 
{

    int bytesRead;
    char pidR[10];
    char pidW[10];
    char buffer[BUFFSIZE];
    
    

    /*if (argc <2)
    {
        
        while((bytesRead = read(0,buffer,1024)) >0)
        {
            client_server_fifo = open("client_server_fifo", O_WRONLY);
            server_client_fifo = open("server_client_fifo", O_RDONLY);
            
            if (buffer[bytesRead-1] == '\n')
                buffer[bytesRead-1] = '\0';
            
            
            write(client_server_fifo, buffer, 1024);

            while((bytesRead = read(server_client_fifo, buffer, 1024)) > 0)
                write(1, buffer, bytesRead);
            
            
        }
    }

    close(server_client_fifo);
    close(client_server_fifo);
    */
    
    
    
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
    dup2(fifo_R,0);
    

    while(bytesRead = (read(1, buffer, BUFFSIZE))>0){
        write(fifo_W, buffer, bytesRead);
    }
    
    

    char message[] = "status";
    write(fifo_W, message, sizeof(message));
    
   


    
    


    return 0;

}