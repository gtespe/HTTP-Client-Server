//By Grant Espe
//

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
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


    struct timeval start, stop;

    short get_rtt = 0;

    if(argc < 3 || argc > 4){
        printf("USAGE: ./http_client [-p]  <url|ip>  <port>\n");
        return 1;
    }

    if(argc == 4){
        //find the -p arg if it exists
        for(int k = 1; k < argc; k ++){
            if(strcmp(argv[k], "-p") == 0){
                get_rtt = 1;

                //Remove the -p arg and shift argv down 1
                for(; k < argc-1; k++){
                    argv[k] = argv[k+1];
                }

                //decrement argc, because we removed the -p arg 
                //  after setting the flag
                argc -= 1;
                break;
            }
        }

        //If there were four args, but one was NOT -p
        if(!get_rtt){
            printf("USAGE: ./http_client [-p]  <url|ip>  <port>\n");
            return 1;
        }
    }

    //allocate space for buffers
    char out_buffer[OUT_BUFFER_LEN];
    //char in_buffer[IN_BUFFER_LEN];
    char in_buffer[IN_BUFFER_LEN];

    char* raw_url = argv[1];

    //allocate space for the domain name and page
    char* page = malloc(32);
    char* domain = malloc(32);

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

    //convert domain to ip if necessary
    char* ip = malloc(32);
    host_to_ip(domain,ip);
    
    //fill out_buffer
    sprintf(out_buffer, "GET %s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "User-Agent: %s\r\n"
            "\r\n", page, domain, USERAGENT);

    printf("\n\nSending \n%s \n\n", out_buffer);
    
    //construct server address struct
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip);
    server_address.sin_port = htons(server_port);

    //Start timer for RTT
    if(get_rtt) 
        gettimeofday(&start, NULL);

    //coneckt to server
    int connect_result = connect(socketfd, (const struct sockaddr*)&server_address,
            sizeof(server_address));

    //end timer for rtt
    // NOTE: this isnt in an if statement to reduce the margin of error
    gettimeofday(&stop, NULL);

    //Calculate and print the RTT in milliseconds
    if(get_rtt){
        int RTT = (stop.tv_sec*1000)+(stop.tv_usec/1000);
        RTT -= (start.tv_sec*1000)+(start.tv_usec/1000);
        printf("RTT = %d ms\n\n");
    }

    //test for connect failure
    if(connect_result < 0){
        printf("\nconnect() failure\n");
        return 1;
    }

    int out_len = strlen(out_buffer);

    //send buffer
    int send_result = send(socketfd, out_buffer, strlen(out_buffer), 0);
    if(send_result != out_len){
        printf("\n send() sent different number of bytes than expected \n");
        return 1;
    }

    //receive stream from socket
    while(1){
        //set bytesRcvd, fill in_buffer, and check for errors
        if((bytesRcvd = recv(socketfd, in_buffer, IN_BUFFER_LEN - 1, 0)) <= 0){
            printf("\n recv() failed or connection lost \n");
            break;
        }


        totalBytesRcvd += bytesRcvd;
        
        //terminate string
        in_buffer[bytesRcvd] = '\0';
        printf("%s", in_buffer);

        //Once we get the end of the HTML page, close the connection
        if(strstr(in_buffer, "</html>") != NULL){
            break;
        }
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
