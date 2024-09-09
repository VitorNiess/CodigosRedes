#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>

#define PORT 8080
#define MAX_PACKET_SIZE 65536

void handle_error(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void receive_file(int server_socket) {
    char buffer[MAX_PACKET_SIZE];
    size_t bytes_received;

    while ((bytes_received = recv(server_socket, buffer, sizeof(buffer), 0)) > 0) {
        // Processar os dados recebidos (por exemplo, gravar em um arquivo)
        // Aqui, estamos apenas imprimindo os dados recebidos.
        fwrite(buffer, 1, bytes_received, stdout);
    }

    if (bytes_received == -1) {
        handle_error("Erro ao receber dados do servidor");
    }
}

int main() {
    struct timeval start, end;
    long seconds, microseconds;

    gettimeofday(&start, NULL);

    int client_socket;
    struct sockaddr_in server_address;

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        handle_error("Falha ao criar o socket do cliente");
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        handle_error("Falha ao conectar-se ao servidor");
    }

    printf("Conectado ao servidor\n");

    // Enviar solicitação ao servidor (por exemplo, "GET /file.txt")
    const char *request = "GET /file.txt";
    if (send(client_socket, request, strlen(request), 0) == -1) {
        handle_error("Erro ao enviar solicitação ao servidor");
    }

    // Receber o arquivo do servidor
    receive_file(client_socket);

    // Fechar o socket do cliente
    close(client_socket);

    gettimeofday(&end, NULL);

    seconds = end.tv_sec - start.tv_sec;
    microseconds = end.tv_usec - start.tv_usec;
    double elapsed = seconds + microseconds * 1e-6;

    printf("Tempo decorrido: %f segundos\n", elapsed);

    return 0;
}
