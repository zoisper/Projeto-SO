#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>

int main(int argc, char const *argv[]) 
{

   	char buffer[1024];
    mkfifo("client_server_fifo", 0644);
    mkfifo("server_client_fifo", 0644);

    int client_server_fifo = open("client_server_fifo", O_RDONLY);
    int server_client_fifo = open("server_client_fifo", O_WRONLY);
    read(client_server_fifo, buffer, 1024);
    printf("%s\n",buffer );



    return 0;
}