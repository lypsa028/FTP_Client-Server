// CLIENT SIDE
// header file inclusion
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

// pre defined PORT and SIZE of array
#define PORT 8080
#define SIZE 1024

// function for printing the result received from the server for pwd command
void got_pwd(int clientSocket, char buff[SIZE])
{
    if (recv(clientSocket, buff, SIZE, 0) < 0)
    {
        printf("\n[-] Data not received correctly or not found\n");
    }
    else
    {
        printf("\nCurrent Working Directory is: %s\n", buff);
    }
}

// function for printing the result received from the server for cdup command
void got_cdup(int clientSocket, char buff[SIZE])
{
    chdir("..");
    getcwd(buff, SIZE);
    printf("\nServer changed client Directory and is: %s\n", buff);
}

int main(int argc, char **argv)
{

    // initialisation of the variables
    int clientSocket, ser_connect, choice, fd, n, size;
    struct sockaddr_in serverAddr;
    struct stat stat_buf;
    char buff[SIZE], path[100]; // here buff[SIZE] and path[] are used to store the results

    // Creation of the Client Socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        printf("[-] Error in connection...\n");
        exit(1);
    }
    printf("[+] Client Socket is created successfully...\n");

    memset(&serverAddr, '\0', sizeof(serverAddr)); // here the memset is used to assign NULL value to the sizeof the struct serveAddr

    // Below are the Port No and the IP address of the server
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connecting the Client to the Server
    ser_connect = connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (ser_connect < 0)
    {
        printf("[-] Error in connection...\n");
        exit(1);
    }
    printf("[+] Connected to Server...\n");

    while (1)
    {
        // User enters the command from the below choice
        printf("\n***********************************************\n");
        printf("\nChoices:\n");
        printf("1 - USER - print user name of the local machine\n");
        printf("2 - LIST - list the files and directories\n");
        printf("3 - PWD - print the present working directory path\n"); 
        printf("4 - MKD - create new directory\n");
        printf("5 - RMD - remove the directory\n");
        printf("6 - GET - download file from server\n");
        printf("7 - PUT - upload file to server\n");
        printf("8 - DELE - delete any file\n");
        printf("9 - CDUP - server change the client path to its immediate parent\n");
        printf("10 - CWD - change the working directory to specified path\n");
        printf("11 - EXIT - exit the server\n");
        printf("\n***********************************************\n");
        printf("\nEnter your choice: ");
        scanf("%d", &choice);
        switch (choice)
        {
        case 1:
            printf("\nCommand: USER\n");
            strcpy(buff, "USER");              // here the USER command is copied in the buffer
            send(clientSocket, buff, SIZE, 0); // the copied USER command is sent to the server
            recv(clientSocket, buff, SIZE, 0); // the name of the USER which the server sends is received by the client
            printf("\nUSER is: %s\n", buff);
            break;
        
        case 2:
            printf("\nCommand: LIST\n");
            strcpy(buff, "LIST");              // here the LIST command is copied in the buffer
            send(clientSocket, buff, SIZE, 0); // here the LIST command is sent to the server
            recv(clientSocket, buff, SIZE, 0); // the LIST which the server gets will be received by the client here
            puts(buff);                        // prints the LIST
            break;

        case 3:
            printf("\nCommand: PWD\n");
            strcpy(buff, "PWD");               // here the PWD command is copied in the buffer
            send(clientSocket, buff, SIZE, 0); // here the PWD command is sent to the server
            got_pwd(clientSocket, buff);       // this will call the got_pwd function and print the path of the directory
            break;
        
        case 4:
            printf("\nCommand: MKD\n");
            strcpy(buff, "MKD");               // here the MKD command is copied in the buffer
            send(clientSocket, buff, SIZE, 0); // here the MKD command is sent to the server
            recv(clientSocket, buff, SIZE, 0); // Server asks to enter the directory name to be made
            printf("%s ", buff);
            scanf("%s", buff);                 // User enters the name of the directory to be made
            send(clientSocket, buff, SIZE, 0); // Sends the name to the server
            recv(clientSocket, buff, SIZE, 0); // Client receives that the directory is made
            puts(buff);

            break;
        case 5:
            printf("\nCommand: RMD\n");
            strcpy(buff, "RMD");               // here the RMD command is copied in the buffer
            send(clientSocket, buff, SIZE, 0); // here the RMD command is sent to the server
            recv(clientSocket, buff, SIZE, 0); // Server asks to enter the directory name to be removed
            printf("%s ", buff);
            scanf("%s", buff);                 // User enters the name of the directory to be removed
            send(clientSocket, buff, SIZE, 0); // Sends the name to the server
            recv(clientSocket, buff, SIZE, 0); // Client receives that the directory is removed
            puts(buff);
            break;

        case 6:
            printf("\nCommand: GET\n");
            strcpy(buff, "GET");               // here the GET command is copied in the buffer
            send(clientSocket, buff, SIZE, 0); // here the GET command is sent to the server
            printf("\nEnter the name of the file you want to download from server: ");
            scanf("%s", buff);
            send(clientSocket, buff, SIZE, 0); // client sends the name of the file to the server which is to be downloaded

            fd = open(buff, O_WRONLY | O_CREAT, S_IRWXU);        // opens the file
            while ((n = read(clientSocket, buff, SIZE - 1)) > 0) // takes all the content of the file and checks the size of the file
            {
                buff[n] = '\0';
                write(fd, buff, n);
                if (n < SIZE - 2)
                    break;
            }
            printf("\nFile transfer completed successfully :)\n"); // client successfully receives the file
            close(fd);
            break;

        case 7:
            printf("\nCommand: PUT\n");
            strcpy(buff, "PUT");               // here the PUT command is copied in the buffer
            send(clientSocket, buff, SIZE, 0); // here the PUT command is sent to the server
            printf("\nEnter the name of the file you want to upload on server: ");
            scanf("%s", buff);
            send(clientSocket, buff, SIZE, 0); // sends the name of the file which client wants to upload to the server

            fd = open(buff, O_RDONLY, S_IRUSR); // opens the file and further checks the status and size of the file
            fstat(fd, &stat_buf);
            size = stat_buf.st_size;
            printf("\nSize of the file is %d\n", size);
            printf("\nFile opened successfully :)\n");

            while ((n = read(fd, buff, SIZE - 1)) > 0)
            {
                buff[n] = '\0';
                write(clientSocket, buff, n);
            }
            printf("\nFile transfer completed :)\n"); // client successfully sent the file
            close(fd);
            break;

        case 8:
            printf("\nCommand: DELE\n");
            strcpy(buff, "DELE");              // here the DELE command is copied in the buffer
            send(clientSocket, buff, SIZE, 0); // here the DELE command is sent to the server
            recv(clientSocket, buff, SIZE, 0); // Server asks to enter the name of the file to be deleted
            printf("%s ", buff);
            scanf("%s", buff);                 // User enters the name of the file to be deleted
            send(clientSocket, buff, SIZE, 0); // Sends the name to the server
            recv(clientSocket, buff, SIZE, 0); // Client receives that the directory is deleted
            puts(buff);

            break;
        
        case 9:
            printf("\nCommand: CDUP\n");
            strcpy(buff, "CDUP");              // here the CDUP command is copied in the buffer
            send(clientSocket, buff, SIZE, 0); // here the CDUP command is sent to the server
            recv(clientSocket, buff, SIZE, 0); // receives the current working directory path from the server
            printf("%s\n", buff);
            got_cdup(clientSocket, buff); // calls the function and it will go to the immediate parent directory
            bzero(buff, SIZE);
            break;

        case 10:
            printf("\nCommand: CWD\n");
            strcpy(buff, "CWD");               // here the CWD command is copied in the buffer
            send(clientSocket, buff, SIZE, 0); // here the CWD command is sent to the server
            recv(clientSocket, buff, SIZE, 0); // here the client will receive the current working directory path
            printf("\nEnter path: ");
            scanf("%s", path);    // asks the user to enter the name of the directory where he wants to go
            if (chdir(path) != 0) // checks weather the directory exists or not
            {
                printf("\n[-] Path not found!!\n");
            }
            else
            {
                printf("\nChanged Working Directory is: %s\n", getcwd(buff, 100));
            }
            break;

        case 11:
            printf("\nCommand: EXIT\n");
            strcpy(buff, "EXIT");              // here the EXIT command is copied in the buffer
            send(clientSocket, buff, SIZE, 0); // here the EXIT command is sent to the server
            recv(clientSocket, buff, SIZE, 0);
            printf("Bye Bye Server :)\n");
            close(clientSocket);
            exit(1);
            break;
        default:
            printf("\nPlease input data correctly!!\n");
        }
    }
    return 0;
}