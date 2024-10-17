#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main(){
	
	int skt = socket(AF_INET, SOCK_STREAM, 0);
	
	struct sockaddr_in host_address;
    host_address.sin_family = AF_INET;
    host_address.sin_port = htons(PORT);
    host_address.sin_addr.s_addr = htonl(INADDR_ANY); 
	
	bind(skt, (struct sockaddr*)&host_address, sizeof(host_address) );
	
	listen(skt, 10);
	
	int acptsocket = accept(skt, 0, 0);
	
	char buffer[256] = {0};
	
	recv(acptsocket, buffer, 255, 0);
	
	
    const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
    send(acptsocket, response, strlen(response), 0);

    
    close(acptsocket);
    close(skt);
	
	return 0;}
