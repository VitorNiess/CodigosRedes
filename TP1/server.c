#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(){
    char *ip = "127.0.0.1";
    int port = 5566;
    int server_socket, client_socket1, client_socket2, n;
    struct sockaddr_in server_addr, client1, client2;;
    socklen_t addr_size;
    char buffer[1024];

    char player1_move[10], player2_move[10], result[200], winner[200];
    int player1_points = 0, player2_points = 0;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0){
        perror("[-] Erro no socket.");
        exit(1);
    }
    printf("[+] Socket criado.\n");

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    n = bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (n < 0){
        perror("[-] Erro na conexão.");
        exit(1);
    }
    printf("[+] Conexão feita na porta: %d\n", port);

    listen(server_socket, 5);
    printf("Aguardando os jogadores...\n");

    int c = sizeof(struct sockaddr_in);
    client_socket1 = accept(server_socket, (struct sockaddr *)&client1, &c);
    printf("Jogador 1 conectado!\n");
    
    client_socket2 = accept(server_socket, (struct sockaddr *)&client2, &c);
    printf("Jogador 2 conectado!\n");

    while (1){
        recv(client_socket1, player1_move, 10, 0);
        recv(client_socket2, player2_move, 10, 0);
        printf("Jogador 1 escolheu: %s\nJogador 2 escolheu: %s\n", player1_move, player2_move);

        if (strcmp(player1_move, "pedra") == 0){

            if (strcmp(player2_move, "tesoura") == 0){
                printf("\nJogador 1 venceu!\n");
                player1_points++;
                sprintf(result, "Jogador 1 venceu!\nPlacar -> Jogador 1: %d / Jogador 2: %d", player1_points, player2_points);
            } else if (strcmp(player2_move, "papel") == 0){
                printf("Jogador 2 venceu!\n");
                player2_points++;
                sprintf(result, "Jogador 2 venceu!\nPlacar -> Jogador 1: %d / Jogador 2: %d", player1_points, player2_points);
            } else{
                printf("Empate!\n");
                sprintf(result, "Empate!\nPlacar -> Jogador 1: %d / Jogador 2: %d", player1_points, player2_points);
            }

        } else if (strcmp(player1_move, "tesoura") == 0){

            if (strcmp(player2_move, "papel") == 0){
                printf("\nJogador 1 venceu!\n");
                player1_points++;
                sprintf(result, "Jogador 1 venceu!\nPlacar -> Jogador 1: %d / Jogador 2: %d", player1_points, player2_points);
            } else if (strcmp(player2_move, "pedra") == 0){
                printf("Jogador 2 venceu!\n");
                player2_points++;
                sprintf(result, "Jogador 2 venceu!\nPlacar -> Jogador 1: %d / Jogador 2: %d", player1_points, player2_points);
            } else{
                printf("Empate!\n");
                sprintf(result, "Empate!\nPlacar -> Jogador 1: %d / Jogador 2: %d", player1_points, player2_points);
            }

        } else if (strcmp(player1_move, "papel") == 0){

            if (strcmp(player2_move, "pedra") == 0){
                printf("\nJogador 1 venceu!\n");
                player1_points++;
                sprintf(result, "Jogador 1 venceu!\nPlacar -> Jogador 1: %d / Jogador 2: %d", player1_points, player2_points);
            } else if (strcmp(player2_move, "tesoura") == 0){
                printf("Jogador 2 venceu!\n");
                player2_points++;
                sprintf(result, "Jogador 2 venceu!\nPlacar -> Jogador 1: %d / Jogador 2: %d", player1_points, player2_points);
            } else{
                printf("Empate!\n");
                sprintf(result, "Empate!\nPlacar -> Jogador 1: %d / Jogador 2: %d", player1_points, player2_points);
            }

        }

        if(player1_points == 5){
            strcpy(winner, "\nO jogador 1 ganhou, pois chegou em 5 pontos!\nObrigado por jogar!");
            send(client_socket1, winner, sizeof(winner), 0);
            send(client_socket2, winner, sizeof(winner), 0);
            printf("\nPlayer 1 has won the game!");
            break;
        }

        if(player2_points == 5){
            strcpy(winner, "\nO jogador 2 ganhou, pois chegou em 5 pontos!\nObrigado por jogar!");
            send(client_socket1, winner, sizeof(winner), 0);
            send(client_socket2, winner, sizeof(winner), 0);
            printf("\nPlayer 1 has won the game!");
            break;
        }

        send(client_socket1, result, sizeof(result), 0);
        send(client_socket2, result, sizeof(result), 0);
    }

    close(client_socket1);
    close(client_socket2);

    return 0;
}