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
    
    char out_buffer[OUT_BUFFER_LEN];
    char filename [125];
    float http_version;

    //replace the first \n with a null terminator
    //   this is kind of a hacky way of making sure the sscanf only
    //   reads the first line
    for(int k = 0; k < bytesRcvd; k++){
        if(in_buffer[k] == '\n'){
            in_buffer[k] = '\0';
            break;
        }
    }
    
    //check for sscanf errors
    if(sscanf(in_buffer, "GET %s HTTP/%f\r", filename, http_version) != 2){
        
        //Request parsing error, send 403
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
                                "Date: %c\r\n"
                                "Content-Type: text/plain\r\n"
                                "Content-Length: %d\r\n"
                                "\r\n", date, file_len);

            int out_len = strlen(out_buffer);
            if(send(socket, out_buffer, out_len, 0) != out_len){
                printf("\nsent different number of bytes than expected");
            }
            printf("200 sent\n");

            char filechunk[FILECHUNK_SIZE];
            int bytes_read = fread(filechunk, FILECHUNK_SIZE, 1, fp);

            while(bytes_read > 0){
                //send filechunks and repeat
                printf("Sending: \n  %s\n", filechunk);
                send(socket, filechunk, FILECHUNK_SIZE, 0);
                bytes_read = fread(filechunk, FILECHUNK_SIZE, 1, fp);
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

