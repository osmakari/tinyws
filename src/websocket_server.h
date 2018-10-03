#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#define MAX_CLIENTS 32
#define SERVER_GUID "a7726808-6b4f-4bbe-b4e2-2a59170032b0"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

struct _ws_cli {
    int socket;
    uint8_t auth;
};


struct websocket_server {
    struct _ws_cli *clients[MAX_CLIENTS];
    char *rx_buffer;
    char *tx_buffer;
};

#endif