#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(){
    char *ip = "127.0.0.1";
    int port = 5566;

    int sock;
    struct sockaddr_in addr;
    socklen_t addr_size;
    char buffer[1024];
    int n;
    char move[10], result[200];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0){
        perror("[-] Erro no socket.");
        exit(1);
    }
    printf("[+] Socket criado.\n");

    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = port;
    addr.sin_addr.s_addr = inet_addr(ip);

    connect(sock, (struct sockaddr*)&addr, sizeof(addr));

    while (1){
        printf("Faca sua escolha! Pedra, papel ou tesoura? ");
        scanf("%s", move);

        if(strcmp(move, "pedra") != 0 && strcmp(move, "tesoura") != 0 && strcmp(move, "papel") != 0){
            printf("\nEscolha uma opcao valida!\n");
            continue;
        }

        send(sock, move, sizeof(move), 0);
        recv(sock, result, sizeof(result), 0);

        if(strcmp(result, "\nO jogador 1 ganhou, pois chegou em 5 pontos!\nObrigado por jogar!") == 0){
            printf("\n%s\n\n", result);
            break;
        }

        else if(strcmp(result, "\nO jogador 2 ganhou, pois chegou em 5 pontos!\nObrigado por jogar!") == 0){
            printf("\n%s\n\n", result);
            break;
        }

        printf("\n%s\n\n", result);
    }

    close(sock);
    printf("Desconectado do servidor.\n");

    return 0;
}