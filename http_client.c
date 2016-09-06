//By Grant Espe
//

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

#define OUT_BUFFER_LEN (513)
#define IN_BUFFER_LEN (1025)
#define USERAGENT "http_client 1.0"

int host_to_ip(char* hostname, char* ip);
int parse_url(char* raw_url, char* domain, char* page);

int main(int argc, char* argv[]){

    if(argc < 3){
        printf("\n Invalid Command Line Args, Quiting... \n");
        return 1;
    }

    char out_buffer[OUT_BUFFER_LEN];
    char in_buffer[IN_BUFFER_LEN];

    char* raw_url = argv[1];

    char* page = malloc(33);
    char* domain = malloc (33);

    //get the domain and page
    parse_url(raw_url, domain, page);

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

    char* ip = malloc(32);
    host_to_ip(domain,ip);
    printf("\nip = %s\n", ip);
    
    //fill out_buffer
    sprintf(out_buffer, "GET %s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "User-Agent: %s\r\n"
            "\r\n", page, domain, USERAGENT);

    printf("\n\nSending \n%s \n\n", out_buffer);

    server_address.sin_addr.s_addr = inet_addr(ip);
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
    while(totalBytesRcvd < IN_BUFFER_LEN){
        if((bytesRcvd = recv(socketfd, in_buffer, IN_BUFFER_LEN - 1, 0)) <= 0){
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

//Converts a hostname to an ip address
int host_to_ip(char* hostname, char* ip){

   struct hostent *host = gethostbyname(hostname);
   if(host == NULL){
        printf("\nHost to IP conversion error \n");
        return 1;
   }

   //get the address
   struct in_addr **addresses;
   addresses = (struct in_addr **) host->h_addr_list;
   char* result=inet_ntoa(*addresses[0]);

   printf("\nIP == %s\n", result);
   strcpy(ip, result);

   return 0;
}

//Gets the page or directory out of an url (default /)
int parse_url(char* raw_url, char* domain, char* page){

    int contains_slash = 0;
    int url_len = strlen(raw_url);
    
    int index;
    for(index = 0; index < url_len; index++){
        if(raw_url[index] == '/'){
            contains_slash = 1;
            break;
        }
    }

    //if the url contains a slash, put page equal to everything after the slash
    if(contains_slash){
        strcpy(page, &raw_url[index]);
        printf("ehert\n");
        raw_url[index] = '\0';
        strcpy(domain, raw_url);
    }
    else{
        strcpy(page, "/");
        strcpy(domain, raw_url);
    }

    return 0;
}
