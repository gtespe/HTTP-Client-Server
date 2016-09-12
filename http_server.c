#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>

#define MAXPENDING (5)
#define IN_BUFFER_LEN (1024)
#define OUT_BUFFER_LEN (1024)
#define FILECHUNK_SIZE (1024)

int handle_client(int socket);

int main (int argc, char* argv[]){

    if(argc != 2){
        printf("USAGE: ./http_server <port>\n");
    }

    int server_socket;
    int client_socket;
    
    struct sockaddr_in client_address;
    struct sockaddr_in my_address;

    unsigned short my_port = atoi(argv[1]);
    
    if((server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        printf("socket() failed, quitting");
        return 1;
    }

    int set_option = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&set_option,
                    sizeof(int));

    //Fill server address struct
    memset(&my_address, 0, sizeof(my_address));
    my_address.sin_family = AF_INET;
    my_address.sin_addr.s_addr = htonl(INADDR_ANY);
    my_address.sin_port = htons(my_port);

    //Bind and listen
    if(bind(server_socket, (struct sockaddr*) &my_address, sizeof(my_address)) < 0){
        printf("bind() failed, quitting");
        return 1;
    }

    if(listen(server_socket,MAXPENDING) < 0){
        printf("listen() failed, quitting");
        return 1;
    }
    //start infinite loop
    while(1){
        printf("Waiting for client...\n");

        //size of client address struct
        int client_address_size = sizeof(client_address);
        //accept incoming connection


        if((client_socket = accept(server_socket, (struct sockaddr *) 
                &my_address, &client_address_size)) < 0){
            printf("accept() failed, quitting");
            return 1;
        }

        printf("Client connected at %s\n", inet_ntoa(client_address.sin_addr));
        handle_client(client_socket);
        close(server_socket);
    }
    return 0;
}


//Fetches the file and sends it to the client over the socket
// Sends http statuses as well.
int handle_client(int socket){
    //Get the date for http
    time_t t = time(NULL);
    struct tm* date = localtime(&t);

    int bytesRcvd;
    char in_buffer[IN_BUFFER_LEN]; 

    if((bytesRcvd = recv(socket, &in_buffer, IN_BUFFER_LEN,0)) <= 0){
        printf("\nrecv() failed or client disconnected, quitting...\n");
        return 1;
    }

    printf("\n recv successful:\n %s\n", in_buffer);
    
    char out_buffer[OUT_BUFFER_LEN];

    char* http_method = strtok(in_buffer, " ");
    char* filename = strtok(NULL, " ");

    //This is to make the filename ignore the initial /
    filename+=1;

    char* http_version = strtok(NULL, " ");

    printf("\n httpmethod: %s\n", http_method);
    printf("\n filename: %s\n", filename);
    printf("\n http_version: %s\n", http_version);

    //check for parsing errors
    if(0){
        //Error parsing request, send 403
        printf("Invalid request, sending 403\n");
        sprintf(out_buffer, "HTTP/1.1 403 Bad Request\r\n" 
                            "Date: %c\r\n"
                            "\r\n", date);
        int out_len = strlen(out_buffer);
        if(send(socket, out_buffer, out_len, 0) != out_len){
            printf("\nsent different number of bytes than expected");
        }
        printf("403 sent\n");
        return 1;
    }
    else{
        //check if the file exists
        if(access(filename, F_OK) != -1){
            //open the requested file and find its length
            FILE *fp = fopen(filename, "r");
            fseek(fp, 0, SEEK_END);
            int file_len = ftell(fp);
            fseek(fp, 0, SEEK_SET);

            //File exists, send 200 ok, and then the file
            sprintf(out_buffer, "HTTP/1.1 200 OK\r\n" 
                                "Date: %s\r\n"
                                "Content-Type: text/plain\r\n"
                                "Content-Length: %d\r\n"
                                "\r\n", date, file_len);

            int out_len = strlen(out_buffer);

            if(send(socket, out_buffer, out_len, 0) != out_len){
                printf("\nsent different number of bytes than expected");
            }
            printf("200 sent\n");

            char *filechunk = (char*)calloc(FILECHUNK_SIZE,1);
            int bytes_read = fread(filechunk, sizeof(filechunk), 1, fp);
            int chunk_len = strlen(filechunk);

            //This is for the last send(), so that we can cut off the extra bytes
            int leftover_len = file_len % chunk_len;

            while(bytes_read > 0){
                //send filechunks and repeat
                printf("Sending: \n  %s\n", filechunk);
                send(socket, filechunk, chunk_len, 0);
                bytes_read = fread(filechunk, sizeof(filechunk), 1, fp);
                chunk_len = strlen(filechunk);
            }
            
            if(bytes_read == 0){
                //There wasn't an error, we just reached EOF
                //   Send the leftover bytes
                send(socket, filechunk, leftover_len, 0);
            }
            else{
                //There was an error
                printf("FILE READ ERROR, Quitting");
                return 1;
            }
        }
        else{
            //file doesnt exist, send 404
            sprintf(out_buffer, "HTTP/1.1 404 Not Found\r\n" 
                                "Date: %c\r\n"
                                "\r\n", date);

            int out_len = strlen(out_buffer);
            if(send(socket, out_buffer, out_len, 0) != out_len){
                printf("\nsent different number of bytes than expected");
            }
            printf("404 sent\n");

            return 1;
        }
    }
}

