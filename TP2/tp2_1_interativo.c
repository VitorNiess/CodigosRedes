#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 8080
#define FILE_PATH "file.txt"
#define IMAGE_PATH "image.png"

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

    // Obtém o tamanho do arquivo
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Lê o conteúdo do arquivo
    char *file_content = malloc(file_size);
    fread(file_content, 1, file_size, file);
    fclose(file);

    // Monta a resposta HTTP com o conteúdo do arquivo
    char response_header[1024];
    sprintf(response_header, "HTTP/1.1 200 OK\nContent-Type: %s\nContent-Length: %ld\n\n", content_type, file_size);

    // Envia o cabeçalho da resposta
    send(client_socket, response_header, strlen(response_header), 0);

    // Envia o conteúdo do arquivo
    send(client_socket, file_content, file_size, 0);

    // Libera a memória alocada para o conteúdo do arquivo
    free(file_content);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Cria um socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        handle_error("Falha ao criar o socket");
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Liga o socket ao endereço e à porta
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        handle_error("Falha ao fazer o bind");
    }

    // Escuta por conexões
    if (listen(server_fd, 3) == -1) {
        handle_error("Falha ao escutar");
    }

    while (1) {
        printf("Aguardando conexões...\n");

        // Aceita uma conexão de cliente
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) == -1) {
            handle_error("Falha ao aceitar a conexão");
        }

        printf("Conexão aceita\n");

        // Verifica se a solicitação é para o arquivo ou resposta simples
        char buffer[1024];
        ssize_t bytes_received = recv(new_socket, buffer, sizeof(buffer), 0);

        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';

            if (strstr(buffer, "GET /file.txt") != NULL) {
                // Solicitação para o arquivo de texto
                serve_file(new_socket, FILE_PATH, "text/plain");
            } else if (strstr(buffer, "GET /image.png") != NULL) {
                // Solicitação para a imagem PNG
                serve_file(new_socket, IMAGE_PATH, "image/png");
            } else {
                // Resposta simples
                char *response = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello, World!";
                if (send(new_socket, response, strlen(response), 0) == -1) {
                    perror("Falha ao enviar resposta");
                }
                printf("Resposta enviada\n");
            }
        }

        // Fecha o socket do cliente
        close(new_socket);
        printf("Conexão fechada\n");
    }

    return 0;
}
