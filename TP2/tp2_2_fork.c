#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PORT 8080
#define FILE_PATH "file.txt"
#define IMAGE_PATH "image.png"

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

void handle_client(int client_socket) {
    // Verifica a solicitação
    char buffer[4096];
    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';

        if (strstr(buffer, "GET /file.txt") != NULL) {
            // Solicitação para o arquivo de texto
            serve_file(client_socket, FILE_PATH, "text/plain");
        } else if (strstr(buffer, "GET /image.png") != NULL) {
            // Solicitação para a imagem PNG
            serve_file(client_socket, IMAGE_PATH, "image/png");
        } else {
            // Resposta simples
            char *response = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello, World!";
            send(client_socket, response, strlen(response), 0);
            printf("Resposta enviada\n");
        }
    }

    // Fecha o socket do cliente
    close(client_socket);
    printf("Conexão fechada\n");
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Cria um socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Falha ao criar o socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Liga o socket ao endereço e à porta
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Falha ao fazer o bind");
        exit(EXIT_FAILURE);
    }

    // Escuta por conexões
    if (listen(server_fd, 3) < 0) {
        perror("Falha ao escutar");
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("Aguardando conexões...\n");

        // Aceita uma conexão de cliente
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Falha ao aceitar a conexão");
            exit(EXIT_FAILURE);
        }

        printf("Conexão aceita\n");

        // Cria um processo filho para atender ao cliente
        pid_t pid = fork();
        if (pid < 0) {
            perror("Falha ao criar o processo filho");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            // Processo filho
            close(server_fd);  // O processo filho não precisa do socket do servidor
            handle_client(client_socket);
            exit(0);  // O processo filho termina após o tratamento do cliente
        } else {
            // Processo pai
            close(client_socket);  // O processo pai não precisa do socket do cliente

            // Espera pelo processo filho para evitar zumbis
            waitpid(pid, NULL, WNOHANG);
        }
    }

    return 0;
}
