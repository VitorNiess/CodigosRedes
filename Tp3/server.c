#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 8080
#define FILE_PATH "arq.txt"
#define IMAGE_PATH "img.png"
#define MAX_PACKET_SIZE 65536

void handle_error(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void serve_file(int client_socket, const char *file_path, const char *content_type) {
    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        perror("Falha ao abrir o arquivo");
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char response_header[1024];
    sprintf(response_header, "HTTP/1.1 200 OK\nContent-Type: %s\nContent-Length: %ld\n\n", content_type, file_size);

    send(client_socket, response_header, strlen(response_header), 0);

    char buffer[MAX_PACKET_SIZE];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(client_socket, buffer, bytes_read, 0);
        usleep(10000);  // Adiciona um pequeno atraso entre os pacotes (opcional)
    }

    fclose(file);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        handle_error("Falha ao criar o socket");
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        handle_error("Falha ao fazer o bind");
    }

    if (listen(server_fd, 3) == -1) {
        handle_error("Falha ao escutar");
    }

    while (1) {
        printf("Aguardando conexões...\n");

        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) == -1) {
            handle_error("Falha ao aceitar a conexão");
        }

        printf("Conexão aceita\n");

        char buffer[4096];
        ssize_t bytes_received = recv(new_socket, buffer, sizeof(buffer), 0);

        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';

            if (strstr(buffer, "GET /file.txt") != NULL) {
                serve_file(new_socket, FILE_PATH, "text/plain");
            } else if (strstr(buffer, "GET /image.png") != NULL) {
                serve_file(new_socket, IMAGE_PATH, "image/png");
            } else {
                char *response = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello, World!";
                if (send(new_socket, response, strlen(response), 0) == -1) {
                    perror("Falha ao enviar resposta");
                }
                printf("Resposta enviada\n");
            }
        }

        close(new_socket);
        printf("Conexão fechada\n");
    }

    return 0;
}
