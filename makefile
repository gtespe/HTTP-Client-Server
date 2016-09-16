all: http_client.o http_client http_server.o http_server

http_client.o: http_client.c
	gcc -c http_client.c

http_client: http_client.o
	gcc http_client.o -o http_client

http_server.o: http_server.c
	gcc -pthread -c http_server.c

http_server: http_server.o
	gcc -pthread http_server.o -o http_server

clean:
	rm *.o
	rm http_server
	rm http_client
