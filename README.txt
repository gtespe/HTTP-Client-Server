By Grant Espe

To Compile:

    Simply execute 'make' in your command prompt.
    This will compile both the server and the client.

    if for some weird reason the makefile doesn't work:
        gcc -c http_client.c
        gcc http_client.o -o http_client

        gcc -pthread http_server.c
        gcc -pthread http_server.o -o http_server

To Run:
    
    Use execute one of the following commands:
        ./http_client [-p]  <url>  <port>
        ./http_server <port>

I did manage to get the server multithreaded.

To close the server, send a SIGINT signal with ctrl+c

