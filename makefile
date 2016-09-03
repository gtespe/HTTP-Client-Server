all: http_client.o http_client

http_client.o: http_client.c
	gcc -c http_client.c

http_client: http_client.o
	gcc http_client.o -o http_client
