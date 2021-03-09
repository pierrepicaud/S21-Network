#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>

#define PORT 1337
#define BUFLEN 512
#define MAXCONNECTIONS 8 // It doesn't matter if the clients ip are trash
//#define SERVER_IP "10.241.1.171"
//#define SERVER_IP "10.91.52.64"
//#define SERVER_IP "192.168.0.20"

#define SERVER_IP "10.91.93.226"

void server() {


    // Creating socket file descriptor
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully\n");

    // Clean buffers
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));


    // Filling server information
    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP); /* internet address */
    server_addr.sin_port = htons(PORT); /* port in network byte order */


    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Done with binding\n");
    printf("Listening for incoming messages...\n\n");


    struct sockaddr_in *connected_clients[MAXCONNECTIONS];
    int clients_count = 0;
    char message[BUFLEN];

    while (1) {

        // Receive message
        struct sockaddr_in client;
        int len;// = sizeof(client);
        ssize_t message_len = recvfrom(sockfd, message, BUFLEN, MSG_WAITALL, (struct sockaddr *) &client, &len);
        if (message_len == -1) { perror("recvfrom(...) = -1"); }
        message[message_len] = '\0'; // Make it safer to manage

        // Check if it's known host
        unsigned long client_ip = client.sin_addr.s_addr;
        bool known_host = false;
        for (int i = 0; i < clients_count; ++i) {
            if (connected_clients[i]->sin_addr.s_addr == client_ip) {
                known_host = true;
                printf("Known Host.\n");
                break;
            }
        }
        if (!known_host) {
            connected_clients[clients_count] = malloc(sizeof(client));
            memcpy(connected_clients[clients_count], &client, sizeof(client));
            ++clients_count;
            printf("Added Client.\n");
        }


        printf("Received message from %s | %i: %s\n",
               inet_ntoa(client.sin_addr), ntohs(client.sin_port), message);


        // broadcast message to all connected clients
        for (int i = 0; i < clients_count; ++i) {
//            if (connected_clients[i]->sin_addr.s_addr == client_ip) {
                fprintf(stderr, "Broadcasting message to %s | %i\n", inet_ntoa(connected_clients[i]->sin_addr),ntohs(connected_clients[i]->sin_port) );
                sendto(sockfd, message, message_len, 0, (const struct sockaddr *) connected_clients[i],
                       sizeof(*connected_clients[i]));
//            }
        }
    }

}

void client() {

    int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[BUFLEN], client_message[BUFLEN];
    int server_struct_length = sizeof(server_addr);

    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (socket_desc < 0) {
        printf("Error while creating socket\n");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully\n");

    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    while (1) {

        // Get input from the user:
        printf("Enter message: ");
        fgets(client_message, sizeof(client_message), stdin);
        fflush(stdin);

        // Send the message to server:
        if (sendto(socket_desc, client_message, strlen(client_message), 0,
                   (struct sockaddr *) &server_addr, server_struct_length) < 0) {
            printf("Unable to send message\n");
            exit(EXIT_FAILURE);;
        }

        // Receive the server's response:
        if (recvfrom(socket_desc, server_message, sizeof(server_message), 0,
                     (struct sockaddr *) &server_addr, &server_struct_length) < 0) {
            printf("Error while receiving server's msg\n");
            exit(EXIT_FAILURE);;
        }

//        printf("Server's response: %s\n", server_message);
        printf("Server's response: %s\n", server_message);
        fflush(stdout);

        // Close the socket:
//        close(socket_desc);
    }
}

int main() {
//    server();
    client();
    return 0;
}
