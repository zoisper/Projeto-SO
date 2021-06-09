
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include<string.h>

int main(int argc, char const *argv[]) 
{
    char buffer[1024];
    int bytesRead;
    int client_server_fifo;
    int server_client_fifo;
    
    

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

    char pidR[10];
    char pidW[10];
    sprintf(pidR,"%dR",pid);
    sprintf(pidW,"%dW",pid);
    

    
    mkfifo(pidR,0644);
    mkfifo(pidW,0644);

    client_server_fifo = open("client_server_fifo",O_WRONLY);
    write(client_server_fifo, &pid, sizeof(pid));
     
    
    int fifo_W = open(pidW, O_WRONLY);
    char message[] = "status";
    write(fifo_W, message, sizeof(message));
    
    //int fifo_r = open(pidR, O_RDONLY);


    
    


    return 0;

}