#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#define MAX_CLIENTS 32
#define SERVER_GUID "a7726808-6b4f-4bbe-b4e2-2a59170032b0"
#define RECEIVE_BUFFER_LENGTH 2048

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

struct websocket_server;

struct ws_client {
    int socket;
    uint8_t auth;
    struct websocket_server *connection;
    struct sockaddr_in addr;
    int _client_index;
};


struct websocket_server {
    struct ws_client *clients[MAX_CLIENTS];
    uint16_t port;
    struct sockaddr_in addr;
    int sockfd;
    char *rx;
    void (*onmessage)(struct ws_client *from, char *data, uint16_t length);
    void (*onopen)(struct ws_client *client);
};

struct websocket_server *WS_server (uint16_t port);
int WS_server_update (struct websocket_server *ws);

#endif