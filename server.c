#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_HEADERS 100

typedef struct {
    char method[10];
    char path[100];
    char protocol[20];
    char headers[MAX_HEADERS][BUFFER_SIZE];
    int header_count;
} RequestInfo;

RequestInfo parse_request(const char *buffer) {
    RequestInfo info = {0};
    const char *line_end = strstr(buffer, "\r\n");
    
    if (line_end) {
        char request_line[BUFFER_SIZE] = {0};
        strncpy(request_line, buffer, line_end - buffer);
        sscanf(request_line, "%9s %99s %19s", info.method, info.path, info.protocol);

        const char *headers_start = line_end + 2;
        const char *headers_end = strstr(headers_start, "\r\n\r\n");
        if (headers_end) {
            char headers_section[BUFFER_SIZE] = {0};
            strncpy(headers_section, headers_start, headers_end - headers_start);
            
            char *header_line = strtok(headers_section, "\r\n");
            while (header_line && info.header_count < MAX_HEADERS) {
                strncpy(info.headers[info.header_count++], header_line, BUFFER_SIZE - 1);
                header_line = strtok(NULL, "\r\n");
            }
        }
    }
    return info;
}

void send_response(int client_socket, int status_code, const char *content_type, const char *body) {
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response),
             "HTTP/1.1 %d %s\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "Connection: close\r\n"
             "\r\n%s",
             status_code, (status_code == 200) ? "OK" : "Bad Request",
             content_type, strlen(body), body);

    send(client_socket, response, strlen(response), 0);
}

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);

    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        perror("Receive failed or connection closed");
        close(client_socket);
        return NULL;
    }

    RequestInfo request_info = parse_request(buffer);
    printf("Received request:\nMethod: %s\nPath: %s\nProtocol: %s\n", 
           request_info.method, request_info.path, request_info.protocol);

    for (int i = 0; i < request_info.header_count; i++) {
        if (strstr(request_info.headers[i], "Content-Type")) {
            printf("Header: %s\n", request_info.headers[i]);
        }
    }

    if (strcmp(request_info.method, "POST") == 0) {
        send_response(client_socket, 200, "text/plain", "POST request received!");
    } else if (strcmp(request_info.method, "GET") == 0) {
        send_response(client_socket, 200, "text/plain", "Hello, World!");
    } else {
        send_response(client_socket, 400, "text/plain", "Unsupported Method");
    }

    close(client_socket);
    return NULL;
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 10) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);

    while (1) {
        int *client_socket = malloc(sizeof(int));
        if (!client_socket) {
            perror("Failed to allocate memory");
            continue;
        }

        *client_socket = accept(server_socket, NULL, NULL);
        if (*client_socket < 0) {
            perror("Accept failed");
            free(client_socket);
            continue;
        }

        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, client_socket) != 0) {
            perror("Thread creation failed");
            close(*client_socket);
            free(client_socket);
            continue;
        }

        pthread_detach(thread);
    }

    close(server_socket);
    return 0;
}
