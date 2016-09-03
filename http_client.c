//By Grant Espe
//

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[]){

    if(argc < 3){
        printf("\n Invalid Command Line Args, Quiting... \n");
        return 1;
    }

    char out_buffer[513];
    char in_buffer[1025];

    char* url = argv[1];

    //fill out_buffer
    sprintf(out_buffer, "GET %s HTTP/1.1\r\n"
            "\r\n", url);
   

    int bytesRcvd = 0;
    int totalBytesRcvd = 0;
    unsigned short server_port = atoi(argv[2]);

    int socketfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socketfd < 0){
        printf("\nsocket() failure\n");
        return 1;
    }


    //construct server address struct
    struct sockaddr_in server_address;
    memset(&server_address, '0', sizeof(server_address));
    server_address.sin_family = AF_INET;


    //-----------------FIX THIS-------------------
    server_address.sin_addr.s_addr = inet_addr(url);

    server_address.sin_port = htons(server_port);

    //coneckt to server
    int connect_result = connect(socketfd, (const struct sockaddr*)&server_address,
            sizeof(server_address));

    if(connect_result < 0){
        printf("\nconnect() failure\n");
        return 1;
    }

    //send buffer
    int send_result = send(socketfd, out_buffer, strlen(out_buffer), 0);
    if(send_result != strlen(out_buffer)){
        printf("\n send() sent different number of bytes than expected \n");
        return 1;
    }

    //receive stream
    int in_len = strlen(in_buffer);
    while(totalBytesRcvd < strlen(in_buffer)){
        if((bytesRcvd = recv(socketfd, in_buffer, in_len - 1, 0)) <= 0){
            printf("\n recv() failed or connection lost \n");
            return 1;
        }
        totalBytesRcvd += bytesRcvd;
        
        //terminate string
        in_buffer[bytesRcvd] = '\0';
        printf("%s", in_buffer);
    }
    printf("\n");
    close(socketfd);

    return 0;
}
