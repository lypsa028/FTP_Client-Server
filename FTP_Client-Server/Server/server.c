//SERVER SIDE

//Header files inclusions
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

//pre defined PORT number and size of buffer array.
#define PORT 8080
#define SIZE 1024

//function to perform pwd command and to get present working directory.
void perform_pwd(int newSocket, char buff[SIZE]){
    chdir("/path/to/change/directory/to");
    //getcwd() is a inbuild function which give use the current working directory path.
    getcwd(buff, SIZE);
    //send the result to client.
    send(newSocket, buff, SIZE, 0);
}

//function to perform ls command and to get list of files and directory.
void perform_list(int newSocket, char buff[SIZE]){
    DIR *dp;
    int i;
    struct dirent *dir;
    //putting zeros in buffer for safer side if some garbage value is present.
    bzero(buff,SIZE);
    strcpy(buff,"LIST\n");
    dp = opendir("./");
    if (dp)
    {
        while ((dir = readdir(dp)) != NULL)
        {   
            //concat each file and directory in the buff array.
            strcat(buff,dir->d_name);
            strcat(buff,"\n");
        }
        closedir(dp);
    }
     //send the result to client.
    send(newSocket, buff, SIZE, 0);
}

//function to perform cdup command and to get immediate parent of the client.
void perform_cdup(int newSocket, char buff[SIZE]){
    sprintf(buff,"Changed Client Directory");
    //send the result to client.
    send(newSocket, buff, SIZE, 0);
}

//function to perform mkdir and to create a directory on server.
void perform_mkdir(int newSocket, char buff[SIZE]){
    if(mkdir(buff, 0777) == 0){
        send(newSocket, "Directory created successfully :)\n", SIZE, 0);
    }
    else
        send(newSocket, "Cant create directory as it already exists!!\n", SIZE, 0);  
}

//function to perform rmdir and to remove a directory from server.
void perform_rmdir(int newSocket, char buff[SIZE]){
    if(rmdir(buff) == 0){
        send(newSocket, "Directory removed successfully :)\n", SIZE, 0);
    }
    else
        send(newSocket, "Cant remove directory as it does not exists!!\n", SIZE, 0);  
}

//function to perform dele and to delete file from server.
void perform_dele(int newSocket, char buff[SIZE]){
    if(remove(buff) == 0){
        send(newSocket, "File removed successfully :)\n", SIZE, 0);
    }
    else
        send(newSocket, "Cant remove file as it does not exists!!\n", SIZE, 0);  
}


int main(int argc, char *argv[])
{
    //variable initializations and declarations
    int socketfd, cli_bind,fd,size,n;
    struct sockaddr_in serverAddr;

    int newSocket;
    struct sockaddr_in newAddr;

    socklen_t addr_size;
    struct stat stat_buf;

    //char arrays for storing commands received from clients
    //or to send results to clients.
    char buff[SIZE],cmd[20];
   
    // child for having multiple clients.
    pid_t childpid;

    //creating Server Socket and checking for connection.
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0)
    {
        printf("[-] Error in connection...\n");
        exit(1);
    }
    printf("[+] Server Socket is created successfully...\n");

    //memset is used to fill NULL values in struct atleast to garbage free.
    memset(&serverAddr, '\0', sizeof(serverAddr));
    //taking PORT and IP adrress
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    //binding server and client and checking if binding is OK or NOT.
    cli_bind = bind(socketfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (cli_bind < 0)
    {
        printf("[-] Error in Binding..\n");
        exit(1);
    }
    printf("[+] Bind to port: %d\n", 8080);

    //listen function is used handle multiple clients.
    //I have set 11 as max client handling.
    //you can increase the number of clients.
    if (listen(socketfd, 11) == 0)
    {
        printf("[+] Listening...\n");
    }
    else
    {
        printf("[-] Error in Binding..\n");
    }

    //server enters into infinite loop
    while (1)
    {   
        //server accepts the client using accept function.
        newSocket = accept(socketfd, (struct sockaddr *)&newAddr, &addr_size);
        if (newSocket < 0)
        {
            exit(1);
        }
        //here IP address and PORT number of the client will be displayed.
        printf("Connection accepted from %s: %d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

        if ((childpid = fork()) == 0)
        {
            close(socketfd);

            while (1)
            {   
                //putting zeros in buffer for safer side if some garbage value is present.
                //or any old data is still inside buffer. 
                bzero(buff, sizeof(buff));
                //received data from client.
                recv(newSocket, buff, SIZE, 0);
                //store the buff data into cmd array.
                strcpy(cmd,buff);
                //comparing the cmd data to EXIT and if exist the performing exit.
                if (strcmp(cmd, "EXIT") == 0)
                {
                    printf("[-] Disconnected from %s: %d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
                    send(newSocket, buff, SIZE, 0);
                    exit(1);
                }
                //comparing the cmd data to PWD and if exist the performing PWD command.
                else if(strcmp(cmd,"PWD") == 0) {
                    //user funtion to perform pwd command.
                    perform_pwd(newSocket,buff);
                }
                //comparing the cmd data to LIST and if exist the performing ls command.
                else if(strcmp(cmd,"LIST") == 0) {
                    //user funtion to perform ls command.
                    perform_list(newSocket,buff);
                }
                //comparing the cmd data to USER and if exist the performing USER command.
                else if(strcmp(cmd,"USER") == 0){ 
                    //inbuilt function for user the machine user name.
                    getlogin_r(buff,SIZE);
                    send(newSocket, buff, SIZE, 0);
                }
                //comparing the cmd data to CDUP and if exist the performing CDUP command.
                else if(strcmp(cmd,"CDUP") == 0){
                    //user function to perform cdup command.
                    perform_cdup(newSocket,buff);
                }
                //comparing the cmd data to CWD and if exist the performing cwd command.
                else if(strcmp(cmd,"CWD") == 0){
                    getcwd(buff, SIZE);
                    send(newSocket, buff, SIZE, 0);
                }
                //comparing the cmd data to MKD and if exist the performing mkdir command.
                else if((strcmp(cmd,"MKD") | strcmp(buff,buff)) == 0){
                    //server will send message to user for directory name.
                    send(newSocket, "Enter directory name: ", SIZE, 0);
                    recv(newSocket, buff, SIZE, 0);
                    //user function to perform mkdir command.
                    perform_mkdir(newSocket,buff);
                }
                //comparing the cmd data to RMD and if exist the performing rmdir command.
                else if((strcmp(cmd,"RMD") | strcmp(buff,buff)) == 0){
                    //server will send message to user for directory name to be removed.
                    send(newSocket, "Enter directory name you want to remove: ", SIZE, 0);
                    recv(newSocket, buff, SIZE, 0);
                    //user function to perform mkdir command.
                    perform_rmdir(newSocket,buff);
                }
                //comparing the cmd data to DELE and if exist the performing remove file command.
                else if((strcmp(cmd,"DELE") | strcmp(buff,buff)) == 0){
                    //server will send message to user for file name to be removed.
                    send(newSocket, "Enter file name you want to delete: ", SIZE, 0);
                    recv(newSocket, buff, SIZE, 0);
                    //user function to perform remove command.
                    perform_dele(newSocket,buff);
                }
                //comparing the cmd data to GET and if exist the performing get command.
                else if((strcmp(cmd,"GET") | strcmp(buff,buff)) == 0){
                    //file name will be received to the server which client wants to
                    //download.
                    recv(newSocket, buff, SIZE, 0);
                    printf("\nFile name is %s\n", buff);

                    //file will be opend and status of file is found using fstat() function.
                    //size of the file is stored in size variable. 
                    fd=open(buff,O_RDONLY,S_IRUSR);
                    fstat(fd, &stat_buf);
                    size = stat_buf.st_size;

                    printf("\nSize of file is %d\n", size);
                    printf("\nFile opened successfully :)\n");
                    
                    //reading file from the server and writing it on client.
                    while ( (n = read(fd, buff, SIZE-1)) > 0)
                    {
                        buff[n] = '\0';
                        write(newSocket,buff,n);
                    }
                    printf("\nFile transfer completed :)\n");
                    bzero(buff,SIZE);
                    close(fd);
                }
                //comparing the cmd data to PUT and if exist the performing put command.
                else if((strcmp(cmd,"PUT") | strcmp(buff,buff)) == 0){
                    //file name will be received to the server which client wants to
                    //upload.
                     recv(newSocket, buff, SIZE, 0);
                     printf("\nFile name is %s\n", buff);
                     
                    //file will be opend using open() finction.
                     fd=open(buff,O_WRONLY|O_CREAT,S_IRWXU);
                     //reading file from the client side and storing in the server.
                     while ( (n = read(newSocket, buff, SIZE-1)) > 0)
                     {
                         buff[n] = '\0';
                         write(fd,buff,n);
                         if( n < SIZE-2)
                             break;
                     }
                     printf("\nFile receiving completed successfully :)\n");
                     close(fd);
                }
            }
        }
    }
    //closing the client connection.
    close(newSocket);
    return 0;
}   
