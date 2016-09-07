#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

#define MAXPENDING (5)

int main (int argc, char* argv[]){

    if(argc != 2){
        printf("USAGE: ./http_server <port>\n");
    }

    int server_socket;
    int client_socket;
    
    struct sockaddr_in client_address;
    struct sockaddr_in my_address;

    unsigned short my_port = atoi(argv[1]);

    
    if(server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP < 0)){
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


    }
    return 0;
}

