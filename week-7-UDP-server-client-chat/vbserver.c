#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define PORT 1337
#define MAX_MESSAGE_SIZE 512
#define MAX_CLIENTS 8
#define SERVER_IP "10.91.52.64"

int main(void){
    // Create socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock <= 0){
        perror("Error: can't create socket\n");
        exit(EXIT_FAILURE);
    }

    // bind socket
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    // serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serveraddr.sin_port = htons(PORT);

    if(bind(sock, (const struct sockaddr*)&serveraddr, sizeof(serveraddr))){
        perror("ERROR: Can't bind socket\n");
        exit(EXIT_FAILURE);
    }

    printf("Server Online.\n");

    struct sockaddr_in *connected_clients[MAX_CLIENTS];
    int clients_count = 0;

    char message[MAX_MESSAGE_SIZE];
    while(1){
        // Receive message
        struct sockaddr_in client;
        int len;
        ssize_t message_len = recvfrom(sock, message, MAX_MESSAGE_SIZE, 0, (struct sockaddr*)&client, &len);
        message[message_len] = '\0';

        // Check if it's a known client
        unsigned long client_ip = client.sin_addr.s_addr;
        bool known_host = false;
        for(int i = 0; i < clients_count; ++i){
            if(connected_clients[i]->sin_addr.s_addr == client_ip){
                known_host = true;
                break;
            }
        }
        if(!known_host){
            connected_clients[clients_count] = malloc(sizeof(client));
            memcpy(connected_clients[clients_count], &client, sizeof(client));
            ++clients_count;
        }

        printf("From: %s Message: %s\n", inet_ntoa(client.sin_addr), message);

        // broadcast message to all connected clients
        for(int i = 0; i < clients_count; ++i){
            if(connected_clients[i]->sin_addr.s_addr != client_ip){
                fprintf(stderr, "Broadcasting message to %s\n", inet_ntoa(connected_clients[i]->sin_addr));
                sendto(sock, message, message_len, 0, (const struct sockaddr*)connected_clients[i], sizeof(*connected_clients[i]));
            }
        }
    }

    close(sock);
    return EXIT_SUCCESS;
}
