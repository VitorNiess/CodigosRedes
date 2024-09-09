#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>

#define PORT 8080
#define MAX_CLIENTS 5
int client_sockets[MAX_CLIENTS];

void handle_client(int client_socket, const char *request) {
    
    // Verifica se a solicitação é para o arquivo ou resposta simples
    if (strstr(request, "GET /file.txt") != NULL) {
        // Solicitação para o arquivo de texto
        FILE *file = fopen("file.txt", "rb");
        if (file != NULL) {
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
            sprintf(response_header, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: %ld\n\n", file_size);

            // Envia o cabeçalho da resposta
            send(client_socket, response_header, strlen(response_header), 0);

            // Envia o conteúdo do arquivo
            send(client_socket, file_content, file_size, 0);

            // Libera a memória alocada para o conteúdo do arquivo
            free(file_content);
        } else {
            // Se falhar ao abrir o arquivo, envia uma resposta de erro
            char *error_response = "HTTP/1.1 500 Internal Server Error\nContent-Type: text/plain\nContent-Length: 21\n\nError opening file.txt";
            send(client_socket, error_response, strlen(error_response), 0);
        }
    } else if (strstr(request, "GET /image.png") != NULL) {
        // Solicitação para a imagem PNG
        FILE *file = fopen("image.png", "rb");
        if (file != NULL) {
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
            sprintf(response_header, "HTTP/1.1 200 OK\nContent-Type: image/png\nContent-Length: %ld\n\n", file_size);

            // Envia o cabeçalho da resposta
            send(client_socket, response_header, strlen(response_header), 0);

            // Envia o conteúdo do arquivo
            send(client_socket, file_content, file_size, 0);

            // Libera a memória alocada para o conteúdo do arquivo
            free(file_content);
        } else {
            // Se falhar ao abrir o arquivo, envia uma resposta de erro
            char *error_response = "HTTP/1.1 500 Internal Server Error\nContent-Type: text/plain\nContent-Length: 22\n\nError opening image.png";
            send(client_socket, error_response, strlen(error_response), 0);
        }
        
    } else if (strstr(request, "CHAT:") != NULL) {
        // Código para lidar com mensagens de chat
        // Extrai a mensagem do formato "CHAT: mensagem"
        const char *chat_message = strstr(request, "CHAT:") + strlen("CHAT:");
        printf("Chat message: %s\n", chat_message);

        // Envie a mensagem para todos os clientes, exceto o remetente
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int other_client_socket = client_sockets[i];
            if (other_client_socket > 0 && other_client_socket != client_socket) {
                send(other_client_socket, chat_message, strlen(chat_message), 0);
            }
        }
        
    } else {
        // Resposta simples
        char *response = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello, World!";
        send(client_socket, response, strlen(response), 0);
    }

    // Fecha o socket do cliente após a resposta
    close(client_socket);
}

int main() {
    int server_fd;
    fd_set master, read_fds;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int fdmax, activity, new_socket;

    // Inicializa a matriz de descritores de socket do cliente
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = 0;
    }

    // Inicializa os conjuntos mestre e de leitura
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    // Cria um socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
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
    if (listen(server_fd, 10) < 0) {
        perror("Falha ao escutar");
        exit(EXIT_FAILURE);
    }

    // Adiciona o socket do servidor ao conjunto mestre
    FD_SET(server_fd, &master);
    fdmax = server_fd; /* até agora, é este*/

    printf("Aguardando conexões...\n");

    while (1) {
        read_fds = master; // Copia o conjunto mestre

        // Aguarda atividade em qualquer socket
        activity = select(fdmax + 1, &read_fds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("Erro na chamada select");
            exit(EXIT_FAILURE);
        }

        // Se o socket do servidor está pronto para leitura, é uma nova conexão
        if (FD_ISSET(server_fd, &read_fds)) {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) == -1) {
                perror("Falha ao aceitar a conexão");
                exit(EXIT_FAILURE);
            }

            printf("Conexão aceita\n");

            // Adiciona o novo socket à matriz de descritores
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = new_socket;
                    break;
                }
            }

            // Adiciona o novo socket ao conjunto mestre
            FD_SET(new_socket, &master);

            // Atualiza o valor máximo do descritor de arquivo, se necessário
            if (new_socket > fdmax) {
                fdmax = new_socket;
            }
        }

        // Processa dados de clientes
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int client_socket = client_sockets[i];
            if (client_socket > 0 && FD_ISSET(client_socket, &read_fds)) {
                // Handle the client's request and send a response
                char buffer[1024];
                ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        
                if (bytes_received > 0) {
                    buffer[bytes_received] = '\0';
                    handle_client(client_socket, buffer);
                }
        
                // Remove o socket do conjunto mestre e o fecha
                FD_CLR(client_socket, &master);
                close(client_socket);
                client_sockets[i] = 0;
        
                printf("Conexão fechada\n");
            }
        }
    }

    return 0;
}
